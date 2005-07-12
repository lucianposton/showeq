/*
 * packetstream.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

/* Implementation of EQPacketStream class */
#include "packetstream.h"
#include "packetformat.h"
#include "packetinfo.h"
#include "diagnosticmessages.h"

#include <stdio.h>

//----------------------------------------------------------------------
// Macros

// The following defines are used to diagnose packet handling behavior
// this define is used to diagnose packet processing (in processPacket mostly)
//#define PACKET_PROCESS_DIAG 3

// this define is used to diagnose cache handling (in processPacket mostly)
//#define PACKET_CACHE_DIAG 3

// this define is used to debug packet info (in dispatchPacket mostly)
//#define PACKET_INFO_DIAG 3

// this define is used to debug packet decode info (decompression)
//#define PACKET_DECODE_DIAG 3

// this define is used to debug sessions (request, response, disconnect)
//#define PACKET_SESSION_DIAG 3

// diagnose structure size changes
#define PACKET_PAYLOAD_SIZE_DIAG 1

// used to translate EQStreamID to a string for debug and reporting
static const char* const EQStreamStr[] = {"client-world", "world-client", "client-zone", "zone-client"};

//----------------------------------------------------------------------
// constants

// Arbitrary cutoff used to determine the relationship between
// an arq sequence may be from a wrap and not just be really old.
const int16_t arqSeqWrapCutoff = 1024;

//----------------------------------------------------------------------
// EQPacketStream class methods

////////////////////////////////////////////////////
// Constructor
EQPacketStream::EQPacketStream(EQStreamID streamid, uint8_t dir, 
			       uint16_t arqSeqGiveUp,
			       EQPacketOPCodeDB& opcodeDB, 
			       QObject* parent, const char* name)
  : QObject(parent, name),
    m_opcodeDB(opcodeDB),
    m_dispatchers(61),  // prime number that should be plenty large
    m_streamid(streamid),
    m_dir(dir),
    m_packetCount(0),
    m_session_tracking_enabled(0),
    m_maxCacheCount(0),
    m_arqSeqExp(0),
    m_arqSeqGiveUp(arqSeqGiveUp),
    m_arqSeqFound(false),
    m_fragment(streamid),
    m_sessionId(0),
    m_sessionKey(0),
    m_sessionClientPort(0),
    m_maxLength(0),
    m_decodeKey(0),
    m_validKey(true)
{
  m_dispatchers.setAutoDelete(true);
}

////////////////////////////////////////////////////
// Destructor
EQPacketStream::~EQPacketStream()
{
  reset();
}

////////////////////////////////////////////////////
// setup connection
bool EQPacketStream::connect2(const QString& opcodeName, 
			      const char* payloadType,  EQSizeCheckType szt, 
			      const QObject* receiver, const char* member)
{
  const EQPacketOPCode* opcode = m_opcodeDB.find(opcodeName);
  if (!opcode)
  {
    seqDebug("connect2: Unknown opcode '%s' with payload type '%s'",
	     (const char*)opcodeName, payloadType);
    seqDebug("\tfor receiver '%s' of type '%s' to member '%s'",
	     receiver->name(), receiver->className(), member);
    return false;
  }

  EQPacketPayload* payload;

  // try to find a matching payload for this opcode
  EQPayloadListIterator pit(*opcode);
  while ((payload = pit.current()) != 0)
  {
    // if all the parameters match, then use this payload
    if ((payload->dir() & m_dir) && 
	(payload->typeName() == payloadType) && 
	(payload->sizeCheckType() == szt))
      break;

    ++pit;
  }

  // if no payload found, create one and issue a warning
  if (!payload)
  {
    seqDebug("connect2: Warning! opcode '%s' has no matching payload.",
	     (const char*)opcodeName);
    seqDebug("\tdir '%d' payload '%s' szt '%d'",
	    m_dir, payloadType, szt);
    seqDebug("\tfor receiver '%s' of type '%s' to member '%s'",
	    receiver->name(), receiver->className(), member);

    return false;
  }

  // attempt to find an existing dispatch
  EQPacketDispatch* dispatch = m_dispatchers.find((void*)payload);

  // if no existing dispatch was found, create one
  if (!dispatch)
  {
    // construct a name for the dispatch
    QCString dispatchName(256);
    dispatchName.sprintf("PacketDispatch:%s:%s:%d:%s:%d",
			 (const char*)name(), (const char*)opcodeName,
			 payload->dir(), (const char*)payload->typeName(), 
			 payload->sizeCheckType());

    // create new dispatch object
    dispatch = new EQPacketDispatch(this, dispatchName);

    // insert dispatcher into dispatcher dictionary
    m_dispatchers.insert((void*)payload, dispatch);
  }

  // attempt to connect the dispatch object to the receiver
  return dispatch->connect(receiver, member);
}

////////////////////////////////////////////////////
// stream reset
void EQPacketStream::reset()
{
  resetCache();
  m_fragment.reset();
  m_arqSeqExp = 0;
  m_arqSeqFound = false;
  m_sessionClientPort = 0;
  m_sessionId = 0;
  m_sessionKey = 0;
}

