/*
 * mapcore.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 */

// Author: Zaphod (dohpaz@users.sourceforge.net)
//    Many parts derived from existing ShowEQ/SINS map code

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//
//

// ZBTEMP: Note: Currently hardcoded to use int16_t type for point data, 
// should migrate this to a compiler define/typedef based value that 
// can be defined differently depending on the needs of other projects
//
#ifndef _MAPCORE_H
#define _MAPCORE_H

#include <stdlib.h>
#include <stdio.h>

#include <qstring.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qpointarray.h>

#include "point.h"
#include "fixpt.h"

///////////////////////////////////////////
// forward declarations
class MapData;
class MapLine;
class MapLocation;
class MapParameters;
class MapImage;

///////////////////////////////////////////
// type definitions
typedef Point3D<int16_t> MapPoint;
typedef Point3DArray<int16_t> MapPointArray;

///////////////////////////////////////////
// enumerated types
enum MapLineStyle
{
  tMap_Normal,
  tMap_DepthFiltered,
  tMap_FadedFloors,
};

enum MapOptimizationMethod
{
  tMap_MemoryOptim = 0,
  tMap_NormalOptim = 1,
  tMap_BestOptim = 2,
  tMap_DefaultOptim = 3,
  tMap_NoOptim = 4,
};

//----------------------------------------------------------------------
// constants

//----------------------------------------------------------------------
// MapParameters
class MapParameters
{
 public:
  // q format used for map fixed point math
  enum { qFormat = 14 };


 public:
  MapParameters(const MapData& mapData);
  ~MapParameters();

  // get methods
  const MapPoint& player() const { return m_curPlayer; }
  const QPoint& playerOffset() const { return m_curPlayerOffset; }
  int playerXOffset() const { return m_curPlayerOffset.x(); }
  int playerYOffset() const { return m_curPlayerOffset.y(); }
  int16_t playerHeadRoom() const { return m_playerHeadRoom; }
  int16_t playerFloorRoom() const { return m_playerFloorRoom; }
  const QSize& screenLength() const { return m_screenLength; }
  int screenLengthX() const { return m_screenLength.width(); }
  int screenLengthY() const { return m_screenLength.height(); }
  const QRect& screenBounds() const { return m_screenBounds; }
  const QPoint& screenCenter() const { return m_screenCenter; }
  int screenCenterX() const { return m_screenCenter.x(); }
  int screenCenterY() const { return m_screenCenter.y(); }
  const QSize& zoomMapLength() const { return m_zoomMapLength; }
  int zoomMapLengthX() const { return m_zoomMapLength.width(); }
  int zoomMapLengthY() const { return m_zoomMapLength.height(); }
  int panOffsetX() const { return m_panOffsetX; }
  int panOffsetY() const { return m_panOffsetY; }
  int zoom() const { return m_zoom; }
  int zoomDefault() const { return m_zoomDefault; }
  double ratio() const { return m_ratio; }
  double ratioX() const { return m_ratio; }
  double ratioY() const { return m_ratio; } 
  int ratioIFixPt() const { return m_ratioIFixPt; }

  int gridResolution() const { return m_gridResolution; }
  const QColor& gridLineColor() const { return m_gridLineColor; }
  const QColor& gridTickColor() const { return m_gridTickColor; }
  const QColor& backgroundColor() const { return m_backgroundColor; }
  const QFont& font() const { return m_font; }
  int16_t headRoom() const { return m_headRoom; }
  int16_t floorRoom() const { return m_floorRoom; }
  MapOptimizationMethod mapOptimizationMethod() { return m_optimization; }
  QPixmap::Optimization pixmapOptimizationMethod();
  MapLineStyle mapLineStyle() { return m_mapLineStyle; }
  bool fadeFloors() const { return (m_mapLineStyle == tMap_FadedFloors); }
  bool depthFiltering() const { return (m_mapLineStyle == tMap_DepthFiltered); }
  bool showBackgroundImage() const { return m_showBackgroundImage; }
  bool showLocations() const { return m_showLocations; }
  bool showLines() const { return m_showLines; }
  bool showGridLines() const { return m_showGridLines; }
  bool showGridTicks() const { return m_showGridTicks; }

  // utility methods
  int calcXOffset (int mapCoordinate) const;
  int invertXOffset (int worldX) const;
  int calcYOffset (int mapCoordinate) const;
  int invertYOffset (int worldY) const;

  int calcXOffsetI(int mapCoordinate) const;
  int calcYOffsetI(int mapCoordinate) const;

  // set/adjust methods
  void setPlayer(const MapPoint& pos);

