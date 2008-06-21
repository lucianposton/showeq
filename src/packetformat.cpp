/*
 * packetformat.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2004 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2004 Zaphod (dohpaz@users.sourceforge.net). 
 */

/* Implementation of packet format classes class */

#include "packetformat.h"
#include "diagnosticmessages.h"

#include <zlib.h>

//----------------------------------------------------------------------
// EQProtocolPacket class methods
EQProtocolPacket::EQProtocolPacket(EQProtocolPacket& packet, bool copy)
  : m_ownCopy(copy)
{
  // Take the easy stuff first.
  m_length = packet.m_length;
  m_netOp = packet.m_netOp;
  m_flags = packet.m_flags;
  m_payloadLength = packet.m_payloadLength;
  m_rawPayloadLength = packet.m_rawPayloadLength;
  m_arqSeq = packet.m_arqSeq;
  m_subpacket = packet.m_subpacket;
  m_bDecoded = packet.m_bDecoded;

  if (!copy)
  {
    // just copy all their values
    m_packet = packet.m_packet;
    m_payload = packet.m_payload;
    m_rawPayload = packet.m_rawPayload;
    m_bAllocedPayload = false;
  }
  else
  {
    // Need to copy copy their values for buffers. m_packet first.
    m_packet = new uint8_t[m_length];
    memcpy(m_packet, packet.m_packet, m_length);

    // Still have m_payload, m_rawPayload to do. Only if this packet
    // alloced itself otherwise these point into m_packet.
    if (packet.m_bAllocedPayload)
    {
      // Have packet owned payload to copy over.
      m_rawPayload = new uint8_t[m_rawPayloadLength];
      memcpy(m_rawPayload, packet.m_rawPayload, m_rawPayloadLength);
      m_bAllocedPayload = true;

      m_payload = m_rawPayload + (packet.m_payload - packet.m_rawPayload);
    }
    else
    {
      // No packet owned payload and init copied the raw payload. Just set
      // pointers into m_packet
      m_rawPayload = m_packet + (packet.m_rawPayload - packet.m_packet);
      m_payload = m_packet + (packet.m_payload - packet.m_packet);
      m_bAllocedPayload = false;
    }
  }
}

EQProtocolPacket::~EQProtocolPacket()
{
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;

  if (m_bAllocedPayload)
  {
    delete[] m_rawPayload;
  }
}

EQProtocolPacket& EQProtocolPacket::operator=(const EQProtocolPacket& packet)
{
  // if this was a deep copy, delete the existing data
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;
  if (m_bAllocedPayload)
  {
    delete[] m_rawPayload;
    m_bAllocedPayload = false;
  }

  init(packet.m_packet, packet.m_length, m_ownCopy, packet.m_subpacket);

  return *this;
}

void EQProtocolPacket::init(uint8_t* packet, uint32_t length, 
  bool copy, bool subpacket)
{
  m_subpacket = subpacket;

  if (!copy)
  {
    // the data is someone elses memory
    m_ownCopy = false;
    
    // just copy the pointer
    m_packet = packet;

    // save the length
    m_length = length;
  }
  else
  {
    // this is our own copy
    m_ownCopy = true;

    // allocate memory for the copy
    m_packet = new uint8_t[length];

    // copy the data
    memcpy((void*)m_packet, (void*)packet, length);

    // save the length
    m_length = length;
  }

  // finish initialization
  init();
}

////////////////////////////////////////////////////////////////////////////
// Initialize the packet. After this is done, flags will be correct and the
// payload will be pointing at the payload, unless flags say this is a
// compressed packet all bets are off until you've called decode.
void EQProtocolPacket::init()
{
  // Get the net op code. Leave in network order. Need this to decide things.
  m_netOp = *(uint16_t*)(m_packet);

  // Now line up the payload as best we can. Note that if this packet is 
  // compressed, the opcode could potentially be wrong and the payload
  // not aligned properly. You need to call decode to make sure. But on
  // non-compressed packets this is good enough and you don't need to decode
  if (! hasFlags())
  {
    // No flags. Netopcode, then payload. Easy.
    m_flags = 0;
    m_rawPayload = &m_packet[2];
    m_bAllocedPayload = false;

    // Total - net op - crc
    m_rawPayloadLength = m_length - 2 - (hasCRC() ? 2 : 0);

    // Decoded since no flags
    m_payload = m_rawPayload;
    m_payloadLength = m_rawPayloadLength;
    m_bDecoded = true;
  }
  else
  {
    // Flags in the stream. Placement depends on whether this is an app or net
    // opcode.
    if (IS_APP_OPCODE(m_netOp))
    {
      // opcode is an app opcode. Flags are byte 2 of the packet.
      m_flags = m_packet[1];

      // The opcode is split by the flags. If it is compressed (based on those 
      // above flags) then this is wrong but will fixed by decode
      m_netOp = m_packet[2] << 8 | m_packet[0];
    }
    else
    {
      // Flags at byte #3 after net opcode. m_netOp is correct.
      m_flags = m_packet[2];
    }

    // Either way, let's start the payload at byte 4 for now. Decode may
    // change this. Length is total - netop - flags - crc.
    m_rawPayload = &m_packet[3];
    m_rawPayloadLength = m_length - 2 - 1 - (hasCRC() ? 2 : 0);
    m_bAllocedPayload = false;

    if (! (m_flags & PROTOCOL_FLAG_COMPRESSED))
    {
      // We have the packet here, let's finish the job.
      m_payloadLength = m_rawPayloadLength;
      m_payload = m_rawPayload;

      m_bDecoded = true;
    }
    else
    {
      m_bDecoded = false;
    }
  }

  // Take seq off the top if necessary
  if (hasArqSeq() && m_bDecoded)
  {
    m_arqSeq = eqntohuint16(m_payload);
    m_payload += 2;
    m_payloadLength -= 2;
  }
}

