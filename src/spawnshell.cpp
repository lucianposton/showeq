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

#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <limits.h>
#include <math.h>

#include "spawnshell.h"
#include "util.h"
#include "itemdb.h"

//----------------------------------------------------------------------
// useful macro definitions

// define this to have the spawnshell print diagnostics
//#define SPAWNSHELL_DIAG

// define this to diagnose structures passed in to SpawnShell
//#define SPAWNSHELL_DIAG_STRUCTS

// define this to diagnose issues with the PlayerSpawn
//#define SPAWNSHELL_DIAG_PLAYERSPAWN

// define this to have the spawnshell validate names to help spot errors
//#define SPAWNSHELL_NAME_VALIDATE

//----------------------------------------------------------------------
// constants

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
SpawnShell::SpawnShell(FilterMgr& filterMgr, EQPlayer* player)
  : QObject(NULL, "spawnshell"),
    m_player(player),
    m_filterMgr(filterMgr)
{
   m_playerIdSet = false;
   m_playerId = 0;
   m_playerSpawn = NULL;
   m_cntDeadSpawnIDs = 0;
   m_posDeadSpawnIDs = 0;
   memset((void*)m_deadSpawnID, 0, sizeof(m_deadSpawnID));

   // connect the FilterMgr's signals to SpawnShells slots
   connect(&m_filterMgr, SIGNAL(filtersChanged()),
	   this, SLOT(refilterSpawns()));
   connect(&m_filterMgr, SIGNAL(runtimeFiltersChanged(uint8_t)),
	   this, SLOT(refilterSpawnsRuntime()));

   // connect SpawnShell slots to EQPlayer signals
   connect(m_player, SIGNAL(setID(uint16_t)),
	   this, SLOT(setPlayerID(uint16_t)));
}

void SpawnShell::clearMap(ItemMap& map)
{
  ItemMap::iterator it;
  Item* item;

  it = map.begin();
  while (it != map.end())
  {
    item = it->second;
    map.erase(it);
    it++;
    delete item;
  }
}

void SpawnShell::clear(void)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::clear()\n");
#endif
   emit clearItems();

   clearMap(m_spawns);
   clearMap(m_coins);
   clearMap(m_drops);
   m_playerIdSet = false;
   m_playerId = 0;
   m_playerSpawn = NULL;
   m_cntDeadSpawnIDs = 0;
   m_posDeadSpawnIDs = 0;
   memset((void*)m_deadSpawnID, 0, sizeof(m_deadSpawnID));
} // end clear

const Item* SpawnShell::findID(itemType type, int id)
{
   ItemMap& theMap = getMap(type);
   ItemMap::iterator it;

   it = theMap.find(id);
   if (it != theMap.end())
     return it->second;

   return NULL;
}

