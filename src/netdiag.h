/*
 * netdiag.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 *
 */

#ifndef EQNETDIAG_H
#define EQNETDIAG_H

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>

#include "seqwindow.h"
#include "packet.h"

class NetDiag : public SEQWindow
{
  Q_OBJECT 
 public:
  NetDiag(EQPacket* packet, QWidget* parent, const char* name);
  ~NetDiag();

 public slots:
   void numPacket              (int, int);
   void resetPacket            (int, int);
   void clientChanged          (uint32_t);
   void clientPortLatched      (uint16_t);
   void serverPortLatched      (uint16_t);
   void sessionTrackingChanged (uint8_t);
   void seqReceive             (int, int);
   void seqExpect              (int, int);
   void keyChanged             (void);
   void cacheSize              (int, int);

 protected:
   QString print_addr(in_addr_t);

 private:
  EQPacket* m_packet;
  QSpinBox* m_playbackSpeed;
  QLabel* m_packetTotal[MAXSTREAMS];
  QLabel* m_packetRecent[MAXSTREAMS];
  QLabel* m_packetAvg[MAXSTREAMS];
  QLabel* m_seqExp[MAXSTREAMS];
  QLabel* m_seqCur[MAXSTREAMS];
  QLabel* m_clientLabel;
  QLabel* m_sessionLabel;
  QLabel* m_serverPortLabel;
  QLabel* m_clientPortLabel;
  QLabel* m_decodeKeyLabel;
  QLabel* cache[MAXSTREAMS];

  int  m_packetStartTime[MAXSTREAMS];
  int  m_initialcount[MAXSTREAMS];
};

#endif // EQNETDIAG_H
