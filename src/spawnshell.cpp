/*
 * spawnshell.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

/*
 * Adapted from spawnlist.cpp - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date   - 7/31/2001
 */


#include <qfile.h>
#include <qdatastream.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <limits.h>
#include <math.h>

#include "spawnshell.h"
#include "filtermgr.h"
#include "zonemgr.h"
#include "player.h"
#include "util.h"
#include "itemdb.h"
#include "guild.h"

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

//----------------------------------------------------------------------
// Handy utility function
#ifdef SPAWNSHELL_NAME_VALIDATE
static bool isValidName(const char* name, uint32_t len)
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
		       EQItemDB* itemDB,
                       GuildMgr* guildMgr)
  : QObject(NULL, "spawnshell"),
    m_zoneMgr(zoneMgr),
    m_player(player),
    m_filterMgr(filterMgr),
    m_itemDB(itemDB),
    m_guildMgr(guildMgr),
    m_spawns(701),
    m_drops(211),
    m_coins(101),
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
   m_coins.setAutoDelete(true);
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

   // connect SpawnShell slots to ZoneMgr signals
   connect(m_zoneMgr, SIGNAL(zoneBegin(const QString&)),
	   this, SLOT(clear(void)));
   connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
	   this, SLOT(clear(void)));

   // connect Player signals to SpawnShell signals
   connect(m_player, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SIGNAL(changeItem(const Item*, uint32_t)));

   // connect Player signals to SpawnShell slots
   connect(m_player, SIGNAL(changedID(uint16_t)),
	   this, SLOT(playerChangedID(uint16_t)));
   connect(m_player, SIGNAL(getPlayerGuildTag()),
	   this, SLOT(setPlayerGuildTag()));

   // restore the spawn list if necessary
   if (showeq_params->restoreSpawns)

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
   printf("SpawnShell::clear()\n");
#endif

   emit clearItems();

   m_spawns.clear();
   m_coins.clear();
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

const Item* SpawnShell::findID(itemType type, int id)
{
  const Item* item = NULL;
  
  if (type != tPlayer)
    item = getMap(type).find(id);

  if (item != NULL)
    return item;

  if ((type == tSpawn) && (id == m_player->id()))
    item = m_player;

  return item;
}

const Item* SpawnShell::findClosestItem(itemType type, 
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

const Spawn* SpawnShell::findSpawnByName(const QString& name)
{
  ItemIterator it(m_spawns);
  const Spawn* spawn;

  for (; it.current(); ++it)
  {
    // the item and coerce it to the Spawn type
    spawn = (const Spawn*)it.current();

    if (name == spawn->name())
      return spawn;
  }

  if (name == m_player->name())
    return m_player;

  return NULL;
}

void SpawnShell::deleteItem(itemType type, int id)
{
#ifdef SPAWNSHELL_DIAG
   printf ("SpawnShell::deleteItem()\n");
#endif
   ItemMap& theMap = getMap(type);

   Item* item = theMap.find(id);

   if (item != NULL)
   {
     if (item->filterFlags() & FILTER_FLAG_ALERT)
       emit handleAlert(item, tDelSpawn);
     
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
  uint32_t flags = m_filterMgr.filterFlags(item->filterString(), level);

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
  uint32_t flags = m_filterMgr.runtimeFilterFlags(item->filterString(), level);

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

void SpawnShell::dumpSpawns(itemType type, QTextStream& out)
{
   ItemIterator it(getMap(type));

   for (; it.current(); ++it)
     out << it.current()->dumpString() << endl;
}

// same-name slots, connecting to Packet signals
void SpawnShell::newGroundItem(const makeDropStruct *d)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newGroundItem(makeDropStruct *)\n");
#endif
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  if (!d)
    return;
  
  // attempt to get the item name
  QString name;
  if (m_itemDB != NULL)
    name = m_itemDB->GetItemLoreName(d->itemNr);
  
  Drop* item = (Drop*)m_drops.find(d->dropId);
  if (item != NULL)
  {
    item->update(d, name);
    updateFilterFlags(item);
    item->updateLastChanged();
    emit changeItem(item, tSpawnChangedALL);
  }
  else
  {
    item = new Drop(d, name);
    updateFilterFlags(item);
    m_drops.insert(d->dropId, item);
    emit addItem(item);
  }

  if (item->filterFlags() & FILTER_FLAG_ALERT)
    emit handleAlert(item, tNewSpawn);
}

void SpawnShell::removeGroundItem(const remDropStruct *d)
{
#ifdef SPAWNSHELL_DIAG
  printf("SpawnShell::removeGroundItem(remDropStruct *)\n");
#endif
  if (d)
    deleteItem(tDrop, d->dropId);
}

void SpawnShell::compressedDoorSpawn(const cDoorSpawnsStruct *c)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::compressedDoorSpawn(compressedDoorStruct*)\n");
#endif
   for (int i=0; i<c->count; i++)
   {
      newDoorSpawn((const doorStruct*)&c->doors[i]);
   }
}

void SpawnShell::newDoorSpawn(const doorStruct* d)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newDoorSpawn(doorStruct*)\n");
#endif
   Item* item = m_doors.find(d->doorId);
   if (item != NULL)
   {
     Door* door = (Door*)item;
     door->update(d);
     updateFilterFlags(door);
     item->updateLastChanged();
     emit changeItem(door, tSpawnChangedALL);
   }
   else
   {
     item = (Item*)new Door(d);
     updateFilterFlags(item);
     m_doors.insert(d->doorId, item);
     emit addItem(item);
   }

   if (item->filterFlags() & FILTER_FLAG_ALERT)
     emit handleAlert(item, tNewSpawn);
}