////////////////////////////////////////////////////
// cache reset
void EQPacketStream::resetCache()
{
    // first delete all the entries
    EQPacketMap::iterator it = m_cache.begin();
    EQProtocolPacket* packet;
#ifdef PACKET_CACHE_DIAG
    seqDebug("Clearing Cache[%s]: Count: %d", EQStreamStr[m_streamid], m_cache.size());
#endif 
    while (it != m_cache.end())
    {
      packet = it->second;
      delete packet;
      it++;
    }

    // now clear the cache
#ifdef PACKET_CACHE_DIAG
    seqDebug("Resetting sequence cache[%s]", EQStreamStr[m_streamid]);
#endif
    m_cache.clear();
    emit cacheSize(0, m_streamid);
}

////////////////////////////////////////////////////
// setCache 
// adds current packet to specified cache
void EQPacketStream::setCache(uint16_t serverArqSeq, EQProtocolPacket& packet)
{
   // check if the entry already exists in the cache
   EQPacketMap::iterator it = m_cache.find(serverArqSeq);

   if (it == m_cache.end())
   {
   // entry doesn't exist, so insert an entry into the cache

#ifdef PACKET_PROCESS_DIAG
      seqDebug("SEQ: Insert arq (%04x) stream %d into cache", serverArqSeq, m_streamid);
#endif

      m_cache.insert(EQPacketMap::value_type(serverArqSeq, 
         new EQProtocolPacket(packet, true)));
      emit cacheSize(m_cache.size(), (int)m_streamid);
   }
   else
   {
     // replacing an existing entry, make sure the new data is valid
#ifdef APPLY_CRC_CHECK
     if (! packet.hasCRC() || calculateCRC(packet) == packet.crc())
#endif
     {
#ifdef PACKET_PROCESS_DIAG
        seqDebug("SEQ: Update arq (%04x) stream %d in cache", serverArqSeq, m_streamid);
#endif

        *it->second = packet;
     }
#if defined(PACKET_PROCESS_DIAG) && defined(APPLY_CRC_CHECK)
     else
        seqDebug("SEQ: Not Updating arq (%04x) stream %d into cache, CRC error!",
               serverArqSeq, m_streamid);
#endif
   }

#ifdef PACKET_CACHE_DIAG
   if (m_cache.size() > m_maxCacheCount)
      m_maxCacheCount = m_cache.size();
#endif // PACKET_CACHE_DIAG
}

////////////////////////////////////////////////////
// Cache processing
void EQPacketStream::processCache()
{
#if defined(PACKET_CACHE_DIAG)
  seqDebug("SEQ: START checking stream %s cache, arq %04x, cache count %04d",
         EQStreamStr[m_streamid], m_arqSeqExp, m_cache.size());
#endif
  EQPacketMap::iterator it;
  EQPacketMap::iterator eraseIt;
  EQProtocolPacket* packet;

  // check if the cache has grown large enough that we should give up
  // on seeing the current serverArqSeqExp
  // 
  // If people see this a lot, they either have pathetic network cards, or
  // are having problems keeping up with packets (slow computer? Too much
  // net traffic?). Some possible solutions to this are to turn on session
  // tracking to filter out more PF_PACKET packets from getting passed out of
  // the kernel and to up the socket receive buffer sizes. See FAQ for
  // more information.
  if (m_cache.size() >= m_arqSeqGiveUp)
  {
    // ok, if the expected server arq sequence isn't here yet, give up
    
    // attempt to find the current expencted arq seq
    it = m_cache.find(m_arqSeqExp);
    
    // keep trying to find a new serverArqSeqExp if we haven't found a good
    // one yet...
    while(it == m_cache.end())
    {
      seqWarn("SEQ: Giving up on finding arq %04x in stream %s cache, skipping!",
	     m_arqSeqExp, EQStreamStr[m_streamid]);
      
      // incremente the expected arq sequence number
      m_arqSeqExp++;
      emit seqExpect(m_arqSeqExp, (int)m_streamid);
      
      // attempt to find the new current expencted arq seq
      it = m_cache.find(m_arqSeqExp);
    }
  }
  else
  {
    // haven't given up yet, just try to find the current serverArqSeqExp
    // attempt to find the current expected ARQ seq
    it = m_cache.find(m_arqSeqExp);
  }


  // iterate over cache until we reach the end or run out of
  // immediate followers
  while (it != m_cache.end())
  {
    // get the PacketFormat for the iterator
    packet = it->second;
    
    // make sure this is the expected packet
    // (we might have incremented to the one after the one returned
    // by find above).
    if (packet->arqSeq() != m_arqSeqExp)
      break;
    
#ifdef PACKET_CACHE_DIAG
    seqDebug("SEQ: found next arq %04x in stream %s cache, cache count %04d",
	   m_arqSeqExp, EQStreamStr[m_streamid], m_cache.size());
#endif
    
    // validate the packet with a crc check. If the packet is for an old
    // session, we probably shouldn't be using it!
#ifdef APPLY_CRC_CHECK
    if (packet->hasCRC() && packet->crc() != calculateCRC(*packet))
    {
#if defined (PACKET_CACHE_DIAG)
      // Something's screwed up
      seqDebug("SEQ: INVALID PACKET: Bad CRC in packet in stream %s cache with arq %04x! Droping it, but leaving expected seq as %04x",
	    EQStreamStr[m_streamid], packet->arqSeq(), m_arqSeqExp);
#endif

      // Need to drop from the cache
      eraseIt = it;
        
      // increment the current position iterator
      it++;
        
      // erase the packet from the cache
      m_cache.erase(eraseIt);
      emit cacheSize(m_cache.size(), (int)m_streamid);
        
    #ifdef PACKET_CACHE_DIAG
      seqDebug("SEQ: REMOVING arq %04x from stream %s cache, cache count %04d",
         packet->arqSeq(), EQStreamStr[m_streamid], m_cache.size());
    #endif
      // delete the packet
      delete packet;

      // No sense looping some more.
      break;
    }
    else
#endif /* APPLY_CRC_CHECK */
    {
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
      seqDebug("SEQ: Found next arq in stream %s cache, incrementing arq seq, %04x", 
	     EQStreamStr[m_streamid], packet->arqSeq());
#endif
    
      // Process the packet since it's next in the sequence and was just
      // received out of order
      processPacket(*packet, packet->isSubpacket());
      
      // Need to drop from the cache
      eraseIt = it;
      
      // increment the current position iterator
      it++;
      
      // erase the packet from the cache
      m_cache.erase(eraseIt);
      emit cacheSize(m_cache.size(), (int)m_streamid);
    
#ifdef PACKET_CACHE_DIAG
      seqDebug("SEQ: REMOVING arq %04x from stream %s cache, cache count %04d",
	     packet->arqSeq(), EQStreamStr[m_streamid], m_cache.size());
#endif
      // delete the packet
      delete packet;
  
      if (m_arqSeqExp == 0)
        it = m_cache.begin();
    }
  }
  
#ifdef PACKET_CACHE_DIAG
  seqDebug("SEQ: FINISHED checking stream %s cache, arq %04x, cache count %04d",
         EQStreamStr[m_streamid], m_arqSeqExp, m_cache.size());
#endif
}

