/*
 * spawnshell.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 * 
 * Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

/*
 * Adapted from spawnlist.cpp - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date   - 7/31/2001
 */

#include "spawnshell.h"
#include "filtermgr.h"
#include "zonemgr.h"
#include "player.h"
#include "util.h"
#include "itemdb.h"
#include "guild.h"
#include "packetcommon.h"
#include "diagnosticmessages.h"

#include <qfile.h>
#include <qdatastream.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <limits.h>
#include <math.h>

//----------------------------------------------------------------------
// useful macro definitions

// define this to have the spawnshell print diagnostics
//#define SPAWNSHELL_DIAG

// define this to diagnose structures passed in to SpawnShell
//#define SPAWNSHELL_DIAG_STRUCTS

// define this to have the spawnshell validate names to help spot errors
//#define SPAWNSHELL_NAME_VALIDATE

//----------------------------------------------------------------------
// constants
static const char magicStr[5] = "spn4"; // magic is the size of uint32_t + a null
static const uint32_t* magic = (uint32_t*)magicStr;
static const char * Spawn_Corpse_Designator = "'s corpse";

//----------------------------------------------------------------------
// Handy utility function
#ifdef SPAWNSHELL_NAME_VALIDATE
static bool isValidName(const char* name, size_t len)
{
  int i = 0;

  // loop over the string until the maximum length is reached
  while (i < len)
  {
    // if the terminating NULL has been found, we're done
    if ( name[i] == 0 )
      break;

    // if the current character is outside the normal range, fail the name
    if ( (name[i] < ' ') || (name[i] > '~') ) 
      return false;

    // keep going until done
    i++;
  }

  // if we finished with i being the buffer length, fail the name
  // because it's not NULL terminated
  if (i == len)
    return false;

  // it's a real name, return success
  return true;
}
#endif

//----------------------------------------------------------------------
// SpawnShell
SpawnShell::SpawnShell(FilterMgr& filterMgr, 
		       ZoneMgr* zoneMgr, 
		       Player* player,
                       GuildMgr* guildMgr)
  : QObject(NULL, "spawnshell"),
    m_zoneMgr(zoneMgr),
    m_player(player),
    m_filterMgr(filterMgr),
    m_guildMgr(guildMgr),
    m_spawns(701),
    m_drops(211),
    m_doors(307),
    m_players(2)
{
   m_cntDeadSpawnIDs = 0;
   m_posDeadSpawnIDs = 0;
   for (int i = 0; i < MAX_DEAD_SPAWNIDS; i++)
     m_deadSpawnID[i] = 0;

   // these should auto delete
   m_spawns.setAutoDelete(true);
   m_drops.setAutoDelete(true);
   m_doors.setAutoDelete(true);

   // we don't want this one to auto-delete
   m_players.setAutoDelete(false); 

   // bogus list
   m_players.insert(0, m_player);

   // connect the FilterMgr's signals to SpawnShells slots
   connect(&m_filterMgr, SIGNAL(filtersChanged()),
	   this, SLOT(refilterSpawns()));
   connect(&m_filterMgr, SIGNAL(runtimeFiltersChanged(uint8_t)),
	   this, SLOT(refilterSpawnsRuntime()));

   connect(m_player, SIGNAL(levelChanged(uint8_t)),
	   this, SLOT(playerLevelChanged(uint8_t)));
   connect(m_player, SIGNAL(guildJoined(uint16_t)),
	   this, SLOT(playerGuildChanged(uint16_t)));

   // connect SpawnShell slots to ZoneMgr signals
   connect(m_zoneMgr, SIGNAL(zoneBegin()),
	   this, SLOT(clear(void)));

   // connect Player signals to SpawnShell signals
   connect(m_player, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SIGNAL(changeItem(const Item*, uint32_t)));

   // connect Player signals to SpawnShell slots
   connect(m_player, SIGNAL(changedID(uint16_t)),
	   this, SLOT(playerChangedID(uint16_t)));

   // restore the spawn list if necessary
   if (showeq_params->restoreSpawns)
     restoreSpawns();

   // create the timer
   m_timer = new QTimer(this);

   // connect the timer
   connect(m_timer, SIGNAL(timeout()),
	   this, SLOT(saveSpawns(void)));

   // start the timer (changed to oneshot to help prevent a backlog on slower
   // machines)
   if (showeq_params->saveSpawns)
     m_timer->start(showeq_params->saveSpawnsFrequency, true);
}

void SpawnShell::clear(void)
{
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::clear()");
#endif

   emit clearItems();

   m_spawns.clear();
   m_doors.clear();
   m_drops.clear();

   // clear the players list, reinsert the player
   m_players.clear();
   m_players.insert(0, m_player);

   // emit an changeItem for the player
   emit changeItem(m_player, tSpawnChangedALL);

   m_cntDeadSpawnIDs = 0;
   m_posDeadSpawnIDs = 0;
   for (int i = 0; i < MAX_DEAD_SPAWNIDS; i++)
     m_deadSpawnID[i] = 0;
} // end clear

