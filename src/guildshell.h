/*
 * guildshell.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2004 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#ifndef _GUILDSHELL_H_
#define _GUILDSHELL_H_

#include <stdint.h>
#include <time.h>

#include <qstring.h>
#include <qobject.h>
#include <qdict.h>

//----------------------------------------------------------------------
// forward declarations
class QTextStream;

class NetStream;
class ZoneMgr;

struct GuildMemberUpdate;

//----------------------------------------------------------------------
// GuildMember
class GuildMember
{
 public:
  GuildMember(NetStream& netStream);
  ~GuildMember();

  void update(const GuildMemberUpdate* gmu);

  const QString& name() const { return m_name; }
  uint8_t level() const { return m_level; }
  uint8_t classVal() const { return m_class; }
  QString classString() const;
  uint32_t guildRank() const { return m_guildRank; }
  const QString& guildRankString() const;
  time_t lastOn() const { return m_lastOn; }
  const QString& publicNote() const { return m_publicNote; }
  uint16_t zoneId() const { return m_zoneId; }
  uint16_t zoneInstance() const { return m_zoneInstance; }

 protected:
  QString m_name;
  uint8_t m_level;
  uint8_t m_class;
  uint32_t m_guildRank;
  time_t m_lastOn;
  QString m_publicNote;
  uint16_t m_zoneId;
  uint16_t m_zoneInstance;
};

//----------------------------------------------------------------------
// GuildMemberDict
typedef QDict<GuildMember> GuildMemberDict;
typedef QDictIterator<GuildMember> GuildMemberDictIterator;

//----------------------------------------------------------------------
// GuildShell
class GuildShell : public QObject
{
  Q_OBJECT
 public:
  GuildShell(ZoneMgr* zoneMgr, QObject* parent = 0, const char* name = 0);
  ~GuildShell();
  const GuildMemberDict& members() { return m_members; }
  size_t maxNameLength() { return m_maxNameLength; }

  void dumpMembers(QTextStream& out);
  
  QString zoneString(uint16_t zoneid) const;

 public slots:
  void guildMemberList(const uint8_t* data, size_t len);
  void guildMemberUpdate(const uint8_t* data, size_t len);

 signals:
  void cleared();
  void loaded();
  void added(const GuildMember* gm);
  void removed(const GuildMember* gm);
  void updated(const GuildMember* gm);

 protected:

  GuildMemberDict m_members;
  size_t m_maxNameLength;
  ZoneMgr* m_zoneMgr;
};

#endif // _GUILDSHELL_H_
