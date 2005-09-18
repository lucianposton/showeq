/*
 * map.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 * 
 * Portions Copyright 2001-2004 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

#define __STDC_LIMIT_MACROS
#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif


#include "map.h"
#include "ui_mapicondialog.h"
#include "util.h"
#include "main.h"
#include "filtermgr.h"
#include "zonemgr.h"
#include "spawnmonitor.h"
#include "player.h"
#include "spawnshell.h"
#include "datalocationmgr.h"
#include "diagnosticmessages.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qapplication.h>
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
#include <qtimer.h>
#include <qstrlist.h>
#include <qimage.h>

#if 1 // ZBTEMP: Until we setup a better way to enter location name/color
#include <qinputdialog.h>
#endif

//----------------------------------------------------------------------
// Macros
//#define DEBUG

//#define DEBUGMAP

//#define DEBUGMAPLOAD


//----------------------------------------------------------------------
// constants
const int panAmmt = 8;

//----------------------------------------------------------------------
// CLineDlg
CLineDlg::CLineDlg(QWidget *parent, QString name, MapMgr *mapMgr) 
  : QDialog(parent, name, FALSE)
{
#ifdef DEBUGMAP
  debug ("CLineDlg()");
#endif /* DEBUGMAP */
   
  QFont labelFont;
  labelFont.setBold(true);

  QBoxLayout *topLayout = new QVBoxLayout(this);
  QBoxLayout *row2Layout = new QHBoxLayout(topLayout);
  QBoxLayout *row1Layout = new QHBoxLayout(topLayout);
  
  QLabel *colorLabel = new QLabel ("Color", this);
  colorLabel->setFont(labelFont);
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
  m_LineColor->insertItem("darkGray");
  m_LineColor->insertItem("white");
  m_LineColor->insertItem("blue");
  m_LineColor->insertItem("green");
  m_LineColor->insertItem("cyan");
  m_LineColor->insertItem("red");
  m_LineColor->insertItem("magenta");
  m_LineColor->insertItem("yellow");
  m_LineColor->insertItem("white");
  
  m_LineColor->setFont(labelFont);
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
  nameLabel->setFont(labelFont);
  nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  nameLabel->setFixedWidth(nameLabel->sizeHint().width()+5);
  nameLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
  row2Layout->addWidget(nameLabel);

  m_LineName  = new QLineEdit(this, "LineName");
  m_LineName->setFont(labelFont);
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
MapLabel::MapLabel( Map* map )
  : QLabel(map, "mapLabel", 
       WStyle_Customize + WStyle_NoBorderEx + WStyle_Tool + WType_TopLevel
       + WStyle_Dialog + WX11BypassWM)
{
  m_Map = map;
  setMargin( 1 );
  setIndent( 0 );
  setAutoMask( false );
  setFrameStyle( QFrame::Plain | QFrame::Box );
  setLineWidth( 1 );
  setAlignment( AlignLeft | AlignTop );
  polish();
  setText( "" );
  adjustSize();        
}

void MapLabel::popup(const QPoint& pos)
{
  // make sure the widgets size is current.
  adjustSize();

  // borrowed from QPopupMenu::popup()

  // get info about the widget and its environment
  QWidget *desktop = QApplication::desktop();
  int sw = desktop->width();                  // screen width
  int sh = desktop->height();                 // screen height
  int sx = desktop->x();                      // screen pos
  int sy = desktop->y();
  int x  = pos.x();
  int y  = pos.y();
  int w  = width();
  int h  = height();

  // the complete widget must be visible, move it if necessary
  if ( x+w > sw )
    x = sw - w;
  if ( y+h > sh )
    y = sh - h;
  if ( x < sx )
    x = sx;
  if ( y < sy )
    y = sy;
  move( x, y );

  // show the widget
  show();
}

void MapLabel::mousePressEvent(QMouseEvent*)
{
  // hide if the user clicks on the label
  hide();
}

//----------------------------------------------------------------------
// MapMgr
MapMgr::MapMgr(const DataLocationMgr* dataLocMgr, 
           SpawnShell* spawnShell, Player* player, ZoneMgr* zoneMgr,
           QWidget* dialogParent, QObject* parent, const char* name)
  : QObject(parent, name),
    m_dataLocMgr(dataLocMgr),
    m_spawnShell(spawnShell),
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
  connect (m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
       this, SLOT(killSpawn(const Item*)));
  connect (m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
       this, SLOT(changeItem(const Item*, uint32_t)));
  connect(m_spawnShell, SIGNAL(clearItems()),
       this, SLOT(clearItems()));

  // supply the MapMgr slots with signals from ZoneMgr
  connect(zoneMgr, SIGNAL(zoneBegin(const QString&)),
      this, SLOT(zoneBegin(const QString&)));
  connect(zoneMgr, SIGNAL(zoneChanged(const QString&)),
      this, SLOT(zoneChanged(const QString&)));
  connect(zoneMgr, SIGNAL(zoneEnd(const QString&, const QString&)),
      this, SLOT(zoneEnd(const QString&, const QString&)));

  // if there is a short zone name already, try to load its map
  QString shortZoneName = zoneMgr->shortZoneName();
  if (!shortZoneName.isEmpty())
    loadZoneMap(shortZoneName);
}

MapMgr::~MapMgr()
{
}

uint16_t MapMgr::spawnAggroRange(const Spawn* spawn) 
{ 
  uint16_t* range = m_spawnAggroRange.find(spawn->id()); 
  return (!range) ? 0 : *range;
}

void MapMgr::zoneBegin(const QString& shortZoneName)
{
#ifdef DEBUGMAP
  debug ("zoneBegin(%s)", 
     shortZoneName);
#endif /* DEBUGMAP */
  
  // clear the map data
  m_mapData.clear();
  
  // signal that the map has been unloaded
  emit mapUnloaded();
  
  // atttempt to load the new map
  loadZoneMap(shortZoneName);
}

void MapMgr::zoneChanged(const QString& shortZoneName)
{
#ifdef DEBUGMAP
  debug ("zoneChanged(%s)", 
     (const char*)shortZoneName);
#endif /* DEBUGMAP */

  // clear the map data
  m_mapData.clear();
  
  // signal that the map has been unloaded
  emit mapUnloaded();

  // atttempt to load the new map
  loadZoneMap(shortZoneName);
}

void MapMgr::zoneEnd(const QString& shortZoneName, const QString& longZoneName)
{
#ifdef DEBUGMAP
  debug ("zoneEnd(%s, %s)", 
     (const char*)longZoneName, (const char*)shortZoneName);
#endif /* DEBUGMAP */

  // atttempt to load the new map
  loadZoneMap(shortZoneName);
}

void MapMgr::loadZoneMap(const QString& shortZoneName)
{
  // first attempt to find map with .map suffix
  QFileInfo fileInfo = m_dataLocMgr->findExistingFile("maps", 
                              shortZoneName + ".map");
  
  // if that file doesn't exist, try a straight .txt suffix
  if (!fileInfo.exists())
    fileInfo = m_dataLocMgr->findExistingFile("maps", 
                          shortZoneName + ".txt");

  // if that file doesn't exist, try a  _1.txt suffix
  if (!fileInfo.exists())
    fileInfo = m_dataLocMgr->findExistingFile("maps", 
                          shortZoneName + "_1.txt");

  if (fileInfo.exists())
  {
    // load the map if it's not already loaded
    if (fileInfo.absFilePath() != m_mapData.fileName())
      loadFileMap(fileInfo.absFilePath());
  }
  else 
  {
    seqInfo("No Map found for zone '%s'!", (const char*)shortZoneName);
    seqInfo("    Checked for all variants of '%s.map', '%s.txt', and '%s_1.txt'",
        (const char*)shortZoneName, 
        (const char*)shortZoneName, (const char*)shortZoneName);
    seqInfo("    in directories '%s' and '%s'!",
        (const char*)m_dataLocMgr->userDataDir("maps").absPath(),
        (const char*)m_dataLocMgr->pkgDataDir("maps").absPath());
  }
}

void MapMgr::loadMap ()
{
#ifdef DEBUGMAP
  debug ("loadMap()");
#endif /* DEBUGMAP */

  QString fileName = m_mapData.fileName();

  if (fileName.isEmpty())
    fileName = m_dataLocMgr->findExistingFile("maps", fileName).absFilePath();

  // create a file dialog the defaults to the currently open map
  fileName = QFileDialog::getOpenFileName(fileName, "*.map;*.txt");

  if (fileName.isEmpty ())
    return;

  seqInfo("Attempting to load map: %s", (const char*)fileName);

  // load the map
  loadFileMap(fileName, false, true);
}

void MapMgr::importMap ()
{
#ifdef DEBUGMAP
  debug ("importMap()");
#endif /* DEBUGMAP */

  QString fileName = m_mapData.fileName();

  if (fileName.isEmpty())
    fileName = m_dataLocMgr->findExistingFile("maps", fileName).absFilePath();

  // create a file dialog the defaults to the currently open map
  fileName = QFileDialog::getOpenFileName(fileName, "*.map;*.txt");

  if (fileName.isEmpty ())
    return;

  seqInfo("Attempting to import map: %s", (const char*)fileName);

  // load the map
  loadFileMap(fileName, true, true);
}


