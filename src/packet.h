
/*
 * packet.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#ifndef EQPACKET_H
#define EQPACKET_H

#define MAXSTREAMS          4

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if !defined(__BYTE_ORDER)
#if defined(BYTE_ORDER)
#define __BYTE_ORDER	BYTE_ORDER
#define __LITTLE_ENDIAN	LITTLE_ENDIAN
#define __BIG_ENDIAN	BIG_ENDIAN
#else
#error "BYTE_ORDER should be LITTLE_ENDIAN or BIG_ENDIAN (the latter is untested)"
#endif
#endif
#else
#include <stdint.h>
#endif

#ifdef __linux__
#include <endian.h>
#endif

#include <map>

#include <qqueue.h>

#include <qmap.h>
#include <qobject.h>
#include <qregexp.h>
#include "everquest.h"
#include "opcodes.h"
#include "decode.h"
#include "util.h"

#if defined (__GLIBC__) && (__GLIBC__ < 2)
#error "Need glibc 2.1.3 or better"
#endif

#if (defined(__FreeBSD__) || defined(__linux__)) && defined(__GLIBC__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ < 2)
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#endif

#include <netinet/in_systm.h>
#include <net/if.h>
#include <netinet/if_ether.h>
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
#include <pthread.h>

extern "C" { // fix for bpf not being c++ happy
#include <pcap.h>
}

//----------------------------------------------------------------------
// forward declarations

// Used in the packet capture filter setup.  If address_type is
//   MAC_ADDRESS_TYPE, then we use the hostname string as a MAC address
// for the filter. cpphack
const uint8_t DEFAULT_ADDRESS_TYPE = 10;   /* These were chosen arbitrarily */
const uint8_t IP_ADDRESS_TYPE = 11;
const uint8_t MAC_ADDRESS_TYPE =  12;

//----------------------------------------------------------------------
// forward declarations
class VPacket;
class PacketCaptureThread;


//----------------------------------------------------------------------
// Misc internal structures

// Internal structure to keep track of
// the time that the time the server sent
// to the client and a time stamp of the
// packet for further calculation
struct eqTimeOfDay
{
  struct timeOfDayStruct zoneInTime;
         time_t          packetReferenceTime;
};

//----------------------------------------------------------------------
// Useful inline functions
#if (defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN))
inline uint16_t eqntohuint16(const uint8_t* data)
{
  return (uint16_t)((data[0] << 8) | data[1]);
}

inline int16_t eqntohint16(const uint8_t* data)
{
  return (int16_t)((data[0] << 8) | data[1]);
}

inline  uint32_t eqntohuint32(const uint8_t* data)
{
  return (uint32_t)((data[0] << 24) | (data[1] << 16) |
		    (data[2] << 8) | data[3]);
}

inline int32_t eqntohint32(const uint8_t* data)
{
  return (int32_t)((data[0] << 24) | (data[1] << 16) |
		   (data[2] << 8) | data[3]);
}

inline uint16_t eqtohuint16(const uint8_t* data)
{
  return *(uint16_t*)data;
}

inline int16_t eqtohint16(const uint8_t* data)
{
  return *(int16_t*)data;
}

inline uint32_t eqtohuint32(const uint8_t* data)
{
  return *(uint32_t*)data;
}

inline int32_t eqtohint32(const uint8_t* data)
{
  return *(int32_t*)data;
}
#else
#warning "BigEndian hasn't been tested."
inline uint16_t eqntohuint16(const uint8_t* data)
{
  return *(uint16_t*)data;
}

inline int16_t eqntohint16(const uint8_t* data)
{
  return *(int16_t*)data;
}

inline uint32_t eqntohuint32(const uint8_t* data)
{
  return *(uint32_t*)data;
}

inline int32_t eqntohint32(const uint8_t* data)
{
  return *(int32_t*)data;
}

inline uint16_t eqtohuint16(const uint8_t* data)
{
  return (uint16_t)((data[0] << 8) | data[1]);
}

inline int16_t eqtohint16(const uint8_t* data)
{
  return (int16_t)((data[0] << 8) | data[1]);
}

inline  uint32_t eqtohuint32(const uint8_t* data)
{
  return (uint32_t)((data[0] << 24) | (data[1] << 16) |
		    (data[2] << 8) | data[3]);
}

