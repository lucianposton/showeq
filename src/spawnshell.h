/*
 * spawnshell.h
 *
 * ShowEQ Distributed under GPL
 * http://sourceforge.net/projects/seq/
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

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <map>

#include <qtimer.h>
#include <qtextstream.h>

#include "everquest.h"
#include "spawn.h"

//----------------------------------------------------------------------
// forward declarations
class EQPlayer;
class ZoneMgr;
class FilterMgr;
class SpawnShell;

//----------------------------------------------------------------------
// constants

// The maximum number of ID's of deleted spawns to track
const int MAX_DEAD_SPAWNIDS = 50;

//----------------------------------------------------------------------
// enumerated types

// type of action that triggered alert
enum alertType
{
  tNewSpawn,
  tFilledSpawn,
  tKillSpawn,
  tDelSpawn,
};

// type of change done to item
enum changeType
{
  tSpawnChangedNone = 0,
  tSpawnChangedPosition = 1,
  tSpawnChangedHP = 2,
  tSpawnChangedWearing = 4,
  tSpawnChangedFlags = 8,
  tSpawnChangedLevel = 16,
  tSpawnChangedNPC = 32,
  tSpawnChangedFilter = 64,
  tSpawnChangedRuntimeFilter = 128,
  tSpawnChangedConsidered = 256,
  tSpawnChangedName = 512,
  tSpawnChangedALL = 1023, // sum of all previous change types 
};


//----------------------------------------------------------------------
// type definitions
typedef std::map<uint16_t, Item* > ItemMap;
typedef ItemMap::iterator ItemIterator;
typedef ItemMap::const_iterator ItemConstIterator;

//----------------------------------------------------------------------
// SpawnShell
class SpawnShell : public QObject
{
   Q_OBJECT
public:
   SpawnShell(FilterMgr& filterMgr, ZoneMgr* zoneMgr, EQPlayer* player);

   const Item* findID(itemType type, int idSpawn);
   
   const Item* findClosestItem(itemType type, 
			       int16_t x,
			       int16_t y, 
			       double& minDistance);
   const Spawn* findSpawnByRawName(const QString& name);

   void dumpSpawns(itemType type, QTextStream& out);
   FilterMgr* filterMgr(void) { return &m_filterMgr; }
   const ItemMap& getConstMap(itemType type) const;
   const ItemMap& spawns(void) const;
   const ItemMap& drops(void) const;
   const ItemMap& coins(void) const;
   const ItemMap& doors(void) const;
signals:
   void addItem(const Item* item);
   void delItem(const Item* item);
   void changeItem(const Item* item, uint32_t changeType);
   void killSpawn(const Item* deceased, const Item* killer, uint16_t killerId);
   void selectSpawn(const Item* item);
   void spawnConsidered(const Item* item);
   void clearItems();
   void numSpawns(int);
   void handleAlert(const Item* item, alertType type);

   void msgReceived(const QString& msg);

public slots: 
   void clear();

   // slots to receive from EQPacket...
   void newGroundItem(const makeDropStruct*);
   void removeGroundItem(const remDropStruct*);
   void newCoinsItem(const dropCoinsStruct*);
   void compressedDoorSpawn (const cDoorSpawnsStruct* c);
   void newDoorSpawn(const doorStruct* d);
   void removeCoinsItem(const removeCoinsStruct*);
   void zoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len);
   void zoneBegin(const QString& shortZoneName);
   void newSpawn(const newSpawnStruct* spawn);
   void newSpawn(const spawnStruct& s);
   void playerUpdate(const playerPosStruct *pupdate, uint32_t, uint8_t);
   void updateSpawn(uint16_t id, 
		    int16_t x, int16_t y, int16_t z,
		    int16_t xVel, int16_t yVel, int16_t zVel,
		    int8_t heading, int8_t deltaHeading,
		    uint8_t animation);
   void updateSpawns(const mobUpdateStruct* updates);
   void updateSpawnHP(const hpUpdateStruct* hpupdate);
   void spawnWearingUpdate(const wearChangeStruct* wearing);
   void consMessage(const considerStruct* con, uint32_t, uint8_t);
   void deleteSpawn(const deleteSpawnStruct* delspawn);
   void killSpawn(const newCorpseStruct* deadspawn);
   void corpseLoc(const corpseLocStruct* corpseLoc);

   void backfillSpawn(const newSpawnStruct* nspawn);
   void backfillSpawn(const spawnStruct* spawn);
   void backfillZoneSpawns(const zoneSpawnsStruct*, uint32_t);
   void refilterSpawns();
   void refilterSpawnsRuntime();
   void saveSpawns(void);
   void restoreSpawns(void);

 protected:
   void refilterSpawns(itemType type);
   void refilterSpawnsRuntime(itemType type);
   void deleteItem(itemType type, int id);
   bool updateFilterFlags(Item* item);
   bool updateFilterFlags(Spawn* spawn);
   bool updateRuntimeFilterFlags(Item* item);
   bool updateRuntimeFilterFlags(Spawn* spawn);

   ItemMap& getMap(itemType type);

   void clearMap(ItemMap& map);
   
 private:
   ZoneMgr* m_zoneMgr;
   EQPlayer* m_player;

   // track recently killed spawns
   uint16_t m_deadSpawnID[MAX_DEAD_SPAWNIDS];
   uint8_t m_cntDeadSpawnIDs;
   uint8_t m_posDeadSpawnIDs;

   // maps to keep track of the different types of spawns
   ItemMap m_spawns;
   ItemMap m_drops;
   ItemMap m_coins;
   ItemMap m_doors;

   // timer for saving spawns
   QTimer* m_timer;

   // filter manager
   FilterMgr& m_filterMgr;
};

inline
const ItemMap& SpawnShell::getConstMap(itemType type) const
{ 
  switch (type)
  {
  case tSpawn:
    return m_spawns;
  case tCoins:
    return m_coins;
  case tDrop:
    return m_drops;
  case tDoors:
    return m_doors;
  default:
    return m_spawns;
  }
}

inline
ItemMap& SpawnShell::getMap(itemType type)
{ 
  switch (type)
  {
  case tSpawn:
    return m_spawns;
  case tCoins:
    return m_coins;
  case tDrop:
    return m_drops;
  case tDoors:
    return m_doors;
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
const ItemMap& SpawnShell::coins(void) const
{
  return m_coins; 
}

inline
const ItemMap& SpawnShell::doors(void) const
{
  return m_doors; 
}

//--------------------------------------------------

#endif // SPAWNSHELL_H
