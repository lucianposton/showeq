/*
 * spawnshell.h
 *
 * ShowEQ Distributed under GPL
 * http://sourceforge.net/projects/seq/
 * 
 * Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

/*
 * Adapted from spawnlist.h - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date   - 7/31/2001
 */

// Major rework of SpawnShell{} classes - Zaphod (dohpaz@users.sourceforge.net)
//   Based on stuff from CJD's SpawnShell{} and stuff from SpawnList and
//   from Map.  Some optimization ideas adapted from SINS.

#ifndef SPAWNSHELL_H
#define SPAWNSHELL_H

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif
#include <stdio.h>
#include <math.h>

#include <qintdict.h>
#include <qtimer.h>
#include <qtextstream.h>

#include "everquest.h"
#include "spawn.h"

//----------------------------------------------------------------------
// forward declarations
class Player;
class ZoneMgr;
class FilterMgr;
class SpawnShell;
class EQItemDB;
class GuildMgr;

//----------------------------------------------------------------------
// constants

// The maximum number of ID's of deleted spawns to track
const int MAX_DEAD_SPAWNIDS = 50;

//----------------------------------------------------------------------
// enumerated types

//----------------------------------------------------------------------
// type definitions
typedef QIntDict<Item> ItemMap;
typedef QIntDictIterator<Item> ItemIterator;
typedef QIntDictIterator<Item> ItemConstIterator;

//----------------------------------------------------------------------
// SpawnShell
class SpawnShell : public QObject
{
   Q_OBJECT
public:
   SpawnShell(FilterMgr& filterMgr, 
	      ZoneMgr* zoneMgr, 
	      Player* player,
              GuildMgr* guildMgr);

   const Item* findID(spawnItemType type, int idSpawn);
   
   const Item* findClosestItem(spawnItemType type, 
			       int16_t x,
			       int16_t y, 
			       double& minDistance);
   Spawn* findSpawnByName(const QString& name);

   void dumpSpawns(spawnItemType type, QTextStream& out);
   FilterMgr* filterMgr(void) { return &m_filterMgr; }
   const ItemMap& getConstMap(spawnItemType type) const;
   const ItemMap& spawns(void) const;
   const ItemMap& drops(void) const;
   const ItemMap& doors(void) const;
   void toggleUseUpdateRadius(void);
   void setUseUpdateRadius(bool useUpdateRadius);
signals:
   void addItem(const Item* item);
   void delItem(const Item* item);
   void changeItem(const Item* item, uint32_t changeType);
   void killSpawn(const Item* deceased, const Item* killer, uint16_t killerId);
   void selectSpawn(const Item* item);
   void spawnConsidered(const Item* item);
   void clearItems();
   void numSpawns(int);

public slots: 
   void clear();

   // slots to receive from EQPacket...
   void newGroundItem(const uint8_t*, size_t, uint8_t);
   void removeGroundItem(const uint8_t*, size_t, uint8_t);
   void newDoorSpawns(const uint8_t*, size_t, uint8_t);
   void newDoorSpawn(const doorStruct&, size_t, uint8_t);
   void zoneSpawns(const uint8_t* zspawns, size_t len);
   void zoneEntry(const uint8_t* spawn, size_t len);
   void newSpawn(const uint8_t* spawn);
   void newSpawn(const spawnStruct& s);
   void playerUpdate(const uint8_t*pupdate, size_t, uint8_t);
   void npcMoveUpdate(const uint8_t*npcupdate, size_t, uint8_t);
   void updateSpawn(uint16_t id, 
		    int16_t x, int16_t y, int16_t z,
		    int16_t xVel, int16_t yVel, int16_t zVel,
		    int8_t heading, int8_t deltaHeading,
		    uint8_t animation);
   void updateSpawns(const uint8_t* updates);
   void updateSpawnInfo(const uint8_t* spawnupdate);
   void renameSpawn(const uint8_t* renameupdate);
   void illusionSpawn(const uint8_t* illusionupdate);
   void updateSpawnAppearance(const uint8_t* appearanceupdate);
   void shroudSpawn(const uint8_t* spawnupdate, size_t, uint8_t);
   void updateNpcHP(const uint8_t* hpupdate);
   void spawnWearingUpdate(const uint8_t* wearing);
   void consMessage(const uint8_t* con, size_t, uint8_t);
   void deleteSpawn(const uint8_t* delspawn, size_t len);
   void killSpawn(const uint8_t* deadspawn);
   void respawnFromHover(const uint8_t* respawn);
   void corpseLoc(const uint8_t* corpseLoc);

   void playerChangedID(uint16_t playerID);
   void refilterSpawns();
   void refilterSpawnsRuntime();
   void saveSpawns(void);
   void restoreSpawns(void);

 protected:
   void refilterSpawns(spawnItemType type);
   void refilterSpawnsRuntime(spawnItemType type);
   void deleteItem(spawnItemType type, int id);
   bool updateFilterFlags(Item* item);
   bool updateRuntimeFilterFlags(Item* item);

   ItemMap& getMap(spawnItemType type);

 private:
   ZoneMgr* m_zoneMgr;
   Player* m_player;
   FilterMgr& m_filterMgr;
   GuildMgr* m_guildMgr;

   // track recently killed spawns
   uint16_t m_deadSpawnID[MAX_DEAD_SPAWNIDS];
   uint8_t m_cntDeadSpawnIDs;
   uint8_t m_posDeadSpawnIDs;

   // maps to keep track of the different types of spawns
   ItemMap m_spawns;
   ItemMap m_drops;
   ItemMap m_doors;
   ItemMap m_players;

   // timer for saving spawns
   QTimer* m_timer;
   
   // flag for using the update radius
   bool m_useUpdateRadius;
};

inline
const ItemMap& SpawnShell::getConstMap(spawnItemType type) const
{ 
  switch (type)
  {
  case tSpawn:
    return m_spawns;
  case tDrop:
    return m_drops;
  case tDoors:
    return m_doors;
  case tPlayer:
    return m_players;
  default:
    return m_spawns;
  }
}

inline
ItemMap& SpawnShell::getMap(spawnItemType type)
{ 
  switch (type)
  {
  case tSpawn:
    return m_spawns;
  case tDrop:
    return m_drops;
  case tDoors:
    return m_doors;
  case tPlayer:
    return m_players;
  default:
    return m_spawns;
  }
}

inline
const ItemMap& SpawnShell::spawns(void) const
{
  return m_spawns;
}

inline
const ItemMap& SpawnShell::drops(void) const
{
  return m_drops;
}

inline
const ItemMap& SpawnShell::doors(void) const
{
  return m_doors; 
}

inline
void SpawnShell::toggleUseUpdateRadius(void)
{
  m_useUpdateRadius=!m_useUpdateRadius;
}

inline
void SpawnShell::setUseUpdateRadius(bool useUpdateRadius)
{
  m_useUpdateRadius=useUpdateRadius;
}

//--------------------------------------------------

#endif // SPAWNSHELL_H
