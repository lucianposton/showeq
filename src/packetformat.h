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

// Net Op Codes in net order. Underneath, there are actually channels
// 0-3, where 0x0900 is OP_Packet on channel 0, 0x0a00 is OP_Packet on
// channel 1, 0x0b00 is OP_Packet on channel 2, etc. and the same
// with OP_Oversized for 0x0d00-0x1000. Only channel 0 seems used.
static const uint16_t OP_SessionRequest     = 0x0100;
static const uint16_t OP_SessionResponse    = 0x0200;
static const uint16_t OP_Combined           = 0x0300;
static const uint16_t OP_SessionDisconnect  = 0x0500;
static const uint16_t OP_KeepAlive          = 0x0600;
static const uint16_t OP_SessionStatRequest = 0x0700;
static const uint16_t OP_SessionStatResponse= 0x0800;
static const uint16_t OP_Packet             = 0x0900;
static const uint16_t OP_Oversized          = 0x0d00;
static const uint16_t OP_AckFuture          = 0x1100;
static const uint16_t OP_Ack                = 0x1500;
static const uint16_t OP_AppCombined        = 0x1900;
static const uint16_t OP_AckAfterDisconnect = 0x1d00;

// Detect app opcode vs net opcode by the 2nd byte. 0x00 = net opcode.
#define IS_APP_OPCODE(uint16) (((uint16) & 0x00ff) != 0x0000)
#define IS_NET_OPCODE(uint16) (((uint16) & 0x00ff) == 0x0000)

// Protocol flag bitmasks. Actually, we just flag straight to compressed
// which is 5a right now. I wonder what the significance of that is.
#define PROTOCOL_FLAG_COMPRESSED 0x5a

// Should we apply CRC checking. This should be yes, but if for some reason
// it is getting in the way while debugging, can turn it off
#define APPLY_CRC_CHECK

//----------------------------------------------------------------------
// EQProtocolPacket
// A wrapper around a byte array which is the wire data for an
// specific protocol packet. Protocol packets have an owning EQUDPIPPacket
// which has the udp/ip packet info. They may not necessaryily directly be
// the payload of the udp/ip packet, since one udp/ip packet may be
// multiple protocol packets, combined together.
class EQProtocolPacket
{
 public:
  // constructors
  EQProtocolPacket()
    : m_packet(NULL), 
      m_length(0), 
      m_flags(0),
      m_payload(NULL), 
      m_bAllocedPayload(false),
      m_bDecoded(false),
      m_payloadLength(0),
      m_arqSeq(0), 
      m_ownCopy(false), 
      m_subpacket(false)
    {  }
    
  EQProtocolPacket(uint8_t* packet, uint32_t length, 
    bool copy=false, bool subpacket=false)
  {
    init(packet, length, copy, subpacket);
  }

  EQProtocolPacket(EQProtocolPacket& packet, bool copy = false);

  // destructor
  ~EQProtocolPacket();

  // operators
  EQProtocolPacket& operator=(const EQProtocolPacket& packet);

  // Decode the packet. This processed compressed packets and readjusts
  // alignments if needed. If this returns false, using the packet isn't
  // recommended!
  bool decode(uint32_t maxPacketLength);

  uint16_t getNetOpCode() const { return m_netOp; }

  bool hasFlags() const
  {
    // Subpackets don't have flags, the outer packet does.
    if (m_subpacket) return false;

    switch (m_netOp)
    {
      case OP_SessionStatRequest:
      case OP_SessionStatResponse:
      case OP_Combined:
      case OP_Packet:
      case OP_Oversized:
      case OP_AppCombined:
        return true;
      default :
        return IS_APP_OPCODE(m_netOp);
    }
  }
  uint8_t getFlags() const { return m_flags; }

  bool hasArqSeq() const
  {
    return getNetOpCode() == OP_Packet || getNetOpCode() == OP_Oversized;
  }
  uint16_t arqSeq() const { return m_arqSeq; }

  bool hasCRC() const
  {
    // Subpackets don't have CRC, the outer packet does.
    if (m_subpacket) return false;

    switch (m_netOp)
    {
      case OP_SessionStatRequest:
      case OP_SessionStatResponse:
      case OP_Combined:
      case OP_Packet:
      case OP_Oversized:
      case OP_AppCombined:
        return true;
      default :
        return IS_APP_OPCODE(m_netOp);
    }
  }
  uint16_t crc() const
  { 
    // return CRC in the appropriate endianess or DEAD if invalid
    return (m_length >= 2) ? eqntohuint16(&m_packet[m_length - 2]) : 0xDEAD;
  }

  bool isSubpacket() const { return m_subpacket; }

  // Payload is uncompressed (after decode is called) and aligned to the
  // beginning of the payload (after net op, flags, seq if applicable)
  uint8_t* payload() const { return m_payload; }
  uint32_t payloadLength() const { return m_payloadLength; }

  // Raw Packet is compressed and aligned to the start of the net op. Length
  // includes CRC if applicable.
  uint8_t* rawPacket() const { return m_packet; }
  uint16_t rawPacketLength() const { return m_length; }

  // Raw payload is uncompressed (after decode is called) and aligned to
  // the beginning of what was decompressed. This is what is alloced if
  // m_bAllocPayload is true.
  uint8_t* rawPayload() const { return m_rawPayload; }
  uint32_t rawPayloadLength() const { return m_rawPayloadLength; }

 protected:
  void init();
  void init(uint8_t* packet, uint32_t length, 
    bool copy=false, bool subpacket=false);

 private:
  uint8_t* m_packet; // raw packet, untouched starting at net opcode
  uint32_t m_length; // raw packet length
  uint16_t m_netOp; // protocol opcode
  uint8_t m_flags; // protocol flags
  uint8_t* m_payload; // packet payload. Aligned and uncompressed if necessary.
  bool m_bAllocedPayload; // Whether payload was alloced or not
  bool m_bDecoded; // Whether this packet has been decoded
  uint32_t m_payloadLength; // length of payload
  uint8_t* m_rawPayload; // decompressed but not aligned payload
  uint32_t m_rawPayloadLength; // length of raw payload
  uint16_t m_arqSeq; // local copy to speed up comparisons
  bool m_ownCopy;
  bool m_subpacket;
};

inline bool operator<(const EQProtocolPacket& p1, const EQProtocolPacket& p2)
{ 
  return p1.arqSeq() < p2.arqSeq(); 
}

inline bool operator==(const EQProtocolPacket& p1, const EQProtocolPacket& p2)
{
  return p1.arqSeq() == p2.arqSeq(); 
}

//----------------------------------------------------------------------
// EQUDPIPPacketFormat
class EQUDPIPPacketFormat : public EQProtocolPacket
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
  uint8_t* getUDPPayload() const { return m_rawpayload; }
  uint32_t getUDPPayloadLength() const { return m_rawpayloadSize; }

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

  uint32_t getSessionKey() const { return m_sessionKey; }
  void setSessionKey(uint32_t sessionKey) { m_sessionKey = sessionKey; }

 protected:
  void init(uint8_t* data);
  
 private:
  uint32_t m_dataLength;
  struct ip* m_ip;
  struct udphdr *m_udp;
  bool m_ownCopy;
  uint8_t* m_rawpayload;
  uint32_t m_rawpayloadSize;
  uint32_t m_sessionKey;
};

#endif // _PACKETFORMAT_H_

