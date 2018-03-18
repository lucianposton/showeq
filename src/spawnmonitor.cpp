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

#include "spawnmonitor.h"
#include "main.h"
#include "util.h"
#include "datalocationmgr.h"
#include "diagnosticmessages.h"

#include <math.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

namespace {

static int normalize_z(int z, float size)
{
    // A spawn point's z may vary. This seems to be due to the spawn's size.
    // To account for the z variation, we normalize it based on the size.
    return ceil(float(z) - size * 2.0/3.0);
}

static const int Z_FUDGE_FACTOR = 5;
static SpawnPoint* findWithZFudgeFactor(
        const QAsciiDict<SpawnPoint>& spawnpoints,
        const Spawn* spawn,
        QString& foundKey)
{
    // normalize_z() doesn't accommodate large spawns well e.g. size 24, hill
    // giants. So in addition to normalizing z, we also search above and
    // below the normalized z point by a fudge factor.
    //
    // A fudge factor of 4 is sufficient for spawnpoints that contain size 24
    // and size 6 mobs. The fudge factor may need to be increased for
    // unusually large spawns (e.g. cazic thule (size 40) or larger dragons
    // (size 30)) that share a spawnpoint with regular sized spawns.
    const int x = spawn->x();
    const int y = spawn->y();
    const int z = normalize_z(spawn->z(), spawn->size());

    QString key = SpawnPoint::keyFromNormalizedCoords(x, y, z);
    SpawnPoint* result = spawnpoints.find(key);
    if (result)
    {
        foundKey = key;
        return result;
    }

    for (int i = z - Z_FUDGE_FACTOR; i <= z + Z_FUDGE_FACTOR; ++i)
    {
        if (i != z)
        {
            key = SpawnPoint::keyFromNormalizedCoords(x, y, i);
            result = spawnpoints.find(key);
            if (result)
            {
                foundKey = key;
                return result;
            }
        }
    }

    return NULL;
}

} // namespace

SpawnPoint::SpawnPoint(uint16_t spawnID, 
		       const EQPoint& loc, 
		       const QString& name, 
		       time_t diffTime, uint32_t count)
  : EQPoint(loc),
    m_spawnTime(0),
    m_deathTime(0),
    m_diffTime(diffTime),
    m_count(count),
    m_name( name ),
    m_last( QString::null ),
    m_lastID(spawnID),
    m_spawn_counts(),
    m_spawnCountDisplayString(),
    m_spawnedTimeDisplayString()
{
    regenerateDisplayStrings();
}

SpawnPoint::~SpawnPoint()
{
}

QString SpawnPoint::key( int x, int y, int z, float size )
{
  QString t;
  t.sprintf( "%d_%d_%d", x, y, normalize_z(z, size) );
  return t;
}

QString SpawnPoint::keyFromNormalizedCoords( int x, int y, int z )
{
  QString t;
  t.sprintf( "%d_%d_%d", x, y, z );
  return t;
}

unsigned char SpawnPoint::age() const
{
  if ( m_deathTime == 0 )
    return 0;
  
  long	scaledColor = 255;
  float	ratio = (float)( time( 0 ) - m_deathTime ) / (float)m_diffTime;
  scaledColor = (long)( scaledColor * ratio );
  if ( scaledColor > 255 )
    scaledColor = 255;
  else if ( scaledColor < 0 )
    scaledColor = 0;
  return (unsigned char)scaledColor;
}

void SpawnPoint::restart(void)
{
  m_lastID = 0;
  m_deathTime = time( 0 );
}

QString SpawnPoint::spawnCountDisplayString() const
{
    return m_spawnCountDisplayString;
}

QString SpawnPoint::spawnedTimeDisplayString() const
{
    return m_spawnedTimeDisplayString;
}