void SpawnShell::newCoinsItem(const dropCoinsStruct *c)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newCoinsItem(dropCoinsStruct*)\n");
#endif
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  if (!c)
    return;

  Item* item = m_coins.find(c->dropId);
  if (item != NULL)
  {
    Coin* coin = (Coin*)item;
    coin->update(c);
    updateFilterFlags(item);
    item->updateLastChanged();
    emit changeItem(item, tSpawnChangedALL);
  }
  else
  {
    item = new Coin(c);
    updateFilterFlags(item);
    m_coins.insert(c->dropId, item);
    emit addItem(item);
  }
  
  if (item->filterFlags() & FILTER_FLAG_ALERT)
    emit handleAlert(item, tNewSpawn);
}

void SpawnShell::removeCoinsItem(const removeCoinsStruct *c)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::removeCoinsItem(removeCoinsStruc *)\n");
#endif
   if (c)
      deleteItem(tDrop, c->dropId);
}

void SpawnShell::zoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len)
{
  int spawndatasize = (len - 2) / sizeof(spawnStruct);

  for (int i = 0; i < spawndatasize; i++)
    newSpawn(zspawns->spawn[i].spawn);
}

void SpawnShell::newSpawn(const newSpawnStruct* spawn)
{
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  newSpawn(spawn->spawn);
}

void SpawnShell::newSpawn(const spawnStruct& s)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newSpawn(spawnStruct *(name='%s'), bSelected=%s)\n", s.name, bSelected?"true":"false");
#endif
   
   // if this is the SPAWN_SELF it's the player
   if (s.NPC == SPAWN_SELF)
     return;

   // not the player, so check if it's a recently deleted spawn
   for (int i =0; i < m_cntDeadSpawnIDs; i++)
   {
     if ((m_deadSpawnID[i] != 0) && (m_deadSpawnID[i] == s.spawnId))
     {
       // found a match, remove it from the deleted spawn list
       m_deadSpawnID[i] = 0;

       // let the user know what's going on
       printf("%s(%d) has already been removed from the zone before we processed it.\n", 
	      s.name, s.spawnId);
       
       // and stop the attempt to add the spawn.
       return;
     }
   }
   
   Item* item = m_spawns.find(s.spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     spawn->update(&s);
     updateFilterFlags(spawn);
     updateRuntimeFilterFlags(spawn);
     item->updateLastChanged();

     if (spawn->GuildID() < MAXGUILDS)
        spawn->setGuildTag(m_guildMgr->guildIdToName(spawn->GuildID()));

     emit changeItem(item, tSpawnChangedALL);
   }
   else
   {
     item = new Spawn(&s);
     Spawn* spawn = (Spawn*)item;
     updateFilterFlags(spawn);
     updateRuntimeFilterFlags(spawn);
     m_spawns.insert(s.spawnId, item);

     if (spawn->GuildID() < MAXGUILDS)
        spawn->setGuildTag(m_guildMgr->guildIdToName(spawn->GuildID()));

     emit addItem(item);

     // send notification of new spawn count
     emit numSpawns(m_spawns.count());
   }

   if (item->filterFlags() & FILTER_FLAG_ALERT)
     emit handleAlert(item, tNewSpawn);
}

