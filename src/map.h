/*
 * map.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQMAP_H
#define EQMAP_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qmap.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qregexp.h>
#include <qintdict.h>
#include <qtextstream.h>

// includes required for MapMenu
#include <qpopupmenu.h>

// includes required for MapFrame
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "everquest.h"
#include "spawnshell.h"
#include "filtermgr.h"
#include "player.h"
#include "mapcore.h"

//----------------------------------------------------------------------
// forward declarations
class CLineDlg;
class MapLabel;
class MapMgr;
class Map;
class MapFilterLineEdit;
class MapFrame;

//----------------------------------------------------------------------
// enumerated types
enum FollowMode
{ 
  tFollowPlayer = 0,
  tFollowSpawn = 1,
  tFollowNone = 2,
};

//----------------------------------------------------------------------
// constants
const int maxFrameTimes = 40;


//----------------------------------------------------------------------
// CLineDlg
class CLineDlg : public QDialog
{
   Q_OBJECT
 public:
   CLineDlg(QWidget* parent, QString name, MapMgr* mapMgr);

   QComboBox *m_LineColor;
   QLineEdit *m_LineName;
   QFrame    *m_ColorPreview;
 public slots:
   void changeColor(const QString &);
};

//----------------------------------------------------------------------
// MapLabel
class MapLabel: public QLabel
{
 public:
  MapLabel( Map* map );

 protected:

  Map*	m_Map;
};

//----------------------------------------------------------------------
// MapMgr
class MapMgr : public QObject
{
   Q_OBJECT

 public:
   MapMgr(SpawnShell* spawnShell, EQPlayer* player, QWidget* dialogParent);
   virtual ~MapMgr();
   
   const MapData& mapData() { return  m_mapData; }
   uint16_t spawnAggroRange(const Spawn* spawn);

  const QString& curLineColor() { return m_curLineColor; }
  const QString& curLineName() { return m_curLineName; }

  uint8_t getZEM(void) { return m_mapData.zoneZEM(); }
  void setZEM(uint8_t newZEM) { m_mapData.setZoneZEM(newZEM); }

 public slots:
  // Zone Handling
  void zoneEntry(const ServerZoneEntryStruct* zsentry);
  void zoneChange(const zoneChangeStruct* zoneChange, bool client);
  void zoneNew(const newZoneStruct* zoneNew, bool client);

   // Map Handling
  void loadMap(void);
  void loadFileMap(const QString& fileName);
  void saveMap(void);

  // Spawn Handling
  void addItem(const Item* item);
  void delItem(const Item* item);
  void killSpawn(const Item* item);
  void changeItem(const Item* item, uint32_t changeType);
  void clearItems(void);

  // Map Editing
  void addLocation(const MapPoint& point);
  void startLine(const MapPoint& point);
  void addLinePoint(const MapPoint& point);
  void delLinePoint(void);

  // Map Editing control
  void setLineName(const QString &);
  void setLineColor(const QString &);
  void showLineDlg();

  // Preference handling
  void savePrefs(void);

  // dump debug info
  void dumpInfo(QTextStream& out);

 signals:
  void mapLoaded(void);
  void mapUnloaded(void);
  void mapUpdated(void);

 private:
  SpawnShell* m_spawnShell;
  EQPlayer* m_player;
  QWidget* m_dialogParent;
  CLineDlg *m_dlgLineProps;
  MapData m_mapData;
  QIntDict<uint16_t> m_spawnAggroRange;
   
  QString m_curLineColor;
  QString m_curLineName;
  QString m_curLocationColor;
};

inline 
uint16_t MapMgr::spawnAggroRange(const Spawn* spawn) 
{ 
  uint16_t* range = m_spawnAggroRange.find(spawn->id()); 
  return (!range) ? 0 : *range;
}

//----------------------------------------------------------------------
// MapMenu
class MapMenu : public QPopupMenu
{
  Q_OBJECT

 public:
  MapMenu(Map* map, QWidget* parent = 0, const char* name = 0);
  virtual ~MapMenu();

 protected slots:
  void init_Menu(void);

  void select_follow(int itemId);
  void select_mapLine(int itemId);

  void toggle_spawnDepthFilter(int itemId);
  void toggle_tooltip(int itemId);
  void toggle_filtered(int itemId);
  void toggle_map(int itemId);
  void toggle_velocity(int itemId);
  void toggle_animate(int itemId);
  void toggle_player(int itemId);
  void toggle_playerBackground(int itemId);
  void toggle_playerView(int itemId);
  void toggle_gridLines(int itemId);
  void toggle_gridTicks(int itemId);
  void toggle_locations(int itemId);
  void toggle_spawns(int itemId);
  void toggle_drops(int itemId);
  void toggle_coins(int itemId);
  void toggle_highlightConsideredSpawns(int itemId);
  void toggle_spawnNames(int itemId);
  void toggle_mapImage(int itemId);
  void toggle_walkPath(int itemId);
#ifdef DEBUG
  void toggle_debugInfo(int itemId);
#endif
  void select_gridTickColor(int itemId);
  void select_gridLineColor(int itemId);
  void select_backgroundColor(int itemId);
  void select_font(int itemId);

 protected:
  // pointer to the Map this menu controls
  Map* m_map;

  int m_id_followMenu;
  int m_id_followMenu_Player;
  int m_id_followMenu_Spawn;
  int m_id_followMenu_None;
  int m_id_addLocation;
  int m_id_startLine;
  int m_id_addLinePoint;
  int m_id_delLinePoint;
  int m_id_showLineDlg;
  int m_id_editMap;
  int m_id_mapLineStyle;
  int m_id_mapLineStyle_Normal;
  int m_id_mapLineStyle_DepthFiltered;
  int m_id_mapLineStyle_FadedFloors;
  int m_id_spawnDepthFilter;
  int m_id_tooltip;
  int m_id_filtered;
  int m_id_map;
  int m_id_velocity;
  int m_id_animate;
  int m_id_player;
  int m_id_playerBackground;
  int m_id_playerView;
  int m_id_gridLines;
  int m_id_gridTicks;
  int m_id_locations;
  int m_id_spawns;
  int m_id_drops;
  int m_id_coins;
  int m_id_spawnNames;
  int m_id_highlightConsideredSpawns;
  int m_id_walkPath;
  int m_id_mapImage;
#ifdef DEBUG
  int m_id_debugInfo;
#endif
  int m_id_mapOptimization;
  int m_id_mapOptimization_Memory;
  int m_id_mapOptimization_Normal;
  int m_id_mapOptimization_Best;
  int m_id_gridTickColor;
  int m_id_gridLineColor;
  int m_id_backgroundColor;
  int m_id_font;
};

//----------------------------------------------------------------------
// Map
class Map :public QWidget
{
  Q_OBJECT

 public:
  Map (MapMgr* m_mapMgr,
       EQPlayer* player, SpawnShell* spawnshell, 
       const QString& preferenceName, uint32_t runtimeFilterFlagMask,
       QWidget * parent = 0, const char *name = "map");
  virtual ~Map(void);
  
  QSize sizeHint() const; // preferred size
  QSize minimumSizeHint() const; // minimum size
  QSizePolicy sizePolicy() const; // size policy
  QRect getRect()         { return rect(); }
  MapMgr* mapMgr() const { return m_mapMgr; }

  unsigned char getZEM (void);
  void          setZEM (unsigned char newZEM);
  
  // old methods
  MapOptimizationMethod getMapOptimization(void) 
    { return m_param.mapOptimizationMethod(); }
  
  MapMenu* menu();
  
  // get methods
  const QString& preferenceName() { return m_preferenceName; }
  const Item* selectedItem() { return m_selectedItem; }
  FollowMode followMode() const { return m_followMode; }
  int frameRate() const { return m_frameRate; }
  int drawSize() const { return m_drawSize; }
  bool showPlayer() const { return m_showPlayer; }
  bool showPlayerBackground() const { return m_showPlayerBackground; }
  bool showPlayerView() const { return m_showPlayerView; }
  bool showHeading() const { return m_showHeading; }
  bool showSpawns() const { return m_showSpawns; }
  bool showDrops() const { return m_showDrops; }
  bool showCoins() const { return m_showCoins; }
  bool showSpawnNames() const { return m_showSpawnNames; }
  bool showFiltered() const { return m_showFiltered; }
  bool showVelocityLines() const { return m_showVelocityLines; }
#ifdef DEBUG
  bool showDebugInfo() const { return m_showDebugInfo; }
#endif
  bool animate() const { return m_animate; }
  bool spawnDepthFilter() const { return m_spawnDepthFilter; }
  bool highlightConsideredSpawns() const { return m_highlightConsideredSpawns; }
  bool showTooltips() const { return m_showTooltips; }
  bool walkPathShowSelect() const { return m_walkpathshowselect; }
  bool deityPvP() const { return m_deityPvP; }
  bool racePvP() const { return m_racePvP; }
  
  MapLineStyle mapLineStyle() { return m_param.mapLineStyle(); }
  MapOptimizationMethod mapOptimization() { return m_param.mapOptimizationMethod(); }
  int zoom() const { return m_param.zoom(); }
  int panOffsetX() const { return m_param.panOffsetX(); }
  int panOffsetY() const { return m_param.panOffsetY(); }
  int gridResolution() const { return m_param.gridResolution(); }
  const QColor& gridTickColor() const { return m_param.gridTickColor(); }
  const QColor& gridLineColor() const { return m_param.gridLineColor(); }
  const QColor& backgroundColor() const { return m_param.backgroundColor(); }
  const QFont& font() const { return m_param.font(); }
  int headRoom() const { return m_param.headRoom(); }
  int floorRoom() const { return m_param.floorRoom(); }
  
  bool showBackgroundImage() const { return m_param.showBackgroundImage(); }
  bool showLocations() const { return m_param.showLocations(); }
  bool showLines() const { return m_param.showLines(); }
  bool showGridLines() const { return m_param.showGridLines(); }
  bool showGridTicks() const { return m_param.showGridTicks(); }
  
  
 public slots:   
  void savePrefs(void);
  
  void selectSpawn(const Item* item);
  void delItem(const Item* item);
  void clearItems(void);
  
  void mapLoaded(void);
  void mapUnloaded(void);
  void mapUpdated(void);
  
  void addLocation();
  void startLine();
  void addLinePoint();
  void delLinePoint(void);
  void addPathPoint();

  void makeSpawnLine(const Item* item);
  void makeSelectedSpawnLine();
 
  void ZoomIn ();
  void ZoomOut ();
  void increaseGridResolution	(void);
  void decreaseGridResolution	(void);
  void PanRight();
  void PanLeft();
  void PanDown();
  void PanUp();
  void PanUpRight();
  void PanUpLeft();
  void PanDownRight();
  void PanDownLeft();
  void ViewTarget();
  void ViewLock();

  void reAdjust (void);
  void refreshMap(void);
  
  // set methods
  void setFollowMode(FollowMode mode);
  void setShowFiltered(bool val);
  void setFrameRate(int val);
  void setDrawSize(int val);
  void setShowMapLines(bool val);
  void setShowPlayer(bool val);
  void setShowPlayerBackground(bool val);
  void setShowPlayerView(bool val);
  void setShowHeading(bool val);
  void setShowSpawns(bool val);
  void setShowDrops(bool val);
  void setShowCoins(bool val);
  void setShowSpawnNames(bool val);
  void setShowVelocityLines(bool val);
  void setShowDebugInfo(bool val);
  void setAnimate(bool val);
  void setSpawnDepthFilter(bool val);
  void setHighlightConsideredSpawns(bool val);
  void setShowTooltips(bool val);
  void setWalkPathShowSelect(bool val);
  void setDeityPvP(bool val);
  void setRacePvP(bool val);
  
  void setMapLineStyle(MapLineStyle style);
  void setMapOptimization(MapOptimizationMethod method);
  void setZoom(int val);
  void setPanOffsetX(int val);
  void setPanOffsetY(int val);
  void setGridResolution(int val);
  void setGridTickColor(const QColor& color);
  void setGridLineColor(const QColor& color);
  void setBackgroundColor(const QColor& color);
  void setFont(const QFont& font);
  void setHeadRoom(int val);
  void setFloorRoom(int val);
  
  void setShowBackgroundImage(bool val);
  void setShowLocations(bool val);
  void setShowLines(bool val);
  void setShowGridLines(bool val);
  void setShowGridTicks(bool val);

  // dump debug info
  void dumpInfo(QTextStream& out);
  
 signals: 
  void mouseLocation(int16_t xPos, int16_t yPos);
  void spawnSelected(const Item* item);
  void zoomChanged(int zoom);
  void frameRateChanged(int frameRate);
  void panXChanged(int x);
  void panYChanged(int y);
  void headRoomChanged(int headRoom);
  void floorRoomChanged(int floorRoom);

protected:
   const Item* closestSpawnToPoint(const QPoint& pt,
					     uint32_t closestDistance) const;
   void paintEvent (QPaintEvent *);
   void mousePressEvent (QMouseEvent *);
   void mouseMoveEvent( QMouseEvent* );
   void mouseReleaseEvent( QMouseEvent *);
   void mouseDoubleClickEvent( QMouseEvent *);
   void resizeEvent (QResizeEvent *);

   void paintMap (QPainter *);
   void paintPlayerBackground(MapParameters& param, QPainter& p);
   void paintPlayerView(MapParameters& param, QPainter& p);
   void paintPlayer(MapParameters& param, QPainter& p);
   void paintDrops(MapParameters& param, QPainter& p);
   void paintCoins(MapParameters& param, QPainter& p);
   void paintSelectedSpawnSpecials(MapParameters& param, QPainter& p,
				   const QTime& drawTime);
   void paintSpawns(MapParameters& param, QPainter& p, const QTime& drawTime);
   void paintDebugInfo(MapParameters& param, 
		       QPainter& tmp, 
		       float fps, 
		       int drawTime);
   QRect mapRect () const;

private:   
   QString m_preferenceName;
   MapParameters m_param;
   MapMgr* m_mapMgr;
   MapCache m_mapCache;
   MapMenu* m_menu;
   uint32_t m_runtimeFilterFlagMask;
   QTimer* m_timer;

#ifdef DEBUG
   // debug timing info
   QTime m_time;
   int m_frameTimes[maxFrameTimes];
   int m_frameTimeIndex;
   int m_paintCount;
   int m_paintTimeSum;
#endif

   // mouse based panning state info
   bool m_mapPanning;
   int m_mapPanX;
   int m_mapPanY;

   QPixmap m_offscreen;
   QTime m_lastFlash;
   bool m_flash;

   const Item* m_selectedItem;
   EQPlayer* m_player;
   SpawnShell* m_spawnShell;
   MapLabel* m_mapTip;

   FollowMode m_followMode;

   int m_frameRate;
   int m_drawSize;
   bool m_showMapLines;
   bool m_showMapLocations;
   bool m_showPlayer;
   bool m_showPlayerBackground;
   bool m_showPlayerView;
   bool m_showHeading;
   bool m_showDrops;
   bool m_showCoins;
   bool m_showSpawns;
   bool m_showSpawnNames;
   bool m_showFiltered;
   bool m_showVelocityLines;
#ifdef DEBUG
   bool m_showDebugInfo;
#endif
   bool m_animate;
   bool m_spawnDepthFilter;
   bool m_highlightConsideredSpawns;
   bool m_showTooltips;
   bool m_walkpathshowselect;
   bool m_deityPvP;
   bool m_racePvP;
};

//----------------------------------------------------------------------
// MapFilterLineEdit
class MapFilterLineEdit : public QLineEdit
{
 public:
  MapFilterLineEdit(QWidget* parent = 0, const char* name = 0)
    : QLineEdit(parent, name) {}
  ~MapFilterLineEdit() {} ;

 protected:
  virtual void leaveEvent(QEvent* ev) { emit returnPressed(); }
};

//----------------------------------------------------------------------
// MapFrame
class MapFrame : public QVBox
{
   Q_OBJECT

 public:
   MapFrame(FilterMgr* filterMgr,
	    MapMgr* mapMgr,
	    EQPlayer* player, 
	    SpawnShell* spawnshell,
	    const QString& preferenceName = "Map", 
	    const QString& caption = "Map",
	    const char* mapName = "map",
	    QWidget* parent = 0, const char* name = "mapframe");
   virtual ~MapFrame();

   Map* map() { return m_map; }
   const QString& mapPreferenceName() { return m_mapPreferenceName; }
   QString preferenceName() { return mapPreferenceName() + "Frame"; }

 public slots:
   void regexpok     (int ok);
   void setregexp    (const QString&);
   void filterConfirmed();
   void mouseLocation(int16_t xPos, int16_t yPos);
   void setPlayer (int16_t x, int16_t y, int16_t z, 
		   int16_t Dx, int16_t Dy, int16_t Dz, int32_t degrees);
   void savePrefs(void);
   void restoreSize();
   void restorePosition();

  // dump debug info
  void dumpInfo(QTextStream& out);

 protected slots:
   void init_Menu(void);

   void toggle_top_controls(int id);
   void toggle_bottom_controls(int id);
   void toggle_zoom(int id);
   void toggle_playerLocation(int id);
   void toggle_mouseLocation(int id);
   void toggle_filter(int id);
   void toggle_frameRate(int id);
   void toggle_pan(int id);
   void toggle_depthControls(int id);

 private:
   // pointer to the Map that this frame contains/controls
   Map* m_map;

   FilterMgr* m_filterMgr;
   QString m_lastFilter;
   uint32_t m_runtimeFilterFlagMask;
   uint8_t m_runtimeFilterFlag;

   QString m_mapPreferenceName;

   QHBox* m_topControlBox;
   QHBox* m_zoomBox;
   QSpinBox* m_zoom;
   QHBox* m_playerLocationBox;
   QLabel* m_playerLocation;
   QHBox* m_mouseLocationBox;
   QLabel* m_mouseLocation;
   QHBox* m_filterBox;
   MapFilterLineEdit* m_filter;

   QHBox* m_bottomControlBox;
   QHBox* m_frameRateBox;
   QSpinBox* m_frameRate;
   QHBox* m_panBox;
   QSpinBox* m_panX;
   QSpinBox* m_panY;
   QHBox* m_depthControlBox;
   QSpinBox* m_head;
   QSpinBox* m_floor;
   
   int m_id_topControl;
   int m_id_bottomControl;
   int m_id_zoom;
   int m_id_playerLocation;
   int m_id_mouseLocation;
   int m_id_filter;
   int m_id_topControl_Options;
   int m_id_frameRate;
   int m_id_pan;
   int m_id_depthControlRoom;
   int m_id_bottomControl_Options;
};

#endif // EQMAP_H

