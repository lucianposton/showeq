/*
 * group.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 *  Copyright 2003-2007 by the respective ShowEQ Developers
 */

#include "group.h"
#include "player.h"
#include "spawn.h"
#include "spawnshell.h"
#include "everquest.h"
#include "diagnosticmessages.h"
#include "netstream.h"

GroupMgr::GroupMgr(SpawnShell* spawnShell, 
		   Player* player,  
		   QObject* parent, const char* name)
  : QObject( parent, name),
    m_spawnShell(spawnShell),
    m_player(player),
    m_memberCount(0),
    m_membersInZoneCount(0)
{
  for (int i=0; i<MAX_GROUP_MEMBERS; i++)
  {
    m_members[i] = new GroupMember();
  }

  // clear the array of members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
    m_members[i]->m_spawn = 0;
}

GroupMgr::~GroupMgr()
{
  for (int i=0; i<MAX_GROUP_MEMBERS; i++)
  {
    delete m_members[i];
  }
}

// 9/3/2008 - Not used. Group data is no longer sent in charProfile
#if 0
void GroupMgr::player(const charProfileStruct* player)
{
  // reset counters
  m_memberCount = 0;
  m_membersInZoneCount = 0;

  emit cleared();

  // initialize the array of members with information from the player profile
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
//     so it compiles
//     m_members[i]->m_name = player->groupMembers[i];

    if (!m_members[i]->m_name.isEmpty())
      m_memberCount++;

    if (m_members[i]->m_name != player->name)
      m_members[i]->m_spawn = 0;
    else
    {
      m_members[i]->m_spawn = (const Spawn*)m_player;

      m_membersInZoneCount++;
    }

    emit added(m_members[i]->m_name, m_members[i]->m_spawn);
  }
}
#endif

void GroupMgr::groupUpdate(const uint8_t* data, size_t size)
{
  // it's a variable-length packet depending on number of group members and length of names
  NetStream netStream(data, size);
  uint32_t memCount, memNumber, level;
  QString name;

  netStream.skipBytes(4);

  // number of group members
  memCount = netStream.readUInt32NC();

  // leader name
  name = netStream.readText();

  // reset counters
  m_memberCount = 0;
  m_membersInZoneCount = 0;

  emit cleared();

  // update group member information
  for(uint32_t i = 0; i < memCount; i++)
  {
     memNumber = netStream.readUInt32NC();
     name = netStream.readText();
     netStream.skipBytes(3);
     level = netStream.readUInt32NC();

     // copy the member name
     m_members[i]->m_name = name;

     // increment the member count
     m_memberCount++;

     // attempt to retrieve the members spawn
     m_members[i]->m_spawn = m_spawnShell->findSpawnByName(m_members[i]->m_name);

     // incremement the spawn count
     if (m_members[i]->m_spawn)
        m_membersInZoneCount++;

     emit added(m_members[i]->m_name, m_members[i]->m_spawn);
  }

  // clear the rest
  for(uint32_t i = memCount; i < MAX_GROUP_MEMBERS; i++)
  {
     m_members[i]->m_name = "";
     m_members[i]->m_spawn = 0;
  }

// for debugging
#if 0
  for(uint32_t i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
     if(!m_members[i]->m_name.isEmpty())
        seqDebug("GroupMgr::groupUpdate '%s'", m_members[i]->m_name.latin1());
  }
#endif
}

void GroupMgr::addGroupMember(const uint8_t* data)
{
   const groupFollowStruct* gmem = (const groupFollowStruct*)data;

   for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
   {
      if (m_members[i]->m_name.isEmpty())
      {
	 // copy the member name
         m_members[i]->m_name = gmem->invitee;

	 // if there is a member, increment the member count
         if (!m_members[i]->m_name.isEmpty()) 
            m_memberCount++;

	 // attempt to retrieve the member's spawn
         m_members[i]->m_spawn = m_spawnShell->findSpawnByName(m_members[i]->m_name);

	 // incremement the spawn count
         if (m_members[i]->m_spawn)
            m_membersInZoneCount++;

	 // signal the addition
         emit added(m_members[i]->m_name, m_members[i]->m_spawn);

	 // added it, so break
         break;
      }
   }
}

void GroupMgr::removeGroupMember(const uint8_t* data)
{
   const groupDisbandStruct* gmem = (const groupDisbandStruct*)data;

   for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
   {
      // is this the member?
      if (m_members[i]->m_name == gmem->membername)
      {
         // yes, announce its removal
         emit removed(m_members[i]->m_name, m_members[i]->m_spawn);

         // decrement member count
         m_memberCount--;

         // if the member is in zone decrement zone count
         m_members[i]->m_spawn = m_spawnShell->findSpawnByName(m_members[i]->m_name);
         if(m_members[i]->m_spawn)
            m_membersInZoneCount--;

         // clear it
         m_members[i]->m_name = "";
         m_members[i]->m_spawn = 0;
         break;
      }
   }
}

