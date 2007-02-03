/*
 * packetlog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2003-2007 by the respective ShowEQ Developers
 */

#include <qdatetime.h>
#include <ctype.h>

#include "bazaarlog.h"
#include "spawnshell.h"
#include "diagnosticmessages.h"

BazaarLog::BazaarLog(EQPacket& packet, const QString& fname,
		     QObject* parent, SpawnShell& shell, const char* name)
  : SEQLogger(fname, parent, name),
    m_packet(packet),
    m_shell(shell)
{
}

BazaarLog::~BazaarLog()
{
}

void BazaarLog::bazaarSearch(const uint8_t* data, size_t len, uint8_t dir)
{
  if (!open()) return;

  if (len == 0 || data == 0 || len < sizeof(bazaarSearchResponseStruct))
    {
      seqWarn("Short / empty bazaar data passed to BazaarLog::bazaarSearch");
      return;
    }

  QString tmp;
  const struct bazaarSearchResponseStruct* r = (const bazaarSearchResponseStruct*) data;
  const size_t bsize = sizeof(bazaarSearchResponseStruct);
  const size_t msize = sizeof(r[0].mark);
  for(int i = 0;i*bsize+msize < len && r[i].mark == 7;i++)
    {
      const struct bazaarSearchResponseStruct& resp = r[i];

      // First copy and remove count from item name
      char name[256];
      // assert(255>sizeof(resp.item_name));
      strncpy(name,resp.item_name,sizeof(resp.item_name));
      char *p;
      if ((p = rindex(name,'(')) != NULL && isdigit(*(p+1)))
	*p=0;
      Item *merchant = m_shell.spawns().find(resp.player_id);
      const char *merchant_name = "unknown";
      if (merchant)
	merchant_name = merchant->name();
      QString csv;
      csv.sprintf("1^%d^%d^%d^%s^%s",
		  int(time(NULL)),resp.price,resp.count,
		  merchant_name, name);
      m_out << csv << endl;
    }

  flush();
}

#ifndef QMAKEBUILD
#include "bazaarlog.moc"
#endif