  void setGridResolution(int res);
  void increaseGridResolution(void);
  void decreaseGridResolution(void);
  void setZoomDefault(int zoom);
  bool setZoom(int zoom);
  bool zoomIn();
  bool zoomOut();
  void panX(int xPan);
  void panY(int yPan);
  void panXY(int xPan, int yPan);
  void clearPan();
  void setPan(int xPan, int yPan);
  void setPanX(int xPan);
  void setPanY(int yPan);
  void setScreenSize(const QSize& size);
  void setBackgroundColor(const QColor& color) { m_backgroundColor = color; }
  void setFont(const QFont& font) { m_font = font; }
  void setMapOptimizationMethod(MapOptimizationMethod method) 
    { m_optimization = method; }  
  void setMapLineStyle(MapLineStyle style) { m_mapLineStyle = style; }
  void setShowBackgroundImage(bool val) { m_showBackgroundImage = val; }
  void setShowLocations(bool val) { m_showLocations = val; }
  void setShowLines(bool val) { m_showLines = val; }
  void setShowGridLines(bool val) { m_showGridLines = val; }
  void setShowGridTicks(bool val) { m_showGridTicks = val; }
  void setGridLineColor(const QColor& color) { m_gridLineColor = color; }
  void setGridTickColor(const QColor& color) { m_gridTickColor = color; }
  void setHeadRoom(int16_t headRoom);
  void setFloorRoom(int16_t floorRoom);
  
  void reAdjust(MapPoint* targetPoint);
  void reAdjust();
  
  void reset();
 private:
  const MapData* m_mapData;
  MapPoint m_curPlayer;
  QPoint m_curPlayerOffset;
  int16_t m_playerHeadRoom; 
  int16_t m_playerFloorRoom;
  QSize m_screenLength;
  QRect m_screenBounds;
  QPoint m_screenCenter;
  QSize m_zoomMapLength;
  uint32_t m_zoom;
  uint32_t m_zoomDefault;
  int m_panOffsetX;
  int m_panOffsetY;
  double m_ratio;
  int m_ratioIFixPt;

  // configuration parameters
  int m_gridResolution;
  QColor m_gridLineColor;
  QColor m_gridTickColor;
  QColor m_backgroundColor;
  QFont m_font;
  int16_t m_headRoom;
  int16_t m_floorRoom;

  MapPoint m_targetPoint;
  bool m_targetPointSet;

  MapOptimizationMethod m_optimization;
  MapLineStyle m_mapLineStyle;
  bool m_showBackgroundImage; 
  bool m_showLocations;
  bool m_showLines;
  bool m_showGridLines;
  bool m_showGridTicks;
};

inline 
int MapParameters::calcXOffset (int mapCoordinate) const
{ 
  return screenCenterX() - (int)(mapCoordinate / m_ratio); 
}

inline 
int MapParameters::invertXOffset (int worldX) const
{
  return (int)rint((screenCenterX() - worldX) * m_ratio); 
}

inline 
int MapParameters::calcYOffset (int mapCoordinate) const
{ 
  return screenCenterY() - (int)(mapCoordinate / m_ratio); 
}

inline 
int MapParameters::invertYOffset (int worldY) const
{ 
  return (int)rint((screenCenterY() - worldY) * m_ratio); 
}

inline 
int MapParameters::calcXOffsetI(int mapCoordinate) const
{ 
  return screenCenterX() - 
    fixPtMulII(m_ratioIFixPt, qFormat, mapCoordinate);
}

inline 
int MapParameters::calcYOffsetI(int mapCoordinate) const
{ 
  return screenCenterY() - 
    fixPtMulII(m_ratioIFixPt, qFormat, mapCoordinate);
}

inline 
void MapParameters::setGridResolution(int res) 
{
  if ((res >= 50) && (res <= 1500))
    m_gridResolution = res; 
}

inline 
void MapParameters::increaseGridResolution(void)
{
  if (m_gridResolution <= 100)
    m_gridResolution = 50;
  else
    m_gridResolution -= 100;
  reAdjust();
}

inline 
void MapParameters::decreaseGridResolution(void)
{
  if (m_gridResolution >= 1500)
    return;
  
  m_gridResolution += 100;
  reAdjust();
}

inline 
bool MapParameters::setZoom(int zoom)
{
  if ((zoom <= 32) &&
      (zoom >= 1))
  {
    m_zoom = zoom;
    reAdjust();
    return true;
  }
  
  return false;
}

inline 
void MapParameters::setZoomDefault(int zoom)
{
  if ((zoom <= 32) &&
      (zoom >= 1))
    m_zoomDefault = zoom;
}

inline 
bool MapParameters::zoomIn() 
{ 
  if (m_zoom < 32) 
  {
    m_zoom *= 2; 
    reAdjust();
    return true;
  }
  return false;
}

inline 
bool MapParameters::zoomOut() 
{ 
  if (m_zoom > 1) 
  {
    m_zoom /= 2; 
    if (m_zoom == 1) 
      clearPan();
    
    reAdjust();
    
    return true;
  }
  return false;
}

