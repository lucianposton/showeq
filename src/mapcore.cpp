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

#include <dirent.h>
#include <errno.h>

#include <qpainter.h>

#include "mapcore.h"

//----------------------------------------------------------------------
// MapParameters
MapParameters::MapParameters(const MapData& mapData)
  : m_mapData(&mapData),
    m_curPlayer(0, 0, 0),
    m_screenLength(600, 600)
{
  m_screenCenter = QPoint(300, 300);
  m_zoomMapLength = QSize(100, 100);
  m_panOffsetX = 0;
  m_panOffsetY = 0;
  m_ratio = 1.0;

  // calculate fixed point inverse ratio using the defiend qFormat 
  // for calculate speed purposes (* is faster than /)
  m_ratioIFixPt = fixPtToFixed<int, double>((1.0 / m_ratio), qFormat);

  m_targetPoint = MapPoint(0, 0, 0);
  m_targetPointSet = false;

  m_gridResolution = 500;
  m_gridTickColor.setRgb(225, 200, 75);
  m_gridLineColor.setRgb(75, 200, 75);
  m_backgroundColor = Qt::black;

  m_headRoom = 75;
  m_floorRoom = 75;

  m_mapLineStyle = tMap_Normal;
  m_showBackgroundImage = true;
  m_showLocations = true;
  m_showLines = true;
  m_showGridLines = true;
  m_showGridTicks = true;

  reset();
}

MapParameters::~MapParameters()
{
}

void MapParameters::reset()
{
  m_zoom = 1;
  m_panOffsetX = 0;
  m_panOffsetY = 0;
}

void MapParameters::reAdjust(MapPoint* targetPoint)
{
  if (targetPoint)
  {
    m_targetPoint = *targetPoint;
    m_targetPointSet = true;
  }
  else
  {
    m_targetPoint.setPoint(0, 0, 0);
    m_targetPointSet = false;
  }

  reAdjust();
}

void MapParameters::reAdjust()
{
  // get the map length
  const QSize& mapSize = m_mapData->size();

  if (m_zoom > 32)
    m_zoom = 32;

  // calculate zoomed map size
  int pxrat = ((mapSize.width()) / (m_zoom));
  int pyrat = ((mapSize.height()) / (m_zoom));

  // if it's a bit small, zoom out
  if ((pxrat <= 2) || (pyrat <= 2))
  {
     if (m_zoom > 1) 
     {
       m_zoom /= 2; 
       if (m_zoom == 1) 
	 clearPan();

       // recalculate zoomed map size
       pxrat = ((mapSize.width()) / (m_zoom));
       pyrat = ((mapSize.height()) / (m_zoom));
     }
  }

  // save zoomed map size
  m_zoomMapLength.setWidth(pxrat);
  m_zoomMapLength.setHeight(pyrat);

  double xrat = (double)m_screenLength.width() / m_zoomMapLength.width();
  double yrat = (double)m_screenLength.height() / m_zoomMapLength.height();

  int xoff = 0;
  int yoff = 0;

  if (xrat < yrat) 
  {
    m_zoomMapLength.setHeight((int)(m_screenLength.height() / xrat));
    if (m_zoom <= 1)
      yoff = (m_zoomMapLength.height() - pyrat) / 2;
  } 
  else if (yrat) 
  {
    m_zoomMapLength.setWidth((int)(m_screenLength.width() / yrat));
    if (m_zoom <= 1)
      xoff = (m_zoomMapLength.width() - pxrat) / 2;
  }

  // calculate the scaling ratio to use
  m_ratio = (double)m_zoomMapLength.width() / (double)m_screenLength.width();

  // calculate fixed point inverse ratio using the defiend qFormat 
  // for calculate speed purposes (* is faster than /)
  m_ratioIFixPt = fixPtToFixed<int, double>((1.0 / m_ratio), qFormat);

  int iZMaxX, iZMinX, iZMaxY, iZMinY;

  // center on the target based on target
  if (m_targetPointSet)
  {
    iZMaxX = m_targetPoint.xPos() + m_panOffsetX + m_zoomMapLength.width() / 2;
    iZMinX = m_targetPoint.xPos() + m_panOffsetX - m_zoomMapLength.width() / 2;
    
    iZMaxY = m_targetPoint.yPos() + m_panOffsetY + m_zoomMapLength.height() / 2;
    iZMinY = m_targetPoint.yPos() + m_panOffsetY - m_zoomMapLength.height() / 2;
  }
  else
  {
    iZMaxX = m_panOffsetX + m_zoomMapLength.width()  / 2;
    iZMinX = m_panOffsetX - m_zoomMapLength.width() / 2;
    
    iZMaxY = m_panOffsetY + m_zoomMapLength.height() / 2;
    iZMinY = m_panOffsetY - m_zoomMapLength.height() / 2;
  }

  // try not to have blank space on the sides
  if (iZMinX < m_mapData->minX())
  {
    iZMaxX -= (iZMinX - m_mapData->minX());
    iZMinX -= (iZMinX - m_mapData->minX());
  }
  if (iZMinY < m_mapData->minY())
  {
    iZMaxY -= (iZMinY - m_mapData->minY());
    iZMinY -= (iZMinY - m_mapData->minY());
  }

  if (iZMaxX > m_mapData->maxX())
  {
    iZMinX -= iZMaxX - m_mapData->maxX();
    iZMaxX -= iZMaxX - m_mapData->maxX();
  }
  if (iZMaxY > m_mapData->maxY())
  {
    iZMinY -= (iZMaxY - m_mapData->maxY());
    iZMaxY -= (iZMaxY - m_mapData->maxY());
  }

  // calculate the new screen center
  m_screenCenter.setX((((iZMaxX + xoff) * m_screenLength.width()) / 
		       m_zoomMapLength.width()));
  m_screenCenter.setY((((iZMaxY + yoff) * m_screenLength.height()) / 
		       m_zoomMapLength.height()));

  // calculate screen bounds
  m_screenBounds = QRect(int((m_screenCenter.x() - m_screenLength.width()) * 
			     m_ratio),
			 int((m_screenCenter.y() - m_screenLength.height()) * 
			     m_ratio),
			 int(m_screenLength.width() * m_ratio),
			 int(m_screenLength.height() * m_ratio));


  // adjust pre-calculate player offsets
  m_curPlayerOffset.setX(calcXOffset(m_curPlayer.xPos()));
  m_curPlayerOffset.setY(calcYOffset(m_curPlayer.yPos()));
}

