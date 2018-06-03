/*
 * group.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#ifndef _GROUP_H_
#define _GROUP_H_

#include <stdint.h>

#include <qobject.h>
#include <qstring.h>
#include "everquest.h"


//----------------------------------------------------------------------
// forward declarations
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
  const uint8_t memberLevelBySlot( uint16_t slot );

  size_t groupSize() { return m_memberCount; }
  size_t groupMemberCount() { return m_memberCount; }
  size_t groupMembersInZoneCount() { return m_membersInZoneCount; }
  void setUsePvPExpBonus(bool enable);
  bool isPvPExpBonusActive() { return m_pvpExpBonusIsActive; }
  float groupBonus();
  float groupExpShare();
  
  unsigned long totalLevels();
  
 public slots:
  void player(const uint8_t* player); 
  void groupUpdate(const uint8_t* data, size_t size);
  void addItem(const Item* item);
  void delItem(const Item* item);
  void killSpawn(const Item* item);
  void changeItem(const Item* item, uint32_t changeType);

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
    uint8_t m_level;
    const Spawn* m_spawn;
  }* m_members[MAX_GROUP_MEMBERS];
  size_t m_memberCount;
  size_t m_membersInZoneCount;
  bool m_pvpExpBonusIsActive;
};

#endif // _GROUP_H_
