/*
 * map.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qfiledialog.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qaccel.h>
#include <qcolordialog.h>
#include <qfontdialog.h>

#if 1 // ZBTEMP: Until we setup a better way to enter location name/color
#include <qinputdialog.h>
#endif

#include "map.h"
#include "util.h"
#include "main.h"
#include "filter.h"
#include "spawnlist.h"
#include "itemdb.h"


//----------------------------------------------------------------------
// Macros
//#define DEBUG

//#define DEBUGMAP

//#define DEBUGMAPLOAD


//----------------------------------------------------------------------
// constants
const int panAmmt = 8;

//----------------------------------------------------------------------
// utility functions
int keyPref(QString pref, QString section, QString def)
{
  // get the key string
  QString keyString = pSEQPrefs->getPrefString(pref, section, def);

  // get the key code
  int key = QAccel::stringToKey(keyString);
  
  // fix the key code (deal with Qt brain death)
  key &= ~Qt::UNICODE_ACCEL;

  return key;
}


// CLineDlg
CLineDlg::CLineDlg(QWidget *parent, QString name, MapMgr *mapMgr) 
  : QDialog(parent, name, FALSE)
{
#ifdef DEBUGMAP
  debug ("CLineDlg()");
#endif /* DEBUGMAP */
   
  QBoxLayout *topLayout = new QVBoxLayout(this);
  QBoxLayout *row2Layout = new QHBoxLayout(topLayout);
  QBoxLayout *row1Layout = new QHBoxLayout(topLayout);
  
  QLabel *colorLabel = new QLabel ("Color", this);
  colorLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
  colorLabel->setFixedHeight(colorLabel->sizeHint().height());
  colorLabel->setFixedWidth(colorLabel->sizeHint().width()+10);
  colorLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
  row1Layout->addWidget(colorLabel);
  
  m_LineColor = new QComboBox(FALSE, this, "LineColor");
  m_LineColor->insertItem("gray");
  m_LineColor->insertItem("darkBlue");
  m_LineColor->insertItem("darkGreen");
  m_LineColor->insertItem("darkCyan");
  m_LineColor->insertItem("darkRed");
  m_LineColor->insertItem("darkMagenta");
  /* DarkYellow is not in the standard X11R6 rgb.txt file so its
     use should be discouraged.  cpphack */
  // m_LineColor->insertItem("darkYellow");
  m_LineColor->insertItem("darkGray");
  m_LineColor->insertItem("white");
  m_LineColor->insertItem("blue");
  m_LineColor->insertItem("green");
  m_LineColor->insertItem("cyan");
  m_LineColor->insertItem("red");
  m_LineColor->insertItem("magenta");
  m_LineColor->insertItem("yellow");
  m_LineColor->insertItem("white");
  
  m_LineColor->setFont(QFont("Helvetica", 12));
  m_LineColor->setFixedHeight(m_LineColor->sizeHint().height());
  m_LineColor->setFixedWidth(m_LineColor->sizeHint().width());
  row1Layout->addWidget(m_LineColor, 0, AlignLeft);
  
  m_ColorPreview = new QFrame(this);
  m_ColorPreview->setFrameStyle(QFrame::Box|QFrame::Raised);
  m_ColorPreview->setFixedWidth(50);
  m_ColorPreview->setFixedHeight(m_LineColor->sizeHint().height());
  m_ColorPreview->setPalette(QPalette(QColor(gray)));
  row1Layout->addWidget(m_ColorPreview);
  
  // Hook on when a color changes
  connect(m_LineColor, SIGNAL(activated(const QString &)), mapMgr, SLOT(setLineColor(const QString &)));
  connect(m_LineColor, SIGNAL(activated(const QString &)), SLOT(changeColor(const QString &)));
  
  QLabel *nameLabel = new QLabel ("Name", this);
  nameLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
  nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  nameLabel->setFixedWidth(nameLabel->sizeHint().width()+5);
  nameLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
  row2Layout->addWidget(nameLabel);

  m_LineName  = new QLineEdit(this, "LineName");
  m_LineName->setFont(QFont("Helvetica", 12, QFont::Bold));
  m_LineName->setFixedHeight(m_LineName->sizeHint().height());
  m_LineName->setFixedWidth(150);
  row2Layout->addWidget(m_LineName);
  
  // Hook on when the line name changes
  connect(m_LineName, SIGNAL(textChanged(const QString &)), mapMgr, SLOT(setLineName(const QString &)));
  
  QPushButton *ok = new QPushButton("OK", this);
  ok->setFixedWidth(30);
  ok->setFixedHeight(30);
  topLayout->addWidget(ok, 0, AlignCenter);
  
  // Hook on pressing the OK button
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  
  for (int i=0; i < m_LineColor->count(); i++)
  {
    if (m_LineColor->text(i) == mapMgr->curLineColor())
      m_LineColor->setCurrentItem(i);
  }

  m_LineName->setText(mapMgr->curLineName());
}

void CLineDlg::changeColor(const QString &color)
{
#ifdef DEBUGMAP
  debug ("changeColor()");
#endif /* DEBUGMAP */
  m_ColorPreview->setPalette(QPalette(QColor(color)));
}

//----------------------------------------------------------------------
// MapLabel
MapLabel::MapLabel( Map* map ): QLabel( 0, "mapLabel", WStyle_StaysOnTop + 
        WStyle_Customize + WStyle_NoBorder + WStyle_Tool  )
{
  m_Map = map;
  setMargin( 1 );
  setIndent( 0 );
  setAutoMask( FALSE );
  setFrameStyle( QFrame::Plain | QFrame::Box );
  setLineWidth( 1 );
  setAlignment( AlignLeft | AlignTop );
  polish();
  setText( "" );
  adjustSize();        
}


//----------------------------------------------------------------------
// MapMgr
MapMgr::MapMgr(SpawnShell* spawnShell, EQPlayer* player, QWidget* dialogParent)
  : m_spawnShell(spawnShell),
    m_player(player),
    m_dialogParent(dialogParent)
{
  m_dlgLineProps = NULL;
  
  // get the preferences
  m_curLineColor = pSEQPrefs->getPrefString("DefaultLineColor", "MapMgr", "gray");
  m_curLineName = pSEQPrefs->getPrefString("DefaultLineName", "MapMgr", "line");
  m_curLocationColor = pSEQPrefs->getPrefString("DefaultLocationColor", "MapMgr", 
						"white");

  // supply the MapMgr slots with signals from SpawnShell
  connect (m_spawnShell, SIGNAL(addItem(const Item*)),
	   this, SLOT(addItem(const Item*)));
  connect (m_spawnShell, SIGNAL(delItem(const Item*)),
	   this, SLOT(delItem(const Item*)));
  connect (m_spawnShell, SIGNAL(killSpawn(const Item*)),
	   this, SLOT(killSpawn(const Item*)));
  connect (m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SLOT(changeItem(const Item*, uint32_t)));
  connect(m_spawnShell, SIGNAL(clearItems()),
	  this, SLOT(clearItems()));
}

MapMgr::~MapMgr()
{
}

void MapMgr::zoneEntry(const ServerZoneEntryStruct* zsentry)
{
#ifdef DEBUGMAP
  debug ("zoneEntry(%s)", 
	 zsentry->zoneShortName);
#endif /* DEBUGMAP */
  
  // clear the map data
  m_mapData.clear();
  
  // signal that the map has been unloaded
  emit mapUnloaded();
  
  QString fileName;
  
  // construct the map file name
  fileName.sprintf("%s/%s.map", MAPDIR, zsentry->zoneShortName);
  
  // load the map
  loadFileMap(fileName);
}

void MapMgr::zoneChange(const zoneChangeStruct* zoneChange, bool client)
{
#ifdef DEBUGMAP
  debug ("zoneChange(%s, %1d)", 
	 zoneChange->zoneName, client);
#endif /* DEBUGMAP */

  // clear the map data
  m_mapData.clear();
  
  // signal that the map has been unloaded
  emit mapUnloaded();

  QString fileName;
  
  // construct the map file name
  fileName.sprintf("%s/%s.map", MAPDIR, zoneChange->zoneName);
  
  // load the map
  loadFileMap(fileName);
}

void MapMgr::zoneNew(const newZoneStruct* zoneNew, bool client)
{
#ifdef DEBUGMAP
  debug ("zoneNew(%s, %s, %1d)", 
	 zoneNew->longName, zoneNew->shortName, client);
#endif /* DEBUGMAP */

  QString fileName;
  
  // construct the map file name
  fileName.sprintf("%s/%s.map", MAPDIR, zoneNew->shortName);
  
  // load the map if it's not already loaded
  if (fileName != m_mapData.fileName())
    loadFileMap(fileName);
}

void MapMgr::loadMap ()
{
#ifdef DEBUGMAP
  debug ("loadMap()");
#endif /* DEBUGMAP */
  
  QString fileName;

  fileName = QFileDialog::getOpenFileName(MAPDIR, "*.map");

  if (fileName.isEmpty ())
    return;

  printf("Attempting to load map: %s\n", (const char*)fileName);

  // load the map
  loadFileMap(fileName);
}


void MapMgr::loadFileMap (const QString& fileName) 
{
#ifdef DEBUGMAP
  debug ("loadFileMap()");
#endif /* DEBUGMAP */

  // load the specified map
  m_mapData.loadMap(fileName);

  const ItemMap& itemMap = m_spawnShell->spawns();
  ItemConstIterator it;
  const Item* item;
  uint16_t range;

  // iterate over the exixsting spawns to adjust the map size and find 
  // ones with aggro information
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    item = it->second;

    // Adjust X and Y for spawns on map
    m_mapData.quickCheckPos(item->xPos(), item->yPos());
    
    if (m_mapData.isAggro(item->transformedName(), &range))
    {
      // create a range to insert into the dictionary
      uint16_t* newrange = new uint16_t;
      
      // save the range value
      *newrange = range;
      
      // insert the spawns ID and aggro range into the dictionary.
      m_spawnAggroRange.insert(item->id(), newrange);
    }
  }

  // update the bounds
  m_mapData.updateBounds();

  // signal that the map has been loaded
  if (m_mapData.mapLoaded())
    emit mapLoaded();
} // END loadFileMap


void MapMgr::saveMap ()
{
#ifdef DEBUGMAP
  debug ("saveMap()");
#endif /* DEBUGMAP */
  m_mapData.saveMap();
}

void MapMgr::addItem(const Item* item)
{
  if ((item == NULL) || (item->type() != tSpawn))
    return;

  // make sure it fits on the map display
  m_mapData.checkPos(item->xPos(), item->yPos());

  uint16_t range;
  if (m_mapData.isAggro(item->transformedName(), &range))
  {
    // create a range to insert into the dictionary
    uint16_t* newrange = new uint16_t;
    
    // save the range value
    *newrange = range;
    
    // insert the spawns ID and aggro range into the dictionary.
    m_spawnAggroRange.insert(item->id(), newrange);
  }

  // signal that the map has changed
  emit mapUpdated();
}

void MapMgr::delItem(const Item* item)
{
  if (item == NULL)
    return;

  // remove from the spawn aggro dictionary
  if (item->type() == tSpawn)
    m_spawnAggroRange.remove(item->id());

  // signal that the map has changed
  emit mapUpdated();
}

void MapMgr::killSpawn(const Item* item)
{
  if ((item == NULL) || (item->type() != tSpawn))
    return;

  // make sure it fits on the map display
  m_mapData.checkPos(item->xPos(), item->yPos());

  // based on the principle of the dead can't aggro, let's remove it from
  // the spawn aggro dictionary (the undead are another matter... ;-)
  m_spawnAggroRange.remove(item->id());

  // signal that the map has changed
  emit mapUpdated();
}

void MapMgr::changeItem(const Item* item, uint32_t changeType)
{
  if (item == NULL)
    return;

  // only need to deal with position changes
  if (changeType & tSpawnChangedPosition)
  {
    // make sure it fits on the map display
    if ( m_mapData.checkPos(item->xPos(), item->yPos()))
      emit mapUpdated(); // signal if the map size has changed
  }
}

void MapMgr::clearItems()
{
  // clear the spawn aggro range info
  m_spawnAggroRange.clear();
}

void MapMgr::addLocation(QWidget* parent, const MapPoint& point)
{
  // ZBTEMP: Should create a real dialog to enter location info
  bool ok;
  QString name = QInputDialog::getText("Location Name",
				       "Please enter a location name",
				       QString::null, &ok, parent);

  // if the user clicked ok, and actually gave a name, add it
  if (ok && !name.isEmpty())
    m_mapData.addLocation(name, m_curLocationColor, 
			  QPoint(point.x(), point.y()));
  
#ifdef DEBUGMAPMAP
  printf("addLocation(): Location %d added at %d/%d\n", numLocations, 
	 point.x(), point.y()); 
#endif
}