QString SpawnPoint::remainingTimeDisplayString(const char* na, const char* now) const
{
    QString remaining;
    if ( diffTime() == 0 || deathTime() == 0 )
    {
        remaining = QString::fromUtf8(na);
    }
    else
    {
        const time_t secs = secsLeft();
        if ( secs > 0 )
            remaining.sprintf( "%3ld:%02ld", secs / 60, secs % 60  );
        else
            remaining = QString::fromUtf8(now);
    }

    return remaining;
}

QString SpawnPoint::displayString() const
{
    QString result;
    result.sprintf("SpawnPoint: %s\n"
           "%.3s/Z: %5d/%5d/%5d\n"
           "Last: %s\n"
           "Spawned: %s\t Remaining: %s\t Count: %d\n"
           "%s",
           (const char*)name(),
           showeq_params->retarded_coords ? "Y/X" : "X/Y",
           showeq_params->retarded_coords ? y() : x(),
           showeq_params->retarded_coords ? x() : y(),
           z(),
           (const char*)last(),
           (const char*)spawnedTimeDisplayString(),
           (const char*)remainingTimeDisplayString(),
           count(),
           (const char*)spawnCountDisplayString());

    return result;
}

void SpawnPoint::regenerateDisplayStrings()
{
    QString spawnCountDisplayString;
    QDictIterator<void> it(m_spawn_counts);
    for ( ; it.current(); ++it)
    {
        const long spawn_count = reinterpret_cast<long>(it.current());
        QString tmp;
        tmp.sprintf("%4.1f%% (%ld):\t%s\n",
            100.0 * float(spawn_count)/float(m_count),
            spawn_count,
            (const char*)it.currentKey());
        spawnCountDisplayString += tmp;
    }

    m_spawnCountDisplayString = spawnCountDisplayString;


    QString spawned;
    if (spawnTime())
    {
        QDateTime dateTime;
        dateTime.setTime_t(spawnTime());
        QDate createDate = dateTime.date();
        if ( createDate != QDate::currentDate() )
            spawned = createDate.dayName( createDate.dayOfWeek() ) + " ";
        spawned += dateTime.time().toString();
    }
    else
    {
        spawned = "";
    }
    m_spawnedTimeDisplayString = spawned;
}

void SpawnPoint::update(const Spawn* spawn)
{
  if (spawn == NULL)
    return;

  m_lastID = spawn->id();
  
  if ( m_lastID )
    m_last = spawn->name();
  else
    m_last = "";
  
  m_spawnTime = time(0);
  
  if (m_deathTime != 0)
    m_diffTime = m_spawnTime - m_deathTime;
  
  m_count++;

  const QString cleanedName = spawn->cleanedName();
  long spawn_name_count = reinterpret_cast<long>(m_spawn_counts.take(cleanedName));
  m_spawn_counts.insert(cleanedName, reinterpret_cast<void*>(spawn_name_count+1));

  regenerateDisplayStrings();
}

void SpawnPoint::setSpawnCount(const QString& name, int count)
{
    if (m_spawn_counts.find(name))
    {
        seqWarn("SpawnPoint::setSpawnCount(%s,%d): duplicate name",
                (const char*)name, count);
    }
    m_spawn_counts.replace(name, reinterpret_cast<void*>(count));
    regenerateDisplayStrings();
}

SpawnMonitor::SpawnMonitor(const DataLocationMgr* dataLocMgr, 
			   ZoneMgr* zoneMgr, SpawnShell* spawnShell, 
			   QObject* parent, const char* name )