void GroupMgr::addItem(const Item* item)
{
  // only care about Spawn class
  if ((item->type() != tSpawn) && (item->type() != tPlayer))
    return;

  const Spawn* spawn = (const Spawn*)item;

  // only care about players
  if (!spawn->isPlayer())
    return;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // is this spawn a group member?
    if (m_members[i]->m_name == spawn->name())
    {
      // yes, so note its Spawn object
      m_members[i]->m_spawn = spawn;

      // decrement member in zone count
      m_membersInZoneCount++;

      break;
    }
  }
}

void GroupMgr::delItem(const Item* item)
{
  if ((item->type() != tSpawn) && (item->type() != tPlayer))
    return;

  const Spawn* spawn = (const Spawn*)item;

  // only care about players
  if (!spawn->isPlayer())
    return;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // is this spawn a group member?
    if (m_members[i]->m_name == spawn->name())
    {
      // yes, so clear its Spawn object
      m_members[i]->m_spawn = 0;

      // decrement member in zone count
      m_membersInZoneCount--;

      break;
    }
  }
}

void GroupMgr::killSpawn(const Item* item)
{
  if ((item->type() != tSpawn) && (item->type() != tPlayer))
    return;

  const Spawn* spawn = (const Spawn*)item;

  // only care about players
  if (!spawn->isPlayer())
    return;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // is this spawn a group member?
    if (m_members[i]->m_name == spawn->name())
    {
      // yes, so clear its Spawn object
      m_members[i]->m_spawn = 0;

      // decrement members in zone count
      m_membersInZoneCount--;

      break;
    }
  }
}

void GroupMgr::dumpInfo(QTextStream& out)
{
  // dump general group manager information
  out << "[GroupMgr]" << endl;
  out << "Members: " << m_memberCount << endl;
  out << "MembersInZone: " << m_membersInZoneCount << endl;
  out << "Player: " << m_player->name() << endl;
  out << "GroupBonus: " << groupBonus() << endl;
  out << "GroupTotalLevels: " << totalLevels() << endl;
  out << "GroupAverageLevel: ";
  if (m_membersInZoneCount)
    out << totalLevels()/m_membersInZoneCount << endl;
  else
    out << totalLevels() << endl;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    if (m_members[i]->m_name.isEmpty())
      continue;

    out << "Member (" << i << "): " << m_members[i]->m_name;

    if (m_members[i]->m_spawn)
      out << " level " << m_members[i]->m_spawn->level()
	  << " " << m_members[i]->m_spawn->raceString()
	  << " " << m_members[i]->m_spawn->classString();

    out << endl;
  }  
}

float GroupMgr::groupBonus()
{
  switch (groupSize())
  {
  case 2:	return 1.02;
  case 3:	return 1.06;
  case 4:	return 1.10;
  case 5:	return 1.14;
  case 6:	return 1.20;
  default:	return 1.00;
  }
}

unsigned long GroupMgr::totalLevels()
{
  // if the player isn't in a group, just return their level
  if (m_memberCount == 0) 
    return m_player->level();

  unsigned long total = 0;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // add up the group member levels
    if (m_members[i]->m_spawn)
      total += m_members[i]->m_spawn->level();
  }

  // shouldn't happen, but just in-case
  if (total == 0)
    total = m_player->level();

  return total;
}

const Spawn* GroupMgr::memberByID(uint16_t id)
{
  // iterate over the members until a matching spawn id is found
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // if this member is in zone, and the spawnid matches, return it
    if (m_members[i]->m_spawn && (m_members[i]->m_spawn->id() == id))
      return m_members[i]->m_spawn;
  }

  // not found
  return 0;
}

const Spawn* GroupMgr::memberByName(const QString& name)
{
  // iterate over the members until a matching spawn name is found
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // if this member has the name, return its spawn
    if (m_members[i]->m_name == name)
      return m_members[i]->m_spawn;
  }
  
  // not found
  return 0;
}

const Spawn* GroupMgr::memberBySlot(uint16_t slot )
{
  // validate slot value
  if (slot >= MAX_GROUP_MEMBERS)
    return 0;

  // return the spawn object associated with the group slot, if any
  return m_members[slot]->m_spawn;
}

#ifndef QMAKEBUILD
#include "group.moc"
#endif