void MapMgr::loadFileMap (const QString& fileName, bool import, bool force) 
{
#ifdef DEBUGMAP
  debug ("loadFileMap()");
#endif /* DEBUGMAP */

  // if not a forced load, and the same map is already loaded, do nothing
  if (!force && m_mapData.mapLoaded() && 
      (m_mapData.fileName() == fileName))
    return;

  // load the specified map
  if (!fileName.endsWith(".txt"))
    m_mapData.loadMap(fileName, import);
  else
    m_mapData.loadSOEMap(fileName, import);

  const ItemMap& itemMap = m_spawnShell->spawns();
  ItemConstIterator it(itemMap);
  const Item* item;
  uint16_t range;

  // iterate over the exixsting spawns to adjust the map size and find 
  // ones with aggro information
  for (; it.current(); ++it)
  {
    // get the item from the list
    item = it.current();

    // Adjust X and Y for spawns on map
    m_mapData.quickCheckPos(item->x(), item->y());
    
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
  QFileInfo fileInfo(m_mapData.fileName());

  fileInfo = m_dataLocMgr->findWriteFile("maps", fileInfo.baseName() + ".map", 
                     false);

  m_mapData.saveMap(fileInfo.absFilePath());
}

void MapMgr::saveSOEMap ()
{
#ifdef DEBUGMAP
  debug ("saveMap()");
#endif /* DEBUGMAP */
  QFileInfo fileInfo(m_mapData.fileName());

  fileInfo = m_dataLocMgr->findWriteFile("maps", fileInfo.baseName() + "_2.txt", 
                     false);

  m_mapData.saveSOEMap(fileInfo.absFilePath());
}

void MapMgr::addItem(const Item* item)
{
  if ((item == NULL) || (item->type() != tSpawn))
    return;

  // make sure it fits on the map display
  m_mapData.checkPos(item->x(), item->y());

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
  m_mapData.checkPos(item->x(), item->y());

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
    if ( m_mapData.checkPos(item->x(), item->y()))
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
                       QLineEdit::Normal,
                       QString::null, &ok, parent);

  // if the user clicked ok, and actually gave a name, add it
  if (ok && !name.isEmpty())
    m_mapData.addLocation(name, m_curLocationColor, 
              QPoint(point.x(), point.y()));
  
#ifdef DEBUGMAP
  seqDebug("addLocation(): Location x added at %d/%d", 
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

  pSEQPrefs->setPrefString("DefaultLineName", "MapMgr", m_curLineName);

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::setLineColor(const QString &color)
{
  // set the color of the current line
  m_mapData.setLineColor(color);

  // save the line color for future use
  m_curLineColor = color;

  pSEQPrefs->setPrefString("DefaultLineColor", "MapMgr", m_curLineColor);

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

void MapMgr::scaleDownZ(int16_t factor)
{
  m_mapData.scaleDownZ(factor);

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::scaleUpZ(int16_t factor)
{
  m_mapData.scaleUpZ(factor);

  // signal that the map has been updated
  emit mapUpdated();
}

void MapMgr::savePrefs(void)
{
#if 0 // ZBTEMP: Migrate to place where ever this is set
  pSEQPrefs->setPrefString("DefaultLocationColor", "MapMgr", m_curLocationColor);
#endif
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
  out << "LLines: " << m_mapData.lLines().count() << endl;
  out << "MLines: " << m_mapData.mLines().count() << endl;
  out << "Locations: " << m_mapData.locations().count() << endl;
  out << "Aggros: " << m_mapData.aggros().count() << endl;
  out << endl;
}

//----------------------------------------------------------------------
// MapMenu
MapMenu::MapMenu(Map* map, QWidget* parent, const char* name)
  : QPopupMenu(parent, name),
    m_map(map),
    m_mapIcons(map->mapIcons())
{
  QString preferenceName = m_map->preferenceName();

  // set the caption to be the preference name of the map
  setCaption(preferenceName);
  setCheckable(true);

  QPopupMenu* subMenu;
  QPopupMenu* subSubMenu;

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

  key = pSEQPrefs->getPrefKey("AddLocationKey", preferenceName, "Ctrl+O");
  m_id_addLocation = subMenu->insertItem("Add Location...",
                     m_map, SLOT(addLocation()), key);
  key = pSEQPrefs->getPrefKey("StartLineKey", preferenceName, "Ctrl+L");
  m_id_startLine = subMenu->insertItem("Start Line",
                       m_map, SLOT(startLine()), key);
  key = pSEQPrefs->getPrefKey("AddLinePointKey", preferenceName, "Ctrl+P");
  m_id_addLinePoint = subMenu->insertItem("Add Line Point",
                      m_map, SLOT(addLinePoint()), key);
  key = pSEQPrefs->getPrefKey("DelLinePointKey", preferenceName, "Ctrl+D");
  m_id_delLinePoint = subMenu->insertItem("Delete Line Point",
                      m_map, SLOT(delLinePoint()), key);
  m_id_showLineDlg = subMenu->insertItem("Show Line Dialog...",
                     m_map, SLOT(showLineDlg()));
  subSubMenu = new QPopupMenu(m_map);
  int x;
  x = subSubMenu->insertItem("Down 8", m_map, SLOT(scaleDownZ(int)));
  subSubMenu->setItemParameter(x, 8);
  x = subSubMenu->insertItem("Down 10", m_map, SLOT(scaleDownZ(int)));
  subSubMenu->setItemParameter(x, 10);
  x = subSubMenu->insertItem("Up 8", m_map, SLOT(scaleUpZ(int)));
  subSubMenu->setItemParameter(x, 8);
  x = subSubMenu->insertItem("Up 10", m_map, SLOT(scaleUpZ(int)));
  subSubMenu->setItemParameter(x, 10);

  subMenu->insertItem("Scale Map Z Coordinates", subSubMenu);
  m_id_editMap = insertItem("Edit", subMenu);

  subMenu = new QPopupMenu(m_map);
  subMenu->setCheckable(true);
  m_id_mapLineStyle_Normal = subMenu->insertItem("Normal");
  subMenu->setItemParameter(m_id_mapLineStyle_Normal, tMap_Normal);
  key = pSEQPrefs->getPrefKey("MapLineNormalKey", preferenceName, "Alt+1");
  subMenu->setAccel(key, m_id_mapLineStyle_Normal);
  m_id_mapLineStyle_DepthFiltered = subMenu->insertItem("Depth Filtered");
  subMenu->setItemParameter(m_id_mapLineStyle_DepthFiltered, tMap_DepthFiltered);
  key = pSEQPrefs->getPrefKey("MapLineDepthFilteredKey", preferenceName, "Alt+2");
  subMenu->setAccel(key, m_id_mapLineStyle_DepthFiltered);
  m_id_mapLineStyle_FadedFloors = subMenu->insertItem("Faded Floors");
  subMenu->setItemParameter(m_id_mapLineStyle_FadedFloors, tMap_FadedFloors);
  key = pSEQPrefs->getPrefKey("MapLineFadedFloorsKey", preferenceName, "Alt+3");
  subMenu->setAccel(key, m_id_mapLineStyle_FadedFloors);
  connect(subMenu, SIGNAL(activated(int)),
      this, SLOT(select_mapLine(int)));
  m_id_mapLineStyle = insertItem("Map Line Display", subMenu);

  m_id_spawnDepthFilter = insertItem("Spawn Depth Filter", 
                     this, SLOT(toggle_spawnDepthFilter(int)));
  key = pSEQPrefs->getPrefKey("SpawnDepthFilteredKey", preferenceName, "Alt+5");
  setAccel(key, m_id_spawnDepthFilter);

  subMenu = new QPopupMenu(m_map);
  subMenu->setCheckable(true);
  
  m_id_tooltip = subMenu->insertItem("Tooltips", this, SLOT(toggle_tooltip(int)));;
  m_id_filtered = subMenu->insertItem("Filtered", this, SLOT(toggle_filtered(int)));
  m_id_map = subMenu->insertItem("Map Lines", this, SLOT(toggle_map(int)));
  m_id_velocity = subMenu->insertItem("Velocity Lines",    this, SLOT(toggle_velocity(int)));
  m_id_animate = subMenu->insertItem("Animate Spawns", this, SLOT(toggle_animate(int)));
  m_id_player = subMenu->insertItem("Player", this, SLOT(toggle_player(int)));
  m_id_playerBackground = subMenu->insertItem("Player Background", this, SLOT(toggle_playerBackground(int)));
  m_id_playerView = subMenu->insertItem("Player View", this, SLOT(toggle_playerView(int)));
  m_id_gridLines = subMenu->insertItem("Grid Lines", this, SLOT(toggle_gridLines(int)));;
  m_id_gridTicks = subMenu->insertItem("Grid Ticks", this, SLOT(toggle_gridTicks(int)));
  m_id_locations = subMenu->insertItem("Locations", this, SLOT(toggle_locations(int)));
  m_id_spawns = subMenu->insertItem("Spawns", this, SLOT(toggle_spawns(int)));
  m_id_unknownSpawns = subMenu->insertItem("Unknown Spawns", 
                       this, SLOT(toggle_unknownSpawns(int)));
  m_id_spawnPoints = subMenu->insertItem("Spawn Points", this, SLOT( toggle_spawnPnts(int) ));
  m_id_drops = subMenu->insertItem("Drops", this, SLOT(toggle_drops(int)));
  m_id_doors = subMenu->insertItem("Doors", this, SLOT(toggle_doors(int)));
  m_id_spawnNames = subMenu->insertItem("Spawn Names", this, SLOT(toggle_spawnNames(int)));
  m_id_highlightConsideredSpawns =
    subMenu->insertItem("Highlight Considered Spawns", this, SLOT(toggle_highlightConsideredSpawns(int)));
  m_id_walkPath =
    subMenu->insertItem("Selections Walk Path", this, SLOT(toggle_walkPath(int)));
  m_id_npcWalkPaths =
    subMenu->insertItem("NPC Walk Paths", this, SLOT(toggle_npcWalkPaths(int)));
  m_id_mapImage =
    subMenu->insertItem("Map Image", this, SLOT(toggle_mapImage(int)));
  m_id_pvp = 
    subMenu->insertItem("PvP", this, SLOT(toggle_pvp(int)));
  m_id_deityPvP = 
    subMenu->insertItem("Deity PvP", this, SLOT(toggle_deityPvP(int)));
  m_id_racePvP = 
    subMenu->insertItem("Race PvP", this, SLOT(toggle_racePvP(int)));
  m_id_zoneSafePoint = 
    subMenu->insertItem("Zone Safe Point", this, SLOT(toggle_zoneSafePoint(int)));
#ifdef DEBUG
  m_id_debugInfo = subMenu->insertItem("Debug Info", this, SLOT(toggle_debugInfo(int)));
#endif
  m_id_showSub = insertItem("Show", subMenu);
  

  subMenu = new QPopupMenu;
  subMenu->setCheckable(true);
  m_id_mapOptimization_Memory = subMenu->insertItem("Memory");
  subMenu->setItemParameter(m_id_mapOptimization_Memory, tMap_MemoryOptim); 
 m_id_mapOptimization_Normal = subMenu->insertItem("Normal");
  subMenu->setItemParameter(m_id_mapOptimization_Normal, tMap_NormalOptim);
  m_id_mapOptimization_Best = subMenu->insertItem("Speed");
  subMenu->setItemParameter(m_id_mapOptimization_Best, tMap_BestOptim);
  connect(subMenu, SIGNAL(activated(int)),
      this, SLOT(select_mapOptimization(int)));
  m_id_mapOptimization = insertItem("Map Optimization", subMenu);

  m_id_gridTickColor = insertItem("Grid Tick Color...",
                  this, SLOT(select_gridTickColor(int)));
  m_id_gridLineColor = insertItem("Grid Line Color...",
                  this, SLOT(select_gridLineColor(int)));
  m_id_backgroundColor = insertItem("Background Color...",
                    this, SLOT(select_backgroundColor(int)));
  m_id_font = insertItem("Font...",
             this, SLOT(select_font(int)));
  insertItem("Edit Map Icons...",
         m_map, SLOT(showMapIconDialog()));

  subMenu = new QPopupMenu;
  m_drawSizeSpinBox = new QSpinBox(1, 6, 1, subMenu);
  m_drawSizeSpinBox->setValue(m_mapIcons->drawSize());
  connect(m_drawSizeSpinBox, SIGNAL(valueChanged(int)),
      m_mapIcons, SLOT(setDrawSize(int)));
  m_id_drawSize = subMenu->insertItem(m_drawSizeSpinBox);
  m_id_drawSizeMenu = insertItem("Draw Size", subMenu);

  subMenu = new QPopupMenu;
  QHBox* tmpHBox = new QHBox(subMenu);
  m_fovSpinBoxLabel = new QLabel("Distance:", tmpHBox);
  m_fovSpinBox = new QSpinBox(20, 1200, 20, tmpHBox, "FOV");
  m_fovSpinBox->setValue(m_mapIcons->fovDistance());
  connect(m_fovSpinBox, SIGNAL(valueChanged(int)),
      m_mapIcons, SLOT(setFOVDistance(int)));
  m_id_FOVDistance = subMenu->insertItem(tmpHBox);
  m_id_FOVColor = subMenu->insertItem("Color...",
                      this, SLOT(select_fovColor(int)));

  subSubMenu = new QPopupMenu;
  subMenu->setCheckable(true);
  m_id_FOVNoBrush = subSubMenu->insertItem("No Background");
  subSubMenu->setItemParameter(m_id_FOVNoBrush, Qt::NoBrush); 
  m_id_FOVSolidPattern = subSubMenu->insertItem("Solid");
  subSubMenu->setItemParameter(m_id_FOVSolidPattern, Qt::SolidPattern); 
  m_id_FOVDense1Pattern = subSubMenu->insertItem("94% fill");
  subSubMenu->setItemParameter(m_id_FOVDense1Pattern, Qt::Dense1Pattern); 
  m_id_FOVDense2Pattern = subSubMenu->insertItem("88% fill");
  subSubMenu->setItemParameter(m_id_FOVDense2Pattern, Qt::Dense2Pattern); 
  m_id_FOVDense3Pattern  = subSubMenu->insertItem("63% fill");
  subSubMenu->setItemParameter(m_id_FOVDense3Pattern, Qt::Dense3Pattern); 
  m_id_FOVDense4Pattern = subSubMenu->insertItem("50% fill");
  subSubMenu->setItemParameter(m_id_FOVDense4Pattern, Qt::Dense4Pattern); 
  m_id_FOVDense5Pattern = subSubMenu->insertItem("37% fill");
  subSubMenu->setItemParameter(m_id_FOVDense5Pattern, Qt::Dense5Pattern); 
  m_id_FOVDense6Pattern = subSubMenu->insertItem("12% fill");
  subSubMenu->setItemParameter(m_id_FOVDense6Pattern, Qt::Dense6Pattern); 
  m_id_FOVDense7Pattern = subSubMenu->insertItem("6% fill");
  subSubMenu->setItemParameter(m_id_FOVDense7Pattern, Qt::Dense7Pattern); 
  m_id_FOVHorPattern = subSubMenu->insertItem("Horizontal lines");
  subSubMenu->setItemParameter(m_id_FOVHorPattern, Qt::HorPattern); 
  m_id_FOVVerPattern = subSubMenu->insertItem("Vertical lines");
  subSubMenu->setItemParameter(m_id_FOVVerPattern, Qt::VerPattern); 
  m_id_FOVCrossPattern = subSubMenu->insertItem("Crossing lines");
  subSubMenu->setItemParameter(m_id_FOVCrossPattern, Qt::CrossPattern); 
  m_id_FOVBDiagPattern = subSubMenu->insertItem("Diagonal lines (directed /)");
  subSubMenu->setItemParameter(m_id_FOVBDiagPattern, Qt::BDiagPattern); 
  m_id_FOVFDiagPattern = subSubMenu->insertItem("Diagonal lines (directed \\)");
  subSubMenu->setItemParameter(m_id_FOVFDiagPattern, Qt::FDiagPattern); 
  m_id_FOVDiagCrossPattern = subSubMenu->insertItem("Diagonal crossing lines");
  subSubMenu->setItemParameter(m_id_FOVDiagCrossPattern, Qt::DiagCrossPattern); 
  connect(subSubMenu, SIGNAL(activated(int)),
      this, SLOT(select_fovStyle(int)));
  subMenu->insertItem("FOV Style", subSubMenu);

  subSubMenu = new QPopupMenu;
  subMenu->setCheckable(true);
  m_id_FOVDistanceBased = subSubMenu->insertItem("Distance Based");
  subSubMenu->setItemParameter(m_id_FOVDistanceBased, tFOVDistanceBased); 
  m_id_FOVScaledClassic = subSubMenu->insertItem("Scaled Classic");
  subSubMenu->setItemParameter(m_id_FOVScaledClassic, tFOVScaledClassic); 
  m_id_FOVClassic = subSubMenu->insertItem("Classic");
  subSubMenu->setItemParameter(m_id_FOVClassic, tFOVClassic); 
  connect(subSubMenu, SIGNAL(activated(int)),
      this, SLOT(select_fovMode(int)));
  subMenu->insertItem("FOV Mode", subSubMenu);

  m_id_FOV = insertItem("Player FOV", subMenu);

  subMenu = new QPopupMenu;
  m_zoomDefaultSpinBox = new QSpinBox(1, 32, 1, subMenu);
  m_zoomDefaultSpinBox->setValue(m_mapIcons->drawSize());
  connect(m_zoomDefaultSpinBox, SIGNAL(valueChanged(int)),
      m_map, SLOT(setZoomDefault(int)));
  m_id_zoomDefault = subMenu->insertItem(m_zoomDefaultSpinBox);
  m_id_zoomDefaultMenu = insertItem("Default Zoom", subMenu);

  m_id_cacheAlwaysRepaint = insertItem("Always Repaint Map Cache",
                       this, 
                       SLOT(toggle_cacheAlwaysRepaint()));

  m_id_cacheChanges = insertItem("Cache Changes",
                 this, 
                 SLOT(toggle_cacheChanges()));

  insertItem("Save Map Image...",
         m_map, SLOT(saveMapImage()));

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
  setItemChecked(m_id_spawnPoints, m_map->showSpawnPoints());
  setItemChecked(m_id_unknownSpawns, m_map->showUnknownSpawns());
  setItemChecked(m_id_drops, m_map->showDrops());
  setItemChecked(m_id_doors, m_map->showDoors());
  setItemChecked(m_id_spawnNames, m_mapIcons->showSpawnNames());
  setItemChecked(m_id_highlightConsideredSpawns, 
         m_map->highlightConsideredSpawns());
  setItemChecked(m_id_walkPath, m_map->walkPathShowSelect());
  setItemChecked(m_id_npcWalkPaths, m_mapIcons->showNPCWalkPaths());
  setItemChecked(m_id_mapImage, m_map->showBackgroundImage());
  setItemChecked(m_id_pvp, m_map->pvp());
  setItemChecked(m_id_deityPvP, m_map->deityPvP());
  setItemChecked(m_id_racePvP, m_map->racePvP());
  setItemChecked(m_id_zoneSafePoint, m_map->showZoneSafePoint());
#ifdef DEBUG
  setItemChecked(m_id_debugInfo, m_map->showDebugInfo());
#endif
  MapOptimizationMethod method = m_map->mapOptimization();

  setItemChecked(m_id_mapOptimization_Memory, (method == tMap_MemoryOptim));
  setItemChecked(m_id_mapOptimization_Normal, (method == tMap_NormalOptim));
  setItemChecked(m_id_mapOptimization_Best, (method == tMap_BestOptim));

  m_drawSizeSpinBox->setValue(m_mapIcons->drawSize());

  m_fovSpinBox->setValue(m_mapIcons->fovDistance());

  int fovStyle = m_map->fovStyle();
  setItemChecked(m_id_FOVNoBrush, (fovStyle == Qt::NoBrush));
  setItemChecked(m_id_FOVSolidPattern, (fovStyle == Qt::SolidPattern));
  setItemChecked(m_id_FOVDense1Pattern, (fovStyle == Qt::Dense1Pattern));
  setItemChecked(m_id_FOVDense2Pattern, (fovStyle == Qt::Dense2Pattern));
  setItemChecked(m_id_FOVDense3Pattern, (fovStyle == Qt::Dense3Pattern));
  setItemChecked(m_id_FOVDense4Pattern, (fovStyle == Qt::Dense4Pattern));
  setItemChecked(m_id_FOVDense5Pattern, (fovStyle == Qt::Dense5Pattern));
  setItemChecked(m_id_FOVDense6Pattern, (fovStyle == Qt::Dense6Pattern));
  setItemChecked(m_id_FOVDense7Pattern, (fovStyle == Qt::Dense7Pattern));
  setItemChecked(m_id_FOVHorPattern, (fovStyle == Qt::HorPattern));
  setItemChecked(m_id_FOVVerPattern, (fovStyle == Qt::VerPattern));
  setItemChecked(m_id_FOVCrossPattern, (fovStyle == Qt::CrossPattern));
  setItemChecked(m_id_FOVBDiagPattern, (fovStyle == Qt::BDiagPattern));
  setItemChecked(m_id_FOVFDiagPattern, (fovStyle == Qt::FDiagPattern));
  setItemChecked(m_id_FOVDiagCrossPattern, (fovStyle == Qt::DiagCrossPattern));

  init_fovMenu();

  m_zoomDefaultSpinBox->setValue(m_map->zoomDefault());

  setItemChecked(m_id_cacheAlwaysRepaint, m_map->cacheAlwaysRepaint());
  setItemChecked(m_id_cacheChanges, m_map->cacheChanges());
}

void MapMenu::init_fovMenu(void)
{
  FOVMode fovMode = m_map->fovMode();

  // calculate new base FOV Distance
  int newFOVDistMin = 20;
  int newFOVDistMax = 1200;
  int newFOVDistInc = 20;
  if (fovMode != tFOVDistanceBased)
  {
    newFOVDistMin = 5;
    newFOVDistMax = 120;
    newFOVDistInc = 1;
    
    m_fovSpinBoxLabel->setText("Base Radius:");
  }
  else 
    m_fovSpinBoxLabel->setText("Distance:");

  int fovDistance = m_mapIcons->fovDistance();
  m_fovSpinBox->setRange(newFOVDistMin, newFOVDistMax);
  m_fovSpinBox->setLineStep(newFOVDistInc);
  m_fovSpinBox->setValue(fovDistance);

  setItemChecked(m_id_FOVDistanceBased, (fovMode == tFOVDistanceBased));
  setItemChecked(m_id_FOVScaledClassic, (fovMode == tFOVScaledClassic));
  setItemChecked(m_id_FOVClassic, (fovMode == tFOVClassic));
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

void MapMenu::toggle_pvp(int itemId)
{
  m_map->setPvP(!m_map->pvp());
}

void MapMenu::toggle_deityPvP(int itemId)
{
  m_map->setDeityPvP(!m_map->deityPvP());
}

void MapMenu::toggle_racePvP(int itemId)
{
  m_map->setRacePvP(!m_map->racePvP());
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

void MapMenu::toggle_spawnPnts(int itemId)
{
  m_map->setShowSpawnPoints(!m_map->showSpawnPoints());
}

void MapMenu::toggle_unknownSpawns(int itemId)
{
  m_map->setShowUnknownSpawns(!m_map->showUnknownSpawns());
}

void MapMenu::toggle_drops(int itemId)
{
  m_map->setShowDrops(!m_map->showDrops());
}

void MapMenu::toggle_doors(int itemId)
{
  m_map->setShowDoors(!m_map->showDoors());
}

void MapMenu::toggle_spawnNames(int itemId)
{
  m_mapIcons->setShowSpawnNames(!m_mapIcons->showSpawnNames());
}

void MapMenu::toggle_highlightConsideredSpawns(int itemId)
{
  m_map->setHighlightConsideredSpawns(!m_map->highlightConsideredSpawns());
}

void MapMenu::toggle_walkPath(int itemId)
{
  m_map->setWalkPathShowSelect(!m_map->walkPathShowSelect());
}

void MapMenu::toggle_npcWalkPaths(int itemId)
{
  m_mapIcons->setShowNPCWalkPaths(!m_mapIcons->showNPCWalkPaths());
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

void MapMenu::toggle_cacheAlwaysRepaint()
{
  m_map->setCacheAlwaysRepaint(!m_map->cacheAlwaysRepaint());
}

void MapMenu::toggle_cacheChanges()
{
  m_map->setCacheChanges(!m_map->cacheChanges());
}

void MapMenu::toggle_zoneSafePoint(int itemId)
{
  m_map->setShowZoneSafePoint(!m_map->showZoneSafePoint());
}

void MapMenu::select_mapOptimization(int itemId)
{
  m_map->setMapOptimization((MapOptimizationMethod)itemParameter(itemId));
}

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

  if (ok)
    m_map->setFont(newFont);
}

void MapMenu::select_fovColor(int itemId)
{
  QString name = QString("ShowEQ - ") + m_map->preferenceName() 
    + " Player FOV Color";
  QColor newColor = QColorDialog::getColor(m_map->fovColor(),
                       m_map, (const char*)name);

  if (newColor.isValid())
    m_map->setFOVColor(newColor);
}

void MapMenu::select_fovStyle(int itemId)
{
  m_map->setFOVStyle(itemParameter(itemId));
}

void MapMenu::select_fovMode(int itemId)
{
  FOVMode oldFOVMode = m_map->fovMode();
  FOVMode newFOVMode = (FOVMode)itemParameter(itemId);

  if (oldFOVMode != newFOVMode)
  {
    // set the new FOV Mode
    uint16_t newFOVDistance = 0;
    if ((newFOVMode != tFOVDistanceBased) && 
    (oldFOVMode == tFOVDistanceBased))
      newFOVDistance = 40;
    else if (newFOVMode == tFOVDistanceBased)
      newFOVDistance = 200;

    m_map->setFOVMode(newFOVMode);

    if (newFOVDistance != 0)
      m_mapIcons->setFOVDistance(newFOVDistance);

    init_fovMenu();
  }
}

//----------------------------------------------------------------------
// Map
Map::Map(MapMgr* mapMgr, 
     Player* player, 
     SpawnShell* spawnshell, 
     ZoneMgr* zoneMgr,
     SpawnMonitor* spawnMonitor,
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
    m_mapIcons(0),
    m_mapIconDialog(0),
    m_runtimeFilterFlagMask(runtimeFilterFlagMask),
    m_player(player),
    m_spawnShell(spawnshell),
    m_zoneMgr(zoneMgr),
    m_spawnMonitor(spawnMonitor)
{
#ifdef DEBUGMAP
  debug ("Map()");
#endif /* DEBUGMAP */

  // save the name used for preferences 
  QString prefString = Map::preferenceName();
  QString tmpPrefString;
  QString tmpDefault;
  QString tmp;

  // create the map icons object
  m_mapIcons = new MapIcons(player, preferenceName + "Icons", 
                this, "mapicons");

  connect(m_mapIcons, SIGNAL(changed()),
      this, SLOT(reAdjustAndRefreshMap()));

  // load the map icon information
  m_mapIcons->load();

  // setup filter check ordering
  m_filterCheckOrdering[0] = 
    pSEQPrefs->getPrefInt("Filter0", prefString, FILTERED_FILTER);
  m_filterCheckOrdering[1] = 
    pSEQPrefs->getPrefInt("Filter1", prefString, TRACER_FILTER);
  m_filterCheckOrdering[2] = 
    pSEQPrefs->getPrefInt("Filter2", prefString, LOCATE_FILTER);
  m_filterCheckOrdering[3] = 
    pSEQPrefs->getPrefInt("Filter3", prefString, HUNT_FILTER);
  m_filterCheckOrdering[4] = 
    pSEQPrefs->getPrefInt("Filter4", prefString, ALERT_FILTER);
  m_filterCheckOrdering[5] = 
    pSEQPrefs->getPrefInt("Filter5", prefString, CAUTION_FILTER);
  m_filterCheckOrdering[6] = 
    pSEQPrefs->getPrefInt("Filter6", prefString, DANGER_FILTER);

  tmpPrefString = "Caption";
  tmpDefault = QString("ShowEQ - ") + prefString;
  setCaption(pSEQPrefs->getPrefString(tmpPrefString, prefString, tmpDefault));

  tmpPrefString = "CacheChanges";
  m_cacheChanges = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "AnimateSpawnMovement";
  m_animate = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "VelocityLines";
  m_showVelocityLines = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "SpawnDepthFilter";
  m_spawnDepthFilter = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "Framerate";
  m_frameRate = pSEQPrefs->getPrefInt(tmpPrefString, prefString, 5);

#ifdef DEBUG
  tmpPrefString = "ShowDebugInfo";
  m_showDebugInfo = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);
#endif

  tmpPrefString = "ShowPlayer";
  m_showPlayer = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowPlayerBackground";
  m_showPlayerBackground = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowPlayerView";
  m_showPlayerView = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowDroppedItems";
  m_showDrops = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowDoors";
  m_showDoors = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "ShowSpawns";
  m_showSpawns = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowSpawnPoints";
  m_showSpawnPoints = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowUnknownSpawns";
  m_showUnknownSpawns = pSEQPrefs->getPrefBool(tmpPrefString, prefString, 
                           showeq_params->createUnknownSpawns);

  tmpPrefString = "HighlightConsideredSpawns";
  m_highlightConsideredSpawns = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "ShowTooltips";
  m_showTooltips = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  tmpPrefString = "WalkPathShowSelect";
  m_walkpathshowselect = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "ShowFiltered";
  m_showFiltered = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);


  tmpPrefString = "FOVMode";
  m_fovMode = (FOVMode)pSEQPrefs->getPrefInt(tmpPrefString, prefString, 
                         tFOVDistanceBased);

  tmpPrefString = "FOVStyle";
  m_fovStyle = pSEQPrefs->getPrefInt(tmpPrefString, prefString, QBrush::Dense7Pattern);

  tmpPrefString = "FOVColor";
  m_fovColor = pSEQPrefs->getPrefColor(tmpPrefString, prefString, QColor("#505050"));

  // mainly for backwards compatibility
  tmpPrefString = "MapDepthFilter";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, false))
    m_param.setMapLineStyle(tMap_DepthFiltered);

  tmpPrefString = "FadingFloors";
  if (pSEQPrefs->getPrefBool(tmpPrefString, prefString, false))
    m_param.setMapLineStyle(tMap_FadedFloors);

  // the new setting overrides old settings
  tmpPrefString = "MapLineStyle";
  if (pSEQPrefs->isPreference(tmpPrefString, prefString))
    m_param.setMapLineStyle((MapLineStyle)pSEQPrefs->getPrefInt(tmpPrefString, prefString,
                                tMap_Normal));

  tmpPrefString = "ShowMapPoints";
  m_param.setShowLocations(pSEQPrefs->getPrefBool(tmpPrefString, prefString, true));

  tmpPrefString = "ShowMapLines";
  m_param.setShowLines(pSEQPrefs->getPrefBool(tmpPrefString, prefString, true));

  tmpPrefString = "ShowGridLines";
  m_param.setShowGridLines(pSEQPrefs->getPrefBool(tmpPrefString, prefString, true));

  tmpPrefString = "ShowGridTicks";
  m_param.setShowGridTicks(pSEQPrefs->getPrefBool(tmpPrefString, prefString, true));

  tmpPrefString = "ShowBackgroundImage";
  m_param.setShowBackgroundImage(pSEQPrefs->getPrefBool(tmpPrefString, prefString, true));

  tmpPrefString = "GridResolution";
  m_param.setGridResolution(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 500));

  tmpPrefString = "Font";
  m_param.setFont(pSEQPrefs->getPrefFont(tmpPrefString, prefString));

  tmpPrefString = "GridTickColor";
  m_param.setGridTickColor(pSEQPrefs->getPrefColor(tmpPrefString, prefString, QColor("#E1C819")));

  tmpPrefString = "GridLineColor";
  m_param.setGridLineColor(pSEQPrefs->getPrefColor(tmpPrefString, prefString, QColor("#194819")));

  tmpPrefString = "BackgroundColor";
  m_param.setBackgroundColor(pSEQPrefs->getPrefColor(tmpPrefString, prefString, QColor("black")));

  tmpPrefString = "HeadRoom";
  m_param.setHeadRoom(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 75));

  tmpPrefString = "FloorRoom";
  m_param.setFloorRoom(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 75));

  tmpPrefString = "OptimizeMethod";
  m_param.setMapOptimizationMethod((MapOptimizationMethod)pSEQPrefs->getPrefInt(tmpPrefString, prefString, (int)tMap_NormalOptim));

  tmpPrefString = "ZoomDefault";
  m_param.setZoomDefault(pSEQPrefs->getPrefInt(tmpPrefString, prefString, 1));

  tmpPrefString = "CacheAlwaysRepaint";
  m_mapCache.setAlwaysRepaint(pSEQPrefs->getPrefBool(tmpPrefString, prefString, false));

  tmpPrefString = "PvP";
  m_pvp = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "DeityPvP";
  m_deityPvP = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "RacePvP";
  m_racePvP = pSEQPrefs->getPrefBool(tmpPrefString, prefString, false);

  tmpPrefString = "ShowZoneSafePoint";
  m_showZoneSafePoint = pSEQPrefs->getPrefBool(tmpPrefString, prefString, true);

  // Accelerators
  QAccel *accel = new QAccel(this);
  int key;
  key = pSEQPrefs->getPrefKey("ZoomInKey", prefString, "+");
  accel->connectItem(accel->insertItem(key), this, SLOT(zoomIn()));

  key = pSEQPrefs->getPrefKey("ZoomOutKey", prefString, "-");
  accel->connectItem(accel->insertItem(key), this, SLOT(zoomOut()));

  key = pSEQPrefs->getPrefKey("PanDownLeftKey", prefString, "Ctrl+1");
  accel->connectItem(accel->insertItem(key), this, SLOT(panDownLeft()));

  key = pSEQPrefs->getPrefKey("PanDownKey", prefString, "Ctrl+2");
  accel->connectItem(accel->insertItem(key), this, SLOT(panDown()));

  key = pSEQPrefs->getPrefKey("PanDownRightKey", prefString, "Ctrl+3");
  accel->connectItem(accel->insertItem(key), this, SLOT(panDownRight()));

  key = pSEQPrefs->getPrefKey("PanLeftKey", prefString, "Ctrl+4");
  accel->connectItem(accel->insertItem(key), this, SLOT(panLeft()));

  key = pSEQPrefs->getPrefKey("CenterSelectedKey", prefString, "Ctrl+5");
  accel->connectItem(accel->insertItem(key), this, SLOT(viewTarget()));

  key = pSEQPrefs->getPrefKey("PanRightKey", prefString, "Ctrl+6");
  accel->connectItem(accel->insertItem(key), this, SLOT(panRight()));

  key = pSEQPrefs->getPrefKey("PanUpLeftKey", prefString, "Ctrl+7");
  accel->connectItem(accel->insertItem(key), this, SLOT(panUpLeft()));

  key = pSEQPrefs->getPrefKey("PanUpKey", prefString, "Ctrl+8");
  accel->connectItem(accel->insertItem(key), this, SLOT(panUp()));

  key = pSEQPrefs->getPrefKey("PanUpRightKey", prefString, "Ctrl+9");
  accel->connectItem(accel->insertItem(key), this, SLOT(panUpRight()));

  key = pSEQPrefs->getPrefKey("ViewLockKey", prefString, "Ctrl+0");
  accel->connectItem(accel->insertItem(key), this, SLOT(viewLock()));

  m_followMode = tFollowPlayer;
  
  m_selectedItem = NULL;

  setMinimumSize(100, 100);

#ifdef DEBUG
  for (int i = 0; i < maxFrameTimes; i++)
    m_frameTimes[i] = 0;
  m_frameTimeIndex = 0;
  m_paintCount = 0;
  m_paintTimeSum = 0;
#endif

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
  connect (m_spawnShell,SIGNAL(changeItem(const Item*, uint32_t)),
       this, SLOT(changeItem(const Item*, uint32_t)));

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
  m_mapIcons->save();
}

