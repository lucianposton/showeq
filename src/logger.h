/*
 * logger.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#ifndef SEQLOGGER_H
#define SEQLOGGER_H

#include <qobject.h>

#include "spawn.h"
#include "util.h"

class SEQLogger : public QObject
{
   Q_OBJECT

public:
    SEQLogger(const QString& fname, QObject* parent=0, const char* name="SEQLogger");
    SEQLogger(FILE *fp, QObject* parent=0, const char* name="SEQLogger");
protected:
    int logOpen(void);
    int outputf(const char *fmt, ...);
    int output(const void *data, int length);
    void flush() { fflush(m_FP); }
    FILE* m_FP;
    QString m_filename;
    int m_errOpen;
};

class SpawnLogger: public SEQLogger 
{
   Q_OBJECT

public:
    SpawnLogger(const QString& filename);
    SpawnLogger(FILE *fp);

public slots:
    void logTimeSync(const timeOfDayStruct *tday);
    void logNewZone(const QString& zone);
    void logZoneSpawn(const newSpawnStruct* nspawn);
    void logZoneSpawn(const spawnStruct *spawn);
    void logZoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len);
    void logNewSpawn(const newSpawnStruct* spawn);
    void logKilledSpawn(const Item* item, const Item* kitem, uint16_t kid);
    void logDeleteSpawn(const Item *spawn);

private:
    void logSpawnInfo(const char *type, const char *name, int id, int level, 
                      int x, int y, int z, time_t timeCurrent, 
                      const char *killer, int kid, int guildid);
    int version;
    QString zoneShortName;
    EQTime *l_time;
};

#endif