inline int32_t eqtohint32(const uint8_t* data)
{
  return (int32_t)((data[0] << 24) | (data[1] << 16) |
		   (data[2] << 8) | data[3]);
}
#endif

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

enum EQStreamID 
{
  unknown_stream = -1,
  client2world = 0, 
  world2client = 1,
  client2zone = 2, 
  zone2client = 3 
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
#if 0 // ZBTEMP
    return ((uint8_t)m_flagsLo.m_skip + 
	    ((isARSP() || isSpecARQ()) ? 2 : 0));
#else
    return ((uint8_t)m_flagsLo.m_skip + 
	    (isARSP() ? 2 : 0)// +
	    //(isSpecARQ() ? 4 : 0)
);
#endif
  }

  // The following accessors are only valid if the corresponding
  // flag is set.
  uint16_t seq() const { return eqntohuint16(&m_data[0]); }
  uint16_t arsp() const { return eqntohuint16(&m_data[2]); }
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
    { return eqntohuint32(&m_data[len - 2 - 4]); }

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
      unsigned int m_unknown2:1;       // 0x08 = ?
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
    m_arq(0), m_ownCopy(false)
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
    return eqntohuint32(&m_postSkipData[m_postSkipDataLength - 4]);
  }
  uint32_t calcCRC32() 
  {
    // calculate the CRC on the packet data, up to but not including the
    // CRC32 stored at the end.
    return ::calcCRC32((uint8_t*)m_packet, m_length - 4);
  }

  bool isValid() { return crc32() == calcCRC32(); }

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
};

inline bool operator<(const EQPacketFormat& p1,
	       const EQPacketFormat& p2)
{ return p1.arq() < p2.arq(); }

inline bool operator==(const EQPacketFormat& p1,
		const EQPacketFormat& p2)
{ return p1.arq() == p2.arq(); }

//----------------------------------------------------------------------
// map type used for caching packets.
// The reason an STL Map was chosen is because of it's consitent 
// O(log N) behavior, and the key based ordering it enforces
// is convenient for processing.  The Qt hash based collections really
// arent' appropriate for the packet cache for multiple reasons:
// 1) They're iterators are based on key hash order and not key value
// ordering, making certain algorithms more difficult and potentially 
// time consuming. 2) insertions into an STL map can be optimized 
// with the packet data's behavior using an iterator as a hint for 
// insertion location to typically yield amortized constant time behavior.
// 3) Another optimization possible with this data set using a map 
// is that after a matchintg arq is found in the map,  finding/checking 
// for the next expected arq in the map only requires moving the iterator 
// forward (using operator++()) once and checking if the next key in the list
// is the expected arq.  This results in the check for followers to only 
// taking amortized constant time (as opposed to the O(log N) of map::find()
// or constant average time of the hash find methods.
typedef std::map<uint16_t, EQPacketFormat* > EQPacketMap;

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

//----------------------------------------------------------------------
// EQPacket
class EQPacket : public QObject
{
   Q_OBJECT 
 public:
   
   EQPacket(QObject *parent = 0,
            const char *name = 0);
   ~EQPacket();           
   void start(int delay = 0);
   void stop(void);
   void setViewUnknownData(bool);

   const QString pcapFilter();
   int packetCount(int);
   uint32_t clientAddr(void);
   uint16_t clientPort(void);
   uint16_t serverPort(void);
   uint8_t session_tracking_enabled(void);
   int playbackSpeed(void);
   int currentCacheSize(int);
   uint16_t serverSeqExp(int);

   void InitializeOpCodeMonitor(void);
   
   bool m_bOpCodeMonitorInitialized;
   #define OPCODE_SLOTS 15
   unsigned int MonitoredOpCodeList      [OPCODE_SLOTS][3];
   QString MonitoredOpCodeAliasList [OPCODE_SLOTS];
   
   bool logMessage(const QString& filename,
		   const QString& message);
   bool EQPacket::logData ( const QString& filename,
			    uint32_t       len,
			    const uint8_t* data,
			    const QString& prefix);
   bool logData(const QString& filename,
		uint32_t       len,
		const uint8_t* data,
		in_addr_t      saddr = 0,
		in_addr_t      daddr = 0,
		in_port_t      sport = 0,
		in_port_t      dport = 0
                );
               