QPixmap::Optimization MapParameters::pixmapOptimizationMethod()
{
  switch (m_optimization)
  {
  case tMap_MemoryOptim:  
    return QPixmap::MemoryOptim; 
  case tMap_NoOptim:
    return QPixmap::NoOptim; 
  case tMap_NormalOptim:  
    return QPixmap::NormalOptim;
  case tMap_BestOptim:  
    return QPixmap::BestOptim;
  case tMap_DefaultOptim:
  default: 
    return QPixmap::DefaultOptim;
  }
   /* Optimization Methods:
        DefaultOptim - A pixmap with this optimization mode set always has the default optimization type
               - default optimization type for qPixMap is NormalOptim
        NoOptim      - no optimization (currently the same as MemoryOptim).
        MemoryOptim  - optimize for minimal memory use. 
        NormalOptim  - optimize for typical usage. Often uses more memory than MemoryOptim, and often faster. 
        BestOptim    - optimize for pixmaps that are drawn very often and where performance is critical.
                       Generally uses more memory than NormalOptim and may provide a little better speed
   */
}

void MapParameters::setPlayer(const MapPoint& pos)
{ 
  m_curPlayer = pos; 

  // re-calculate precomputed player head/floor room
  m_playerHeadRoom = m_curPlayer.zPos() + m_headRoom;
  m_playerFloorRoom = m_curPlayer.zPos() - m_floorRoom;

  m_curPlayerOffset.setX(calcXOffset(m_curPlayer.xPos()));
  m_curPlayerOffset.setY(calcYOffset(m_curPlayer.yPos()));
}

void MapParameters::setHeadRoom(int16_t headRoom)
{ 
  m_headRoom = headRoom; 

  // re-calculate precomputed player head/floor room
  m_playerHeadRoom = m_curPlayer.zPos() + m_headRoom;
}

void MapParameters::setFloorRoom(int16_t floorRoom) 
{ 
  m_floorRoom = floorRoom; 

  // re-calculate precomputed player head/floor room
  m_playerFloorRoom = m_curPlayer.zPos() - m_floorRoom;
}

//----------------------------------------------------------------------
// MapCommon
MapCommon::~MapCommon()
{
}

//----------------------------------------------------------------------
// MapLineL
MapLineL::MapLineL()
  : MapCommon(), m_zPos(0), m_heightSet(false)
{
}

MapLineL::MapLineL(const QString& name, 
		   const QString& color, 
		   uint32_t size)
  : MapCommon(name, color),
    QPointArray(size), 
    m_zPos(0),
    m_heightSet(false)
{
}

MapLineL::MapLineL(const QString& name, 
		   const QString& color, 
		   uint32_t size, 
		   int16_t zPos)
  : MapCommon(name, color),
    QPointArray(size), 
    m_zPos(zPos),
    m_heightSet(true)
{
}

MapLineL::~MapLineL()
{
}

//----------------------------------------------------------------------
// MapLineM
MapLineM::MapLineM()
  : MapCommon()
{
}

MapLineM::MapLineM(const QString& name, const QString& color, uint32_t size)
  : MapCommon(name, color),
    MapPointArray(size)
{
}

MapLineM::MapLineM(const QString& name, const QString& color, const MapPoint& point)
  : MapCommon(name, color),
    MapPointArray(1)
{
  // set the first point
  setPoint(0, point);
}

MapLineM::~MapLineM()
{
}

//----------------------------------------------------------------------
// MapLocation
MapLocation::MapLocation()
{
}

MapLocation::MapLocation(const QString& name, 
			 const QString& color, 
			 const QPoint& point)
  : MapCommon(name, color),
    QPoint(point)
{
}

MapLocation::MapLocation(const QString& name, 
			 const QString& color, 
			 int16_t xPos, 
			 int16_t yPos)
  : MapCommon(name, color),
    QPoint(xPos, yPos)
{
}

MapLocation::~MapLocation()
{
}

//----------------------------------------------------------------------
// MapAggro
MapAggro::~MapAggro()
{
}


//----------------------------------------------------------------------
// MapData
MapData::MapData()
{
  // make all lists auto delete
  m_lLines.setAutoDelete(true);
  m_mLines.setAutoDelete(true);
  m_locations.setAutoDelete(true);
  m_aggros.setAutoDelete(true);

  // clear the structure
  clear();
}

MapData::~MapData()
{
}

void MapData::clear()
{
  m_fileName = "";
  m_zoneLongName = "";
  m_zoneShortName = "";
  m_minX = -50;
  m_maxX = 50;
  m_minY = -50;
  m_maxY = 50;
  updateBounds();
  m_lLines.clear();
  m_mLines.clear();
  m_locations.clear();
  m_aggros.clear();

  m_mapLoaded = false;
  m_imageLoaded = false;
  
  m_editLineM = NULL;
  m_editLocation = NULL;
  m_zoneZEM = 75;
}

#define	MAX_LINE_LENGTH		16384

