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

/**
 * All app packets have flags/crc. Some net packets have flags/crc.
 * Subpackets (whether app or net) have no flags/crc. Subpackets not shown below.
 *
 *                  net packet without flags/crc
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     0x00      |      netOp      |           payload           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * after init():
 * * m_packetData     is at byte #1
 * * m_uncompressData is at byte #3
 * * m_payload        is at byte #3
 *
 *                    net packet with flags/crc
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     0x00      |      netOp      |     flags     |  payload... |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | ... payload (cont.) ...         |              crc            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * after init():
 * * m_packetData     is at byte #1
 * * m_uncompressData is at byte #4
 * * m_payload        is at byte #4
 *
 * If flags require decompression, payload is compressed.
 * After decompression:
 * * m_packetData       is at byte #1
 * * m_uncompressedData is at new uncompress(from byte #4)
 * * m_payload          is at m_uncompressedData
 *
 *
 *    net packet with flags/crc with arq seq (OP_Packet & OP_Oversized)
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     0x00      |      netOp      |     flags     |  arqSeq...  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   ...arqSeq   |                    payload...                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | ... payload (cont.) ...         |              crc            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * after init():
 * * m_packetData     is at byte #1
 * * m_uncompressData is at byte #4
 * * m_payload        is at byte #6
 *
 * If flags require decompression, arqSeq and payload are compressed.
 * After decompression:
 * * m_packetData       is at byte #1
 * * m_uncompressedData is at new uncompress(from byte #4)
 * * m_payload          is at m_uncompressedData + 2 bytes
 *
 *
 *                           app packet
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  appOp (low)  |      flags      | appOp (high)  |  payload... |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | ... payload (cont.) ...         |              crc            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * after init():
 * * m_packetData     is at byte #1
 * * m_uncompressData is at byte #4
 * * m_payload        is at byte #4
 *
 * If flags require decompression, appOp (high) and payload are compressed.
 * After decompression:
 * * m_packetData       is at byte #1
 * * m_uncompressedData is at new uncompress(from byte #3)
 * * m_payload          is at m_uncompressedData + 1 byte
 */

//#define DEBUG_PACKETFORMAT 3

//----------------------------------------------------------------------
// EQProtocolPacket class methods
EQProtocolPacket::EQProtocolPacket(EQProtocolPacket& packet, bool copy)
  : m_ownCopy(copy)
{
  // Take the easy stuff first.
  m_packetDataLength = packet.m_packetDataLength;
  m_netOp = packet.m_netOp;
  m_flags = packet.m_flags;
  m_payloadLength = packet.m_payloadLength;
  m_uncompressedDataLength = packet.m_uncompressedDataLength;
  m_arqSeq = packet.m_arqSeq;
  m_subpacket = packet.m_subpacket;
  m_bDecoded = packet.m_bDecoded;

  if (!copy)
  {
    // just copy all their values
    m_packetData = packet.m_packetData;
    m_payload = packet.m_payload;
    m_uncompressedData = packet.m_uncompressedData;
    m_bAllocedPayload = false;
  }
  else
  {
    // Need to copy copy their values for buffers. m_packetData first.
    m_packetData = new uint8_t[m_packetDataLength];
    memcpy(m_packetData, packet.m_packetData, m_packetDataLength);

    // Still have m_payload, m_uncompressedData to do. Only if this packet
    // alloced itself otherwise these point into m_packetData.
    if (packet.m_bAllocedPayload)
    {
      // Have packet owned payload to copy over.
      m_uncompressedData = new uint8_t[m_uncompressedDataLength];
      memcpy(m_uncompressedData, packet.m_uncompressedData, m_uncompressedDataLength);
      m_bAllocedPayload = true;

      m_payload = m_uncompressedData + (packet.m_payload - packet.m_uncompressedData);
    }
    else
    {
      // No packet owned payload and init copied the raw payload. Just set
      // pointers into m_packetData
      m_uncompressedData = m_packetData + (packet.m_uncompressedData - packet.m_packetData);
      m_payload = m_packetData + (packet.m_payload - packet.m_packetData);
      m_bAllocedPayload = false;
    }
  }
}

EQProtocolPacket::~EQProtocolPacket()
{
  if (m_ownCopy)
    delete [] (uint8_t*)m_packetData;

  if (m_bAllocedPayload)
  {
    delete[] m_uncompressedData;
  }
}

EQProtocolPacket& EQProtocolPacket::operator=(const EQProtocolPacket& packet)
{
  // if this was a deep copy, delete the existing data
  if (m_ownCopy)
    delete [] (uint8_t*)m_packetData;
  if (m_bAllocedPayload)
  {
    delete[] m_uncompressedData;
    m_bAllocedPayload = false;
  }

  init(packet.m_packetData, packet.m_packetDataLength, m_ownCopy, packet.m_subpacket);

  return *this;
}

