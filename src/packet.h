/*
 * packet.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#ifndef EQPACKET_H
#define EQPACKET_H

#define MAXPACKETCACHECOUNT 256
#define MAXPACKETCACHESIZE  640
#define MAXSPAWNDATA        49152

#define DIR_CLIENT 1
#define DIR_SERVER 2


#include <stdint.h>

#ifdef __linux__
#include <endian.h>
#endif

#include <map>

#include <qqueue.h>
#include <qmap.h>
#include <qobject.h>
#include <qregexp.h>
#include "everquest.h"
#include "player.h"
#include "decode.h"
#include "opcodes.h"
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
/* Used in the packet capture filter setup.  If address_type is
   MAC_ADDRESS_TYPE, then we use the hostname string as a MAC address
   for the filter. cpphack
*/
#define DEFAULT_ADDRESS_TYPE   10   /* These were chosen arbitrarily */
#define IP_ADDRESS_TYPE        11 
#define MAC_ADDRESS_TYPE       12


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

// The various zone names
struct zoneName
{
   char shortName [30];
   char longName  [30];
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
    return ((uint8_t)m_flagsLo.m_skip + 
	    ((isARSP() || isSpecARQ()) ? 2 : 0));
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
// time consuming.  2) The nature of the data is such that the data 
// will have a tightly clustered set of keys, which is the worst case 
// performance scenario for hash based collections.  Causing most of the
// keys to end up in a small number of buckets.  This results in 
// near linear time access as opposed to the constant average time that is 
// typical for them on less clustered data sets. 3) To overcome problem
// 2 would require a custom hashing function, which could be calculated, 
// but is more work.  4) insertions into an STL map can be optimized 
// with the packet data's behavior using an iterator as a hint for 
// insertion location to typically yield amortized constant time behavior.
// 5) Another optimization possible with this data set using a mpa 
// is that after a matchintg arq is found in the map,  finding/checking 
// for the next expected arq in the map only requires moving the iterator 
// forward (using operator++()) once and checking if the next key in the list
// is the expected arq.  This results in the check for follers to only 
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
};

//----------------------------------------------------------------------
// EQPacket
class EQPacket : public QObject
{
   Q_OBJECT 
 public:
   
   EQPacket                   ( EQPlayer    *pplayer,
				QObject     *parent  = 0,
				const char *name    = 0
                                );
   ~EQPacket                    ();           
   void start                   (int delay = 0);
   void stop                    (void);
   void setLogAllPackets        (bool);
   void setLogZoneData          (bool);
   void setLogUnknownData       (bool);
   void setViewUnknownData      (bool);

 
 public slots:
   void processPackets     (void);
   void processPlaybackPackets (void);
   void incPlayback        (void);
   void decPlayback        (void);
   void monitorNextClient();   

   // Decoder slots
   void dispatchDecodedCharProfile(const uint8_t* decodedData, uint32_t len);
   void dispatchDecodedNewSpawn(const uint8_t* decodedData, uint32_t len);
   void dispatchDecodedZoneSpawns(const uint8_t* decodedData, uint32_t len);

 signals:
   void addGroup               ( char *,
                                 int
                               );
   
   void remGroup               ( char *,
                                 int
                               );
                               
   void clrGroup               (void);

   // used for net_stats display
   void cacheSize             (int);
   void seqReceive             (int);
   void seqExpect              (int);

   void numPacket              (int);

   void setPlayerName          (const QString&);
   void setPlayerDeity         (uint8_t);
   void playerUpdate           (const playerUpdateStruct* pupdate, bool client);
   void setPlayerID            (uint16_t);
   void updateLevel            (const levelUpStruct* levelup);
   void resetPlayer            (void);
   void wearItem               (const itemPlayerStruct* itemp);
   void updateExp              (const expUpdateStruct* exp);
   void increaseSkill          (const skillIncreaseStruct* skilli);
   void manaChange(const manaDecrementStruct* mana);
   void updateStamina(const staminaStruct* stam);

#if 1 // ZBTEMP: Currently not emit'd
   void attack1Hand1           (const attack1Struct*);
#endif
   void attack2Hand1           (const attack2Struct*);

   
   void consRequest            (const considerStruct*);
   void consMessage            (const considerStruct*);
   
   void clientTarget(const clientTargetStruct* target);
   void compressedDoorSpawn(const compressedDoorStruct* target);
   void spawnWearingUpdate     (const wearChangeStruct*);

   void newGroundItem          (const dropThingOnGround*, bool client);
   void removeGroundItem       (const removeThingOnGround*);
   void newCoinsItem           (const dropCoinsStruct*);
   void removeCoinsItem        (const removeCoinsStruct*);
   void newDoorSpawn           (const doorStruct*);

   void updateSpawns           (const spawnPositionUpdateStruct* updates);
   void updateSpawnHP          (const spawnHpUpdateStruct* hpupdate);

   void refreshMap             (void);
   void newSpawn               (const newSpawnStruct* spawn);
   void deleteSpawn            (const deleteSpawnStruct* delspawn);
   void killSpawn              (const spawnKilledStruct* deadspawn);
   void corpseLoc              (const corpseLocStruct*);
   void newZone                ( char *,
                                 char *,
				 bool
                               );
                               
   void zoneChanged            (void);
   void eqTimeChangedStr       (const QString &);

