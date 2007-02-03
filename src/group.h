/*
 * group.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#ifndef _GROUP_H_
#define _GROUP_H_

#include <stdint.h>

#include <QObject>
#include <QString>

#include "everquest.h"


//----------------------------------------------------------------------
// forward declarations
class QTextStream;

class Player;
class SpawnShell;
class Item;
class Spawn;

class GroupMgr: public QObject
{
  Q_OBJECT
 public:
  GroupMgr(SpawnShell* spawnShell, 
	   Player* player,  
	   QObject* parent = 0, const char* name = 0);
  virtual ~GroupMgr();
 
  const Spawn* memberByID( uint16_t id );
  const Spawn* memberByName( const QString& name );
  const Spawn* memberBySlot( uint16_t slot );

  size_t groupSize() { return m_memberCount; }
  size_t groupMemberCount() { return m_memberCount; }
  size_t groupMembersInZoneCount() { return m_membersInZoneCount; }
  float groupBonus();
  
  unsigned long totalLevels();
  
 public slots:
  void player(const charProfileStruct* player); 
  void groupUpdate(const uint8_t* data, size_t size);
  void addItem(const Item* item);
  void delItem(const Item* item);
  void killSpawn(const Item* item);

  // dump debug info
  void dumpInfo(QTextStream& out);

 signals:
  void added(const QString& name, const Spawn* mem);
  void removed(const QString& name, const Spawn* mem);
  void cleared();
  
 protected:
  SpawnShell* m_spawnShell;
  Player* m_player;
  struct GroupMember
  {
    QString m_name;
    const Spawn* m_spawn;
  }* m_members[MAX_GROUP_MEMBERS];
  size_t m_memberCount;
  size_t m_membersInZoneCount;
};

#endif // _GROUP_H_
