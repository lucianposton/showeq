/*
 * packetformat.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2004 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2004 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _PACKETFORMAT_H_
#define _PACKETFORMAT_H_

#include "packetcommon.h"

#if defined (__GLIBC__) && (__GLIBC__ < 2)
#error "Need glibc 2.1.3 or better"
#endif

#if (defined(__FreeBSD__) || defined(__linux__)) && defined(__GLIBC__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ < 2)
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#endif

#include <netinet/in.h>
#include <netinet/ip.h>  

#if defined(__linux__)
#define __FAVOR_BSD
#endif

#include <netinet/udp.h>

#ifdef __FAVOR_BSD
#undef __FAVOR_BSD
#endif 

#include <arpa/inet.h>

#include "util.h"

// Forward declarations
class QString;

//----------------------------------------------------------------------
// enumerated types
enum EQPacketHeaderFlagsHi
{
  seqEnd = 0x80,
  closingHi = 0x40,
  seqStart = 0x20,
  asq = 0x10,
  fragment = 0x08,
  closingLo = 0x04,
  ackRequest = 0x02,
};

enum EQPacketHeaderFlagsLo
{
  ackResponse = 0x04,
  specAckRequest = 0x01
};

//----------------------------------------------------------------------
// EQPacketFormatRaw

// Why EQPacketFormatRaw, because the EQPacket name was taken... ;-)
// (/thank Xylor for the format of the packets)
// (/thank fee for helping me to understand the format)
// Some simplifying assumptions based on observed packet behavior
// that are used to optimize access
// 1) the arq bit is always set when the fragment bit is set
// 2) the arq is always after 2 bytes past the embedded ack/req data.
// 3) the fragment stuff is always 4 bytes after the embedded ack/req data
//    based on 1).
// 
#pragma pack(1)
class EQPacketFormatRaw
{
 public:
  // accessors to check for individual flasg
  bool isARQ() const { return m_flagsHi.m_arq; }
  bool isClosingLo() const { return m_flagsHi.m_closingLo; }
  bool isFragment() const { return m_flagsHi.m_fragment; }
  bool isASQ() const { return m_flagsHi.m_asq; }
  bool isSEQStart() const { return m_flagsHi.m_seqStart; }
  bool isClosingHi() const { return m_flagsHi.m_closingHi; }
  bool isSEQEnd() const { return m_flagsHi.m_seqEnd; }
  bool isARSP() const { return m_flagsLo.m_arsp; }
  bool isSpecARQ() const { return m_flagsLo.m_specARQ; }
  bool isNAK() const { return m_flagsLo.m_nak; }

  // Check flag against the EQPacketHeaderFlag{Hi, Lo} values
  bool checkFlagHi(uint8_t mask) const
    { return ((m_flagsHiVal & mask) != 0); }
  bool checkFlagLo(uint8_t mask) const
    { return ((m_flagsLoVal & mask) != 0); }

  // accessors to return the flag values
  uint8_t flagsHi() const { return m_flagsHiVal; }
  uint8_t flagsLo() const { return m_flagsLoVal; }

  // number of uint8_t's to skip when examining packets
  uint8_t skip() const
  {
    return ((uint8_t)m_flagsLo.m_skip + 
	    (isARSP() ? 2 : 0) + 
	    (isNAK() ? 2 : 0));
  }

  // The following accessors are only valid if the corresponding
  // flag is set.
  uint16_t seq() const { return eqntohuint16(&m_data[0]); }
  uint16_t arsp() const { return eqntohuint16(&m_data[2]); }
  uint16_t nak() const { return  eqntohuint16(&m_data[4]); }
  uint16_t arq() const 
    { return eqntohuint16(&m_data[2 + skip()]); }
  uint16_t fragSeq() const
    { return eqntohuint16(&m_data[4 + skip()]); }
  uint16_t fragCur() const
    { return eqntohuint16(&m_data[6 + skip()]); }
  uint16_t fragTot() const
    { return eqntohuint16(&m_data[8 + skip()]); }
  uint8_t asqHigh() const { return m_data[10 + skip()]; }
  uint8_t asqLow() const { return m_data[11 + skip()]; }
  uint32_t crc32(uint16_t len) const 
    {   return (len >= 6) ? eqntohuint32(&m_data[len - 2 - 4]) : 0xDEADBEEF; }

  uint8_t* payload()
  {
    return m_data + // m_data is already passed the flag bytes
      (skip() // skip arsp, specARQ data
       + (isASQ() ? 1 : 0) // skip asqHigh
       + (isARQ() ? 2 : 0) // skip arq
       + ((isASQ() && isARQ()) ? 1 : 0) // skip asqLow
       + (isFragment() ? 6 : 0) // skip fragment data
       + 2 // seq
       );
  }

  QString headerFlags(const QString& prefix = "",
		      bool brief = false) const;

 private:
  union 
  {
    struct 
    { 
      unsigned int m_unknown1:1;       // 0x01 = ?
      unsigned int m_arq:1;            // 0x02 = arq
      unsigned int m_closingLo:1;      // 0x04 = closingLo
      unsigned int m_fragment:1;       // 0x08 = fragment
      unsigned int m_asq:1;            // 0x10 = asq
      unsigned int m_seqStart:1;       // 0x20 = seqStart
      unsigned int m_closingHi:1;      // 0x40 = closingHi
      unsigned int m_seqEnd:1;         // 0x80 = seqEnd
    } m_flagsHi;
    uint8_t m_flagsHiVal;
  };
  union
  {
    struct
    {
      unsigned int m_specARQ:1;        // 0x01 = speckARQ
      unsigned int m_unknown1:1;       // 0x02 = ?
      unsigned int m_arsp:1;           // 0x04 = ARSP
      unsigned int m_nak:1;            // 0x08 = NAK?
      unsigned int m_skip:4;           // amount of data to skip
    } m_flagsLo;
    uint8_t m_flagsLoVal;
  };
  uint8_t m_data[];
};
#pragma pack()

//----------------------------------------------------------------------
// EQPacketFormat
class EQPacketFormat
{
 public:
  // constructors
  EQPacketFormat()
    : m_packet(NULL), m_length(0), 
    m_postSkipData(NULL), m_postSkipDataLength(0), 
    m_payload(NULL), m_payloadLength(0),
    m_arq(0), m_ownCopy(false), m_isValid(false)
    {  }
    
  EQPacketFormat(uint8_t* data, 
		 uint16_t length,
		 bool copy = false)
  { 
    init((EQPacketFormatRaw*)data, length, copy);
  }

  EQPacketFormat(EQPacketFormatRaw* packet, 
		 uint16_t length,
		 bool copy = false)
  {
    init(packet, length, copy);
  }

  EQPacketFormat(EQPacketFormat& packet, bool copy = false);

  // destructor
  ~EQPacketFormat();

  // operators
  EQPacketFormat& operator=(const EQPacketFormat& packet);

  // accessors to check for individual flags
  bool isARQ() const { return m_packet->isARQ(); }
  bool isClosingLo() const { return m_packet->isClosingLo(); }
  bool isFragment() const { return m_packet->isFragment(); }
  bool isASQ() const { return m_packet->isASQ(); }
  bool isSEQStart() const { return m_packet->isSEQStart(); }
  bool isClosingHi() const { return m_packet->isClosingHi(); }
  bool isSEQEnd() const { return m_packet->isSEQEnd(); }
  bool isARSP() const { return m_packet->isARSP(); }
  bool isNAK() const { return m_packet->isNAK(); }
  bool isSpecARQ() const
    { return m_packet->isSpecARQ(); }

  // Check flag against the EQPacketHeaderFlag{Hi, Lo} values
  bool checkFlagHi(uint8_t mask) const
    { return m_packet->checkFlagHi(mask); }
  bool checkFlagLo(uint8_t mask) const
    { return m_packet->checkFlagLo(mask); }

  // accessors to return the flag values
  uint8_t flagsHi() const { return m_packet->flagsHi(); }
  uint8_t flagsLo() const { return m_packet->flagsLo(); }

  uint8_t skip() const { return m_packet->skip(); }

  uint16_t seq() const { return m_packet->seq(); }

  // The following accessors are only valid if the corresponding
  // flag is set.
  uint16_t arsp() const { return m_packet->arsp(); }
  uint16_t arq() const { return m_arq; }
  uint16_t fragSeq() const 
    { return eqntohuint16(&m_postSkipData[4]); }
  uint16_t fragCur() const 
    { return eqntohuint16(&m_postSkipData[6]); }
  uint16_t fragTot() const 
    { return eqntohuint16(&m_postSkipData[8]); }
  uint8_t asqHigh() const { return m_postSkipData[10]; }
  uint8_t asqLow() const { return m_postSkipData[11]; }
  uint32_t crc32() const
  { 
    // return CRC in the appropriate endianess or DEADBEEF if invalid
    return (m_length >= 4) ? eqntohuint32(&((uint8_t*)m_packet)[m_length - 4]) : 0xDEADBEEF;
  }
  uint32_t calcCRC32() const
  {
    // calculate the CRC on the packet data, up to but not including the
    // CRC32 stored at the end or return different invalid packet value
    // then crc32() above.
    return (m_length >= 4) ? ::calcCRC32((uint8_t*)m_packet, m_length - 4) : 0xFEEDDEAD;
  }

  bool isValid() const { return m_isValid; }
  bool validate();

  uint8_t* payload() const { return m_payload; }
  uint16_t payloadLength() const { return m_payloadLength; }

  const EQPacketFormatRaw* getRawPacket() const { return m_packet; }
  uint16_t getRawPacketLength() const { return m_length; }

  QString headerFlags(const QString& prefix = "",
		      bool brief = false) const;

 protected:
  void init();
  void init(EQPacketFormatRaw* packet, 
	    uint16_t length,
	    bool copy = false);
  
 private:
  EQPacketFormatRaw* m_packet;
  uint16_t m_length;
  uint8_t* m_postSkipData;
  uint16_t m_postSkipDataLength;
  uint8_t* m_payload;
  uint16_t m_payloadLength;
  uint16_t m_arq; // local copy to speed up comparisons
  bool m_ownCopy;
  bool m_isValid;
};

inline bool operator<(const EQPacketFormat& p1,
	       const EQPacketFormat& p2)
{ 
  return p1.arq() < p2.arq(); 
}

inline bool operator==(const EQPacketFormat& p1,
		const EQPacketFormat& p2)
{
  return p1.arq() == p2.arq(); 
}

//----------------------------------------------------------------------
// EQUDPIPPacketFormat
class EQUDPIPPacketFormat : public EQPacketFormat
{
 public:
  // constructors
  EQUDPIPPacketFormat(uint8_t* data, 
		      uint32_t length, 
		      bool copy = false);

  EQUDPIPPacketFormat(EQUDPIPPacketFormat& packet,
		      bool copy = false);

  // destructor
  ~EQUDPIPPacketFormat();

  // operators
  EQUDPIPPacketFormat& operator=(const EQUDPIPPacketFormat& packet);

  // UDP accessors
  in_port_t getSourcePort() const { return ntohs(m_udp->uh_sport); }
  in_port_t getSourcePortN() const { return m_udp->uh_sport; }
  in_port_t getDestPort() const { return ntohs(m_udp->uh_dport); }
  in_port_t getDestPortN() const { return m_udp->uh_dport; }
  uint8_t * getUDPPayload() const { return m_rawpayload; }

  // IP accessors
  uint8_t getIPVersion() const { return (uint8_t)m_ip->ip_v; }

  // IPv4 accessors
  uint8_t getIPv4TOS() const { return m_ip->ip_tos; }
  uint16_t getIPv4IDRaw() const { return m_ip->ip_id; }
  uint16_t getIPv4FragOff() const { return m_ip->ip_off; }
  uint8_t getIPv4Protocol() const { return m_ip->ip_p; }
  uint8_t getIPv4TTL() const { return m_ip->ip_ttl; }
  in_addr_t getIPv4Source() const { return htonl(m_ip->ip_src.s_addr); }
  in_addr_t getIPv4SourceN() const { return m_ip->ip_src.s_addr; }
  QString getIPv4SourceA() const { return inet_ntoa(m_ip->ip_src); }
  in_addr getIPv4SourceInAddr() const { return m_ip->ip_src; }

  in_addr_t getIPv4Dest() const { return htonl(m_ip->ip_dst.s_addr); }
  in_addr_t getIPv4DestN() const { return m_ip->ip_dst.s_addr; }
  QString getIPv4DestA() const { return inet_ntoa(m_ip->ip_dst); }
  in_addr getIPv4DestInAddr() const { return m_ip->ip_dst; }
  
  // Don't currently support IPv6, so no IPv6 accessors

  QString headerFlags(bool brief = false) const;

 protected:
  void init(uint8_t* data);
  
 private:
  uint32_t m_dataLength;
  struct ip* m_ip;
  struct udphdr *m_udp;
  bool m_ownCopy;
  uint8_t* m_rawpayload;
};

#endif // _PACKETFORMAT_H_