inline 
void MapParameters::panX(int xPan) 
{ 
  m_panOffsetX += m_zoomMapLength.width() / xPan;
  reAdjust();
}

inline 
void MapParameters::panY(int yPan) 
{ 
  m_panOffsetY += m_zoomMapLength.height() / yPan; 
  reAdjust();
}

inline 
void MapParameters::panXY(int xPan, int yPan)
{
  m_panOffsetX += m_zoomMapLength.width() / xPan;
  m_panOffsetY += m_zoomMapLength.height() / yPan;
  reAdjust();
}

inline 
void MapParameters::clearPan() 
{ 
  m_panOffsetX = 0; 
  m_panOffsetY = 0; 
  reAdjust();
}

inline 
void MapParameters::setPan(int xPan, int yPan) 
{ 
  m_panOffsetX = xPan; 
  m_panOffsetY = yPan; 
  reAdjust();
}

inline 
void MapParameters::setPanX(int xPan)
{
  m_panOffsetX = xPan;
  reAdjust();
}

inline 
void MapParameters::setPanY(int yPan)
{
  m_panOffsetY = yPan;
  reAdjust();
}

inline 
void MapParameters::setScreenSize(const QSize& size) 
{ 
  m_screenLength = size; 
  reAdjust();
}

//----------------------------------------------------------------------
// MapCommon
class MapCommon
{
 public:
  MapCommon() {}
  MapCommon(const QString& name, const QString& color)
    : m_name(name), m_color(color) {}
  virtual ~MapCommon();

  const QString& name() { return m_name; }
  const QString& color() { return m_color; }

  void setName(const QString& name) { m_name = name; }
  void setColor(const QString& color) { m_color = color; }

 private:
  QString m_name;
  QString m_color;
};

//----------------------------------------------------------------------
// MapLineL
class MapLineL : public MapCommon, public QPointArray
{
 public:
  MapLineL();
  MapLineL(const QString& name, const QString& color, uint32_t size);
  MapLineL(const QString& name, const QString& color, uint32_t size, int16_t z);
  virtual ~MapLineL();
  
  int16_t z() const { return m_z; }
  bool heightSet() const { return m_heightSet; }
  const QRect& boundingRect() const { return m_bounds; }

  void setZPos(uint16_t z)
    {  m_z = z; m_heightSet = true; }
  void calcBounds() { m_bounds = QPointArray::boundingRect(); }

 private:
  int16_t m_z;
  bool m_heightSet;
  QRect m_bounds;
};

//----------------------------------------------------------------------
// MapLineM
class MapLineM : public MapCommon, public MapPointArray
{
 public:
  MapLineM();
  MapLineM(const QString& name, const QString& color, uint32_t size);
  MapLineM(const QString& name, const QString& color, const MapPoint& point);
  virtual ~MapLineM();

  QRect boundingRect() const { return m_bounds; }
  void calcBounds() { m_bounds = MapPointArray::boundingRect(); }

 private:
  QRect m_bounds;
};

//----------------------------------------------------------------------
// MapLocation
class MapLocation : public MapCommon, public QPoint
{
 public:
  MapLocation();
  MapLocation(const QString& name, const QString& color, const QPoint& point);
  MapLocation(const QString& name, const QString& color, 
	      int16_t x, int16_t y);
  virtual ~MapLocation();
 private:
};

//----------------------------------------------------------------------
// MapAggro
class MapAggro
{
 public:
  MapAggro();
  MapAggro(const QString& name, uint16_t range) : m_name(name), m_range(range) {}
  virtual ~MapAggro();

  const QString& name() { return m_name; }
  uint16_t range() { return m_range; }

  void setName(const QString& name) { m_name = name; }
  void setRange(uint16_t range);
 private:
  QString m_name;
  uint16_t m_range;
};

//----------------------------------------------------------------------
// MapData
class MapData
{
 public:
  // constructor/destructor
  MapData();
  ~MapData();

  // map loading/clearing/saving
  void clear();
  void loadMap(const QString& fileName);
  void saveMap() const;

  // accessors
  const QString& fileName() const { return m_fileName; }
  const QString& zoneShortName() const { return m_zoneShortName; }
  const QString& zoneLongName() const { return m_zoneLongName; }
  const QRect& boundingRect() const { return m_boundingRect; }
  const QSize& size() const { return m_size; }
  uint8_t zoneZEM() const { return m_zoneZEM; }
  int16_t minX() const { return m_minX; }
  int16_t minY() const { return m_minY; }
  int16_t maxX() const { return m_maxX; }
  int16_t maxY() const { return m_maxY; }
  const QPixmap& image() const { return m_image; }
  bool imageLoaded() const { return m_imageLoaded; }
  bool mapLoaded() const { return m_mapLoaded; }
  bool isAggro(const QString& name, uint16_t* range) const;

