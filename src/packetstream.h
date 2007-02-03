/*
 * packetstream.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _PACKETSTREAM_H_
#define _PACKETSTREAM_H_

#include <qobject.h>
#include <map>

#include "packetcommon.h"
#include "packetfragment.h"
#include "packetinfo.h"

#if (defined(__FreeBSD__) || defined(__linux__)) && defined(__GLIBC__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ < 2)
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#endif

#include <netinet/in.h>

class EQUDPIPPacketFormat;
class EQProtocolPacket;
class EQPacketOPCodeDB;
class EQPacketOPCode;

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
// is that after a matching arq is found in the map,  finding/checking 
// for the next expected arq in the map only requires moving the iterator 
// forward (using operator++()) once and checking if the next key in the list
// is the expected arq.  This results in the check for followers to only 
// taking amortized constant time (as opposed to the O(log N) of map::find()
// or constant average time of the hash find methods.
typedef std::map<uint16_t, EQProtocolPacket* > EQPacketMap;

//----------------------------------------------------------------------
// EQPacketStream
class EQPacketStream : public QObject
{
  Q_OBJECT

 public:
  EQPacketStream(EQStreamID streamid, uint8_t dir, uint16_t m_arqSeqGiveUp,
		 EQPacketOPCodeDB& opcodeDB, 
		 QObject* parent = 0, const char* name = 0);
  ~EQPacketStream();
  void reset();
  uint8_t sessionTracking();
  void setSessionTracking(uint8_t);
  uint16_t arqSeqGiveUp();
  void setArqSeqGiveUp(uint16_t);
  int packetCount(void);
  uint8_t dir();
  EQStreamID streamID();
  size_t currentCacheSize();
  uint16_t arqSeqExp();
  bool connect2(const QString& opcodeName, 
		const char* payload,  EQSizeCheckType szt, 
		const QObject* receiver, const char* member);
  void receiveSessionKey(uint32_t sessionId, EQStreamID streamid, 
    uint32_t sessionKey);
  void close(uint32_t sessionId, EQStreamID streamid, uint8_t sessionTracking);
  uint16_t calculateCRC(EQProtocolPacket& packet);
  uint32_t getSessionKey() const { return m_sessionKey; }
  uint32_t getMaxLength() const { return m_maxLength; }
  
 public slots:
  void handlePacket(EQUDPIPPacketFormat& pf);
  
 signals:
  void rawPacket(const uint8_t* data, size_t len, uint8_t dir, 
		 uint16_t opcode);
  void decodedPacket(const uint8_t* data, size_t len, uint8_t dir,
		     uint16_t opcode, const EQPacketOPCode* opcodeEntry);
  void decodedPacket(const uint8_t* data, size_t len, uint8_t dir,
		     uint16_t opcode, const EQPacketOPCode* opcodeEntry,
		     bool unknown);

  // this signals stream closure
  void closing(uint32_t sessionId, EQStreamID streamId);

  // this signals a change in the session tracking state
  void sessionTrackingChanged(uint8_t);
  void lockOnClient(in_port_t serverPort, in_port_t clientPort);

  // Signal a new session key being received
  void sessionKey(uint32_t sessionId, EQStreamID streadid, uint32_t sessionKey);
		    
  // used for net_stats display
  void cacheSize(int, int);
  void seqReceive(int, int);
  void seqExpect(int, int);
  void numPacket(int, int);
  void resetPacket(int, int);
  void maxLength(int, int);

 protected:
  void resetCache();
  void setCache(uint16_t serverArqSeq, EQProtocolPacket& packet);
  void processCache();
  void processPacket(EQProtocolPacket& packet, bool subpacket);
  void dispatchPacket(const uint8_t* data, size_t len,
		      uint16_t opCode, const EQPacketOPCode* opcodeEntry);


  EQPacketOPCodeDB& m_opcodeDB;
  Q3PtrDict<EQPacketDispatch> m_dispatchers;
  EQStreamID m_streamid;
  uint8_t m_dir;
  int m_packetCount;
  uint8_t m_session_tracking_enabled;

  // ARQ cache handling
  EQPacketMap m_cache;
  size_t m_maxCacheCount;
  uint16_t m_arqSeqExp;
  uint16_t m_arqSeqGiveUp;
  bool m_arqSeqFound;
  
  // Fragment handling
  EQPacketFragmentSequence m_fragment;

  // Session info
  uint32_t m_sessionId;
  uint32_t m_sessionKey;
  in_port_t m_sessionClientPort;
  uint32_t m_maxLength;

  // encryption
  int64_t m_decodeKey;
  bool m_validKey;
};

inline uint8_t EQPacketStream::sessionTracking()
{
  return m_session_tracking_enabled;
}

inline void EQPacketStream::setSessionTracking(uint8_t val)
{
  m_session_tracking_enabled = val;
}

inline uint16_t EQPacketStream::arqSeqGiveUp()
{
  return m_arqSeqGiveUp;
}

inline void EQPacketStream::setArqSeqGiveUp(uint16_t val)
{
  m_arqSeqGiveUp = val;
}

inline int EQPacketStream::packetCount(void)
{
  return m_packetCount;
}

inline uint8_t EQPacketStream::dir()
{
  return m_dir;
}

inline EQStreamID EQPacketStream::streamID()
{
  return m_streamid;
}

inline size_t EQPacketStream::currentCacheSize()
{
  return m_cache.size();
}

inline uint16_t EQPacketStream::arqSeqExp()
{
  return m_arqSeqExp;
}

#endif //  _PACKETSTREAM_H_