void MapMgr::startLine(const MapPoint& point)
{
#ifdef DEBUGMAP
  debug ("startLine()");
#endif /* DEBUGMAP */
  // start the line
  m_mapData.startLine(m_curLineName, m_curLineColor, point);

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::addLinePoint(const MapPoint& point) 
{
#ifdef DEBUGMAP
  debug ("addLinePoint()");
#endif /* DEBUGMAP */
  // add a line point
  m_mapData.addLinePoint(point);

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::delLinePoint(void)
{
#ifdef DEBUGMAP
  debug ("delLinePoint()");
#endif /* DEBUGMAP */
  m_mapData.delLinePoint();

  // signal that the map has been updated
  emit mapUpdated();
} // END delLinePoint

void MapMgr::setLineName(const QString &name)
{
  // set the name of the current line
  m_mapData.setLineName(name);

  // save the name for future use
  m_curLineName = name;

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::setLineColor(const QString &color)
{
  // set the color of the current line
  m_mapData.setLineColor(color);

  // save the line color for future use
  m_curLineColor = color;

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::showLineDlg(QWidget* parent)
{
   // Creat the line properties dialog the first time it is needed
   if (m_dlgLineProps == NULL)
      m_dlgLineProps = new CLineDlg(parent, "LineDlg", this);

   m_dlgLineProps->show();
}

void MapMgr::savePrefs(void)
{
  pSEQPrefs->setPrefString("DefaultLineColor", "MapMgr", m_curLineColor);
  pSEQPrefs->setPrefString("DefaultLineName", "MapMgr", m_curLineName);
  pSEQPrefs->setPrefString("DefaultLocationColor", "MapMgr", m_curLocationColor);
}

void MapMgr::dumpInfo(QTextStream& out)
{
  out << "[MapMgr]" << endl;
  out << "DefaultLineColor: " << m_curLineColor << endl;
  out << "DefaultLineName: " << m_curLineName << endl;
  out << "DefaultLocationColor: " << m_curLocationColor << endl;
  out << "ImageLoaded: " << m_mapData.imageLoaded() << endl;
  out << "MapLoaded: " << m_mapData.mapLoaded() << endl;
  out << "MapFileName: " << m_mapData.fileName() << endl;
  out << "ZoneShortName: " << m_mapData.zoneShortName() << endl;
  out << "ZoneLongName: " << m_mapData.zoneLongName() << endl;
  out << "boundingRect: top(" << m_mapData.boundingRect().top() 
      << ") bottom(" << m_mapData.boundingRect().bottom() 
      << ") left(" << m_mapData.boundingRect().left()
      << ") right(" << m_mapData.boundingRect().right() << ") " << endl;
  out << "size: width(" << m_mapData.size().width()
      << ") height(" << m_mapData.size().height() << ")" << endl;
  out << "ZoneZEM: " << m_mapData.zoneZEM() << endl;
  out << endl;
}

//----------------------------------------------------------------------
// MapMenu
MapMenu::MapMenu(Map* map, QWidget* parent = 0, const char* name = 0)
  : m_map(map)
{
  QString preferenceName = m_map->preferenceName();

  // set the caption to be the preference name of the map
  setCaption(preferenceName);

  QPopupMenu* subMenu;

  subMenu = new QPopupMenu;
  subMenu->setCheckable(true);
  m_id_followMenu_Player = subMenu->insertItem("Player");
  subMenu->setItemParameter(m_id_followMenu_Player, tFollowPlayer);
  m_id_followMenu_Spawn = subMenu->insertItem("Spawn");
  subMenu->setItemParameter(m_id_followMenu_Spawn, tFollowSpawn);
  m_id_followMenu_None = subMenu->insertItem("None");
  subMenu->setItemParameter(m_id_followMenu_None, tFollowNone);
  connect(subMenu, SIGNAL(activated(int)),
	  this, SLOT(select_follow(int)));
  m_id_followMenu = insertItem("Follow", subMenu);

  subMenu = new QPopupMenu(m_map);
  int key; 

  key = keyPref("AddLocationKey", preferenceName, "Ctrl+O");
  m_id_addLocation = subMenu->insertItem("Add Location...",
					 m_map, SLOT(addLocation()), key);
  key = keyPref("StartLineKey", preferenceName, "Ctrl+L");
  m_id_startLine = subMenu->insertItem("Start Line",
				       m_map, SLOT(startLine()), key);
  key = keyPref("AddLinePointKey", preferenceName, "Ctrl+P");
  m_id_addLinePoint = subMenu->insertItem("Add Line Point",
					  m_map, SLOT(addLinePoint()), key);
  key = keyPref("DelLinePointKey", preferenceName, "Ctrl+D");
  m_id_delLinePoint = subMenu->insertItem("Delete Line Point",
					  m_map, SLOT(delLinePoint()), key);
  m_id_showLineDlg = subMenu->insertItem("Show Line Dialog...",
					 m_map, SLOT(showLineDlg()));
  m_id_editMap = insertItem("Edit", subMenu);

  subMenu = new QPopupMenu(m_map);
  subMenu->setCheckable(true);
  m_id_mapLineStyle_Normal = subMenu->insertItem("Normal");
  subMenu->setItemParameter(m_id_mapLineStyle_Normal, tMap_Normal);
  key = keyPref("MapLineNormalKey", preferenceName, "Alt+1");
  subMenu->setAccel(key, m_id_mapLineStyle_Normal);
  m_id_mapLineStyle_DepthFiltered = subMenu->insertItem("Depth Filtered");
  subMenu->setItemParameter(m_id_mapLineStyle_DepthFiltered, tMap_DepthFiltered);
  key = keyPref("MapLineDepthFilteredKey", preferenceName, "Alt+2");
  subMenu->setAccel(key, m_id_mapLineStyle_DepthFiltered);
  m_id_mapLineStyle_FadedFloors = subMenu->insertItem("Faded Floors");
  subMenu->setItemParameter(m_id_mapLineStyle_FadedFloors, tMap_FadedFloors);
  key = keyPref("MapLineFadedFloorsKey", preferenceName, "Alt+3");
  subMenu->setAccel(key, m_id_mapLineStyle_FadedFloors);
  connect(subMenu, SIGNAL(activated(int)),
	  this, SLOT(select_mapLine(int)));
  m_id_mapLineStyle = insertItem("Map Line Display", subMenu);

  m_id_spawnDepthFilter = insertItem("Spawn Depth Filter", 
				     this, SLOT(toggle_spawnDepthFilter(int)));
  m_id_tooltip = insertItem("Show Tooltips", 
			    this, SLOT(toggle_tooltip(int)));;
  m_id_filtered = insertItem("Show Filtered",
			     this, SLOT(toggle_filtered(int)));
  m_id_map = insertItem("Show Map Lines",
			this, SLOT(toggle_map(int)));
  m_id_velocity = insertItem("Show Velocity Lines",
			     this, SLOT(toggle_velocity(int)));
  m_id_animate = insertItem("Animate Spawns",
			    this, SLOT(toggle_animate(int)));
  m_id_player = insertItem("Show Player",
			   this, SLOT(toggle_player(int)));
  m_id_playerBackground = insertItem("Show Player Background", 
				     this, SLOT(toggle_playerBackground(int)));
  m_id_playerView = insertItem("Show Player View",
			       this, SLOT(toggle_playerView(int)));
  m_id_gridLines = insertItem("Show Grid Lines",
			      this, SLOT(toggle_gridLines(int)));;
  m_id_gridTicks = insertItem("Show Grid Ticks", 
			      this, SLOT(toggle_gridTicks(int)));
  m_id_locations = insertItem("Show Locations",
			      this, SLOT(toggle_locations(int)));
  m_id_spawns = insertItem("Show Spawns",
			   this, SLOT(toggle_spawns(int)));
  m_id_drops = insertItem("Show Drops",








			  this, SLOT(toggle_drops(int)));
  m_id_coins = insertItem("Show Coins",
			  this, SLOT(toggle_coins(int)));
  m_id_doors = insertItem("Show Doors",
			  this, SLOT(toggle_doors(int)));
  m_id_spawnNames = insertItem("Show SpawnNames",
			       this, SLOT(toggle_spawnNames(int)));
  m_id_highlightConsideredSpawns =
    insertItem("Highlight Considered Spawns",
	       this, SLOT(toggle_highlightConsideredSpawns(int)));
  m_id_walkPath = insertItem("Show Selections Walk Path",
			     this, SLOT(toggle_walkPath(int)));
  m_id_mapImage = insertItem("Show Map Image",
			     this, SLOT(toggle_mapImage(int)));
#ifdef DEBUG
  m_id_debugInfo = insertItem("Show Debug Info",
			      this, SLOT(toggle_debugInfo(int)));
#endif

  subMenu = new QPopupMenu;
  subMenu->setCheckable(true);
  m_id_mapOptimization_Memory = subMenu->insertItem("Memory");
  subMenu->setItemParameter(m_id_mapOptimization_Memory, tMap_MemoryOptim); 
 m_id_mapOptimization_Normal = subMenu->insertItem("Normal");
  subMenu->setItemParameter(m_id_mapOptimization_Normal, tMap_NormalOptim);
  m_id_mapOptimization_Best = subMenu->insertItem("Speed");
  subMenu->setItemParameter(m_id_mapOptimization_Best, tMap_BestOptim);
  m_id_mapOptimization = insertItem("Map Optimization", subMenu);

  m_id_gridTickColor = insertItem("Grid Tick Color...",
			      this, SLOT(select_gridTickColor(int)));
  m_id_gridLineColor = insertItem("Grid Line Color...",
			      this, SLOT(select_gridLineColor(int)));
  m_id_backgroundColor = insertItem("Background Color...",
				    this, SLOT(select_backgroundColor(int)));
  m_id_font = insertItem("Font...",
			 this, SLOT(select_font(int)));

  connect(this, SIGNAL(aboutToShow()),
	  this, SLOT(init_Menu()));
}

MapMenu::~MapMenu()
{
}

void MapMenu::init_Menu(void)
{
  FollowMode mode = m_map->followMode();
  setItemChecked(m_id_followMenu_Player, (mode == tFollowPlayer));
  setItemChecked(m_id_followMenu_Spawn, (mode == tFollowSpawn));
  setItemChecked(m_id_followMenu_None, (mode == tFollowNone));
  
  const Item* selectedItem = m_map->selectedItem();
  setItemEnabled(m_id_followMenu_Spawn, 
		 ((selectedItem != NULL) && (selectedItem->type() == tSpawn)));

  MapLineStyle style = m_map->mapLineStyle();
  setItemChecked(m_id_mapLineStyle_Normal, (style == tMap_Normal));
  setItemChecked(m_id_mapLineStyle_DepthFiltered, (style == tMap_DepthFiltered));
  setItemChecked(m_id_mapLineStyle_FadedFloors, (style == tMap_FadedFloors));
  setItemChecked(m_id_spawnDepthFilter, m_map->spawnDepthFilter());
  setItemChecked(m_id_tooltip, m_map->showTooltips());
  setItemChecked(m_id_filtered, m_map->showFiltered());
  setItemChecked(m_id_map, m_map->showLines());
  setItemChecked(m_id_velocity, m_map->showVelocityLines());
  setItemChecked(m_id_animate, m_map->animate());
  setItemChecked(m_id_player, m_map->showPlayer());
  setItemChecked(m_id_playerBackground, m_map->showPlayerBackground());
  setItemChecked(m_id_playerView, m_map->showPlayerView());
  setItemChecked(m_id_gridLines, m_map->showGridLines());
  setItemChecked(m_id_gridTicks, m_map->showGridTicks());
  setItemChecked(m_id_locations, m_map->showLocations());
  setItemChecked(m_id_spawns, m_map->showSpawns());
  setItemChecked(m_id_drops, m_map->showDrops());
  setItemChecked(m_id_coins, m_map->showCoins());
  setItemChecked(m_id_doors, m_map->showDoors());
  setItemChecked(m_id_spawnNames, m_map->showSpawnNames());
  setItemChecked(m_id_highlightConsideredSpawns, 
		 m_map->highlightConsideredSpawns());
  setItemChecked(m_id_walkPath, m_map->walkPathShowSelect());
  setItemChecked(m_id_mapImage, m_map->showBackgroundImage());
#ifdef DEBUG
  setItemChecked(m_id_debugInfo, m_map->showDebugInfo());
#endif
  MapOptimizationMethod method = m_map->mapOptimization();

  setItemChecked(m_id_mapOptimization_Memory, (method == tMap_MemoryOptim));
  setItemChecked(m_id_mapOptimization_Normal, (method == tMap_NormalOptim));
  setItemChecked(m_id_mapOptimization_Best, (method == tMap_BestOptim));
}

void MapMenu::select_follow(int itemId)
{
  // set the selected follow mode
  m_map->setFollowMode((FollowMode)itemParameter(itemId));
}

void MapMenu::select_mapLine(int itemId)
{
  m_map->setMapLineStyle((MapLineStyle)itemParameter(itemId));
}

void MapMenu::toggle_spawnDepthFilter(int itemId)
{
  m_map->setSpawnDepthFilter(!m_map->spawnDepthFilter());
}

void MapMenu::toggle_tooltip(int itemId)
{
  m_map->setShowTooltips(!m_map->showTooltips());
}

void MapMenu::toggle_filtered(int itemId)
{
  m_map->setShowFiltered(!m_map->showFiltered());
}

void MapMenu::toggle_map(int itemId)
{
  m_map->setShowLines(!m_map->showLines());
}

void MapMenu::toggle_velocity(int itemId)
{
  m_map->setShowVelocityLines(!m_map->showVelocityLines());
}

void MapMenu::toggle_animate(int itemId)
{
  m_map->setAnimate(!m_map->animate());
}

void MapMenu::toggle_player(int itemId)
{
  m_map->setShowPlayer(!m_map->showPlayer());
}

void MapMenu::toggle_playerBackground(int itemId)
{
  m_map->setShowPlayerBackground(!m_map->showPlayerBackground());
}

void MapMenu::toggle_playerView(int itemId)
{
  m_map->setShowPlayerView(!m_map->showPlayerView());
}

void MapMenu::toggle_gridLines(int itemId)
{
  m_map->setShowGridLines(!m_map->showGridLines());
}

void MapMenu::toggle_gridTicks(int itemId)
{
  m_map->setShowGridTicks(!m_map->showGridTicks());
}

void MapMenu::toggle_locations(int itemId)
{
  m_map->setShowLocations(!m_map->showLocations());
}

void MapMenu::toggle_spawns(int itemId)
{
  m_map->setShowSpawns(!m_map->showSpawns());
}

void MapMenu::toggle_drops(int itemId)
{
  m_map->setShowDrops(!m_map->showDrops());
}

void MapMenu::toggle_coins(int itemId)
{
  m_map->setShowCoins(!m_map->showCoins());
}

void MapMenu::toggle_doors(int itemId)
{
  m_map->setShowDoors(!m_map->showDoors());
}

void MapMenu::toggle_spawnNames(int itemId)
{
  m_map->setShowSpawnNames(!m_map->showSpawnNames());
}

void MapMenu::toggle_highlightConsideredSpawns(int itemId)
{
  m_map->setHighlightConsideredSpawns(!m_map->highlightConsideredSpawns());
}

void MapMenu::toggle_walkPath(int itemId)
{
  m_map->setWalkPathShowSelect(!m_map->walkPathShowSelect());
}

void MapMenu::toggle_mapImage(int itemId)
{
  m_map->setShowBackgroundImage(!m_map->showBackgroundImage());
}

#ifdef DEBUG
void MapMenu::toggle_debugInfo(int itemId)
{
  m_map->setShowDebugInfo(!m_map->showDebugInfo());
}
#endif

void MapMenu::select_gridTickColor(int itemId)
{
  QString name = QString("ShowEQ - ") + m_map->preferenceName() 
    + " Grid Tick Color";
  QColor newColor = QColorDialog::getColor(m_map->gridTickColor(),
					   m_map, (const char*)name);

  if (newColor.isValid())
    m_map->setGridTickColor(newColor);
}

void MapMenu::select_gridLineColor(int itemId)
{
  QString name = QString("ShowEQ - ") + m_map->preferenceName() 
    + " Grid Tick Color";
  QColor newColor = QColorDialog::getColor(m_map->gridLineColor(),
					   m_map, (const char*)name);

  if (newColor.isValid())
    m_map->setGridLineColor(newColor);
}

void MapMenu::select_backgroundColor(int itemId)
{
  QString name = QString("ShowEQ - ") + m_map->preferenceName() 
    + " Background Color";
  QColor newColor = QColorDialog::getColor(m_map->backgroundColor(),
					   m_map, (const char*)name);

  if (newColor.isValid())
    m_map->setBackgroundColor(newColor);
}

void MapMenu::select_font(int itemId)
{
  QString name = QString("ShowEQ - ") + m_map->preferenceName() 
    + " Font";
  bool ok = false;
  QFont newFont;
  newFont = QFontDialog::getFont(&ok, m_map->font(), m_map, (const char*)name);

#if 1 // ZBTEMP: Test
  printf("New Font Key: %s\n", (const char*)newFont.key());
#endif

  if (ok)
    m_map->setFont(newFont);
}



//----------------------------------------------------------------------
// Map
Map::Map(MapMgr* mapMgr, 
	 EQPlayer* player, 
	 SpawnShell* spawnshell, 
	 const QString& preferenceName, 
	 uint32_t runtimeFilterFlagMask,
	 QWidget * parent, 
	 const char *name)
  : QWidget (parent, name),
    m_preferenceName(preferenceName),
    m_param(mapMgr->mapData()),
    m_mapMgr(mapMgr),
    m_mapCache(mapMgr->mapData()),
    m_menu(NULL),
    m_runtimeFilterFlagMask(runtimeFilterFlagMask),
    m_player(player),
    m_spawnShell(spawnshell)
{
#ifdef DEBUGMAP
  debug ("Map()");
#endif /* DEBUGMAP */

  // save the name used for preferences 
  QString prefString = Map::preferenceName();
  QString tmpPrefString;
  QString tmpDefault;

  tmpPrefString = "Caption";
  tmpDefault = QString("ShowEQ - ") + prefString;
  setCaption(pSEQPrefs->getPrefString(tmpPrefString, tmpDefault));

  tmpPrefString = "AnimateSpawnMovement";
  m_animate = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "VelocityLines";
  m_showVelocityLines = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "SpawnDepthFilter";
  m_spawnDepthFilter = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 0);

  tmpPrefString = "Framerate";
  m_frameRate = pSEQPrefs->getPrefInt(tmpPrefString, prefString, 5);

#ifdef DEBUG
  tmpPrefString = "ShowDebugInfo";
  m_showDebugInfo = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 0);
#endif

  tmpPrefString = "ShowPlayer";
  m_showPlayer = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowPlayerBackground";
  m_showPlayerBackground = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowPlayerView";
  m_showPlayerView = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowDroppedItems";
  m_showDrops = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowDroppedCoins";
  m_showCoins = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowDoors";
  m_showDoors = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowSpawns";
  m_showSpawns = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowSpawnNames";
  m_showSpawnNames = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 0);

  tmpPrefString = "HighlightConsideredSpawns";
  m_highlightConsideredSpawns = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "ShowTooltips";
  m_showTooltips = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "WalkPathShowSelect";
  m_walkpathshowselect = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1);

  tmpPrefString = "DrawSize";
  m_drawSize = pSEQPrefs->getPrefInt(tmpPrefString, prefString, 3);


  // mainly for backwards compatibility
  tmpPrefString = "MapDepthFilter";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, 0))
    m_param.setMapLineStyle(tMap_DepthFiltered);

  tmpPrefString = "FadingFloors";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, 0))
    m_param.setMapLineStyle(tMap_FadedFloors);

  // the new setting overrides old settings
  tmpPrefString = "MapLineStyle";
  if (pSEQPrefs->isPreference(tmpPrefString, prefString))
    m_param.setMapLineStyle((MapLineStyle)pSEQPrefs->getPrefInt(tmpPrefString, prefString,
								tMap_Normal));

  tmpPrefString = "ShowMapPoints";
  m_param.setShowLocations(pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1));

  tmpPrefString = "ShowMapLines";
  m_param.setShowLines(pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1));

  tmpPrefString = "ShowGridLines";
  m_param.setShowGridLines(pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1));

  tmpPrefString = "ShowGridTicks";
  m_param.setShowGridTicks(pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1));

  tmpPrefString = "ShowBackgroundImage";
  m_param.setShowBackgroundImage(pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1));

  tmpPrefString = "GridResolution";
  m_param.setGridResolution(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 500));

  // default is Null string, so if no preference stringToFont will use app 
  // default.
  tmpPrefString = "Font";
  QString tmpFontString = pSEQPrefs->getPrefString(tmpPrefString, prefString, "");
  m_param.setFont(stringToFont(tmpFontString));

  tmpPrefString = "GridTickColor";
  m_param.setGridTickColor(QColor(pSEQPrefs->getPrefString(tmpPrefString, prefString, "#E1C819")));

  tmpPrefString = "GridLineColor";
  m_param.setGridLineColor(QColor(pSEQPrefs->getPrefString(tmpPrefString, prefString, "#194819")));

  tmpPrefString = "BackgroundColor";
  m_param.setBackgroundColor(QColor(pSEQPrefs->getPrefString(tmpPrefString, prefString, "black")));

  tmpPrefString = "OverheadDepth";
  m_param.setHeadRoom(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 75));

  tmpPrefString = "UnderfeetDepth";
  m_param.setFloorRoom(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 75));

  tmpPrefString = "OptimizeMethod";
  m_param.setMapOptimizationMethod((MapOptimizationMethod)pSEQPrefs->getPrefInt(tmpPrefString, prefString, (int)tMap_NormalOptim));

  tmpPrefString = "CacheAlwaysRepaint";
  m_mapCache.setAlwaysRepaint(pSEQPrefs->getPrefBool(tmpPrefString, prefString, false));

  tmpPrefString = "DeityPvP";
  m_deityPvP = pSEQPrefs->getPrefBool(tmpPrefString, prefString, showeq_params->deitypvp);

  tmpPrefString = "RacePvP";
  m_racePvP = pSEQPrefs->getPrefBool(tmpPrefString, prefString, showeq_params->pvp);

  // Accelerators
  QAccel *accel = new QAccel(this);
  int key;
  key = keyPref("ZoomInKey", prefString, "+");
  accel->connectItem(accel->insertItem(key), this, SLOT(zoomIn()));

  key = keyPref("ZoomOutKey", prefString, "-");
  accel->connectItem(accel->insertItem(key), this, SLOT(zoomOut()));

  key = keyPref("PanDownLeftKey", prefString, "Ctrl+1");
  accel->connectItem(accel->insertItem(key), this, SLOT(panDownLeft()));

  key = keyPref("PanDownKey", prefString, "Ctrl+2");
  accel->connectItem(accel->insertItem(key), this, SLOT(panDown()));

  key = keyPref("PanDownRightKey", prefString, "Ctrl+3");
  accel->connectItem(accel->insertItem(key), this, SLOT(panDownRight()));

  key = keyPref("PanLeftKey", prefString, "Ctrl+4");
  accel->connectItem(accel->insertItem(key), this, SLOT(panLeft()));

  key = keyPref("CenterSelectedKey", prefString, "Ctrl+5");
  accel->connectItem(accel->insertItem(key), this, SLOT(viewTarget()));

  key = keyPref("PanRightKey", prefString, "Ctrl+6");
  accel->connectItem(accel->insertItem(key), this, SLOT(panRight()));

  key = keyPref("PanUpLeftKey", prefString, "Ctrl+7");
  accel->connectItem(accel->insertItem(key), this, SLOT(panUpLeft()));

  key = keyPref("PanUpKey", prefString, "Ctrl+8");
  accel->connectItem(accel->insertItem(key), this, SLOT(panUp()));

  key = keyPref("PanUpRightKey", prefString, "Ctrl+9");
  accel->connectItem(accel->insertItem(key), this, SLOT(panUpRight()));

  key = keyPref("ViewLockKey", prefString, "Ctrl+0");
  accel->connectItem(accel->insertItem(key), this, SLOT(viewLock()));

  m_followMode = tFollowPlayer;
  
  m_selectedItem = NULL;

