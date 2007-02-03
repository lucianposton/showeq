/*
 * spawnmonitor.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Borrowed from:  SINS Distributed under GPL
 * Portions Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). 
 *
 * For use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef SPAWNMONITOR_H
#define SPAWNMONITOR_H

// unfortunately, Qt's notions of time aren't convenient enough to use for something
//	like spawn timing, since QTime doesn't have any simple way to subtract 2 times
//	that might be more than a full day apart... I rely on the functions in "time.h"
//	for now...
//
//	since we're only really interested in holding timestamps in wall-clock time
//	for spawn points, we don't do any funky real-time -> EQ-time conversions.
//	the first instantiation of the "SpawnPoint" class, m_spawnTime will contain the
//	actual time it is at the time of instantiation, later we get updated by the
//  SpawnMonitor to reflect that spawn point's cycle time and set m_diffTime
//
//	the SpawnMonitor contains one IntDict holding a SpawnPoint for every new
//	spawn that comes in... if a spawn occurs for a SpawnPoint that already exists
//	in "m_spawns", then that SpawnPoint is moved to "m_points" and is promoted
//	to an actual spawn point and deleted from "m_spawns"...
//
//	this is experimental, since it's quite possible there might be false positives
//	because the spawns are random... a few false positivies are acceptable provided
//	there is some utility in the spawn timing functionality... i will probably try
//	to derive some test to weed out the false positives based on how old they are:
//	i.e. if a spawn point in m_points doesn't get an update for over 2 weeks, remove
//	it... i don't know enough yet to determine what length is "too old", since there
//	are rumored to be very rare, yet static, spawns in EQ

#include <time.h>
#include <qobject.h>
#include <q3asciidict.h>
#include "spawn.h"
#include "zonemgr.h"
#include "spawnshell.h"

// forward declarations
class DataLocationMgr;

class SpawnPoint: public EQPoint
{
public:
  SpawnPoint(uint16_t spawnID, const EQPoint& loc, 
	     const QString& name = "", time_t diffTime = 0, 
	     uint32_t count = 1);

  virtual ~SpawnPoint();
  
  long secsLeft() const { return m_diffTime - ( time( 0 ) - m_deathTime ); }
  
  static QString key( int x, int y, int z );
  static QString key( const EQPoint& l ) { return key( l.x(), l.y(), l.z() ); }
  QString key() const { return key( x(), y(), z() ); }

  // getters
  unsigned char age() const;
  QString name() const { return m_name; }
  QString last() const { return m_last; }
  uint16_t lastID() const { return m_lastID; }
  int32_t count() const { return m_count; }
  Spawn* getSpawn() const;
  time_t spawnTime() const { return m_spawnTime; }
  time_t deathTime() const { return m_deathTime; } 
  time_t diffTime() const { return m_diffTime; }

  // setters
  void setName(const QString& newName) { m_name = newName; }
  void setLast(const QString& last) { m_last = last; }
  void setLastID(uint16_t lastID) { m_lastID = lastID; }

  // utility methods
  void update(const Spawn* spawn);
  void restart(void);

 protected:
  time_t m_spawnTime;
  time_t m_deathTime;
  time_t m_diffTime;
  uint32_t m_count;
  QString m_name;
  QString m_last;
  uint16_t m_lastID;
};

class SpawnMonitor: public QObject
{
Q_OBJECT
public:
  SpawnMonitor(const DataLocationMgr* dataLocMgr, 
			     ZoneMgr* zoneMgr, SpawnShell* spawnShell, 
			     QObject* parent = 0, 
			     const char* name = "spawnmonitor" );
  virtual ~SpawnMonitor();
 
  const Q3AsciiDict<SpawnPoint>& spawnPoints() { return m_points; }
  const Q3AsciiDict<SpawnPoint>& spawns() { return m_spawns; }
  const SpawnPoint* selected() { return m_selected; }

public slots:
  void setName(const SpawnPoint* sp, const QString& name);
  void setModified( SpawnPoint* changedSp );
  void setSelected(const SpawnPoint* sp);
  void clear(void);
  void deleteSpawnPoint(const SpawnPoint* sp);
  void newSpawn(const Item* item );
  void killSpawn(const Item* item );
  void zoneChanged( const QString& newZoneName );
  void zoneEnd( const QString& newZoneName );
  void saveSpawnPoints();
  void loadSpawnPoints();

signals:
  void newSpawnPoint( const SpawnPoint* spawnPoint );
  void clearSpawnPoints();
  void selectionChanged(const SpawnPoint* selected);

protected:
  void restartSpawnPoint( SpawnPoint* spawnPoint );
  void checkSpawnPoint(const Spawn* spawn );
 
  const DataLocationMgr* m_dataLocMgr;
  SpawnShell* m_spawnShell;
  QString m_zoneName;
  Q3AsciiDict<SpawnPoint> m_spawns;
  Q3AsciiDict<SpawnPoint> m_points;
  const SpawnPoint* m_selected;
  bool m_modified;
};

#endif