const Item* SpawnShell::findClosestItem(itemType type, 
						  int16_t x, int16_t y,
						  double& minDistance)
{
   ItemMap& theMap = getMap(type);
   ItemMap::iterator it;
   double distance;
   Item* item;
   Item* closest = NULL;

   // find closest spawn

   // iterate over all the items in the map
   for (it = theMap.begin(); it != theMap.end(); it++)
   {
     // get the item
     item = it->second;
     
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

const Spawn* SpawnShell::findSpawnByRawName(const QString& name)
{
  ItemMap::iterator it;
  const Spawn* spawn;

  for (it = m_spawns.begin(); 
       it != m_spawns.end();
       it++)
  {
    // the item and coerce it to the Spawn type
    spawn = (const Spawn*)it->second;

    if (name == spawn->rawName())
      return spawn;
  }

  return NULL;
}

void SpawnShell::deleteItem(itemType type, int id)
{
#ifdef SPAWNSHELL_DIAG
   printf ("SpawnShell::deleteItem()\n");
#endif
   ItemMap& theMap = getMap(type);
   ItemMap::iterator it;

   it = theMap.find(id);

   if (it != theMap.end())
   {
     Item* item = it->second;

     if (item->filterFlags() & FILTER_FLAG_ALERT)
       emit handleAlert(item, tDelSpawn);
     
     emit delItem(item);
     item = it->second;
     theMap.erase(it);
     delete item;

     // send notifcation of new spawn count
     emit numSpawns(m_spawns.size());
   }
}

bool SpawnShell::updateFilterFlags(Item* item)
{
  // get the filter flags
  uint32_t flags = m_filterMgr.filterFlags(item->filterString(), 0);

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

bool SpawnShell::updateFilterFlags(Spawn* spawn)
{
  // get the filter flags
  uint32_t flags = m_filterMgr.filterFlags(spawn->filterString(), 
					   spawn->level());

  // see if the new filter flags are different from the old ones
  if (flags != spawn->filterFlags())
  {
    // yes, set the new filter flags
    spawn->setFilterFlags(flags);

    // return true to indicate that the flags have changed
    return true;
  }

  // flags haven't changed
  return false;
}

bool SpawnShell::updateRuntimeFilterFlags(Item* item)
{
  // get the filter flags
  uint32_t flags = m_filterMgr.runtimeFilterFlags(item->filterString(), 0);

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

bool SpawnShell::updateRuntimeFilterFlags(Spawn* spawn)
{
  // get the filter flags
  uint32_t flags = m_filterMgr.runtimeFilterFlags(spawn->filterString(), 
						  spawn->level());

  // see if the new filter flags are different from the old ones
  if (flags != spawn->runtimeFilterFlags())
  {
    // yes, set the new filter flags
    spawn->setRuntimeFilterFlags(flags);

    // return true to indicate that the flags have changed
    return true;
  }

  // flags haven't changed
  return false;
}

void SpawnShell::dumpSpawns(itemType type, QTextStream& out)
{
   ItemMap& theMap = getMap(type);
   ItemMap::iterator it;

   for (it = theMap.begin(); it != theMap.end(); it++)
     out << it->second->dumpString() << endl;
}

// same-name slots, connecting to Packet signals
void SpawnShell::newGroundItem(const dropThingOnGround *d)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newGroundItem(dropThingOnGround *)\n");
#endif
   if (!d)
     return;
   ItemMap::iterator it;

   // attempt to get the item name
   QString name;
   if (pItemDB != NULL)
     name = pItemDB->GetItemLoreName(d->itemNr);
  
   Drop* item;
   it = m_drops.find(d->dropId);
   if (it != m_drops.end())
   {
     item = (Drop*)it->second;
     item->update(d, name);
     updateFilterFlags(item);
     emit changeItem(item, tSpawnChangedALL);
   }
   else
   {
     item = new Drop(d, name);
     updateFilterFlags(item);
     m_drops.insert(ItemMap::value_type(d->dropId, item));
     emit addItem(item);
   }

   if (item->filterFlags() & FILTER_FLAG_ALERT)
     emit handleAlert(item, tNewSpawn);
}

void SpawnShell::removeGroundItem(const removeThingOnGround *d)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::removeGroundItem(removeThingOnGround *)\n");
#endif
   if (d)
      deleteItem(tDrop, d->dropId);
}

void SpawnShell::newCoinsItem(const dropCoinsStruct *c)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newCoinsItem(dropCoinsStruct*)\n");
#endif
   if (!c)
     return;
   ItemMap::iterator it;
   Coin* item;
   it = m_coins.find(c->dropId);
   if (it != m_coins.end())
   {
     item = (Coin*)it->second;
     item->update(c);
     updateFilterFlags(item);
     emit changeItem(item, tSpawnChangedALL);
   }
   else
   {
     item = new Coin(c);
     updateFilterFlags(item);
     m_coins.insert(ItemMap::value_type(c->dropId, item));
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

void SpawnShell::zoneSpawns(const zoneSpawnsStruct* zspawns, int len)
{
  int spawndatasize = (len - 2) / sizeof(spawnStruct);

  for (int i = 0; i < spawndatasize; i++)
  {
    newSpawn(zspawns->spawn[i].spawn);
  }
}

void SpawnShell::newSpawn(const newSpawnStruct* spawn)
{
  newSpawn(spawn->spawn);
}

void SpawnShell::newSpawn(const spawnStruct& s)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::newSpawn(spawnStruct *(name='%s'), bSelected=%s)\n", s.name, bSelected?"true":"false");
#endif
   
   uint16_t spawnId;

   // if this is the SPAWN_SELF it's the player
   if (s.NPC == SPAWN_SELF)
   {
     // if the player ID is set, use it for this spawn
     if (m_playerIdSet)
       spawnId = m_playerId;
     else
       spawnId = s.spawnId;
   }
   else
   {
     spawnId = s.spawnId;

     // not the player, so check if it's a recently deleted spawn
     for (int i =0; i < m_cntDeadSpawnIDs; i++)
       if ((m_deadSpawnID[i] != 0) && (m_deadSpawnID[i] == spawnId))
       {
	 // found a match, remove it from the deleted spawn list
	 m_deadSpawnID[i] = 0;

	 // let the user know what's going on
	 printf("%s(%d) has already been removed from the zone before we processed it.\n", 
		s.name, spawnId);

	 // and stop the attempt to add the spawn.
	 return;
       }
   }
   
   ItemMap::iterator it;
   Spawn* item;
   it = m_spawns.find(spawnId);
   if (it != m_spawns.end())
   {
     item = (Spawn*)it->second;
     item->update(&s);
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     emit changeItem(item, tSpawnChangedALL);
   }
   else
   {
     item = new Spawn(spawnId, &s);
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     m_spawns.insert(ItemMap::value_type(spawnId, item));
     emit addItem(item);

     // if this is the player, keep a direct pointer to it for player related
     // updates
     if (item->isSelf())
     {
       printf("Created player spawn via newSpawn shouldn't happen!\n");
       m_playerSpawn = item;
     }

     // send notification of new spawn count
     emit numSpawns(m_spawns.size());
   }

   if (item->filterFlags() & FILTER_FLAG_ALERT)
     emit handleAlert(item, tNewSpawn);
}

void SpawnShell::playerUpdate(const playerUpdateStruct *pupdate, bool client)
{
  if (!client && (pupdate->spawnId != m_playerId))
    return;

  if (m_playerSpawn != NULL)
  {
    m_playerSpawn->setPos(pupdate->xPos, pupdate->yPos, pupdate->zPos,
			  showeq_params->walkpathrecord,
			  showeq_params->walkpathlength);
    m_playerSpawn->setDeltas(pupdate->deltaX, pupdate->deltaY, 
			     pupdate->deltaZ);
    m_playerSpawn->setHeading(pupdate->heading, pupdate->deltaHeading);
    m_playerSpawn->updateLast();

    emit changeItem(m_playerSpawn, tSpawnChangedPosition);
  }
  else
  {
    // create the spawn using info (possibly default) from EQPlayer
    m_playerSpawn = new Spawn(pupdate->spawnId, 
			      m_player->getPlayerName(),
			      m_player->getPlayerLastName(),
			      m_player->getPlayerRace(),
			      m_player->getPlayerClass(),
			      m_player->getPlayerLevel(),
			      m_player->getPlayerDeity());

    // and set it's info
    m_playerSpawn->setPos(pupdate->xPos, pupdate->yPos, pupdate->zPos,
			  showeq_params->walkpathrecord,
			  showeq_params->walkpathlength);
    m_playerSpawn->setDeltas(pupdate->deltaX, pupdate->deltaY, 
			     pupdate->deltaZ);
    m_playerSpawn->setHeading(pupdate->heading, pupdate->deltaHeading);
    m_playerSpawn->updateLast();
    updateFilterFlags(m_playerSpawn);
    updateRuntimeFilterFlags(m_playerSpawn);

    m_spawns.insert(ItemMap::value_type(pupdate->spawnId, 
						  m_playerSpawn));

    emit addItem(m_playerSpawn);

#ifdef SPAWNSHELL_DIAG_PLAYERSPAWN
    printf("Created Fake PlayerSpawn from update: id=%d NPC=%d race=%s class=%s Name=%s\n",
	   m_playerSpawn->id(), m_playerSpawn->NPC(),
	   (const char*)m_playerSpawn->raceName(), 
	   (const char*)m_playerSpawn->className(),
	   (const char*)m_playerSpawn->name());
#endif // SPAWNSHELL_DIAG_PLAYERSPAWN
  }
}

void SpawnShell::updateSpawn(uint16_t id, 
			     int16_t x, int16_t y, int16_t z,
			     int16_t xVel, int16_t yVel, int16_t zVel,
			     int8_t heading, int8_t deltaHeading)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::updateSpawn(id=%d, x=%d, y=%d, z=%d, xVel=%d, yVel=%d, zVel=%d)\n", id, x, y, z, xVel, yVel, zVel);
#endif

   Spawn* item;
   ItemMap::iterator it = m_spawns.find(id);
   if (it != m_spawns.end())
   {
     item = (Spawn*)it->second;

     item->setPos(x, y, z,
		  showeq_params->walkpathrecord,
		  showeq_params->walkpathlength);
     item->setDeltas(xVel, yVel, zVel);
     item->setHeading(heading, deltaHeading);
     item->updateLast();
     emit changeItem(item, tSpawnChangedPosition);
   }
   else if (showeq_params->showUnknownSpawns)
   {
     item = new Spawn(id, x, y, z, xVel, yVel, zVel, 
				heading, deltaHeading);
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     m_spawns.insert(ItemMap::value_type(id, item));
     emit addItem(item);

     // send notification of new spawn count
     emit numSpawns(m_spawns.size());
   }
}

void SpawnShell::updateSpawns(const spawnPositionUpdateStruct* updates)
{
   for (int a = 0; a < updates->numUpdates; a++)
   {
     // I have done some checking.  It appears that this feild
     // (animation) contains the curent animation loop of the mob
     // in question (walking/running/standing etc..
     // 0 = staning.  When this is 0 the mob stops moving in the game
     // even though the velocity numbers arent 0.  this fix should reduce
     // drift when a mob stops moving. (or turns)

     if (updates->spawnUpdate[a].animation != 0)
     {
       updateSpawn(updates->spawnUpdate[a].spawnId,
		   updates->spawnUpdate[a].xPos,
		   updates->spawnUpdate[a].yPos,
		   updates->spawnUpdate[a].zPos,
		   updates->spawnUpdate[a].deltaX,
		   updates->spawnUpdate[a].deltaY,
		   updates->spawnUpdate[a].deltaZ,
		   updates->spawnUpdate[a].heading,
		   updates->spawnUpdate[a].deltaHeading);
     }
     else
     {
       updateSpawn(updates->spawnUpdate[a].spawnId,
		   updates->spawnUpdate[a].xPos,
		   updates->spawnUpdate[a].yPos,
		   updates->spawnUpdate[a].zPos,
		   0, 0, 0,
		   updates->spawnUpdate[a].heading, 
		   0);
     }
   }
}

void SpawnShell::updateSpawnHP(const spawnHpUpdateStruct* hpupdate)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::updateSpawnHp(id=%d, hp=%d, maxHp=%d)\n", 
	  hpupdate->spawnId, hpupdate->curHp, hpupdate->maxHp);
