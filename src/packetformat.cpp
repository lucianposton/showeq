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

//----------------------------------------------------------------------
// EQProtocolPacket class methods
EQProtocolPacket::EQProtocolPacket(EQProtocolPacket& packet, bool copy)
  : m_ownCopy(copy)
{
  if (!copy)
  {
    // just copy all their values
    m_packet = packet.m_packet;
    m_length = packet.m_length;
    m_payload = packet.m_payload;
    m_payloadLength = packet.m_payloadLength;
    m_arqSeq = packet.m_arqSeq;
    m_subpacket = packet.m_subpacket;
  }
  else
  {
    init(packet.m_packet, packet.m_length, copy, packet.m_subpacket);
  }
}

EQProtocolPacket::~EQProtocolPacket()
{
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;
}

EQProtocolPacket& EQProtocolPacket::operator=(const EQProtocolPacket& packet)
{
  // if this was a deep copy, delete the existing data
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;

  // initialize as deep as this object previously was
  init(packet.m_packet, packet.m_length, m_ownCopy, packet.m_subpacket);

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

void EQProtocolPacket::init()
{
  // Get the net op code. Leave in network order.
  m_netOp = *(uint16_t*)(m_packet);

  // get the location of the payload
  m_payload = &m_packet[2];
    
  // calculate the length of the payload (len - net op - crc)
  m_payloadLength = m_length - 2 - (hasCRC() ? 2 : 0);
    
  // make a local copy of the arq to speed up comparisons
  m_arqSeq = eqntohuint16(m_payload);
}

QString EQProtocolPacket::headerFlags(const QString& prefix, bool brief) const
{
  QString tmp;

  if (brief)
  {
    tmp = prefix + ": ";
  }
  else
  {
    tmp = prefix + " NetOpCode: " + QString::number(getNetOpCode(), 16);

    if (hasArqSeq())
    {
      tmp += (", Seq: " + QString::number(arqSeq()));
    }
  }

  return tmp;
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

QString EQUDPIPPacketFormat::headerFlags(bool brief) const
{
  QString tmp;
  tmp.sprintf("[%s:%d -> %s:%d]", 
	      (const char*)getIPv4SourceA(), getSourcePort(),
	      (const char*)getIPv4DestA(), getDestPort());

  return EQProtocolPacket::headerFlags(tmp, brief);
}