 public slots:
   void processPackets(void);
   void processPlaybackPackets(void);
   void incPlayback(void);
   void decPlayback(void);
   void setPlayback(int);
   void monitorIPClient(const QString& address);   
   void monitorMACClient(const QString& address);   
   void monitorNextClient();   
   void monitorDevice(const QString& dev);   
   void session_tracking();
   void setArqSeqGiveUp(int giveUp);

   void dispatchDecodedZoneSpawns(const uint8_t* decodedData, uint32_t len);

 signals:
   // used for net_stats display
   void cacheSize(int, int);
   void seqReceive(int, int);
   void seqExpect(int, int);
   void numPacket(int, int);
   void resetPacket(int, int);
   void playbackSpeedChanged(int);
   void clientChanged(uint32_t);
   void clientPortLatched(uint16_t);
   void serverPortLatched(uint16_t);
   void sessionTrackingChanged(uint8_t);

   void toggle_session_tracking(void);
 
   // logging
   void toggle_log_AllPackets(void);
   void toggle_log_WorldData(void);
   void toggle_log_ZoneData(void);
   void toggle_log_UnknownData();
                               

   // Player signals
   void setPlayerID(uint16_t);
   void backfillPlayer(const charProfileStruct *, uint32_t, uint8_t);
   void increaseSkill(const skillIncStruct* skilli, uint32_t, uint8_t);
   void manaChange(const manaDecrementStruct* mana, uint32_t, uint8_t);
   void playerUpdate(const playerSpawnPosStruct* pupdate, uint32_t, uint8_t);
   void playerUpdate(const playerSelfPosStruct* pupdate, uint32_t, uint8_t);
   void updateExp(const expUpdateStruct* exp, uint32_t, uint8_t);
   void updateAltExp(const altExpUpdateStruct* altexp, uint32_t, uint8_t);
   void updateLevel(const levelUpUpdateStruct* levelup, uint32_t, uint8_t);
   void updateStamina(const staminaStruct* stam, uint32_t, uint8_t);

   void attack2Hand1(const attack2Struct*, uint32_t, uint8_t);
   void action2Message(const action2Struct*, uint32_t, uint8_t);
   
   void consMessage(const considerStruct*, uint32_t, uint8_t);
   
   void clientTarget(const clientTargetStruct* target, uint32_t, uint8_t);
   void newDoorSpawns(const doorSpawnsStruct *, uint32_t, uint8_t);
   void newDoorSpawn(const doorStruct *, uint32_t, uint8_t);
   void spawnWearingUpdate(const wearChangeStruct*, uint32_t, uint8_t);

   void newGroundItem(const makeDropStruct*, uint32_t, uint8_t);
   void removeGroundItem(const remDropStruct*, uint32_t, uint8_t);
   void newCoinsItem(const dropCoinsStruct*, uint32_t, uint8_t);
   void removeCoinsItem(const removeCoinsStruct*, uint32_t, uint8_t);

   void updateSpawns(const spawnPositionUpdate* updates, uint32_t, uint8_t);
   void updateSpawnMaxHP(const SpawnUpdateStruct* spawnupdate, uint32_t, uint8_t);
   void updateSpawnInfo(const SpawnUpdateStruct* spawnupdate, uint32_t, uint8_t);

   void newSpawn(const newSpawnStruct* spawn, uint32_t, uint8_t);
   void deleteSpawn(const deleteSpawnStruct* delspawn, uint32_t, uint8_t);
   void killSpawn(const newCorpseStruct* deadspawn, uint32_t, uint8_t);
   void corpseLoc(const corpseLocStruct*, uint32_t, uint8_t);
   void eqTimeChangedStr(const QString &);
   void timeOfDay(const timeOfDayStruct *tday, uint32_t, uint8_t);

