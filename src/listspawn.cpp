#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <qstring.h>
#include <q3textstream.h>
#include <qregexp.h>

#include "cgiconv.h"
#include "spawnshell.h"
#include "util.h"


int main (int argc, char *argv[])
{
   FILE *sdb;
   struct dbSpawnStruct dbSpawn;
   int counter=0;

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

  Spawn fake(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  // start at class value 1, and go until maximum known class value
  for (int i = 1; i <= 32; ++i)
  {
    fake.setClassVal(i);
    out << "<OPTION value=\"" << i << "\"";
    if (searchClass == i)
      out << " selected";
    out << ">" << fake.classString() << "</OPTION>\n";
  }
  out << "</SELECT></TD>\n";
  
  // Submission button
  out << "<TD><INPUT type=\"submit\" value=\"Search\"/></TD>\n";

  // close the form
  out << 
    "</TR>\n"
    "</TABLE>\n"
    "</FORM>\n";

  /////////////////////
  // on with the search
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


   // start the result table
   // beware Netscape 4.x style sheet brain death
  if ((userAgent.contains("Mozilla/4.") == 1) && 
      (userAgent.contains("MSIE") == 0))
    out << "<TABLE border=2 cellspacing=0 cellpadding=2>\n";
  else
    out << "<TABLE cellspacing=0 cellpadding=2>\n";
  out <<
   "<TR>\n"
   "<TD align=center><STRONG>Short Name</STRONG></TD>\n"
   "<TD align=center><STRONG>Long Name</STRONG></TD>\n"
   "<TD align=center><STRONG>Zone Name</STRONG></TD>\n"
   "<TD align=center><STRONG>Level</STRONG></TD>\n"
   "<TD align=center><STRONG>HP</STRONG></TD>\n"
   "<TD align=center><STRONG>Race</STRONG></TD>\n"
   "<TD align=center><STRONG>Class</STRONG></TD>\n"
   "</TR>\n";

  QString spawnName;
  QRegExp stripExp("[0-9]");

  // iterate over the contents of the spawn database
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

    // display the spawn's data
    out << "<TR>\n";
    out << "<TD><A HREF=\"showspawn.cgi?name=" << spawnName 
	<< "\">" << spawnName << "</A></TD>\n";
    out << "<TD>" << spawn.realName() << "</TD>\n";
    out << "<TD>" << dbSpawn.zoneName << "</TD>\n";
    out << "<TD align=right>" << spawn.level() << "</TD>\n";
    out << "<TD align=right>" << spawn.HP() << "/"
	<< spawn.maxHP() << "</TD>\n";
    out << "<TD>" << spawn.raceString() << "</TD>\n";
    out << "<TD>" << spawn.classString() << "</TD>\n";
    out << "</TR>\n";
    
    // increment the count
    counter++;
  }
  
  // close the table
  out << "</TABLE>\n";

  // display the number of matches
  out << "<P>"  << counter << " matches found</P>\n";

  // close the document 
  out << footer;

  // close the spawn database
  fclose (sdb);
}