  // make sure map is big enough, returns true if size modified
  bool checkPos(int16_t x, int16_t y);
  void quickCheckPos(int16_t x, int16_t y);
  void updateBounds();

  // map editing
  void addLocation(const QString& name, const QString& color, const QPoint& point);
  void setLocationName(const QString& name);
  void setLocationColor(const QString& color);
  void startLine(const QString& name, const QString& color, const MapPoint& point);
  void addLinePoint(const MapPoint& point);
  void delLinePoint(void);
  void setLineName(const QString& name);
  void setLineColor(const QString& color);
  void setFileName(const QString& name) { m_fileName = name; }
  void setZoneLongName(const QString& name) { m_zoneShortName = name; }
  void setZoneShortName(const QString& name) { m_zoneLongName = name; }
  void setZoneZEM(uint8_t zem) { m_zoneZEM = zem; }

  // map painting
  void paintGrid(MapParameters& param, QPainter& p) const;
  void paintLines(MapParameters& param, QPainter& p) const;
  void paintDepthFilteredLines(MapParameters& param, QPainter& p) const;
  void paintFadedFloorsLines(MapParameters& param, QPainter& p) const;
  void paintLocations(MapParameters& param, QPainter& p) const;
  bool paintMapImage(MapParameters& param, QPainter& p) const;

 private:
  int16_t m_minX;
  int16_t m_minY;
  int16_t m_maxX;
  int16_t m_maxY;
  QRect m_boundingRect;
  QSize m_size;
  QString m_fileName;
  QString m_zoneLongName;
  QString m_zoneShortName;
  QList<MapLineL> m_lLines;
  QList<MapLineM> m_mLines;
  MapLineM* m_editLineM;
  QList<MapLocation> m_locations;
  MapLocation* m_editLocation;
  QList<MapAggro> m_aggros;
  uint8_t m_zoneZEM;
  QPixmap m_image;
  bool m_imageLoaded;
  bool m_mapLoaded;
};

inline
bool MapData::checkPos(int16_t x, int16_t y)
{
  bool flag = false;

#if defined(MAP_DEBUG)
  printf("in x: %i, in y: %i, max(%i,%i) Min(%i,%i)\n", x, y, m_maxX, m_maxY, m_minX, m_minY);
#endif /* MAP_DEBUG */

  if (x > m_maxX)
  {
    m_maxX = x;
    flag = true;
  }
  if (y > m_maxY)
  {
    m_maxY = y;
    flag = true;
  }
  if (x < m_minX)
  {
    m_minX = x;
    flag = true;
  }
  if (y < m_minY)
  {
    m_minY = y;
    flag = true;
  }

  // update the boundary information if bounds changed
  if (flag)
    updateBounds();

  return flag;
}

inline
void MapData::quickCheckPos(int16_t x, int16_t y)
{
  // quick, no-nonsense checking of the bounds, for batch use.
  // call updateBounds() after finished with the batch
  if (x > m_maxX)
    m_maxX = x;
  if (y > m_maxY)
    m_maxY = y;
  if (x < m_minX)
      m_minX = x;
  if (y < m_minY)
    m_minY = y;
}

inline
void MapData::updateBounds()
{
  // update the boundary information
  m_boundingRect =  QRect(QPoint(m_minX, m_minY), QPoint(m_maxX, m_maxY));
  m_size.setWidth(m_boundingRect.width());
  m_size.setHeight(m_boundingRect.height());
}

//----------------------------------------------------------------------
// MapCache
class MapCache 
{
 public:
  MapCache(const MapData& data);
  ~MapCache();

  const QPixmap& getMapImage(MapParameters& param);

  // get methods
  bool needRepaint(MapParameters& param);
  bool alwaysRepaint() const { return m_alwaysRepaint; }
#ifdef DEBUG
  uint32_t paintCount() const { return m_paintCount; }
#endif

  // set methods
  void setAlwaysRepaint(bool val) { m_alwaysRepaint = val; }
  
 private:
  const MapData& m_mapData;
  QPixmap m_mapImage;
  MapParameters m_lastParam;
#ifdef DEBUG
  uint32_t m_paintCount;
#endif
  bool m_painted;
  bool m_alwaysRepaint;
};

//----------------------------------------------------------------------
// assorted utility functions
inline bool inRect(const QRect& rect, 
		   const int16_t& x, 
		   const int16_t& y)
{
  return ((rect.left() <= x) && (rect.right() >= x) &&
	  (rect.top() <= y) && (rect.bottom() >= y));
}

inline bool inRoom(const int16_t& headRoom, 
		   const int16_t& floorRoom, 
		   const int16_t& z)
{
  return ((z <= headRoom) &&
	  (z >= floorRoom));
}

#endif // _MAPCORE_H