void EQPacketStream::dispatchPacket(const uint8_t* data, size_t len, 
				    uint16_t opCode, 
				    const EQPacketOPCode* opcodeEntry)
{
  emit decodedPacket(data, len, m_dir, opCode, opcodeEntry);

  bool unknown = true;

  // unless there is an opcode entry, there is nothing to dispatch...
  if (opcodeEntry)
  {
    EQPacketPayload* payload;
    EQPacketDispatch* dispatch;

#ifdef PACKET_INFO_DIAG
    seqDebug(
	    "dispatchPacket: attempting to dispatch opcode %#04x '%s'",
	    opcodeEntry->opcode(), (const char*)opcodeEntry->name());
#endif

    // iterate over the payloads in the opcode entry, and dispatch matches
    EQPayloadListIterator pit(*opcodeEntry);
    bool found = false;
    while ((payload = pit.current()) != 0)
    {
      // see if this packet matches
      if (payload->match(data, len, m_dir))
      {
	found = true;
	unknown = false; // 

#ifdef PACKET_INFO_DIAG
	seqDebug(
		"\tmatched payload, find dispatcher in dict (%d/%d)",
		m_dispatchers.count(), m_dispatchers.size());
#endif

	// find the dispather for the payload
	dispatch = m_dispatchers.find((void*)payload);
	
	// if found, dispatch
	if (dispatch)
	{
#ifdef PACKET_INFO_DIAG
	  seqDebug("\tactivating signal...");
#endif
	  dispatch->activate(data, len, m_dir);
	}
      }

      // go to next possible payload
      ++pit;
    }

 #ifdef PACKET_PAYLOAD_SIZE_DIAG
    if (!found && !opcodeEntry->isEmpty())
    {
      QString tempStr;
      tempStr.sprintf("%s  (%#04x) (dataLen: %u) doesn't match:",
		      (const char*)opcodeEntry->name(), opcodeEntry->opcode(), 
		      len);
      
      for (payload = pit.toFirst(); 
	   payload != 0; 
	   payload = ++pit)
      {
	if (payload->dir() & m_dir)
        {
	  if (payload->sizeCheckType() == SZC_Match)
	    tempStr += QString(" sizeof(%1):%2")
	      .arg(payload->typeName()).arg(payload->typeSize());
	  else if (payload->sizeCheckType() == SZC_Modulus)
	    tempStr += QString(" modulus of sizeof(%1):%2")
	      .arg(payload->typeName()).arg(payload->typeSize());
	}
      }      

      seqWarn(tempStr);
    }
#endif // PACKET_PAYLOAD_SIZE_DIAG
  }
#ifdef PACKET_INFO_DIAG
  else
  {
    seqWarn("dispatchPacket(): buffer size %d opcode %04x stream %s (%d) not in opcodeDB",
       len, opCode, EQStreamStr[m_streamid], m_streamid);
  }
#endif

  emit decodedPacket(data, len, m_dir, opCode, opcodeEntry, unknown);
}

