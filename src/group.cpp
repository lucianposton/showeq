/*
 * group.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#include "group.h"

GroupMgr::GroupMgr(SpawnShell* spawnShell, 
		   EQPlayer* player,  
		   const char* name)
  : QObject( NULL, name ),
    m_spawnShell(spawnShell),
    m_player(player)
{
  // just to be sure.
  m_group.setAutoDelete(false);
}
 
void GroupMgr::handleGroupInfo( const struct groupMemberStruct* gmem )
{
  //  return;
  QString newName = gmem->membername;
  const Spawn* member;

#ifdef DEBUG  
  printf("code: 1:%d 2:%d 3:%d Name:%s\n", 
	 gmem->bgARC, gmem->oper, gmem->ARC2, gmem->membername);
#endif

  // is an empty "memberName" really the signal to clear the group?
  if ( newName.isEmpty() )
  {
    printf( "Clearing out group.\n" );
    emit clrGroup();
    m_group.clear();
  }
  else
  {
    member = memberByName( newName );
    if ( member )
    {
      printf ("Removing: '%s' from group.\n", gmem->membername);
      emit remGroup( member );
      m_group.removeRef( member );
    }
    else
    {
      const Spawn* newMember = 
	m_spawnShell->findSpawnByRawName(newName);
      
      if ( newMember )
      {
	printf ("Adding: '%s' to group.\n", gmem->membername);
	m_group.append( newMember );
      }
      emit addGroup( newMember );
    }
  }
  
  printf( "Group Members are (%i):\n", m_group.count() );
  int count = 0;
  for ( member = m_group.first(); member != NULL; member = m_group.next() )
  {
    count++;
    printf( "Mem#%i: %s(%i)\n", 
	    count, (const char*)member->name(), member->id() );
  }
}

void GroupMgr::delItem(const Item* item)
{
  //  return;
  const Spawn* spawn = spawnType(item);

  if (spawn == NULL)
    return;

  const Spawn* member;
  for ( member = m_group.first(); member != NULL; member = m_group.next() )
  {
    if (member == spawn)
    {
      emit remGroup(member);
      m_group.removeRef(member);
    }
  }
}

int GroupMgr::groupSize()
{
  return m_group.count() + 1;
}

int GroupMgr::groupPercentBonus()
{
  switch ( groupSize() )
  {
  case 1:		return 100;
  case 2:		return 102;
  case 3:		return 106;
  case 4:		return 110;
  case 5:		return 114;
  case 6:		return 120;
  default:	return 100;
  }
}

unsigned long GroupMgr::totalLevels()
{
  unsigned long total = m_player->getPlayerLevel();
  
  const Spawn* member;
  for ( member = m_group.first(); member != NULL; member = m_group.next() )
    total += member->level();

  return total;
}

const Spawn* GroupMgr::memberByID(uint16_t id )
{
  const Spawn* member;
  for ( member = m_group.first(); member != NULL; member = m_group.next() )
  {
    if (member->id() == id)
      return member;
  }

  return NULL;
}

const Spawn* GroupMgr::memberByName(const QString& name)
{
  const Spawn* member;
  for ( member = m_group.first(); member != NULL; member = m_group.next() )
  {
    if (member->rawName() == name)
      return member;
  }

  return NULL;
}