void MapData::loadMap(const QString& fileName)
{
  FILE* fh;
  char line    [MAX_LINE_LENGTH];
  char tempstr [MAX_LINE_LENGTH];
  char* tempStr;
  int mx, my, mz;
  QPoint lPoint;
  MapPoint mPoint;
  uint numPoints;
  int globHeight=0;
  bool globHeightSet = false;
  int filelines = 1;  // number of lines in map file
  QString name;
  QString color;
  uint16_t rangeVal;
  uint32_t linePoints;
  MapLineL* currentLineL = NULL;
  MapLineM* currentLineM = NULL;

  // if the same map is already loaded, don't reload it.
  if (m_mapLoaded && (m_fileName.lower() == fileName.lower()))
    return;

  // set the map filename
  setFileName(fileName);

  // clear any existing map data
  clear();

  const char* filename = (const char*)fileName;

  fh = fopen ((const char*)filename, "r");

  /* Perhaps some lacky forgot to use the correct case for the map name?
     
     Let's try dumping a list of files in the map directory and comparing
     (wihout regard to case) the filename with other map files... */
  if (errno == ENOENT)
  {
    struct dirent **eps;
    int n;
    
    n = scandir (MAPDIR, &eps, NULL, alphasort);
    if (n >= 0)
    {
      for (int cnt = 0; cnt < n; ++cnt)
      {
	if (strstr(eps[cnt]->d_name, ".map") > 0)
	{
	  char chEntry[128];
	  
	  sprintf(chEntry, "%s/%s", MAPDIR, eps[cnt]->d_name);

	  if (strcasecmp (chEntry, filename) == 0)
	  {
	    filename = chEntry;
	    fh = fopen(filename, "r");
	    break;
	  }
	}
      }
    }
  }

  /* Kind of stupid to try a non-existant map, don't you think? */
  if (strstr (filename, "/.map"))
    return;

  if (fh == NULL)
  {
    printf("Error opening map file '%s'!\n", filename);

    return;
  }

  // note the file name 
  m_fileName = filename;

  // read the first line
  if (fgets(line, MAX_LINE_LENGTH, fh) == NULL) 
  {
    printf("Error reading map file '%s'!\n", filename);
    return;
  }
  filelines = 1;

#ifdef DEBUGMAPLOAD
  printf("Zone info line: %s\n", line);
#endif

  // Zone name
  tempStr = strtok(line, ",");
  if (tempStr == NULL) 
  {
    fprintf(stderr, "Error reading zone name in map '%s'!\n", filename);
    return;
  }
  m_zoneLongName = tempStr;

  // Zone short name
  tempStr = strtok(NULL, ",");
  if (tempStr == NULL) 
  {
    fprintf(stderr, "Error reading short name in map '%s'!\n", filename);
    return;
  }
  m_zoneShortName = tempStr;
  
  // Zone size in X direction
  tempStr = strtok(NULL, ",");
  if (tempStr == NULL) 
  {
    fprintf(stderr, "Error reading X size in map '%s'!\n", filename);
    return;
  }
  m_size.setWidth(atoi(tempStr));

  // Zone size in Y direction
  tempStr = strtok(NULL, "\n");
  if (tempStr == NULL) 
  {
    fprintf(stderr, "Error reading Y size in map '%s'!\n", filename);
    return;
  }
  m_size.setHeight(atoi(tempStr));

    // For each line of file
  while (fgets(line, MAX_LINE_LENGTH, fh) != NULL)
  {
    // increment line count
    filelines++;
     
#ifdef DEBUGMAPLOAD
    printf("Map line %d: %s", filelines, line);
#endif
    strcpy (tempstr, strtok (line, ","));
    switch (tempstr[0]) 
    {
    case 'A':  //Creates aggro ranges.
      tempStr = strtok (NULL, ",\n");
      if (tempStr == NULL) 
      {
	fprintf(stderr, "Line %d in map '%s' has an A marker with no Name expression!\n", 
		filelines, filename);
	break;
      }
      name = tempStr;
      tempStr = strtok (NULL, ",\n");
      if (tempStr == NULL) 
      {
	fprintf(stderr, "Line %d in map '%s' has an A marker with no Range radius!\n", 
		filelines, filename);
	break;
      }
      rangeVal = atoi(tempStr);

      // create and add a new aggro object
      m_aggros.append(new MapAggro(name, rangeVal));

      break;
    case 'H':  //Sets global height for L lines.
      tempStr = strtok (NULL, ",\n");
      if (tempStr == NULL) 
      {
	fprintf(stderr, "Line %d in map '%s' has an H marker with no Z!\n", 
		filelines, filename);
	break;
      }
      globHeight = atoi(tempStr);
      globHeightSet = true;
      break;
    case 'Z':  // Quick and dirty ZEM implementation
      tempStr = strtok (NULL, ",\n");
      if (tempStr == NULL) 
      {
	fprintf(stderr, "Line %d in map '%s' has an Z marker with no ZEM!\n", 
		filelines, filename);
	break;
      }
      m_zoneZEM = atoi(tempStr);
#ifdef DEBUGMAPLOAD
      printf("ZEM set to %d\n", m_zoneZEM);
#endif
      break;
    case 'L':
#ifdef DEBUGMAPLOAD
      printf("L record  [%d]: %s\n", filelines, line);
#endif
      // Line Name
      name = strtok(NULL, ",");
      if (name.isNull()) 
      {
	fprintf(stderr, "Error reading line name on line %d in map '%s'!\n", 
		filelines, filename);
	continue;
      }
      
      // Line Color
      color = strtok(NULL, ",");
      if (color.isEmpty()) 
      {
	fprintf(stderr, "Error reading line color on line %d in map '%s'!\n", 
		filelines, filename);
	continue;
      }
      
      // Number of points
      tempStr = strtok (NULL, ",");
      if (tempStr == NULL) 
      {
	fprintf(stderr, "Error reading number of points on line %d in map '%s'!\n",
		filelines, filename);
	continue;
      }
      linePoints = atoi(tempStr);

      // create the appropriate style L line depending on if the global 
      // height has been set
      if (globHeightSet)
	currentLineL = new MapLineL(name, color, linePoints, globHeight);
      else
	currentLineL = new MapLineL(name, color, linePoints);

      numPoints = 0;
      while (1) 
      {
	lPoint.setX(0);
	lPoint.setY(0);
	
	// X coord
	tempStr = strtok (NULL, ",\n");
	if (tempStr == NULL)
	  break;
	mx = atoi(tempStr);
	
	// Y coord
	tempStr = strtok (NULL, ",\n");
	if (tempStr == NULL) 
	{
	  fprintf(stderr, "Line %d in map '%s' has an X coordinate with no Y!\n", 
		  filelines, filename);
	  continue;
	}
	my = atoi(tempStr);
	
	// make sure there is space for the point
	if (numPoints >= currentLineL->size())
	  currentLineL->resize(2 * currentLineL->size());

	// store the point
	currentLineL->setPoint(numPoints, mx, my);

	// adjust map boundaries
	quickCheckPos(mx, my);

	numPoints++;
      }

      if (numPoints > linePoints) 
	fprintf(stderr, "L Line %d in map '%s' has more points than specified!\n", 
		filelines, filename);
      else if (numPoints < linePoints) 
	fprintf(stderr, "L Line %d in map '%s' has fewer points than specified!\n",
		filelines, filename);

      // if there are no points, or just the obligatory point from someone just
      // hitting the line button, don't bother adding the line
      if (numPoints < 2)
      {
	fprintf(stderr, "L Line %d in map '%s' only had %d points, not loading.\n",
		filelines, filename, numPoints);

	// delete the line
	delete currentLineL;

	// next...
	continue;
      }

      // remove any extras to save memory
      if (numPoints < currentLineL->size())
	currentLineL->resize(numPoints);

      // calculate the XY bounds of the line
      currentLineL->calcBounds();

      // add it to the list of L lines
      m_lLines.append(currentLineL);

      break;
      
    case 'M':
#ifdef DEBUGMAPLOAD
      printf("M record  [%d]: %s\n", filelines, line);
#endif
      // Line Name
      name = strtok(NULL, ",");
      if (name.isNull()) 
      {
	fprintf(stderr, "Error reading line name on line %d in map '%s'!\n", 
		filelines, filename);
	continue;
      }
      
      // Line Color
      color = strtok(NULL, ",");
      if (color.isEmpty()) 
      {
	fprintf(stderr, "Error reading line color on line %d in map '%s'!\n", 
	       filelines, filename);
	continue;
      }
      
      // Number of points
      tempStr = strtok (NULL, ",");
      if (tempStr == NULL) 
      {
	fprintf(stderr, "Error reading number of points on line %d in map '%s'!\n", 
		filelines, filename);
	continue;
      }
      linePoints = atoi(tempStr);

      // make sure twit didn't pass 0
      if (linePoints == 0)
	linePoints = 1;

      // create an M line
      currentLineM = new MapLineM(name, color, linePoints);

      numPoints = 0;
      while (1) 
      {
	mx = my = mz = 0;
	
	// X coord
	tempStr = strtok (NULL, ",\n");
	if (tempStr == NULL)
	  break;
	mx = atoi(tempStr);
	
	// Y coord
	tempStr = strtok (NULL, ",\n");
	if (tempStr == NULL) 
	{
	  fprintf(stderr, "Line %d in map '%s' has an X coordinate with no Y!\n", 
		  filelines, filename);
	  continue;
	}
	my = atoi(tempStr);
	
	// Z coord
	tempStr = strtok (NULL, ",\n");
	if (tempStr == NULL) 
	{
	  fprintf(stderr, "Line %d in map '%s' has X and Y coordinates with no Z!\n", 
		  filelines, filename);
	  continue;
	}
	mz = atoi(tempStr);

	// expand the line if necessary
	if (numPoints >= currentLineM->size())
	  currentLineM->resize(2 * currentLineM->size());

	// set the point data
	currentLineM->setPoint(numPoints, mx, my, mz);

	// adjust map boundaries
	quickCheckPos(mx, my);

	// increment point count
	numPoints++;
      }

      // verify number of points, versus those specified
      if (numPoints > linePoints) 
	fprintf(stderr, "M Line %d in map '%s' has more points than specified!\n", 
		filelines, filename);
      else if (numPoints < linePoints) 
	fprintf(stderr, "M Line %d in map '%s' has fewer points than specified!\n",
		filelines, filename);

      // if there are no points, or just the obligatory point from someone just
      // hitting the line button, don't bother adding the line
      if (numPoints < 2)
      {
	fprintf(stderr, "L Line %d in map '%s' only had %d points, not loading.\n",
		filelines, filename, numPoints);

	// delete the line
	delete currentLineL;

	// next...
	continue;
      }
      
      // remove any extras to save memory
      if (numPoints < currentLineM->size())
	currentLineM->resize(numPoints);

      // calculate the XY bounds of the line
      currentLineM->calcBounds();

      // add it to the list of L lines
      m_mLines.append(currentLineM);

      break;
	
    case 'P':
#ifdef DEBUGMAPLOAD
      printf("P record  [%d]: %s\n", filelines, line);
#endif
      
      name = strtok (NULL,","); // Location name
      color = strtok(NULL, ","); // Location color
      mx = atoi (strtok (NULL, ",\n"));
      my = atoi (strtok (NULL, ",\n"));

      // add it to the list of locations
      m_locations.append(new MapLocation(name, color, mx, my));

      // adjust map boundaries
      quickCheckPos(mx, my);

      break;
    }
  }
  fclose (fh);

  // calculate the bounding rect
  updateBounds();

  m_mapLoaded = true;
  
  m_imageLoaded = false;
  QString imageFileName = filename;
  imageFileName.truncate(imageFileName.findRev('.'));
  imageFileName += ".pgm";

  if (m_image.load(imageFileName))
  {
    m_imageLoaded = true;
    printf("Loaded map image: '%s'\n", (const char*)imageFileName);
  }

  printf("Loaded map: '%s'\n", filename);
}