const Item* SpawnShell::findID(spawnItemType type, int id)
{
  const Item* item = NULL;
  
  if ((type == tSpawn) && (id == m_player->id()))
    return (const Item*)m_player;

  if (type != tPlayer)
    item = getMap(type).find(id);

  return item;
}

const Item* SpawnShell::findClosestItem(spawnItemType type, 
					int16_t x, int16_t y,
					double& minDistance)
{
   ItemMap& theMap = getMap(type);
   ItemIterator it(theMap);
   double distance;
   Item* item;
   Item* closest = NULL;

   // find closest spawn

   // iterate over all the items in the map
   for (; it.current(); ++it)
   {
     // get the item
     item = it.current();
     
     // calculate the distance from the specified point
     distance = item->calcDist(x, y);

     // is this distance closer?
     if (distance < minDistance)
     {
       // yes, note it
       minDistance = distance;
       closest = item;
     }
   }

   // return the closest item.
   return closest;
}

Spawn* SpawnShell::findSpawnByName(const QString& name)
{
  ItemIterator it(m_spawns);
  Spawn* spawn;

  for (; it.current(); ++it)
  {
    // the item and coerce it to the Spawn type
    spawn = (Spawn*)it.current();

    if (name == spawn->name())
      return spawn;
  }

  if (name == m_player->name())
    return m_player;

  return NULL;
}

void SpawnShell::deleteItem(spawnItemType type, int id)
{
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::deleteItem()");
#endif
   ItemMap& theMap = getMap(type);

   Item* item = theMap.find(id);

   if (item != NULL)
   {
     emit delItem(item);
     theMap.remove(id);

     // send notifcation of new spawn count
     emit numSpawns(m_spawns.count());
   }
}

bool SpawnShell::updateFilterFlags(Item* item)
{
  uint8_t level = 0;

  if (item->type() == tSpawn)
    level = ((Spawn*)item)->level();

  // get the filter flags
  uint32_t flags = m_filterMgr.filterMask(item->filterString(), level);

  // see if the new filter flags are different from the old ones
  if (flags != item->filterFlags())
  {
    // yes, set the new filter flags
    item->setFilterFlags(flags);

    // return true to indicate that the flags have changed
    return true;
  }

  // flags haven't changed
  return false;
}

bool SpawnShell::updateRuntimeFilterFlags(Item* item)
{
  uint8_t level = 0;

  if (item->type() == tSpawn)
    level = ((Spawn*)item)->level();

  // get the filter flags
  uint32_t flags = m_filterMgr.runtimeFilterMask(item->filterString(), level);

  // see if the new filter flags are different from the old ones
  if (flags != item->runtimeFilterFlags())
  {
    // yes, set the new filter flags
    item->setRuntimeFilterFlags(flags);

    // return true to indicate that the flags have changed
    return true;
  }

  // flags haven't changed
  return false;
}

void SpawnShell::dumpSpawns(spawnItemType type, QTextStream& out)
{
   ItemIterator it(getMap(type));

   for (; it.current(); ++it)
     out << it.current()->dumpString() << endl;
}

// same-name slots, connecting to Packet signals
void SpawnShell::newGroundItem(const uint8_t* data, size_t, uint8_t dir)
{
  const makeDropStruct *d = (const makeDropStruct *)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::newGroundItem(makeDropStruct *)");
#endif
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  if (dir != DIR_Server)
    return;

  if (!d)
    return;
  
  QString name;
  Drop* item = (Drop*)m_drops.find(d->dropId);
  if (item != NULL)
  {
    item->update(d, name);
    if (!showeq_params->fast_machine)
       item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
    else
       item->setDistanceToPlayer(m_player->calcDist(*item));
    updateFilterFlags(item);
    item->updateLastChanged();
    emit changeItem(item, tSpawnChangedALL);
  }
  else
  {
    item = new Drop(d, name);
    if (!showeq_params->fast_machine)
       item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
     else
       item->setDistanceToPlayer(m_player->calcDist(*item));
    updateFilterFlags(item);
    m_drops.insert(d->dropId, item);
    emit addItem(item);
  }
}

void SpawnShell::removeGroundItem(const uint8_t* data, size_t, uint8_t dir)
{
#ifdef SPAWNSHELL_DIAG
  seqDebug("SpawnShell::removeGroundItem(remDropStruct *)");
#endif

  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  if (dir != DIR_Server)
    return;

  const remDropStruct *d = (const remDropStruct *)data;

  if (d)
    deleteItem(tDrop, d->dropId);
}

void SpawnShell::newDoorSpawns(const uint8_t* data, size_t len, uint8_t dir)
{
  int nDoors = len / sizeof(doorStruct);
  const doorStruct* doors = (const doorStruct*)data;
  for (int i = 0; i < nDoors; i++)
    newDoorSpawn(doors[i], sizeof(doorStruct), dir);
}

void SpawnShell::newDoorSpawn(const doorStruct& d, size_t len, uint8_t dir)
{
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::newDoorSpawn(doorStruct*)");
#endif
   Item* item = m_doors.find(d.doorId);
   if (item != NULL)
   {
     Door* door = (Door*)item;
     door->update(&d);
     if (!showeq_params->fast_machine)
        item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
     else
        item->setDistanceToPlayer(m_player->calcDist(*item));
     updateFilterFlags(door);
     item->updateLastChanged();
     emit changeItem(door, tSpawnChangedALL);
   }
   else
   {
     item = (Item*)new Door(&d);
     if (!showeq_params->fast_machine)
        item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
     else
        item->setDistanceToPlayer(m_player->calcDist(*item));
     updateFilterFlags(item);
     m_doors.insert(d.doorId, item);
     emit addItem(item);
   }
}

