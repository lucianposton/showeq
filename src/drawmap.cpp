/*
 * drawmap.cpp
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C" {
#include <gd.h>
#include <gdfonts.h>
}

#include <qstring.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "cgiconv.h"
#include "util.h"
#include "main.h"

///////////////////////////////////
// TODO:
//  - Look into possibly using a QT based solution for generating
//    the pixmap
//  - Possibly make certain code common between here and the ShowEQ map
//    display
//  - Add options to control the display of the map (level, zoom, etc...)

// For when I'm doing research on this crazy thing...
// #define ZBTEMP 1

#define	MAX_LINES		8192
#define MAX_POINTS              1024

int locationColor [256];
QString locationName  [256];
int locationX[256];
int locationY[256];
short int lineType[MAX_LINES];
int linePoints[MAX_LINES];
int lineX[MAX_LINES][MAX_POINTS];
int lineY[MAX_LINES][MAX_POINTS];
int lineZ[MAX_LINES][MAX_POINTS];
int lineColor[MAX_LINES];

int maxPoints = 0;
int maxPointsReal = 0;
int maxX = 50;
int minX = -50;
int maxY = 50;
int minY = -50;
#ifdef ZBTEMP
int maxZ = 0;
int minZ = 0;
#endif
int mapLengthX = 100;
int mapLengthY = 100;
int numLocations = 0;
int numLines = 0;
int screenCenterX = 300;
int screenCenterY = 300;
int screenLengthX = 600;
int screenLengthY = 600;

char zoneLong[128];
char zoneShort[128];
int ZoomAmmount = 2;
int zoomMaxX;
int zoomMaxY;
int zoomMinX;
int zoomMinY;
int zoomMapLengthX;
int zoomMapLengthY;
int x=0;
int y=0;

void reAdjust ()
{
  int xoff = 0, yoff = 0;
  screenLengthX = 600;
  screenLengthY = 600;

  mapLengthX = maxX - minX;
  mapLengthY = maxY - minY;

  if (!mapLengthX)
    {
      mapLengthX = 5000;
      maxX = 2500;
      minX = -2500;
    }
  if (!mapLengthY)
    {
      mapLengthY = 5000;
      maxY = 2500;
      minY = -2500;
    }

  zoomMaxX = x + (mapLengthX / (ZoomAmmount));
  zoomMinX = x - (mapLengthX / (ZoomAmmount));

  zoomMaxY = y + (mapLengthY / (ZoomAmmount));
  zoomMinY = y - (mapLengthY / (ZoomAmmount));

  if (zoomMaxX > maxX)
    {
      zoomMinX -= (zoomMaxX - maxX);
      zoomMaxX -= (zoomMaxX - maxX);
    }

  if (zoomMaxY > maxY)
    {
      zoomMinY -= (zoomMaxY - maxY);
      zoomMaxY -= (zoomMaxY - maxY);
    }

  if (zoomMinX < minX)
    {
      zoomMaxX -= (zoomMinX - minX);
      zoomMinX -= (zoomMinX - minX);
    }

  if (zoomMinY < minY)
    {
      zoomMaxY -= (zoomMinY - minY);
      zoomMinY -= (zoomMinY - minY);
    }

  zoomMapLengthX = zoomMaxX - zoomMinX;
  zoomMapLengthY = zoomMaxY - zoomMinY;

  if (!zoomMapLengthX)
    {
      zoomMapLengthX = 5000;
      zoomMaxX = 2500;
      zoomMinX = -2500;
    }
  if (!mapLengthY)
    {
      zoomMapLengthY = 5000;
      zoomMaxY = 2500;
      zoomMinY = -2500;
    }

  if (zoomMapLengthX > zoomMapLengthY)
    {
      yoff = (zoomMapLengthX - zoomMapLengthY) / 2;
      xoff = 0;
      zoomMapLengthY = zoomMapLengthX;
    }
  else
    {
      yoff = 0;
      xoff = (zoomMapLengthY - zoomMapLengthX) / 2;
      zoomMapLengthX = zoomMapLengthY;
    }

  if (mapLengthX > mapLengthY)
    mapLengthY = mapLengthX;
  else
    mapLengthX = mapLengthY;

  screenCenterX = ((zoomMaxX * 600 + xoff * 600) / zoomMapLengthX);
  screenCenterY = ((zoomMaxY * 600 + yoff * 600) / zoomMapLengthY);
}

int lookupColor(const QString colorname)
{
  // this must be kept in sync with list in map painting class
  static const char* colornames[] = 
  {
    "grey",
    "darkblue",
    "darkgreen",
    "darkcyan",
    "darkred",
    "darkmagenta",
    "darkyellow",
    "darkgray",
    "white",
    "blue",
    "green",
    "cyan",
    "red",
    "magenta", 
    "yellow",
    "gray"
  };
  
  QString lcolorname = colorname.lower();

  for (uint32_t i = 0; i < sizeof(colornames)/sizeof(const char*); i++)
  {
    if (lcolorname == colornames[i])
      return i;
  }

  return 0;
}

void loadFileMap (const char *filename)
{
  FILE * fh;
  char line[4096];
  char tempstr[4096];
  char* tmpStr;
  int fileLines = 0;
  int globHeight = 0;

  maxX = 0;
  minX = 0;
  maxY = 0;
  minY = 0;
#ifdef ZBTEMP
  maxZ = 0;
  minZ = 0;
#endif

  numLocations = 0;
  numLines = 0;
  int numPoints = 0;

  if ((fh = fopen (filename, "r")) != NULL)
  {
    fgets (line, 4095, fh);
    tmpStr = strtok (line, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Zone Name!\n", filename);
      return;
    }
    strcpy (zoneLong, tmpStr);	// Zone name
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Short Zone Name!\n", filename);
      return;
    }
    strcpy (zoneShort, tmpStr);	// Zone short name
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Screen Center X!\n", filename);
      return;
    }
    screenCenterX = atoi (tmpStr);
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Screen Center Y!\n", filename);
      return;
    }
    screenCenterY = atoi (tmpStr);
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Screen Length X!\n", filename);
      return;
    }
    screenLengthX = atoi (tmpStr);
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Screen Length Y!\n", filename);
      return;
    }
    screenLengthY = atoi (tmpStr);
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Map Length X!\n", filename);
      return;
    }
    mapLengthX = atoi (tmpStr);
    tmpStr = strtok (NULL, ",");
    if (tmpStr == NULL)
    {
      fprintf(stderr, "Map '%s' appears to be seriously messed up! No Map Length Y!\n", filename);
      return;
    }
    mapLengthY = atoi (tmpStr);
    fileLines++;

    while (fgets (line, 4095, fh))
    {
      fileLines++;
      strcpy (tempstr, strtok (line, ","));
      switch (tempstr[0])
      {
      case 'H':
	tmpStr = strtok (NULL, ",\n");
	if (tmpStr == NULL)
	{
	  fprintf(stderr, "Line %d in map '%s' has an H marker with no Z!\n",
		  fileLines, filename);
	  break;
	}
	globHeight = atoi(tmpStr);
	break;
      case 'L':
      case 'M': // don't know if this is right, but what the hell...
	// L = 0, M = 1
	lineType[numLines] = tempstr[0] - 'L';
	// Line name
	tmpStr = strtok (NULL, ",");	
	if (tmpStr == NULL)
	{
	  fprintf(stderr, "Error reading line name on line %d in map '%s'\n", fileLines, filename);
	  break;
	}
	// Line color
	tmpStr = strtok (NULL, ",");
	if (tmpStr == NULL)
	{
	  fprintf(stderr, "Error reading line color on line %d in map '%s'\n", fileLines, filename);
	  break;
	}
	lineColor[numLines] = lookupColor (tmpStr);
	
	// Number of points
	tmpStr = strtok (NULL, ",");
	if (tmpStr == NULL)
	{
	  fprintf(stderr, "Error reading number of points on line %d in map '%s'\n", fileLines, filename);
	  break;
	}
	linePoints[numLines] = atoi (tmpStr);	
	if (linePoints[numLines] > maxPoints)
	  maxPoints = linePoints[numLines];

	numPoints = 0;
	while (1)
	{
	  tmpStr = strtok (NULL, ",\n");
	  if (tmpStr == NULL)
	    break;

	  lineX[numLines][numPoints] = atoi (tmpStr);
	  tmpStr = strtok (NULL, ",\n");
	  if (tmpStr == NULL)
	  {
	    fprintf(stderr, 
		    "Line %d in map '%s' has point %d with an X coordinate with no Y!\n",
		    fileLines, filename, numPoints);    
	    break;
	  }
	  lineY[numLines][numPoints] = atoi (tmpStr);
	  // currently if it's type M, just eat the Z coord
	  if (lineType[numLines] == 1)
	  {
	    tmpStr = strtok (NULL, ",\n");
	    if (tmpStr == NULL)
	    {
	      fprintf(stderr, 
		      "Line %d in map '%s' has point %d with an X & Y coordinates with no Z with lineType = M!\n",
		      fileLines, filename, numPoints);
	      // should probably abort, here, but hey, see if it works...
	      lineZ[numLines][numPoints] = globHeight;	     
	    }
	    else
	      lineZ[numLines][numPoints] = atoi (tmpStr);
	  }
	  else
	    lineZ[numLines][numPoints] = globHeight;
	  
	  if (lineX[numLines][numPoints] > maxX)
	    maxX = lineX[numLines][numPoints];
	  if (lineY[numLines][numPoints] > maxY)
	    maxY = lineY[numLines][numPoints];
	  if (lineX[numLines][numPoints] < minX)
	    minX = lineX[numLines][numPoints];
	  if (lineY[numLines][numPoints] < minY)
	    minY = lineY[numLines][numPoints];

#ifdef ZBTEMP
	  if (lineZ[numLines][numPoints] > maxY)
	    maxZ = lineZ[numLines][numPoints];
	  if (lineZ[numLines][numPoints] < minY)
	    minZ = lineZ[numLines][numPoints];
#endif

	  numPoints++;
	}
	if (numPoints > maxPointsReal)
	  maxPointsReal = numPoints;

	if (numPoints > linePoints[numLines])
	{
	  fprintf(stderr, "Line %d in map '%s' has more points (%d > %d) then specified!\n", fileLines, filename, numPoints, linePoints[numLines]);
	  linePoints[numLines] = numPoints;
	}
	else if (numPoints < linePoints[numLines])
	{
	  fprintf(stderr, "Line %d in map '%s' has fewer points (%d < %d) then specified!\n", fileLines, filename, numPoints, linePoints[numLines]);
	  linePoints[numLines] = numPoints;
	}

	numLines++;
	break;
      case 'P':
	// Location name
	locationName[numLocations] = strtok (NULL, ",");	
	// Location color
	locationColor[numLocations] = lookupColor(strtok (NULL, ","));
	locationX[numLocations] = atoi (strtok (NULL, ",\n"));
	locationY[numLocations] = atoi (strtok (NULL, ",\n"));

	// if map label isn't inside map dimensions, change map size
	if (locationX[numLocations] > maxX)
	  maxX = locationX[numLocations];
	if (locationY[numLocations] > maxY)
	  maxY = locationY[numLocations];
	if (locationX[numLocations] < minX)
	  minX = locationX[numLocations];
	if (locationY[numLocations] < minY)
	  minY = locationY[numLocations];

	numLocations++;
	break;
      }
    }
    
    fclose (fh);
    char message[128];
    sprintf (message, "%s [%s]", zoneLong, zoneShort);
    
    reAdjust ();
  }
#ifdef ZBTEMP
  fprintf(stderr, "Max Points theoretically in any Line: %d\n", maxPoints);
  fprintf(stderr, "Max Points really in any Line: %d\n", maxPointsReal);
  fprintf(stderr, "Min Z in Zone: %d\n", minZ);
  fprintf(stderr, "Max Z in Zone: %d\n", maxZ);
#endif
}


int calcXOffset (int mapCoordinate)
{
  int screenLength = screenLengthX;
  int mapLength = zoomMapLengthX;
  int screenCenter = screenCenterX;

  float ratio = (float) mapLength / (float) screenLength;	/* 9.6 */

  float offset = (float) mapCoordinate / ratio;
  float screenCoordinate = (float) screenCenter - offset;

  return (int) screenCoordinate;

}