void SpawnShell::playerUpdate(const playerPosStruct *pupdate, uint32_t, uint8_t dir)
{
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

  if ((dir != DIR_CLIENT) && 
      (pupdate->spawnId != m_player->id())) // PC Corpse Movement
  {
       updateSpawn(pupdate->spawnId,  
                   pupdate->x,
                   pupdate->y,
                   pupdate->z,
                   0, 0, 0,
                   pupdate->heading,
                   0, 0);
  }
}

void SpawnShell::updateSpawn(uint16_t id, 
			     int16_t x, int16_t y, int16_t z,
			     int16_t xVel, int16_t yVel, int16_t zVel,
			     int8_t heading, int8_t deltaHeading,
			     uint8_t animation)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::updateSpawn(id=%d, x=%d, y=%d, z=%d, xVel=%d, yVel=%d, zVel=%d)\n", id, x, y, z, xVel, yVel, zVel);
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

     spawn->updateLast();
     item->updateLastChanged();
     emit changeItem(item, tSpawnChangedPosition);
   }
   else if (showeq_params->createUnknownSpawns)
   {
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

	 printf("(%d) had been removed from the zone, but saw a position update on it, so assuming bogus update.\n", 
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

void SpawnShell::updateSpawns(const mobUpdateStruct* updates)
{
  // if zoning, then don't do anything
  if (m_zoneMgr->isZoning())
    return;

   for (int a = 0; a < updates->numUpdates; a++)
   {
     // I have done some checking.  It appears that this feild
     // (animation) contains the curent animation loop of the mob
     // in question (walking/running/standing etc..
     // 0 = staning.  When this is 0 the mob stops moving in the game
     // even though the velocity numbers arent 0.  this fix should reduce
     // drift when a mob stops moving. (or turns)

     updateSpawn(updates->spawnUpdate[a].spawnId,
		 updates->spawnUpdate[a].x,
		 updates->spawnUpdate[a].y,
		 updates->spawnUpdate[a].z,
		 updates->spawnUpdate[a].deltaX,
		 updates->spawnUpdate[a].deltaY,
		 updates->spawnUpdate[a].deltaZ,
		 updates->spawnUpdate[a].heading,
		 updates->spawnUpdate[a].deltaHeading,
		 updates->spawnUpdate[a].animation);
   }
}

void SpawnShell::updateSpawnHP(const hpUpdateStruct* hpupdate)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::updateSpawnHp(id=%d, hp=%d, maxHp=%d)\n", 
	  hpupdate->spawnId, hpupdate->curHp, hpupdate->maxHp);
#endif
   Item* item = m_spawns.find(hpupdate->spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     spawn->setHP(hpupdate->curHp);
     spawn->setMaxHP(hpupdate->maxHp);
     item->updateLastChanged();
     emit changeItem(item, tSpawnChangedHP);
   }
}

void SpawnShell::spawnWearingUpdate(const wearChangeStruct *wearing)
{
  Item* item = m_spawns.find(wearing->spawnId);
  if (item != NULL)
  {
    Spawn* spawn = (Spawn*)item;
    spawn->setEquipment(wearing->wearSlotId, wearing->newItemId);
    uint32_t changeType = tSpawnChangedWearing;
    if (updateFilterFlags(item))
      changeType |= tSpawnChangedFilter;
    if (updateRuntimeFilterFlags(item))
      changeType |= tSpawnChangedRuntimeFilter;
    item->updateLastChanged();
    emit changeItem(item, changeType);
  }
}