MapMenu* Map::menu()
{
  // return the existing menu if it exists
  if (m_menu != NULL)
    return m_menu;

  // create the menu
  m_menu = new MapMenu(this, this, "map menu");

  // make sure to use the applications font
  m_menu->setFont(QFont());

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
#ifdef DEBUGMAP
    FILE *f;
    f=fopen("/tmp/coords","at");
    if(f) 
    {
      fprintf (f,"%f, %f\n",
           (m_param.screenCenterX() - me->x()) * m_param.ratioX(),
           (m_param.screenCenterY() - me->y()) * m_param.ratioY());
      fclose(f);
    }
#endif /* DEBUGMAP */
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
    uint32_t dist = 15;
    // find the nearest spawn within a reasonable range
    closestSpawn = closestSpawnToPoint(me->pos(), dist);

    // check for closest spawn point
    const SpawnPoint* closestSP = NULL;
    if (m_showSpawnPoints)
      closestSP = closestSpawnPointToPoint(me->pos(), dist);

    // only get a spawn point if the user clicked closer to it then a
    // the closest spawn
    if (closestSP != NULL)
    {
      m_spawnMonitor->setSelected(closestSP);

      return;
    }

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
      if(!m_cacheChanges)
        refreshMap ();
    }
  }
}

void Map::zoomIn()
{
#ifdef DEBUGMAP
   debug("Map::zoomIn()");
#endif /* DEBUGMAP */
   if (m_player->id() != 1)
   {
     if (m_param.zoomIn())
     {
       emit zoomChanged(m_param.zoom());

       // requires ReAdjust
       reAdjust();

       if (!m_cacheChanges)
         refreshMap ();
     }
   }
}