void MapData::saveMap() const
{
#ifdef DEBUG
  debug ("saveMap()");
#endif /* DEBUG */
  FILE * fh;
  uint32_t i;

  const char* filename = (const char*)m_fileName;

  if ((fh = fopen(filename, "w")) == NULL) 
  {
    printf("Error saving map '%s'!\n", filename);
    return;
  }
  
  // write out header info
  fprintf(fh, "%s,%s,%d,%d\n", 
	  (const char*)m_zoneLongName, (const char*)m_zoneShortName,
	  m_size.width(), m_size.height());

  // write out ZEM info if non-default
  if (m_zoneZEM != 75)
    fprintf(fh, "Z,%i\n", m_zoneZEM);

  // write out the L (2D) lines with possible fixed Z
  bool heightSet = false;
  int16_t lastHeightSet = 0;
  MapLineL* currentLineL;
  QListIterator<MapLineL> mlit(m_lLines);
  for (currentLineL = mlit.toFirst(); 
       currentLineL != NULL; 
       currentLineL = ++mlit)
  {
    // was the global height set?
    if (currentLineL->heightSet())
    {
      // if no height was set previously, or this one doesn't match the previously
      // set height, write out an H record
      if ((!heightSet) || (lastHeightSet != currentLineL->zPos()))
      {
	// write out an H record.
	fprintf(fh, "H,%i\n", currentLineL->zPos());

	// note the last height set, and that it was set
	lastHeightSet = currentLineL->zPos();
	heightSet = true;
      }
    }

    // write out the start of the line info
    fprintf (fh, "L,%s,%s,%d", 
	     (const char*)currentLineL->name(), 
	     (const char*)currentLineL->color(), 
	     currentLineL->size());

    // write out all the 2D points in the line
    for(i = 0; i < currentLineL->size(); i++)
    {
      QPoint curQPoint = currentLineL->point(i);
      fprintf (fh, ",%d,%d", curQPoint.x(), curQPoint.y());
    }

    // terminate the line
    fprintf (fh, "\n");
  }

  // write out the M (3D) lines
  MapLineM* currentLineM;
  QListIterator<MapLineM> mmit(m_mLines);
  for (currentLineM = mmit.toFirst(); 
       currentLineM; 
       currentLineM = ++mmit)
  {
    // write out the start of the line info
    fprintf (fh, "M,%s,%s,%d", 
	     (const char*)currentLineM->name(), 
	     (const char*)currentLineM->color(), 
	     currentLineM->size());

    // write out all the 3D points in the line
    for(i = 0; i < currentLineM->size(); i++)
    {
      MapPoint curMPoint = currentLineM->point(i);

      fprintf (fh, ",%d,%d,%d", 
	       curMPoint.xPos(), curMPoint.yPos(), curMPoint.zPos());
    }
    // terminate the line
    fprintf (fh, "\n");
  }

  // write out location information
  QListIterator<MapLocation> lit(m_locations);
  for(; lit.current(); ++lit)
  {
    MapLocation* currentLoc = lit.current();

    fprintf (fh, "P,%s,%s,%d,%d\n", 
	     (const char*)currentLoc->name(), 
	     (const char*)currentLoc->color(), 
	     currentLoc->x(),
	     currentLoc->y());
  }

  // write out aggro information
  QListIterator<MapAggro> ait(m_aggros);
  for (; ait.current(); ++ait)
  {
    MapAggro* currentAggro = ait.current();
    
    fprintf (fh, "A,%s,%d\n", 
	     (const char*)currentAggro->name(), currentAggro->range());
  }

#ifdef DEBUGMAP
  printf("saveMap() - map '%s' saved with %d L lines, %d M lines, %d locations\n", filename,
	 m_lLines.count(), m_mLines.count(), m_locations.count());
#endif
  
  fclose (fh);
}