void SpawnShell::consMessage(const considerStruct * con, uint32_t, uint8_t dir) 
{
  Item* item;
  Spawn* spawn;

  if (dir == DIR_CLIENT)
  {
    if (con->playerid != con->targetid) 
    {
      item = m_spawns.find(con->targetid);
      if (item != NULL)
      {
	spawn = (Spawn*)item;

	// note that this spawn has been considered
	spawn->setConsidered(true);
	
	emit spawnConsidered(item);
      }
    }
    return;
  }

  QString lvl("");
  QString hps("");
  QString cn("");

  QString msg("Faction: Your faction standing with ");

  // is it you that you've conned?
  if (con->playerid == con->targetid) 
  {
    // print it's deity
    printf("Diety: %s\n", (const char*)m_player->deityName());
    
    // well, this is You
    msg += "YOU";
  }
  else 
  {
    // find the spawn if it exists
    item = m_spawns.find(con->targetid);
    
    // has the spawn been seen before?
    if (item != NULL)
    {
      Spawn* spawn = (Spawn*)item;
      // yes
      printf("Diety: %s\n", (const char*)spawn->deityName());

      int changed = tSpawnChangedNone;

      /* maxhp and curhp are available when considering players, */
      /* but not when considering mobs. */
      if (con->maxHp || con->curHp)
      {
         if (spawn->NPC() == SPAWN_NPC_UNKNOWN)
         {
	   spawn->setNPC(SPAWN_PLAYER);        // player
	   changed |= tSpawnChangedNPC;
         }
         spawn->setMaxHP(con->maxHp);
         spawn->setHP(con->curHp);
         changed |= tSpawnChangedHP;
      }
      else if (item->NPC() == SPAWN_NPC_UNKNOWN)
      {
         spawn->setNPC(SPAWN_NPC);
         changed |= tSpawnChangedNPC;
      }

      // note the updates if any
      if (changed != tSpawnChangedNone)
      {
        if (updateFilterFlags(item))
           changed |= tSpawnChangedFilter;
        if (updateRuntimeFilterFlags(item))
           changed |= tSpawnChangedRuntimeFilter;

	item->updateLastChanged();
        emit changeItem(item, changed);
      }

      // note that this spawn has been considered
      spawn->setConsidered(true);

      emit spawnConsidered(item);

      msg += item->name();
    } // end if spawn found
    else
      msg += "Spawn:" + QString::number(con->targetid, 16);
  } // else not yourself
  
  switch (con->level) 
  {
     case 0:
     {
        cn.sprintf(" (even)");
        break;
     }
     case 2:
     {
        cn.sprintf(" (green)");
        break;
     }
     case 4:
     {
        cn.sprintf(" (blue)");
        break;
     }
     case 13:
     {
        cn.sprintf(" (red)");
        break;
     }
     case 15:
     {
        cn.sprintf(" (yellow)");
        break;
     }
     case 18:
     {
        cn.sprintf(" (cyan)");
        break;
     }
     default:
     {
        cn.sprintf(" (unknown: %d)", con->level);
        break;
     }
  }

  msg += cn;

  if (con->maxHp || con->curHp)
  {
    lvl.sprintf(" (%i/%i HP)", con->curHp, con->maxHp);
    msg += lvl;
  }
  
  msg += QString(" is: ") + print_faction(con->faction) + " (" 
    + QString::number(con->faction) + ")!";
  
  emit msgReceived(msg);
} // end consMessage()

void SpawnShell::deleteSpawn(const deleteSpawnStruct* delspawn)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::deleteSpawn(id=%d)\n", delspawn->spawnId);
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

void SpawnShell::killSpawn(const newCorpseStruct* deadspawn)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::killSpawn(id=%d, kid=%d)\n", 
	  deadspawn->spawnId, deadspawn->killerId);
#endif
   Item* item;
   Item* killer;

   item = m_spawns.find(deadspawn->spawnId);
   if (item != NULL)
   {
     Spawn* spawn = (Spawn*)item;
     killer = m_spawns.find(deadspawn->killerId);

     // ZBTEMP: This is temporary until we can find a better way
     // set the last kill info on the player (do this before changing name)
     m_player->setLastKill(spawn->name(), spawn->level());

     spawn->killSpawn();
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     emit killSpawn(item, killer, deadspawn->killerId);

     if (item->filterFlags() & FILTER_FLAG_ALERT)
       emit handleAlert(item, tKillSpawn);
   }
}

