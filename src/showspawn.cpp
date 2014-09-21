/*
 * showspawn.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <qstring.h>
#include <qregexp.h>
#include <q3textstream.h>

#include "cgiconv.h"
#include "spawn.h"
#include "util.h"

// forward declarations
void printdata (Q3TextStream& out, int len, unsigned char *data);

int main (int argc, char *argv[])
{
  FILE *sdb;
  struct dbSpawnStruct dbSpawn;
  int count=0;

  // CGI Convenience class
  CGI cgiconv;
  
  // search variables
  QString searchName = "";
  QString searchZone = "";
  int searchLevel = 0;
  QString searchRace = "";
  int searchClass = 0;

  // are we performing a serch (default = false)
  bool doSearch = false;

  // process any CGI data
  cgiconv.processCGIData();

  // If there are form parameters use them for searching
  if (cgiconv.getParamCount() != 0)
  {
    searchName = cgiconv.getParamValue("name");
    searchZone = cgiconv.getParamValue("zone");
    searchLevel = cgiconv.getParamValue("level").toInt();
    searchRace = cgiconv.getParamValue("race");
    searchClass = cgiconv.getParamValue("class").toInt();

    if (!searchName.isEmpty() || !searchZone.isEmpty() ||
	!searchRace.isEmpty() ||
	(searchLevel != 0) || (searchClass != 0))
      doSearch = true;
  }
  else if (argc == 2)
  {
    // use the argument for the name search
    searchName = argv[1];
    
    // note that a search is being done.
    doSearch = true;
  } 

  // open the output data stream
  Q3TextStream out(stdout, QIODevice::WriteOnly);
  out.setEncoding(Q3TextStream::Latin1);
  out.flags(Q3TextStream::showbase | Q3TextStream::dec);

  const char* header =
    "Content-type: text/html; charset=iso-8859-1\n\n"
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n"
    "<HTML>\n"
    "  <HEAD>\n"
    "    <TITLE>ShowEQ Spawn List</TITLE>\n"
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
  
  // check for browser type
  QString userAgent = cgiconv.getHTTPUserAgent();
  out << "    <!-- Output for UserAgent: " << userAgent << "-->\n";


  out << "<FORM method=\"POST\" action=\"" << cgiconv.getScriptName() 
      << "\">\n";

  // beware Netscape 4.x style sheet brain death
  if ((userAgent.contains("Mozilla/4.") == 1) && 
      (userAgent.contains("MSIE") == 0))
    out << "<TABLE border=2 cellspacing=0 cellpadding=2>\n";
  else
    out << "<TABLE cellspacing=0 cellpadding=2>\n";

  out << 
    "<TR>"
    "<TH>Name</TH><TH>Zone</TH><TH>Level</TH><TH>Race</TH><TH>Class</TH>\n"
    "<TD><INPUT type=\"reset\" value=\"Reset\"/></TD>\n"
    "</TR>\n";

  out << "<TR>";

  // name field
  out << "<TD><INPUT type=\"text\" name=\"name\" value=\"" 
      << searchName << "\"/></TD>\n";

  // zone field
  out << "<TD><INPUT type=\"text\" name=\"zone\" value=\"" 
      << searchZone << "\"/></TD>\n";

  // level field
  out << 
    "<TD><INPUT type=\"text\" name=\"level\" size=\"2\" maxlength=\"2\""
    " value=\"";
  if (searchLevel)
    out << searchLevel;
  out << "\"/></TD>\n";

  // race field
  out << "<TD><INPUT type=\"text\" name=\"race\" value=\"" 
      << searchRace << "\"/></TD>\n";

  // Class field
  out << "<TD><SELECT name=\"class\" size=\"1\">\n";
  out << "<OPTION value=\"0\"";
  if (searchClass == 0)
    out << " selected";
  out << ">Any</OPTION>\n";

  // create a fake spawn to get class names
  Spawn fakeSpawn;

  // start at class value 1, and go until maximum known class value
  for (int i = 1; i <= 32; ++i)
  {
    // set the fake spawns class
    fakeSpawn.setClassVal(i);

    out << "<OPTION value=\"" << i << "\"";
    if (searchClass == i)
      out << " selected";

    // output the name corresponding to the class value
    out << ">" << fakeSpawn.classString() << "</OPTION>\n";
  }
  out << "</SELECT></TD>\n";
  
  // Submission button
  out << "<TD><INPUT type=\"submit\" value=\"Search\"/></TD>\n";

  // close the form
  out << 
    "</TR>\n"
    "</TABLE>\n"
    "</FORM>\n";

   sdb = fopen (SPAWNFILE, "r");
   
   if (sdb == NULL) 
   {
     // display the error
     out << "<H1>Unable to open file '" SPAWNFILE "' (errno = " 
	 << errno << ")</H1>\n";

     // close the document
     out << footer;

     // nothing more can be done
     exit(1);
   }

  QString spawnName;
  QRegExp stripExp("[0-9]");

  while (fread (&dbSpawn, sizeof(dbSpawnStruct), 1, sdb))
  {
    Spawn spawn(&dbSpawn.spawn);

    // stash the name for later use and cooking
    spawnName = spawn.name();
    
    // is a search being performed, then do it already...
    if (doSearch)
    {
      // is it a name search, if so do we have a match
      if ((!searchName.isEmpty()) &&
	  (spawnName.find(searchName, 0, false) == -1))
	continue;  // nope, you are the weakest link, good bye...

      // is it a zone search, if so check
      if ((!searchZone.isEmpty()) &&
	  (QString(dbSpawn.zoneName).find(searchZone, 0, false) == -1))
	continue;
      
      // is it a level search, if so check
      if ((searchLevel != 0) && 
	  (searchLevel != spawn.level()))
	continue;
      
      // is it a race search, if so check
      if ((!searchRace.isEmpty()) &&
	  (spawn.raceString().find(searchRace, 0, false) == -1))
	continue;
      
      // is it a class search, if so check
      if ((searchClass != 0) &&
	  (searchClass != spawn.classVal()))
	continue;
    }
	
    // strip the numbers off the spawn name
    spawnName.replace(stripExp, "");

    // display the data
    out << "<H1>" << spawn.realName() << "</H1>\n";
    out << "<P>ShortName: " << spawn.transformedName() << "<BR>\n";
    out << "Level: " << spawn.level() << "<BR>\n";
    out << "HP: " << spawn.HP() << "/"
	<< spawn.maxHP() << "<BR>\n";
    out << "Race: " << spawn.raceString() << "<BR>\n";
    out << "Class: " << spawn.classString() << "<BR>\n";
    out << "Found in Zone: " << dbSpawn.zoneName << "<BR>\n";
    out << "Position: " <<  spawn.y() << ", " 
	<< spawn.x() << ", "
	<< spawn.z() << "<BR>\n";
    out << "Mob ID: " << spawn.id() << "<BR>\n";
    out << "<B>Packet data:</B>\n";
    out <<"<PRE>\n";
    printdata (out, sizeof(spawnStruct), (unsigned char *)&dbSpawn.spawn);
    out <<"</PRE>\n";
    out <<"<HR>\n";

    // increment counter
    count++;
  }

  // clsoe the DB
   fclose (sdb);

   // display the number of records found
   out << "<P>Found " << count << " matches.</P>\n";

   out << footer;
}

void printdata (Q3TextStream& out, int len, unsigned char *data)
{
  char hex[128];
  char asc[128];
  char tmp[32];
  
  QString outstr;
  QRegExp escapeExp1("<");
  QRegExp escapeExp2("&");

  hex[0]=0; asc[0]=0;
  int c;
  for (c=0; c<len; c++)
  {
    if ((!(c%16))&&c)
    {
      outstr.sprintf ("%03d | %s | %s \n", c-16, hex, asc);
      out << outstr;

      hex[0]=0; asc[0]=0;
    }

    sprintf (tmp, "%02x ", data[c]);
    strcat (hex, tmp);
    
    // is the character printable
    if ((data[c]>=32) && (data[c]<=126))
    {
      // it's printable, escape it if necessary.
      if (data[c] == '<')
	strcpy(tmp, "&lt;");
      else if (data[c] == '>')
	strcpy(tmp, "&gt;");
      else if (data[c] == '&')
	strcpy(tmp, "&amp;");
      else
      {
	// no escaping needed
	tmp[0] = data[c];
	tmp[1] = '\0';
      }
    }
    else // if it's not printable, just insert a '.'
      strcpy (tmp, ".");

    strcat (asc, tmp);

  }
  if (c%16)
    c=c-(c%16);
  else
    c-=16;

  outstr.sprintf ("%03d | %-48s | %s \n\n", c, hex, asc);
  out << outstr;
}