void SpawnShell::zoneSpawns(const uint8_t* data, size_t len)
{
  int spawndatasize = len / sizeof(spawnStruct);

  const spawnStruct* zspawns = (const spawnStruct*)data;

  for (int i = 0; i < spawndatasize; i++)
  {
#if 0
  // Dump position updates for debugging spawn struct position changes
  for (int j=54; j<70; i++)
  {
      printf("%.2x", zspawns[i][j]);

      if ((j+1) % 8 == 0)
      {
          printf("    ");
      }
      else
      {
          printf(" ");
      }
  }
  printf("\n");
#endif

#if 0
    // Debug positioning without having to recompile everything...
#pragma pack(1)
    struct pos
{
/*0054*/ signed   deltaHeading:10;  // change in heading
         signed   x:19;             // x coord
         signed   padding0054:3;    // ***Placeholder
/*0058*/ signed   y:19;             // y coord
         signed   animation:10;     // ***Placeholder (seems like speed)
         signed   padding0058:3;    // animation
/*0062*/ signed   z:19;             // z coord
         signed   deltaY:13;        // change in y
/*0066*/ signed   deltaX:13;        // change in x
         unsigned heading:12;       // heading
         signed   padding0066:7;    // ***Placeholder
/*0070*/ signed   deltaZ:13;        // change in z
         signed   padding0070:19;   // ***Placeholder
/*0074*/
};
#pragma pack(0)
    struct pos *p = (struct pos *)(data + i*sizeof(spawnStruct) + 54);
    printf("[%.2x](%f, %f, %f), dx %f dy %f dz %f head %f dhead %f anim %d (%x, %x, %x, %x)\n",
            zspawns[i].spawnId, 
            float(p->x)/8.0, float(p->y/8.0), float(p->z)/8.0, 
            float(p->deltaX)/4.0, float(p->deltaY)/4.0, 
            float(p->deltaZ)/4.0, 
            float(p->heading), float(p->deltaHeading),
            p->animation, p->padding0054, p->padding0058, 
            p->padding0066, p->padding0070);
#endif
    emit initialZoneSpawn(true);
    newSpawn(zspawns[i]);
    emit initialZoneSpawn(false);
  }
}

void SpawnShell::newSpawn(const uint8_t* data)
{
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
  {
#ifdef SPAWNSHELL_DIAG
    const spawnStruct* spawn = (const spawnStruct*)data;
    seqDebug("SpawnShell::newSpawn(spawnStruct *(id=%d, name='%s')) DROPPED while zoning", spawn->spawnId, spawn->name);
#endif
    return;
  }

  const spawnStruct* spawn = (const spawnStruct*)data;


  newSpawn(*spawn);
}

void SpawnShell::newSpawn(const spawnStruct& s)
{
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::newSpawn(spawnStruct *(id=%d, name='%s'))", s.spawnId, s.name);
#endif
   // if this is the SPAWN_SELF it's the player
   if (s.NPC == SPAWN_SELF)
     return;

   // When s.NPC is not set to SPAWN_SELF, check whether id matches known
   // player id
   if (s.spawnId == m_player->id())
       return;

   // not the player, so check if it's a recently deleted spawn
   for (int i =0; i < m_cntDeadSpawnIDs; i++)
   {
     if ((m_deadSpawnID[i] != 0) && (m_deadSpawnID[i] == s.spawnId))
     {
       // found a match, remove it from the deleted spawn list
       m_deadSpawnID[i] = 0;

       // let the user know what's going on
       seqDebug("%s(%d) has already been removed from the zone before we processed it.", 
	      s.name, s.spawnId);
       
       // and stop the attempt to add the spawn.
       return;
     }
   }

   Item* item = m_spawns.find(s.spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
#ifdef SPAWNSHELL_DIAG
     seqDebug("SpawnShell::newSpawn(): changeItem, before : %s", (const char*)spawn->dumpString());
#endif

     spawn->update(&s);

     spawn->setGuildTag(m_guildMgr->guildIdToName(spawn->guildID()));
     if (!showeq_params->fast_machine)
        item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
     else
        item->setDistanceToPlayer(m_player->calcDist(*item));
     spawn->setIsInPvPLevelRangeToPlayer(spawn->isOtherPlayer() && abs(m_player->level() - spawn->level()) < 5);
     spawn->setIsPlayersGuildmate(!m_player->guildTag().isEmpty() && spawn->guildID() == m_player->guildID());
     updateFilterFlags(spawn);
     updateRuntimeFilterFlags(spawn);
     item->updateLastChanged();

#ifdef SPAWNSHELL_DIAG
     seqDebug("SpawnShell::newSpawn(): changeItem, after : %s", (const char*)spawn->dumpString());
#endif
     emit changeItem(item, tSpawnChangedALL);
   }
   else
   {
     item = new Spawn(&s);
     Spawn* spawn = (Spawn*)item;

     spawn->setGuildTag(m_guildMgr->guildIdToName(spawn->guildID()));
     if (!showeq_params->fast_machine)
        item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
     else
        item->setDistanceToPlayer(m_player->calcDist(*item));
     spawn->setIsInPvPLevelRangeToPlayer(spawn->isOtherPlayer() && abs(m_player->level() - spawn->level()) < 5);
     spawn->setIsPlayersGuildmate(!m_player->guildTag().isEmpty() && spawn->guildID() == m_player->guildID());
     updateFilterFlags(spawn);
     updateRuntimeFilterFlags(spawn);
     m_spawns.insert(s.spawnId, item);

     emit addItem(item);

     // send notification of new spawn count
     emit numSpawns(m_spawns.count());
   }
}

