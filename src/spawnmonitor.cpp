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

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include "spawnmonitor.h"
#include "main.h"
#include "util.h"

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
  t.sprintf( "%d%d%d", x, y, z );
  return t;
}

Spawn* SpawnPoint::getSpawn() const
{
  return NULL;
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


SpawnMonitor::SpawnMonitor(ZoneMgr* zoneMgr,
			   SpawnShell* spawnShell, 
			   QObject* parent, const char* name )
: QObject( parent, name ),
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
  connect(zoneMgr, SIGNAL(zoneChanged(const QString&)), 
	  this, SLOT( zoneChanged(const QString&)));
  connect(zoneMgr, SIGNAL(zoneEnd(const QString&, const QString&)), 
	  this, SLOT( zoneEnd( const QString&)));

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
  setModified(sp);
}

void SpawnMonitor::setModified( SpawnPoint* changedSp )
{
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
}

void SpawnMonitor::newSpawn(const Item* item)
{
//	debug( "SpawnMonitor::handleNewSpawn" );
  if (item->type() == tSpawn)
    checkSpawnPoint( (Spawn*)item );
};

void SpawnMonitor::killSpawn(const Item* killedSpawn)
{
  QAsciiDictIterator<SpawnPoint>		it( m_points );
  
  SpawnPoint*		sp;
  while ( ( sp = it.current() ) )
  {
    if ( killedSpawn->id() == sp->m_lastID )
    {
      //printf( "death time: %d    %d\n", killedSpawn->spawnId, time( 0 ) );
      restartSpawnPoint( sp );
      break;
    }
    ++it;
  }
}

void SpawnMonitor::zoneChanged( const QString& newZoneName )
{
  if ( m_zoneName != newZoneName )
  {
    saveSpawnPoints();
    
    emit clearSpawnPoints();
    m_spawns.clear();
    m_points.clear();
    m_zoneName = newZoneName;
    
    loadSpawnPoints();
  }
}

void SpawnMonitor::zoneEnd( const QString& newZoneName )
{
  QString		lower = newZoneName.lower();
  
  if ( m_zoneName != lower )
  {
    m_zoneName = lower;
    emit clearSpawnPoints();
    m_spawns.clear();
    m_points.clear();
    loadSpawnPoints();
  }
}

void SpawnMonitor::restartSpawnPoint( SpawnPoint* sp )
{
  sp->m_lastID = 0;
  sp->m_deathTime = time( 0 );
}
	
void SpawnMonitor::checkSpawnPoint(const Spawn* spawn )
{
//	debug( "SpawnMonitor::checkSpawnPoint" );

  // ignore everything but mobs
  if ( ( spawn->NPC() != SPAWN_NPC ) || ( spawn->petOwnerID() != 0 ) )
    return;
  
  QString		key = SpawnPoint::key( *spawn );
  
  //	printf( "key: '%s'", (const char*)key );

  SpawnPoint*		sp;
  sp = m_points.find( key );
  if ( sp )
  {
    //if ( sp->m_lastID != 0 )
    //	printf( "DOUBLE SPAWN\n" );
    
    sp->m_lastID = spawn->id();
    
    if ( sp->m_lastID )
      sp->m_last = spawn->name();
    else
      sp->m_last = "";
    
    sp->m_spawnTime = time( 0 );
    
    //		printf( "spawn time: %d    %d\n", sp->m_lastID, sp->m_spawnTime );
    
    if ( sp->m_deathTime != 0  )
    {
      sp->m_diffTime = sp->m_spawnTime - sp->m_deathTime;
      //			printf( "setting diffTime: %d\n", sp->m_diffTime );
    }
    
    sp->m_count++;
  }
  else
  {
    sp = m_spawns.find( key );
    if ( sp )
    {
      sp->m_lastID = spawn->id();
    
      if ( sp->m_lastID )
	sp->m_last = spawn->name();
      else
	sp->m_last = "";
      sp->m_spawnTime = time( 0 );
      sp->m_count++;
      
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
//	debug( "SpawnMonitor::saveSpawnPoints" );

  if ( !m_zoneName.length() )
  {
    fprintf( stderr, "Zone name not set in 'SpawnMonitor::saveSpawnPoints'!\n" );
    return;
  }
  
  QString fileName;
  
  fileName = QString(LOGDIR "/") + m_zoneName + ".sp";
  
  QString newName = fileName + ".new";
  QFile spFile( newName );
  
  if (!spFile.open(IO_WriteOnly))
  {
    printf("Failed to open %s for writing", (const char*)newName);
    return;
  }
  
  QTextStream output(&spFile);
  
  QAsciiDictIterator<SpawnPoint> it( m_points );
  SpawnPoint* sp;
  
  while ((sp = it.current()))
  {
    ++it;
    //		printf( "writing: %d %d %d '%s'\n", sp->x, sp->y, sp->z, (const char*)sp->m_name );
    output	<< sp->x()
		<< " "
		<< sp->y()
		<< " "
		<< sp->z()
		<< " "
		<< (unsigned long)sp->m_diffTime
		<< " "
		<< sp->count()
		<< " "
		<< sp->m_name
		<< '\n';
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
	printf( "Failed to rename %s to %s\n", 
		(const char*)newName, (const char*)fileName);
    }
  }
  else
  {
    if (!dir.rename(newName, fileName))
      printf("Failed to rename %s to %s\n", 
	     (const char*)newName, (const char*)fileName);
  }
  m_modified = false;
  printf("Saved spawn points: %s\n", (const char*)fileName);
}


void SpawnMonitor::loadSpawnPoints()
{
  QString fileName;
  
  fileName = QString(LOGDIR "/") + m_zoneName + ".sp";
  
  if (!findFile(fileName))
  {
    printf("Can't find spawn point file %s\n", (const char*)fileName);
    return;
  }
  
  QFile spFile(fileName);
  
  if (!spFile.open(IO_ReadOnly))
  {
    printf( "Can't open spawn point file %s\n", (const char*)fileName );
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
    //		printf( "x: %d\n", x );
    input >> y;
    //		printf( "y: %d\n", y );
    input >> z;
    //		printf( "z: %d\n", z );
    input >> diffTime;
    //		printf( "diff: %d\n", diffTime );
    input >> count;
    name = input.readLine();
    name = name.stripWhiteSpace();
    //		printf( "name: '%s'\n", (const char*)name );
    
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
	printf("Warning: spawn point key already in use!\n");
	delete p;
      }
    }
  }

  printf("Loaded spawn points: %s\n", (const char*)fileName);
  m_modified = false;
}