: QObject( parent, name ),
  m_dataLocMgr(dataLocMgr),
  m_spawnShell(spawnShell),
  m_spawns( 613 ),
  m_points( 211 ),
  m_selected(NULL),
  m_isInitialZoneSpawn(false)
{
  m_spawns.setAutoDelete( true );
  m_points.setAutoDelete( true );
  
  connect(spawnShell, SIGNAL(addItem(const Item*)), 
	  this, SLOT( newSpawn(const Item*)));
  connect(spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)), 
	  this, SLOT( killSpawn(const Item*)));
  connect(zoneMgr, SIGNAL(zoneBegin()),
	  this, SLOT( zoneExit()));
  connect(zoneMgr, SIGNAL(logOut()),
	  this, SLOT( zoneExit()));
  connect(zoneMgr, SIGNAL(zoneBegin(const QString&)), 
	  this, SLOT( zoneEnter( const QString&)));
  connect(spawnShell, SIGNAL(initialZoneSpawn(bool)),
          this, SLOT(initialZoneSpawn(bool)));

  m_modified = false;
}

SpawnMonitor::~SpawnMonitor()
{
}

void SpawnMonitor::setName(const SpawnPoint* csp, const QString& name)
{
  if (csp == NULL)
    return;
  
  SpawnPoint* sp = (SpawnPoint*)csp;
  sp->setName(name);
  m_modified = true;
}

void SpawnMonitor::setSelected(const SpawnPoint* selected)
{
  // if it's already the selected one, then just return
  if (m_selected == selected)
    return;

  m_selected = selected;

  emit selectionChanged(m_selected);
}

void SpawnMonitor::clear(void)
{
  m_selected = NULL;
  emit selectionChanged(m_selected);
  emit clearSpawnPoints();
  m_spawns.clear();
  m_points.clear();
}

void SpawnMonitor::deleteSpawnPoint(const SpawnPoint* sp)
{
  // if deleting the selected spawn point, change the selection to NUL
  if (m_selected == sp)
  {
    m_selected = NULL;
    emit selectionChanged(m_selected);
  }

  // remove the spawn point (will automatically delete it).
  m_spawns.remove(sp->key());
  m_modified = true;
}

void SpawnMonitor::initialZoneSpawn(bool isInitialZoneSpawn)
{
    m_isInitialZoneSpawn = isInitialZoneSpawn;
}

void SpawnMonitor::newSpawn(const Item* item)
{
  if (item->type() == tSpawn)
    checkSpawnPoint( (Spawn*)item );
};

void SpawnMonitor::killSpawn(const Item* killedSpawn)
{
  QAsciiDictIterator<SpawnPoint>		it( m_points );
  
  SpawnPoint*		sp;
  while ( ( sp = it.current() ) )
  {
    if ( killedSpawn->id() == sp->lastID() )
    {
      restartSpawnPoint( sp );
      return;
    }
    ++it;
  }

  QAsciiDictIterator<SpawnPoint> it_spawns(m_spawns);
  while ( ( sp = it_spawns.current() ) )
  {
    if ( killedSpawn->id() == sp->lastID() )
    {
      restartSpawnPoint( sp );
      return;
    }
    ++it_spawns;
  }
}

void SpawnMonitor::zoneExit()
{
    saveSpawnPoints();
    // clear zone name to prevent any intermittent spawns from saving
    m_zoneName = "";
}

void SpawnMonitor::zoneEnter( const QString& newZoneName )
{
  QString lower = newZoneName.lower();

  saveSpawnPoints();
  clear();
  m_zoneName = lower;
  loadSpawnPoints();
}

void SpawnMonitor::restartSpawnPoint( SpawnPoint* sp )
{
  sp->restart();
}
	