int calcYOffset (int mapCoordinate)
{
  int screenLength = screenLengthY;
  int mapLength = zoomMapLengthY;
  int screenCenter = screenCenterY;

  float ratio = (float) mapLength / (float) screenLength;	/* 9.6 */

  float offset = (float) mapCoordinate / ratio;
  float screenCoordinate = (float) screenCenter - offset;

  return (int) screenCoordinate;
}

void paintMap ()
{
  /* Declare the image */
  gdImagePtr im;

  /* Color indexes */
  int black;
  int white;
  int gridgray;
  int mapcolors[16];

  /* Allocate the image */
  im = gdImageCreate(600, 600);

  /* Allocate colors */
  black = gdImageColorAllocate (im, 0, 0, 0);
  white = gdImageColorAllocate (im, 255, 255, 255);
  gridgray = gdImageColorAllocate (im, 64, 64, 64);

  /* Allocate map colors */
  mapcolors[0] = gdImageColorAllocate (im, 196, 196, 196); // gray
  mapcolors[1] = gdImageColorAllocate (im, 0, 0, 128); // darkblue
  mapcolors[2] = gdImageColorAllocate (im, 0, 128, 0); // darkgreen
  mapcolors[3] = gdImageColorAllocate (im, 0, 128, 128); // darkcyan
  mapcolors[4] = gdImageColorAllocate (im, 128, 0, 0); // darkred
  mapcolors[5] = gdImageColorAllocate (im, 128, 0, 128); // darkmagenta
  mapcolors[6] = gdImageColorAllocate (im, 128, 128, 0); // darkyellow
  mapcolors[7] = gdImageColorAllocate (im, 128, 128, 128); // darkgray
  mapcolors[8] = gdImageColorAllocate (im, 255, 255, 255); // white
  mapcolors[9] = gdImageColorAllocate (im, 0, 0, 255); // blue
  mapcolors[10] = gdImageColorAllocate (im, 0, 255, 0); // green
  mapcolors[11] = gdImageColorAllocate (im, 0, 255, 255); // cyan
  mapcolors[12] = gdImageColorAllocate (im, 255, 0, 0); // red
  mapcolors[13] = gdImageColorAllocate (im, 255, 0, 255); // magenta? (purple)
  mapcolors[14] = gdImageColorAllocate (im, 255, 255, 0); // yellow
  mapcolors[15] = gdImageColorAllocate (im, 196, 196, 196); // gray
  
  /* Draw the grid */
  for (int gx = (zoomMinX / 1000) - 1; gx <= (zoomMaxX / 1000) + 1; gx++)
    {
      int sx, sy;
      gdImageLine (im, sx=calcXOffset (gx * 1000), sy=0, calcXOffset (gx *
				      1000), 600, gridgray);
      char label[32];
      sprintf (label, "%d", gx*1000);
      gdImageString (im, gdFontSmall, sx+1, sy, (unsigned char *)label, gridgray);
      gdImageString (im, gdFontSmall, sx+1, sy-10, (unsigned char *)label, gridgray);
    }
  for (int gy = (zoomMinY / 1000) - 1; gy <= (zoomMaxY / 1000) + 1; gy++)
    {
      int sx, sy;
      gdImageLine (im, sx=0, sy=calcYOffset (gy * 1000), 600, calcYOffset (gy
					* 1000), gridgray);  
      char label[32];
      sprintf (label, "%d", gy*1000);
      gdImageString (im, gdFontSmall, sx, sy, (unsigned char *)label, gridgray);
    }

  /* Draw lines */
  for (int l = 0; l < numLines; l++)
    {
      for (int n =0; n<(linePoints[l]-1); n++)
        {
          gdImageLine (im, 
		      calcXOffset (lineX[l][n]), calcYOffset (lineY[l][n]),
		      calcXOffset (lineX[l][n+1]), calcYOffset (lineY[l][n+1]),
			      mapcolors[lineColor[l]]);
        }
    }

  /* Paint the locations */
  int tmpcolor;
   for (int n = 0; n < numLocations; n++)
   {
     if (!locationColor[n]) 
      {
	tmpcolor = white;
      }
     else 
      {
	tmpcolor = mapcolors[locationColor[n]];
      }
   
     gdImageString (im, gdFontSmall, 
		    calcXOffset (locationX[n]) - 2, 
		    calcYOffset (locationY[n]) - 2,
		    (unsigned char*)(const char *)locationName[n], tmpcolor);
   }

  /* Print the http header */
#ifdef HAVE_GD_IMAGEPNG
  printf ("Content-type: image/png\n\n");
#else
  printf ("Content-type: image/gif\n\n");
#endif

  /* Output image */
#ifdef HAVE_GD_IMAGEPNG
  gdImagePng(im, stdout);
#else
  gdImageGif(im, stdout);
#endif

  /* Destroy image */
  gdImageDestroy(im);

}

