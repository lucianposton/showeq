/*
 * group.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#ifndef _GROUP_H_
#define _GROUP_H_

#include <qstring.h>

#include "spawnshell.h"

//----------------------------------------------------------------------
// forward declarations
class Player;

class GroupMgr: public QObject
{
  Q_OBJECT
 public:
  GroupMgr(SpawnShell* spawnShell, 
	   Player* player,  
	   const char* name = NULL);
 
  const Spawn* memberByID( uint16_t id );
  const Spawn* memberByName( const QString& name );
  const Spawn* memberBySlot( uint16_t slot );

  int groupSize();
  int groupPercentBonus();
  
  unsigned long totalLevels();
  
 public slots:
  void handleGroupInfo(const groupMemberStruct* gmem);
  void delItem(const Item* item);

 signals:
  void addGroup( const Spawn* mem );
  void remGroup( const Spawn* mem );
  void clrGroup();
  
 protected:
  QList<Spawn> m_group;
  SpawnShell* m_spawnShell;
  Player* m_player;
};

#endif // _GROUP_H_