bool MapData::isAggro(const QString& name, uint16_t* range) const
{
  MapAggro* aggro;
  QListIterator<MapAggro> ait(m_aggros);
  for (aggro = ait.toFirst();
       aggro != NULL;
       aggro = ++ait)
  {
    // does the name match this aggro?
    if (name.find(aggro->name(), 0, false) != -1)
    {
      if (range != NULL)
	*range = aggro->range();

      return true;
    }
  }

  return false;
}

void MapData::addLocation(const QString& name, 
			  const QString& color, 
			  const QPoint& point)
{
  // create the new location
  m_editLocation = new MapLocation(name, color, point);
  
  // add it to the list of locations
  m_locations.append(m_editLocation);
}

void MapData::setLocationName(const QString& name)
{
  // make sure there is a location to edit
  if (m_editLocation == NULL)
    return;

  // set the location name
  m_editLocation->setName(name);
}

void MapData::setLocationColor(const QString& color)
{
  // make sure there is a location to edit
  if (m_editLocation == NULL)
    return;

  // set the location color
  m_editLocation->setColor(color);
}

void MapData::startLine(const QString& name, 
			const QString& color, 
			const MapPoint& point)
{
  // create the new line, with just the first point
  m_editLineM = new MapLineM(name, color, point); 

  // calculate the XY bounds of the line
  m_editLineM->calcBounds();

  // add line to the line list
  m_mLines.append(m_editLineM);
}

void MapData::addLinePoint(const MapPoint& point)
{
  // make sure there is a line to add to
  if (m_editLineM == NULL)
    return;

  uint32_t pos = m_editLineM->size();

  // increase the size of the line by one point
  m_editLineM->resize(pos + 1);
  
  // set the point data
  m_editLineM->setPoint(pos, point);

  // calculate the XY bounds of the line
  m_editLineM->calcBounds();
}

void MapData::delLinePoint(void)
{
  // make sure there is a line to add to
  if (m_editLineM == NULL)
    return;

  // remove the last entry from the line
  m_editLineM->resize(m_editLineM->size() - 1);

  // if the user has deleted that last point in the line, delete the line
  if (m_editLineM->size() == 0)
  {
    // remove the line
    m_mLines.remove(m_editLineM);

    // clear the currently edited line entry
    m_editLineM = NULL;
  }
  else
  {
    // calculate the XY bounds of the line
    m_editLineM->calcBounds();
  }
}

void MapData::setLineName(const QString& name)
{
  // make sure there is a line to add to
  if (m_editLineM == NULL)
    return;

  // set the line name
  m_editLineM->setName(name);
}

void MapData::setLineColor(const QString& color)
{
  // make sure there is a line to add to
  if (m_editLineM == NULL)
    return;

  // set the line color
  m_editLineM->setColor(color);
}

void MapData::paintGrid(MapParameters& param, QPainter& p) const
{
  // if nothing will be drawn, don't go through the motions
  if (!param.showGridLines() && !param.showGridTicks())
    return;

  /* Paint the grid */

  // set the brush
  p.setBrush(QColor (80, 80, 80));

  // Need to put in some stuff to auto resize the gridres
  // based on the screenLength and map size
  int gridres = param.gridResolution();
  int offsetPos;
  int pos;
  const QRect& screenBounds = param.screenBounds();
  int lastGrid = (maxX() / gridres) + 1;

  // start from the minimum position and increment to the last grid position
  for (int gx = (minX() / gridres) - 1; 
       gx <= lastGrid; 
       gx++)
  {
    // calculate position
    pos = gx * gridres;
    
    // haven't reached visible portion yet, continue
    if (screenBounds.left() > pos)
      continue;

    // past the visible portion, stop
    if (screenBounds.right() < pos)
      break;

    // calculate offset
    offsetPos = param.calcXOffsetI(pos);

    // if grid lines are show, draw them
    if (param.showGridLines())
    {
      p.setPen(param.gridLineColor());
      p.drawLine(offsetPos, 0,
		   offsetPos, param.screenLengthY());
    }
    
    // if grid ticks are shown, draw them
    if (param.showGridTicks())
    {
      p.setPen(param.gridTickColor());
      p.drawText(offsetPos, param.screenLengthY(), QString::number(pos));
    }
  }

  lastGrid = (maxY() / gridres) + 1;

  // start from the minimum position and increment to the last grid position
  for (int gy = (minY() / gridres) - 1; 
       gy <= lastGrid; 
       gy++)
  {
    // calculate position
    pos = gy * gridres;

    // haven't reached visible portion yet, continue
    if (screenBounds.top() > pos)
      continue;

    if (screenBounds.bottom() < pos)
      break;

    // calculate the offset position
    offsetPos = param.calcYOffsetI(pos);

    // if grid lines are shown, draw them
    if (param.showGridLines())
    {
      p.setPen(param.gridLineColor());
      p.drawLine(0, offsetPos,
		   param.screenLengthX(), offsetPos);
    }
    
    // if grid ticks are shown, draw thm
    if (param.showGridTicks())
    {
      p.setPen(param.gridTickColor());
      p.drawText(0, offsetPos, QString::number(pos));
    }

  }  
}