#ifdef DEBUG
  for (int i = 0; i < maxFrameTimes; i++)
    m_frameTimes[i] = 0;
  m_frameTimeIndex = 0;
  m_paintCount = 0;
  m_paintTimeSum = 0;
#endif

  m_lastFlash.start();
  m_flash = false;
  
  // Setup m_param
  m_param.setScreenSize(size());
  
  // Setup offscreen image
  m_offscreen.resize(m_param.screenLength());
  m_offscreen.setOptimization(m_param.pixmapOptimizationMethod());
  
  m_mapTip = new MapLabel( this );
  this->setMouseTracking( TRUE );

  m_mapPanning = false;

  setMouseTracking(true);
  
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), 
	  this, SLOT(refreshMap()));
  
  // supply the Map slots with signals from MapMgr
  connect(m_mapMgr, SIGNAL(mapLoaded()),
	  this, SLOT(mapLoaded()));
  connect(m_mapMgr, SIGNAL(mapUnloaded()),
	  this, SLOT(mapUnloaded()));
  connect(m_mapMgr, SIGNAL(mapUpdated()),
	  this, SLOT(mapUpdated()));

  // supply the Map slots with signals from SpawnShell
  connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	  this, SLOT(delItem(const Item*)));
  connect(m_spawnShell, SIGNAL(clearItems()),
	  this, SLOT(clearItems()));
  connect (m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SLOT(changeItem(const Item*, uint32_t)));

  //  if (m_mapMgr->mapData().mapLoaded() || showeq_params->fast_machine)
    m_timer->start(1000/m_frameRate, false);

#ifdef DEBUG  
  if (m_showDebugInfo)
    m_time.start();
#endif
} // end Map() constructor

Map::~Map(void)
{
}

void Map::savePrefs(void)
{
  QString prefString = preferenceName();
  QString tmpPrefString;

  tmpPrefString = "AnimateSpawnMovement";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_animate);

  tmpPrefString = "VelocityLines";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showVelocityLines);

  tmpPrefString = "SpawnDepthFilter";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_spawnDepthFilter);

  tmpPrefString = "Framerate";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_frameRate);

#ifdef DEBUG
  tmpPrefString = "ShowDebugInfo";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showDebugInfo);
#endif

  tmpPrefString = "ShowPlayer";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showPlayer);

  tmpPrefString = "ShowPlayerBackground";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showPlayerBackground);

  tmpPrefString = "ShowPlayerView";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showPlayerView);

  tmpPrefString = "ShowDroppedItems";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showDrops);

  tmpPrefString = "ShowDroppedCoins";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showCoins);

  tmpPrefString = "ShowDoors";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showDoors);

  tmpPrefString = "ShowSpawns";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showSpawns);

  tmpPrefString = "ShowSpawnNames";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showSpawnNames);

  tmpPrefString = "HighlightConsideredSpawns";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_highlightConsideredSpawns);

  tmpPrefString = "ShowTooltips";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_showTooltips);

  tmpPrefString = "WalkPathShowSelect";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_walkpathshowselect);

  tmpPrefString = "DrawSize";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_drawSize);

  tmpPrefString = "MapLineStyle";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.mapLineStyle());

  tmpPrefString = "ShowMapPoints";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.showLocations());

  tmpPrefString = "ShowMapLines";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.showLines());

  tmpPrefString = "ShowGridLines";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.showGridLines());

  tmpPrefString = "ShowGridTicks";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.showGridTicks());

  tmpPrefString = "ShowBackgroundImage";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.showBackgroundImage());

  tmpPrefString = "GridResolution";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.gridResolution());

  tmpPrefString = "GridTickColor";
  pSEQPrefs->setPrefString(tmpPrefString, prefString, m_param.gridTickColor().name());

  tmpPrefString = "GridLineColor";
  pSEQPrefs->setPrefString(tmpPrefString, prefString, m_param.gridLineColor().name());

  tmpPrefString = "BackgroundColor";
  pSEQPrefs->setPrefString(tmpPrefString, prefString, m_param.backgroundColor().name());

  tmpPrefString = "OverheadDepth";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.headRoom());

  tmpPrefString = "UnderfeetDepth";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_param.floorRoom());

  tmpPrefString = "OptimizeMethod";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString,  
			  (int)m_param.mapOptimizationMethod());

  tmpPrefString = "CacheAlwaysRepaint";
  pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_mapCache.alwaysRepaint());

  if (showeq_params->deitypvp != m_deityPvP)
  {
    tmpPrefString = "DeityPvP";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_deityPvP);
  }

  if (showeq_params->pvp != m_racePvP)
  {
    tmpPrefString = "RacePvP";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_racePvP);
  }

  tmpPrefString = "Font";
  pSEQPrefs->setPrefString(tmpPrefString, prefString, fontToString(m_param.font()));
}

MapMenu* Map::menu()
{
  if (m_menu != NULL)
    return m_menu;

  m_menu = new MapMenu(this, this, "map menu");

  return m_menu;
}

QSize Map::sizeHint() const // preferred size
{
#ifdef DEBUGMAP
  debug ("sizeHint()");
#endif /* DEBUGMAP */
  
  return QSize(600, 600);
}

QSize Map::minimumSizeHint() const // minimum size
{
#ifdef DEBUGMAP
  debug ("minimumSizeHint()");
#endif /* DEBUGMAP */
  return QSize(300, 300);
}