////////////////////////////////////////////////////////////////
// Take a raw wire packet and align the payload, decompressing if necessary
bool EQProtocolPacket::decode(uint32_t maxPayloadLength)
{
  // No double decoding...
  if (m_bDecoded)
  {
    return true;
  }

  // Decoding is only necessary for compressed packets
  if (hasFlags() && getFlags() & PROTOCOL_FLAG_COMPRESSED)
  {
    // Compressed app opcode? If so, net op is half compressed. Align
    // the buffer we need to uncompress.
    if (IS_APP_OPCODE(getNetOpCode()))
    {
      // Total - 1/2 netop - flags - crc
      m_payloadLength = m_length - 1 - 1 - (hasCRC() ? 2 : 0);
      m_payload = &m_packet[2];
    }
    else
    {
      // Total - netop - flags - crc
      m_payloadLength = m_length - 2 - 1 - (hasCRC() ? 2 : 0);
      m_payload = &m_packet[3];
    }

    // Compressed. Need to inflate. RawPayload is going to be our decompress
    // buffer and needs to be managed properly.
    m_rawPayload = new uint8_t[maxPayloadLength];
    m_rawPayloadLength = maxPayloadLength; // alloced size for zlib
    m_bAllocedPayload = true;

    // Decompress
    uint32_t retval = uncompress(m_rawPayload, (uLongf*)&m_rawPayloadLength,
      m_payload, m_payloadLength);

    if (retval != 0)
    {
      seqWarn("Uncompress failed for packet op %04x, flags %02x. Error was %s (%d)",
        getNetOpCode(), getFlags(), zError(retval), retval);

      delete[] m_rawPayload;
      m_bAllocedPayload = false;
      return false;
    }

    // Align buffer pointers in the decompressed buffer and reconstitue the 
    // opcode if it's a split compressed app opcode
    if (IS_APP_OPCODE(getNetOpCode()))
    {
      // Actual net op is first raw byte + first uncompressed byte
      m_netOp = m_rawPayload[0] << 8 | m_packet[0];

      // payload is the actual payload, skipping the 2nd byte of the opcode
      m_payload = &m_rawPayload[1];
      m_payloadLength = m_rawPayloadLength - 1;
    }
    else
    {
      // Net op is correct. Payload is correct.
      m_payload = m_rawPayload;
      m_payloadLength = m_rawPayloadLength;
    }

    // Take seq off the top if necessary
    if (hasArqSeq())
    {
      m_arqSeq = eqntohuint16(m_payload);
      m_payload += 2;
      m_payloadLength -= 2;
    }

    m_bDecoded = true;
  }

  return true;
}

//----------------------------------------------------------------------
// EQUDPIPPacketFormat class methods
EQUDPIPPacketFormat::EQUDPIPPacketFormat(uint8_t* data, 
					 uint32_t length, 
					 bool copy)
{
  uint8_t* ipdata;
  if (copy)
  {
    // allocate our own copy
    ipdata = new uint8_t[length];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)data, length);
  }
  else // just use the data that was passed in
    ipdata = data;

  // note whether or not this object ownw the memory
  m_ownCopy = copy;

  // No session yet
  m_sessionKey = 0;

  // initialize the object
  init(ipdata);
}

EQUDPIPPacketFormat::EQUDPIPPacketFormat(EQUDPIPPacketFormat& packet,
					 bool copy)
{
  // note whether or not this object ownw the memory
  m_ownCopy = copy;
  m_sessionKey = packet.getSessionKey();

  if (copy)
  {
    // allocate our own copy
    uint8_t* ipdata = new uint8_t[packet.m_dataLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_dataLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_dataLength = packet.m_dataLength;
    m_ip = packet.m_ip;
    m_udp = packet.m_udp;
  }
}

EQUDPIPPacketFormat::~EQUDPIPPacketFormat()
{
  if (m_ownCopy && (m_ip != NULL))
    delete [] (uint8_t*)m_ip;
}

EQUDPIPPacketFormat& EQUDPIPPacketFormat::operator=(const EQUDPIPPacketFormat& packet)
{
  if (m_ownCopy && (m_ip != NULL))
    delete [] (uint8_t*)m_ip;

  if (m_ownCopy)
  {
    // allocate our own copy
    uint8_t* ipdata = new uint8_t[packet.m_dataLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_dataLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_dataLength = packet.m_dataLength;
    m_ip = packet.m_ip;
    m_udp = packet.m_udp;
  }

  return *this;
}

void EQUDPIPPacketFormat::init(uint8_t* data)
{
  uint32_t ipHeaderLength, length;

  // we start at the IP header
  m_ip = (struct ip*)data;

  // retrieve the total length from the header
  m_dataLength = ntohs (m_ip->ip_len);

  // use this length to caclulate the rest
  length = m_dataLength;

  // skip past the IP header
  ipHeaderLength  = m_ip->ip_hl * 4;
  length  -= ipHeaderLength;
  data += ipHeaderLength;

  // get the UDP header
  m_udp   = (struct udphdr *) data;

  // skip over UDP header
  length  -= sizeof  (struct udphdr);
  data += (sizeof (struct udphdr));
  m_rawpayload = data;
  m_rawpayloadSize = length;

  // initialize underlying EQProtocolPacket with the UDP payload
  EQProtocolPacket::init(data, length, false);
}