void SpawnShell::playerUpdate(const uint8_t* data, size_t len, uint8_t dir)
{
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

#if 0 
  // Dump position updates for debugging client update changes
  for (int i=0; i<len; i++)
  {
      printf("%.2x", data[i]);

      if ((i+1) % 8 == 0)
      {
          printf("    ");
      }
      else
      {
          printf(" ");
      }
  }
  printf("\n");
#endif

  const playerSpawnPosStruct *pupdate = (const playerSpawnPosStruct *)data;

  if ((dir != DIR_Client) && 
      (pupdate->spawnId != m_player->id())) // PC Corpse Movement
  {
    int16_t y = pupdate->y >> 3;
    int16_t x = pupdate->x >> 3;
    int16_t z = pupdate->z >> 3;
    
    int16_t dy = pupdate->deltaY >> 2;
    int16_t dx = pupdate->deltaX >> 2;
    int16_t dz = pupdate->deltaZ >> 2;
    
#if 0
    // Debug positioning without having to recompile everything...
#pragma pack(1)
    struct pos
{
/*0000*/ uint16_t spawnId;          // spawn id of the thing moving
/*0002*/ signed   deltaHeading:10;  // change in heading
         signed   x:19;             // x coord
         signed   padding0002:3;    // ***Placeholder
/*0006*/ signed   y:19;             // y coord
         signed   animation:10;     // ***Placeholder (seems like speed)
         signed   padding0006:3;    // animation
/*0010*/ signed   z:19;             // z coord
         signed   deltaY:13;        // change in y
/*0014*/ signed   deltaX:13;        // change in x
         unsigned heading:12;       // heading
         signed   padding0014:7;    // ***Placeholder
/*0018*/ signed   deltaZ:13;        // change in z
         signed   padding0018:19;   // ***Placeholder
/*0022*/
};
#pragma pack(0)
    struct pos *p = (struct pos *)data;
    printf("[%.2x](%f, %f, %f), dx %f dy %f dz %f head %f dhead %f anim %d (%x, %x, %x, %x)\n",
            p->spawnId, float(p->x)/8.0, float(p->y/8.0), float(p->z)/8.0, 
            float(p->deltaX)/4.0, float(p->deltaY)/4.0, 
            float(p->deltaZ)/4.0, 
            float(p->heading), float(p->deltaHeading),
            p->animation, p->padding0002, p->padding0006, 
            p->padding0014, p->padding0018);
#endif

    updateSpawn(pupdate->spawnId, x, y, z, dx, dy, dz,
		pupdate->heading, pupdate->deltaHeading,pupdate->animation);
  }
}

void SpawnShell::updateSpawn(uint16_t id, 
			     int16_t x, int16_t y, int16_t z,
			     int16_t xVel, int16_t yVel, int16_t zVel,
			     int8_t heading, int8_t deltaHeading,
			     uint8_t animation)
{
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::updateSpawn(id=%d, x=%d, y=%d, z=%d, xVel=%d, yVel=%d, zVel=%d)", id, x, y, z, xVel, yVel, zVel);
#endif

   Item* item = m_spawns.find(id);

   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;

     spawn->setPos(x, y, z,
		   showeq_params->walkpathrecord,
		   showeq_params->walkpathlength);
     spawn->setAnimation(animation);
     if ((animation != 0) && (animation != 66))
     {
       spawn->setDeltas(xVel, yVel, zVel);
       spawn->setHeading(heading, deltaHeading);
     } 
    else
     {
       spawn->setDeltas(0, 0, 0);
       spawn->setHeading(heading, 0);
     }

     // Distance
     if (!showeq_params->fast_machine)
        item->setDistanceToPlayer(m_player->calcDist2DInt(*item));
     else
        item->setDistanceToPlayer(m_player->calcDist(*item));
        
     spawn->updateLast();
     item->updateLastChanged();
     emit changeItem(item, tSpawnChangedPosition);
   }
   else if (showeq_params->createUnknownSpawns)
   {
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::updateSpawn(id=%d): creating unknown spawn", id);
#endif
     // not the player, so check if it's a recently deleted spawn
     for (int i =0; i < m_cntDeadSpawnIDs; i++)
     {
       // check dead spawn list for spawnID, if it was deleted, shouldn't
       // see new position updates, so therefore this is probably 
       // for a new spawn (spawn ID being reused)
       if ((m_deadSpawnID[i] != 0) && (m_deadSpawnID[i] == id))
       {
	 // found a match, ignore it
	 m_deadSpawnID[i] = 0;

	 seqDebug("(%d) had been removed from the zone, but saw a position update on it, so assuming bogus update.", 
		id);

	 return;
       }
     }

     item = new Spawn(id, x, y, z, xVel, yVel, zVel, 
		      heading, deltaHeading, animation);
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     m_spawns.insert(id, item);
     emit addItem(item);

     // send notification of new spawn count
     emit numSpawns(m_spawns.count());
   }
}

