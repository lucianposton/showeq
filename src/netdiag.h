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

#include "packet.h"

class NetDiag : public QFrame
{
  Q_OBJECT 
 public:
  NetDiag(EQPacket* packet, QWidget* parent, const char* name);
  ~NetDiag();

 public slots:
   void numPacket              (int);
   void resetPacket            (int);
   void clientChanged          (uint32_t);
   void clientPortLatched      (uint16_t);
   void serverPortLatched      (uint16_t);
   void sessionTrackingChanged (uint8_t);
   void seqReceive             (int);
   void seqExpect              (int);
   void keyChanged             (void);
   virtual void setCaption(const QString&);

 protected:
   QString print_addr(in_addr_t);

 private:
  EQPacket* m_packet;
  QSpinBox* m_playbackSpeed;
  QLabel* m_packetTotal;
  QLabel* m_packetRecent;
  QLabel* m_packetAvg;
  QLabel* m_seqExp;
  QLabel* m_seqCur;
  QLabel* m_clientLabel;
  QLabel* m_sessionLabel;
  QLabel* m_serverLabel;
  QLabel* m_decodeKeyLabel;

  int  m_packetStartTime;
  int  m_initialcount;
};

#endif // EQNETDIAG_H
