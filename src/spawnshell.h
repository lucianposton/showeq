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

// type of action that triggered alert
enum alertType
{
  tNewSpawn,
  tFilledSpawn,
  tKillSpawn,
  tDelSpawn,
};

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
	      EQItemDB* itemDB,
              GuildMgr* guildMgr);

   const Item* findID(spawnItemType type, int idSpawn);
   
   const Item* findClosestItem(spawnItemType type, 
			       int16_t x,
			       int16_t y, 
			       double& minDistance);
   const Spawn* findSpawnByName(const QString& name);

   void dumpSpawns(spawnItemType type, QTextStream& out);
   FilterMgr* filterMgr(void) { return &m_filterMgr; }
   const ItemMap& getConstMap(spawnItemType type) const;
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
   void newGroundItem(const makeDropStruct*, uint32_t, uint8_t);
   void removeGroundItem(const remDropStruct*, uint32_t, uint8_t);
   void newCoinsItem(const dropCoinsStruct*);
   //   void compressedDoorSpawn (const cDoorSpawnsStruct* c);
   void newDoorSpawn(const doorStruct* d, uint32_t len, uint8_t dir);
   void removeCoinsItem(const removeCoinsStruct*);
   void zoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len);
   void newSpawn(const newSpawnStruct* spawn);
   void newSpawn(const spawnStruct& s);
   void playerUpdate(const playerSpawnPosStruct *pupdate, uint32_t, uint8_t);
   void updateSpawn(uint16_t id, 
		    int16_t x, int16_t y, int16_t z,
		    int16_t xVel, int16_t yVel, int16_t zVel,
		    int8_t heading, int8_t deltaHeading,
		    uint8_t animation);
   void updateSpawns(const spawnPositionUpdate* updates);
   void updateSpawnMaxHP(const SpawnUpdateStruct* spawnupdate);
   void updateNpcHP(const hpNpcUpdateStruct* hpupdate);
   void spawnWearingUpdate(const wearChangeStruct* wearing);
   void consMessage(const considerStruct* con, uint32_t, uint8_t);
   void deleteSpawn(const deleteSpawnStruct* delspawn);
   void killSpawn(const newCorpseStruct* deadspawn);
   void corpseLoc(const corpseLocStruct* corpseLoc);

   void playerChangedID(uint16_t playerID);
   void refilterSpawns();
   void refilterSpawnsRuntime();
   void saveSpawns(void);
   void restoreSpawns(void);
   void setPlayerGuildTag(void);

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
   EQItemDB* m_itemDB;
   GuildMgr* m_guildMgr;

   // track recently killed spawns
   uint16_t m_deadSpawnID[MAX_DEAD_SPAWNIDS];
   uint8_t m_cntDeadSpawnIDs;
   uint8_t m_posDeadSpawnIDs;

   // maps to keep track of the different types of spawns
   ItemMap m_spawns;
   ItemMap m_drops;
   ItemMap m_coins;
   ItemMap m_doors;
   ItemMap m_players;

   // timer for saving spawns
   QTimer* m_timer;
};

inline
const ItemMap& SpawnShell::getConstMap(spawnItemType type) const
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
  case tCoins:
    return m_coins;
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