void MapData::paintLines(MapParameters& param, QPainter& p) const
{
  //----------------------------------------------------------------------
  /* Paint the lines */
#ifdef DEBUGMAP
  printf("Paint the lines\n");
#endif
  // Note: none of the map loops below check for zero length lines,
  // because all line manipulation code makes sure that they don't occur

  // stuff used no matter how the map is drawn
  MapLineL* currentLineL;
  MapLineM* currentLineM;

  // set the brush
  p.setBrush(QColor (80, 80, 80));

  const QRect& screenBounds = param.screenBounds();

  // determine which painting method is to be used.
  // no depth filtering, cool, let's make this quick and easy
  bool lastInBounds;
  bool curInBounds;
  int16_t curX, curY;
  int cur2DX, cur2DY;
  uint numPoints;
  QPoint* lData;
  MapPoint* mData;
  
  // first paint the L lines
  QListIterator<MapLineL> mlit(m_lLines);
  for (currentLineL = mlit.toFirst(); 
       currentLineL != NULL; 
       currentLineL = ++mlit)
  {
    // if line is outside the currently visible region, skip it.
    if (!currentLineL->boundingRect().intersects(screenBounds))
      continue;
    
    // get the number of points in the line
    numPoints = currentLineL->size();
    
    // get the underlying array
    lData = currentLineL->data();
    
    // set pen color
#ifdef DEBUGMAP
    printf("lineColor = '%s'\n", (char *) currentLineL->color());
#endif
    p.setPen(currentLineL->color());
    
    cur2DX = lData[0].x();
    cur2DY = lData[0].y();
    
    // see if the starting position is in bounds
    lastInBounds = inRect(screenBounds, cur2DX, cur2DY);
    
    // move to the starting position
    p.moveTo(param.calcXOffsetI(cur2DX), 
	     param.calcYOffsetI(cur2DY));
    
    // iterate over all the points in the line
    for (uint32_t i = 1; i < numPoints; i++)
    {
      cur2DX = lData[i].x();
      cur2DY = lData[i].y();
      
      // determine if the current position is in bounds
      curInBounds = inRect(screenBounds, cur2DX, cur2DY);
      
      // draw the line segment if either end is in bounds
      if (lastInBounds || curInBounds)
	p.lineTo(param.calcXOffsetI(cur2DX),
		 param.calcYOffsetI(cur2DY));
      else
	p.moveTo(param.calcXOffsetI(cur2DX),
		 param.calcYOffsetI(cur2DY));
      
      // current becomes the last
      lastInBounds = curInBounds;
    }
  }
  
  // then paint the M lines
  QListIterator<MapLineM> mmit(m_mLines);
  for (currentLineM = mmit.toFirst(); 
       currentLineM; 
       currentLineM = ++mmit)
  {
    // if line is outside the currently visible region, skip it.
    if (!currentLineM->boundingRect().intersects(screenBounds))
      continue;
    
    // get the number of points in the line
    numPoints = currentLineM->size();
    
    // get the underlying array
    mData = currentLineM->data();
    
    // set pen color
#ifdef DEBUGMAP
    printf("lineColor = '%s'\n", (char *) currentLineM->color());
#endif
    p.setPen(currentLineM->color());
    
    curX = mData[0].x();
    curY = mData[0].y();
    
    // see if the starting position is in bounds
    lastInBounds = inRect(screenBounds, curX, curY);
    
    // move to the starting position
    p.moveTo(param.calcXOffsetI(curX), 
	     param.calcYOffsetI(curY));
    
    // iterate over all the points in the line
    for (uint32_t i = 1; i < numPoints; i++)
    {
      curX = mData[i].x();
      curY = mData[i].y();
      
      // determine if the current position is in bounds
      curInBounds = inRect(screenBounds, curX, curY);
      
      // draw the line segment if either end is in bounds
      if (lastInBounds || curInBounds)
	p.lineTo(param.calcXOffsetI(curX),
		 param.calcYOffsetI(curY));
      else
	p.moveTo(param.calcXOffsetI(curX),
		 param.calcYOffsetI(curY));
      
      // current becomes the last
      lastInBounds = curInBounds;
    }
  }
}

void MapData::paintDepthFilteredLines(MapParameters& param, QPainter& p) const
{
  //----------------------------------------------------------------------
  /* Paint the lines */
#ifdef DEBUGMAP
  printf("Paint Depth Filtered lines\n");
#endif
  // Note: none of the map loops below check for zero length lines,
  // because all line manipulation code makes sure that they don't occur

  // stuff used no matter how the map is drawn
  MapLineL* currentLineL;
  MapLineM* currentLineM;

  // set the brush
  p.setBrush(QColor (80, 80, 80));

  const QRect& screenBounds = param.screenBounds();

  // map depth filtering, without faded floors
  bool lastInBounds;
  bool curInBounds;
  int16_t curX, curY, curZ;
  int cur2DX, cur2DY;
  uint32_t numPoints;
  QPoint* lData;
  MapPoint* mData;
  
  // get the players position for it's Z information
  MapPoint playerPos = param.player();

  // first paint the L lines
  QListIterator<MapLineL> mlit(m_lLines);
  for (currentLineL = mlit.toFirst(); 
       currentLineL != NULL; 
       currentLineL = ++mlit)
  {
    // if line is outside the currently visible region, skip it.
    if (!currentLineL->boundingRect().intersects(screenBounds))
      continue;
    
    // since it's an L type line, check for the depth is easy
    // just check if height is set, and if so, check if it's within range
    if (currentLineL->heightSet() && 
	!inRoom(param.playerHeadRoom(), param.playerFloorRoom(), 
		currentLineL->zPos()))
      continue;  // outside of range, continue to the next line
    
    // get the number of points in the line
    numPoints = currentLineL->size();
    
    // get the underlying array
    lData = currentLineL->data();
    
    // set the line color
#ifdef DEBUGMAP
    printf("lineColor = '%s'\n", (char *) currentLineL->color());
#endif
    p.setPen(currentLineL->color());
    
    cur2DX = lData[0].x();
    cur2DY = lData[0].y();
    
    // see if the starting position is in bounds
    lastInBounds = inRect(screenBounds, cur2DX, cur2DY);
    
    // move to the starting position
    p.moveTo(param.calcXOffsetI(cur2DX), 
	     param.calcYOffsetI(cur2DY));
    
    // iterate over all the points in the line
    for (uint32_t i = 1; i < numPoints; i++)
    {
      cur2DX = lData[i].x();
      cur2DY = lData[i].y();
      
      // determine if the current position is in bounds
      curInBounds = inRect(screenBounds, cur2DX, cur2DY);
      
      // draw the line segment if either end is in bounds
      if (lastInBounds || curInBounds)
	p.lineTo(param.calcXOffsetI(cur2DX),
		 param.calcYOffsetI(cur2DY));
      else
	p.moveTo(param.calcXOffsetI(cur2DX),
		 param.calcYOffsetI(cur2DY));
      
      // current becomes the last
      lastInBounds = curInBounds;
    }
  }
  
  // then paint the M lines
  QListIterator<MapLineM> mmit(m_mLines);
  for (currentLineM = mmit.toFirst(); 
       currentLineM; 
       currentLineM = ++mmit)
  {
    // if line is outside the currently visible region, skip it.
    if (!currentLineM->boundingRect().intersects(screenBounds))
      continue;
    
    // get the number of points in the line
    numPoints = currentLineM->size();
    
    // get the underlying array
    mData = currentLineM->data();
    
    // set the line color
#ifdef DEBUGMAP
    printf("lineColor = '%s'\n", (char *) currentLineM->color());
#endif
    p.setPen(currentLineM->color());
    
    // get current coordinates
    curX = mData[0].x();
    curY = mData[0].y();
    curZ = mData[0].z();

    // see if the starting position is in bounds
    lastInBounds = (inRect(screenBounds, curX, curY) &&
		    inRoom(param.playerHeadRoom(), param.playerFloorRoom(),
			   curZ));
    
#ifdef DEBUGMAP
    printf("Line has %i points:\n", currentLineM->size());
#endif
    
    // move to the starting position
    p.moveTo(param.calcXOffsetI(curX), 
	     param.calcYOffsetI(curY));
    
    // iterate over all the points in the line
    for (uint32_t i = 1; i < numPoints; i++)
    {
      // get current coordinates
      curX = mData[i].x();
      curY = mData[i].y();
      curZ = mData[0].z();
      
      // determine if the current position is in bounds
      curInBounds = (inRect(screenBounds, curX, curY) &&
		     inRoom(param.playerHeadRoom(), param.playerFloorRoom(),
			    curZ));
      
      // draw the line segment if either end is in bounds
      if (lastInBounds || curInBounds)
	p.lineTo(param.calcXOffsetI(curX),
		 param.calcYOffsetI(curY));
      else
	p.moveTo(param.calcXOffsetI(curX),
		 param.calcYOffsetI(curY));
      
      // current becomes the last
      lastInBounds = curInBounds;
    }
  }
}