   void itemShop(const itemInShopStruct* items, uint32_t, uint8_t);
   void moneyOnCorpse(const moneyOnCorpseStruct* money, uint32_t, uint8_t);
   void itemPlayerReceived(const itemOnCorpseStruct* itemc, uint32_t, uint8_t);
   void tradeItemOut(const tradeItemOutStruct* itemt, uint32_t, uint8_t);
   void tradeItemIn(const tradeItemInStruct* itemr, uint32_t, uint8_t);
   void tradeContainerIn(const tradeContainerInStruct* itemr, uint32_t, uint8_t);
   void tradeBookIn(const tradeBookInStruct* itemr, uint32_t, uint8_t);
   void channelMessage(const channelMessageStruct* cmsg, uint32_t, uint8_t);
   void simpleMessage(const simpleMessageStruct* fmsg, uint32_t, uint8_t);
   void formattedMessage(const formattedMessageStruct* fmsg, uint32_t, uint8_t);
   void random(const randomReqStruct* randr, uint32_t, uint8_t);
   void random(const randomStruct* randr, uint32_t, uint8_t);
   void emoteText(const emoteTextStruct* emotetext, uint32_t, uint8_t);
#if 1 // ZBTEMP: dead signals as of 08/26/03 patch - cleanup later
   void playerItem(const playerItemStruct* itemp, uint32_t, uint8_t);
   void playerBook(const playerBookStruct* bookp, uint32_t, uint8_t);
   void playerContainer(const playerContainerStruct* containp, uint32_t, uint8_t);
#endif
   void inspectData(const inspectDataStruct* inspt, uint32_t, uint8_t);
   void spMessage(const spMesgStruct* spmsg, uint32_t, uint8_t);
   void handleSpell(const memSpellStruct* mem, uint32_t, uint8_t);
   void beginCast(const beginCastStruct* bcast, uint32_t, uint8_t);
   void startCast(const startCastStruct* cast, uint32_t, uint8_t);
   void spellFaded(const spellFadedStruct* cast, uint32_t, uint8_t);
   void systemMessage(const sysMsgStruct* smsg, uint32_t, uint8_t);
   void moneyUpdate(const moneyUpdateStruct* money, uint32_t, uint8_t);
   void moneyThing(const moneyThingStruct* money, uint32_t, uint8_t);
   void groupInfo(const groupMemberStruct* gmem, uint32_t, uint8_t);
   void groupInvite(const groupInviteStruct* gmem, uint32_t, uint8_t);
   void groupDecline(const groupDeclineStruct* gmem, uint32_t, uint8_t);
   void groupAccept(const groupAcceptStruct* gmem, uint32_t, uint8_t);
   void groupDelete(const groupDeleteStruct* gmem, uint32_t, uint8_t);
   void zoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t, uint8_t);
   void zoneEntry(const ServerZoneEntryStruct* zsentry, uint32_t, uint8_t);
   void zoneEntry(const ClientZoneEntryStruct* zsentry, uint32_t, uint8_t);
   void zoneChange(const zoneChangeStruct* zoneChange, uint32_t, uint8_t);
   void zoneNew(const newZoneStruct* zoneNew, uint32_t, uint8_t);
   void summonedItem(const summonedItemStruct*, uint32_t, uint8_t);
   void summonedContainer(const summonedContainerStruct*, uint32_t, uint8_t);
   void logOut(const uint8_t*, uint32_t, uint8_t);
   void msgReceived(const QString &);
   void stsMessage(const QString &, int = 0);

   void backfillSpawn(const newSpawnStruct *, uint32_t, uint8_t);
   void backfillZoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t);
 
   // Spell signals
   void interruptSpellCast(const badCastStruct *, uint32_t, uint8_t);

   // forwarded signals
   void keyChanged(void);

   // other signals
   void zoneServerInfo(const uint8_t*, uint32_t, uint8_t);
   void playerItems(const playerItemsStruct *, uint32_t, uint8_t);
   void bookText(const bookTextStruct*, uint32_t, uint8_t);
   void doorOpen(const uint8_t*, uint32_t, uint8_t);
   void illusion(const uint8_t*, uint32_t, uint8_t);
   void castOn(const castOnStruct*, uint32_t, uint8_t);
   void openVendor(const uint8_t*, uint32_t, uint8_t);
   void closeVendor(const uint8_t*, uint32_t, uint8_t);
   void openGM(const uint8_t*, uint32_t, uint8_t);
   void closeGM(const uint8_t*, uint32_t, uint8_t);
   void spawnAppearance(const spawnAppearanceStruct*, uint32_t, uint8_t);
   void newGuildInZone(const newGuildInZoneStruct*, uint32_t, uint8_t);
   void bindWound(const bindWoundStruct*, uint32_t, uint8_t);
   void unknownOpcode(const uint8_t*, uint32_t, uint8_t);

   void worldGuildList(const char*, uint32_t);
   void updateNpcHP(const hpNpcUpdateStruct* hpupdate, uint32_t, uint8_t);

 private:
      
   PacketCaptureThread *m_packetCapture;
   VPacket             *m_vPacket;
   QString print_addr   (in_addr_t addr);
   
   QTimer         *m_timer;
   int            m_packetCount[MAXSTREAMS];
   uint16_t       m_serverPort;
   uint16_t       m_clientPort;
   bool           m_busy_decoding;
   bool           m_viewUnknownData;
   bool           m_detectingClient;
   uint16_t       m_serverArqSeqGiveUp;
   uint32_t       m_client_addr;
   uint8_t        m_session_tracking_enabled;

   EQStreamID     m_eqstreamid;
   uint8_t        m_eqstreamdir;
   
   EQPacketMap*   m_serverCache[MAXSTREAMS];
   
   uint16_t       m_serverArqSeqExp[MAXSTREAMS];
   bool           m_serverArqSeqFound[MAXSTREAMS];

   uint8_t *m_fragmentData[MAXSTREAMS];
   uint16_t m_fragmentDataSize[MAXSTREAMS];
   uint16_t m_fragmentDataAllocSize[MAXSTREAMS];
   uint16_t m_fragmentSeq[MAXSTREAMS];
   uint16_t m_fragmentCur[MAXSTREAMS];


   void resetEQPacket();
   void setCache(uint16_t serverArqSeq, EQUDPIPPacketFormat& packet);
   void processCache();
   void resetCache(int);

   struct eqTimeOfDay m_eqTime;

   int  getEQTimeOfDay (time_t timeConvert, struct timeOfDayStruct *eqTimeOfDay);
   void dispatchPacket   (int size, unsigned char *buffer);
   void dispatchSplitData (EQPacketFormat& pf, uint8_t dir, EQStreamID streamid);
   void dispatchZoneData  (uint32_t len, uint8_t* data, uint8_t direction = 0);
   void dispatchWorldData (uint32_t len, uint8_t* data, uint8_t direction = 0);
   void dispatchWorldChatData (uint32_t len, uint8_t* data, uint8_t direction = 0);
   void logRawData (const char   *filename, unsigned char *data, unsigned int len);
   void printUnknown(unsigned int uiOpCodeIndex, uint16_t opCode, uint32_t len, 
		     uint8_t *data, uint8_t dir);

};

