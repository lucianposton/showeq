/*
 *  listitem.cpp
 *  
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

/* CGI program to list the items in the item database
 * a name can be passed on the command line
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <qstring.h>
#include <q3textstream.h>

#include "cgiconv.h"
#include "util.h"
#include "itemdb.h"
#include "qmap.h"

// forward declaration
void displayForm(Q3TextStream& out, 
		 CGI& cgiconv, 
		 bool displayIcon);

int main (int argc, char *argv[])
{
  int counter=0;

  QString buff;

  // CGI Convenience class
  CGI cgiconv;

  // search variables
  QString searchName;
  QString searchType;
  QString searchSlot;
  QString searchSkill;
  QString searchRace;
  QString searchClass;
  QString searchEffect;

  // is a name search occurring (default = false)
  bool doNameSearch = false;

  // is any other search occurring (default = false)
  bool doSearch = false;

  // should the item icons be displayed (default = false)
  bool displayIcon = DISPLAY_ICONS;

  // process any CGI data
  cgiconv.processCGIData();

  // If there are form parameters use them for searching
  if (cgiconv.getParamCount() != 0)
  {
    searchName = cgiconv.getParamValue("name");
    searchType = cgiconv.getParamValue("type");
    searchSlot = cgiconv.getParamValue("slot");
    searchSkill = cgiconv.getParamValue("skill");
    searchRace = cgiconv.getParamValue("race");
    searchClass = cgiconv.getParamValue("class");
    searchEffect = cgiconv.getParamValue("effect");

    // if the name CGI form field isn't empty, perform a name search
    if (!searchName.isEmpty())
      doNameSearch = true;

    // if any of the other CGI form fields is not empty, perform a search
    if (!searchType.isEmpty() || !searchSlot.isEmpty() || 
	!searchSkill.isEmpty() || !searchRace.isEmpty() || 
	!searchClass.isEmpty() || !searchEffect.isEmpty())
      doSearch = true;
    
    if (cgiconv.getParamValue("showIcon") == "y")
      displayIcon = true;
    else if (cgiconv.getParamValue("hideIcon") == "y")
      displayIcon = false;

#if 0
    cgiconv.logCGIData("/tmp/listitem.cgi.txt");
#endif
  }
  else if (argc == 2)
  {
    // otherwise if just a second argument passed in, use it
    searchName = argv[1];

    // note that a search is to be performed
    doNameSearch = true;
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
    "    <TITLE>ShowEQ Item List</TITLE>\n"
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

  // display the CGI form
  displayForm(out, cgiconv, displayIcon);

  // if there is a search, display what it is
  if (doSearch || doNameSearch)
  {
    out << "<P><STRONG>Searching for items";
    if (!searchName.isEmpty())
      out << " with name containing '" << searchName << "'";

    if (!searchType.isEmpty())
    { 
      if (searchType == "B")
	out << " that are a book";
      else if (searchType == "C")
	out << " that are a container";
    } 
    
    if (!searchSlot.isEmpty())
      out << " that go in '" << searchSlot << "' slot";

    if (!searchSkill.isEmpty())
      out << " for skill '" <<  searchSkill << "'";

    if (!searchRace.isEmpty())
      out << " for race '" << searchRace << "'";

    if (!searchClass.isEmpty())
      out << " for class '" << searchClass << "'";

    if (!searchEffect.isEmpty())
      out << " with effect containing '" << searchEffect << "'";

    out << ".</STRONG></P>\n";
  }

  // beware Netscape 4.x style sheet brain death
  if ((userAgent.contains("Mozilla/4.") == 1) && 
      (userAgent.contains("MSIE") == 0))
    out << "<TABLE border=2 cellspacing=0 cellpadding=2>\n";
  else
    out << "<TABLE cellspacing=0 cellpadding=2>\n";

  out << "<TR>";
  out << "<TD align=center><STRONG>Icon</STRONG></TD>";
  out << "<TD align=center><STRONG>Item #</STRONG></TD>";
  out << "<TD align=center><STRONG>Name</STRONG></TD>";
  out << "<TD align=center><STRONG>Lore</STRONG></TD>";
  out << "<TD align=center><STRONG>Weight</STRONG></TD>";
  out << "<TD align=center><STRONG>Size</STRONG></TD>";
  out << "<TD align=center><STRONG>Base Price</STRONG></TD>";
  out << "<TD align=center><STRONG>Material</STRONG></TD>";
  out << "<TD align=center><STRONG>Flags</STRONG></TD>";
  out << "<TD align=center><STRONG>Slots</STRONG></TD>";
  out << "<TD align=center><STRONG>AC</STRONG></TD>";
  out << "<TD align=center><STRONG>Damage</STRONG></TD>";
  out << "<TD align=center><STRONG>Delay</STRONG></TD>";
  out << "<TD align=center><STRONG>Range</STRONG></TD>";
  out << "<TD align=center><STRONG>Skill</STRONG></TD>";
  out << "<TD align=center><STRONG>Bonuses</STRONG></TD>";
  out << "<TD align=center><STRONG>Resistances</STRONG></TD>";
  out << "<TD align=center><STRONG>Effect</STRONG></TD>";
  out << "</TR>\n";

  // retrieve a pointer to the item database   
  EQItemDB* pItemDB = new EQItemDB;

  // if unsuccessful then can't do any more
  if (pItemDB == NULL)
  {
    out << "</TABLE>\n";
    out << "<H3>Error: No Item DataBase</H3>";
    out << footer;
    return -1;
  }

  // retrieve an iterator over the DATA_DB
  EQItemDBIterator* pIter = new EQItemDBIterator(pItemDB, EQItemDB::DATA_DB);

  // if unsuccessful then can't do any more
  if (pIter == NULL)
  {
    out << "</TABLE>\n";
    out << "<H3>Error: No DataBase</H3>";
    out << footer;

    delete pItemDB;

    return -1;
  }

  bool hasNext = true;  
  uint32_t currentItemNr;
  uint32_t nextItemNr;
  QString nameString;
  QString loreString;
  bool hasEntry = false;
  EQItemDBEntry* entry = NULL;

  // retrieve the first number
  hasNext = pIter->GetFirstItemNumber(&nextItemNr);
  
  // iterate until there isn't a next item number
  while (hasNext)
  {
    // delete the entry since we no longer need it.
    delete entry;

    // fill the entry with a NULL just in case
    entry = NULL;
    
    // the next item number is now the current item number
    currentItemNr = nextItemNr;

    // retrieve entry from iterator, since it is on the data database
    hasEntry = pIter->GetItemData(&entry);

    // attempt to retrive a new next item number
    hasNext = pIter->GetNextItemNumber(&nextItemNr);

    // attempt to retrieve the item name and lore
    loreString = entry->GetLoreName();
    nameString = entry->GetName();
     
    // if there is a search to be performed, do it.
    if (doNameSearch)
    {
      if (!searchName.isEmpty()) 
      { 
	if ((loreString.find(searchName, 0, false) == -1) &&
	    (nameString.find(searchName, 0, false) == -1))
	  continue;
      }
    }

    if (doSearch)
    {
      if (!searchType.isEmpty())
      {
	if ((searchType == "B") && !entry->IsBook())
	  continue;
	if ((searchType == "C") && !entry->IsContainer())
	  continue;
      }

      if (!searchSlot.isEmpty())
      {	
	// There are multiple slots, some with numbers, so search for
	// the correct slot in the list
	if ((print_slot(entry->GetSlots()).find(searchSlot)) == -1)
	  continue;
      }

      if (!searchSkill.isEmpty())
      {	
	// there is only one skill, check for exact match
	if (print_skill(entry->GetSkill()) != searchSkill)
	  continue;
      }

      QString tmpval;

      if (!searchRace.isEmpty())
      {	
	// get the list of races
	tmpval = print_races(entry->GetRaces());

	// check to see if the item is for ALL/No races, or
	// if it contains the requested race
	if ((tmpval != "ALL") &&
	    (tmpval != "NONE") &&
	    (tmpval.find(searchRace) == -1))
	  continue;
      }

      if (!searchClass.isEmpty())
      {	
	// get the list of races
	tmpval = print_classes(entry->GetClasses());

	// check to see if the item is for ALL/No classes, or
	// if it contains the requested class
	if ((tmpval != "ALL") &&
	    (tmpval != "NONE") && 
	    (tmpval.find(searchClass) == -1))
	  continue;
      }

      if (!searchEffect.isEmpty())
      {
	// retrieve the spell Id's
	int spellId = entry->GetSpellId();

	// perform a quick check to make sure there is a spell on them
	if (spellId == ITEM_SPELLID_NOSPELL)
	  continue;

	QString tmpval;
	if (spellId != ITEM_SPELLID_NOSPELL)
	  tmpval = spell_name(spellId);

	if (tmpval.find(searchEffect, 0, false) == -1)
	  continue;
      }
    }

    // display the item information
    out << "<TR>";
    
    // only display icon number if we have that data
    if (displayIcon)
      out << "<TD><IMG SRC=\"" << ICON_DIR << entry->GetIconNr() 
	  << ".png\" alt=\"Icon: " << entry->GetIconNr() << "\"/></TD>";
    else
      out << "<TD>" << entry->GetIconNr() << "</TD>";

    out << "<TD><A HREF=\"showitem.cgi?item=" << currentItemNr
	<< ";displayBinary=y" 
	<< (displayIcon ? ";showIcon=y" : ";hideIcon=y")
	<< "\">" << currentItemNr << "</A></TD>";

    // only display a name if we have the item name
    if (!nameString.isEmpty())
      out << "<TD><A HREF=\"showitem.cgi?item=" << currentItemNr 
	  << (displayIcon ? ";showIcon=y" : ";hideIcon=y")
	  << "\">" << nameString << "</A></TD>";
    else
      out << "<TD>&nbsp</TD>";

    // display the lore string 
    out << "<TD><A HREF=\"showitem.cgi?item=" << currentItemNr 
	<< (displayIcon ? ";showIcon=y" : ";hideIcon=y")
	<< "\">" << loreString << "</A></TD>";

    // if we have more data for this item, print it
    out << "<TD align=right>" << entry->GetWeight() << "</TD>"; 
    out << "<TD>" << size_name(entry->GetSize()) << "</TD>";
    out << "<TD align=right>" << (int)entry->GetCost() << "</TD>"; 
    if (entry->GetMaterial())
      out << QString("<TD align=center><FONT COLOR=\"#%1\">%2</FONT>")
	.arg(entry->GetColor(), 6, 16)
	.arg(print_material(entry->GetMaterial()));
    else
      out << "<TD>&nbsp</TD>";
    
    buff = "";

    // Flags
    out << "<TD>";

    if (entry->IsBook())
      buff += "BOOK ";
    if (entry->IsContainer())
      buff += "CONTAINER ";
    if (entry->GetNoDrop()==0)
      buff += "NO-DROP ";
    if (entry->GetNoRent()==0)
      buff += "NO-Rent ";
    if (entry->GetMagic()==1)
      buff += "MAGIC ";
    if (entry->GetLoreFlag())
      buff += "LORE";
    else if (entry->GetSummonedFlag())
      buff += "SUMMONED";
    else if (entry->GetArtifactFlag())
      buff += "ARTIFACT";
    else if (entry->GetPendingLoreFlag())
      buff += "PENDING-LORE";

    if (buff.isEmpty())
      buff = "&nbsp";
    out << buff << "</TD>";
    
    if (entry->GetSlots())
      out << "<TD align=center>" << print_slot(entry->GetSlots()) 
	  << "</TD>";
    else
      out << "<TD>&nbsp</TD>";
    
    if (entry->GetAC())
      out << "<TD>" << (int)entry->GetAC() << "</TD>";
    else
      out << "<TD>&nbsp</TD>";
    if (entry->GetDamage())
      out << "<TD>" << (int)entry->GetDamage() << "</TD>";
    else
      out << "<TD>&nbsp</TD>";
    if (entry->GetDelay())
      out << "<TD>" << (int)entry->GetDelay() << "</TD>";
    else
      out << "<TD>&nbsp</TD>";
    if (entry->GetRange())
      out << "<TD>" << (int)entry->GetRange() << "</TD>";
    else
      out << "<TD>&nbsp</TD>";
    if (entry->GetDamage())
      out << "<TD>" << print_skill(entry->GetSkill()) << "</TD>";
    else
      out << "<TD>&nbsp</TD>";
    
    // Bonuses
    out << "<TD>";
    
    if (entry->GetSTR())
      out << "Str:" << (int)entry->GetSTR() << " ";
    if (entry->GetAGI())
      out << "Agi:" << (int)entry->GetAGI() << " ";
    if (entry->GetSTA())
      out << "Sta:" << (int)entry->GetSTA() << " ";
    if (entry->GetDEX())
      out << "Dex:" << (int)entry->GetDEX() << " ";
    if (entry->GetINT())
      out << "Int:" << (int)entry->GetINT() << " ";
    if (entry->GetWIS())
      out << "Wis:" << (int)entry->GetWIS() << " ";
    if (entry->GetCHA())
      out << "Cha:" << (int)entry->GetCHA() << " ";
    if (entry->GetMana())
      out << "Mana:" << (int)entry->GetMana() << " ";
    if (entry->GetHP())
      out << "HP:" << (int)entry->GetHP() << " ";
    if (entry->GetLight())
      out << "Light:" << (int)entry->GetLight() << " ";
    out << "&nbsp</TD>";
    
    // Resistances
    out << "<TD>";
    if (entry->GetCR())
      out << "Cold:" << (int)entry->GetCR() << " ";
    if (entry->GetFR())
      out << "Fire:" << (int)entry->GetFR() << " ";
    if (entry->GetMR())
      out << "Magic:" << (int)entry->GetMR() << " ";
    if (entry->GetDR())
      out << "Dis:" << (int)entry->GetDR() << " ";
    if (entry->GetPR())
      out << "Poison:" << (int)entry->GetPR() << " ";
    out << "&nbsp</TD>";
    
    if (entry->GetSpellId() != ITEM_SPELLID_NOSPELL)
      out << "<TD>" << spell_name(entry->GetSpellId()) << "</TD>";
    else
      out << "<TD>&nbsp</TD>";

    // close off the row
    out << "</TR>\n";

    // increment the counter
    counter++;
  }
  
  // delete the entry left from the last time through the loop
  delete entry;

  // delete the iterator
  delete pIter;

  // shutdown the ItemDB instance
  pItemDB->Shutdown();

  // delete the ItemDB instance
  delete pItemDB;

  out << "</TABLE>\n";
  out << "<P>" << counter << " matches found</P>\n";

  out << "</BODY>";
  out << "</HTML>";
}

void displaySelectOption(Q3TextStream& out, 
			 const QString& value, 
			 const QString& name,
			 const QString currentChk)
{
  out << "<OPTION value=\"" << value << "\"";
  if (value == currentChk)
    out << " selected";
  out << ">" << name << "</OPTION>\n";
}

void displayForm(Q3TextStream& out, 
		 CGI& cgiconv, 
		 bool displayIcon)
{
  QString currentChk;
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
  out << "<TH>Name</TH><TH>Type</TH><TH>Slot</TH><TH>Skill</TH><TH>Race</TH>"
         "<TH>Class</TH><TH>Effect</TH><TH>Icon</TH>\n";
  out << "<TD><INPUT type=\"reset\" value=\"Reset\"/></TD>\n";
  out << "</TR>";
  out << "<TR>";

  // name field
  out << "<TD><INPUT type=\"text\" name=\"name\" value=\"" 
      << cgiconv.getParamValue("name") << "\"/></TD>\n";
  
  // Type field
  currentChk = cgiconv.getParamValue("type");
  out << "<TD><SELECT name=\"type\" size=\"1\">\n";
  displaySelectOption(out, "", "Any", currentChk);
  displaySelectOption(out, "C", "Container", currentChk);
  displaySelectOption(out, "B", "Book", currentChk);
  out << "</SELECT></TD>\n";

  // Slot field
  currentChk = cgiconv.getParamValue("slot");
  out << "<TD><SELECT name=\"slot\" size=\"1\">\n";
  displaySelectOption(out, "", "Any", currentChk);
  displaySelectOption(out, "EAR", "Ear", currentChk);
  displaySelectOption(out, "HEAD", "Head", currentChk);
  displaySelectOption(out, "FACE", "Face", currentChk);
  displaySelectOption(out, "NECK", "Neck", currentChk);
  displaySelectOption(out, "SHOULDERS", "Shoulders", currentChk);
  displaySelectOption(out, "ARMS", "Arms", currentChk);
  displaySelectOption(out, "BACK", "Back", currentChk);
  displaySelectOption(out, "WRIST", "Wrist", currentChk);
  displaySelectOption(out, "RANGE", "Range", currentChk);
  displaySelectOption(out, "HANDS", "Hands", currentChk);
  displaySelectOption(out, "MELEE", "Melee", currentChk);
  displaySelectOption(out, "FINGER", "Finger", currentChk);
  displaySelectOption(out, "CHEST", "Chest", currentChk);
  displaySelectOption(out, "LEGS", "Legs", currentChk);
  displaySelectOption(out, "FEET", "Feet", currentChk);
  displaySelectOption(out, "WAIST", "Waist", currentChk);
  displaySelectOption(out, "AMMO", "Ammo", currentChk);
  out << "</SELECT></TD>\n";

  // Skill field
  currentChk = cgiconv.getParamValue("skill");
  out << "<TD><SELECT name=\"skill\" size=\"1\">\n";
  displaySelectOption(out, "", "Any", currentChk);
  displaySelectOption(out, "1H Slash", "1H Slash", currentChk);
  displaySelectOption(out, "2H Slash", "2H Slash", currentChk);
  displaySelectOption(out, "1H Blunt", "1H Blunt", currentChk);
  displaySelectOption(out, "2H Blunt", "2H Blunt", currentChk);
  displaySelectOption(out, "Piercing", "Piercing", currentChk);
  displaySelectOption(out, "Archery", "Archery", currentChk);
  displaySelectOption(out, "Throwing", "Throwing", currentChk);
  out << "</SELECT></TD>\n";

  // Race field
  currentChk = cgiconv.getParamValue("race");
  out << "<TD><SELECT name=\"race\" size=\"1\">\n";
  displaySelectOption(out, "", "Any", currentChk);
  displaySelectOption(out, "ALL", "All", currentChk);
  displaySelectOption(out, "HUM", "Human", currentChk);
  displaySelectOption(out, "BAR", "Barbarian", currentChk);
  displaySelectOption(out, "ERU", "Erudite", currentChk);
  displaySelectOption(out, "ELF", "Wood Elf", currentChk);
  displaySelectOption(out, "HIE", "High Elf", currentChk);
  displaySelectOption(out, "DEF", "Dark Elf", currentChk);
  displaySelectOption(out, "HEF", "Half Elf", currentChk);
  displaySelectOption(out, "DWF", "Dwarf", currentChk);
  displaySelectOption(out, "TRL", "Troll", currentChk);
  displaySelectOption(out, "OGR", "Ogre", currentChk);
  displaySelectOption(out, "HFL", "Halfling", currentChk);
  displaySelectOption(out, "GNM", "Gnome", currentChk);
  displaySelectOption(out, "IKS", "Iksar", currentChk);
  displaySelectOption(out, "VAH", "Vah Shir", currentChk);
  out << "</SELECT></TD>\n";

  // Class field
  currentChk = cgiconv.getParamValue("class");
  out << "<TD><SELECT name=\"class\" size=\"1\">\n";
  displaySelectOption(out, "", "Any", currentChk);
  displaySelectOption(out, "ALL", "All", currentChk);
  displaySelectOption(out, "WAR", "Warrior", currentChk);
  displaySelectOption(out, "CLR", "Cleric", currentChk);
  displaySelectOption(out, "PAL", "Paladin", currentChk);
  displaySelectOption(out, "RNG", "Ranger", currentChk);
  displaySelectOption(out, "SHD", "Shadow Knight", currentChk);
  displaySelectOption(out, "DRU", "Druid", currentChk);
  displaySelectOption(out, "MNK", "Monk", currentChk);
  displaySelectOption(out, "BRD", "Bard", currentChk);
  displaySelectOption(out, "ROG", "Rogue", currentChk);
  displaySelectOption(out, "SHM", "Shaman", currentChk);
  displaySelectOption(out, "NEC", "Necromancer", currentChk);
  displaySelectOption(out, "WIZ", "Wizard", currentChk);
  displaySelectOption(out, "MAG", "Magician", currentChk);
  displaySelectOption(out, "ENC", "Enchanter", currentChk);
  displaySelectOption(out, "BST", "Beastlord", currentChk);
  out << "</SELECT></TD>\n";

  // Effect field
  out << "<TD><INPUT type=\"text\" name=\"effect\" value=\"" 
      << cgiconv.getParamValue("effect") << "\"/></TD>\n";

  // Should the icon be displayed
  out << "<TD>";
  if (displayIcon) 
    out << "<INPUT type=\"checkbox\" name=\"hideIcon\" value=\"y\" unchecked>\n"
        << "Hide</TD>\n";

  else
    out << "<INPUT type=\"hidden\" name=\"hideIcon\" value=\"y\">\n"
        << "<INPUT type=\"checkbox\" name=\"showIcon\" value=\"y\" unchecked>\n"
        << "Show</TD>\n";
  
  // Submission button
  out << "<TD><INPUT type=\"submit\" value=\"Search\"/></TD>\n";

  out << "</TR>";
  out << "</TABLE>\n";
  out << "</FORM>\n";
}

