/*
 * spawnlog.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _SPAWNLOG_H_
#define _SPAWNLOG_H_

#include <qobject.h>
#include "logger.h"
#include "everquest.h"

////////////////////////
// forward declarations
class QDateTime;
class QTime;
class DateTimeMgr;
class Item;

class SpawnLog: public SEQLogger 
{
   Q_OBJECT

public:
    SpawnLog(DateTimeMgr* dateTimeMgr, const QString& filename);
    SpawnLog(DateTimeMgr* dateTimeMgr, FILE *fp);

public slots:
    void logNewZone(const QString& zone);
    void logZoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len);
    void logNewSpawn(const newSpawnStruct* spawn);
    void logKilledSpawn(const Item* item, const Item* kitem, uint16_t kid);
    void logDeleteSpawn(const Item *spawn);

private:
    void logSpawnInfo(const char *type, const char *name, int id, int level, 
                      int x, int y, int z, 
                      const char *killer, int kid, int guildid);
    void logSpawnInfo(const char *type, const char *name, int id, int level, 
                      int x, int y, int z, 
		      const QDateTime& eqDate, const QTime& time, 
                      const char *killer, int kid, int guildid);
    int version;
    QString zoneShortName;
    DateTimeMgr* m_dateTimeMgr;
};

#endif // _SPAWNLOG_H_