void SpawnShell::updateSpawns(const uint8_t* data)
{
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  const spawnPositionUpdate* updates = (const spawnPositionUpdate*)data;
  updateSpawn(updates->spawnId, 
	      updates->x >> 3, updates->y >> 3, updates->z >> 3,
	      0,0,0,updates->heading,0,0);
}

void SpawnShell::updateSpawnInfo(const uint8_t* data)
{
   const SpawnUpdateStruct* su = (const SpawnUpdateStruct*)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::updateSpawnInfo(id=%d, sub=%d, hp=%d, maxHp=%d)", 
	  su->spawnId, su->subcommand, su->arg1, su->arg2);
#endif

   Item* item = m_spawns.find(su->spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     switch(su->subcommand) {
     case 17: // current hp update
       spawn->setHP(su->arg1);
       item->updateLastChanged();
       emit changeItem(item, tSpawnChangedHP);
       break;
     }
   }
}

void SpawnShell::renameSpawn(const uint8_t* data)
{
    const spawnRenameStruct* rename = (const spawnRenameStruct*)data;
#ifdef SPAWNSHELL_DIAG
    seqDebug("SpawnShell::renameSpawn(oldname=%s, newname=%s)",
             rename->old_name, rename->new_name);
#endif
    
    Spawn* renameMe = findSpawnByName(rename->old_name);

    if (renameMe != NULL)
    {
        renameMe->setName(rename->new_name);

        uint32_t changeType = tSpawnChangedName;

        if (updateFilterFlags(renameMe))
          changeType |= tSpawnChangedFilter;
        if (updateRuntimeFilterFlags(renameMe))
          changeType |= tSpawnChangedRuntimeFilter;

        renameMe->updateLastChanged();
        emit changeItem(renameMe, changeType);
    }
    else
    {
        seqWarn("SpawnShell: tried to rename %s to %s, but the original mob didn't exist in the spawn list", rename->old_name, rename->new_name);
    }
}

void SpawnShell::illusionSpawn(const uint8_t* data)
{
    const spawnIllusionStruct* illusion = (const spawnIllusionStruct*)data;
#ifdef SPAWNSHELL_DIAG
    seqDebug("SpawnShell::illusionSpawn(id=%d, name=%s, new race=%d)",
             illusion->spawnId, illusion->name, illusion->race);
#endif
    
    Item* item = m_spawns.find(illusion->spawnId);
   
    if (item != NULL)
    {
        Spawn* spawn = (Spawn*) item;

        // Update what we can
        spawn->setGender(illusion->gender);
        spawn->setRace(illusion->race);

        int changed = tSpawnChangedRace;
        if (updateFilterFlags(spawn))
            changed |= tSpawnChangedFilter;
        if (updateRuntimeFilterFlags(spawn))
            changed |= tSpawnChangedRuntimeFilter;
        spawn->updateLastChanged();
        emit changeItem(spawn, changed);
#ifdef SPAWNSHELL_DIAG
        seqDebug("SpawnShell: Illusioned %s (id=%d) into race %d",
                 illusion->name, illusion->spawnId, illusion->race);
#endif
    }
    else
    {
        // Someone with an illusion up zoning in will generate an
        // OP_Illusion BEFORE the OP_NewSpawn, so they won't be
        // in the spawn list. Their spawnStruct will have their
        // illusioned race anyways.
    }
}

