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
#include <deque.h>

#include <qobject.h>
#include <qevent.h>

#include "everquest.h"

typedef struct
{
  int len;
  unsigned char data[0];
} EQPktRec;

typedef deque<EQPktRec *> EQPktQueue;

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

 signals:
  void keyChanged (void);
  void dispatchDecodedCharProfile(const uint8_t* decodedData, uint32_t len);
  void dispatchDecodedNewSpawn(const uint8_t* decodedData, uint32_t len);
  void dispatchDecodedZoneSpawns(const uint8_t* decodedData, uint32_t len);

 public:
  int DecodePacket(const uint8_t *data, uint32_t len, 
		   uint8_t* decodedData, uint32_t* decodedDataLen,
		   const char *cli);
  void LocateKey ();
  void FoundKey ();

  // overloaded event member for syncronization
  virtual bool event(QEvent*);

 private:
  int InflatePacket(const uint8_t *pbDataIn, uint32_t cbDataInLen, 
		    uint8_t* pbDataOut, uint32_t* pcbDataOutLen);
  pthread_t m_locateThread;
  bool m_locateActive;
  pthread_mutex_t m_mutexQueue;
  EQPktQueue m_queuePlayerProfile;
  EQPktQueue m_queueZoneSpawns;
  EQPktQueue m_queueSpawns;
  uint32_t m_decodeKey;
};

#endif	// EQDECODE_H