void Map::zoomOut()
{
#ifdef DEBUGMAP
   debug("Map::zoomOut()");
#endif /* DEBUGMAP */

   if (m_player->id() != 1)
   {
     if (m_param.zoomOut())
     {
       emit zoomChanged(m_param.zoom());

       // requires ReAdjust
       reAdjust();

       if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
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

   if(!m_cacheChanges)
     refreshMap ();
}

void Map::increaseGridResolution (void)
{
  m_param.increaseGridResolution();

   if(!m_cacheChanges)
     refreshMap ();
}

void Map::decreaseGridResolution (void)
{
  m_param.decreaseGridResolution();

   if(!m_cacheChanges)
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
  
  if(!m_cacheChanges)
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

       m_param.setPan(location.x(), location.y());
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

       // retrieve the approximate current player position
       MapPoint targetPoint;
       m_player->approximatePosition(m_animate, QTime::currentTime(),
                   targetPoint);
       
       // set the current pan to it's position to avoid jarring the user
       m_param.setPan(targetPoint.x(), targetPoint.y());
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

   if(!m_cacheChanges)
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
       // retrieve the approximate current player position
       MapPoint targetPoint;
       m_player->approximatePosition(m_animate, QTime::currentTime(),
                   targetPoint);
       
       // set the current pan to it's position to avoid jarring the user
       m_param.setPan(targetPoint.x(), targetPoint.y());
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
         
         m_param.setPan(location.x(), location.y());
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
  
  if(!m_cacheChanges)
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

  pSEQPrefs->setPrefBool("ShowFiltered", preferenceName(), m_showFiltered);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setFrameRate(int val) 
{ 
  // make sure the value is within range
  if ((val >= 1) && (val <= 60))
  {
    m_frameRate = val;

    QString tmpPrefString = "Framerate";
    pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_frameRate);

    emit frameRateChanged(m_frameRate);

    if (m_timer->isActive())
      m_timer->changeInterval(1000/m_frameRate);
  }
}

void Map::setFOVStyle(int val)
{
  if ((val < Qt::NoBrush) || (val > Qt::DiagCrossPattern))
    return;

  m_fovStyle = val;

  QString tmpPrefString = "FOVStyle";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_fovStyle);
}

void Map::setFOVMode(FOVMode mode)
{
  if ((mode < tFOVDistanceBased) || (mode > tFOVClassic))
    return;

  m_fovMode = mode;

  QString tmpPrefString = "FOVMode";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_fovMode );
}

void Map::setFOVColor(const QColor& color)
{
  m_fovColor = color;

  // set color preference
  pSEQPrefs->setPrefColor("FOVColor", preferenceName(), m_fovColor);
}

void Map::setShowMapLines(bool val) 
{ 
  m_showMapLines = val; 
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowPlayer(bool val) 
{ 
  m_showPlayer = val; 
  
  QString tmpPrefString = "ShowPlayer";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showPlayer);

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowPlayerBackground(bool val) 
{ 
  m_showPlayerBackground = val; 
  
  QString tmpPrefString = "ShowPlayerBackground";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showPlayerBackground);

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowPlayerView(bool val) 
{ 
  m_showPlayerView = val; 

  QString tmpPrefString = "ShowPlayerView";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showPlayerView);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowHeading(bool val) 
{ 
  m_showHeading = val; 
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowSpawns(bool val) 
{ 
  m_showSpawns = val; 

  QString tmpPrefString = "ShowSpawns";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showSpawns);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowSpawnPoints(bool val) 
{ 
  m_showSpawnPoints = val; 

  QString tmpPrefString = "ShowSpawnPoints";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showSpawnPoints);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowUnknownSpawns(bool val) 
{ 
  m_showUnknownSpawns = val; 

  QString tmpPrefString = "ShowUnknownSpawns";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showUnknownSpawns);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowDrops(bool val) 
{ 
  m_showDrops = val; 

  QString tmpPrefString = "ShowDroppedItems";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showDrops);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowDoors(bool val) 
{ 
  m_showDoors = val; 

  QString tmpPrefString = "ShowDoors";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showDoors);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowVelocityLines(bool val) 
{ 
  m_showVelocityLines = val; 
  
  QString tmpPrefString = "VelocityLines";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showVelocityLines);

  if(!m_cacheChanges)
    refreshMap ();
}

#ifdef DEBUG
void Map::setShowDebugInfo(bool val) 
{ 
  m_showDebugInfo = val; 
  
  QString tmpPrefString = "ShowDebugInfo";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showDebugInfo);

  if(!m_cacheChanges)
    refreshMap ();
}
#endif

void Map::setAnimate(bool val) 
{ 
  m_animate = val; 

  QString tmpPrefString = "AnimateSpawnMovement";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_animate);

  // this requires a reAdjust
  reAdjust();

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setCacheChanges(bool val) 
{ 
  m_cacheChanges = val; 

  QString tmpPrefString = "CacheChanges";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_cacheChanges);

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setSpawnDepthFilter(bool val)
{
  m_spawnDepthFilter = val; 
  
  QString tmpPrefString = "SpawnDepthFilter";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_spawnDepthFilter);

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setHighlightConsideredSpawns(bool val) 
{ 
  m_highlightConsideredSpawns = val; 

  QString tmpPrefString = "HighlightConsideredSpawns";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_highlightConsideredSpawns);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowTooltips(bool val) 
{ 
  m_showTooltips = val; 

  QString tmpPrefString = "ShowTooltips";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_showTooltips);

  // make sure it's hidden if they hid it
  if (!m_showTooltips)
    m_mapTip->hide();
}

