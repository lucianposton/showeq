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
#include "decode.h"
#include "packetstream.h"
#include "packetformat.h"
#include "packetinfo.h"
#include "diagnosticmessages.h"

#include <zlib.h>

#include <stdio.h>

//----------------------------------------------------------------------
// Macros

// The following defines are used to diagnose packet handling behavior
// this define is used to diagnose packet processing (in dispatchPacket mostly)
//#define PACKET_PROCESS_DIAG 3 // verbosity level 0-n

// this define is used to diagnose cache handling (in dispatchPacket mostly)
//#define PACKET_CACHE_DIAG 3 // verbosity level (0-n)

// this define is used to diagnose decompression/decoding 
//#define PACKET_DECODE_DIAG

// this define is used to debug packet info 
//#define PACKET_INFO_DIAG

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

#ifdef PACKET_CACHE_DIAG
  printf("EQPacketStream: Maximum Packet Cache Used on stream %s: %d", 
	 EQStreamStr[m_streamid], maxServerCacheCount);
#endif
 
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
  stopDecode();
}

////////////////////////////////////////////////////
// cache reset
void EQPacketStream::resetCache()
{
    // first delete all the entries
    EQPacketMap::iterator it = m_cache.begin();
    EQPacketFormat* pf;
#ifdef PACKET_CACHE_DIAG
    seqDebug("Clearing Cache[%s]: Count: %d", EQStreamStr[m_streamid], m_cache.size());
#endif 
    while (it != m_cache.end())
    {
      pf = it->second;
      delete pf;
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
void EQPacketStream::setCache(uint16_t serverArqSeq, EQPacketFormat& packet)
{
   // check if the entry already exists in the cache
   EQPacketMap::iterator it = m_cache.find(serverArqSeq);

   if (it == m_cache.end())
   {
   // entry doesn't exist, so insert an entry into the cache

#ifdef PACKET_PROCESS_DIAG
      seqDebug("SEQ: Insert arq (%04x) stream %d into cache", serverArqSeq, m_streamid);
#endif

      m_cache.insert(EQPacketMap::value_type(serverArqSeq, new EQPacketFormat(packet, true)));
      emit cacheSize(m_cache.size(), (int)m_streamid);
   }
   else
   {
     // replacing an existing entry, make sure the new data is valid
     if (packet.isValid())
     {

#ifdef PACKET_PROCESS_DIAG
        seqDebug("SEQ: Update arq (%04x) stream %d in cache", serverArqSeq, m_streamid);
#endif

        *it->second = packet;
     }

#ifdef PACKET_PROCESS_DIAG
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
  EQPacketFormat* pf;

  // check if the cache has grown large enough that we should give up
  // on seeing the current serverArqSeqExp
  // this should really only kick in for people with pathetic
  // network cards that missed the packet.
  if (m_cache.size() >= m_arqSeqGiveUp)
  {
    // ok, if the expected server arq sequence isn't here yet, give up
    
    // attempt to find the current expencted arq seq
    it = m_cache.find(m_arqSeqExp);
    
    // keep trying to find a new serverArqSeqExp if we haven't found a good
    // one yet...
    while(it == m_cache.end())
    {
      seqDebug("SEQ: Giving up on finding arq %04x in stream %s cache, skipping!",
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
    pf = it->second;
    
    // make sure this is the expected packet
    // (we might have incremented to the one after the one returned
    // by find above).
    if (pf->arq() != m_arqSeqExp)
      break;
    
#ifdef PACKET_CACHE_DIAG
    seqDebug("SEQ: found next arq %04x in stream %s cache, cache count %04d",
	   m_arqSeqExp, EQStreamStr[m_streamid], m_cache.size());
#endif
    
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
    // validate the packet against a memory corruption
    if (!pf->isValid())
    {
      // Something's screwed up
      seqDebug("SEQ: INVALID PACKET: Bad CRC32 in packet in stream %s cache with arq %04x!",
	     EQStreamStr[m_streamid], pf->arq());
    }
#endif
    
    
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
    seqDebug("SEQ: Found next arq in stream %s cache, incrementing arq seq, %04x", 
	   EQStreamStr[m_streamid], pf->arq());
#endif
    
    // Duplicate ARQ processing functionality from dispatchPacket,
    // should prolly be its own function processARQ() or some such beast
    
    if (!pf->isASQ() && !pf->isFragment() && !pf->isClosingHi())
    {
      // seems to be a sort of ping from client to server, has ARQ
      // but no ASQ, Flags look like 0x0201 (network byte order)
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
      seqDebug("SEQ: ARQ without ASQ from stream %s arq 0x%04x",
	     EQStreamStr[m_streamid], pf->arq());
#endif
    } // since the servers do not care about client closing sequences, we won't either
    else if (pf->isClosingHi() && pf->isClosingLo() && 
	     (m_streamid == zone2client))
      {
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
	seqDebug("EQPacketStream: Closing HI & LO, stream %s arq %04x", 
	       EQStreamStr[m_streamid], pf->arq());  
#endif
	if (m_session_tracking_enabled)
	  m_session_tracking_enabled = 1; 
	
	emit closing();
	
	break;
      } // if the packet isn't a fragment dispatch normally, otherwise to split
    else if (pf->isFragment())
      processFragment(*pf);
    else
      processPayload(pf->payload(), pf->payloadLength());
    
    eraseIt = it;
    
    // increment the current position iterator
    it++;
    
    // erase the packet from the cache
    m_cache.erase(eraseIt);
    emit cacheSize(m_cache.size(), (int)m_streamid);
    
#ifdef PACKET_CACHE_DIAG
    seqDebug("SEQ: REMOVING arq %04x from stream %s cache, cache count %04d",
	   pf->arq(), EQStreamStr[m_streamid], m_cache.size());
#endif
    // delete the packet
    delete pf;

    // increment the expected arq number
    m_arqSeqExp++;
    emit seqExpect(m_arqSeqExp, (int)m_streamid);
    
    if (m_arqSeqExp == 0)
      it = m_cache.begin();
  }
  
#ifdef PACKET_CACHE_DIAG
  seqDebug("SEQ: FINISHED checking stream %s cache, arq %04x, cache count %04d",
         EQStreamStr[m_streamid], m_arqSeqExp, m_cache.size());
#endif
}

////////////////////////////////////////////////////
// Fragment processing
void EQPacketStream::processFragment(EQPacketFormat& pf)
{
  // add the fragment to the fragment sequence
  m_fragment.addFragment(pf);
  
  // if the sequence is now complete, then time to process it.
  if (m_fragment.isComplete())
  {
    // process the packet
    processPayload(m_fragment.data(), m_fragment.size());
    
    // finished with fragment, so reset the fragment sequencer
    m_fragment.reset();
  }
}

////////////////////////////////////////////////////
// initialize decoder
void EQPacketStream::initDecode()
{
  m_validKey = true;
  m_decodeKey = 0;
}

////////////////////////////////////////////////////
// stop decoding until re-init
void EQPacketStream::stopDecode()
{
  m_validKey = false;
}

////////////////////////////////////////////////////
// decrypt/uncompress packet
uint8_t* EQPacketStream::decodeOpCode(uint8_t *data, size_t *len, 
				      uint16_t& opCode)
{
  bool s_encrypt = opCode & FLAG_CRYPTO;
  bool compressed = opCode & FLAG_COMP;

  if (s_encrypt)
  {
    if (!m_validKey)
      return NULL;
    
#ifdef PACKET_DECODE_DIAG
    seqDebug("decoding 0x%04x with 0x%08llx on stream %s", opCode, m_decodeKey, EQStreamStr[m_streamid]);
#endif
    
    int64_t offset = (m_decodeKey % 5) + 5;
    *((int64_t *)(data+offset)) ^= m_decodeKey;
    m_decodeKey ^= *((int64_t *)(data+offset));
    m_decodeKey += *len;
  }
  
  if (compressed)
  {
    static uint8_t decompressed[200000];
    size_t dcomplen = 199998;
    uint32_t retval;
    
    retval = uncompress(decompressed, (uLongf*)&dcomplen, data, (*len));
    if (retval != 0)
    {
      if (s_encrypt) 
      {
	seqWarn("Lost sync, relog or zone to reset");
	m_validKey = false;
      }
      
      seqWarn("uncompress failed on 0x%04x: %d - %s\nno further attempts will be made until zone on stream %s.", 
	      opCode, retval, zError(retval), EQStreamStr[m_streamid]);
      return NULL;
    }
    
#ifdef PACKET_DECODE_DIAG
    seqDebug ("clean uncompress of 0x%04fx on stream %s: %s", opCode, zError (retval), EQStreamStr[m_streamid]);
#endif 

    opCode &= ~FLAG_COMP;
    if (s_encrypt) 
      opCode &= ~FLAG_CRYPTO;
    data = decompressed;
    *len = dcomplen;
  }

  return data;
}

////////////////////////////////////////////////////
// decode, decrypt, and unroll packet
void EQPacketStream::decodePacket(uint8_t *data, size_t len, uint16_t opCode)
{
  emit rawPacket(data, len, m_dir, opCode);

  if (opCode & FLAG_CRYPTO || opCode & FLAG_COMP) 
  {
    data = decodeOpCode (data, &len, opCode);
    if (data == NULL)
      return;

    emit rawPacket(data, len, m_dir, opCode);
  }
  
  const EQPacketOPCode* opcodeEntry = 0;

  // this works, but could really use a cleanup - mvern
  while (len > 2) 
  {
    if (opCode & FLAG_COMBINED) 
    {
      bool repeatop = false;
      uint8_t *dptr = data;
      size_t left = len;
      int32_t count = 1;
      
      if (opCode & FLAG_IMPLICIT) 
      {
	opCode = opCode & ~FLAG_IMPLICIT;
	repeatop = true;
      }

#ifdef PACKET_DECODE_DIAG
      seqDebug("unrolling on %s: 0x%04x", EQStreamStr[m_streamid], opCode);
#endif
      opCode = opCode & ~FLAG_COMBINED;
      
      while (count > 0) 
      {
	uint16_t size = 0;

	opcodeEntry = m_opcodeDB.find(opCode);

	if (opcodeEntry)
	{
	  size = opcodeEntry->implicitLen();
#ifdef PACKET_DECODE_DIAG // ZBTEMP
	  seqDebug("opcode %04x implicitlen %d",
		  opCode, size);
#endif
	}
#ifdef PACKET_DECODE_DIAG
	else 
	  seqDebug("No opcodeEntry for %04x", opCode);
#endif 
	
	if (size == 0) 
	{ // Not an implicit length opcode
	  if (dptr[0] == 0xff) 
	  { // > 255 length
	    left--;
	    dptr++;
	    size = ntohs(*((uint16_t *)dptr));
	    left -= 2;
	    dptr += 2;
	  } 
	  else 
	  { // single octet length
	    size = dptr[0];
	    left--;
	    dptr++;
	  }
	}
#ifdef PACKET_DECODE_DIAG
	seqDebug ("size on %s: %d", EQStreamStr[m_streamid], size);
#endif
		
	if (size > left) 
	{
	  seqWarn("error on %s: size > left (size=%d, left=%d, opcode=0x%04x)", 
		     EQStreamStr[m_streamid], size, left, opCode);
	  return;
	}
	
#ifdef PACKET_DECODE_DIAG
	seqDebug("sending from %s: 0x%04x, %d", 
	       EQStreamStr[m_streamid], opCode, size);
#endif
	dispatchPacket(dptr, size, opCode, opcodeEntry);
	
	// next
	dptr += size;
	left -= size;
	
	count--;
	if (repeatop) 
	{
	  count = dptr[0];
	  dptr++;
	  left--;
	  repeatop = false;
#ifdef PACKET_DECODE_DIAG
	  seqDebug ("repeating %d times on %s",
		  count, EQStreamStr[m_streamid]);
#endif
	}
      }
	    
      if (left > 2) 
      {
	opCode = *((unsigned short *)dptr);
	dptr += 2;
	left -= 2;

#ifdef PACKET_DECODE_DIAG
	seqDebug("doing leftover from %s: 0x%04x, %d", 
	       EQStreamStr[m_streamid], opCode, left);
#endif
	if (opCode & FLAG_COMBINED)
	{
	  data = dptr;
	  len = left;
	  continue;
	} 
	else 
	{
#ifdef PACKET_DECODE_DIAG
	  seqDebug("sending from %s: 0x%04x, %d", 
		 EQStreamStr[m_streamid], opCode, size);
#endif
	  dispatchPacket(dptr, left, opCode, m_opcodeDB.find(opCode));
	}
      }

      return;
    }
    else
      break;
  }
  
#ifdef PACKET_DECODE_DIAG
  seqDebug ("sending from %s: 0x%04x, %d",
	  EQStreamStr[m_streamid], opCode, size);
#endif
  dispatchPacket(data, len, opCode, m_opcodeDB.find(opCode));
}

////////////////////////////////////////////////////
// process the packets payload, decoding if necessary
void EQPacketStream::processPayload(uint8_t* data, size_t len)
{
  uint16_t opCode = *(uint16_t*)data;
  
  data += 2;
  len -= 2;
  
  if (opCode & FLAG_DECODE)
    decodePacket(data, len, opCode);
  else
  {
    emit rawPacket(data, len, m_dir, opCode);

    dispatchPacket(data, len, opCode, m_opcodeDB.find(opCode));
  }
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

  emit decodedPacket(data, len, m_dir, opCode, opcodeEntry, unknown);
}

////////////////////////////////////////////////////
// handle a new packet on the stream
void EQPacketStream::handlePacket(EQUDPIPPacketFormat& packet)
{
  emit numPacket(++m_packetCount, (int)m_streamid);

  if (!packet.isARQ()) // process packets that don't have an arq sequence
  { 
    // we only handle packets with opcodes
    if (packet.payloadLength() < 2)
      return;
    
    // process the packets payload immediately
    processPayload(packet.payload(), packet.payloadLength());
  }
  else if (packet.isARQ()) // process ARQ sequences
  {
     uint16_t arqSeq = packet.arq();
     emit seqReceive(arqSeq, (int)m_streamid);
      
     /* this conditions should only be met once per zone/world, New Sequence */
     if (packet.isSEQStart() && !packet.isClosingLo() && 
	 (m_session_tracking_enabled < 2))
     {
#ifdef PACKET_PROCESS_DIAG
       seqDebug("EQPacket: SEQStart found, setting arq seq, %04x  stream %s",
	      arqSeq, EQStreamStr[m_streamid]);
#endif

       initDecode();
       
       // hey, a SEQStart, use it's packet to set ARQ
       m_arqSeqExp = arqSeq;
       m_arqSeqFound = true;
       emit seqExpect(m_arqSeqExp, (int)m_streamid);
       
       if ((m_streamid == zone2client) && m_session_tracking_enabled)
       {
	 m_session_tracking_enabled = 2;
	 
	 emit lockOnClient(packet.getSourcePort(),
			   packet.getDestPort());
	 
	 // notify that the client port has been latched
	 emit sessionTrackingChanged(m_session_tracking_enabled);
       }
     }
     else if (!m_arqSeqFound && m_session_tracking_enabled == 0 &&
              !packet.isClosingHi() && !packet.isClosingLo() && 
	      (m_streamid == zone2client))
     {
#ifdef PACKET_PROCESS_DIAG
       seqDebug("SEQ: new sequence found, setting arq seq, %04x  stream %s",
	      arqSeq, EQStreamStr[m_streamid]);
#endif
       m_arqSeqExp = arqSeq;
       m_arqSeqFound = true;
       emit seqExpect(m_arqSeqExp, (int)m_streamid);
     }
     // is this the currently expected sequence, if so, do something with it.
     if (m_arqSeqExp == arqSeq)
     {
       m_arqSeqExp = arqSeq + 1;
       emit seqExpect(m_arqSeqExp, (int)m_streamid);
       
#ifdef PACKET_PROCESS_DIAG
       seqDebug("SEQ: Found next arq in data stream %s, incrementing arq seq, %04x", 
	      EQStreamStr[m_streamid], arqSeq);
#endif

       if (!packet.isASQ() && !packet.isFragment() && !packet.isClosingHi())
       {
	 // seems to be a sort of ping from client to server, has ARQ
	 // but no ASQ, Flags look like 0x0201 (network byte order)
#ifdef PACKET_PROCESS_DIAG
	 seqDebug("SEQ: ARQ without ASQ from stream %s arq 0x%04x",
		EQStreamStr[m_streamid], arqSeq);
#endif
       }
       // since the servers do not care about client closing sequences, we won't either
       // Hey clients have rights too, or not! 
       else if (packet.isClosingHi() && packet.isClosingLo() && 
		(m_streamid == zone2client))
       {
	 if (m_session_tracking_enabled)
	   m_session_tracking_enabled = 1; 
	 
	 emit closing();
	 
	 return;
       } // if the packet is a fragment do appropriate processing
       else if (packet.isFragment())
	 processFragment(packet);
       else if (packet.payloadLength() >= 2) // has to have an opcode
	 processPayload(packet.payload(), packet.payloadLength());
     } // it's a packet from the future, add it to the cache
     else if ( ( (arqSeq > m_arqSeqExp) && 
                 (arqSeq < (uint32_t(m_arqSeqExp + arqSeqWrapCutoff))) ) || 
               (arqSeq < (int32_t(m_arqSeqExp) - arqSeqWrapCutoff)) ) 
     {
#ifdef PACKET_PROCESS_DIAG
       seqDebug("SEQ: out of order arq %04x stream %s, sending to cache, %04d",
	      arqSeq, EQStreamStr[m_streamid], m_cache.size());
#endif
       
       setCache(arqSeq, packet);
     }
     
     // if the cache isn't empty, then check for the expected ARQ sequence
     if (!m_cache.empty()) 
       processCache();
  } /* end ARQ processing */
}

#include "packetstream.moc"