void SpawnMonitor::checkSpawnPoint(const Spawn* spawn )
{
  // ignore everything but mobs
  if ( ( spawn->NPC() != SPAWN_NPC ) || ( spawn->petOwnerID() != 0 ))
    return;

#ifdef DEBUG_SPAWNMONITOR
  seqDebug("SpawnMonitor::checkSpawnPoint() %d %s (%d,%d,%d)->(%s) size=%f",
          spawn->id(), (const char*)spawn->name(),
          spawn->x(), spawn->y(), spawn->z(),
          (const char*)SpawnPoint::key(*spawn),
          spawn->size());
#endif
  
  SpawnPoint*		sp;
  QString key;
  sp = findWithZFudgeFactor(m_points, spawn, key);
  if ( sp )
  {
    m_modified = true;
    sp->update(spawn);
  }
  else
  {
    sp = findWithZFudgeFactor(m_spawns, spawn, key);
    // Check m_isInitialZoneSpawn so that we don't add a spawnpoint for two
    // spawns that happen to be standing on same point when player zones in
    if ( sp && !m_isInitialZoneSpawn )
    {
      sp->update(spawn);
      
      m_points.insert( key, sp );
      emit newSpawnPoint( sp );
      m_modified = true;
      if (m_spawns.take( key ) == NULL)
      {
          seqWarn("SpawnMonitor::checkSpawnPoint(%s,id=%d): Failed to take "
                  "spawnpoint from m_spawns. key=%s",
                  (const char*)spawn->name(), spawn->id(),
                  (const char*)key);
      }
    }
    else if ( !sp )
    {
      key = SpawnPoint::key( *spawn );
      EQPoint loc(spawn->x(), spawn->y(), normalize_z(spawn->z(), spawn->size()));
      sp = new SpawnPoint( spawn->id(), loc );
      m_spawns.insert( key, sp );
    }
  }
}

void SpawnMonitor::saveSpawnPoints()
{
  // only save if modified
  if (!m_modified)
    return;

  if ( !m_zoneName.length() )
  {
    // This case occurs when the server sends spawns before OP_ZoneNew,
    // causing m_modified to be set before the m_zoneName.
    seqWarn("Zone name not set in 'SpawnMonitor::saveSpawnPoints'!" );
    return;
  }
  
  QString fileName;
  
  fileName = m_zoneName + ".sp";

  QFileInfo fileInfo = 
    m_dataLocMgr->findWriteFile("spawnpoints", fileName, false);

  fileName = fileInfo.absFilePath();

  QString newName = fileName + ".new";
  QFile spFile( newName );
  
  if (!spFile.open(IO_WriteOnly))
  {
    seqWarn("Failed to open %s for writing", (const char*)newName);
    return;
  }
  
  QTextStream output(&spFile);
  
  QAsciiDictIterator<SpawnPoint> it( m_points );
  SpawnPoint* sp;
  
  while ((sp = it.current()))
  {
    ++it;
    output	<< sp->x()
		<< " "
		<< sp->y()
		<< " "
		<< sp->z()
		<< " "
		<< (unsigned long)sp->diffTime()
		<< " "
		<< sp->count()
		<< " "
		<< sp->name()
		<< '\n';

    if (!sp->spawn_counts().isEmpty())
    {
        QString fileName_spc = m_zoneName + sp->key() + ".spc";

        QFileInfo fileInfo_spc = 
            m_dataLocMgr->findWriteFile("spawnpoints/"+m_zoneName, fileName_spc, false);

        fileName_spc = fileInfo_spc.absFilePath();

        const QString newName_spc = fileName_spc + ".new";
        QFile spcFile( newName_spc );

        if (!spcFile.open(IO_WriteOnly))
        {
            seqWarn("Failed to open %s for writing", (const char*)newName_spc);
            return;
        }

        QTextStream output_spc(&spcFile);
        QDictIterator<void> spawn_counts_it(sp->spawn_counts());
        for ( ; spawn_counts_it.current(); ++spawn_counts_it)
        {
            output_spc << reinterpret_cast<long>(spawn_counts_it.current())
                << " "
                << spawn_counts_it.currentKey()
                << '\n';
        }
        spcFile.close();

        QFileInfo fi( spcFile );
        QFile old( fileName_spc );
        QDir dir( fi.dir() );
        QString backupName = fileName_spc + ".bak";

        if (old.exists())
        {
            if (dir.rename( fileName_spc, backupName))
            {
                if (!dir.rename( newName_spc, fileName_spc))
                    seqWarn( "Failed to rename %s to %s", 
                            (const char*)newName_spc, (const char*)fileName_spc);
            }
        }
        else
        {
            if (!dir.rename(newName_spc, fileName_spc))
                seqWarn("Failed to rename %s to %s", 
                        (const char*)newName_spc, (const char*)fileName_spc);
        }
    }
  }
  
  spFile.close();
  
  QFileInfo fi( spFile );
  QFile old( fileName );
  QDir dir( fi.dir() );
  QString backupName = fileName + ".bak";
  
  if (old.exists())
  {
    if (dir.rename( fileName, backupName))
    {
      if (!dir.rename( newName, fileName))
	seqWarn( "Failed to rename %s to %s", 
		(const char*)newName, (const char*)fileName);
    }
  }
  else
  {
    if (!dir.rename(newName, fileName))
      seqWarn("Failed to rename %s to %s", 
	     (const char*)newName, (const char*)fileName);
  }
  m_modified = false;
  seqDebug("Saved spawn points: %s", (const char*)fileName);
}