void Map::setWalkPathShowSelect(bool val) 
{ 
  m_walkpathshowselect = val; 

  QString tmpPrefString = "WalkPathShowSelect";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_walkpathshowselect);
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setPvP(bool val) 
{ 
  m_pvp = val; 

  pSEQPrefs->setPrefBool("PvP", preferenceName(), m_pvp);
  
  // Can only have one style of PvP on at once
  if (val && m_deityPvP)
  {
    m_deityPvP = false;
    pSEQPrefs->setPrefBool("DeityPvP", preferenceName(), m_deityPvP);
  }
  if (val && m_racePvP)
  {
    m_racePvP = false;
    pSEQPrefs->setPrefBool("RacePvP", preferenceName(), m_racePvP);
  }

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setDeityPvP(bool val) 
{ 
  m_deityPvP = val; 

  pSEQPrefs->setPrefBool("DeityPvP", preferenceName(), m_deityPvP);
  
  // Can only have one style of PvP on at once
  if (val && m_pvp)
  {
    m_pvp = false;
    pSEQPrefs->setPrefBool("PvP", preferenceName(), m_pvp);
  }
  if (val && m_racePvP)
  {
    m_racePvP = false;
    pSEQPrefs->setPrefBool("RacePvP", preferenceName(), m_racePvP);
  }

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setRacePvP(bool val) 
{ 
  m_racePvP = val; 
  
  pSEQPrefs->setPrefBool("RacePvP", preferenceName(), m_racePvP);

  if (val && m_pvp)
  {
    m_pvp = false;
    pSEQPrefs->setPrefBool("PvP", preferenceName(), m_pvp);
  }
  if (val && m_racePvP)
  {
    m_deityPvP = false;
    pSEQPrefs->setPrefBool("DeityPvP", preferenceName(), m_deityPvP);
  }

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setMapLineStyle(MapLineStyle style) 
{ 
  m_param.setMapLineStyle(style); 

  QString tmpPrefString = "MapLineStyle";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_param.mapLineStyle());
  
  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setMapOptimization(MapOptimizationMethod method) 
{ 
  // set the general optimization method
  m_param.setMapOptimizationMethod(method);

  QString tmpPrefString = "OptimizeMethod";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(),  
              (int)m_param.mapOptimizationMethod());

  // set the offscreen images optimization method
  m_offscreen.setOptimization(m_param.pixmapOptimizationMethod());
}

void Map::setZoom(int val) 
{ 
  if (m_player->id() != 1)
  {
    if (m_param.setZoom(val))
    {
      emit zoomChanged(m_param.zoom());

      // requires reAdjust
      reAdjust();
      
      if (!m_cacheChanges)
        refreshMap ();
    }
  }
}

void Map::setZoomDefault(int val) 
{ 
  if (val == m_param.zoomDefault())
    return;

  m_param.setZoomDefault(val);

  QString tmpPrefString = "ZoomDefault";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(),  
            m_param.zoomDefault());

  emit zoomDefaultChanged(val);
}

void Map::setPanOffsetX(int val) 
{ 
  m_param.setPanX(val); 

  // this requires a reAdjust
  reAdjust();

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setPanOffsetY(int val) 
{ 
  m_param.setPanY(val); 

  // this requires a reAdjust
  reAdjust();

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setGridResolution(int val) 
{ 
  m_param.setGridResolution(val); 

  QString tmpPrefString = "GridResolution";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_param.gridResolution());

  reAdjust();

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setGridTickColor(const QColor& color) 
{ 
  m_param.setGridTickColor(color); 

  // set color preference
  pSEQPrefs->setPrefColor("GridTickColor", preferenceName(), m_param.gridTickColor());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setGridLineColor(const QColor& color) 
{ 
  m_param.setGridLineColor(color); 

  // set color preference
  pSEQPrefs->setPrefColor("GridLineColor", preferenceName(), m_param.gridLineColor());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setBackgroundColor(const QColor& color) 
{ 
  m_param.setBackgroundColor(color); 

  // set color preference
  pSEQPrefs->setPrefColor("BackgroundColor", preferenceName(), m_param.backgroundColor());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setFont(const QFont& font) 
{ 
  m_param.setFont(font); 

  QString tmpPrefString = "Font";
  pSEQPrefs->setPrefFont(tmpPrefString, preferenceName(), m_param.font());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setHeadRoom(int val) 
{ 
  m_param.setHeadRoom(val); 

  QString tmpPrefString = "HeadRoom";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_param.headRoom());

  emit headRoomChanged(m_param.headRoom());

  reAdjust();

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setFloorRoom(int val) 
{ 
  m_param.setFloorRoom(val); 

  QString tmpPrefString = "FloorRoom";
  pSEQPrefs->setPrefInt(tmpPrefString, preferenceName(), m_param.floorRoom());

  emit floorRoomChanged(m_param.floorRoom());

  reAdjust();

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowBackgroundImage(bool val) 
{ 
  m_param.setShowBackgroundImage(val); 

  QString tmpPrefString = "ShowBackgroundImage";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_param.showBackgroundImage());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowLocations(bool val) 
{ 
  m_param.setShowLocations(val); 

  QString tmpPrefString = "ShowMapPoints";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_param.showLocations());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowLines(bool val) 
{ 
  m_param.setShowLines(val); 

  QString tmpPrefString = "ShowMapLines";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_param.showLines());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowGridLines(bool val) 
{ 
  m_param.setShowGridLines(val); 

  QString tmpPrefString = "ShowGridLines";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_param.showGridLines());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setShowGridTicks(bool val) 
{ 
  m_param.setShowGridTicks(val); 

  QString tmpPrefString = "ShowGridTicks";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_param.showGridTicks());

  if(!m_cacheChanges)
    refreshMap ();
}

void Map::setCacheAlwaysRepaint(bool val) 
{ 
  m_mapCache.setAlwaysRepaint(val); 

  QString tmpPrefString = "CacheAlwaysRepaint";
  pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_mapCache.alwaysRepaint());
}

void Map::setShowZoneSafePoint(bool val)
{
  m_showZoneSafePoint = val;

  if(!m_cacheChanges)
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
  out << "ShowDoors: " << m_showDoors << endl;
  out << "ShowSpawns: " << m_showSpawns << endl;
  out << "ShowFiltered: " << m_showFiltered << endl;
  out << "HighlightConsideredSpawns: " << m_highlightConsideredSpawns << endl;
  out << "ShowTooltips: " << m_showTooltips << endl;
  out << "WalkPathShowSelect: " << m_walkpathshowselect << endl;
  out << "FOVStyle: " << m_fovStyle << endl;
  out << "FOVMode: " << m_fovMode << endl;
  out << "FOVColor: " << m_fovColor.name() << endl;

  out << endl;
  out << "[" << preferenceName() << " Parameters]" << endl;
  out << "MapLineStyle: " << m_param.mapLineStyle() << endl;
  out << "ShowMapPoints: " << m_param.showLocations() << endl;
  out << "ShowMapLines: " << m_param.showLines() << endl;
  out << "ShowGridLines: " << m_param.showGridLines() << endl;
  out << "ShowGridTicks: " << m_param.showGridTicks() << endl;
  out << "ShowBackgroundImage: " << m_param.showBackgroundImage() << endl;
  out << "GridResolution: " << m_param.gridResolution() << endl; 
  out << "GridTickColor: " << m_param.gridTickColor().name() << endl;
  out << "GridLineColor: " << m_param.gridLineColor().name() << endl;
  out << "BackgroundColor: " << m_param.backgroundColor().name() << endl;
  out << "HeadRoom: " << m_param.headRoom() << endl;
  out << "FloorRoom: " << m_param.floorRoom() << endl;
  out << "OptimizeMethod: " << (int)m_param.mapOptimizationMethod() << endl;

  out << endl;
  m_mapIcons->dumpInfo(out);

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
  out << "scaledFOVDistance: " << m_scaledFOVDistance << endl;
  out << "playerHeadRoom: " << m_param.playerHeadRoom() << endl;
  out << "playerFloorRoom: " << m_param.playerFloorRoom() << endl;
  out << "FollowMode: " << m_followMode << endl;
  out << "MapPanning: " << m_mapPanning << endl;
  out << "PvP: " << m_pvp << endl;
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

void Map::showMapIconDialog()
{
  if (!m_mapIconDialog)
  {
    // first create the dialog
    m_mapIconDialog = new MapIconDialog(this, caption() + " Icon Dialog",
                    false);
    
    // then pass it this objects map icons object
    m_mapIconDialog->setMapIcons(m_mapIcons);
  }

  // show the dialog
  m_mapIconDialog->show();
}

void Map::resizeEvent (QResizeEvent *qs)
{
#ifdef DEBUGMAP
   debug ("resizeEvent()");
#endif /* DEBUGMAP */
   m_param.setScreenSize(qs->size());

   m_offscreen.resize(m_param.screenLength());

   reAdjust();

   QWidget::resizeEvent(qs);
}

void Map::reAdjustAndRefreshMap(void)
{
  // first,, readjust the map state
  reAdjust();

  // then, repaint the map
   repaint(mapRect(), FALSE);
}

void Map::refreshMap(void)
{
#ifdef DEBUGMAP
   debug ("refreshMap()");
#endif /* DEBUGMAP */
   repaint(mapRect(), FALSE);
}

void Map::reAdjust()
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
    // retrieve the approximate current player position
    MapPoint targetPoint;
    m_player->approximatePosition(m_animate, QTime::currentTime(), 
                  targetPoint);

    // adjust around players location
    m_param.reAdjust(&targetPoint);
  }
  break;
  case tFollowNone:
    m_param.reAdjust(NULL);
    break;
  }

  switch (m_fovMode)
  {
  case tFOVDistanceBased:
    // scaled FOV Distance (m_fovDistance * scale)
    m_scaledFOVDistance = fixPtMulII(m_param.ratioIFixPt(), 
                     MapParameters::qFormat,
                     m_mapIcons->fovDistance());
    break;
  case tFOVScaledClassic:
    m_scaledFOVDistance = m_mapIcons->fovDistance() * m_param.zoom();
    break;
  case tFOVClassic:
    m_scaledFOVDistance = m_mapIcons->fovDistance();
    break;
  }
}

void Map::addLocation(void)
{
#ifdef DEBUGMAP
  debug ("addLocation()");
#endif /* DEBUGMAP */

  // get it's approximage location
  MapPoint point;
  m_player->approximatePosition(m_animate, QTime::currentTime(), point);

#ifdef DEBUGMAP
  seqDebug("addLocation() point(%d, %d, %d)", point.x(), point.y(), point.z());
#endif

  // add the location
  m_mapCache.forceRepaint();
  m_mapMgr->addLocation(this, point);
}

void Map::startLine (void)
{
#ifdef DEBUGMAP
  debug ("startLine()");
#endif /* DEBUGMAP */
  // get it's approximate position
  MapPoint point;
  m_player->approximatePosition(m_animate, QTime::currentTime(), point);

#ifdef DEBUGMAP
  seqDebug("startLine() point(%d, %d, %d)", point.x(), point.y(), point.z());
#endif

  // start the line using the player spawns position
  m_mapMgr->startLine(point);
}

void Map::addLinePoint() 
{
#ifdef DEBUGMAP
  debug ("addLinePoint()");
#endif /* DEBUGMAP */

  // get the player spawns approximate position
  MapPoint point;
  m_player->approximatePosition(m_animate, QTime::currentTime(), point);


#ifdef DEBUGMAP
  seqDebug("addLinePoint() point(%d, %d, %d)", point.x(), point.y(), point.z());
#endif

  // add it as the next line point
  m_mapCache.forceRepaint();
  m_mapMgr->addLinePoint(point);
}


void Map::delLinePoint(void)
{
#ifdef DEBUGMAP
  debug ("delLinePoint()");
#endif /* DEBUGMAP */

  m_mapCache.forceRepaint();
  m_mapMgr->delLinePoint();
} // END delLinePoint


void Map::showLineDlg(void)
{
  // show the line dialog
  m_mapMgr->showLineDlg(this);
}

void Map::scaleDownZ(int id)
{
  m_mapCache.forceRepaint();
  m_mapMgr->scaleDownZ(m_menu->itemParameter(id));
}

void Map::scaleUpZ(int id)
{
  m_mapCache.forceRepaint();
  m_mapMgr->scaleUpZ(m_menu->itemParameter(id));
}

void Map::addPathPoint() 
{

  // get the player spawns approximate position
  MapPoint point;
  m_player->approximatePosition(m_animate, QTime::currentTime(), point);

#ifdef DEBUGMAP
  FILE *f;
  f=fopen("/tmp/coords","at");
  if(f) {
    fprintf (f,"%f, %f\n",
             (double)point.x(),
             (double)point.y());
    fclose(f);
  }
#endif
}

QRect Map::mapRect () const
{
#ifdef DEBUGMAP
   seqDebug("mapRect()");
   static int rendercount = 0;
   rendercount++;
   seqDebug("%i, (0,0,%i,%i)",rendercount, width (), height ());
#endif /* DEBUGMAP */
   QRect r (0, 0, width (), height ());
   r.moveBottomLeft (rect ().bottomLeft ());
#ifdef DEBUGMAP
   seqDebug("hmm2");
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

  EQPoint playerPos;

  // retrieve the approximate current player position, and set the 
  // parameters player position to it.
  m_player->approximatePosition(m_animate, drawTime, playerPos);
  m_param.setPlayer(playerPos);
  
  // make sure the player stays visible
  if ((m_param.zoom() > 1) &&
      ((m_followMode == tFollowPlayer) &&
       (!inRect(m_param.screenBounds(), 
        playerPos.x(), playerPos.y()))))
    reAdjust();

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
    
    if (!inRect(m_param.screenBounds(), playerPos.x(), playerPos.y()))
      reAdjust();
  }

  // copy the background
  const QPixmap& tmpPix = m_mapCache.getMapImage(m_param);
  bitBlt(&m_offscreen, 0, 0,
     &tmpPix, 0, 0, tmpPix.width(), tmpPix.height(),
     CopyROP, true);

  //Now, if we're animating, allow player to walk off. Grr, centering issue.

  /* Begin painting */
  tmp.begin (&m_offscreen);
  tmp.setPen (NoPen);
  tmp.setFont (m_param.font());

  if ((m_player->validPos()) ||
      ((!m_zoneMgr->isZoning()) && 
       (m_player->id() != 0)))
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

  if (m_showZoneSafePoint)
  {
    const Point3D<int16_t>& safePoint = m_zoneMgr->safePoint();
    m_mapIcons->paintIcon(m_param, tmp, 
              m_mapIcons->icon(tIconTypeZoneSafePoint),
              safePoint, QString("Safe Point"),
              QPoint(m_param.calcXOffsetI(safePoint.x()),
                 m_param.calcYOffsetI(safePoint.y())));
  }

  if (m_showDoors)
    paintDoors(m_param, tmp);

  if (m_showSpawnPoints)
    paintSpawnPoints(m_param, tmp);

  if (m_showSpawns)
    paintSpawns(m_param, tmp, drawTime);

  paintSelectedSpawnSpecials(m_param, tmp, drawTime);
  paintSelectedSpawnPointSpecials(m_param, tmp, drawTime);

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
  bitBlt(this, 0, 0,
     &m_offscreen, 0, 0, m_offscreen.width(), m_offscreen.height(),
     CopyROP, true);
}

void Map::paintPlayerBackground(MapParameters& param, QPainter& p)
{
  /* Paint player position background */
  p.setPen (m_fovColor);

  QBrush tmpBrush;
  tmpBrush.setColor(m_fovColor);
  tmpBrush.setStyle((Qt::BrushStyle)m_fovStyle);
  p.setBrush(tmpBrush);
  if(m_fovStyle == Qt::SolidPattern) p.setRasterOp(Qt::OrROP);

  // sizeWH is 2 * centerOffset
  int sizeWH = m_scaledFOVDistance << 1; 

  // FOV Distance 
  p.drawEllipse (m_param.playerXOffset() - m_scaledFOVDistance, 
         m_param.playerYOffset() - m_scaledFOVDistance, 
         sizeWH, sizeWH);
  
  if(m_fovStyle == Qt::SolidPattern) 
    p.setRasterOp(Qt::CopyROP);
  
}

void Map::paintPlayerView(MapParameters& param, QPainter& p)
{
  /* Paint the player direction */
#ifdef DEBUGMAP
  seqDebug("Paint the player direction");
#endif
  
  int const player_circle_radius = 4;
  
  int16_t playerAngle = m_player->headingDegrees();
  if (playerAngle != -1)
  {
    double const pi = 3.14159265358979323846;
    double const radians_per_circle = pi * 2;
    double const angle = (360 - playerAngle - 180) / 360.0 * radians_per_circle;
    int start_offset_x = int(sin( angle - radians_per_circle * 0.25 ) * player_circle_radius);
    int start_offset_y = int(cos( angle - radians_per_circle * 0.25 ) * player_circle_radius);
    double const fov_angle = radians_per_circle * 0.25;
    double fox_angle_offset = fov_angle / 2;
    
    p.setPen(yellow); // color
    p.drawLine( m_param.playerXOffset(), m_param.playerYOffset(),
        m_param.playerXOffset() + int (sin( angle ) * m_scaledFOVDistance),
        m_param.playerYOffset() + int (cos( angle ) * m_scaledFOVDistance) );
    
    p.setPen(red); // color
    for ( int n = 2; n--; )
    {
      int const start_x = m_param.playerXOffset() + start_offset_x;
      int const start_y = m_param.playerYOffset() + start_offset_y;
      
      p.drawLine( start_x, start_y,
          start_x + int (sin( angle - fox_angle_offset ) * m_scaledFOVDistance),
          start_y + int (cos( angle - fox_angle_offset ) * m_scaledFOVDistance) );
      start_offset_x *= -1;
      start_offset_y *= -1;
      fox_angle_offset *= -1;
    }
  }
}

void Map::paintPlayer(MapParameters& param, QPainter& p)
{
#ifdef DEBUGMAP
  seqDebug("Paint player position");
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
  seqDebug("Paint the dropped items");
#endif
  const ItemMap& itemMap = m_spawnShell->drops();
  ItemConstIterator it(itemMap);
  const Item* item;
  const QRect& screenBounds = m_param.screenBounds();
  MapIcon mapIcon;
  uint32_t filterFlags;
  uint8_t flag;

  // all drops are the same color
  p.setPen(yellow);

  /* Paint the dropped items */
  for (; it.current(); ++it)
  {
    // get the item from the list
    item = it.current();

    filterFlags = item->filterFlags();
 
    // make sure drop is within bounds
    if (!inRect(screenBounds, item->x(), item->y()) ||
        (m_spawnDepthFilter &&
         ((item->z() > m_param.playerHeadRoom()) ||
          (item->z() < m_param.playerFloorRoom()))) || 
        (!m_showFiltered && (filterFlags & FILTER_FLAG_FILTERED)))
      continue;

    mapIcon = m_mapIcons->icon(tIconTypeDrop);
    
    // only bother checking for specific flags if any are set...
    if (filterFlags != 0)
    {
      for (int i = 0; i < SIZEOF_FILTERS; i++)
      {
        flag = m_filterCheckOrdering[i];
        if (filterFlags & (1 << flag))
          mapIcon.combine(m_mapIcons->icon(tIconTypeFilterFlagBase + flag));
      }
    }
    
    // check runtime filter flags
    if(item->runtimeFilterFlags() & m_runtimeFilterFlagMask)
      mapIcon.combine(m_mapIcons->icon(tIconTypeRuntimeFiltered));
    
    // paint the icon
    m_mapIcons->paintItemIcon(param, p, mapIcon, item,
                  QPoint(param.calcXOffsetI(item->x()),
                     param.calcYOffsetI(item->y())));
  }
}

void Map::paintDoors(MapParameters& param,
             QPainter& p)
{
#ifdef DEBUGMAP
  seqDebug("Paint the door items");
#endif
  const ItemMap& itemMap = m_spawnShell->doors();
  ItemConstIterator it(itemMap);
  const Door* item;
  const QRect& screenBounds = m_param.screenBounds();
  MapIcon mapIcon;
  uint32_t filterFlags;
  uint8_t flag;

  // doors only come in one color
  p.setPen(QColor (110, 60, 0));

  /* Paint the door items */
  for (; it.current(); ++it)
  {
    // get the item from the list
    item = (const Door*)it.current();

    filterFlags = item->filterFlags();

    // make sure doors are within bounds
    if (!inRect(screenBounds, item->x(), item->y()) ||
        (m_spawnDepthFilter &&
         ((item->z() > m_param.playerHeadRoom()) ||
          (item->z() < m_param.playerFloorRoom()))) || 
        (!m_showFiltered && (filterFlags & FILTER_FLAG_FILTERED)))
      continue;

    mapIcon = m_mapIcons->icon(tIconTypeDoor);

    // add zone door effects
    if (item->zonePoint() != 0xFFFFFFFF)
      mapIcon.combine(m_mapIcons->icon(tIconTypeZoneDoor));

    // only bother checking for specific flags if any are set...
    if (filterFlags != 0)
    {
      for (int i = 0; i < SIZEOF_FILTERS; i++)
      {
        flag = m_filterCheckOrdering[i];
        if (filterFlags & (1 << flag))
          mapIcon.combine(m_mapIcons->icon(tIconTypeFilterFlagBase + flag));
      }
    }

    // check runtime filter flags
    if(item->runtimeFilterFlags() & m_runtimeFilterFlagMask)
      mapIcon.combine(m_mapIcons->icon(tIconTypeRuntimeFiltered));

    // paint the icon
    m_mapIcons->paintItemIcon(param, p, mapIcon, item, 
                  QPoint(param.calcXOffsetI(item->x()),
                     param.calcYOffsetI(item->y())));
  }
}             

const QColor& Map::raceTeamHighlightColor(const Spawn* spawn) const
{
  uint8_t playerLevel = m_player->level();
  int diff = spawn->level() - playerLevel;
  if (diff < -8)  //They are much easier than you.
    return green; 
  if (diff > 8)  //They are much harder than you.
    return darkRed; 

  if (diff < 0) 
    diff *= -1;
  
  // if we are within 8 levels of other player
  if (diff <= 8)
  {
    // they are in your range
    switch ( (spawn->level() - playerLevel) + 8)
    {
    // easy
    case 0:  // you are 8 above them
    case 1:  // you are 7 above them
      return green; 
      break;
    case 2:  // you are 6 above them
    case 3:  // you are 5 above them
      return darkGreen; 
      break;
      
    // moderate
    case 4:  // you are 4 above them
    case 5:  // you are 3 above them
      return blue; 
      break;
    case 6:  // you are 2 above them
    case 7:  // you are 1 above them
      return darkBlue; 
      break;
      
    // even
    case 8:  // you are even with them
      return white; 
      break;
        
    // difficult 
    case 9:  // you are 1 below them
    case 10:  // you are 2 below them
      return yellow; 
      break;
        
    // downright hard
    case 11:  // you are 3 below them
    case 12:  // you are 4 below them
      return magenta; 
      break;
    case 13:  // you are 5 below them
    case 14:  // you are 6 below them
      return red; 
      break;
    case 15:  // you are 7 below them
    case 16:  // you are 8 below them
      return darkRed; 
      break;
    }
  }
  
  return black;
}

const QColor& Map::deityTeamHighlightColor(const Spawn* spawn) const
{
  uint8_t playerLevel = m_player->level();
  int diff = spawn->level() - playerLevel;
  if (diff < -5)  //They are much easier than you.
    return green; 
  if (diff > 5)  //They are much harder than you.
    return darkRed; 

  if (diff < 0) 
    diff *= -1;

  // if we are within 8 levels of other player
  if (diff <= 5)
  {
    // they are in your range
    switch ( (spawn->level() - playerLevel) + 5)
    {
    // easy
    case 0:  // you are 5 above them
    case 1:  // you are 4 above them
      return green; 
      break;
    case 2:  // you are 3 above them
      return darkGreen; 
      break;
      
    // moderate
    case 3:  // you are 2 above them
      return blue; 
      break;
    case 4:  // you are 1 above them
      return darkBlue; 
      break;
      
    // even
    case 5:  // you are even with them
      return white; 
      break;
      
    // difficult 
    case 6:  // you are 1 below them
      return yellow; 
      break;
      
      // downright hard
    case 7:  // you are 2 below them
    case 8:  // you are 3 below them
      return magenta; 
      break;
    case 9:  // you are 4 below them
      return red; 
      break;
    case 10:  // you are 5 below them
      return darkRed; 
      break;
    }
  }

  return black;
}

void Map::paintSpawns(MapParameters& param,
              QPainter& p,
              const QTime& drawTime)
{
#ifdef DEBUGMAP
  seqDebug("Paint the spawns");
#endif
  const ItemMap& itemMap = m_spawnShell->spawns();
  ItemConstIterator it(itemMap);
  const Item* item;
  QPointArray  atri(3);
  QString spawnNameText;
  QFontMetrics fm(param.font());
  EQPoint spawnOffset;
  EQPoint location;
  QPen tmpPen;
  uint8_t flag;
  int spawnOffsetXPos, spawnOffsetYPos;
  uint16_t range;
  int scaledRange;
  int sizeWH;
  uint32_t filterFlags;
  const QRect& screenBounds = m_param.screenBounds();
  MapIcon mapIcon;
  bool up2date = false;

  /* Paint the spawns */
  const Spawn* spawn;
  // iterate over all spawns in of the current type
  while (it.current())
  {
    // get the item from the list
    item = it.current();

    // increment iterator to the next spawn
    ++it;

#ifdef DEBUGMAP
    spawn = spawnType(item);
    
    if (spawn == NULL)
    {
      seqWarn("Got non Spawn from iterator over type tSpawn (Tyep:%d ID: %d)!",
          item->type(), item->id());
      continue;
    }
#else
    // since only things of type Spawn should be in the list, 
    // just do a quicky conversion
    spawn = (const Spawn*)item;
#endif

    filterFlags = item->filterFlags();

    if (((m_spawnDepthFilter &&
         ((item->z() > m_param.playerHeadRoom()) ||
          (item->z() < m_param.playerFloorRoom()))) || 
        (!m_showFiltered && (filterFlags & FILTER_FLAG_FILTERED)) ||
        (!m_showUnknownSpawns && spawn->isUnknown())) &&
        (item != m_selectedItem))
      continue;
 
    // get the approximate position of the spawn
    up2date = spawn->approximatePosition(m_animate, drawTime, location);
    
    // check that the spawn is within the screen bounds
    if (!inRect(screenBounds, location.x(), location.y()))
      continue; // not in bounds, next...
    
    // calculate the spawn's offset location
    spawnOffsetXPos = m_param.calcXOffsetI(location.x());
    spawnOffsetYPos = m_param.calcYOffsetI(location.y());
    QPoint point(spawnOffsetXPos, spawnOffsetYPos);
    
    
    //--------------------------------------------------
#ifdef DEBUGMAP
    printf("Draw velocities\n");
#endif
    /* Draw velocities */
    if (m_showVelocityLines &&
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
      mapIcon = m_mapIcons->icon(tIconTypeSpawnNPC);
      
      // retrieve the spawns aggro range
      range = m_mapMgr->spawnAggroRange(spawn);
      
      // if aggro range is known (non-zero), draw the aggro range circle
      if (range != 0)
      {
        scaledRange = fixPtMulII(m_param.ratioIFixPt(), 
                     MapParameters::qFormat, range);
        sizeWH = scaledRange << 1;
        
        p.setBrush(NoBrush);
        p.setPen(red); 
        
        p.drawEllipse(spawnOffsetXPos - scaledRange, 
                  spawnOffsetYPos - scaledRange, 
                  sizeWH, 
                  sizeWH);
      }
    }
    else if (spawn->isOtherPlayer())
    {
      if (!up2date)
        mapIcon = m_mapIcons->icon(tIconTypeSpawnPlayerOld);
      else
        mapIcon = m_mapIcons->icon(tIconTypeSpawnPlayer);
    }
    else if (spawn->NPC() == SPAWN_NPC_CORPSE) // x for NPC corpse
      mapIcon = m_mapIcons->icon(tIconTypeSpawnNPCCorpse);
    else if (spawn->NPC() == SPAWN_PC_CORPSE) // x for PC corpse
      mapIcon = m_mapIcons->icon(tIconTypeSpawnPlayerCorpse);
    else if (spawn->isUnknown())
      mapIcon = m_mapIcons->icon(tIconTypeSpawnUnknown);
    
    // if the spawn was considered, note it.
    if (m_highlightConsideredSpawns && spawn->considered())
      mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnConsidered));
    
     // only bother checking for specific flags if any are set...
    if (filterFlags != 0)
    {
      for (int i = 0; i < SIZEOF_FILTERS; i++)
      {
        flag = m_filterCheckOrdering[i];
        if (filterFlags & (1 << flag))
          mapIcon.combine(m_mapIcons->icon(tIconTypeFilterFlagBase + flag));
      }
    }
    
    // check runtime filter flags
    if(spawn->runtimeFilterFlags() & m_runtimeFilterFlagMask)
      mapIcon.combine(m_mapIcons->icon(tIconTypeRuntimeFiltered));
     
    // if PvP is not enabled, don't try to do it, 
    // paint the current spawn and continue to the next
    if (! (m_racePvP || m_pvp || m_deityPvP))
    {
      m_mapIcons->paintSpawnIcon(param, p, mapIcon, spawn, location, point);
      continue;
    }
    
    //--------------------------------------------------
#ifdef DEBUGMAP
    printf("PvP handling\n");
#endif
    
    const Spawn* owner;
    
    if (spawn->petOwnerID() != 0)
      owner = spawnType(m_spawnShell->findID(tSpawn, spawn->petOwnerID()));
    else 
      owner = NULL;
    
    if (m_pvp)
    {
      // New Combined Zek rules can only PvP +/- 4 levels from you. So
      // if spawn isn't you...
      if (spawn->isOtherPlayer())
      {
        int levelDiff = m_player->level() - spawn->level();
        
        if (abs(levelDiff) < 5)
        {
          // Gank away!
          mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerPvPEnabled));

          QPen p2(mapIcon.highlightPen());
          
          if (levelDiff < 0)
          {
            p2.setColor(yellow);
          }
          else if (levelDiff > 0)
          {
            p2.setColor(blue);
          }
          else
          {
            p2.setColor(white);
          }
          mapIcon.setHighlightPen(p2);
        }
      }
      else
      {
        // Pet owned by someone who is pvp?
        if (owner != NULL)
        {
          int levelDiff = m_player->level() - owner->level();

          if (abs(levelDiff) < 5)
          {
            // Gank away! Add color circle.
            mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPetPvPEnabled));

            QPen p2(mapIcon.highlightPen());
            
            if (levelDiff < 0)
            {
              p2.setColor(yellow);
            }
            else if (levelDiff > 0)
            {
              p2.setColor(blue);
            }
            else
            {
              p2.setColor(white);
            }
            mapIcon.setHighlightPen(p2);
          }
        }
      }
    }
    else if (m_racePvP)
    {
      // if spawn is another pc, on a different team, and within 8 levels
      // highlight it flashing
      if (spawn->isOtherPlayer())
      {
        mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerTeamBase -1 +
                     spawn->raceTeam()));
     
        // if not the same team as us
        if (!m_player->isSameRaceTeam(spawn))
        {
          mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerTeamOtherRace));
          QPen p2(mapIcon.highlightPen());
          p2.setColor(raceTeamHighlightColor(spawn));
          mapIcon.setHighlightPen(p2);
        }
      } // if decorate pvp
      // circle around pvp pets
      else if ((owner != NULL) && !m_player->isSameRaceTeam(owner))
        mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerTeamOtherRacePet));
    } // end racePvp
    else if (m_deityPvP)
    {
      if (spawn->isOtherPlayer())
      {
        mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerTeamBase -1 + 
                         spawn->deityTeam()));
     
        // if not the same team as us
        if (!m_player->isSameDeityTeam(spawn))
        {
          mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerTeamOtherDeity));
          QPen p2(mapIcon.highlightPen());
          p2.setColor(deityTeamHighlightColor(spawn));
          mapIcon.setHighlightPen(p2);
        }
      } // if decorate pvp
     // circle around deity pvp pets
      else if ((owner != NULL) && !m_player->isSameDeityTeam(owner))
        mapIcon.combine(m_mapIcons->icon(tIconTypeSpawnPlayerTeamOtherDeityPet));
    } // end if deityPvP
    
    // paint the spawn icon
    m_mapIcons->paintSpawnIcon(param, p, mapIcon, spawn, location, point);
  } // end for spawns

  //----------------------------------------------------------------------
