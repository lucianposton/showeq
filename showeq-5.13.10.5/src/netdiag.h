/*
 * netdiag.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 *
 */

#ifndef EQNETDIAG_H
#define EQNETDIAG_H

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <netinet/in.h>

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>

#include "packetcommon.h"
#include "seqwindow.h"

//----------------------------------------------------------------------
// forward declarations
class EQPacket;

//----------------------------------------------------------------------
// NetDiag window class
class NetDiag : public SEQWindow
{
  Q_OBJECT 
 public:
  NetDiag(EQPacket* packet, QWidget* parent, const char* name);
  ~NetDiag();

 public slots:
   void numPacket              (int, int);
   void resetPacket            (int, int);
   void clientChanged          (in_addr_t);
   void clientPortLatched      (in_port_t);
   void serverPortLatched      (in_port_t);
   void sessionTrackingChanged (uint8_t);
   void filterChanged          ();
   void seqReceive             (int, int);
   void seqExpect              (int, int);
   void cacheSize              (int, int);
   void maxLength              (int, int);

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
  QLabel* m_cache[MAXSTREAMS];
  QLabel* m_maxLength[MAXSTREAMS];
  QLabel* m_filterLabel;

  int  m_packetStartTime[MAXSTREAMS];
  int  m_initialcount[MAXSTREAMS];
};

#endif // EQNETDIAG_H