void SpawnShell::updateSpawnAppearance(const uint8_t* data)
{
    const spawnAppearanceStruct* app = (const spawnAppearanceStruct*)data;
#ifdef SPAWNSHELL_DIAG
    seqDebug("SpawnShell::updateSpawnAppearance(id=%d, sub=%d, parm=%08x)",
             app->spawnId, app->type, app->parameter);
#endif

   Item* item = m_spawns.find(app->spawnId);

   if (item != NULL)
   {
       Spawn* spawn = (Spawn*)item;

       switch(app->type) 
       {
           case 1: // level update
               {
               spawn->setLevel(app->parameter);
               spawn->setIsInPvPLevelRangeToPlayer(spawn->isOtherPlayer() && abs(m_player->level() - spawn->level()) < 5);
               int changed = tSpawnChangedLevel;
               if (updateFilterFlags(spawn))
                   changed |= tSpawnChangedFilter;
               if (updateRuntimeFilterFlags(spawn))
                   changed |= tSpawnChangedRuntimeFilter;
               spawn->updateLastChanged();
               emit changeItem(spawn, changed);
               }
               break;
           case 5: // light update
               {
               if (spawn->light() != app->parameter)
               {
                   spawn->setLight(app->parameter);
                   int changed = tSpawnChangedWearing;
                   if (updateFilterFlags(spawn))
                       changed |= tSpawnChangedFilter;
                   if (updateRuntimeFilterFlags(spawn))
                       changed |= tSpawnChangedRuntimeFilter;
                   spawn->updateLastChanged();
                   emit changeItem(spawn, changed);
               }
               }
               break;
           case 22: // guild update
               {
               spawn->setGuildID(app->parameter);
               spawn->setGuildTag(m_guildMgr->guildIdToName(spawn->guildID()));
               spawn->setIsPlayersGuildmate(!m_player->guildTag().isEmpty() && spawn->guildID() == m_player->guildID());
               int changed = tSpawnChangedGuild;
               if (updateFilterFlags(spawn))
                   changed |= tSpawnChangedFilter;
               if (updateRuntimeFilterFlags(spawn))
                   changed |= tSpawnChangedRuntimeFilter;
               spawn->updateLastChanged();
               emit changeItem(spawn, changed);
               }
               break;
       }

      /* Other types for OP_SpawnAppearance (from eqemu guys)
#define AT_Die			0	// this causes the client to keel over and zone to bind point
#define AT_WhoLevel		1	// the level that shows up on /who
#define AT_Invis		3	// 0 = visible, 1 = invisible
#define AT_PVP			4	// 0 = blue, 1 = pvp (red)
#define AT_Light		5	// light type emitted by player (lightstone, shiny shield)
#define AT_Anim			14	// 100=standing, 110=sitting, 111=ducking, 115=feigned, 105=looting
#define AT_Sneak		15	// 0 = normal, 1 = sneaking
#define AT_SpawnID		16	// server to client, sets player spawn id
#define AT_HP			17	// Client->Server, my HP has changed (like regen tic)
#define AT_Linkdead		18	// 0 = normal, 1 = linkdead
#define AT_Levitate		19	// 0=off, 1=flymode, 2=levitate
#define AT_GM			20	// 0 = normal, 1 = GM - all odd numbers seem to make it GM
#define AT_Anon			21	// 0 = normal, 1 = anon, 2 = roleplay
#define AT_GuildID		22
#define AT_GuildRank	23	// 0=member, 1=officer, 2=leader
#define AT_AFK			24	// 0 = normal, 1 = afk
#define AT_Pet			25	// Param is EntityID of owner, or 0 for when charm breaks
#define AT_Split		28	// 0 = normal, 1 = autosplit on
#define AT_Size			29	// spawn's size
#define AT_NPCName		31	// change PC's name's color to NPC color 0 = normal, 1 = npc name
#define AT_ShowHelm		43	// 0 = do not show helmet graphic, 1 = show graphic
#define AT_DamageState	44	// The damage state of a destructible object (0 through 4)
//#define AT_Trader		300	// Bazzar Trader Mode
#define AT_NameTagColor	1000	// 0xffffffff is reset/blue, 0xa51414 is pvp/red
       */
   }
}

void SpawnShell::charmUpdate(const uint8_t* data)
{
    const Charm_Struct* update = (const Charm_Struct*)data;
#ifdef SPAWNSHELL_DIAG
    seqDebug("SpawnShell::charmUpdate(owner_id=%d, pet_id=%d, command=%d)",
            update->owner_id, update->pet_id, update->command);
#endif
    Item* item = m_spawns.find(update->pet_id);
    if (item != NULL)
    {
        Spawn* spawn = (Spawn*)item;
        spawn->setPetOwnerID(update->command ? update->owner_id : 0);
        emit changeItem(item, tSpawnChangedPetOwner);
        item->updateLastChanged();
    }
}

void SpawnShell::mobHealthUpdate(const uint8_t* data)
{
  const mobHealthStruct* hpupdate = (const mobHealthStruct*)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::mobHealthUpdate(id=%d, hp=%d)",
           hpupdate->spawnId, hpupdate->hp);
#endif
   Item* item = m_spawns.find(hpupdate->spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     if (spawn->HP() != hpupdate->hp) {
         spawn->setHP(hpupdate->hp);
         spawn->setMaxHP(100);
         emit changeItem(item, tSpawnChangedHP);
     }
     item->updateLastChanged();
   }
}

void SpawnShell::updateNpcHP(const uint8_t* data)
{
  const hpNpcUpdateStruct* hpupdate = (const hpNpcUpdateStruct*)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::updateNpcHP(id=%d, maxhp=%d hp=%d)", 
	  hpupdate->spawnId, hpupdate->maxHP, hpupdate->curHP);
#endif
   Item* item = m_spawns.find(hpupdate->spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     spawn->setHP(hpupdate->curHP);
     spawn->setMaxHP(hpupdate->maxHP);
     item->updateLastChanged();
     emit changeItem(item, tSpawnChangedHP);
   }
}

void SpawnShell::spawnWearingUpdate(const uint8_t* data)
{
  const wearChangeStruct *wearing = (const wearChangeStruct *)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::spawnWearingUpdate(id=%d, slot=%d, material=%d)",
           wearing->spawnId, wearing->wearSlotId, wearing->materialId);