#ifdef DEBUGMAP
  seqDebug("Done drawing spawns");
#endif
}

void Map::paintSelectedSpawnSpecials(MapParameters& param, QPainter& p,
                     const QTime& drawTime)
{
  if (m_selectedItem == NULL)
    return;

#ifdef DEBUGMAP
  seqDebug("Draw the line of the selected spawn");
#endif
  EQPoint location;

  if (m_selectedItem->type() == tSpawn)
  {
    ((const Spawn*)m_selectedItem)->approximatePosition(m_animate, 
                            drawTime, 
                            location);
    m_mapIcons->paintSpawnIcon(param, p, m_mapIcons->icon(tIconTypeItemSelected), 
                  (Spawn*)m_selectedItem, location, 
                  QPoint(m_param.calcXOffsetI(location.x()), 
                     m_param.calcYOffsetI(location.y())));
  }
  else if (m_selectedItem->type() == tPlayer)
  {
    m_mapIcons->paintSpawnIcon(param, p, m_mapIcons->icon(tIconTypeItemSelected), 
                  (Spawn*)m_selectedItem, *m_selectedItem, 
                  QPoint(m_param.calcXOffsetI(m_selectedItem->x()), 
                     m_param.calcYOffsetI(m_selectedItem->y())));
  }
  else
    m_mapIcons->paintItemIcon(param, p, m_mapIcons->icon(tIconTypeItemSelected), 
                  m_selectedItem, 
                  QPoint(param.calcXOffsetI(m_selectedItem->x()),
                     param.calcYOffsetI(m_selectedItem->y())));
}

