/*
 * guildshell.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2004 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#include "guildshell.h"
#include "netstream.h"
#include "util.h"
#include "zonemgr.h"
#include "everquest.h"
#include "diagnosticmessages.h"

#include <qdatetime.h>

//----------------------------------------------------------------------
// diagnostic defines
// #define GUILDSHELL_DIAG 1

//----------------------------------------------------------------------
// constants
static const QString guildRanks[] = { "M", "O", "L", "?", };

//----------------------------------------------------------------------
// GuildMember implementation
GuildMember::GuildMember(NetStream& netStream)
{
  m_name = netStream.readText();
  m_level = uint8_t(netStream.readUInt32());
  m_class = uint8_t(netStream.readUInt32());
  m_guildRank = netStream.readUInt32();
  m_lastOn = time_t(netStream.readUInt32());
  m_publicNote = netStream.readText();
  m_zoneInstance = netStream.readUInt16();
  m_zoneId = netStream.readUInt16();
}

GuildMember::~GuildMember()
{
}

void GuildMember::update(const GuildMemberUpdate* gmu)
{
  if (gmu->type == 0xe3)
  {
    m_zoneId = gmu->zoneId;
    m_zoneInstance = gmu->zoneInstance;
    m_lastOn = gmu->lastOn;
  }
}

QString GuildMember::classString() const
{
  return ::classString(m_class);
}

const QString& GuildMember::guildRankString() const
{
  if (m_guildRank <= 2)
    return guildRanks[m_guildRank];
  else
    return guildRanks[3]; // return the unknown rank character
}

//----------------------------------------------------------------------
// GuildShell implementation
GuildShell::GuildShell(ZoneMgr* zoneMgr, QObject* parent, const char* name)
  : QObject(parent, name),
    m_maxNameLength(0),
    m_zoneMgr(zoneMgr)
{
  m_members.setAutoDelete(true);
}

GuildShell::~GuildShell()
{
}

QString GuildShell::zoneString(uint16_t zoneid) const
{
  if (zoneid == 0)
    return "";
   else if (zoneid == 65535)
    return "Anonymous";
  else
    return m_zoneMgr->zoneNameFromID(zoneid);
}

void GuildShell::dumpMembers(QTextStream& out)
{
  QDateTime dt;
  GuildMemberDictIterator it(m_members);
  GuildMember* member;

  QString format("%1 %2  %3 %4  %5  %6");
  QString dateFormat("ddd MMM dd hh:mm:ss yyyy");
  
  // calculate the maximum class name width
  size_t maxClassNameLength = 0;
  for (uint8_t i = 1; i <= PLAYER_CLASSES; i++)
    if (classString(i).length() > maxClassNameLength)
      maxClassNameLength = classString(i).length();
    
  out << "Guild has " << m_members.count() << " members: " << endl;

  int nameFieldWidth = - m_maxNameLength;
  int classFieldWidth = - maxClassNameLength;

  out << format.arg("Members", nameFieldWidth)
    .arg("Lv", 2).arg("Class", classFieldWidth)
    .arg("R", 1)
    .arg("Last On", -24)
    .arg("Zone", -18);
  out << " Public Note" << endl;

  QString zone;
  while ((member = it.current()))
  {
    dt.setTime_t(member->lastOn());
    zone = zoneString(member->zoneId());
    if (member->zoneInstance())
      zone += ":" + QString::number(member->zoneInstance());
    out << format.arg(member->name(), nameFieldWidth)
      .arg(member->level(), 2).arg(member->classString(), classFieldWidth)
      .arg(member->guildRankString(), 1)
      .arg(dt.toString(dateFormat), -24)
      .arg(zone, -18);

    out << " " << member->publicNote() << endl;
    ++it;
  }
}


void GuildShell::guildMemberList(const uint8_t* data, size_t len)
{
  // clear out any existing member data
  emit cleared();
  m_members.clear();

  m_maxNameLength = 0;

  // construct a netstream object on the data
  NetStream gml(data, len);
  
  // read the player name from the front of the stream
  QString player = gml.readText();

  // read the player count from the stream
  uint32_t count;
  count = gml.readUInt32();

#ifdef GUILDSHELL_DIAG
  seqDebug("Guild has %d members:", count);
#endif

  GuildMember* member;

#ifdef GUILDSHELL_DIAG
  QDateTime dt;
#endif // GUILDSHELL_DIAG

  // iterate over the data until we reach the end of it
  while (!gml.end())
  {
    // create a new guildmember initializing it from the NetStream
    member = new GuildMember(gml);

    // insert the new member into the dictionary
    m_members.insert(member->name(), member);
    
    // check for new longest member name
    if (member->name().length() > m_maxNameLength)
      m_maxNameLength = member->name().length();

    emit added(member);

#ifdef GUILDSHELL_DIAG
    dt.setTime_t(member->lastOn());
    seqDebug("%-64s\t%d\t%s\t%d\t%s\t'%s'\t%s:%d",
	     (const char*)member->name(),
	     member->level(),
	     (const char*)classString(member->classVal()),
	     member->guildRank(), 
	     (const char*)dt.toString(),
	     (const char*)member->publicNote(),
	     (const char*)m_zoneMgr->zoneNameFromID(member->zoneId()),
	     member->zoneInstance());
#endif	     
  }

  emit loaded();

#ifdef GUILDSHELL_DIAG
  seqDebug("Finished processing %d guildmates. %d chars in longest name.", 
	   m_members.count(), m_maxNameLength);
#endif // 
}

void GuildShell::guildMemberUpdate(const uint8_t* data, size_t len)
{
  const GuildMemberUpdate* gmu = (const GuildMemberUpdate*)data;

  QString memberName = QString::fromUtf8(gmu->name);

  // find the member
  GuildMember* member = m_members[memberName];

  // update the guild members information
  if (member)
  {
    member->update(gmu);
    emit updated(member);
#ifdef GUILDSHELL_DIAG
    QDateTime dt;
    dt.setTime_t(member->lastOn());
    seqDebug("%s is now in zone %s (lastOn: %s).",
	     (const char*)member->name(), 
	     (const char*)m_zoneMgr->zoneNameFromID(member->zoneId()),
	     (const char*)dt.toString());
#endif // GUILDSHELL_DIAG
  }
  else
  {
#ifdef GUILDSHELL_DIAG
    seqDebug("GuildShell::guildMemberUpdate(): Failed to find '%s'(%d)!",
	     (const char*)memberName, memberName.length());
#if GUILDSHELL_DIAG > 1
    seqDebug("%d in members dict.", m_members.count());
    GuildMemberDictIterator it(m_members); // See QDictIterator
    for( ; it.current(); ++it )
	seqDebug("'%s'(%d): '%s'(%d)", 
		 (const char*)it.currentKey(), it.currentKey().length(),
		 (const char*)it.current()->name(), it.current()->name().length());
#endif // GUILDSHELL_DIAG > 1
#endif // GUILDSHELL_DIAG
  }
}




