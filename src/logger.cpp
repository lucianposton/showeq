/*
 * logger.cpp
 *
 * packet/data logging class
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <qstring.h>
#include <qlist.h>
#include "everquest.h"
#include "spawn.h"
#include "logger.h"
#include "util.h"

SEQLogger::SEQLogger(FILE *fp, QObject* parent, const char* name)
  : QObject(parent, name)
{
    m_FP = fp;
    m_errOpen = 0;
}

SEQLogger::SEQLogger(const QString& fname, QObject* parent, const char* name)
  : QObject(parent, name)
{
    m_FP = NULL;
    m_filename = fname;
    m_errOpen = 0;
}

int
SEQLogger::logOpen()
{
    if (m_FP != NULL)
        return(0);

    m_FP = fopen((const char*)m_filename,"a");

    if (m_FP == NULL)
    { 
        if (m_errOpen == 0)
        {
            fprintf(stderr,"Error opening %s: %s (will keep trying)\n",
		    (const char*)m_filename, strerror(errno));
            m_errOpen = 1;
        }

        return(-1);
    }
 
    m_errOpen = 0;

    return(0);
}

int
SEQLogger::outputf(const char *fmt, ...)
{
    va_list args;
    int count;

    if (m_FP == NULL)
        return(0);

    va_start(args, fmt);
    count = vfprintf(m_FP, fmt, args);
    va_end(args);
    return(count);
}

int
SEQLogger::output(const void* data, int length)
{
    int i;
    int count = 0;
    unsigned char *ptr = (unsigned char *) data;

    for(i = 0; i < length; i++,ptr++)
        count += outputf("%.2X", *ptr);

    return(count);
}

SpawnLogger::SpawnLogger(const QString& fname)
  : SEQLogger(fname, NULL, "SpawnLogger")
{
    version = 3;
    zoneShortName = "unknown";
    l_time = new EQTime();
    return;
}

SpawnLogger::SpawnLogger(FILE *fp)
  : SEQLogger(fp, NULL, "SpawnLogger")
{
    version = 3;
    zoneShortName = "unknown";
    l_time = new EQTime();
    return;
}

void
SpawnLogger::logTimeSync(const timeOfDayStruct *tday)
{
    l_time->setepoch(time(NULL),tday);
}

void
SpawnLogger::logSpawnInfo(const char *type, const char *name, int id, int level,
                          int x, int y, int z, time_t timeCurrent,
                          const char *killedBy, int kid, int guildid)
{
    struct timeOfDayStruct eqDate;
    struct tm* current;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    eqDate  = l_time->eqdate(timeCurrent);
    current = localtime(&timeCurrent);

    outputf("%s:%s(%d):%d:%d,%d,%d:%02d.%02d.%02d:%d:%s:%02d.%02d.%02d.%02d.%04d:%s(%d):%d\n",
        type,
        name,
        id,
        level,
        x,
        y,
        z,
        current->tm_hour, current->tm_min, current->tm_sec,
        version,
        (const char*)zoneShortName,
        eqDate.hour,
        eqDate.minute,
        eqDate.month,
        eqDate.day,
        eqDate.year,
        killedBy,
        kid,
        guildid
    );

    flush();

    return;
}

void 
SpawnLogger::logZoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len)
{
  int spawndatasize = len / sizeof(spawnStruct);

  for (int i = 0; i < spawndatasize; i++)
    logZoneSpawn(&zspawns->spawn[i].spawn);
}

void
SpawnLogger::logZoneSpawn(const spawnStruct *spawn)
{
    logSpawnInfo("z",spawn->name,spawn->spawnId,spawn->level,
                 spawn->x, spawn->y, spawn->z, time(NULL), "", 0, spawn->guildID);

    return;
}

void
SpawnLogger::logZoneSpawn(const newSpawnStruct *nspawn)
{
  const spawnStruct* spawn = &nspawn->spawn;
  logSpawnInfo("z",spawn->name,spawn->spawnId,spawn->level,
	       spawn->x, spawn->y, spawn->z, time(NULL), "", 0, spawn->guildID);
  
  return;
}

void
SpawnLogger::logNewSpawn(const newSpawnStruct* nspawn)
{
  const spawnStruct* spawn = &nspawn->spawn;
  logSpawnInfo("+",spawn->name,spawn->spawnId,spawn->level,
	       spawn->x, spawn->y, spawn->z, time(NULL), "", 0, spawn->guildID);

  return;
}

void
SpawnLogger::logKilledSpawn(const Item *item, const Item* kitem, uint16_t kid)
{
  if (item == NULL)
    return;

  const Spawn* spawn = (const Spawn*)item;
  const Spawn* killer = (const Spawn*)kitem;

  logSpawnInfo("x",(const char *) spawn->name(),spawn->id(), spawn->level(), 
	       spawn->x(), spawn->y(), spawn->z(), time(NULL),
	       killer ? (const char*)killer->name() : "unknown",
	       kid, spawn->GuildID());

  return;
}

void
SpawnLogger::logDeleteSpawn(const Item *item)
{
  if (item->type() != tSpawn)
    return;

  const Spawn* spawn = (const Spawn*)item;

  logSpawnInfo("-",(const char *)spawn->name(),spawn->id(),spawn->level(),
	       spawn->x(), spawn->y(), spawn->z(), time(NULL),"",0, spawn->GuildID());

  return;
}

void
SpawnLogger::logNewZone(const QString& zonename)
{
    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("----------\nNEW ZONE: %s\n----------\n", (const char*)zonename);
    outputf(" :name(spawnID):Level:Xpos:Ypos:Zpos:H.m.s:Ver:Zone:eqHour.eqMinute.eqMonth.eqDay.eqYear:killedBy(spawnID)\n");
    flush();
    zoneShortName = zonename;
}