void SpawnMonitor::loadSpawnPoints()
{
  QString fileName;
  
  fileName = m_zoneName + ".sp";

  QFileInfo fileInfo = 
    m_dataLocMgr->findExistingFile("spawnpoints", fileName, false);

  if (!fileInfo.exists())
  {
    seqDebug("Previous spawn point file (%s) not found",
	   (const char*)fileInfo.absFilePath());
    return;
  }
  
  fileName = fileInfo.absFilePath();

  QFile spFile(fileName);
  
  if (!spFile.open(IO_ReadOnly))
  {
    seqWarn( "Can't open spawn point file %s", (const char*)fileName );
    return;
  }
  
  QTextStream input( &spFile );
  
  int16_t x, y, z;
  unsigned long diffTime;
  int count;
  QString name;

  while (!input.atEnd())
  {
    input >> x;
    input >> y;
    input >> z;
    input >> diffTime;
    input >> count;
    name = input.readLine();
    name = name.stripWhiteSpace();
    
    EQPoint	loc(x, y, z);
    SpawnPoint*	p = new SpawnPoint( 0, loc, name, diffTime, count );
    if (p)
    {
      QString key = p->key();
      
      if (m_points.find(key))
      {
	seqWarn("Warning: spawn point key already in use!");
	delete p;
    continue;
      }

      QString fileName_spc = m_zoneName + key + ".spc";
      QFileInfo fileInfo_spc = 
          m_dataLocMgr->findExistingFile("spawnpoints/"+m_zoneName, fileName_spc, false);

      if (!fileInfo_spc.exists())
      {
          seqDebug("Previous spawn point count file (%s) not found",
                  (const char*)fileInfo_spc.absFilePath());
          m_points.insert(key, p);
          emit newSpawnPoint(p);
          continue;
      }

      fileName_spc = fileInfo_spc.absFilePath();
      QFile spcFile(fileName_spc);
      if (!spcFile.open(IO_ReadOnly))
      {
          seqWarn( "Can't open spawn point count file %s", (const char*)fileName_spc );
          m_points.insert(key, p);
          emit newSpawnPoint(p);
          continue;
      }

      int total_spawn_count = 0;
      QTextStream input_spc( &spcFile );
      while (!input_spc.atEnd())
      {
          int spawn_count;
          QString spawn_name;
          input_spc >> spawn_count;
          total_spawn_count += spawn_count;
          spawn_name = input_spc.readLine();
          spawn_name.remove(0,1); // Remove leading space delimiter
          p->setSpawnCount(spawn_name, spawn_count);
      }
      spcFile.close();

      if (total_spawn_count != count)
      {
          p->setCount(total_spawn_count);
      }

      m_points.insert(key, p);
      emit newSpawnPoint(p);
    }
  }

  spFile.close();
  seqDebug("Loaded spawn points: %s", (const char*)fileName);
  m_modified = false;
}

#include "spawnmonitor.moc"