   void itemShop(const itemShopStruct* items);
   void moneyOnCorpse(const moneyOnCorpseStruct* money);
   void itemPlayerReceived(const itemReceivedPlayerStruct* itemc);
   void tradeItemOut(const tradeItemStruct* itemt);
   void tradeItemIn(const itemReceivedPlayerStruct* itemr);
   void channelMessage(const channelMessageStruct* cmsg, bool client);
   void random(const randomStruct* randr);
   void emoteText(const emoteTextStruct* emotetext);
   void playerBook(const bookPlayerStruct* bookp);
   void playerContainer(const containerPlayerStruct* containp);
   void inspectData(const inspectingStruct* inspt);
   void spMessage(const spMesgStruct* spmsg);
   void handleSpell(const spellCastStruct* mem, bool client);
   void beginCast(const beginCastStruct* bcast, bool client);
   void startCast(const castStruct* cast);
   void systemMessage(const systemMessageStruct* smsg);
   void moneyUpdate(const moneyUpdateStruct* money);
   void moneyThing(const moneyUpdateStruct* money);
   void groupInfo(const groupMemberStruct* gmem);
   void zoneSpawns(const zoneSpawnsStruct* zspawns, int len);
   void zoneEntry(const ServerZoneEntryStruct* zsentry);
   void zoneEntry(const ClientZoneEntryStruct* zsentry);
   void zoneChange(const zoneChangeStruct* zoneChange, bool client);
   void zoneNew(const newZoneStruct* zoneNew, bool client);
   void summonedItem(const summonedItemStruct*);
 
   void msgReceived            (const QString &);
   void stsMessage             (const QString &,
                                 int              = 0
                               );
                               
   void toggle_log_AllPackets  (void);
   void toggle_log_ZoneData    (void);
   void toggle_log_UnknownData (void);

   
   // Decoder signals
   void resetDecoder           (void);
   void backfillPlayer     (const playerProfileStruct *);
   void backfillSpawn      (const spawnStruct *);

   // Spell signals
   void interruptSpellCast(const interruptCastStruct *);
 
 public:

   QObject        *m_parent;

   QString shortZoneName() { return m_currentZoneName.shortName; }
   QString longZoneName() { return m_currentZoneName.longName; }
   EQPlayer *getplayer               (void);   
   long getclientaddr                (void);
   void      InitializeOpCodeMonitor (void);
   
   bool            m_bOpCodeMonitorInitialized;
   #define         OPCODE_SLOTS 10
   unsigned int    MonitoredOpCodeList      [OPCODE_SLOTS][3];
   QString         MonitoredOpCodeAliasList [OPCODE_SLOTS];
   

 private:
   /* The player object, keeps track player's level, race and class.
      Will soon track all player stats. */
      
   EQPlayer      *m_player;
   EQDecode      *m_decode;
   PacketCaptureThread *m_packetCapture;

   unsigned long  m_client_addr;

   struct        zoneName            m_currentZoneName;
                 
   QString print_addr   (in_addr_t addr);
   
   void  decodePacket   ( int            size,
                          unsigned char *buffer
                        );
   
   int   getEQTimeOfDay ( time_t                   timeConvert,
                          struct timeOfDayStruct *eqTimeOfDay
                        );

 public:
   bool logData ( const char*    filename,
		  uint32_t       len,
		  const uint8_t* data,
		  in_addr_t       saddr    = 0,
		  in_addr_t       daddr    = 0,
		  in_port_t       sport    = 0,
		  in_port_t      dport    = 0
		  );
               
               
 private:
   void logRawData        ( const char   *filename,
                            unsigned char *data,
                            unsigned int   len
                          );

   bool           m_busy_decoding;
   //   int            fd;
   //   unsigned long  packetcount;
   QTimer        *m_timer;
   
   
   void dispatchWorldData     ( uint32_t       len,
                                uint8_t*       data,
                                uint8_t        direction = 0
                              );

   void dispatchZoneData      ( uint32_t       len,
                                uint8_t*       data,
                                uint8_t        direction = 0
                              );


   void dispatchZoneSplitData ( EQPacketFormat& pf );


   bool           m_zoning;

   int            m_nPacket;

   int            m_serverAddr;
   int            m_serverPort;
   bool           m_serverArqSeqFound;
   uint16_t       m_serverArqSeqExp;
   uint16_t       m_serverArqSeqGiveUp;

   EQPacketMap    m_serverCache;

   unsigned char  m_serverData     [MAXSPAWNDATA];
   uint32_t            m_serverDataSize;
   
   bool           m_viewUnknownData;

   VPacket        *m_vPacket;
   
   struct eqTimeOfDay         m_eqTime;

   bool           m_detectingClient;
 public:
};

//----------------------------------------------------------------------
// PacketCaptureThread
class PacketCaptureThread
{
 public:
         PacketCaptureThread();
         ~PacketCaptureThread();
         void start (const char *device, const char *host, bool realtime, uint8_t address_type);
         uint16_t getPacket (unsigned char *buff); 
         void setFilter (const char *device, const char *hostname, bool realtime,
                        uint8_t address_type, uint16_t zone_server_port);
         
 private:
         static void* loop(void *param);
         static void packetCallBack(u_char * param, const struct pcap_pkthdr *ph, const u_char *data);

         struct packetCache 
	 {
           struct packetCache *next;
           ssize_t len;
           unsigned char data[0];
         };
	 pthread_t m_tid;
         pthread_mutex_t m_pcache_mutex;
         struct packetCache *m_pcache_first;
         struct packetCache *m_pcache_last;
         pcap_t *m_pcache_pcap;

};
#endif // EQPACKET_H