#endif
   Spawn* item;
   ItemMap::iterator it = m_spawns.find(hpupdate->spawnId);
   if (it != m_spawns.end())
   {
     item = (Spawn*)it->second;
     item->setHP(hpupdate->curHp);
     item->setMaxHP(hpupdate->maxHp);
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     emit changeItem(item, tSpawnChangedHP);
   }
}

void SpawnShell::spawnWearingUpdate(const wearChangeStruct *wearing)
{
   Spawn* item;
   ItemMap::iterator it = m_spawns.find(wearing->spawnId);
   if (it != m_spawns.end())
   {
     item = (Spawn*)it->second;
     item->setEquipment(wearing->wearSlotId, wearing->newItemId);
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     emit changeItem(item, tSpawnChangedWearing);
   }
}

void SpawnShell::consRequest(const considerStruct * con) 
{
  if (con->playerid != con->targetid) 
  {
    Spawn* item;
    ItemMap::iterator it = m_spawns.find(con->targetid);
    if (it != m_spawns.end())
    {
      item = (Spawn*)it->second;

      // note that this spawn has been considered
      item->setConsidered(true);

      emit spawnConsidered(item);
    }
  }
}

void SpawnShell::consMessage(const considerStruct * con) 
{
  Spawn* item;
  QString lvl("");
  QString hps("");
  QString cn("");

  QString msg("Faction: Your faction standing with ");

  //printf("%i, %i, %i, %i\n", con->unknown1[0], con->unknown1[1], con->unknown2[0], con->unknown2[1]);

  // is it you that you've conned?
  if (con->playerid == con->targetid) 
  {
    // this is one way to get the player ID
    if (!m_playerIdSet)
      setPlayerID(con->playerid);

    // find the spawn if it exists
    ItemMap::iterator it = m_spawns.find(con->targetid);
    if (it != m_spawns.end())
    {
      item = (Spawn*)it->second;

      // print it's deity
      printf("\nDiety: %s\n", (const char*)item->deityName());
    }

    // well, this is You
    msg += "YOU";
  }
  else 
  {
    // find the spawn if it exists
    ItemMap::iterator it = m_spawns.find(con->targetid);
    
    // has the spawn been seen before?
    if (it != m_spawns.end())
    {
      // yes, get the corresponding item
      item = (Spawn*)it->second;


      printf("\nDiety: %s\n", (const char*)item->deityName());

      int changed = tSpawnChangedNone;

      /* maxhp and curhp are available when considering players, */
      /* but not when considering mobs. */
      if (con->maxHp || con->curHp)
      {
         if (item->NPC() == SPAWN_NPC_UNKNOWN)
         {
            item->setNPC(SPAWN_PLAYER);        // player
            changed |= tSpawnChangedNPC;
         }
         item->setMaxHP(con->maxHp);
         item->setHP(con->curHp);
         changed |= tSpawnChangedHP;
      }
      else if (item->NPC() == SPAWN_NPC_UNKNOWN)
      {
         item->setNPC(SPAWN_NPC);
         changed |= tSpawnChangedNPC;
      }

      // note the updates if any
      if (changed != tSpawnChangedNone)
      {
        if (updateFilterFlags(item))
           changed |= tSpawnChangedFilter;
        if (updateRuntimeFilterFlags(item))
           changed |= tSpawnChangedRuntimeFilter;

        emit changeItem(item, changed);
      }

      // note that this spawn has been considered
      item->setConsidered(true);

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

void SpawnShell::updateLevel(const levelUpStruct* levelup)
{
  if (m_playerSpawn != NULL)
  {
    m_playerSpawn->setLevel(levelup->level);

    emit changeItem(m_playerSpawn, tSpawnChangedLevel);
  }
}

void SpawnShell::deleteSpawn(const deleteSpawnStruct* delspawn)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::deleteSpawn(id=%d)\n", delspawn->spawnId);
#endif
   if (m_posDeadSpawnIDs < MAX_DEAD_SPAWNIDS)
     m_posDeadSpawnIDs++;
   else
     m_posDeadSpawnIDs = 0;
   
   if (m_cntDeadSpawnIDs < MAX_DEAD_SPAWNIDS)
     m_cntDeadSpawnIDs++;

   m_deadSpawnID[m_posDeadSpawnIDs] = delspawn->spawnId;

   deleteItem(tSpawn, delspawn->spawnId);
}

void SpawnShell::killSpawn(const spawnKilledStruct* deadspawn)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::killSpawn(id=%d)\n", deadspawn->spawnId);
#endif
#ifdef SPAWNSHELL_DIAG_STRUCTS
   printf("spawnKilledStruct: spawnId=%d", deadspawn->spawnId);
   ItemMap::iterator tmpit = m_spawns.find(deadspawn->spawnId);
   if (tmpit != m_spawns.end())
       printf("(%s)", (const char*)tmpit->second->name());
   printf(" killerId=%d", deadspawn->killerId);
   tmpit = m_spawns.find(deadspawn->killerId);
   if (tmpit != m_spawns.end())
       printf("(%s)", (const char*)tmpit->second->name());
   printf(" unknown0006={");
   for (uint j = 0; j < sizeof(deadspawn->unknown0006); j++)
     printf("%2.2hhx, ", deadspawn->unknown0006[j]);
   printf("} spellId=%04x(%s) type=%d damage=%d\n",
	  deadspawn->spellId, (const char*)spell_name(deadspawn->spellId),
	  deadspawn->type, deadspawn->damage);
#endif

   ItemMap::iterator it = m_spawns.find(deadspawn->spawnId);
   if (it != m_spawns.end())
   {
     Spawn* item = (Spawn*)it->second;

     // ZBTEMP: This is temporary until we can find a better way
     // set the last kill info on the player (do this before changing name)
     m_player->setLastKill(item->name(), item->level());

     item->killSpawn();
     updateFilterFlags(item);
     updateRuntimeFilterFlags(item);
     emit killSpawn(item);

     if (item->filterFlags() & FILTER_FLAG_ALERT)
       emit handleAlert(item, tKillSpawn);
   }
}

void SpawnShell::corpseLoc(const corpseLocStruct* corpseLoc)
{
   ItemMap::iterator it = m_spawns.find(corpseLoc->spawnId);
   if (it != m_spawns.end())
   {
     Spawn* item = (Spawn*)it->second;

     // set the corpses location, and make sure it's not moving... 
     item->setPos(int16_t(corpseLoc->xPos), int16_t(corpseLoc->yPos), 
		  int16_t(corpseLoc->zPos),
		  showeq_params->walkpathrecord,
		  showeq_params->walkpathlength);
     item->killSpawn();
     item->updateLast();

     // signal that the spawn has changed
     emit killSpawn(item);
   }
}

int SpawnShell::playerId()
{
   return m_playerId;
}

void SpawnShell::setPlayerID(uint16_t id)
{
#ifdef SPAWNSHELL_DIAG
   printf("SpawnShell::setPlayerId(id=%d)\n", id);
#endif
   m_playerId = id;
   m_playerIdSet = true;
   ItemMap::iterator it;

   if (m_playerSpawn != NULL)
   {
     // notify others of the delteion of this item
     emit delItem(m_playerSpawn);

     // iterate over the list until we find the player
     for (it = m_spawns.begin(); it != m_spawns.end(); it++)
     {
       // is this the player spawn?
       if (m_playerSpawn == (Spawn*)it->second)
       {
	 // remove the player spawn from the list
	 m_spawns.erase(it);
	 break;
       }
     }

     // set the player spawns ID
     m_playerSpawn->setID(id);

     // add new spawn back to the list under the new id
     m_spawns.insert(ItemMap::value_type(id, m_playerSpawn));

     // notify others the spawn was re-added
     emit addItem(m_playerSpawn);

#ifdef SPAWNSHELL_DIAG_PLAYERSPAWN
    printf("PlayerSpawn ID Set: id=%d NPC=%d race=%s class=%s Name=%s\n",
	   m_playerSpawn->id(), m_playerSpawn->NPC(),
	   (const char*)m_playerSpawn->raceName(), 
	   (const char*)m_playerSpawn->className(),
	   (const char*)m_playerSpawn->name());
#endif // SPAWNSHELL_DIAG_PLAYERSPAWN
   }

   if ((m_spawns.find(id) == m_spawns.end()) ||
       (m_playerSpawn == NULL))
   {
     // player spawn doesnt exist, fabricate one since we have the id.
     m_playerSpawn = new Spawn(id, m_player->getPlayerName(),
					 m_player->getPlayerLastName(),
					 m_player->getPlayerRace(),
					 m_player->getPlayerClass(),
					 m_player->getPlayerLevel(),
					 m_player->getPlayerDeity());

     // add new spawn to the list
     m_spawns.insert(ItemMap::value_type(id, m_playerSpawn));
     
     // notify others of the new spawn
     emit addItem(m_playerSpawn);

#ifdef SPAWNSHELL_DIAG_PLAYERSPAWN
    printf("Created Fake PlayerSpawn: id=%d NPC=%d race=%s class=%s Name=%s\n",
	   m_playerSpawn->id(), m_playerSpawn->NPC(),
	   (const char*)m_playerSpawn->raceName(), 
	   (const char*)m_playerSpawn->className(),
	   (const char*)m_playerSpawn->name());
#endif // SPAWNSHELL_DIAG_PLAYERSPAWN
   }

#if 0 // ZBTEMP: Diagnostics, just in case
   // sanity checks
   if (m_playerSpawn->id() != id)
     fprintf(stderr, "SpawnShell::setPlayerId(id=%d): Player spawn doesn't have player ID, has %d\n", id, m_playerSpawn->id());
   if (!m_playerSpawn->isSelf())
     fprintf(stderr, "SpawnShell::setPlayerId(id=%d): Player spawn doesn't have correct NPC setting, has %d\n", id, m_playerSpawn->NPC());
#endif
}

void SpawnShell::backfillSpawn(const spawnStruct *spawn)
{
  ItemMap::iterator it;

  uint16_t spawnId;

  spawnId = spawn->spawnId;

  it = m_spawns.find(spawnId);
  
  if (it == m_spawns.end())
  {
    // if it's not already in the list, then just add it.
    newSpawn(*spawn);

    return;
  }

  Spawn* item = (Spawn*)it->second;
  // if we got the self item, then somethings screwy, so only update if
  // not the self spawn
  if (!item->isSelf())
  {
    item->backfill(spawn);
    updateFilterFlags(item);
    updateRuntimeFilterFlags(item);

    emit changeItem(item, tSpawnChangedALL);
    
    if (item->filterFlags() & FILTER_FLAG_ALERT)
      emit handleAlert(item, tFilledSpawn);
  }
}

void SpawnShell::backfillPlayer(const playerProfileStruct* player)
{
  // if there is a player spawn, fill it in
  if (m_playerSpawn != NULL)
  {
    // backfill the player spawn
    m_playerSpawn->backfill(player);
    updateFilterFlags(m_playerSpawn);
    updateRuntimeFilterFlags(m_playerSpawn);

    emit changeItem(m_playerSpawn, tSpawnChangedALL);

    if (m_playerSpawn->filterFlags() & FILTER_FLAG_ALERT)
      emit handleAlert(m_playerSpawn, tFilledSpawn);

#ifdef SPAWNSHELL_DIAG_PLAYERSPAWN
    printf("Backfilled PlayerSpawn: id=%d NPC=%d race=%s class=%s Name=%s\n",
	   m_playerSpawn->id(), m_playerSpawn->NPC(),
	   (const char*)m_playerSpawn->raceName(), 
	   (const char*)m_playerSpawn->className(),
	   (const char*)m_playerSpawn->name());
#endif // SPAWNSHELL_DIAG_PLAYERSPAWN
  }
  else
  {
     // player spawn doesnt exist, fabricate one since we have the id.
    m_playerSpawn = new Spawn(player, m_player->getPlayerDeity());
    updateFilterFlags(m_playerSpawn);
    updateRuntimeFilterFlags(m_playerSpawn);

    // add new spawn to the list using fake id
    m_spawns.insert(ItemMap::value_type(0, m_playerSpawn));
    
    // notify others of the new spawn
    emit addItem(m_playerSpawn);

    if (m_playerSpawn->filterFlags() & FILTER_FLAG_ALERT)
      emit handleAlert(m_playerSpawn, tNewSpawn);

#ifdef SPAWNSHELL_DIAG_PLAYERSPAWN
    printf("Created PlayerSpawn: id=%d NPC=%d race=%s class=%s Name=%s\n",
	   m_playerSpawn->id(), m_playerSpawn->NPC(),
	   (const char*)m_playerSpawn->raceName(), 
	   (const char*)m_playerSpawn->className(),
	   (const char*)m_playerSpawn->name());
#endif // SPAWNSHELL_DIAG_PLAYERSPAWN
  }
}

void SpawnShell::refilterSpawns()
{
  refilterSpawns(tSpawn);
  refilterSpawns(tDrop);
  refilterSpawns(tCoins);
}

void SpawnShell::refilterSpawns(itemType type)
{
   ItemMap& theMap = getMap(type);
   ItemMap::iterator it;

   if (type == tSpawn)
   {
     Spawn* spawn;
     // iterate over all the items in the map
     for (it = theMap.begin(); it != theMap.end(); it++)
     {
       // get the item
       spawn = (Spawn*)it->second;
       
       // update the flags, if they changed, send a notification
       if (updateFilterFlags(spawn))
	 emit changeItem(spawn, tSpawnChangedFilter);
     }
   }
   else
   {
     Item* item;
     // iterate over all the items in the map
     for (it = theMap.begin(); it != theMap.end(); it++)
     {
       // get the item
       item = it->second;
       
       // update the flags, if they changed, send a notification
       if (updateFilterFlags(item))
	 emit changeItem(item, tSpawnChangedFilter);
     }
   }
}

void SpawnShell::refilterSpawnsRuntime()
{
  refilterSpawnsRuntime(tSpawn);
}

void SpawnShell::refilterSpawnsRuntime(itemType type)
{
   ItemMap& theMap = getMap(type);
   ItemMap::iterator it;

   if (type == tSpawn)
   {
     Spawn* spawn;
     // iterate over all the items in the map
     for (it = theMap.begin(); it != theMap.end(); it++)
     {
       // get the item
       spawn = (Spawn*)it->second;
       
       // update the flags, if they changed, send a notification
       if (updateRuntimeFilterFlags(spawn))
	 emit changeItem(spawn, tSpawnChangedRuntimeFilter);
     }
   }
   else
   {
     Item* item;
     // iterate over all the items in the map
     for (it = theMap.begin(); it != theMap.end(); it++)
     {
       // get the item
       item = it->second;
       
       // update the flags, if they changed, send a notification
       if (updateRuntimeFilterFlags(item))
	 emit changeItem(item, tSpawnChangedRuntimeFilter);
     }
   }
}