void SpawnShell::corpseLoc(const corpseLocStruct* corpseLoc)
{
  Item* item = m_spawns.find(corpseLoc->spawnId);
  if (item != NULL)
  {
    Spawn* spawn = (Spawn*)item;

    // set the corpses location, and make sure it's not moving... 
    spawn->setPos(int16_t(corpseLoc->x), int16_t(corpseLoc->y), 
		  int16_t(corpseLoc->z),
		  showeq_params->walkpathrecord,
		  showeq_params->walkpathlength);
    spawn->killSpawn();
    spawn->updateLast();
    spawn->updateLastChanged();
    
    // signal that the spawn has changed
    emit killSpawn(item, NULL, 0);
  }
}

void SpawnShell::backfillZoneSpawns(const zoneSpawnsStruct* zdata, uint32_t len)
{
  int zoneSpawnsStructHeaderData = 
    ((uint8_t*)&zdata->spawn[0]) - (uint8_t*)zdata;

  int zoneSpawnsStructPayloadCount = 
       (len - zoneSpawnsStructHeaderData) / sizeof(spawnZoneStruct);

  for (int j = 0; j < zoneSpawnsStructPayloadCount; j++)
    backfillSpawn(&zdata->spawn[j].spawn);

  // if the spawns are to be saved, this is a good time to do it.
  if (showeq_params->saveSpawns)
  {
    // stop the timer because saveSpawns will reset it.
    m_timer->stop();

    // save the spawns
    saveSpawns();
  }
}

void SpawnShell::backfillSpawn(const newSpawnStruct *ndata)
{
  const spawnStruct* sdata = &ndata->spawn;
  backfillSpawn(sdata);
}

void SpawnShell::backfillSpawn(const spawnStruct *spawn)
{
  Item* item = m_spawns.find(spawn->spawnId);
  
  if (item == NULL)
  {
    // if it's not already in the list, then just add it.
    newSpawn(*spawn);

    return;
  }

  Spawn* spawnItem = (Spawn*)item;

  // if we got the self item, then somethings screwy, so only update if
  // not the self spawn
  if (!spawnItem->isSelf())
  {
    spawnItem->backfill(spawn);
    updateFilterFlags(spawnItem);
    updateRuntimeFilterFlags(spawnItem);

    if (spawnItem->GuildID() < MAXGUILDS)
        spawnItem->setGuildTag(m_guildMgr->guildIdToName(spawnItem->GuildID()));

    spawnItem->updateLastChanged();
    emit changeItem(spawnItem, tSpawnChangedALL);
    
    if (spawnItem->filterFlags() & FILTER_FLAG_ALERT)
      emit handleAlert(spawnItem, tFilledSpawn);
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

void SpawnShell::setPlayerGuildTag()
{
    m_player->setGuildTag(m_guildMgr->guildIdToName(m_player->GuildID()));
}

void SpawnShell::refilterSpawns()
{
  refilterSpawns(tSpawn);
  refilterSpawns(tDrop);
  refilterSpawns(tCoins);
  refilterSpawns(tDoors);
}

void SpawnShell::refilterSpawns(itemType type)
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
  refilterSpawnsRuntime(tCoins);
  refilterSpawnsRuntime(tDoors);
}

void SpawnShell::refilterSpawnsRuntime(itemType type)
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
      fprintf(stderr, 
	      "Failure loading %s: Bad magic string!\n",
	      (const char*)fileName);
      return;
    }

    // check the test value at the top of the file
    d >> testVal;
    if (testVal != sizeof(spawnStruct))
    {
      fprintf(stderr, 
	      "Failure loading %s: Bad spawnStruct size!\n",
	      (const char*)fileName);
      return;
    }

    // attempt to validate that the info is from the current zone
    QString zoneShortName;
    d >> zoneShortName;
    if (zoneShortName != m_zoneMgr->shortZoneName().lower())
    {
      fprintf(stderr,
	      "\aWARNING: Restoring spawns for potentially incorrect zone (%s != %s)!\n",
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

    fprintf(stderr,
	    "Restored SPAWNS: count=%d!\n",
	    m_spawns.count());
  }
  else
  {
    fprintf(stderr,
	    "Failure loading %s: Unable to open!\n",
	    (const char*)fileName);
  }
}
