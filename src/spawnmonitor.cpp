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

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>


SpawnPoint::SpawnPoint(uint16_t spawnID, 
		       const EQPoint& loc, 
		       const QString& name, 
		       time_t diffTime, uint32_t count)
  : EQPoint(loc),
    m_spawnTime(time(0)),
    m_deathTime(0),
    m_diffTime(diffTime),
    m_count(count),
    m_name( name ),
    m_last( QString::null ),
    m_lastID(spawnID)
{
}

SpawnPoint::~SpawnPoint()
{
}

QString SpawnPoint::key( int x, int y, int z)
{
  QString		t;
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
}

SpawnMonitor::SpawnMonitor(const DataLocationMgr* dataLocMgr, 
			   ZoneMgr* zoneMgr, SpawnShell* spawnShell, 
			   QObject* parent, const char* name )
: QObject( parent, name ),
  m_dataLocMgr(dataLocMgr),
  m_spawnShell(spawnShell),
  m_spawns( 613 ),
  m_points( 211 ),
  m_selected(NULL)
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
      break;
    }
    ++it;
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
  
  QString		key = SpawnPoint::key( *spawn );
  
  SpawnPoint*		sp;
  sp = m_points.find( key );
  if ( sp )
  {
    m_modified = true;
    sp->update(spawn);
  }
  else
  {
    sp = m_spawns.find( key );
    if ( sp )
    {
      sp->update(spawn);
      
      m_points.insert( key, sp );
      emit newSpawnPoint( sp );
      m_modified = true;
      m_spawns.take( key );
    }
    else
    {
      sp = new SpawnPoint( spawn->id(), *spawn );
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

    // TODO: save list of known spawns
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
  uint32_t count;
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
      
      if (!m_points.find(key))
      {
	m_points.insert(key, p);
	emit newSpawnPoint(p);
      }
      else
      {
	seqWarn("Warning: spawn point key already in use!");
	delete p;
      }
    }
  }

  seqDebug("Loaded spawn points: %s", (const char*)fileName);
  m_modified = false;
}

#include "spawnmonitor.moc"