inline int EQPacket::packetCount(int stream)
{
  return m_packetCount[stream];
}

inline uint32_t EQPacket::clientAddr(void)
{
   return m_client_addr;
}

inline uint16_t EQPacket::clientPort(void)
{
  return m_clientPort;
}

inline uint16_t EQPacket::serverPort(void)
{
  return m_serverPort;
}

inline uint8_t EQPacket::session_tracking_enabled(void)
{
  return m_session_tracking_enabled;
}

inline int EQPacket::currentCacheSize(int stream)
{
  return m_serverCache[stream]->size();
}

inline uint16_t EQPacket::serverSeqExp(int stream)
{
  return m_serverArqSeqExp[stream];
}

//----------------------------------------------------------------------
// PacketCaptureThread
class PacketCaptureThread
{
 public:
         PacketCaptureThread();
         ~PacketCaptureThread();
         void start (const char *device, const char *host, bool realtime, uint8_t address_type);
	 void stop ();
         uint16_t getPacket (unsigned char *buff); 
         void setFilter (const char *device, const char *hostname, bool realtime,
                        uint8_t address_type, uint16_t zone_server_port, uint16_t client_port);
	 const QString getFilter();
         
 private:
         static void* loop(void *param);
         static void packetCallBack(u_char * param, const struct pcap_pkthdr *ph, const u_char *data);

         struct packetCache 
	 {
           struct packetCache *next;
           ssize_t len;
           unsigned char data[0];
         };
         struct packetCache *m_pcache_first;
         struct packetCache *m_pcache_last;

	 pthread_t m_tid;
         pthread_mutex_t m_pcache_mutex;

         pcap_t *m_pcache_pcap;

	 QString m_pcapFilter;
};

#endif // EQPACKET_H