void Map::paintSelectedSpawnPointSpecials(MapParameters& param, 
                      QPainter& p,
                      const QTime& drawTime)
{
  const SpawnPoint* sp;

  // blue flashing square around selected spawn point
  sp = m_spawnMonitor->selected();

  // if no spawn point selected, or not showing a line to selected 
  // spawn point and not flashing, just return
  if (sp == NULL)
    return;

  m_mapIcons->paintSpawnPointIcon(m_param, p, 
                 m_mapIcons->icon(tIconTypeSpawnPointSelected), sp,
                 QPoint(param.calcXOffsetI(sp->x()),
                    param.calcYOffsetI(sp->y())));
}

void Map::paintSpawnPoints( MapParameters& param, QPainter& p )
{
  const QRect& screenBounds = m_param.screenBounds();
  
  // get the spawn point count
  long count = m_spawnMonitor->spawnPoints().count();
  
  if (!count )
    return;
  
  // get an iterator over the list of spawn points
  QAsciiDictIterator<SpawnPoint> it( m_spawnMonitor->spawnPoints() );
  const SpawnPoint* sp;

  const MapIcon& mapIcon = m_mapIcons->icon(tIconTypeSpawnPoint);

  // iterate over the list of spawn points
  while ((sp = it.current()))
  {
    ++it;

    // make sure spawn point is within bounds
    if (!inRect(screenBounds, sp->x(), sp->y()) ||
        (m_spawnDepthFilter &&
         ((sp->z() > m_param.playerHeadRoom()) ||
          (sp->z() < m_param.playerFloorRoom()))))
      continue;

    m_mapIcons->paintSpawnPointIcon(m_param, p, mapIcon, sp,
                   QPoint(param.calcXOffsetI(sp->x()),
                      param.calcYOffsetI(sp->y())));
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

    if(!m_cacheChanges)
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
  
  uint32_t dist = 5;
  // check for closest spawn
  const Item* item = closestSpawnToPoint(event->pos(), dist);

  // check for closest spawn point
  const SpawnPoint* sp = NULL;
  if (m_showSpawnPoints)
    sp = closestSpawnPointToPoint(event->pos(), dist);

  // spawn point was closer, display it's info
  if (sp != NULL)
  {
    QString remaining;
    if ( sp->diffTime() == 0 || sp->deathTime() == 0 )
      remaining = "\277 ?";  
    else
    {
      long secs = sp->secsLeft();
    
      if ( secs > 0 )
    remaining.sprintf( "%2ld:%02ld", secs / 60, secs % 60  );
      else
    remaining = "now"; 
    }

    // construct and set the spawned string
    QString spawned;
    QDateTime dateTime;
    dateTime.setTime_t(sp->spawnTime());
    QDate createDate = dateTime.date();

    // spawn time
    if ( createDate != QDate::currentDate() )
      spawned = createDate.dayName( createDate.dayOfWeek() ) + " ";
    
    spawned += dateTime.time().toString();
    
    QString string;
    string.sprintf("SpawnPoint: %s\n"
           "%.3s/Z: %5d/%5d/%5d\n"
           "Last: %s\n"
           "Spawned: %s\t Remaining: %s\t Count: %d", 
           (const char*)sp->name(),
           showeq_params->retarded_coords ? "Y/X" : "X/Y",
           showeq_params->retarded_coords ? sp->y() : sp->x(),
           showeq_params->retarded_coords ? sp->x() : sp->y(),
           sp->z(),
           (const char*)sp->last(), 
           (const char*)spawned,
           (const char*)remaining,
           sp->count());

    m_mapTip->setText( string  );
    QPoint popPoint = mapToGlobal(event->pos());
    m_mapTip->popup(QPoint(popPoint.x() + 15, popPoint.y() + 15));
  }
  else if (item != NULL)
  {
    QString string;

    const Spawn* spawn = 0;
    const Door* door = 0;
    if ((item->type() == tSpawn) || (item->type() == tPlayer))
      spawn = (const Spawn*)item;
    else if (item->type() == tDoors)
      door = (const Door*)item;

    if (spawn)
    {
      QString guild;
      if (spawn->guildID() < MAX_GUILDS)
      {
        if (!spawn->guildTag().isEmpty())
          guild.sprintf("<%s>", (const char*)spawn->guildTag());
        else
          guild = QString::number(spawn->guildID());
      }
      else
          guild = " ";

      QString hp;
      
      if (spawn->HP() <= 0)
        hp = "<= 0";
          else
        hp = QString::number(spawn->HP());

      QString lastName = spawn->lastName();
      if (!lastName.isEmpty())
      {
        // if the spawn isn't a player enclose the name in parenthesis
        if (!spawn->isPlayer())
        {
          lastName.prepend("(");
          lastName.append(") ");
        }
        else
          lastName.append(" ");
      }
      else 
        lastName = "";

      string.sprintf("%s %s%s\n"
             "Level: %2d\tHP: %s\t %.3s/Z: %5d/%5d/%5d\n"
             "Race: %s\t Class: %s", 
             (const char*)spawn->transformedName().utf8(),
             (const char*)lastName.utf8(),
             guild.latin1(),
             spawn->level(), (const char*)hp,
             showeq_params->retarded_coords ? "Y/X" : "X/Y",
             showeq_params->retarded_coords ? spawn->y() : spawn->x(),
             showeq_params->retarded_coords ? spawn->x() : spawn->y(),
             item->z(),
             (const char*)spawn->raceString(), 
             (const char*)spawn->classString());
      if (m_deityPvP)
        string += " Deity: " + spawn->deityName();

      if (spawn->isNPC())
        string += "\tType: " + spawn->typeString();
      else
           string += "\tGender: " + spawn->genderName();

      string += "\nEquipment: " + spawn->info();
    }
    else
    {
      string.sprintf("%s\n"
             "%.3s/Z: %5d/%5d/%5d\n"
             "Race: %s\t Class: %s", 
             (const char*)item->transformedName().utf8(),
             showeq_params->retarded_coords ? "Y/X" : "X/Y",
             showeq_params->retarded_coords ? item->y() : item->x(),
             showeq_params->retarded_coords ? item->x() : item->y(),
             item->z(),
             (const char*)item->raceString(), 
             (const char*)item->classString());

      if ((door) && (door->zonePoint() != 0xFFFFFFFF))
      {
        const zonePointStruct* zp = m_zoneMgr->zonePoint(door->zonePoint());
        if (zp)
        {
          QString doorInfo("\nDestination Zone: %1 (%2/%3/%4 - %5)");
          if (showeq_params->retarded_coords)
            string += doorInfo.arg(m_zoneMgr->zoneNameFromID(zp->zoneId))
              .arg(zp->y).arg(zp->x).arg(zp->z).arg(zp->heading);
          else
            string += doorInfo.arg(m_zoneMgr->zoneNameFromID(zp->zoneId))
              .arg(zp->x).arg(zp->y).arg(zp->z).arg(zp->heading);
        }
      }
    }

    m_mapTip->setText( string  );
    QPoint popPoint = mapToGlobal(event->pos());
    m_mapTip->popup(QPoint(popPoint.x() + 15, popPoint.y() + 15));
  }
  else
    m_mapTip->hide();
} 

void Map::selectSpawn(const Item* item)
{
  // sanity check
  if (item == NULL)
    return;
  
  /* seqDebug("%s", item->ID()); */
  m_selectedItem = item;
  
  // if following the selected spawn, call reAdjust to focus on the new one
  if (m_followMode == tFollowSpawn)
    reAdjust();

  if(!m_cacheChanges)
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

  if(!m_cacheChanges)
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
      if (item == (const Item*)m_player)
        reAdjust();
    }
  }
}

