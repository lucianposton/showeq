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

//----------------------------------------------------------------------
// EQPacketFormatRaw class methods
QString EQPacketFormatRaw::headerFlags(const QString& prefix, 
				       bool brief) const
{
  QString tmp;
  if (!prefix.isEmpty())
  {
    if (brief)
      tmp = prefix + ": ";
    else
      tmp = prefix + "[Hdr (" + QString::number(flagsHi(), 16) + ", "
	+ QString::number(flagsLo(), 16) + "): ";
  }
  else if (!brief)
    tmp = "[Hdr (" + QString::number(flagsHi(), 16) + ", "
	+ QString::number(flagsLo(), 16) + "): ";

  if (isARQ())
    tmp += "ARQ, ";
  if (isClosingLo())
    tmp += "closingLo, ";
  if (isFragment())
    tmp += "Fragment, ";
  if (isASQ())
    tmp += "ASQ, ";
  if (isSEQStart())
    tmp += "SEQStart, ";
  if (isClosingHi())
    tmp += "closingHi, ";
  if (isSEQEnd())
    tmp += "SEQEnd, ";
  if (isARSP())
    tmp += "ARSP, ";
  if (isNAK())
    tmp += "NAK, ";
  if (isSpecARQ())
    tmp += "SpecARQ, ";
  if (m_flagsHi.m_unknown1)
    tmp += "HiUnknown1, ";

  if (skip() != 0)
    tmp += QString("Skip: ") + QString::number(skip(), 16) + ", ";
  
  tmp += QString("seq: ") + QString::number(seq(), 16);

  tmp += "] ";

  if (!brief)
  {
    if (isARQ())
      tmp += QString("[ARQ: ") + QString::number(arq(), 16) + "] ";
    
    if (isFragment())
    {
      tmp += QString("[FragSeq: ") + QString::number(fragSeq(), 16)
	+ ", FragCur: " + QString::number(fragCur(), 16)
	+ ", FragTot: " + QString::number(fragTot(), 16) + "] ";
    }
  }

  return tmp;
}

//----------------------------------------------------------------------
// EQPacketFormat class methods
EQPacketFormat::EQPacketFormat(EQPacketFormat& packet, bool copy)
  : m_ownCopy(copy),
    m_isValid(packet.m_isValid)
{
  if (!copy)
  {
    // just copy all their values
    m_packet = packet.m_packet;
    m_length = packet.m_length;
    m_postSkipData = packet.m_postSkipData;
    m_postSkipDataLength = packet.m_postSkipDataLength;
    m_payload = packet.m_payload;
    m_payloadLength = packet.m_payloadLength;
    m_arq = packet.m_arq;
  }
  else
  {
    init(packet.m_packet, packet.m_length, copy);
  }
}

EQPacketFormat::~EQPacketFormat()
{
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;
}

EQPacketFormat& EQPacketFormat::operator=(const EQPacketFormat& packet)
{
  // if this was a deep copy, delete the existing data
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;

  // initialize as deep as this object previously was
  init(packet.m_packet, packet.m_length, m_ownCopy);

  return *this;
}

void EQPacketFormat::init(EQPacketFormatRaw* packet,
			  uint16_t length,
			  bool copy)
{
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
    // NOTE: We should use an allocater for this instead of normal new
    m_packet = (EQPacketFormatRaw*)new uint8_t[length];

    // copy the data
    memcpy((void*)m_packet, (void*)packet, length);

    // save the length
    m_length = length;
  }

  // finish initialization
  init();
}

void EQPacketFormat::init()
{
  // finish initialization iff the packet has a valid CRC32
  if (validate())
  {
    m_postSkipData = ((uint8_t*)m_packet) + m_packet->skip() + 2;
    
    // calculate the length of the rest of the data
    m_postSkipDataLength = m_length - (m_postSkipData - ((uint8_t*)m_packet));
    
    // get the location of the payload
    m_payload = m_packet->payload();
    
    // calculate the lenght of the payload (length - diff - len(checkSum))
    m_payloadLength = m_length - (m_payload - ((uint8_t*)m_packet)) - 4; 
    
    // make a local copy of the arq to speed up comparisons
    m_arq = eqntohuint16(&m_postSkipData[2]);
  }
  else 
  {
    m_postSkipData = 0;
    m_postSkipDataLength = 0;
    m_payload = 0;
    m_payloadLength = 0;
    m_arq = 0;
  }
}

bool EQPacketFormat::validate()
{ 
  m_isValid = ((m_packet != NULL) && 
	       ((m_length >= 4) && (crc32() == calcCRC32()))); 
  return m_isValid;
}

QString EQPacketFormat::headerFlags(const QString& prefix, 
				    bool brief) const
{
  QString tmp;

  if (m_isValid)
  {
    if (brief)
      tmp = prefix + ": ";
    else
      tmp = prefix + " Hdr (" + QString::number(flagsHi(), 16) + ", "
	+ QString::number(flagsLo(), 16) + "): ";
    
    tmp += m_packet->headerFlags("", true);
    
    if (!brief)
    {
      if (isARQ())
	tmp += QString("arq: ") + QString::number(arq(), 16) + ", ";
      
      if (isFragment())
	tmp += QString("FragSeq: ") + QString::number(fragSeq(), 16)
	  + ", FragCur: " + QString::number(fragCur(), 16)
	  + ", FragTot: " + QString::number(fragTot(), 16) + ", ";

      tmp += QString("Opcode: ") 
	+ QString::number(eqntohuint16(payload()), 16);
    }
  }
  else
      tmp = prefix + ": INVALID CRC32! Possible non-EQ packet?!";

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

  // initialize the object
  init(ipdata);
}

EQUDPIPPacketFormat::EQUDPIPPacketFormat(EQUDPIPPacketFormat& packet,
					 bool copy)
{
  // note whether or not this object ownw the memory
  m_ownCopy = copy;

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

  // initialize underlying EQPacketFormat with the UDP payload
  EQPacketFormat::init((EQPacketFormatRaw*)data, length, false);
}

QString EQUDPIPPacketFormat::headerFlags(bool brief) const
{
  QString tmp;
  tmp.sprintf("[%s:%d -> %s:%d]", 
	      (const char*)getIPv4SourceA(), getSourcePort(),
	      (const char*)getIPv4DestA(), getDestPort());

  return EQPacketFormat::headerFlags(tmp, brief);
}