void MapData::paintFadedFloorsLines(MapParameters& param, QPainter& p) const
{
  //----------------------------------------------------------------------
  /* Paint the lines */
#ifdef DEBUGMAP
  printf("Paint Faded Floor lines\n");
#endif
  // Note: none of the map loops below check for zero length lines,
  // because all line manipulation code makes sure that they don't occur

  // stuff used no matter how the map is drawn
  MapLineL* currentLineL;
  MapLineM* currentLineM;

  // set the brush
  p.setBrush(QColor (80, 80, 80));

  const QRect& screenBounds = param.screenBounds();

  // depth filtering with faded floors
  int oldColor, newColor, useColor;
  bool lastInBounds;
  bool curInBounds;
  int16_t curX, curY, curZ;
  int cur2DX, cur2DY, cur2DZ;
  uint32_t numPoints;
  QPoint* lData;
  MapPoint* mData;
  
  // get the players position for it's Z information
  MapPoint playerPos = param.player();
  
  double topm = 0 - 255.0 / (double)param.headRoom();
  double botm = 255.0 / (double)param.floorRoom();
  double topb = 255 - (topm * playerPos.zPos());
  double botb = 255 - (botm * playerPos.zPos());
  
  // first paint the L lines
  QListIterator<MapLineL> mlit(m_lLines);
  for (currentLineL = mlit.toFirst(); 
       currentLineL != NULL; 
       currentLineL = ++mlit)
  {
    // if line is outside the currently visible region, skip it.
    if (!currentLineL->boundingRect().intersects(screenBounds))
      continue;
    
    // get the number of points in the line
    numPoints = currentLineL->size();
    
    // get the underlying array
    lData = currentLineL->data();
    
    // get first point coordinates
    cur2DX = lData[0].x();
    cur2DY = lData[0].y();
    cur2DZ = currentLineL->zPos();
    
    // color determination is different depending on if a height was set
    if (!currentLineL->heightSet())
    {
      // set the line color
#ifdef DEBUGMAP
      printf("lineColor = '%s'\n", (char *) currentLineL->color());
#endif
      p.setPen(currentLineL->color());
    }
    else
    {
      // calculate color to use for the line (since L type, only do this once)
      if (currentLineL->zPos() > playerPos.zPos())
	useColor = (int)((cur2DZ * topm) + topb);
      else 
	useColor = (int)((cur2DZ * botm) + botb);
      
      if (useColor > 255) useColor = 255;
      if (useColor < 0) useColor = 0;
      
      // set the line color
#ifdef DEBUGMAP
      printf("lineColor = '#%2x%2x%2x'\n", useColor, useColor, useColor);
#endif
      p.setPen(QColor(useColor, useColor, useColor));
    }
    
    // see if the starting position is in bounds
    lastInBounds = inRect(screenBounds, cur2DX, cur2DY);
    
    // move to the starting position
    p.moveTo(param.calcXOffsetI(cur2DX), 
	     param.calcYOffsetI(cur2DY));
    
    // iterate over all the points in the line
    for (uint32_t i = 1; i < numPoints; i++)
    {
      // get coordinates
      cur2DX = lData[i].x();
      cur2DY = lData[i].y();
      
      // determine if the current position is in bounds
      curInBounds = inRect(screenBounds, cur2DX, cur2DY);
      
      // draw the line segment if either end is in bounds
      if (lastInBounds || curInBounds)
	p.lineTo(param.calcXOffsetI(cur2DX),
		 param.calcYOffsetI(cur2DY));
      else
	p.moveTo(param.calcXOffsetI(cur2DX),
		 param.calcYOffsetI(cur2DY));
      
      // current becomes the last
      lastInBounds = curInBounds;
    }
  }
  
  // then paint the M lines
  QListIterator<MapLineM> mmit(m_mLines);
  for (currentLineM = mmit.toFirst(); 
       currentLineM; 
       currentLineM = ++mmit)
  {
    // if line is outside the currently visible region, skip it.
    if (!currentLineM->boundingRect().intersects(screenBounds))
      continue;
    
    // get the number of points in the line
    numPoints = currentLineM->size();
    
    // get the underlying array
    mData = currentLineM->data();
    
    // set the line color
#ifdef DEBUGMAP
    printf("lineColor = '%s'\n", (char *) currentLineM->color());
#endif
    p.setPen(currentLineM->color());
    
    // get starting point coordinates
    curX = mData[0].x();
    curY = mData[0].y();
    curZ = mData[0].z();
    
    // see if the starting position is in bounds
    lastInBounds = inRect(screenBounds, curX, curY);
    
#ifdef DEBUGMAP
    printf("Line has %i points:\n", currentLineM->size());
#endif
    
    // calculate starting color info for the line
    if (curZ > playerPos.zPos()) 
      oldColor = (int)((curZ * topm) + topb);
    else 
      oldColor = (int)((curZ * botm) + botb);
    
    if (oldColor > 255) oldColor = 255;
    if (oldColor < 0) oldColor = 0;
    
    // move to the starting position
    p.moveTo(param.calcXOffsetI(curX), 
	     param.calcYOffsetI(curY));
    
    // iterate over all the points in the line
    for (uint i = 1; i < numPoints; i++)
    {
      curX = mData[i].x();
      curY = mData[i].y();
      curZ = mData[i].z();
      
      // calculate the new color
      if (curZ > playerPos.zPos()) 
	newColor = (int)((curZ * topm) + topb);
      else 
	newColor = (int)((curZ * botm) + botb);
      if (newColor > 255) newColor = 255;
      if (newColor < 0) newColor = 0;
      
      // determine if the current position is in bounds
      curInBounds = inRect(screenBounds, curX, curY);
      
      // the use color is the average of the two colors
      useColor = (newColor + oldColor) / 2;
      
      // draw the line segment if either end is in bounds
      if ((lastInBounds || curInBounds) && (useColor != 0))
      {
	p.setPen(QColor(useColor, useColor, useColor));
	p.lineTo(param.calcXOffsetI(curX),
		 param.calcYOffsetI(curY));
      }
      else
	p.moveTo(param.calcXOffsetI(curX),
		 param.calcYOffsetI(curY));
      
      // current becomes the last/old
      lastInBounds = curInBounds;
      oldColor = newColor;
    }
  }
}