void EQProtocolPacket::init(uint8_t* packet, uint16_t length, 
  bool copy, bool subpacket)
{
  m_subpacket = subpacket;

  if (!copy)
  {
    // the data is someone elses memory
    m_ownCopy = false;
    
    // just copy the pointer
    m_packetData = packet;

    // save the length
    m_packetDataLength = length;
  }
  else
  {
    // this is our own copy
    m_ownCopy = true;

    // allocate memory for the copy
    m_packetData = new uint8_t[length];

    // copy the data
    memcpy((void*)m_packetData, (void*)packet, length);

    // save the length
    m_packetDataLength = length;
  }

#ifdef DEBUG_PACKETFORMAT
    seqDebug("EQProtocolPacket::init() m_packetDataLength=%d subpacket=%d",
            length, subpacket);
#endif

  // finish initialization
  init();
}

////////////////////////////////////////////////////////////////////////////
// Initialize the packet. After this is done, flags will be correct and the
// payload will be pointing at the payload, unless flags say this is a
// compressed packet all bets are off until you've called decompressPacket.
void EQProtocolPacket::init()
{
  // Get the net op code. Leave in network order. Need this to decide things.
  m_netOp = *(uint16_t*)(m_packetData);

  // Now line up the payload as best we can. Note that if this packet is 
  // compressed, the opcode could potentially be wrong and the payload
  // not aligned properly. You need to call decompressPacket to make sure. But
  // on non-compressed packets this is good enough and you don't need to
  // decompressPacket
  if (! hasFlags())
  {
    // No flags. Netopcode, then payload. Easy.
    m_flags = 0;
    m_uncompressedData = &m_packetData[2];
    m_bAllocedPayload = false;

    // Total - net op - crc
    if (m_packetDataLength < 2 + (hasCRC() ? 2 : 0)) {
        seqWarn("EQProtocolPacket::init() truncated m_uncompressedDataLength to 0."
                " m_netOp=%#04x m_packetDataLength=%d hasCRC()=%d"
                " hasFlags()=%d m_flags=%#04x",
                m_netOp, m_packetDataLength, hasCRC(),
                hasFlags(), m_flags);
        m_uncompressedDataLength = 0;
    } else {
        m_uncompressedDataLength = m_packetDataLength - 2 - (hasCRC() ? 2 : 0);
    }

    // Decoded since no flags
    m_payload = m_uncompressedData;
    m_payloadLength = m_uncompressedDataLength;
    m_bDecoded = true;
  }
  else
  {
    // Flags in the stream. Placement depends on whether this is an app or net
    // opcode.
    if (IS_APP_OPCODE(m_netOp))
    {
      // opcode is an app opcode. Flags are byte 2 of the packet.
      m_flags = m_packetData[1];

      // The opcode is split by the flags. If it is compressed (based on those 
      // above flags) then this is wrong but will fixed by decompressPacket
      m_netOp = m_packetData[2] << 8 | m_packetData[0];
    }
    else
    {
      // Flags at byte #3 after net opcode. m_netOp is correct.
      m_flags = m_packetData[2];
    }

    // Either way, let's start the payload at byte 4 for now. decompressPacket
    // may change this. Length is total - netop - flags - crc.
    m_uncompressedData = &m_packetData[3];
    if (m_packetDataLength < 2 + 1 + (hasCRC() ? 2 : 0)) {
        m_uncompressedDataLength = 0;
        seqWarn("EQProtocolPacket::init() truncated m_uncompressedDataLength to 0."
                " m_netOp=%#04x m_packetDataLength=%d hasCRC()=%d"
                " hasFlags()=%d m_flags=%#04x",
                m_netOp, m_packetDataLength, hasCRC(),
                hasFlags(), m_flags);
    } else {
        m_uncompressedDataLength = m_packetDataLength - 2 - 1 - (hasCRC() ? 2 : 0);
    }
    m_bAllocedPayload = false;

    if (! (m_flags & PROTOCOL_FLAG_COMPRESSED))
    {
      // We have the packet here, let's finish the job.
      m_payloadLength = m_uncompressedDataLength;
      m_payload = m_uncompressedData;

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
#ifdef DEBUG_PACKETFORMAT
    seqDebug("EQProtocolPacket::init() m_packetDataLength=%d"
            " m_uncompressedDataLength=%d m_payloadLength=%d m_flags=%#04x"
            " m_arqSeq=%d m_netOp=%#04x m_bDecoded=%d (pre-decompressPacket call)",
            m_packetDataLength,
            m_uncompressedDataLength, m_payloadLength, m_flags,
            m_arqSeq, m_netOp, m_bDecoded);
#endif
}

////////////////////////////////////////////////////////////////
// Take a raw wire packet and align the payload, decompressing if necessary
bool EQProtocolPacket::decompressPacket(uint32_t maxPayloadLength)
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
      m_payloadLength = m_packetDataLength - 1 - 1 - (hasCRC() ? 2 : 0);
      m_payload = &m_packetData[2];
    }
    else
    {
      // Total - netop - flags - crc
      m_payloadLength = m_packetDataLength - 2 - 1 - (hasCRC() ? 2 : 0);
      m_payload = &m_packetData[3];
    }

    // Compressed. Need to inflate. m_uncompressData is going to be our decompress
    // buffer and needs to be managed properly.
    m_uncompressedData = new uint8_t[maxPayloadLength];
    m_uncompressedDataLength = maxPayloadLength; // alloced size for zlib
    m_bAllocedPayload = true;

    // Decompress
    uLongf destLen = m_uncompressedDataLength;
    uint32_t retval = uncompress(m_uncompressedData, &destLen,
      m_payload, m_payloadLength);
    m_uncompressedDataLength = destLen;

    if (retval != 0)
    {
      seqWarn("Uncompress failed for packet op %04x, flags %02x. Error was %s (%d)",
        getNetOpCode(), getFlags(), zError(retval), retval);

      delete[] m_uncompressedData;
      m_bAllocedPayload = false;
      return false;
    }

    // Align buffer pointers in the decompressed buffer and reconstitue the 
    // opcode if it's a split compressed app opcode
    if (IS_APP_OPCODE(getNetOpCode()))
    {
      // Actual net op is first raw byte + first uncompressed byte
      m_netOp = m_uncompressedData[0] << 8 | m_packetData[0];

      // payload is the actual payload, skipping the 2nd byte of the opcode
      m_payload = &m_uncompressedData[1];
      m_payloadLength = m_uncompressedDataLength - 1;
    }
    else
    {
      // Net op is correct. Payload is correct.
      m_payload = m_uncompressedData;
      m_payloadLength = m_uncompressedDataLength;
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

#ifdef DEBUG_PACKETFORMAT
    seqDebug("EQProtocolPacket::decompressPacket() m_packetDataLength=%d"
            " m_uncompressedDataLength=%d m_payloadLength=%d m_flags=%#04x"
            " m_arqSeq=%d m_netOp=%#04x m_bDecoded=%d",
            m_packetDataLength,
            m_uncompressedDataLength, m_payloadLength, m_flags,
            m_arqSeq, m_netOp, m_bDecoded);
#endif

  return true;
}

//----------------------------------------------------------------------
// EQUDPIPPacketFormat class methods
EQUDPIPPacketFormat::EQUDPIPPacketFormat(uint8_t* data, 
					 uint32_t length, 
					 bool copy)
{
#ifdef DEBUG_PACKETFORMAT
    seqDebug("EQUDPIPPacketFormat::ctor() length=%lu copy=%d", length, copy);
#endif
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
    uint8_t* ipdata = new uint8_t[packet.m_ipTotalLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_ipTotalLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_ipTotalLength = packet.m_ipTotalLength;
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
    uint8_t* ipdata = new uint8_t[packet.m_ipTotalLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_ipTotalLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_ipTotalLength = packet.m_ipTotalLength;
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
  m_ipTotalLength = ntohs (m_ip->ip_len);

  // use this length to caclulate the rest
  length = m_ipTotalLength;

  // skip past the IP header
  ipHeaderLength  = m_ip->ip_hl * 4;
  length  -= ipHeaderLength;
  data += ipHeaderLength;

  // get the UDP header
  m_udp   = (struct udphdr *) data;

  // skip over UDP header
  length  -= sizeof  (struct udphdr);
  data += (sizeof (struct udphdr));
  m_udpPayload = data;
  m_udpPayloadLength = length;

#ifdef DEBUG_PACKETFORMAT
    seqDebug("EQUDPIPPacketFormat::init() ipHeaderLength=%d sizeof(udphdr)=%d"
            " m_ipTotalLength=%d m_udpPayloadLength=%d",
            ipHeaderLength, sizeof(struct udphdr),
            m_ipTotalLength, m_udpPayloadLength);
#endif
  // initialize underlying EQProtocolPacket with the UDP payload
  EQProtocolPacket::init(data, length, false);
}
