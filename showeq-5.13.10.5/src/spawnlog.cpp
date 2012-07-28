/*
 * spawnlog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2001-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003,2007 Zaphod (dohpaz@users.sourceforge.net). 
 */

#include "spawn.h"
#include "datetimemgr.h"
#include "everquest.h"
#include "spawnlog.h"

SpawnLog::SpawnLog(DateTimeMgr* dateTimeMgr, const QString& fname)
  : SEQLogger(fname, NULL, "SpawnLog"),
    m_dateTimeMgr(dateTimeMgr)
{
    version = 4;
    zoneShortName = "unknown";
}

SpawnLog::SpawnLog(DateTimeMgr* dateTimeMgr, FILE *fp)
  : SEQLogger(fp, NULL, "SpawnLog"),
    m_dateTimeMgr(dateTimeMgr)
{
    version = 4;
    zoneShortName = "unknown";
}

inline void 
SpawnLog::logSpawnInfo(const char *type, const char *name, int id, int level,
                          int x, int y, int z, 
                          const char *killedBy, int kid, int guildid)
{
  const QDateTime& eqDate = m_dateTimeMgr->updatedDateTime();
  const QTime& time = QTime::currentTime(Qt::LocalTime);

  logSpawnInfo(type, name, id, level, x, y, z, 
	       eqDate, time, 
	       killedBy, kid, guildid);
}

void
SpawnLog::logSpawnInfo(const char *type, const char *name, int id, int level,
                          int x, int y, int z, 
			  const QDateTime& eqDate, const QTime& time,
                          const char *killedBy, int kid, int guildid)
{
  if (!open())
      return;
  
  const QDate& eqDateDate = eqDate.date();
  const QTime& eqDateTime = eqDate.time();
  
  outputf("%s:%s(%d):%d:%d,%d,%d:%02d.%02d.%02d:%d:%s:%02d.%02d.%02d.%02d.%04d:%s(%d):%d\n",
	  type,
	  name,
	  id,
	  level,
	  x,
	  y,
	  z,
	  time.hour(), time.minute(), time.second(),
	  version,
	  (const char*)zoneShortName,
	  eqDateTime.hour(),
	  eqDateTime.minute(),
	  eqDateDate.month(),
	  eqDateDate.day(),
	  eqDateDate.year(),
	  killedBy,
	  kid,
	  guildid);
  
  flush();
}

void 
SpawnLog::logZoneSpawns(const uint8_t* data, size_t len)
{
  const spawnStruct* zspawns = (const spawnStruct*)data;
  int spawndatasize = len / sizeof(spawnStruct);
  
  const QDateTime& eqDate = m_dateTimeMgr->updatedDateTime();
  const QTime& time = QTime::currentTime(Qt::LocalTime);
  
  for (int i = 0; i < spawndatasize; i++)
  {
    const spawnStruct& spawn = zspawns[i];
    logSpawnInfo("z",spawn.name,spawn.spawnId,spawn.level,
                 (spawn.x >> 3), (spawn.y >> 3), (spawn.z >> 3), 
		 eqDate, time, "", 0, spawn.guildID);
  }
}

void
SpawnLog::logNewSpawn(const uint8_t* data)
{
  const spawnStruct& spawn = *(const spawnStruct*)data;
  logSpawnInfo("+",spawn.name,spawn.spawnId,spawn.level,
	       (spawn.x >> 3), (spawn.y >> 3), (spawn.z >> 3), 
	       "", 0, spawn.guildID);
}

void
SpawnLog::logKilledSpawn(const Item *item, const Item* kitem, uint16_t kid)
{
  if (item == NULL)
    return;
  
  const Spawn* spawn = (const Spawn*)item;
  const Spawn* killer = (const Spawn*)kitem;
  
  logSpawnInfo("x",(const char *) spawn->name(),spawn->id(), spawn->level(), 
	       spawn->x(), spawn->y(), spawn->z(), 
	       killer ? (const char*)killer->name() : "unknown",
	       kid, spawn->guildID());
}

void
SpawnLog::logDeleteSpawn(const Item *item)
{
  if (item->type() != tSpawn)
    return;
  
  const Spawn* spawn = (const Spawn*)item;
  
  logSpawnInfo("-",(const char *)spawn->name(),spawn->id(),spawn->level(),
	       spawn->x(), spawn->y(), spawn->z(), "",0, spawn->guildID());
}

void
SpawnLog::logNewZone(const QString& zonename)
{
  if (!open())
      return;

  outputf("----------\nNEW ZONE: %s\n----------\n", (const char*)zonename);
  outputf(" :name(spawnID):Level:Xpos:Ypos:Zpos:H.m.s:Ver:Zone:eqHour.eqMinute.eqMonth.eqDay.eqYear:killedBy(spawnID)\n");
  flush();
  zoneShortName = zonename;
}

#ifndef QMAKEBUILD
#include "spawnlog.moc"
#endif