const Item* Map::closestSpawnToPoint(const QPoint& pt, 
                     uint32_t& closestDistance) const
{
  const Item* closestItem = NULL;

  uint32_t distance;
  EQPoint location;
  EQPoint testPoint;

  const Item* item;
  spawnItemType itemTypes[] = { tSpawn, tDrop, tDoors, tPlayer };
  const bool* showType[] = { &m_showSpawns, &m_showDrops, 
                 &m_showDoors, &m_showPlayer };
  
  for (uint8_t i = 0; i < (sizeof(itemTypes) / sizeof(spawnItemType)); i++)
  {
    if (!*showType[i])
      continue;

    const ItemMap& itemMap = m_spawnShell->getConstMap(itemTypes[i]);
    ItemConstIterator it(itemMap);

    // iterate over all spawns in of the current type
    for (; it.current(); ++it)
    {
      // get the item from the list
      item = it.current();
    
      if (m_spawnDepthFilter &&
          ((item->z() > m_param.playerHeadRoom()) ||
           (item->z() < m_param.playerFloorRoom())))
        continue;

      if (!m_showFiltered && (item->filterFlags() & FILTER_FLAG_FILTERED))
        continue;

      if ((item->type() == tSpawn) || (item->type() == tPlayer))
      {
        if (!m_showUnknownSpawns && ((const Spawn*)item)->isUnknown())
          continue;

        ((const Spawn*)item)->approximatePosition(m_animate, 
                              QTime::currentTime(), 
                              location);

        testPoint.setPoint(m_param.calcXOffsetI(location.x()), 
                   m_param.calcYOffsetI(location.y()), 0);
      }
      else
        testPoint.setPoint(m_param.calcXOffsetI(item->x()), 
                   m_param.calcYOffsetI(item->y()), 0);

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

const SpawnPoint* Map::closestSpawnPointToPoint(const QPoint& pt, 
                        uint32_t& closestDistance) const
{
  const SpawnPoint* closestSP = NULL;

  uint32_t distance;
  EQPoint testPoint;

  QAsciiDictIterator<SpawnPoint> it(m_spawnMonitor->spawnPoints());
  SpawnPoint* sp;

  while ((sp = it.current()))
  {
    ++it;

    if (m_spawnDepthFilter &&
        ((sp->z() > m_param.playerHeadRoom()) ||
         (sp->z() < m_param.playerFloorRoom())))
      continue;

    testPoint.setPoint(m_param.calcXOffsetI(sp->x()), 
               m_param.calcYOffsetI(sp->y()), 0);

    distance = testPoint.calcDist2DInt(pt);

    if (distance < closestDistance)
    {
      closestDistance = distance;
      closestSP = sp;
    }
  }

  return closestSP;
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
  
  if (!m_cacheChanges)
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
  
  if(!m_cacheChanges)
    refreshMap();
}


void Map::saveMapImage(void)
{
  QStrList formats(QImageIO::outputFormats());
  QString filters;
  for (char* tmp =formats.first(); tmp != 0; tmp = formats.next())
    filters += QString(tmp) + QString(" (*.") + QString(tmp) + ")\n";
  
  QFileDialog fileDlg(QString::null, filters, this, "saveMapImage", true);
  fileDlg.setCaption("Save Map Image Filename");
  fileDlg.setMode(QFileDialog::AnyFile);

  if (fileDlg.exec() != QDialog::Accepted)
    return;

  QString filter = fileDlg.selectedFilter();
  QString filename = fileDlg.selectedFile();

  if (!filename.isEmpty())
    m_offscreen.save(filename, 
             filter.left(filter.find(' ')));
}

//----------------------------------------------------------------------
// MapFrame
MapFrame::MapFrame(FilterMgr* filterMgr,
           MapMgr* mapMgr,
           Player* player, 
           SpawnShell* spawnshell,
           ZoneMgr* zoneMgr,
           SpawnMonitor* spawnMonitor,
           const QString& prefName, 
           const QString& defCaption,
           const char* mapName,
           QWidget* parent, const char* name)
  : SEQWindow(prefName + "Frame", defCaption, parent, name),
    m_mapPreferenceName(prefName)
{
  m_filterMgr = filterMgr;
  
  QString prefString = MapFrame::preferenceName();
  QString tmpPrefString;

  QLabel* tmpLabel;

  // setup the vertical box
  m_vertical = new QVBoxLayout(boxLayout());

  // setup the top control window
  m_topControlBox = new QHBox(this);
  m_vertical->addWidget(m_topControlBox);
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
  m_map = new Map(mapMgr, player, spawnshell, zoneMgr, spawnMonitor,
          m_mapPreferenceName, m_runtimeFilterFlagMask, 
          this, mapName);
  m_vertical->addWidget(m_map);

  // setup bottom control window
  m_bottomControlBox = new QHBox(this);
  m_vertical->addWidget(m_bottomControlBox);
  m_bottomControlBox->setSpacing(1);
  m_bottomControlBox->setMargin(0);
  tmpPrefString = "ShowBottomControlBox";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, 1))
    m_bottomControlBox->hide();

  
  // setup Zoom control
  m_zoomBox = new QHBox(m_topControlBox);
  tmpLabel = new QLabel(m_zoomBox);
  tmpLabel->setText("Zoom:");
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
  m_playerLocation = new QLabel(m_playerLocationBox);
  m_playerLocation->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  m_playerLocation->setText("0      0      0      ");
  m_playerLocation->setMinimumWidth(90);
  tmpLabel->setBuddy(m_playerLocation);
  tmpPrefString = "ShowPlayerLocation";
  if (!pSEQPrefs->getPrefBool(tmpPrefString, prefString, false))
    m_playerLocationBox->hide();
  connect (player, SIGNAL(posChanged(int16_t,int16_t,int16_t,
                    int16_t,int16_t,int16_t,int32_t)), 
       this, SLOT(setPlayer(int16_t,int16_t,int16_t,
                int16_t,int16_t,int16_t,int32_t)));

  // setup Mouse Location display
  m_mouseLocationBox = new QHBox(m_topControlBox);
  tmpLabel = new QLabel(m_mouseLocationBox);
  tmpLabel->setText("Cursor:");
  m_mouseLocation = new QLabel(m_mouseLocationBox);
  m_mouseLocation->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  m_mouseLocation->setText("0      0      ");
  m_mouseLocation->setMinimumWidth(70);
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
  m_panX = new QSpinBox(-8192, 8192, 16, m_panBox);
  m_panX->setValue(m_map->panOffsetX());
  tmpLabel = new QLabel(m_panBox);
  tmpLabel->setText("Y:");
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
  m_head = new QSpinBox(5, 3000, 10, m_depthControlBox);
  m_head->setValue(m_map->headRoom());
  tmpLabel = new QLabel(m_depthControlBox);
  tmpLabel->setText("Floor:");
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

  mapMenu->insertItem("Status Font...",
              this, 
              SLOT(set_font(int)));

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

QPopupMenu* MapFrame::menu()
{
  return m_map->menu();
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
    
  //seqDebug("New Filter: %s", (const char*)str);

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

void MapFrame::mouseLocation(int16_t x, int16_t y)
{
  QString cursorPos;
  cursorPos.sprintf(" %+5hd, %+5hd", y, x);
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
  SEQWindow::savePrefs();

  // make the map belonging to this frame save it's preferences
  if (m_map)
    m_map->savePrefs();
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

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowStatusBox";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_topControlBox->isVisible());
  }
}

void MapFrame::toggle_bottom_controls(int id)
{
  if (m_bottomControlBox->isVisible())
    m_bottomControlBox->hide();
  else
    m_bottomControlBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowControlBox";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_bottomControlBox->isVisible());
  }
}

void MapFrame::set_font(int id)
{
  QString name = caption() + " Font";
  bool ok = false;

  // setup a default new status font
  QFont newFont;
  newFont.setPointSize(8);

  // get new status font
  newFont = QFontDialog::getFont(&ok, 
                 font(),
                 this, name);

  // if the user clicked ok and selected a valid font, set it
  if (ok)
    setWindowFont(newFont);
}

void MapFrame::toggle_zoom(int id)
{
  if (m_zoomBox->isVisible())
    m_zoomBox->hide();
  else
    m_zoomBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowZoom";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_zoomBox->isVisible());
  }
}

void MapFrame::toggle_playerLocation(int id)
{
  if (m_playerLocationBox->isVisible())
    m_playerLocationBox->hide();
  else
    m_playerLocationBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowPlayerLocation";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_playerLocationBox->isVisible());
  }
}

void MapFrame::toggle_mouseLocation(int id)
{
  if (m_mouseLocationBox->isVisible())
    m_mouseLocationBox->hide();
  else
    m_mouseLocationBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowMouseLocation";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_mouseLocationBox->isVisible());
  }
}

void MapFrame::toggle_filter(int id)
{
  if (m_filterBox->isVisible())
    m_filterBox->hide();
  else
    m_filterBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowFilter";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_filterBox->isVisible());
  }
}

void MapFrame::toggle_frameRate(int id)
{
  if (m_frameRateBox->isVisible())
    m_frameRateBox->hide();
  else
    m_frameRateBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowFrameRate";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_frameRateBox->isVisible());
  }
}

void MapFrame::toggle_pan(int id)
{
  if (m_panBox->isVisible())
    m_panBox->hide();
  else
    m_panBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowPanControls";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_panBox->isVisible());
  }
}

void MapFrame::toggle_depthControls(int id)
{
  if (m_depthControlBox->isVisible())
    m_depthControlBox->hide();
  else
    m_depthControlBox->show();

  QString tmpPrefString = "SaveControls";
  if (pSEQPrefs->getPrefBool(tmpPrefString, preferenceName(), true))
  {
    tmpPrefString = "ShowDepthFilterControls";
    pSEQPrefs->setPrefBool(tmpPrefString, preferenceName(), m_depthControlBox->isVisible()); 
 }
}


#include "map.moc"
