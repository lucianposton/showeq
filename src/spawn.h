/*
 * spawn.h
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

#ifndef _SPAWN_H_
#define _SPAWN_H_

#include <stdint.h>
//#include <time.h>
//#include <stdio.h>
//#include <sys/time.h>
#include <math.h>

#include <qdatetime.h>

#include "everquest.h"
#include "point.h"

//----------------------------------------------------------------------
// forward declarations
class SpawnShell;

//----------------------------------------------------------------------
// enumerated types
enum itemType 
{ 
  tUnknown, 
  tCoins, 
  tDoors, 
  tDrop,
  tSpawn 
};

//----------------------------------------------------------------------
// type definitions
typedef Point3D<int16_t> EQPoint;
typedef EQPoint SpawnTrackPoint;
typedef QList<SpawnTrackPoint> SpawnTrackList;
typedef QListIterator<SpawnTrackPoint> SpawnTrackListIterator;

//----------------------------------------------------------------------
// constants

// default minimum distance for finding the closest item
const double DEFAULT_MIN_DISTANCE = HUGE_VAL;

//----------------------------------------------------------------------
// Item
class Item : public EQPoint
{
 public:
  // constructor/destructor
  Item(itemType t, uint16_t id);
  virtual ~Item();

  // get methods
  itemType type() const { return m_type; }
  uint16_t id() const { return m_ID; }
  QString realName() const { return m_name; }
  const QTime& lastUpdate() const { return m_lastUpdate; }
  QString lastUpdateStr() const 
    { return m_lastUpdate.toString(); }
  const QTime& spawnTime() const { return m_spawnTime; }
  QString spawnTimeStr() const
    { return m_spawnTime.toString(); }
  uint32_t filterFlags() const { return m_filterFlags; }
  uint32_t runtimeFilterFlags() const { return m_runtimeFilterFlags; }
  QString name() const;
  uint8_t NPC() const { return m_NPC; }

  // virtual methods that provide reasonable default values
  virtual QString transformedName() const;
  virtual uint8_t race() const;
  virtual QString raceName() const;
  virtual uint8_t classVal() const;
  virtual QString className() const;
  virtual QString info() const;
  virtual QString filterString() const;
  virtual QString dumpString() const;

  // set methods
  void setPos(int16_t xPos, int16_t yPos, int16_t zPos);

  void setName(const QString& name)
    { m_name = name; }

  void updateLast()
  {
    m_lastUpdate.restart();
  }

  void setFilterFlags(uint32_t filterFlags) { m_filterFlags = filterFlags; }
  void setRuntimeFilterFlags(uint32_t filterFlags) 
    { m_runtimeFilterFlags = filterFlags; }

 protected:
  void setNPC(uint8_t NPC) { m_NPC = NPC; }

  // common item data
  QString m_name;
  QTime m_lastUpdate; 
  QTime m_spawnTime; 
  uint32_t m_filterFlags;
  uint32_t m_runtimeFilterFlags;
  uint16_t m_ID;
  uint8_t m_NPC;
  itemType m_type;
};

//----------------------------------------------------------------------
// Spawn
class Spawn : public Item
{
 public:
  // constructors/destructor
  Spawn();
  Spawn(const spawnStruct* s);
  Spawn(uint16_t id, const spawnStruct* s);
  Spawn(const playerProfileStruct* player,
		  uint8_t deity);

  Spawn(uint16_t id, 
		  const QString& name, const QString& lastName, 
		  uint8_t race, uint8_t classVal,
		  uint8_t level, uint8_t deity = 0);
		  
  // create an unknown spawn using the data available.
  Spawn(uint16_t id, 
		  int16_t xPos, int16_t yPos, int16_t zPos,
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ,
		  int8_t heading, int8_t deltaHeading);

  // destructive copy constructor, clears the track list of the 
  // spawn being copied.
  Spawn(Spawn*, uint16_t id);
  virtual ~Spawn();

  // spawn specific get methods
  const QString& rawName() const { return m_rawName; }
  int16_t deltaX() const { return m_deltaX; }
  int16_t deltaY() const { return m_deltaY; }
  int16_t deltaZ() const { return m_deltaZ; }
  int8_t heading() const { return m_heading; }
  int16_t headingAngle() const { return 360 - (m_heading * 360) / 256; }
  int8_t deltaHeading() const { return m_deltaHeading; }
  uint16_t HP() const { return m_curHP; }
  uint16_t maxHP() const { return m_maxHP; }
  uint8_t level() const { return m_level; }
  uint16_t petOwnerID() const { return m_petOwnerID; }
  uint8_t light() const { return m_light; }
  QString lightName() const;
  uint8_t gender() const { return m_gender; }
  uint8_t deity() const { return m_deity; }
  QString deityName() const;
  int8_t deityTeam() const { return m_deityTeam; }
  int8_t raceTeam() const { return m_raceTeam; }
  bool considered() const { return m_considered; }
  uint16_t equipment(uint8_t wearingSlot) const 
    { return m_equipment[wearingSlot]; }
  QString equipmentStr(uint8_t wearingSlot) const;
  const SpawnTrackList& trackList() const { return m_spawnTrackList; }
  SpawnTrackList& trackList() { return m_spawnTrackList; }
  QString cleanedName() const;
  bool approximatePosition(bool animating, 
			   const QTime& curTime,
			   EQPoint& newPos) const;
  

  // virtual get method overloads
  virtual QString transformedName() const;
  virtual uint8_t race() const;
  virtual QString raceName() const;
  virtual uint8_t classVal() const;
  virtual QString className() const;
  virtual QString info() const;
  virtual QString filterString() const;
  virtual QString dumpString() const;

  // convenience test methods
  bool isSelf() const { return (m_NPC == SPAWN_SELF); }
  bool isCorpse() const 
  {
    return ((m_NPC == SPAWN_PC_CORPSE) ||
	    (m_NPC == SPAWN_NPC_CORPSE));
  }
  bool isPlayer() const
  {
    return ((m_NPC == SPAWN_PLAYER) ||
	    (m_NPC == SPAWN_SELF));
  }
  bool isOtherPlayer() const
    { return (m_NPC == SPAWN_PLAYER); }
  bool isNPC() const 
    { return (m_NPC == SPAWN_NPC); }
  bool isUnknown() const
    { return (m_NPC == SPAWN_NPC_UNKNOWN); }
  bool isSameRaceTeam(const Spawn* spawn) const
    { return (raceTeam() == spawn->raceTeam()); }
  bool isSameDeityTeam(const Spawn* spawn) const
    { return (deityTeam() == spawn->deityTeam()); }

  // virtual set method overload
  void setPos(int16_t xPos, int16_t Pos, int16_t zPos,
	      bool walkpathrecord = false, size_t walkpathlength = 0);

  // update methods 

  // updates all of the, irregardless of existing values
  void update(const spawnStruct* s);

  // updates the data, careful not to overwrite existing correct data
  void backfill(const spawnStruct* s);
  void backfill(const playerProfileStruct* player);

  // change spawn state
  void killSpawn();

  // spawn specific set methods
  void setDeltas(int16_t deltaX, int16_t deltaY, int16_t deltaZ);
  void setHeading(int8_t heading, int8_t deltaHeading)
    { m_heading = heading; m_deltaHeading = deltaHeading; }
  void setHeading(int8_t heading) { m_heading = heading; }
  void setDeltaHeading(int8_t deltaHeading) { m_deltaHeading = deltaHeading; }
  void setPetOwnerID(uint16_t petOwnerID) { m_petOwnerID = petOwnerID; }
  void setLight(uint8_t light) { m_light = light; }
  void setGender(uint8_t gender) { m_gender = gender; }
  void setDeity(uint8_t deity) { m_deity = deity; calcDeityTeam(); }
  void setConsidered(bool considered) { m_considered = considered; }
  void setRace(uint8_t race) { m_race = race; calcRaceTeam(); }
  void setClassVal(uint8_t classVal) { m_class = classVal; }
  void setHP(uint16_t HP) { m_curHP = HP; }
  void setMaxHP(uint16_t maxHP) { m_maxHP = maxHP; }
  void setLevel(uint8_t level) { m_level = level; }
  void setEquipment(uint8_t wearSlot, uint16_t itemID)
    { if (wearSlot < 9) { m_equipment[wearSlot] = itemID; } }
  void setNPC(uint8_t NPC) { m_NPC = NPC; }
  void setID(uint16_t id) { m_ID = id; }

 protected:
  void calcRaceTeam();
  void calcDeityTeam();

  // spawn specific data
  QString m_rawName;
  SpawnTrackList m_spawnTrackList;
  int m_cookedDeltaXFixPt;
  int m_cookedDeltaYFixPt;
  int m_cookedDeltaZFixPt;
  uint16_t m_petOwnerID;
  int16_t m_deltaX;
  int16_t m_deltaY;
  int16_t m_deltaZ;
  uint16_t m_curHP;
  uint16_t m_maxHP;
  int8_t m_heading;
  int8_t m_deltaHeading;
  uint8_t m_level;
  uint8_t m_race;
  int8_t m_raceTeam;
  uint8_t m_deity;
  int8_t m_deityTeam;
  uint8_t m_gender;
  uint8_t m_class;
  uint8_t m_light;
  uint16_t m_equipment[9];
  bool m_considered;
};


//----------------------------------------------------------------------
// Coin
class Coin : public Item
{
 public:
  // constructor/destructor
  Coin(const dropCoinsStruct* c);
  virtual ~Coin();

  // coin specific get methods
  uint32_t amount() const { return m_amount; }
  uint8_t coinType() const { return m_coinType; }

  // virtual get method overloads
  virtual QString raceName() const;
  virtual QString className() const;

  // update methods
  void update(const dropCoinsStruct* c);

  // coin specific set methods
  void setAmount(uint32_t amount)
    { m_amount = amount; }
  void setCoinType(uint8_t type)
    { m_coinType = type; }

 protected:
  // coin specific data
  uint32_t m_amount;
  uint8_t m_coinType;
};

//----------------------------------------------------------------------
// Door
class Door : public Item
{
 public:
  Door(const doorStruct* d);
  virtual ~Door();

  // virtual get method overloads
  virtual QString raceName() const;
  virtual QString className() const;

  // update methods
  void update(const doorStruct* d);

};

//----------------------------------------------------------------------
// Drop
class Drop : public Item
{
 public:
  // constructor/destructor
  Drop(const dropThingOnGround* d, const QString& name);
  virtual ~Drop();

  // drop specific get methods
  uint16_t itemNr() const { return m_itemNr; }
  QString idFile() const { return m_idFile; }

  // virtual get method overloads
  virtual QString raceName() const;
  virtual QString className() const;
  
  // update methods
  void update(const dropThingOnGround* d, const QString& name);

  // drop specific set methods
  void setItemNr(uint16_t itemNr) 
    {  m_itemNr = itemNr; }
  void setIdFile(const QString& idFile)
    { m_idFile = idFile; }

 protected:
  // drop specific data
  uint16_t m_itemNr;
  QString m_idFile;
};

//----------------------------------------------------------------------
// Item safe casts inlines
inline const Spawn* spawnType(const Item* item)
{
  // if this is an item of spawn type, return the pointer to Spawn
  if (item->type() == tSpawn)
    return (const Spawn*)item;
  else
    return NULL; // otherwise NULL
}

inline Spawn* spawnType(Item* item)
{
  // if this is an item of spawn type, return the pointer to Spawn
  if (item->type() == tSpawn)
    return (Spawn*)item;
  else
    return NULL; // otherwise NULL
}


inline const Coin* coinType(const Item* item)
{
  // if this is an item of coin type, return the pointer to Coin
  if (item->type() == tCoins)
    return (const Coin*)item;
  else
    return NULL; // otherwise NULL
}

inline Coin* coinType(Item* item)
{
  // if this is an item of coin type, return the pointer to Coin
  if (item->type() == tCoins)
    return (Coin*)item;
  else
    return NULL; // otherwise NULL
}

inline const Door* doorType(const Item* item)
{
  // if this is an item of door type, return the pointer to Door
  if (item->type() == tDoors)
    return (const Door*)item;
  else
    return NULL; // otherwise NULL
}

inline Door* doorType(Item* item)
{
  // if this is an item of door type, return the pointer to Door
  if (item->type() == tDoors)
    return (Door*)item;
  else
    return NULL; // otherwise NULL
}


inline const Drop* dropType(const Item* item)
{
  // if this is an item of drop type, return the pointer to Drop
  if (item->type() == tDrop)
    return (const Drop*)item;
  else
    return NULL; // otherwise NULL
}

inline Drop* dropType(Item* item)
{
  // if this is an item of drop type, return the pointer to Drop
  if (item->type() == tDrop)
    return (Drop*)item;
  else
    return NULL; // otherwise NULL
}

#endif // _SPAWN_H_