int main (int argc, char *argv[])
{
  // CGI Convenience class
  CGI cgiconv;

  // state variables
  // assume the user didn't pass any map name
  bool drawMap = false;

  // name of the map to display if any
  QString mapName;

  // process any CGI data
  cgiconv.processCGIData();

  /* Check if we were passed a map to read */	
  if (argc == 2)
  {
    drawMap = true;
    mapName = argv[1];
  }
  else if (cgiconv.getParamCount() != 0)
  {
    mapName = cgiconv.getParamValue("map");
    drawMap = (cgiconv.getParamValue("draw") == "y");
  }

  // if in drawmap mode, then that's all that's done
  if (drawMap)
  {	
    QRegExp slashExp("/");
    mapName.replace(slashExp, "_");

    mapName.prepend(MAPDIR "/");
    loadFileMap ((const char*)mapName);
    paintMap();
  }
  else
  {
    // open the output data stream
    QTextStream out(stdout, IO_WriteOnly);
    out.setEncoding(QTextStream::Latin1);
    out.flags(QTextStream::showbase | QTextStream::dec);
    
    const char* header =
      "Content-type: text/html; charset=iso-8859-1\n\n"
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n"
      "<HTML>\n"
      "  <HEAD>\n"
      "    <TITLE>ShowEQ Map Display</TITLE>\n"
      "    <style type=\"text/css\">\n"
      "      <!--\n"
      "          table { border: black 2px solid }\n"
      "          td { border: black 1px solid }\n"
      "          th { border: black 1px solid }\n"
      "      -->\n"
      "    </style>\n" 
      "  </HEAD>\n"
      "  <BODY>\n";

    /* Print HTML header */
    out << header;

    const char* footer = 
      "  </BODY>\n"
      "</HTML>\n";

    // otherwise display an HTML form to allow the user to choose a map
    QDir  mapDir(MAPDIR, "*.map", (QDir::Name | QDir::IgnoreCase), 
		 (QDir::Files | QDir::Readable));

    if (!mapDir.exists())
    {
      out << "<H1>The map directory '" MAPDIR "' doesn't exist!</H1>\n";
      out << footer;
      return 0;
    }

    QString userAgent = cgiconv.getHTTPUserAgent();

    out << "<FORM method=\"POST\" action=\"" << cgiconv.getScriptName() 
	<< "\">\n";

    // beware Netscape 4.x style sheet brain death
    if ((userAgent.contains("Mozilla/4.") == 1) && 
	(userAgent.contains("MSIE") == 0))
      out << "<TABLE border=2 cellspacing=0 cellpadding=2>\n";
    else
      out << "<TABLE cellspacing=0 cellpadding=2>\n";
    
    out << "<TR>";
    out << "  <TH>Map Name</TH>";
    out << "  <TD><INPUT type=\"reset\" value=\"Reset\"/></TD>\n";
    out << "</TR>";
    out << "<TR>";
    out << "<TD><SELECT name=\"map\" size=\"1\">\n";

    // create a file info list
    const QFileInfoList *list = mapDir.entryInfoList();

    // create an iterator over the list
    QFileInfoListIterator it( *list );
    
    // pointer to the file info for the current file
    QFileInfo *fi;

    while ( (fi=it.current()) ) 
    {
      out << "<OPTION value=\"" << fi->fileName() << "\"";
      if (mapName == fi->fileName())
	out << " selected";
      out << ">" << fi->baseName() << "</OPTION>\n";
      
      // goto next element from list
      ++it;
    }

    out << "</SELECT></TD>\n";
  
    // Submission button
    out << "<TD><INPUT type=\"submit\" value=\"Display\"/></TD>\n";

    out << 
      "</TR>"
      "</TABLE>"
      "</FORM>";


    if (!mapName.isEmpty())
    {
      out << "<H1>Map: " << mapName << "</H1>\n";
      out << "<DIV><IMG src=\"" << cgiconv.getScriptName()
	  << "?map=" << mapName << ";draw=y\" alt=\"Map: "
	  << mapName << "\"/></DIV>\n";
    }

    out << footer;
  }

  return 0;
}