QSizePolicy Map::sizePolicy() const // size policy
{
#ifdef DEBUGMAP
  debug ("sizePolicy()");
#endif /* DEBUGMAP */
  return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Map::mouseDoubleClickEvent(QMouseEvent * me)
{
#ifdef DEBUGMAP
  debug ("mouseDoubleClickEvent()");
#endif /* DEBUGMAP */
  if (me->button () == MidButton)
    viewTarget();
}

void Map::mouseReleaseEvent(QMouseEvent* me)
{
#ifdef DEBUGMAP
  debug ("mouseReleaseEvent()");
#endif /* DEBUGMAP */
  if (me->button() == MidButton)
    m_mapPanning = false;
}

void Map::mousePressEvent(QMouseEvent* me)
{
#ifdef DEBUGMAP
  debug ("mousePressEvent()");
#endif /* DEBUGMAP */
  if (me->button () == RightButton) 
  {
    // display the Map's menu
    menu()->popup(mapToGlobal(me->pos()));
    FILE *f;
    f=fopen("/tmp/coords","at");
    if(f) 
    {
      fprintf (f,"%f, %f\n",
	       (m_param.screenCenterX() - me->x()) * m_param.ratioX(),
	       (m_param.screenCenterY() - me->y()) * m_param.ratioY());
      fclose(f);
    }
  }
  else if (me->button () == MidButton) 
  {
    m_mapPanning = true;
    m_mapPanX     = me->x ();
    m_mapPanY     = me->y ();
  } 
  else 
  {
    const Item* closestSpawn;
    // find the nearest spawn within a reasonable range
    closestSpawn = closestSpawnToPoint(me->pos(), 15);
    
    // make sure the user actually clicked vaguely near a spawn
    if (closestSpawn != NULL)
    {
      // note new selection
      m_selectedItem = closestSpawn;
      
      // notify others of new selection
      emit spawnSelected(m_selectedItem);
      
      // reAdjust to make sure it's focused around
      reAdjust();

      // repaint if necessary
      if(!showeq_params->fast_machine)
	refreshMap ();
    }
  }
}

void Map::zoomIn()
{
#ifdef DEBUGMAP
   debug("Map::zoomIn()");
#endif /* DEBUGMAP */
   if (m_player->getPlayerID() != 1)
   {
     if (m_param.zoomIn())
     {
       emit zoomChanged(m_param.zoom());

       // requires ReAdjust
       reAdjust();

       if (!showeq_params->fast_machine)
         refreshMap ();
     }
   }
}

void Map::zoomOut()
{
#ifdef DEBUGMAP
   debug("Map::zoomOut()");
#endif /* DEBUGMAP */

   if (m_player->getPlayerID() != 1)
   {
     if (m_param.zoomOut())
     {
       emit zoomChanged(m_param.zoom());

       // requires ReAdjust
       reAdjust();

       if(!showeq_params->fast_machine)
         refreshMap ();
     }
   }    
}

void Map::panRight()
{
#ifdef DEBUGMAP
   debug("Map::panRight()");
#endif /* DEBUGMAP */
   m_param.panX(-panAmmt);

   emit panXChanged(m_param.panOffsetX());
   
   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::panLeft()
{
#ifdef DEBUGMAP
   debug("Map::panLeft()");
#endif /* DEBUGMAP */
   m_param.panX(panAmmt);

   emit panXChanged(m_param.panOffsetX());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}


void Map::panDown()
{
#ifdef DEBUGMAP
   debug("Map::panDown()");
#endif /* DEBUGMAP */
   m_param.panY(-panAmmt);

   emit panYChanged(m_param.panOffsetY());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}


void Map::panUp()
{
#ifdef DEBUGMAP
   debug("Map::panUp()");
#endif /* DEBUGMAP */
   m_param.panY(panAmmt);

   emit panYChanged(m_param.panOffsetY());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::panUpRight()
{
#ifdef DEBUGMAP
   debug("Map::panUpRight()");
#endif /* DEBUGMAP */
   m_param.panXY(-panAmmt, panAmmt);

   emit panXChanged(m_param.panOffsetX());
   emit panYChanged(m_param.panOffsetY());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::panUpLeft()
{
#ifdef DEBUGMAP
   debug("Map::panUpLeft()");
#endif /* DEBUGMAP */
   m_param.panXY(panAmmt, panAmmt);

   emit panXChanged(m_param.panOffsetX());
   emit panYChanged(m_param.panOffsetY());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::panDownRight()
{
#ifdef DEBUGMAP
   debug("Map::panDownRight()");
#endif /* DEBUGMAP */
   m_param.panXY(-panAmmt, -panAmmt);

   emit panXChanged(m_param.panOffsetX());
   emit panYChanged(m_param.panOffsetY());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::panDownLeft()
{
#ifdef DEBUGMAP
   debug("Map::panDownLeft()");
#endif /* DEBUGMAP */
   m_param.panXY(panAmmt, -panAmmt);

   emit panXChanged(m_param.panOffsetX());
   emit panYChanged(m_param.panOffsetY());

   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::increaseGridResolution (void)
{
  m_param.increaseGridResolution();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::decreaseGridResolution (void)
{
  m_param.decreaseGridResolution();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::viewTarget()
{
#ifdef DEBUGMAP
  debug("Map::viewTarget()");
#endif /* DEBUGMAP */
  
  switch (m_followMode)
  {
  case tFollowSpawn:
  case tFollowPlayer:
    m_param.clearPan();
    emit panXChanged(m_param.panOffsetX());
    emit panYChanged(m_param.panOffsetY());
    break;
  case tFollowNone:
    m_param.clearPan();
    emit panXChanged(m_param.panOffsetX());
    emit panYChanged(m_param.panOffsetY());
    if (m_selectedItem != NULL)
      m_followMode = tFollowSpawn;
    else
      m_followMode = tFollowPlayer;
  };
  
  reAdjust();
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::viewLock()
{
#ifdef DEBUGMAP
   debug("Map::viewLock()");
#endif /* DEBUGMAP */
   switch (m_followMode)
   {
   case tFollowNone:
     // next mode is focused on selection if there is one or player if not
     if (m_selectedItem != NULL)
       m_followMode = tFollowSpawn;
     else
       m_followMode = tFollowPlayer;
     m_param.clearPan();
     emit panXChanged(m_param.panOffsetX());
     emit panYChanged(m_param.panOffsetY());
     break;
   case tFollowSpawn:
     if (m_selectedItem != NULL)
     {
       // next mode is follow none
       m_followMode = tFollowNone;
       MapPoint location;
       if (m_selectedItem->type() == tSpawn)
       {
	 const Spawn* spawn = (const Spawn*)m_selectedItem;
	 
	 spawn->approximatePosition(m_animate, QTime::currentTime(),
				      location);
       }
       else
	 location.setPoint(*m_selectedItem);
       m_param.setPan(location.xPos(), location.yPos());
       emit panXChanged(m_param.panOffsetX());
       emit panYChanged(m_param.panOffsetY());
     }
     else
     {
       // next mode is follow player
       m_followMode = tFollowPlayer;
       m_param.clearPan();
       emit panXChanged(m_param.panOffsetX());
       emit panYChanged(m_param.panOffsetY());
     }
     break;
   case tFollowPlayer:
     if (m_selectedItem == NULL)
     {
       // next mode is follow none
       m_followMode = tFollowNone;

       // retrieve the approximate position of the player
       const Spawn* player = m_spawnShell->playerSpawn();
       
       // retrieve the approximate current player position
       MapPoint targetPoint;
       player->approximatePosition(m_animate, QTime::currentTime(),
				   targetPoint);
       
       // set the current pan to it's position to avoid jarring the user
       m_param.setPan(targetPoint.xPos(), targetPoint.yPos());
       emit panXChanged(m_param.panOffsetX());
       emit panYChanged(m_param.panOffsetY());
     }
     else
     {
       // next mode is follow spawn
       m_followMode = tFollowSpawn;
       m_param.clearPan();
       emit panXChanged(m_param.panOffsetX());
       emit panYChanged(m_param.panOffsetY());
     }
     break;
   }

  // this requires a reAdjust
   reAdjust();

   if(!showeq_params->fast_machine)
     refreshMap ();
}

void Map::setFollowMode(FollowMode mode) 
{ 
  // if the mode is the same, then nothing to do
  if (m_followMode == mode)
    return;

  switch(mode)
  {
  case tFollowSpawn:
    // if no spawn is selected, ignore the new setting
    if (m_selectedItem == NULL)
      return;

    // clear any panning parameters
    m_param.clearPan();
  case tFollowPlayer:
    m_param.clearPan();
    break;
  case tFollowNone:
    if (m_followMode == tFollowPlayer)
    {
       // retrieve the approximate position of the player
       const Spawn* player = m_spawnShell->playerSpawn();
       
       // retrieve the approximate current player position
       MapPoint targetPoint;
       player->approximatePosition(m_animate, QTime::currentTime(),
				   targetPoint);
       
       // set the current pan to it's position to avoid jarring the user
       m_param.setPan(targetPoint.xPos(), targetPoint.yPos());
    }
    else if (m_followMode == tFollowSpawn)
    {
       m_followMode = tFollowNone;
       if (m_selectedItem)
       {
	 MapPoint location;
	 if (m_selectedItem->type() == tSpawn)
	 {
	   const Spawn* spawn = 
	     (const Spawn*)m_selectedItem;
	   spawn->approximatePosition(m_animate, QTime::currentTime(),
				      location);
	 }
	 else
	   location.setPoint(*m_selectedItem);
	 m_param.setPan(location.xPos(), location.yPos());
       }
       else
	 m_param.clearPan();
    }
    else // in case someone adds a new mode and forgets us...
      m_param.clearPan();
  }

  emit panXChanged(m_param.panOffsetX());
  emit panYChanged(m_param.panOffsetY());

  m_followMode = mode; 
  
  // this requires a reAdjust
  reAdjust();
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

//
// ShowFiltered
//
// Toggle viewing of filtered spawns in map - they will show as grey dots
//
void
Map::setShowFiltered(bool bView)
{
  m_showFiltered = bView;

  refreshMap();
}

void Map::setFrameRate(int val) 
{ 
  // make sure the value is within range
  if ((val >= 1) && (val <= 60))
  {
    m_frameRate = val;

    emit frameRateChanged(m_frameRate);

    if (/*(m_player->getPlayerID() != 1) &&*/ m_timer->isActive())
      m_timer->changeInterval(1000/m_frameRate);
  }
}

void Map::setDrawSize(int val) 
{ 
  if ((val < 1) || (val > 6))
    return;

  m_drawSize = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowMapLines(bool val) 
{ 
  m_showMapLines = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowPlayer(bool val) 
{ 
  m_showPlayer = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowPlayerBackground(bool val) 
{ 
  m_showPlayerBackground = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowPlayerView(bool val) 
{ 
  m_showPlayerView = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowHeading(bool val) 
{ 
  m_showHeading = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowSpawns(bool val) 
{ 
  m_showSpawns = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowDrops(bool val) 
{ 
  m_showDrops = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowCoins(bool val) 
{ 
  m_showCoins = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowDoors(bool val) 
{ 
  m_showDoors = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowSpawnNames(bool val) 
{ 
  m_showSpawnNames = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowVelocityLines(bool val) 
{ 
  m_showVelocityLines = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

#ifdef DEBUG
void Map::setShowDebugInfo(bool val) 
{ 
  m_showDebugInfo = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}
#endif

void Map::setAnimate(bool val) 
{ 
  m_animate = val; 

  // this requires a reAdjust
  reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setSpawnDepthFilter(bool val)
{
  m_spawnDepthFilter = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setHighlightConsideredSpawns(bool val) 
{ 
  m_highlightConsideredSpawns = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowTooltips(bool val) 
{ 
  m_showTooltips = val; 

  // make sure it's hidden if they hid it
  if (!m_showTooltips)
    m_mapTip->hide();
}

void Map::setWalkPathShowSelect(bool val) 
{ 
  m_walkpathshowselect = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setDeityPvP(bool val) 
{ 
  m_deityPvP = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setRacePvP(bool val) 
{ 
  m_racePvP = val; 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setMapLineStyle(MapLineStyle style) 
{ 
  m_param.setMapLineStyle(style); 
  
  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setMapOptimization(MapOptimizationMethod method) 
{ 
  // set the general optimization method
  m_param.setMapOptimizationMethod(method);

  // set the offscreen images optimization method
  m_offscreen.setOptimization(m_param.pixmapOptimizationMethod());

}

void Map::setZoom(int val) 
{ 
  if (m_player->getPlayerID() != 1)
  {
    if (m_param.setZoom(val))
    {
      emit zoomChanged(m_param.zoom());

      // requires reAdjust
      reAdjust();
      
      if (!showeq_params->fast_machine)
	refreshMap ();
    }
  }
}

void Map::setPanOffsetX(int val) 
{ 
  m_param.setPanX(val); 

  // this requires a reAdjust
  reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setPanOffsetY(int val) 
{ 
  m_param.setPanY(val); 

  // this requires a reAdjust
  reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setGridResolution(int val) 
{ 
  m_param.setGridResolution(val); 

  reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setGridTickColor(const QColor& color) 
{ 
  m_param.setGridTickColor(color); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setGridLineColor(const QColor& color) 
{ 
  m_param.setGridLineColor(color); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setBackgroundColor(const QColor& color) 
{ 
  m_param.setBackgroundColor(color); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setFont(const QFont& font) 
{ 
  m_param.setFont(font); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setHeadRoom(int val) 
{ 
  m_param.setHeadRoom(val); 

  emit headRoomChanged(m_param.headRoom());

  reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setFloorRoom(int val) 
{ 
  m_param.setFloorRoom(val); 

  emit floorRoomChanged(m_param.floorRoom());

  reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowBackgroundImage(bool val) 
{ 
  m_param.setShowBackgroundImage(val); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowLocations(bool val) 
{ 
  m_param.setShowLocations(val); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowLines(bool val) 
{ 
  m_param.setShowLines(val); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowGridLines(bool val) 
{ 
  m_param.setShowGridLines(val); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::setShowGridTicks(bool val) 
{ 
  m_param.setShowGridTicks(val); 

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::dumpInfo(QTextStream& out)
{
  out << "[" << preferenceName() << "]" << endl;
  out << "Caption: " << caption() << endl;
  out << "AnimateSpawnMovement: " << m_animate << endl;
  out << "VelocityLines: " << m_showVelocityLines << endl;
  out << "SpawnDepthFilter: " << m_showVelocityLines << endl;
  out << "FrameRate: " << m_frameRate << endl;
#ifdef DEBUG
  out << "ShowDebugInfo: " << m_showDebugInfo << endl;
#endif
  out << "ShowPlayer: " << m_showPlayer << endl;
  out << "ShowPlayerBackground: " << m_showPlayerBackground << endl;
  out << "ShowPlayerView: " << m_showPlayerView << endl;
  out << "ShowDroppedItems: " << m_showDrops << endl;
  out << "ShowDroppedCoins: " << m_showCoins << endl;
  out << "ShowDoors: " << m_showDoors << endl;
  out << "ShowSpawns: " << m_showSpawns << endl;
  out << "ShowSpawnNames: " << m_showSpawnNames << endl;
  out << "HighlightConsideredSpawns: " << m_highlightConsideredSpawns << endl;
  out << "ShowTooltips: " << m_showTooltips << endl;
  out << "WalkPathShowSelect: " << m_walkpathshowselect << endl;
  out << "DrawSize: " << m_drawSize << endl;

  out << endl;
  out << "[" << preferenceName() << " Parameters]" << endl;
  out << "MapLineStyle: " << m_param.mapLineStyle() << endl;
  out << "ShowMapPoints: " << m_param.showLocations() << endl;
  out << "ShowMapLines: " << m_param.showLines() << endl;
  out << "ShowGridLines: " << m_param.showGridLines() << endl;
  out << "ShowGridTicks: " << m_param.showGridTicks() << endl;
  out << "ShowBackgroundImage: " << m_param.showBackgroundImage() << endl;
  out << "Font: " << fontToString(m_param.font()) << endl;
  out << "GridResolution: " << m_param.gridResolution() << endl; 
  out << "GridTickColor: " << m_param.gridTickColor().name() << endl;
  out << "GridLineColor: " << m_param.gridLineColor().name() << endl;
  out << "BackgroundColor: " << m_param.backgroundColor().name() << endl;
  out << "HeadRoom: " << m_param.headRoom() << endl;
  out << "FloorRoom: " << m_param.floorRoom() << endl;
  out << "OptimizeMethod: " << (int)m_param.mapOptimizationMethod() << endl;

  out << endl;
  out << "[" << preferenceName() << " State]" << endl;
  out << "screenLength: width(" << m_param.screenLength().width()
      << ") height(" << m_param.screenLength().height() << ")" << endl;
  out << "screenBounds: top(" << m_param.screenBounds().top() 
      << ") bottom(" << m_param.screenBounds().bottom() 
      << ") left(" << m_param.screenBounds().left()
      << ") right(" << m_param.screenBounds().right() << ") " << endl;
  out << "screenCenter: x(" << m_param.screenCenter().x()
      << ") y(" << m_param.screenCenter().y() << ")" << endl;
  out << "zoomMapLength: width(" << m_param.zoomMapLength().width() 
      << ") height(" << m_param.zoomMapLength().height() << ")" << endl;
  out << "panOffsetX: " << m_param.panOffsetX() << endl;
  out << "panOffsetY: " << m_param.panOffsetY() << endl;
  out << "zoom: " << m_param.zoom() << endl;
  out << "ratio: " << m_param.ratio() << endl; 
  out << "ratioIFixPt: " << m_param.ratioIFixPt() 
      << " (q = " << MapParameters::qFormat << ")" << endl;
  out << "player: x(" << m_param.player().x() 
      << ") y(" << m_param.player().y() 
      << ") z(" << m_param.player().z() << ")" << endl;
  out << "playerOffset: x(" << m_param.playerOffset().x() 
      << ") y(" << m_param.playerOffset().y() << ")" << endl;
  out << "playerHeadRoom: " << m_param.playerHeadRoom() << endl;
  out << "playerFloorRoom: " << m_param.playerFloorRoom() << endl;
  out << "FollowMode: " << m_followMode << endl;
  out << "MapPanning: " << m_mapPanning << endl;
  out << "DeityPvP: " << m_deityPvP << endl;
  out << "RacePvP: " << m_racePvP << endl;
  out << "CacheAlwaysRepaint: " << m_mapCache.alwaysRepaint() << endl;
  out << endl;

#ifdef DEBUG
  out << "[" << preferenceName() << " Statistics]" << endl;
  if (m_showDebugInfo)
  {
    long totalTime = 0;
    float fps = 0.0;
    for (int i = 0; i < maxFrameTimes; i++)
      totalTime += m_frameTimes[i];
  
  fps = float(maxFrameTimes) / (totalTime / 1000.0);

  out << "Actual FPS: " << fps << endl;
  }
  out << "Paint Count: " << m_paintCount << endl;
  out << "Cache Paint Count: " << m_mapCache.paintCount() << endl;
  out << "Average Cache Paints per Map Paint: " <<
    double(m_mapCache.paintCount()) / double(m_paintCount) << endl;
  out << "Average Paint Time: " 
      << double(m_paintTimeSum) / double(m_paintCount) 
      << " milliseconds " << endl;
  out << endl;
#endif // DEBUG
}

void Map::resizeEvent (QResizeEvent *qs)
{
#ifdef DEBUGMAP
   debug ("resizeEvent()");
#endif /* DEBUGMAP */
   m_param.setScreenSize(qs->size());

   m_offscreen.resize(m_param.screenLength());

   reAdjust();
}

void Map::refreshMap (void)
{
#ifdef DEBUGMAP
   debug ("refreshMap()");
#endif /* DEBUGMAP */
   repaint (mapRect (), FALSE);
}

void Map::reAdjust ()
{
  switch (m_followMode)
  {
  case tFollowSpawn:
    // only follow spawns that exists and are spawns, all others are nonsense
    if ((m_selectedItem != NULL) && (m_selectedItem->type() == tSpawn))
    {
      // following spawn, get it's approximate location
      EQPoint location;
      ((const Spawn*)m_selectedItem)->approximatePosition(m_animate,
								    QTime::currentTime(),
								    location);

      // adjust around it's location
      m_param.reAdjust(&location);
      break;
    }

    // no more target, change target mode back to follow player
    m_followMode = tFollowPlayer;

    // fall thru to next case since it's the new mode
  case tFollowPlayer:
  { 
    // retrieve the approximate position of the player
    const Spawn* player = m_spawnShell->playerSpawn();
    
    // if there's a player, follow it
    if (player)
    {
      // retrieve the approximate current player position
      MapPoint targetPoint;
      player->approximatePosition(m_animate, QTime::currentTime(), 
				  targetPoint);

      // adjust around players location
      m_param.reAdjust(&targetPoint);
    }
    else 
      m_param.reAdjust(NULL);
  }
  break;
  case tFollowNone:
    m_param.reAdjust(NULL);
    break;
  }
}

void Map::addLocation(void)
{
#ifdef DEBUGMAP
  debug ("addLocation()");
#endif /* DEBUGMAP */

  // get the player spawn
  const Spawn* spawn = m_spawnShell->playerSpawn();

  if (!spawn)
    return;

  // get it's approximage location
  MapPoint point;
  spawn->approximatePosition(m_animate, QTime::currentTime(), point);

#ifdef DEBUGMAP
  printf("addLocation() point(%d, %d, %d)\n", point.x(), point.y(), point.z());
#endif

  // add the location
  m_mapMgr->addLocation(this, point);
}

void Map::startLine (void)
{
#ifdef DEBUGMAP
  debug ("startLine()");
#endif /* DEBUGMAP */
  // get the player spawn
  const Spawn* spawn = m_spawnShell->playerSpawn();

  if (!spawn)
    return;

  // get it's approximate position
  MapPoint point;
  spawn->approximatePosition(m_animate, QTime::currentTime(), point);

#ifdef DEBUGMAP
  printf("startLine() point(%d, %d, %d)\n", point.x(), point.y(), point.z());
#endif

  // start the line using the player spawns position
  m_mapMgr->startLine(point);
}

void Map::addLinePoint() 
{
#ifdef DEBUGMAP
  debug ("addLinePoint()");
#endif /* DEBUGMAP */

  // get the player spawn
  const Spawn* spawn = m_spawnShell->playerSpawn();

  if (!spawn)
    return;

  // get the player spawns approximate position
  MapPoint point;
  spawn->approximatePosition(m_animate, QTime::currentTime(), point);


#ifdef DEBUGMAP
  printf("addLinePoint() point(%d, %d, %d)\n", point.x(), point.y(), point.z());
#endif

  // add it as the next line point
  m_mapMgr->addLinePoint(point);
}


void Map::delLinePoint(void)
{
#ifdef DEBUGMAP
  debug ("delLinePoint()");
#endif /* DEBUGMAP */

  m_mapMgr->delLinePoint();
} // END delLinePoint


void Map::showLineDlg(void)
{
  // show the line dialog
  m_mapMgr->showLineDlg(this);
}

void Map::addPathPoint() 
{
  FILE *f;

  // get the player spawn
  const Spawn* spawn = m_spawnShell->playerSpawn();
  
  // get the player spawns approximate position
  MapPoint point;
  spawn->approximatePosition(m_animate, QTime::currentTime(), point);

  f=fopen("/tmp/coords","at");
  if(f) {
    fprintf (f,"%f, %f\n",
             (double)point.x(),
             (double)point.y());
    fclose(f);
  }
}

QRect Map::mapRect () const
{
#ifdef DEBUGMAP
   debug ("mapRect()");
   static int rendercount = 0;
   rendercount++;
   printf("%i, (0,0,%i,%i)\n",rendercount, width (), height ());
#endif /* DEBUGMAP */
   QRect r (0, 0, width (), height ());
   r.moveBottomLeft (rect ().bottomLeft ());
#ifdef DEBUGMAP
   printf("hmm2\n");
   rendercount--;
#endif /* DEBUGMAP */
   return r;
}

//----------------------------------------------------------------------
void Map::paintMap (QPainter * p)
{
#ifdef DEBUGMAP
  debug ("paintMap()");
#endif /* DEBUGMAP */
  QPainter tmp;
  
  QTime drawTime;

  // get the current time
  drawTime.start();

  // retrieve the approximate position of the player
  const Spawn* player = m_spawnShell->playerSpawn();

  EQPoint playerPos;

  // if the player is known, get it's position
  if (player != NULL)
  {
    // retrieve the approximate current player position, and set the 
    // parameters player position to it.
    player->approximatePosition(m_animate, drawTime, playerPos);
    m_param.setPlayer(playerPos);

    // make sure the player stays visible
    if ((m_param.zoom() > 1) &&
	((m_followMode == tFollowPlayer) &&
	 (!inRect(m_param.screenBounds(), 
		  playerPos.xPos(), playerPos.yPos()))))
	  reAdjust();
  }

  // if following a spawn, and there is a spawn, make sure it's visible.
  if ((m_followMode == tFollowSpawn) &&
      (m_param.zoom() > 1) &&
      (m_selectedItem != NULL) && 
      (m_selectedItem->type() == tSpawn))
  {
    EQPoint location;
    
    ((const Spawn*)m_selectedItem)->approximatePosition(m_animate, 
							drawTime, 
							location);
    
    if (!inRect(m_param.screenBounds(), playerPos.xPos(), playerPos.yPos()))
      reAdjust();
  }

  // retrieve the screen bounds
  const QRect& screenBounds = m_param.screenBounds();

  //Now, if we're animating, allow player to walk off. Grr, centering issue.

  /* Begin painting */
  tmp.begin (&m_offscreen);
  tmp.setPen (NoPen);
  tmp.setFont (QFont("Helvetica", 8, QFont::Normal));

  // draw the background
  tmp.drawPixmap(0, 0, m_mapCache.getMapImage(m_param));

  if ((player != NULL) && 
      (inRect(screenBounds, playerPos.xPos(), playerPos.yPos())))
  {
    if (m_showPlayerBackground)
      paintPlayerBackground(m_param, tmp);
    
    if (m_showPlayerView)
      paintPlayerView(m_param, tmp);
    
    if (m_showPlayer)
      paintPlayer(m_param, tmp);
  }

  if (m_showDrops)
    paintDrops(m_param, tmp);

  if (m_showCoins)
    paintCoins(m_param, tmp);

  if (m_showDoors)
    paintDoors(m_param, tmp);

  if (m_showSpawns)
    paintSpawns(m_param, tmp, drawTime);

  paintSelectedSpawnSpecials(m_param, tmp, drawTime);

#ifdef DEBUG
  // increment paint count
  m_paintCount++;

  // get paint time
  int paintTime = drawTime.elapsed();
  
  // add paint time to sum
  m_paintTimeSum += paintTime;

  //--------------------------------------------------
   if (m_showDebugInfo)
   {
     long totalTime = 0;
     float fps = 0.0;

     m_frameTimes[m_frameTimeIndex] = m_time.elapsed();
     m_frameTimeIndex = (m_frameTimeIndex + 1) % maxFrameTimes;

     for (int i = 0; i < maxFrameTimes; i++)
       totalTime += m_frameTimes[i];

     fps = float(maxFrameTimes) / (totalTime / 1000.0);

     // paint the debug info
     paintDebugInfo(m_param, tmp, fps, paintTime);

     // reset the time
     m_time.restart();
   }
#endif

   //--------------------------------------------------
   // finished painting
   tmp.end ();
   
   // draw to the widget
   p->drawPixmap (0, 0, m_offscreen);
}

void Map::paintPlayerBackground(MapParameters& param, QPainter& p)
{
  /* Paint player position background */
  p.setPen (QColor (80, 80, 80));

  QBrush tmpBrush;
  tmpBrush.setColor(QColor (80, 80, 80));
  tmpBrush.setStyle(QBrush::Dense7Pattern);
  p.setBrush(tmpBrush);
  // FOV Distance (40)
  p.drawEllipse (m_param.playerXOffset() - 40, 
		 m_param.playerYOffset() - 40, 80, 80);
}

void Map::paintPlayerView(MapParameters& param, QPainter& p)
{
  /* Paint the player direction */
#ifdef DEBUGMAP
  printf("Paint the player direction\n");
#endif
  
  p.setBrush (QColor (80, 80, 80));

  int const player_circle_radius = 4;
  
  int16_t playerAngle = m_player->getPlayerHeading();

  if (playerAngle != -1)
  {
    double const pi = 3.14159265358979323846;
    double const radians_per_circle = pi * 2;
    double const angle = (360 - playerAngle - 180) / 360.0 * radians_per_circle;
    int const compass_length = 39 + m_param.zoom();  
    int start_offset_x = int(sin( angle - radians_per_circle * 0.25 ) * player_circle_radius);
    int start_offset_y = int(cos( angle - radians_per_circle * 0.25 ) * player_circle_radius);
    double const fov_angle = radians_per_circle * 0.25;
    double fox_angle_offset = fov_angle / 2;
    
    p.setPen(yellow); // color
    p.drawLine( m_param.playerXOffset(), m_param.playerYOffset(),
		  m_param.playerXOffset() + int (sin( angle ) * compass_length),
		  m_param.playerYOffset() + int (cos( angle ) * compass_length) );
    
    p.setPen(red); // color
    for ( int n = 2; n--; )
    {
      int const start_x = m_param.playerXOffset() + start_offset_x;
      int const start_y = m_param.playerYOffset() + start_offset_y;
      
      p.drawLine( start_x, start_y,
		    start_x + int (sin( angle - fox_angle_offset ) * compass_length),
		    start_y + int (cos( angle - fox_angle_offset ) * compass_length) );
      start_offset_x *= -1;
      start_offset_y *= -1;
      fox_angle_offset *= -1;
    }
  }
}

void Map::paintPlayer(MapParameters& param, QPainter& p)
{
#ifdef DEBUGMAP
  printf("Paint player position\n");
#endif
  p.setPen(gray);
  p.setBrush(white);
  p.drawEllipse(m_param.playerXOffset() - 3, 
		m_param.playerYOffset() - 3, 6, 6);
}

void Map::paintDrops(MapParameters& param,
		     QPainter& p)
{
#ifdef DEBUGMAP
  printf("Paint the dropped items\n");
#endif
  const ItemMap& itemMap = m_spawnShell->drops();
  ItemConstIterator it;
  const Item* item;
  const QRect& screenBounds = m_param.screenBounds();
  int ixlOffset;
  int iylOffset;

  // all drops are the same color
  p.setPen(yellow);

  /* Paint the dropped items */
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    item = it->second;

    // make sure coin is within bounds
    if (!inRect(screenBounds, item->xPos(), item->yPos()) ||
	(m_spawnDepthFilter &&
	 ((item->zPos() > m_param.playerHeadRoom()) ||
	  (item->zPos() < m_param.playerFloorRoom()))))
      continue;

    ixlOffset = param.calcXOffsetI(item->xPos());
    iylOffset = param.calcYOffsetI(item->yPos());

    //fixed size:
    p.drawLine(ixlOffset - m_drawSize,
		 iylOffset - m_drawSize,
		 ixlOffset + m_drawSize,
		 iylOffset + m_drawSize);
    p.drawLine(ixlOffset - m_drawSize,
		 iylOffset + m_drawSize,
		 ixlOffset + m_drawSize,
		 iylOffset - m_drawSize);
  }
}

void Map::paintCoins(MapParameters& param,
		     QPainter& p)
{
#ifdef DEBUGMAP
  printf("Paint the coin items\n");
#endif
  const ItemMap& itemMap = m_spawnShell->coins();
  ItemConstIterator it;
  const Item* item;
  const QRect& screenBounds = m_param.screenBounds();
  int ixlOffset;
  int iylOffset;

  // coins only come in one color
  p.setPen(magenta);

  /* Paint the coin items */
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    item = it->second;

    // make sure coin is within bounds
    if (!inRect(screenBounds, item->xPos(), item->yPos()) ||
	(m_spawnDepthFilter &&
	 ((item->zPos() > m_param.playerHeadRoom()) ||
	  (item->zPos() < m_param.playerFloorRoom()))))
      continue;

    ixlOffset = param.calcXOffsetI(item->xPos());
    iylOffset = param.calcYOffsetI(item->yPos());

    //fixed size:
    p.drawLine(ixlOffset - m_drawSize,
		 iylOffset - m_drawSize,
		 ixlOffset + m_drawSize,
		 iylOffset + m_drawSize);
    p.drawLine(ixlOffset - m_drawSize, 
		 iylOffset + m_drawSize,
		 ixlOffset + m_drawSize,
		 iylOffset - m_drawSize);
  }
}		     

void Map::paintDoors(MapParameters& param,
		     QPainter& p)
{
#ifdef DEBUGMAP
  printf("Paint the door items\n");
#endif
  const ItemMap& itemMap = m_spawnShell->doors();
  ItemConstIterator it;
  const Item* item;
  const QRect& screenBounds = m_param.screenBounds();
  int ixlOffset;
  int iylOffset;

  // coins only come in one color
  p.setPen(QColor (110, 60, 0));

  /* Paint the coin items */
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    item = it->second;

    // make sure doors are within bounds
    if (!inRect(screenBounds, item->xPos(), item->yPos()) ||
	(m_spawnDepthFilter &&
	 ((item->zPos() > m_param.playerHeadRoom()) ||
	  (item->zPos() < m_param.playerFloorRoom()))))
      continue;

    ixlOffset = param.calcXOffsetI(item->xPos());
    iylOffset = param.calcYOffsetI(item->yPos());

    p.drawRect(ixlOffset,iylOffset, m_drawSize, m_drawSize);
  }
}		     

void Map::paintSpawns(MapParameters& param,
		      QPainter& p,
		      const QTime& drawTime)
{
#ifdef DEBUGMAP
  printf("Paint the spawns\n");
#endif
  const ItemMap& itemMap = m_spawnShell->spawns();
  ItemConstIterator it;
  const Item* item;
  QPointArray  atri(3);
  uint32_t distance = UINT32_MAX;
  QString spawnNameText;
  QFontMetrics fm(param.font());
  EQPoint spawnOffset;
  EQPoint location;
  QPen tmpPen;
  uint8_t playerLevel = m_player->getPlayerLevel();
  uint32_t scaledFOVDistance = uint32_t(40 * m_param.ratio());
  int spawnOffsetXPos, spawnOffsetYPos;
  const QRect& screenBounds = m_param.screenBounds();
  bool up2date = false;

  // retrieve the approximate position of the player
  const Spawn* player = m_spawnShell->playerSpawn();

  /* Paint the spawns */
  if ((m_lastFlash.msecsTo(drawTime)) > 100)  // miliseconds between flashing
  {
    // invert flash value
    m_flash = !m_flash;

    // reset last flash time, only real diff between start() and restart() is 
    // that restart returns the elapsed time since start() was called, 
    // which we don't need
    m_lastFlash.start(); 
  }

  const Spawn* spawn;
  // iterate over all spawns in of the current type
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    item = it->second;

#ifdef DEBUGMAP
    spawn = spawnType(item);
    
    if (spawn == NULL)
    {
      fprintf(stderr, 
	      "Got non Spawn from iterator over type tSpawn (Tyep:%d ID: %d)!\n",
	      item->type(), item->id());
      continue;
    }
#else
    // since only things of type Spawn should be in the list, 
    // just do a quicky conversion
    spawn = (const Spawn*)item;
#endif

    // only paint if the spawn is not filtered or the m_showFiltered flag is on
    if ((!m_spawnDepthFilter || 
	 (item == m_selectedItem) ||
         ((item->zPos() <= m_param.playerHeadRoom()) && 
	  (item->zPos() >= m_param.playerFloorRoom()))) &&
	((!(item->filterFlags() & FILTER_FLAG_FILTERED)) || m_showFiltered))
	 
    {
      // get the approximate position of the spawn
      up2date = spawn->approximatePosition(m_animate, drawTime, location);

      // check that the spawn is within the screen bounds
      if (!inRect(screenBounds, location.xPos(), location.yPos()))
	  continue; // not in bounds, next...

      // calculate the spawn's offset location
      spawnOffsetXPos = m_param.calcXOffsetI(location.xPos());
      spawnOffsetYPos = m_param.calcYOffsetI(location.yPos());

      //--------------------------------------------------
#ifdef DEBUGMAP
      printf("Draw Spawn Names\n");
#endif
      // Draw Spawn Names if selected and distance is less than the FOV 
      // distance
      if (m_showSpawnNames)
      {
	if (location.calcDist2DInt(param.player()) < scaledFOVDistance)
	{
	  spawnNameText.sprintf("%2d: %s",
				spawn->level(),
				(const char*)spawn->name());
	  int width = fm.width(spawnNameText);
	  p.setPen(darkGray);
	  p.drawText(spawnOffsetXPos - (width / 2),
		     spawnOffsetYPos + 10, spawnNameText);
	}
      }

      //--------------------------------------------------
#ifdef DEBUGMAP
      printf("Draw velocities\n");
#endif
      /* Draw velocities */
      if ((m_showVelocityLines && 
	   !spawn->isSelf()) && //Don't draw line for the self.
	  (spawn->deltaX() || spawn->deltaY())) // only if has a delta
      {
	p.setPen (darkGray);
	p.drawLine (spawnOffsetXPos,
		      spawnOffsetYPos,
		      spawnOffsetXPos - spawn->deltaX(),
		      spawnOffsetYPos - spawn->deltaY());
      }

      //
      // Misc decorations
      //

      //--------------------------------------------------
#ifdef DEBUGMAP
      printf("Draw corpse, team, and filter boxes\n");
#endif
      // handle regular NPC's first, since they are generally the most common
      if (spawn->isNPC())
      {
	if (!(spawn->filterFlags() & FILTER_FLAG_FILTERED))
	{
	  // set pen to black
	  p.setPen(black);
	
	  // set brush to spawn con color
	  p.setBrush(m_player->pickConColor(spawn->level()));
	}
	else
	{
	  // use gray brush
	  p.setBrush(gray);
	  
	  // and use spawn con color
	  p.setPen(m_player->pickConColor(spawn->level())); 
	}

	if(m_flash && (spawn->runtimeFilterFlags() & m_runtimeFilterFlagMask))
	  p.setPen(white);

	// draw the regular spawn dot
	p.drawEllipse (spawnOffsetXPos - m_drawSize, 
		       spawnOffsetYPos - m_drawSize, 
		       2 * m_drawSize, 2 * m_drawSize);
	  
	// retrieve the spawns aggro range
	uint16_t range = m_mapMgr->spawnAggroRange(spawn);

	// if aggro range is known (non-zero), draw the aggro range circle
	if (range != 0)
	{
	  int xrange = abs(m_param.calcXOffsetI (location.xPos() + range) - 
			   m_param.calcXOffsetI (location.xPos() - range));
	  int yrange = abs(m_param.calcYOffsetI (location.yPos() + range) - 
			   m_param.calcYOffsetI (location.yPos() - range));

	  p.setBrush(NoBrush);
	  p.setPen(red); 

	  p.drawEllipse(spawnOffsetXPos - xrange, 
			spawnOffsetYPos - yrange, 
			2 * xrange, 
			2 * yrange);
	}
      }
      else if (spawn->isOtherPlayer())
      {
	if (!up2date)
	{
	  if(m_flash && (spawn->runtimeFilterFlags() & m_runtimeFilterFlagMask))
	    p.setPen(white);
	  else if ((spawn->filterFlags() & FILTER_FLAG_FILTERED))
	    p.setPen(gray);
	  else
	    p.setPen(yellow);

	  p.drawLine(spawnOffsetXPos, 
		     spawnOffsetYPos - m_drawSize,
		     spawnOffsetXPos,
		     spawnOffsetYPos + m_drawSize);
	  p.drawLine(spawnOffsetXPos - m_drawSize, spawnOffsetYPos,
		     spawnOffsetXPos + m_drawSize, spawnOffsetYPos);
	  // don't do anything else for out of data PC data.
	  continue;
	}
	if (!(spawn->filterFlags() & FILTER_FLAG_FILTERED))
	{
	  // set pen to magenta
	  p.setPen(magenta);
	
	  // set brush to spawn con color
	  p.setBrush(m_player->pickConColor(spawn->level()));
	}
	else
	{
	  // use gray brush
	  p.setBrush(gray);
	  
	  // and use spawn con color
	  p.setPen(m_player->pickConColor(spawn->level())); 
	}

	if(m_flash && (spawn->runtimeFilterFlags() & m_runtimeFilterFlagMask))
	  p.setPen(white);

	//Fixed size:
	if (m_deityPvP)
	{
	  int dteam = spawn->deityTeam();
	  
	  switch(dteam)
	  {
	  case DTEAM_GOOD:
	    { // Up Triangle
	      atri.setPoint(0, spawnOffsetXPos, spawnOffsetYPos - 2 * m_drawSize);
	      atri.setPoint(1, spawnOffsetXPos + m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      p.drawPolygon(atri);
	      break;
	    }
	  case DTEAM_NEUTRAL:
	    { // Right Triangle
	      atri.setPoint(0, spawnOffsetXPos + 2 * m_drawSize, spawnOffsetYPos);
	      atri.setPoint(1, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      p.drawPolygon ( atri);
	      break;
	    }
	  case DTEAM_EVIL:
	    { // Down Triangle
	      atri.setPoint(0, spawnOffsetXPos, 
			    spawnOffsetYPos + 2 * m_drawSize);
	      atri.setPoint(1, spawnOffsetXPos + m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      p.drawPolygon ( atri);
	      break;
	    }
	  default:
	    p.drawRect(spawnOffsetXPos - m_drawSize, 
		       spawnOffsetYPos - m_drawSize, 
		       2 * m_drawSize, 2 * m_drawSize);
	    break;
	  }
	}
	else if (m_racePvP)
	{
	  int rteam = spawn->raceTeam();
	  
	  switch(rteam)
	  {
	  case RTEAM_HUMAN:
	    { // Up Triangle
	      atri.setPoint(0, spawnOffsetXPos, spawnOffsetYPos - 2 * m_drawSize);
	      atri.setPoint(1, spawnOffsetXPos + m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      p.drawPolygon(atri);
	      break;
	    }
	  case RTEAM_ELF:
	    { // Right Triangle
	      atri.setPoint(0, spawnOffsetXPos + 2 * m_drawSize, spawnOffsetYPos);
	      atri.setPoint(1, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      p.drawPolygon ( atri);
	      break;
	    }
	  case RTEAM_DARK:
	    { // Down Triangle
	      atri.setPoint(0, spawnOffsetXPos, 
			    spawnOffsetYPos + 2 * m_drawSize);
	      atri.setPoint(1, spawnOffsetXPos + m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos - m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      p.drawPolygon ( atri);
	      break;
	    }
	  case RTEAM_SHORT:
	    { // Left Triangle
	      atri.setPoint(0, spawnOffsetXPos - 2 * m_drawSize, 
			    spawnOffsetYPos);
	      atri.setPoint(1, spawnOffsetXPos + m_drawSize, 
			    spawnOffsetYPos + m_drawSize);
	      atri.setPoint(2, spawnOffsetXPos + m_drawSize, 
			    spawnOffsetYPos - m_drawSize);
	      p.drawPolygon ( atri);
	      break;
	    }
	  default:
	    p.drawRect(spawnOffsetXPos - m_drawSize, 
		       spawnOffsetYPos - m_drawSize, 
		       2 * m_drawSize, 2 * m_drawSize);
	    break;
	  }
	}
	else
	  p.drawRect(spawnOffsetXPos - m_drawSize, 
		     spawnOffsetYPos - m_drawSize, 
		     2 * m_drawSize, 2 * m_drawSize);
      }
      else if (spawn->NPC() == SPAWN_NPC_CORPSE) // x for NPC corpse
      {
	if(m_flash && (spawn->runtimeFilterFlags() & m_runtimeFilterFlagMask))
	  p.setPen(white);
	else if ((spawn->filterFlags() & FILTER_FLAG_FILTERED))
	  p.setPen(gray);
	else
	  p.setPen(cyan);

	//fixed size
	p.drawLine(spawnOffsetXPos, 
		   spawnOffsetYPos - m_drawSize,
		   spawnOffsetXPos,
		   spawnOffsetYPos + m_drawSize);
	p.drawLine(spawnOffsetXPos - m_drawSize, spawnOffsetYPos,
		   spawnOffsetXPos + m_drawSize, spawnOffsetYPos);

	// nothing more to be done to the dead, next...
	continue;
      }
      else if (spawn->NPC() == SPAWN_PC_CORPSE) // x for PC corpse
      {
	if(m_flash && (spawn->runtimeFilterFlags() & m_runtimeFilterFlagMask))
	  tmpPen = white;
	else if ((spawn->filterFlags() & FILTER_FLAG_FILTERED))
	  tmpPen = gray;
	else
	  tmpPen = yellow;

	tmpPen.setWidth(2);
	p.setPen(tmpPen);
	p.setBrush(NoBrush);

	p.drawRect(spawnOffsetXPos - m_drawSize, 
		   spawnOffsetYPos - m_drawSize, 
		   2 * m_drawSize, 2 * m_drawSize);

	// nothing more to be done to the dead, next...
	continue;
      }
      else if (spawn->isUnknown())
      {
	// set pen to black
	p.setPen(black);
	
	// set brush to gray
	p.setBrush(gray);

	// draw the regular spawn dot
	p.drawEllipse (spawnOffsetXPos - m_drawSize, 
		       spawnOffsetYPos - m_drawSize, 
		       2 * m_drawSize, 2 * m_drawSize);

	// nothing more to be done to the unknown, next...
	continue;
      }

      // only bother checking for specific flags if any are set...
      if (spawn->filterFlags() != 0)
      {
	if (spawn->filterFlags() & FILTER_FLAG_DANGER)
	{
	  distance = location.calcDist2DInt(param.player());
	  p.setPen(red);
	  p.setBrush(NoBrush);
	  if(m_flash)
	    p.drawEllipse (spawnOffsetXPos - 4, spawnOffsetYPos - 4, 8, 8);
	  if(distance < 500)
	  {
	    if(m_flash)
	    {
	      p.setPen(red);
	      p.drawLine (m_param.playerXOffset(), 
			  m_param.playerYOffset(),
			  spawnOffsetXPos, spawnOffsetYPos);
	    }
	  }
	  else if(distance < 1000)
	  {
	    p.setPen(yellow);
	    p.drawLine (m_param.playerXOffset(), 
			m_param.playerYOffset(),
			spawnOffsetXPos, spawnOffsetYPos);
	  }
	}
	else if (spawn->filterFlags() & FILTER_FLAG_CAUTION)
	{
	  distance = location.calcDist2DInt(param.player());
	  p.setPen(yellow);
	  p.setBrush(NoBrush);
	  if(m_flash)
	    p.drawEllipse (spawnOffsetXPos - 4, spawnOffsetYPos - 4, 8, 8);
	  if(distance < 500)
	  {
	    p.drawLine (m_param.playerXOffset(), 
			m_param.playerYOffset(),
			spawnOffsetXPos, spawnOffsetYPos);
	  }
	}
	else if (spawn->filterFlags() & FILTER_FLAG_HUNT)
	{
	  p.setPen(gray);
	  p.setBrush(NoBrush);
	  if(m_flash)
	    p.drawEllipse (spawnOffsetXPos - 4, spawnOffsetYPos - 4, 8, 8);
	}
	else if (spawn->filterFlags() & FILTER_FLAG_LOCATE)
	{
	  p.setPen(white);
	  p.setBrush(NoBrush);
	  if(m_flash)
	    p.drawEllipse (spawnOffsetXPos - 4, spawnOffsetYPos - 4, 8, 8);
	  
	  p.drawLine(m_param.playerXOffset(), 
		     m_param.playerYOffset(),
		     spawnOffsetXPos, spawnOffsetYPos);
	}
	else if (spawn->filterFlags() & FILTER_FLAG_TRACER)
        {
	  p.setBrush(NoBrush);
	  p.setPen(yellow);
	  p.drawRect(spawnOffsetXPos - 4, spawnOffsetYPos - 4, 
		     8, 8);
	}
      }

      // if the spawn was considered, note it.
      if (m_highlightConsideredSpawns && 
	  spawn->considered())
      {
	p.setBrush(NoBrush);
	p.setPen(red);
	p.drawRect(spawnOffsetXPos - 4,
		   spawnOffsetYPos - 4, 8, 8);
      }

      //--------------------------------------------------
#ifdef DEBUGMAP
      printf("PvP handling\n");
#endif
      // if PvP is not enabled, don't try to do it, continue to the next spawn
      if (!m_racePvP && !m_deityPvP)
	continue;

      const Spawn* owner;

      if (spawn->petOwnerID() != 0)
	owner = spawnType(m_spawnShell->findID(tSpawn, spawn->petOwnerID()));
      else 
	owner = NULL;

      // if spawn is another pc, on a different team, and within 8 levels
      // highlight it flashing
      if (m_flash)
      {
	if (m_racePvP)
	{
	  if (spawn->isOtherPlayer())
	  {
	    // if not the same team as us
	    if (!player->isSameRaceTeam(spawn))
	    {
	      int diff = spawn->level() - playerLevel;
	      if (diff < 0) diff *= -1;
	      
	      // if we are within 8 levels of other player
	      if (diff <= 8)
	      {
		// they are in your range
		switch ( (spawn->level() - playerLevel) + 8)
		{
		  // easy
		case 0:  // you are 8 above them
		case 1:  // you are 7 above them
		  p.setPen(green); 
		  break;
		case 2:  // you are 6 above them
		case 3:  // you are 5 above them
		  p.setPen(darkGreen); 
		  break;
		  
		  // moderate
		case 4:  // you are 4 above them
		case 5:  // you are 3 above them
		  p.setPen(blue); 
		  break;
		case 6:  // you are 2 above them
		case 7:  // you are 1 above them
		  p.setPen(darkBlue); 
		  break;
		  
		  // even
		case 8:  // you are even with them
		  p.setPen(white); 
		  break;
		  
		  // difficult 
		case 9:  // you are 1 below them
		case 10:  // you are 2 below them
		  p.setPen(yellow); 
		  break;
		  
		  // downright hard
		case 11:  // you are 3 below them
		case 12:  // you are 4 below them
		  p.setPen(magenta); 
		  break;
		case 13:  // you are 5 below them
		case 14:  // you are 6 below them
		  p.setPen(red); 
		  break;
		case 15:  // you are 7 below them
		case 16:  // you are 8 below them
		  p.setPen(darkRed); 
		  break;
		}
		p.setBrush(NoBrush);
		p.drawRect (spawnOffsetXPos - m_drawSize - 1,
			    spawnOffsetYPos - m_drawSize - 1, 
			    2 * (m_drawSize + 1), 2 * (m_drawSize + 1));
		p.setBrush(SolidPattern);
	      }
	    }
	  } // if decorate pvp
	  
	  // circle around pvp pets
	  if (owner != NULL)
	  {
	    if (!player->isSameRaceTeam(owner))
	    {
	      p.setBrush(NoBrush);
	      p.setPen(m_player->pickConColor(spawn->level()));
	      p.drawEllipse (spawnOffsetXPos - 3,
			     spawnOffsetYPos - 3, 6, 6);
	      p.setBrush(SolidPattern);
	    }
	  }
	} // end racePvp
	else if (m_deityPvP)
	{
	  if (spawn->isOtherPlayer())
	  {
	    // if not the same team as us
	    if (!player->isSameDeityTeam(spawn))
	    {
	      int diff = spawn->level() - playerLevel;
	      if (diff < -5)  //They are much easier than you.
		p.setPen(green); 
	      if (diff > 5)  //They are much harder than you.
		p.setPen(darkRed); 
	      if (diff < 0) diff *= -1;
	      // if we are within 8 levels of other player
	      if (diff <= 5)
	      {
		// they are in your range
		switch ( (spawn->level() - playerLevel) + 5)
		{
		  // easy
		case 0:  // you are 5 above them
		case 1:  // you are 4 above them
		  p.setPen(green); 
		  break;
		case 2:  // you are 3 above them
		  p.setPen(darkGreen); 
		  break;
		  
		  // moderate
		case 3:  // you are 2 above them
		  p.setPen(blue); 
		  break;
		case 4:  // you are 1 above them
		  p.setPen(darkBlue); 
		  break;
		  
		  // even
		case 5:  // you are even with them
		  p.setPen(white); 
		  break;
		  
		  // difficult 
		case 6:  // you are 1 below them
		  p.setPen(yellow); 
		  break;
		  
		  // downright hard
		case 7:  // you are 2 below them
		case 8:  // you are 3 below them
		  p.setPen(magenta); 
		  break;
		case 9:  // you are 4 below them
		  p.setPen(red); 
		  break;
		case 10:  // you are 5 below them
		  p.setPen(darkRed); 
		  break;
		}
		p.setBrush(NoBrush);
		p.drawRect (spawnOffsetXPos - m_drawSize - 1,
			    spawnOffsetYPos - m_drawSize - 1, 
			    2 * (m_drawSize + 1), 2 * (m_drawSize + 1));
		p.setBrush(SolidPattern);
	      }
	    }
	  } // if decorate pvp
	  
	  // circle around deity pvp pets
	  if (owner != NULL)
	  {
	    if (!player->isSameDeityTeam(owner))
	    {
	      p.setBrush(NoBrush);
	      p.setPen(m_player->pickConColor(spawn->level()));
	      p.drawEllipse (spawnOffsetXPos - 3,
			     spawnOffsetYPos - 3, 6, 6);
	      p.setBrush(SolidPattern);
	    }
	  }
	} // end if deityPvP
      } // end if flash 
    } // end if should be painted
  } // end for spawns

  //----------------------------------------------------------------------
#ifdef DEBUGMAP
   printf("Done drawing spawns\n");
#endif
}

void Map::paintSelectedSpawnSpecials(MapParameters& param, QPainter& p,
				     const QTime& drawTime)
{
  if (m_selectedItem == NULL)
    return;

#ifdef DEBUGMAP
  printf("Draw the line of the selected spawn\n");
#endif
  EQPoint location;

  if (m_selectedItem->type() == tSpawn)
    ((const Spawn*)m_selectedItem)->approximatePosition(m_animate, 
								  drawTime, 
								  location);
  else
    location.setPoint(*m_selectedItem);

  int spawnXOffset = m_param.calcXOffsetI(location.xPos());
  int spawnYOffset = m_param.calcYOffsetI(location.yPos());

  p.setPen(magenta);
  p.drawLine(m_param.playerXOffset(),
	     m_param.playerYOffset(),
	     spawnXOffset, 
	     spawnYOffset);
  
#ifdef DEBUGMAP
  printf("Draw the path of the selected spawn\n");
#endif
  const Spawn* spawn;
  if (m_walkpathshowselect && (spawn = spawnType(m_selectedItem)))
  {
    SpawnTrackListIterator trackIt(spawn->trackList());
    
    const SpawnTrackPoint* trackPoint = trackIt.current();
    if (trackPoint)
    {
      p.setPen (blue);
      p.moveTo (m_param.calcXOffsetI(trackPoint->xPos()), 
		m_param.calcYOffsetI(trackPoint->yPos()));
      
      while ((trackPoint = ++trackIt) != NULL)
      {
	p.lineTo (m_param.calcXOffsetI (trackPoint->xPos()), 
		  m_param.calcYOffsetI (trackPoint->yPos()));
      }
      
      p.lineTo (spawnXOffset, spawnYOffset);
    }
  }
}


void Map::paintDebugInfo(MapParameters& param, 
			 QPainter& p, 
			 float fps, 
			 int drawTime)
{
  // show coords of upper left corner and lower right corner
  p.setPen( yellow );
  QString ts;
  ts.sprintf( "(%d, %d)", 
	      (int)(param.screenCenterX() * param.ratioX()), 
	      (int)(param.screenCenterY() * param.ratioY()));
  p.drawText( 10, 8, ts );
  ts.sprintf( "(%d, %d)",
	      (int)((param.screenCenterX() - param.screenLength().width()) * 
		    param.ratioX()),
	      (int)((param.screenCenterY() - param.screenLength().height()) *
		    param.ratioY()));
  p.drawText( width() - 90, height() - 14, ts );
  
  // show frame times
  ts.sprintf( "%2.0ffps/%dms", fps, drawTime);
  p.drawText( this->width() - 60, 8, ts );
}

void Map::paintEvent (QPaintEvent * e)
{
#ifdef DEBUGMAP
   debug ("paintEvent()");
#endif /* DEBUGMAP */
   QRect updateR = e->rect ();
   QPainter p;
   p.begin (this);
   if (updateR.intersects (mapRect ()))
     paintMap (&p);
   p.end ();
}


void Map::makeSelectedSpawnLine()
{
  makeSpawnLine(m_selectedItem);
}

void Map::makeSpawnLine(const Item* item)
{
  if (item == NULL)
    return;

  const Spawn* spawn = spawnType(item);

  if (spawn == NULL)
    return;

   printf("Map::makeSpawnLine(id=%d)\n", spawn->id());
   const SpawnTrackList& trackList = spawn->trackList();
   SpawnTrackListIterator trackIt(spawn->trackList());
   int cnt = trackList.count();

   if (cnt == 0)
     return;

   char dapath[256];
   sprintf(dapath,"%s/%s_mobpath.map", MAPDIR, 
	   (const char*)m_player->getShortZoneName());
   FILE *fh = fopen(dapath,"a");
   if (!fh) return;

   const SpawnTrackPoint* trackPoint;
   int curPt;
   int total = 0;
   while (total < cnt)
   {
     // start the line, up to 255 points in the line
     fprintf (fh, "M,%s,%s,%d", 
	      (const char*)spawn->realName(), "blue", 
	      (((cnt - total) > 255) ? 255 : 0));

     //iterate over the track, writing out the points (up to 255 points)
     for (trackPoint = trackIt.current(), curPt = 0;
	  trackPoint && (curPt < 255);
	  trackPoint = ++trackIt, curPt++, total++)
     {
       fprintf (fh, ",%d,%d,%d", 
		trackPoint->xPos(), trackPoint->yPos(), trackPoint->zPos());
     }

     // close the line
     fprintf (fh, "\n");
   }
   fflush(fh);
   fclose(fh);
}


void Map::mouseMoveEvent( QMouseEvent* event )
{
  // We're moving the map around, only try to move if we are in zoom mode
  // Also, since the mouse is more sensitive, pan a little slower.
  if (m_mapPanning && m_param.zoom() > 1) 
  {
    const QPoint    curpoint    = event->pos();
    
    if (curpoint.x() > m_mapPanX)
      m_param.panX(-3 * panAmmt);
    else if (curpoint.x() < m_mapPanX)
      m_param.panX(3 * panAmmt);
    
    if (curpoint.y() > m_mapPanY)
      m_param.panY(-3 * panAmmt);
    else if (curpoint.y() < m_mapPanY)
      m_param.panY(3 * panAmmt);
    
    m_mapPanX = curpoint.x();
    m_mapPanY = curpoint.y();
    
    emit panXChanged(m_param.panOffsetX());
    emit panYChanged(m_param.panOffsetY());
    
    reAdjust();

    if(!showeq_params->fast_machine)
      refreshMap();
  }
  
  emit mouseLocation(m_param.invertXOffset(event->x()),
		     m_param.invertYOffset(event->y()));
  
  if ( !m_showTooltips)
  {
    m_mapTip->hide();
    return;
  }
  
  if ( m_mapPanning && (m_param.zoom() > 1))
    return;
  
  const Item* item = closestSpawnToPoint(event->pos(), 5);
  
  if (item != NULL)
  {
    QString string;

    const Spawn* spawn = spawnType(item);

    if (spawn)
    {
      QString hp;
      
      if (spawn->HP() <= 0)
	hp = "<= 0";
      else
	hp = QString::number(spawn->HP());

      string.sprintf("%s\nLevel: %2d\tHP: %s\t Y/X/Z: %5d/%5d/%5.1f\nRace: %s\t Class: %s\nEquipment: %s", 
		     (const char*)spawn->transformedName(),
		     spawn->level(), (const char*)hp,
		     spawn->xPos(), spawn->yPos(), item->displayZPos(),
		     (const char*)spawn->raceName(), 
		     (const char*)spawn->className(),
		     (const char*)spawn->info());
    }
    else
      string.sprintf("%s\nY/X/Z: %5d/%5d/%5.1f\nRace: %s\t Class: %s", 
		     (const char*)item->transformedName(),
		     item->xPos(), item->yPos(), item->displayZPos(),
		     (const char*)item->raceName(), 
		     (const char*)item->className());

    m_mapTip->setText( string  );
    m_mapTip->adjustSize();
    QPoint popPoint = mapToGlobal(event->pos());
    m_mapTip->move(popPoint.x() + 15, popPoint.y() + 15);
    m_mapTip->show();
    m_mapTip->raise();        
  }
  else
    m_mapTip->hide();
} 

void Map::selectSpawn(const Item* item)
{
  // sanity check
  if (item == NULL)
    return;
  
  /* printf ("%s\n", item->ID()); */
  m_selectedItem = item;
  
  // if following the selected spawn, call reAdjust to focus on the new one
  if (m_followMode == tFollowSpawn)
    reAdjust();

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::delItem(const Item* item)
{
  if (item == NULL)
    return;

  // if this is the selected spawn, clear the selected spawn
  if (item == m_selectedItem)
  {
    m_selectedItem = NULL;

    // if was following the selected spawn, call reAdjust to fix things
    if (m_followMode == tFollowSpawn)
      reAdjust();
  }

  if(!showeq_params->fast_machine)
    refreshMap ();
}

void Map::clearItems()
{
  // clear the selected spawn since there are no more spawns
  m_selectedItem = NULL;

  // if was following the selected spawn, call reAdjust to fix things
  if (m_followMode == tFollowSpawn)
    reAdjust();

  // refresh the map
  refreshMap();
}


void Map::changeItem(const Item* item, uint32_t changeType)
{
  if (item == NULL)
    return;

  // only need to deal with position changes
  if (changeType & tSpawnChangedPosition)
  {
    if (m_followMode == tFollowSpawn) 
    {
      // follow mode is follow spawn, check if this is the selected spawn
      // and if so, reAdjust around it's position.
      if (item == m_selectedItem)
	reAdjust();
    }
    else if (m_followMode == tFollowPlayer)
    {
      // follow mode is follow player, check if this is the player spawn
      // and if so, reAdjust around it's position.
      if (item == (const Item*)m_spawnShell->playerSpawn())
	reAdjust();
    }
  }
}

const Item* Map::closestSpawnToPoint(const QPoint& pt, 
					       uint32_t closestDistance) const
{
  ItemConstIterator it;
  const Spawn* spawn;
  const Item* closestItem = NULL;

  uint32_t distance;
  EQPoint location;

  const ItemMap& itemMap = m_spawnShell->getConstMap(tSpawn);
  
  // iterate over all spawns in of the current type
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    spawn = (const Spawn*)it->second;
    
    spawn->approximatePosition(m_animate, QTime::currentTime(), location);
    
    EQPoint testPoint;
    testPoint.setPoint(m_param.calcXOffsetI(location.xPos()), 
		       m_param.calcYOffsetI(location.yPos()), 0);
    
    distance = testPoint.calcDist2DInt(pt);
    
    if (distance < closestDistance)
    {
      closestDistance = distance;
      closestItem = (const Item*)spawn;
    }
  }

  const Item* item;
  itemType itemTypes[3] = { tDrop, tCoins, tDoors };
  
  for (int i = 0; i < 3; i++)
  {
    const ItemMap& itemMap = m_spawnShell->getConstMap(itemTypes[i]);

    // iterate over all spawns in of the current type
    for (it = itemMap.begin();
	 it != itemMap.end(); 
	 ++it)
    {
      // get the item from the list
      item = it->second;

      EQPoint testPoint;
      testPoint.setPoint(m_param.calcXOffsetI(item->xPos()), 
			 m_param.calcYOffsetI(item->yPos()), 0);

      distance = testPoint.calcDist2DInt(pt);

      if (distance < closestDistance)
      {
	closestDistance = distance;
	closestItem = item;
      }
    }
  }

  return closestItem;
}

void Map::mapUnloaded(void)
{
#ifdef DEBUGMAP
  debug ("mapUnloaded()");
#endif /* DEBUGMAP */

  m_selectedItem = NULL;
  
  m_param.reset();

  emit zoomChanged(m_param.zoom());
  emit panXChanged(m_param.panOffsetX());
  emit panYChanged(m_param.panOffsetY());

  // stop the map update timer
  //  m_timer->stop();

  m_offscreen.resize(m_param.screenLength());
  
  // force a map refresh
  refreshMap();
  
#ifdef DEBUG
  if (m_showDebugInfo)
    m_time.restart();
#endif
}

void Map::mapLoaded(void)
{
#ifdef DEBUGMAP
  debug ("mapLoaded()");
#endif /* DEBUGMAP */

  reAdjust();
  
  if (!showeq_params->fast_machine)
    refreshMap();
  
  // start the map update timer if necessary
  if (!m_timer->isActive())
    m_timer->start(1000/m_frameRate, false);
  
#ifdef DEBUG
  if (m_showDebugInfo)
    m_time.restart();
#endif
}

void Map::mapUpdated(void)
{
  reAdjust();
  
  if(!showeq_params->fast_machine)
    refreshMap();
}

//----------------------------------------------------------------------
// MapFrame
MapFrame::MapFrame(FilterMgr* filterMgr,
		   MapMgr* mapMgr,
		   EQPlayer* player, 
		   SpawnShell* spawnshell,
		   const QString& preferenceName, 
		   const QString& defCaption,
		   const char* mapName,
		   QWidget* parent, const char* name)
  : QVBox(parent, name),
    m_mapPreferenceName(preferenceName)
{
  m_filterMgr = filterMgr;

  QString prefString = MapFrame::preferenceName();
  QString tmpPrefString;

  tmpPrefString = "Caption";
  setCaption(pSEQPrefs->getPrefString(tmpPrefString, prefString, (const char*)defCaption));

  QLabel* tmpLabel;

  // setup the top control window
  m_topControlBox = new QHBox(this);
  m_topControlBox->setSpacing(1);
  m_topControlBox->setMargin(0);
  tmpPrefString = "ShowTopControlBox";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_topControlBox->hide();

  // setup runtime filter
  m_filterMgr->registerRuntimeFilter(m_mapPreferenceName, 
				     m_runtimeFilterFlag,
				     m_runtimeFilterFlagMask);

  // Create map
  m_map = new Map(mapMgr, player, spawnshell, preferenceName, 
		  m_runtimeFilterFlagMask, this, mapName);

  // setup bottom control window
  m_bottomControlBox = new QHBox(this);
  m_bottomControlBox->setSpacing(1);
  m_bottomControlBox->setMargin(0);
  tmpPrefString = "ShowBottomControlBox";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_bottomControlBox->hide();

  
  // setup Zoom control
  m_zoomBox = new QHBox(m_topControlBox);
  tmpLabel = new QLabel(m_zoomBox);
  tmpLabel->setText("Zoom:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_zoom = new QSpinBox(1, 32, 1, m_zoomBox);
  m_zoom->setWrapping(true);
  m_zoom->setSuffix("x");
  m_zoom->setValue(m_map->zoom());
  tmpLabel->setBuddy(m_zoom);
  tmpPrefString = "ShowZoom";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_zoomBox->hide();
  connect(m_zoom, SIGNAL(valueChanged(int)),
	  m_map, SLOT(setZoom(int)));
  connect(m_map, SIGNAL(zoomChanged(int)),
	  m_zoom, SLOT(setValue(int)));

  // setup Player Location display
  m_playerLocationBox = new QHBox(m_topControlBox);
  tmpLabel = new QLabel(m_playerLocationBox);
  tmpLabel->setText("You:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_playerLocation = new QLabel(m_playerLocationBox);
  m_playerLocation->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  m_playerLocation->setText("0      0      0      ");
  m_playerLocation->setMinimumWidth(90);
  m_playerLocation->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  tmpLabel->setBuddy(m_playerLocation);
  tmpPrefString = "ShowPlayerLocation";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_playerLocationBox->hide();
  connect (player, SIGNAL(posChanged(int16_t,int16_t,int16_t,
					int16_t,int16_t,int16_t,int32_t)), 
	   this, SLOT(setPlayer(int16_t,int16_t,int16_t,
				int16_t,int16_t,int16_t,int32_t)));

  // setup Mouse Location display
  m_mouseLocationBox = new QHBox(m_topControlBox);
  tmpLabel = new QLabel(m_mouseLocationBox);
  tmpLabel->setText("Cursor:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_mouseLocation = new QLabel(m_mouseLocationBox);
  m_mouseLocation->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  m_mouseLocation->setText("0      0      ");
  m_mouseLocation->setMinimumWidth(70);
  m_mouseLocation->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  tmpLabel->setBuddy(m_mouseLocationBox);
  tmpPrefString = "ShowMouseLocation";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_mouseLocationBox->hide();
  connect (m_map, SIGNAL(mouseLocation(int16_t, int16_t)), 
	   this, SLOT(mouseLocation(int16_t, int16_t)));

  // setup Filter
  m_filterBox = new QHBox(m_topControlBox);
  tmpLabel = new QLabel(m_filterBox);
  tmpLabel->setText("Find:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_filter = new MapFilterLineEdit(m_filterBox);
  //  m_filter->setAlignment(Qt::AlignCenter);
  tmpLabel->setBuddy(m_filter);
  tmpPrefString = "ShowFilter";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_filterBox->hide();
#ifdef MAPFRAME_IMMEDIATE_REGEX
  connect (m_filter, SIGNAL(textChanged (const QString &)), 
	   this, SLOT(setregexp(const QString &)));
#else
  connect (m_filter, SIGNAL(returnPressed()),
	   this, SLOT(filterConfirmed()));
#endif

  // setup Frame Rate control
  m_frameRateBox = new QHBox(m_bottomControlBox);
  tmpLabel = new QLabel(m_frameRateBox);
  tmpLabel->setText("Frame Rate:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_frameRate = new QSpinBox(1, 60, 1, m_frameRateBox);
  m_frameRate->setWrapping(true);
  m_frameRate->setSuffix(" fps");
  m_frameRate->setValue(m_map->frameRate());
  tmpLabel->setBuddy(m_frameRate);
  tmpPrefString = "ShowFrameRate";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_frameRateBox->hide();
  m_frameRate->setValue(m_map->frameRate());
  connect(m_frameRate, SIGNAL(valueChanged(int)),
	  m_map, SLOT(setFrameRate(int)));
  connect(m_map, SIGNAL(frameRateChanged(int)),
	  m_frameRate, SLOT(setValue(int)));

  // setup Pan Controls
  m_panBox = new QHBox(m_bottomControlBox);
  tmpLabel = new QLabel(m_panBox);
  tmpLabel->setText("Pan X:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_panX = new QSpinBox(-8192, 8192, 16, m_panBox);
  m_panX->setValue(m_map->panOffsetX());
  tmpLabel = new QLabel(m_panBox);
  tmpLabel->setText("Y:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_panY = new QSpinBox(-8192, 8192, 16, m_panBox);
  m_panY->setValue(m_map->panOffsetY());
  tmpPrefString = "ShowPanControls";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_panBox->hide();
  connect(m_panX, SIGNAL(valueChanged(int)),
	  m_map, SLOT(setPanOffsetX(int)));
  connect(m_panY, SIGNAL(valueChanged(int)),
	  m_map, SLOT(setPanOffsetY(int)));
  connect(m_map, SIGNAL(panXChanged(int)),
	  m_panX, SLOT(setValue(int)));
  connect(m_map, SIGNAL(panYChanged(int)),
	  m_panY, SLOT(setValue(int)));

  m_depthControlBox = new QHBox(m_bottomControlBox);
  tmpLabel = new QLabel(m_depthControlBox);
  tmpLabel->setText("Head:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_head = new QSpinBox(5, 3000, 10, m_depthControlBox);
  m_head->setValue(m_map->headRoom());
  tmpLabel = new QLabel(m_depthControlBox);
  tmpLabel->setText("Floor:");
  tmpLabel->setFont(QFont("Helvetica", showeq_params->statusfontsize));
  m_floor = new QSpinBox(5, 3000, 10, m_depthControlBox);
  m_floor->setValue(m_map->floorRoom());
  tmpPrefString = "ShowDepthFilterControls";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 
			      (m_map->mapLineStyle() == tMap_DepthFiltered)))
    m_depthControlBox->hide();
  connect(m_head, SIGNAL(valueChanged(int)),
	  m_map, SLOT(setHeadRoom(int)));
  connect(m_map, SIGNAL(headRoomChanged(int)),
	  m_head, SLOT(setValue(int)));
  connect(m_floor, SIGNAL(valueChanged(int)),
	  m_map, SLOT(setFloorRoom(int)));
  connect(m_map, SIGNAL(floorRoomChanged(int)),
	  m_floor, SLOT(setValue(int)));

  // add our own menu items to the maps menu
  QPopupMenu* mapMenu = m_map->menu();

  // insert a seperator to seperate our stuff from the rest
  mapMenu->insertSeparator(-1);
  m_id_topControl = mapMenu->insertItem("Show Top Controls",
					this, SLOT(toggle_top_controls(int)));

  m_id_bottomControl = mapMenu->insertItem("Show Bottom Controls",
					   this, 
					   SLOT(toggle_bottom_controls(int)));

  // insert a seperator to seperate main controls from sub-menus
  mapMenu->insertSeparator(-1);
  
  QPopupMenu* subMenu;
  subMenu = new QPopupMenu();
  subMenu->setCheckable(true);
  m_id_zoom = subMenu->insertItem("Show Zoom Controls", 
				  this, SLOT(toggle_zoom(int)));
  m_id_playerLocation = subMenu->insertItem("Show Player Location",
					    this, 
					    SLOT(toggle_playerLocation(int)));
  m_id_mouseLocation = subMenu->insertItem("Show Mouse Location",
					   this, 
					   SLOT(toggle_mouseLocation(int)));

  m_id_filter = subMenu->insertItem("Show Find",
				    this, SLOT(toggle_filter(int)));

  m_id_topControl_Options = mapMenu->insertItem("Top Controls", subMenu);

  subMenu = new QPopupMenu();
  subMenu->setCheckable(true);
  m_id_frameRate = subMenu->insertItem("Show Frame Rate",
				       this, SLOT(toggle_frameRate(int)));
  m_id_pan = subMenu->insertItem("Show Pan",
				 this, SLOT(toggle_pan(int)));
  m_id_depthControlRoom = subMenu->insertItem("Show Depth Filter Controls",
					   this, 
					   SLOT(toggle_depthControls(int)));
  m_id_bottomControl_Options = mapMenu->insertItem("Bottom Controls", subMenu);

  // setup signal to initialize menu items when the map is about to be displayeed
  connect(mapMenu, SIGNAL(aboutToShow()),
	  this, SLOT(init_Menu()));
}

MapFrame::~MapFrame()
{
}

void MapFrame::filterConfirmed()
{
  setregexp(m_filter->text());
}

void MapFrame::setregexp(const QString &str)
{
  if (m_filterMgr == NULL)
    return;

  // quick check to see if this is the same as the last filter
  if (str == m_lastFilter)
    return;
    
  printf("New Filter: %s\n", (const char*)str);

  bool needCommit = false;

  if (!m_lastFilter.isEmpty())
  {
    m_filterMgr->runtimeFilterRemFilter(m_runtimeFilterFlag,
					m_lastFilter);
    needCommit = true;
  }

  if(str.isEmpty()) 
    regexpok(0);
  else
  {
    m_lastFilter = str;

    bool valid = m_filterMgr->runtimeFilterAddFilter(m_runtimeFilterFlag, str);
    
    needCommit = true;
    
    if (valid) 
      regexpok(1);
    else 
      regexpok(2);
  }

  if (needCommit)
    m_filterMgr->runtimeFilterCommit(m_runtimeFilterFlag);
}

void MapFrame::regexpok(int ok) 
{
  static int ook=0;
  if(ok == ook)
    return;

  ook=ok;

  switch(ok)
  {
  case 0: // no text at all
    m_filter->setPalette( QPalette( QColor(200,200,200) ) );
    break;
  case 1: // Ok
    m_filter->setPalette( QPalette( QColor(0,0,255) ) );
    break;
  case 2:  // Bad
  default:
    m_filter->setPalette( QPalette( QColor(255,0,0) ) );
    break;
  } 
}

void MapFrame::mouseLocation(int16_t xPos, int16_t yPos)
{
  QString cursorPos;
  cursorPos.sprintf(" %+5hd, %+5hd", yPos, xPos);
  m_mouseLocation->setText(cursorPos);
}

void MapFrame::setPlayer(int16_t x, int16_t y, int16_t z, 
			 int16_t Dx, int16_t Dy, int16_t Dz, int32_t degrees)
{
  QString playerPos;
  playerPos.sprintf(" %+5hd, %+5hd, %+5hd", y, x, z);
  m_playerLocation->setText(playerPos);
}

void MapFrame::savePrefs(void)
{
  QString prefString = preferenceName();
  QString tmpPrefString;

  tmpPrefString = "Caption";
  pSEQPrefs->setPrefString(tmpPrefString, prefString, caption());

  tmpPrefString = "SaveSize";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
  {
    tmpPrefString = "WindowW";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, width());

    tmpPrefString = "WindowH";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, height());
  }

  tmpPrefString = "SavePosition";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
  {
    tmpPrefString = "WindowX";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, x());

    tmpPrefString = "WindowY";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, y());
  }

  tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
  {
    tmpPrefString = "ShowStatusBox";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_topControlBox->isVisible());

    tmpPrefString = "ShowZoom";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_zoomBox->isVisible());

    tmpPrefString = "ShowPlayerLocation";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_playerLocationBox->isVisible());

    tmpPrefString = "ShowMouseLocation";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_mouseLocationBox->isVisible());

    tmpPrefString = "ShowFilter";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_filterBox->isVisible());

    tmpPrefString = "ShowControlBox";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_bottomControlBox->isVisible());

    tmpPrefString = "ShowFrameRate";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_frameRateBox->isVisible());

    tmpPrefString = "ShowPanControls";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_panBox->isVisible());

    tmpPrefString = "ShowDepthFilterControls";
    pSEQPrefs->setPrefInt(tmpPrefString, prefString, m_depthControlBox->isVisible());
  }

  // make the map belonging to this frame save it's preferences
  if (m_map)
    m_map->savePrefs();
}

void MapFrame::restoreSize()
{
  QString prefString = preferenceName();
  QString wPrefString, hPrefString;
  wPrefString = "WindowW";
  hPrefString = "WindowH";

  // only restore position if it the info is available
  if (pSEQPrefs->isPreference(wPrefString, prefString) &&
      pSEQPrefs->isPreference(hPrefString, prefString))
  {
    resize(pSEQPrefs->getPrefInt(wPrefString, prefString, width()),
	   pSEQPrefs->getPrefInt(wPrefString, prefString, height()));
  }
}

void MapFrame::restorePosition()
{
  QString prefString = preferenceName();
  QString xPrefString, yPrefString;
  xPrefString = "WindowX";
  yPrefString = "WindowY";

  // only restore position if it the info is available
  if (pSEQPrefs->isPreference(xPrefString, prefString) &&
      pSEQPrefs->isPreference(yPrefString, prefString))
  {
    move(pSEQPrefs->getPrefInt(xPrefString, prefString, x()),
	 pSEQPrefs->getPrefInt(yPrefString, prefString, y()));
  }
}

void MapFrame::dumpInfo(QTextStream& out)
{
  // first dump information about the map frame
  out << "[" << preferenceName() << "]" << endl;
  out << "Caption: " << caption() << endl;
  out << "ShowStatusBox: " << m_topControlBox->isVisible() << endl;
  out << "ShowZoom: " << m_zoomBox->isVisible() << endl;
  out << "ShowPlayerLocation: " << m_playerLocationBox->isVisible() << endl;
  out << "ShowMouseLocation: " << m_mouseLocationBox->isVisible() << endl;
  out << "ShowFilter: " << m_filterBox->isVisible() << endl;
  out << "ShowControlBox: " << m_bottomControlBox->isVisible() << endl;
  out << "ShowFrameRate: " << m_frameRateBox->isVisible() << endl;
  out << "ShowPanControls: " << m_panBox->isVisible() << endl; 
  out << "ShowDepthFilterControls: " << m_depthControlBox->isVisible() << endl;
  out << "CurrentFilter: '" << m_lastFilter << "'" << endl;
  out << "RuntimeFilterFlag: " << m_runtimeFilterFlag << endl;
  out << "RuntimeFilterFlagMask: " << m_runtimeFilterFlagMask << endl;
  out << endl;

  // dump information about the map
  if (m_map)
    m_map->dumpInfo(out);
}

void MapFrame::init_Menu(void)
{
  QPopupMenu* mapMenu = m_map->menu();
  mapMenu->setItemEnabled(m_id_topControl_Options, 
			  m_topControlBox->isVisible());
  mapMenu->setItemChecked(m_id_topControl,
			  m_topControlBox->isVisible());
  if (m_topControlBox->isVisible())
  {
    mapMenu->setItemChecked(m_id_zoom, m_zoomBox->isVisible());
    mapMenu->setItemChecked(m_id_playerLocation, 
			    m_playerLocationBox->isVisible());
    mapMenu->setItemChecked(m_id_mouseLocation, 
			    m_mouseLocation->isVisible());
    mapMenu->setItemChecked(m_id_filter, m_filterBox->isVisible());
  }

  mapMenu->setItemEnabled(m_id_bottomControl_Options,
			  m_bottomControlBox->isVisible());
  mapMenu->setItemChecked(m_id_bottomControl,
			  m_bottomControlBox->isVisible());
  if (m_bottomControlBox->isVisible())
  {
    mapMenu->setItemChecked(m_id_frameRate, m_frameRateBox->isVisible());
    mapMenu->setItemChecked(m_id_pan, m_panBox->isVisible());
    mapMenu->setItemChecked(m_id_depthControlRoom, 
			    m_depthControlBox->isVisible());
  }
}

void MapFrame::toggle_top_controls(int id)
{
  if (m_topControlBox->isVisible())
    m_topControlBox->hide();
  else
    m_topControlBox->show();
}

void MapFrame::toggle_bottom_controls(int id)
{
  if (m_bottomControlBox->isVisible())
    m_bottomControlBox->hide();
  else
    m_bottomControlBox->show();
}

void MapFrame::toggle_zoom(int id)
{
  if (m_zoomBox->isVisible())
    m_zoomBox->hide();
  else
    m_zoomBox->show();
}

void MapFrame::toggle_playerLocation(int id)
{
  if (m_playerLocationBox->isVisible())
    m_playerLocationBox->hide();
  else
    m_playerLocationBox->show();
}

void MapFrame::toggle_mouseLocation(int id)
{
  if (m_mouseLocationBox->isVisible())
    m_mouseLocationBox->hide();
  else
    m_mouseLocationBox->show();
}

void MapFrame::toggle_filter(int id)
{
  if (m_filterBox->isVisible())
    m_filterBox->hide();
  else
    m_filterBox->show();
}

void MapFrame::toggle_frameRate(int id)
{
  if (m_frameRateBox->isVisible())
    m_frameRateBox->hide();
  else
    m_frameRateBox->show();
}

void MapFrame::toggle_pan(int id)
{
  if (m_panBox->isVisible())
    m_panBox->hide();
  else
    m_panBox->show();
}

void MapFrame::toggle_depthControls(int id)
{
  if (m_depthControlBox->isVisible())
    m_depthControlBox->hide();
  else
    m_depthControlBox->show();
}