#endif
  Item* item = m_spawns.find(wearing->spawnId);
  if (item != NULL)
  {
    // ZBTEMP: Find newItemID
    Spawn* spawn = (Spawn*)item;
    spawn->setEquipment(wearing->wearSlotId, wearing->materialId);

    uint32_t changeType = tSpawnChangedWearing;
    if (updateFilterFlags(item))
      changeType |= tSpawnChangedFilter;
    if (updateRuntimeFilterFlags(item))
      changeType |= tSpawnChangedRuntimeFilter;
    item->updateLastChanged();
    emit changeItem(item, changeType);
  }
}

void SpawnShell::consMessage(const uint8_t* data, size_t, uint8_t dir) 
{
  const considerStruct * con = (const considerStruct*)data;
  if (con->playerid != con->targetid)
  {
      Item* item = m_spawns.find(con->targetid);
      if (item != NULL)
      {
          Spawn* spawn = (Spawn*)item;

          // note that this spawn has been considered
          spawn->setConsidered(true);

          emit spawnConsidered(item);
      }
  }
}

void SpawnShell::deleteSpawn(const uint8_t* data)
{
  const deleteSpawnStruct* delspawn = (const deleteSpawnStruct*)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::deleteSpawn(id=%d)", delspawn->spawnId);
#endif
   if (m_posDeadSpawnIDs < (MAX_DEAD_SPAWNIDS - 1))
     m_posDeadSpawnIDs++;
   else
     m_posDeadSpawnIDs = 0;
   
   if (m_cntDeadSpawnIDs < MAX_DEAD_SPAWNIDS)
     m_cntDeadSpawnIDs++;

   m_deadSpawnID[m_posDeadSpawnIDs] = delspawn->spawnId;

   deleteItem(tSpawn, delspawn->spawnId);
}

void SpawnShell::killSpawn(const uint8_t* data)
{
  const Death_Struct* deadspawn = (const Death_Struct*)data;
#ifdef SPAWNSHELL_DIAG
   seqDebug("SpawnShell::killSpawn(id=%d, kid=%d)", 
	  deadspawn->spawnId, deadspawn->killerId);
#endif
   Item* item;

   item = m_spawns.find(deadspawn->spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     // ZBTEMP: This is temporary until we can find a better way
     // set the last kill info on the player (do this before changing name)
     m_player->setLastKill(spawn->name(), spawn->level());

     Item* killer;
     killer = m_spawns.find(deadspawn->killerId);
     emit killingSpawn(item, killer, deadspawn->killerId);
     spawn->killSpawn();
     int changeFlags = tSpawnChangedNPC;
     if (updateFilterFlags(spawn))
         changeFlags |= tSpawnChangedFilter;
     if (updateRuntimeFilterFlags(spawn))
         changeFlags |= tSpawnChangedRuntimeFilter;
     spawn->updateLastChanged();
     emit changeItem(spawn, changeFlags);

     spawn->setName(spawn->realName() + Spawn_Corpse_Designator);

     emit killSpawn(item, killer, deadspawn->killerId);
   }
}

void SpawnShell::corpseLoc(const uint8_t* data)
{
  const BecomeCorpse_Struct* corpseLoc = (const BecomeCorpse_Struct*)data;
  Item* item = m_spawns.find(corpseLoc->spawn_id);
  if (item != NULL)
  {
    Spawn* spawn = (Spawn*)item;
        spawn->setPos(int16_t(corpseLoc->x), int16_t(corpseLoc->y), 
                      int16_t(corpseLoc->z),
                      showeq_params->walkpathrecord,
                      showeq_params->walkpathlength);
    spawn->updateLast();
  }
}

void SpawnShell::playerChangedID(uint16_t playerID)
{
  // remove the player from the list (if it had a 0 id)
  m_players.take(0);

  // re-insert the player into the list
  m_players.replace(playerID, m_player);

  emit changeItem(m_player, tSpawnChangedALL);
}

void SpawnShell::playerGuildChanged(uint16_t guildID)
{
    ItemMap& theMap = getMap(tSpawn);
    ItemIterator it(theMap);

    Spawn* spawn;
    for (; it.current(); ++it)
    {
        spawn = (Spawn*)it.current();
        spawn->setIsPlayersGuildmate(!m_player->guildTag().isEmpty() && spawn->guildID() == m_player->guildID());
    }

    refilterSpawns(tSpawn);
    refilterSpawnsRuntime(tSpawn);
}

void SpawnShell::playerLevelChanged(uint8_t playerLevel)
{
    ItemMap& theMap = getMap(tSpawn);
    ItemIterator it(theMap);

    Spawn* spawn;
    for (; it.current(); ++it)
    {
        spawn = (Spawn*)it.current();
        spawn->setIsInPvPLevelRangeToPlayer(spawn->isOtherPlayer() && abs(m_player->level() - spawn->level()) < 5);
    }

    refilterSpawns(tSpawn);
    refilterSpawnsRuntime(tSpawn);
}

void SpawnShell::refilterSpawns()
{
  refilterSpawns(tSpawn);
  refilterSpawns(tDrop);
  refilterSpawns(tDoors);
}

