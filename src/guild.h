/*
 * guild.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#ifndef _GUILD_H_
#define _GUILD_H_

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>

#include "everquest.h"

//------------------------------
// forward declarations
class EQPacket;

class GuildMgr : public QObject
{
  Q_OBJECT

 public:

  GuildMgr(QString, EQPacket* packet, QObject* parent = 0, const char* name = 0);

  ~GuildMgr();

  QString guildIdToName(uint16_t);

 public slots:
  void worldGuildList(const char*, uint32_t);
  void readGuildList();
  void guildList2text(QString);
  void listGuildInfo();

 private:
  QMap<uint16_t,QString> m_guildMap;
 
  void writeGuildList(const char*, uint32_t);

  QString guildsFileName;

};

#endif // _GUILD_H_
