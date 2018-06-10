/*
 * group.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#include "group.h"
#include "player.h"
#include "spawn.h"
#include "spawnshell.h"
#include "everquest.h"
#include "diagnosticmessages.h"
#include "main.h"

//#define DEBUG_GROUPMGR

GroupMgr::GroupMgr(SpawnShell* spawnShell, 
		   Player* player,  
		   QObject* parent, const char* name)
  : QObject( parent, name),
    m_spawnShell(spawnShell),
    m_player(player),
    m_memberCount(0),
    m_membersInZoneCount(0)
{
  m_pvpExpBonusIsActive =
    pSEQPrefs->getPrefBool("PvPExpBonus", "Experience", false);

  for (int i=0; i<MAX_GROUP_MEMBERS; i++)
  {
    m_members[i] = new GroupMember();
  }

  // clear the array of members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
    m_members[i]->m_spawn = 0;
    m_members[i]->m_level = 0;
  }
#ifdef DEBUG_GROUPMGR
  seqDebug("m_membersInZoneCount init %d", m_membersInZoneCount);
  seqDebug("m_memberCount init %d", m_memberCount);
#endif
}

GroupMgr::~GroupMgr()
{
  for (int i=0; i<MAX_GROUP_MEMBERS; i++)
  {
    delete m_members[i];
  }
}

void GroupMgr::player(const uint8_t* data)
{
  const charProfileStruct* player = (const charProfileStruct*)data;

  // reset counters
  m_memberCount = 0;
  m_membersInZoneCount = 0;
#ifdef DEBUG_GROUPMGR
  seqDebug("m_membersInZoneCount player packet, reset to %d", m_membersInZoneCount);
  seqDebug("m_memberCount player packet, reset to %d", m_memberCount);
#endif

  emit cleared();

  // initialize the array of members with information from the player profile
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    m_members[i]->m_name = player->groupMembers[i];
    m_members[i]->m_level = 0;

    if (!m_members[i]->m_name.isEmpty())
    {
      m_memberCount++;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_memberCount player packet, +1 to %d", m_memberCount);
#endif
    }

    if (m_members[i]->m_name != player->name)
      m_members[i]->m_spawn = 0;
    else
    {
      m_members[i]->m_spawn = (const Spawn*)m_player;
      if (m_player) {
          m_members[i]->m_level = m_player->level();
      }
      
      m_membersInZoneCount++;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount player packet, +1 to %d", m_membersInZoneCount);
#endif
    }

    emit added(m_members[i]->m_name, m_members[i]->m_spawn);
  }
}
 
void GroupMgr::groupUpdate(const uint8_t* data, size_t size)
{
  const groupUpdateStruct* gupdate = 0;
  const groupFullUpdateStruct* gfupdate = 0;

  if (size == sizeof(groupFullUpdateStruct))
  {
    // got a full group update packet
    gfupdate = (const groupFullUpdateStruct*)data;

    // what action is this?
    if (gfupdate->action == GUA_FullGroupInfo)
    {
      // reset counters
      m_memberCount = 0;
      m_membersInZoneCount = 0;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount FullGroupInfo reset to %d", m_membersInZoneCount);
      seqDebug("m_memberCount FullGroupInfo, reset to %d", m_memberCount);
#endif

      emit cleared();

      // ok, this is a full update of all group member information
      for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
      {
	// copy the member name
	m_members[i]->m_name = gfupdate->membernames[i];
    m_members[i]->m_level = 0;

	// if their is a member, increment the member count
	if (!m_members[i]->m_name.isEmpty()) 
    {
	  m_memberCount++;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_memberCount FullGroupInfo, +1 to %d", m_memberCount);
#endif
    }

	// attempt to retrieve the members spawn
	m_members[i]->m_spawn = 
	  m_spawnShell->findSpawnByName(m_members[i]->m_name);

	// incremement the spawn count
	if (m_members[i]->m_spawn) {
	  m_membersInZoneCount++;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount FullGroupInfo +1 to %d", m_membersInZoneCount);
#endif
      m_members[i]->m_level = m_members[i]->m_spawn->level();
    }

	emit added(m_members[i]->m_name, m_members[i]->m_spawn);
      }
    }
  }
  else
  {
    gupdate = (const groupUpdateStruct*)data;

    switch (gupdate->action)
    {
    case GUA_Started:
    case GUA_Joined:
      // iterate over all the slots until an empty one is found
      for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
      {
	if (m_members[i]->m_name.isEmpty())
	{
	  // copy the member name
	  m_members[i]->m_name = gupdate->membername;
      m_members[i]->m_level = 0;
	  
	  // if their is a member, increment the member count
	  if (!m_members[i]->m_name.isEmpty()) 
      {
	    m_memberCount++;
#ifdef DEBUG_GROUPMGR
        seqDebug("m_memberCount GUA_Joined, +1 to %d", m_memberCount);
#endif
      }
	  
	  // attempt to retrieve the members spawn
	  m_members[i]->m_spawn = 
	    m_spawnShell->findSpawnByName(m_members[i]->m_name);
	  
	  // incremement the spawn count
	  if (m_members[i]->m_spawn) {
	    m_membersInZoneCount++;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount GUA_Joined, +1 to %d", m_membersInZoneCount);
#endif
        m_members[i]->m_level = m_members[i]->m_spawn->level();
      }

	  // signal the addition
	  emit added(m_members[i]->m_name, m_members[i]->m_spawn);

	  // added it, so break
	  break;
	}	  
      }
      break;
    case GUA_Left:
      // Player was removed from group
      if (gupdate->membername == m_player->name())
      {
          m_memberCount = 0;
          m_membersInZoneCount = 0;
#ifdef DEBUG_GROUPMGR
          seqDebug("m_membersInZoneCount GUA_Left, reset to %d", m_membersInZoneCount);
          seqDebug("m_memberCount GUA_Left, reset to %d", m_memberCount);
#endif

          // iterate over all the member slots and clear them
          for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
          {
              m_members[i]->m_name = "";
              m_members[i]->m_spawn = 0;
              m_members[i]->m_level = 0;
          }
          emit cleared();
      }
      else
      {

      // iterate over all the slots until the member is found
      for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
      {
	// is this the member?
	if (m_members[i]->m_name == gupdate->membername)
	{
	  // yes, announce its removal
	  emit removed(m_members[i]->m_name, m_members[i]->m_spawn);

      m_memberCount--;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_memberCount GUA_Left, -1 to %d", m_memberCount);
#endif
      if (m_members[i]->m_spawn)
      {
          m_membersInZoneCount--;
#ifdef DEBUG_GROUPMGR
          seqDebug("m_membersInZoneCount GUA_Left, -1 to %d", m_membersInZoneCount);
#endif
      }

	  // clear it
	  m_members[i]->m_name = "";
	  m_members[i]->m_spawn = 0;
      m_members[i]->m_level = 0;
	  break;
	}
      }
      }
      break;
    case GUA_LastLeft:
      // reset counters
      m_memberCount = 0;
      m_membersInZoneCount = 0;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount GUA_LastLeft, reset to %d", m_membersInZoneCount);
      seqDebug("m_memberCount GUA_LastLeft, reset to %d", m_memberCount);
#endif

      // iterate over all the member slots and clear them
      for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
      {
	// clear the member
	m_members[i]->m_name = "";
	m_members[i]->m_spawn = 0;
    m_members[i]->m_level = 0;
      }

      emit cleared();
      break;
    }
  }
}

void GroupMgr::addItem(const Item* item)
{
  // only care about Spawn class
  if ((item->type() != tSpawn))
    return;

  const Spawn* spawn = (const Spawn*)item;

  if (!spawn->isOtherPlayer())
    return;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // is this spawn a group member?
    if (m_members[i]->m_name == spawn->name())
    {
      // yes, so note its Spawn object
      m_members[i]->m_spawn = spawn;
      m_members[i]->m_level = spawn->level();

      // decrement member in zone count
      m_membersInZoneCount++;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount addItem, +1 to %d", m_membersInZoneCount);
#endif

      break;
    }
  }
}

void GroupMgr::delItem(const Item* item)
{
  if ((item->type() != tSpawn))
    return;

  const Spawn* spawn = (const Spawn*)item;

  if (!spawn->isOtherPlayer())
    return;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // is this spawn a group member?
    if (m_members[i]->m_name == spawn->name())
    {
      // yes, so clear its Spawn object
      m_members[i]->m_spawn = 0;
      m_members[i]->m_level = 0;

      // decrement member in zone count
      m_membersInZoneCount--;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount delItem, -1 to %d", m_membersInZoneCount);
#endif

      break;
    }
  }
}

void GroupMgr::killingSpawn(const Item* item)
{
  if ((item->type() != tSpawn))
    return;

  const Spawn* spawn = (const Spawn*)item;

  if (!spawn->isOtherPlayer())
    return;

  // iterate over the group members
  for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
  {
    // is this spawn a group member?
    if (m_members[i]->m_name == spawn->name())
    {
      // yes, so clear its Spawn object
      m_members[i]->m_spawn = 0;
      m_members[i]->m_level = 0;

      // decrement members in zone count
      m_membersInZoneCount--;
#ifdef DEBUG_GROUPMGR
      seqDebug("m_membersInZoneCount killSpawn, -1 to %d", m_membersInZoneCount);
#endif

      break;
    }
  }
}

void GroupMgr::changeItem(const Item* item, uint32_t changeType)
{
    if (item->type() != tSpawn && item->type() != tPlayer)
        return;

    const Spawn* spawn = (const Spawn*)item;
    if (!spawn->isPlayer())
        return;

    // tSpawnChangedAll is effectively a new spawn
    if (changeType == tSpawnChangedALL && spawn->isOtherPlayer())
    {
        for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
        {
            if (m_members[i]->m_name == spawn->name())
            {
                m_members[i]->m_spawn = spawn;
                m_members[i]->m_level = spawn->level();

                m_membersInZoneCount++;
#ifdef DEBUG_GROUPMGR
                seqDebug("m_membersInZoneCount changeItem, +1 to %d", m_membersInZoneCount);
#endif
                break;
            }
        }
    }
    else if (changeType & tSpawnChangedLevel)
    {
        for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
        {
            if (m_members[i]->m_spawn && m_members[i]->m_spawn == item)
            {
                if (m_members[i]->m_name == spawn->name())
                {
                    m_members[i]->m_level = spawn->level();
                    break;
                }
            }
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

    out << "Member (" << i << "): " << m_members[i]->m_name << " lvl:" << m_members[i]->m_level;

    if (m_members[i]->m_spawn)
      out << " level " << m_members[i]->m_spawn->level()
	  << " " << m_members[i]->m_spawn->raceString()
	  << " " << m_members[i]->m_spawn->classString();

    out << endl;
  }  
}

float GroupMgr::groupExpShare()
{
    return m_membersInZoneCount ? 1.0/m_membersInZoneCount : 1.0;
}

void GroupMgr::setUsePvPExpBonus(bool enable)
{
  m_pvpExpBonusIsActive = enable;
  pSEQPrefs->setPrefBool("PvPExpBonus", "Experience", enable);
}

float GroupMgr::groupBonus()
{
    if (m_pvpExpBonusIsActive)
    {
        switch (groupMembersInZoneCount())
        {
            case 2:     return 2.72882653061;
            case 3:     return 3.2335227; //3.2982809;?
            case 4:     return 3.6244897959;
            case 5:     return 4.12767379679;
            case 6:     return 4.73195016269;
            default:    return 1.50412;
        }
    }
    else
    {
        switch (groupMembersInZoneCount())
        {
            case 2:	return 1.00;
            case 3:	return 1.042;
            case 4:	return 1.051;
            case 5:	return 1.08203;
            case 6:	return 1.1025;
            default:	return 1.00;
        }
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
      total += m_members[i]->m_level;
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

const uint8_t GroupMgr::memberLevelBySlot(uint16_t slot )
{
  // validate slot value
  if (slot >= MAX_GROUP_MEMBERS)
    return 0;

  // return the spawn object associated with the group slot, if any
  return m_members[slot]->m_level;
}

#include "group.moc"