void SpawnShell::refilterSpawns(spawnItemType type)
{
   ItemMap& theMap = getMap(type);
   ItemIterator it(theMap);

   if (type == tSpawn)
   {
     Spawn* spawn;
     // iterate over all the items in the map
     for (; it.current(); ++it)
     {
       // get the item
       spawn = (Spawn*)it.current();
       
       // update the flags, if they changed, send a notification
       if (updateFilterFlags(spawn))
       {
	 spawn->updateLastChanged();
	 emit changeItem(spawn, tSpawnChangedFilter);
       }
     }
   }
   else
   {
     Item* item;
     // iterate over all the items in the map
     for (; it.current(); ++it)
     {
       // get the item
       item = it.current();
       
       // update the flags, if they changed, send a notification
       if (updateFilterFlags(item))
       {
	 item->updateLastChanged();
	 emit changeItem(item, tSpawnChangedFilter);
       }
     }
   }
}

void SpawnShell::refilterSpawnsRuntime()
{
  refilterSpawnsRuntime(tSpawn);
  refilterSpawnsRuntime(tDrop);
  refilterSpawnsRuntime(tDoors);
}

void SpawnShell::refilterSpawnsRuntime(spawnItemType type)
{
   ItemIterator it(getMap(type));

   if (type == tSpawn)
   {
     Spawn* spawn;
     // iterate over all the items in the map
     for (; it.current(); ++it)
     {
       // get the item
       spawn = (Spawn*)it.current();
       
       // update the flags, if they changed, send a notification
       if (updateRuntimeFilterFlags(spawn))
       {
	 spawn->updateLastChanged();
	 emit changeItem(spawn, tSpawnChangedRuntimeFilter);
       }
     }
   }
   else
   {
     Item* item;
     // iterate over all the items in the map
     for (; it.current(); ++it)
     {
       // get the item
       item = it.current();
       
       // update the flags, if they changed, send a notification
       if (updateRuntimeFilterFlags(item))
       {
	 item->updateLastChanged();
	 emit changeItem(item, tSpawnChangedRuntimeFilter);
       }
     }
   }
}

void SpawnShell::saveSpawns(void)
{
  QFile keyFile(showeq_params->saveRestoreBaseFilename + "Spawns.dat");
  if (keyFile.open(IO_WriteOnly))
  {
    QDataStream d(&keyFile);

    // write the magic string
    d << *magic;

    // write a test value at the top of the file for a validity check
    size_t testVal = sizeof(spawnStruct);
    d << testVal;

    // save the name of the current zone
    d << m_zoneMgr->shortZoneName().lower();

    // save the spawns
    ItemMap& theMap = getMap(tSpawn);

    // save the number of spawns
    testVal = theMap.count();
    d << testVal;

    ItemIterator it(theMap);
    Spawn* spawn;

    // iterate over all the items in the map
    for (; it.current(); ++it)
    {
      // get the spawn
      spawn = (Spawn*)it.current();

      // save the spawn id
      d << spawn->id();

      // save the spawn
      spawn->saveSpawn(d);
    }
  }

   // re-start the timer
   if (showeq_params->saveSpawns)
     m_timer->start(showeq_params->saveSpawnsFrequency, true);
}

void SpawnShell::restoreSpawns(void)
{
  QString fileName = showeq_params->saveRestoreBaseFilename + "Spawns.dat";
  QFile keyFile(fileName);
  if (keyFile.open(IO_ReadOnly))
  {
    size_t i;
    size_t testVal;
    uint16_t id;
    Spawn* item;

    QDataStream d(&keyFile);

    // check the magic string
    uint32_t magicTest;
    d >> magicTest;

    if (magicTest != *magic)
    {
      seqWarn("Failure loading %s: Bad magic string!",
	      (const char*)fileName);
      return;
    }

    // check the test value at the top of the file
    d >> testVal;
    if (testVal != sizeof(spawnStruct))
    {
      seqWarn("Failure loading %s: Bad spawnStruct size!",
	      (const char*)fileName);
      return;
    }

    // attempt to validate that the info is from the current zone
    QString zoneShortName;
    d >> zoneShortName;
    if (zoneShortName != m_zoneMgr->shortZoneName().lower())
    {
      seqWarn("\aWARNING: Restoring spawns for potentially incorrect zone (%s != %s)!",
	      (const char*)zoneShortName, 
	      (const char*)m_zoneMgr->shortZoneName().lower());
    }

    // read the expected number of elements
    d >> testVal;

    // read in the spawns
    for (i = 0; i < testVal; i++)
    {
      // get the spawn id
      d >> id;

      // re-create the spawn
      item = new Spawn(d, id);

      // filter and add it to the list
      updateFilterFlags(item);
      updateRuntimeFilterFlags(item);
      m_spawns.insert(id, item);
      emit addItem(item);
    }

    emit numSpawns(m_spawns.count());

    seqInfo("Restored SPAWNS: count=%d!",
	    m_spawns.count());
  }
  else
  {
    seqWarn("Failure loading %s: Unable to open!",
	    (const char*)fileName);
  }
}

#include "spawnshell.moc"
