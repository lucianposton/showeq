/*
 * decode.h
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#ifndef EQDECODE_H
#define EQDECODE_H

#include <stdint.h>
#include <pthread.h>
#include <deque>

#include <qobject.h>
#include <qevent.h>

#include "everquest.h"


#define PKTBUF_LEN 65535

typedef struct
{
  int len;
  unsigned char data[0];
} EQPktRec;

typedef std::deque<EQPktRec *> EQPktQueue;

class FoundKeyEvent : public QCustomEvent
{
 public:
  enum { Found = User + 2047 };

  FoundKeyEvent() : QCustomEvent(Found) {}
};

class EQDecode : public QObject
{
  Q_OBJECT 

 public:
  EQDecode (QObject *parent = 0, const char *name = 0);

 public slots:
  void ResetDecoder (void);
  void theKey(uint64_t);

 signals:
  void keyChanged (void);
  void dispatchDecodedCharProfile(const uint8_t* decodedData, uint32_t len);
  void dispatchDecodedNewSpawn(const uint8_t* decodedData, uint32_t len);
  void dispatchDecodedZoneSpawns(const uint8_t* decodedData, uint32_t len);
  void startDecodeBatch(void);
  void finishedDecodeBatch(void);

 public:
  int DecodePacket(const uint8_t *data, uint32_t len, 
		   uint8_t* decodedData, uint32_t* decodedDataLen,
		   const char *cli);
  void LocateKey ();
  void FoundKey ();
  void setHash(uint8_t*, uint32_t);

  // overloaded event member for syncronization
  virtual bool event(QEvent*);

  uint64_t decodeKey() { return m_decodeKey; }
 private:
  int InflatePacket(const uint8_t *pbDataIn, uint32_t cbDataInLen, 
		    uint8_t* pbDataOut, uint32_t* pcbDataOutLen);
  pthread_t m_locateThread;
  bool m_locateActive;
  pthread_mutex_t m_mutexQueue;
  EQPktQueue m_queuePlayerProfile;
  EQPktQueue m_queueZoneSpawns;
  EQPktQueue m_queueSpawns;
  uint64_t m_decodeKey;
  uint8_t m_decodeHash[PKTBUF_LEN];
  uint32_t m_decodeHlen;
};

#endif	// EQDECODE_H