////////////////////////////////////////////////////
// handle a new packet on the stream
void EQPacketStream::handlePacket(EQUDPIPPacketFormat& packet)
{
  emit numPacket(++m_packetCount, (int)m_streamid);

  // Packet is ours now. Logging needs to know this later on.
  packet.setSessionKey(getSessionKey());

  // Only accept packets if we've been initialized unless they are
  // initialization packets!
  if (packet.getNetOpCode() != OP_SessionRequest &&
      packet.getNetOpCode() != OP_SessionResponse &&
      ! m_sessionKey)
  {
#if (defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)) || (defined(PACKET_SESSION_DIAG) && PACKET_SESSION_DIAG > 1)
    seqDebug("discarding packet %s:%d ==>%s:%d netopcode=%04x size=%d. Session not initialized. Need to zone to start picking up packets. Session tracking %s.",
      (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
      (const char*)packet.getIPv4DestA(), packet.getDestPort(),
      packet.getNetOpCode(), packet.payloadLength(),
        (m_session_tracking_enabled == 2 ? "locked on" : 
          (m_session_tracking_enabled == 1 ? "enabled" : "disabled")));
#endif
    return;
  }

  // Only accept packets that correspond to our latched client port, if
  // it is set. This helps filter out multiple sessions on the same physical
  // host when two eq clients zone at the same time. The first one will win.
  if (m_sessionClientPort != 0 &&
      ((dir() == DIR_Server && m_sessionClientPort != packet.getDestPort()) ||
       (dir() == DIR_Client && m_sessionClientPort != packet.getSourcePort())))
  {
#if (defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)) || (defined(PACKET_SESSION_DIAG) && PACKET_SESSION_DIAG > 1)
    seqDebug("discarding packet %s:%d ==>%s:%d netopcode=%04x size=%d. Multiple sessions on the same box? Ignoring all but one of them. Latched client port %d. Session tracking %s.",
      (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
      (const char*)packet.getIPv4DestA(), packet.getDestPort(),
      packet.getNetOpCode(), packet.payloadLength(),
      m_sessionClientPort,
        (m_session_tracking_enabled == 2 ? "locked on" :
          (m_session_tracking_enabled == 1 ? "enabled" : "disabled")));
#endif
    return;
  }

  // Only accept packets that pass the EQ protocol-level CRC check. This helps
  // weed out non-EQ packets that we might see.
#ifdef APPLY_CRC_CHECK
  if (packet.hasCRC())
  {
    uint16_t calcedCRC = calculateCRC(packet);

    if (calcedCRC != packet.crc())
    {
      seqWarn("INVALID PACKET: Bad CRC [%s:%d -> %s:%d] netOp %04x seq %04x len %d crc (%04x != %04x)",
         (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
         (const char*)packet.getIPv4DestA(), packet.getDestPort(),
         packet.getNetOpCode(), packet.arqSeq(), packet.getUDPPayloadLength(),
         packet.crc(), calcedCRC);
      return;
    }
  }
#endif /* APPLY_CRC_CHECK */

  // Decode the packet first
  if (! packet.decode(m_maxLength))
  {
    seqWarn("Packet decode failed for stream %s (%d), op %04x, flags %02x packet dropped.",
      EQStreamStr[m_streamid], m_streamid, packet.getNetOpCode(),
      packet.getFlags());
    return;
  }
#ifdef PACKET_DECODE_DIAG
  else if (packet.hasFlags())
  {
    seqDebug("Successful decode for stream %s (%d), op %04x, flags %02x.",
        EQStreamStr[m_streamid], m_streamid, packet.getNetOpCode(),
        packet.getFlags());
  }
#endif

  // Raw packet
  emit rawPacket(packet.rawPayload(), packet.rawPayloadLength(), m_dir, 
    packet.getNetOpCode());

  processPacket(packet, false); // false = isn't subpacket

  // if the cache isn't empty, then process it.
  if (!m_cache.empty()) 
    processCache();
}

/////////////////////////////////////////////////////
// Handle a protocol level packet. This could be either a top-level
// EQUDPIPPacket or a subpacket that is just an EQProtocolPacket. Either way
// we use net opcodes here.
void EQPacketStream::processPacket(EQProtocolPacket& packet, bool isSubpacket)
{
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
  seqDebug("-->EQPacketStream::processPacket, subpacket=%s on stream %s (%d)",
    (isSubpacket ? "true" : "false"), EQStreamStr[m_streamid], m_streamid);
#endif

  if (IS_APP_OPCODE(packet.getNetOpCode()))
  {
    // This is an app-opcode directly on the wire with no wrapping protocol
    // information. Weird, but whatever gets the stream read, right?
	dispatchPacket(packet.payload(), packet.payloadLength(), 
      packet.getNetOpCode(), m_opcodeDB.find(packet.getNetOpCode()));
    return;
  }

  // Process the net opcode
  switch (packet.getNetOpCode())
  {
    case OP_Combined:
    {
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
      seqDebug("EQPacket: found combined packet (net op: %04x, size %d) on stream %s (%d). Unrolling.", 
        packet.getNetOpCode(), packet.payloadLength(), 
        EQStreamStr[m_streamid], m_streamid);
#endif

      // Rolled up multiple packets inside this packet. Need to unroll them
      // and process them individually. subpacket starts after the net opcode.
      uint8_t* subpacket = packet.payload();

      while (subpacket < packet.payload() + packet.payloadLength())
      {
        // Length specified first on the wire.
        uint8_t subpacketLength = subpacket[0];

        // Move past the length
        subpacket++;

        // OpCode (in net order)
        uint16_t subOpCode = *(uint16_t*)subpacket;
        
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
        seqDebug("EQPacket: unrolling length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
          subpacketLength, EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif
        
        // Opcode is next. Net opcode or app opcode?
        if (subOpCode == 0)
        {
          // App opcode < 0x00ff. Skip the first byte and dispatch the app
          // opcode appropriately
          subpacket++;

          subOpCode = *(uint16_t*)subpacket;

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
          seqDebug("EQPacket: processing unrolled special app opcode, length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
            subpacketLength-3, EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif

          // App opcode. Dispatch it, skipping opcode.
          dispatchPacket(&subpacket[2], subpacketLength-2, 
            subOpCode, m_opcodeDB.find(subOpCode));

        }
        else if (IS_NET_OPCODE(subOpCode))
        {
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
          seqDebug("EQPacket: processing unrolled net opcode, length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
            subpacketLength, EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif

          // Net opcode. false = copy. true = subpacket
          EQProtocolPacket spacket(subpacket, subpacketLength, false, true);

          processPacket(spacket, true);
        }
        else
        {
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
        seqDebug("EQPacket: processing unrolled app opcode, length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
          subpacketLength-2, EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif

          // App opcode. Dispatch it, skipping opcode.
          dispatchPacket(&subpacket[2], subpacketLength-2, 
            subOpCode, m_opcodeDB.find(subOpCode));
        }
        subpacket += subpacketLength;
      }
    }
    break;
    case OP_AppCombined:
    {
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
      seqDebug("EQPacket: found appcombined packet (net op: %04x, size %d) on stream %s (%d). Unrolling.", 
        packet.getNetOpCode(), packet.payloadLength(), 
        EQStreamStr[m_streamid], m_streamid);
#endif

      // Multiple app op codes in the same packet. Need to unroll and dispatch
      // them.
      uint8_t* subpacket = packet.payload();

      while (subpacket < packet.payload() + packet.payloadLength())
      {
        // Length specified first on the wire.
        uint8_t subpacketLength = subpacket[0];

        // Move past the length
        subpacket++;

        if (subpacketLength != 0xff)
        {
          // Dispatch app op code using given packet length. Net order!
          uint16_t subOpCode = *(uint16_t*)(subpacket);

          // Handle 3 byte opcodes properly
          if (subOpCode == 0)
          {
            // 3 byte opcode. Drop the first byte, opcode is byte 2 and 3
            subpacket++;
            subpacketLength--;
            subOpCode = *(uint16_t*)(subpacket);
          }

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
        seqDebug("EQPacket: unrolling length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
          subpacketLength, EQStreamStr[m_streamid], m_streamid, subOpCode);
        seqDebug("EQPacket: processing unrolled app opcode, length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
          subpacketLength-2, EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif

          // Dispatch, skipping op code.
          dispatchPacket(&subpacket[2], subpacketLength-2, 
            subOpCode, m_opcodeDB.find(subOpCode));

          // Move ahead
          subpacket += subpacketLength;
        }
        else
        {
          // If original length is 0xff, it means it is a long one. The length
          // is 2 bytes and next.
          uint16_t longOne = eqntohuint16(subpacket);
 
          // Move past the 2 byte length
          subpacket += 2;

          // OpCode next. Net order for op codes.
          uint16_t subOpCode = *(uint16_t*)subpacket;

          // Handle 3 byte opcodes properly
          if (subOpCode == 0)
          {
            // 3 byte opcode. Drop the first byte, opcode is byte 2 and 3
            subpacket++;
            longOne--;
            subOpCode = *(uint16_t*)(subpacket);
          }
          
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
        seqDebug("EQPacket: unrolling length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
          longOne, EQStreamStr[m_streamid], m_streamid, subOpCode);
        seqDebug("EQPacket: processing unrolled app opcode, length %d bytes from combined packet on stream %s (%d). Opcode %04x", 
          longOne-2, EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif

          // Dispatch, skipping op code.
          dispatchPacket(&subpacket[2], longOne-2, 
            subOpCode, m_opcodeDB.find(subOpCode));

          // Move ahead
          subpacket += longOne;
        }
      }
    }
    break;
    case OP_Packet:
    {
      // Normal unfragmented sequenced packet.
      uint16_t seq = packet.arqSeq();
      emit seqReceive(seq, (int)m_streamid);

      // Future packet?
      if (seq == m_arqSeqExp)
      {
        // Expected packet.
        m_arqSeqExp++;
        emit seqExpect(m_arqSeqExp, (int)m_streamid);

        // OpCode next. Net order for op codes.
        uint16_t subOpCode = *(uint16_t*)(packet.payload());
       
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
        seqDebug("SEQ: Found next sequence number in data stream %s (%d), incrementing expected seq, %04x (op code %04x, sub opcode %04x)", 
          EQStreamStr[m_streamid], m_streamid, seq, 
          packet.getNetOpCode(), subOpCode);
#endif

        // Opcode is next. Net opcode or app opcode?
        if (subOpCode == 0)
        {
          // App opcode < 0x00ff. Skip the first byte and dispatch the app
          // opcode appropriately
          subOpCode = *(uint16_t*)&packet.payload()[1];

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
          seqDebug("EQPacket: special app opcode extracted for opcode 0000 on stream %s (%d). Opcode %04x", 
            EQStreamStr[m_streamid], m_streamid, subOpCode);
#endif

          // App opcode. Dispatch it, skipping opcode.
          dispatchPacket(&packet.payload()[3], packet.payloadLength()-3, 
            subOpCode, m_opcodeDB.find(subOpCode));

        }
        else if (IS_NET_OPCODE(subOpCode))
        {
          // Net opcode. false = no copy. true = subpacket.
          EQProtocolPacket spacket(packet.payload(), 
            packet.payloadLength(), false, true);

          processPacket(spacket, true);
        }
        else
        {
          // App opcode. Dispatch, skipping opcode.
          dispatchPacket(&packet.payload()[2], packet.payloadLength()-2,
            subOpCode, m_opcodeDB.find(subOpCode));
        }
      }
      else if ((seq > m_arqSeqExp && 
                  seq < (uint32_t(m_arqSeqExp + arqSeqWrapCutoff))) ||
               seq < (int32_t(m_arqSeqExp) - arqSeqWrapCutoff))
      {
        // Yeah, future packet. Push it on the packet cache.
#ifdef PACKET_PROCESS_DIAG
        seqDebug("SEQ: out of order sequence %04x stream %s (%d) expecting %04x, sending to cache, %04d",
          seq, EQStreamStr[m_streamid], m_streamid, 
          m_arqSeqExp, m_cache.size());
#endif
        setCache(seq, packet);
      }
      else
      {
#ifdef PACKET_PROCESS_DIAG
        // Past packet outside the cut off
        seqWarn("SEQ: received sequenced %spacket outside expected window on stream %s (%d) netopcode=%04x size=%d. Expecting seq=%04x got seq=%04x, window size %d, dropping packet as in the past.", 
          (isSubpacket ? "sub" : ""),
          EQStreamStr[m_streamid], m_streamid,
          packet.getNetOpCode(), packet.payloadLength(), 
          m_arqSeqExp, seq, arqSeqWrapCutoff);
#endif
      }
    }
    break;
    case OP_Oversized:
    {
      // Fragmented sequenced data packet.
      uint16_t seq = packet.arqSeq();
      emit seqReceive(seq, (int)m_streamid);

      // Future packet?
      if (seq == m_arqSeqExp)
      {
        // Expected packet.
        m_arqSeqExp++;
        emit seqExpect(m_arqSeqExp, (int)m_streamid);
       
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
        seqDebug("SEQ: Found next sequence number in data stream %s (%d), incrementing expected seq, %04x (op code %04x)", 
          EQStreamStr[m_streamid], m_streamid, seq, packet.getNetOpCode());
#endif

        // Push the fragment on.
        m_fragment.addFragment(packet);

        if (m_fragment.isComplete())
        {
          // OpCode from fragment. In network order.
          uint16_t fragOpCode = *(uint16_t*)(m_fragment.data());

#ifdef PACKET_PROCESS_DIAG
        seqDebug("SEQ: Completed oversized app packet on stream %s with seq %04x, total size %d opcode %04x", 
          EQStreamStr[m_streamid], seq, m_fragment.size()-2, fragOpCode);
#endif

          // dispatch fragment. Skip opcode.
          if (fragOpCode == 0)
          {
            // Special app opcode. Skip first byte and op is byte 2 and 3.
            fragOpCode = *(uint16_t*)(&m_fragment.data()[1]);

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
            seqDebug("EQPacket: special app opcode on completed fragment for opcode 0000 on stream %s (%d). Opcode %04x", 
            EQStreamStr[m_streamid], m_streamid, fragOpCode);
#endif

            dispatchPacket(&m_fragment.data()[3], m_fragment.size()-3,
              fragOpCode, m_opcodeDB.find(fragOpCode)); 
          }
          else
          {
            dispatchPacket(&m_fragment.data()[2], m_fragment.size()-2,
              fragOpCode, m_opcodeDB.find(fragOpCode)); 
          }

          m_fragment.reset();
        }
      }
      else if ((seq > m_arqSeqExp && 
                  seq < (uint32_t(m_arqSeqExp + arqSeqWrapCutoff))) ||
               seq < (int32_t(m_arqSeqExp) - arqSeqWrapCutoff))
      {
        // Yeah, future packet. Push it on the packet cache.
#ifdef PACKET_PROCESS_DIAG
        seqDebug("SEQ: out of order sequence %04x stream %s (%d) expecting %04x, sending to cache, %04d",
          seq, EQStreamStr[m_streamid], m_streamid, 
          m_arqSeqExp, m_cache.size());
#endif
        setCache(seq, packet);
      }
      else
      {
#ifdef PACKET_PROCESS_DIAG
        // Past packet outside the cut off
        seqWarn("SEQ: received sequenced %spacket outside expected window on stream %s (%d) netopcode=%04x size=%d. Expecting seq=%04x got seq=%04x, window size %d, dropping packet as in the past.", 
          (isSubpacket ? "sub" : ""),
          EQStreamStr[m_streamid], m_streamid,
          packet.getNetOpCode(), packet.payloadLength(), 
          m_arqSeqExp, seq, arqSeqWrapCutoff);
#endif
      }
    }
    break;
    case OP_SessionRequest:
    {
      // Session request from client to server.
#if defined(PACKET_PROCESS_DIAG) || defined(PACKET_SESSION_DIAG)
      seqDebug("EQPacket: SessionRequest found, resetting expected seq, stream %s (%d) (session tracking %s)",
	    EQStreamStr[m_streamid], m_streamid,
        (m_session_tracking_enabled == 2 ? "locked on" : 
          (m_session_tracking_enabled == 1 ? "enabled" : "disabled")));
#endif
      
      // Pull off session request information
      SessionRequestStruct* request = (SessionRequestStruct*) packet.payload();

      m_sessionId = eqntohuint32((uint8_t*)&(request->sessionId));
      m_maxLength = eqntohuint32((uint8_t*)&(request->maxLength));

#if defined(PACKET_SESSION_DIAG)
      seqDebug("EQPacket: SessionRequest %s:%u->%s:%u, sessionId %u maxLength %u, awaiting key for stream %s (%d)",
        ((EQUDPIPPacketFormat&) packet).getIPv4SourceA().ascii(),
        ((EQUDPIPPacketFormat&) packet).getSourcePort(),
        ((EQUDPIPPacketFormat&) packet).getIPv4DestA().ascii(),
        ((EQUDPIPPacketFormat&) packet).getDestPort(),
        m_sessionId, m_maxLength, EQStreamStr[m_streamid], m_streamid);
#endif

#if defined(PACKET_SESSION_DIAG) && (PACKET_SESSION_DIAG > 1)
      seqDebug("EQPacket: SessionRequest contents: unknown %u, sessionId %u, maxLength %u",
        eqntohuint32((uint8_t*)&(request->unknown0000)), 
        m_sessionId, m_maxLength);
#endif

#if defined(PACKET_SESSION_DIAG) && (PACKET_SESSION_DIAG > 2)
      seqDebug("EQPacket: Raw SessionRequest: %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x",
        packet.payload()[0], packet.payload()[1], packet.payload()[2], 
        packet.payload()[3], packet.payload()[4], packet.payload()[5], 
        packet.payload()[6], packet.payload()[7], packet.payload()[8], 
        packet.payload()[9], packet.payload()[10], packet.payload()[11]);
#endif

      m_arqSeqExp = 0;
      m_arqSeqFound = true;

      if (m_session_tracking_enabled)
      {
        // Save off client port for the stream so we can match against it
        // later. SessionRequest should always be an outer protocol packet
        // so we can cast it to EQUDPIPPacketFormat to get the ip headers.
        m_sessionClientPort = ((EQUDPIPPacketFormat&) packet).getSourcePort();
      }
    }
    break;
    case OP_SessionResponse:
    {
      // Session response from server
#if defined(PACKET_PROCESS_DIAG) || defined(PACKET_SESSION_DIAG)
      seqDebug("EQPacket: SessionResponse found %s:%u->%s:%u, resetting expected seq, stream %s (%d) (session tracking %s)",
        ((EQUDPIPPacketFormat&) packet).getIPv4SourceA().ascii(),
        ((EQUDPIPPacketFormat&) packet).getSourcePort(),
        ((EQUDPIPPacketFormat&) packet).getIPv4DestA().ascii(),
        ((EQUDPIPPacketFormat&) packet).getDestPort(),
	    EQStreamStr[m_streamid], m_streamid,
        (m_session_tracking_enabled == 2 ? "locked on" : 
          (m_session_tracking_enabled == 1 ? "enabled" : "disabled")));
#endif
      
      // Pull off session response information
      SessionResponseStruct* response = 
        (SessionResponseStruct*) packet.payload();

      m_maxLength = eqntohuint32((uint8_t*)&(response->maxLength));
      m_sessionKey = eqntohuint32((uint8_t*)&(response->key));
      m_sessionId = eqntohuint32((uint8_t*)&(response->sessionId));

#if defined(PACKET_SESSION_DIAG)
      seqDebug("EQPacket: SessionResponse sessionId %u maxLength %u, key is %u for stream %s (%d)",
        m_sessionId, m_maxLength, m_sessionKey, 
        EQStreamStr[m_streamid], m_streamid);
#endif

#if defined(PACKET_SESSION_DIAG) && (PACKET_SESSION_DIAG > 1)
      seqDebug("EQPacket: SessionResponse contents: sessionId %u, key %u, unknown %u, unknown %u, maxLength %u, unknown %u",
        m_sessionId, m_sessionKey, 
        eqntohuint16((uint8_t*)&(response->unknown0008)),
        response->unknown0010, m_maxLength,
        eqntohuint32((uint8_t*) &(response->unknown0015)));
#endif

#if defined(PACKET_SESSION_DIAG) && (PACKET_SESSION_DIAG > 2)
      seqDebug("EQPacket: Raw SessionResponse: %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x",
        packet.payload()[0], packet.payload()[1], packet.payload()[2], 
        packet.payload()[3], packet.payload()[4], packet.payload()[5], 
        packet.payload()[6], packet.payload()[7], packet.payload()[8], 
        packet.payload()[9], packet.payload()[10], packet.payload()[11],
        packet.payload()[12], packet.payload()[13], packet.payload()[14], 
        packet.payload()[15], packet.payload()[16], packet.payload()[17], 
        packet.payload()[18]);
#endif

      // Provide key to corresponding stream from this session/stream
      emit sessionKey(m_sessionId, m_streamid, m_sessionKey);

      m_arqSeqExp = 0;
      m_arqSeqFound = true;

      // Session tracking
      if (m_session_tracking_enabled)
      {
        // Save off client port for the stream so we can match against it
        // later. SessionRequest should always be an outer protocol packet
        // so we can cast it to EQUDPIPPacketFormat to get the ip headers.
        m_sessionClientPort = ((EQUDPIPPacketFormat&) packet).getDestPort();

        // If this is the world server talking to us, reset session tracking if
        // it is on so we unlatch the client in case of getting kicked.
        if (m_streamid == world2client)
        {
          m_session_tracking_enabled = 1;
          emit sessionTrackingChanged(m_session_tracking_enabled);
        }
        // If this is the zone server talking to us, close the latch and lock
        else if (m_streamid == zone2client)
        {
          // SessionResponse should always be an outer protocol packet, so
          // the EQProtocolPacket passed in can be cast back to
          // EQUDPIPPacketFormat, which we need to go to get access to the IP
          // headers!
          m_session_tracking_enabled = 2;
  
          emit lockOnClient(((EQUDPIPPacketFormat&) packet).getSourcePort(), 
            ((EQUDPIPPacketFormat&) packet).getDestPort());
          emit sessionTrackingChanged(m_session_tracking_enabled);
        }
      }
    }
    break;
    case OP_SessionDisconnect:
    {
#if defined(PACKET_PROCESS_DIAG) || defined(PACKET_SESSION_DIAG)
      seqDebug("EQPacket: SessionDisconnect found %s:%u->%s:%u, resetting expected seq, stream %s (%d) (session tracking %s)",
        ((EQUDPIPPacketFormat&) packet).getIPv4SourceA().ascii(),
        ((EQUDPIPPacketFormat&) packet).getSourcePort(),
        ((EQUDPIPPacketFormat&) packet).getIPv4DestA().ascii(),
        ((EQUDPIPPacketFormat&) packet).getDestPort(),
	    EQStreamStr[m_streamid], m_streamid,
        (m_session_tracking_enabled == 2 ? "locked on" : 
          (m_session_tracking_enabled == 1 ? "enabled" : "disabled")));
#endif

#if defined(PACKET_SESSION_DIAG) && (PACKET_SESSION_DIAG > 2)
      seqDebug("EQPacket: Raw SessionDisconnect: %02x%02x %02x%02x %02x%02x %02x%02x",
        packet.payload()[0], packet.payload()[1], packet.payload()[2], 
        packet.payload()[3], packet.payload()[4], packet.payload()[5], 
        packet.payload()[6], packet.payload()[7]);
#endif

      m_arqSeqExp = 0;

      // Clear cache
      resetCache();

      // Signal closing. Unlatch session tracking if it is on.
      if (m_session_tracking_enabled)
      {
        m_session_tracking_enabled = 1;
        emit sessionTrackingChanged(m_session_tracking_enabled);

        m_sessionClientPort = 0;
      }

      emit closing(m_sessionId, m_streamid);
    }
    break;
    case OP_Ack:
    case OP_AckFuture:
    case OP_AckAfterDisconnect:
    {
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
      seqDebug("EQPacket: no-op on ACK for net opcode %04x seq %04x, stream %s (%d)",
	    packet.getNetOpCode(), eqntohuint16(packet.payload()), 
        EQStreamStr[m_streamid], m_streamid);
#endif
    }
    break;
    case OP_KeepAlive:
    case OP_SessionStatRequest:
    case OP_SessionStatResponse:
    {
#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 2)
      seqDebug("EQPacket: no-op on stats for net opcode %04x, stream %s (%d)",
	    packet.getNetOpCode(), EQStreamStr[m_streamid], m_streamid);
#endif
    }
    break;
    default :
    {
      seqWarn("EQPacket: Unhandled net opcode %04x, stream %s, size %d",
        packet.getNetOpCode(), EQStreamStr[m_streamid], packet.payloadLength());
    }
  }
}

/////////////////////////////////////////////////
// Process a session key change
void EQPacketStream::receiveSessionKey(uint32_t sessionId, 
  EQStreamID streamid, uint32_t sessionKey)
{
  if (streamid != m_streamid && m_sessionId == sessionId)
  {
    // Key is for us
    m_sessionKey = sessionKey;

#ifdef PACKET_SESSION_DIAG
    seqDebug("EQPacket: Received key %u for session %u on stream %s (%d) from stream %s (%d)",
      m_sessionKey, m_sessionId, EQStreamStr[m_streamid], m_streamid,
      EQStreamStr[streamid], streamid);
#endif
  }
}

///////////////////////////////////////////////////////////////
// Process a session disconnect if it is for us
void EQPacketStream::close(uint32_t sessionId, EQStreamID streamId,
  uint8_t sessionTracking)
{
  if (sessionId == m_sessionId)
  {
     // Close is for us
     reset();
     setSessionTracking(sessionTracking);

#ifdef PACKET_SESSION_DIAG
     seqInfo("EQPacket: SessionDisconnected received on stream %s (%d). Closing session %u on stream %s (%d).",
       EQStreamStr[streamId], streamId, sessionId,
       EQStreamStr[m_streamid], m_streamid);
#endif
  }
}

///////////////////////////////////////////////////////////////
// Calculate the CRC on the given packet using this stream's key
uint16_t EQPacketStream::calculateCRC(EQProtocolPacket& packet)
{
  // CRC is at the end of the raw payload, 2 bytes.
  return ::calcCRC16(packet.rawPacket(), packet.rawPacketLength()-2, 
    m_sessionKey);
}

#include "packetstream.moc"