void MapData::paintLocations(MapParameters& param, QPainter& p) const
{
  //----------------------------------------------------------------------
  /* Paint the locations */
#ifdef DEBUGMAP
  printf("Paint the locations\n");
#endif
  // set the brush
  p.setBrush(QColor (80, 80, 80));

  // set the font
  p.setFont(param.font());

  // iterate over all the map locations
  QListIterator<MapLocation> lit(m_locations);
  for(; lit.current(); ++lit)
  {
    MapLocation* currentLoc = lit.current();

    // set the color
    QColor color(currentLoc->color());

    // make sure the color isn't the same as the background color
    if (color == param.backgroundColor())
      color = QColor( ~ param.backgroundColor().rgb());

    // set the pen color
    p.setPen(color);

    // draw the text
    p.drawText(param.calcXOffsetI(currentLoc->x()) - 2,
	       param.calcYOffsetI(currentLoc->y()) - 2, 
	       currentLoc->name());
  }
}

bool MapData::paintMapImage(MapParameters& param, QPainter& p) const
{
  p.save();

  double scaleX = (double(param.screenLength().width()) / 
		   double(m_image.width())) * double(param.zoom());
  double scaleY = (double(param.screenLength().height()) / 
		   double(m_image.height())) * double(param.zoom());

  if (scaleX > 3.0 || scaleY > 3.0)
    return false;

  p.scale(scaleX, scaleY);

  int x = param.calcXOffset(m_maxX);
  int y = param.calcYOffset(m_maxY);

  p.drawPixmap(x, y, m_image);
  p.restore();

  return true;
}

//----------------------------------------------------------------------
// MapCache
MapCache::MapCache(const MapData& mapData)
  : m_mapData(mapData),
    m_lastParam(mapData)
{
  m_paintCount = 0;
  m_painted = false;
  m_alwaysRepaint = false;
}

MapCache::~MapCache()
{
}

bool MapCache::needRepaint(MapParameters& param)
{
  // if any of these conditions are true, then a repaint is needed
  // NOTE: May need to add more conditions
  if (!m_painted || m_alwaysRepaint ||
      (m_lastParam.screenCenter() != param.screenCenter()) ||
      (m_lastParam.screenLength() != param.screenLength()) ||
      (m_lastParam.ratio() != param.ratio()) ||
      (m_lastParam.zoomMapLength() != param.zoomMapLength()) ||
      (m_lastParam.screenBounds() != param.screenBounds() ) ||
      ((param.mapLineStyle() == tMap_FadedFloors) && 
       (m_lastParam.player().zPos() != param.player().zPos())) ||
      ((param.mapLineStyle() == tMap_DepthFiltered) &&
       ((m_lastParam.playerHeadRoom() != param.playerHeadRoom()) ||
	(m_lastParam.playerFloorRoom() != param.playerFloorRoom()))) ||
      (m_lastParam.mapLineStyle() != param.mapLineStyle()) ||
      (m_lastParam.showLocations() != param.showLocations()) ||
      (m_lastParam.showLines() != param.showLines()) ||
      (m_lastParam.showGridLines() != param.showGridLines()) ||
      (m_lastParam.showGridTicks() != param.showGridTicks()) || 
      (m_lastParam.showBackgroundImage() != param.showBackgroundImage()) ||
      (m_lastParam.gridTickColor() != param.gridTickColor()) || 
      (m_lastParam.backgroundColor() != param.backgroundColor()) || 
      (m_lastParam.font() != param.font()))
    return true;

  // if none of the above conditions is true, no need to repaint.
  return false;
}

const QPixmap& MapCache::getMapImage(MapParameters& param)
{
  // only repaint the map if absolutely necessary.
  if (!needRepaint(param))
    return m_mapImage;

  // increment paint count
  m_paintCount++;

  // set pixmap optimization if it's changed
  if (m_lastParam.mapOptimizationMethod() !=
      param.mapOptimizationMethod())
    m_mapImage.setOptimization(param.pixmapOptimizationMethod());

  // make sure the map is the correct size
  m_mapImage.resize(param.screenLength());

  QPainter tmp;

  // Begin Painting
  tmp.begin (&m_mapImage);
  tmp.setPen (QPen::NoPen);
  tmp.setFont (QFont("Helvetica", 8, QFont::Normal));

  // load the background image or paint the background
  if (!m_mapData.imageLoaded() || 
      !param.showBackgroundImage() ||
      !m_mapData.paintMapImage(param, tmp))
  {
    // paint the map backdrop with the users background color for all 
    // map line styles except faded floor, which only really works with
    // a black background
    if (param.mapLineStyle() != tMap_FadedFloors)
      tmp.fillRect(m_mapImage.rect(), param.backgroundColor());
    else 
      tmp.fillRect(m_mapImage.rect(), Qt::black);
  }

  tmp.setPen (QColor (80, 80, 80));
  tmp.setBrush (QColor (80, 80, 80));

  //----------------------------------------------------------------------
  /* Paint the grid */
  if (param.showGridLines() || param.showGridTicks())
    m_mapData.paintGrid(param, tmp);

  //----------------------------------------------------------------------
  /* Paint the lines */
  if (param.showLines())
  {
    switch (param.mapLineStyle())
    {
    case tMap_Normal:
      m_mapData.paintLines(param, tmp);
      break;
    case tMap_DepthFiltered:
      m_mapData.paintDepthFilteredLines(param, tmp);
      break;
    case tMap_FadedFloors:
      m_mapData.paintFadedFloorsLines(param, tmp);
      break;
#ifdef DEBUGMAP
    default:
      fprintf(stderr, "Unknown Map Line Style: %d!\n", param.mapLineStyle());
      break;
#endif
    }
  }

  //----------------------------------------------------------------------
  /* Paint the locations */
  if (param.showLocations())
    m_mapData.paintLocations(param, tmp);

  // finished painting
  tmp.end();

  // note that painting has been done
  m_painted = true;

  // note parameters used to paint, for later comparison
  m_lastParam = param;

  // return the map image
  return m_mapImage;
}
