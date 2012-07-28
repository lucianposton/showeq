/*
 * spawnlog.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2003-2004 by the respective ShowEQ Developers
 */

#ifndef _BAZAARLOG_H_
#define _BAZAARLOG_H_

#include <qobject.h>
#include "logger.h"

class EQPacket;
class SpawnShell;
 
//----------------------------------------------------------------------
// BazaarLog is an SEQLogger subclass that handles bazaar packets
class BazaarLog : public SEQLogger
{
  Q_OBJECT

 public:
  BazaarLog(EQPacket &packet, const QString& fname,
	    QObject* parent, SpawnShell& shell, const char* name = 0);
  virtual ~BazaarLog();

 public slots:
  void bazaarSearch(const uint8_t*, size_t, uint8_t);

 protected:
  EQPacket& m_packet;
  SpawnShell& m_shell;
};

#endif // _BAZAARLOG_H_
