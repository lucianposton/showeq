/*
* showitem.cpp
*
* ShowEQ Distributed under GPL
* http://seq.sourceforge.net/
*/

/* CGI program to display details about a specific item - The ID of the item
* is passed on the commandline
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <q3textstream.h>

#include "cgiconv.h"
#include "util.h"
#include "itemdb.h"

// forward declarations
void printdata (Q3TextStream& out, int len, char *data);

int main (int argc, char *argv[])
{
  // open the output data stream
  Q3TextStream out(stdout, QIODevice::WriteOnly);
  
  const char* header =
    "Content-type: text/html; charset=iso-8859-1\n\n"
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n"
    "<HTML>\n"
    "  <HEAD>\n"
    "    <TITLE>ShowEQ Item Display</TITLE>\n"
    "    <style type=\"text/css\">\n"
    "      <!--\n"
    "          table { border: black 2px solid }\n"
    "          td { border: black 1px solid }\n"
    "          th { border: black 1px solid }\n"
    "          span.head { color: black }\n"
    "          span.known1 { color: green }\n"
    "          span.known2 { color: blue }\n"
    "          span.varies { color: purple }\n"
    "          span.unknown { color: red }\n"
    "          b.warning { color: red }\n"
    "      -->\n"
    "    </style>\n" 
    "  </HEAD>\n"
    "  <BODY>\n";
  
  /* Print HTML header */
  out << header;
  
  const char* footer = 
    "  </BODY>\n"
    "</HTML>\n";
  
  // number of item to display
  QString itemNumber;
  
  // should binary data be displayed (default = false)
  bool displayBinaryData = false;
  
  // should the icon be displayed
  bool displayIcon = DISPLAY_ICONS;
  
  // CGI Convenience class
  CGI cgiconv;
  
  // process any CGI data
  cgiconv.processCGIData();
  
  // If there are form parameters use them
  if (cgiconv.getParamCount() != 0)
  {
    itemNumber = cgiconv.getParamValue("item");
    
    if (cgiconv.getParamValue("displayBinary") == "y")
      displayBinaryData = true;
    else if (cgiconv.getParamValue("displayBinary") == "n")
      displayBinaryData = false;
    
    if (cgiconv.getParamValue("showIcon") == "y")
      displayIcon = true;
    else if (cgiconv.getParamValue("hideIcon") == "y")
      displayIcon = false;
  }
  else if (argc == 2)
  {
    // use argument for item number
    itemNumber = argv[1];
    
  }
  else if (argc > 2)
  {
    out << "<H1>Error: " << argv[0] << " called with " << argc 
	<< " arguments!</H1>\n";
    out << "Format: " << argv[0] << "?<ItemID>\n";
    out << footer;
    exit(-1);
  }
  
  if (itemNumber.isNull())
    itemNumber = "";

  // check for browser type
  QString userAgent = cgiconv.getHTTPUserAgent();
  out << "    <!-- Output for UserAgent: " << userAgent << "-->\n";
  
  // beware Netscape4 style sheet brain death
  bool isNetscape4 = false;
  if ((userAgent.contains("Mozilla/4.") == 1) && 
      (userAgent.contains("MSIE") == 0))
    isNetscape4 = true;
  
  // display form to allow user to select an item or display the binary data
  out << "    <FORM method=\"POST\" action=\"" << cgiconv.getScriptName() 
      << "\">\n";
  
  if (isNetscape4)
    out << "      <TABLE border cellspacing=0 cellpadding=2>\n";
  else
    out << "      <TABLE cellspacing=0 cellpadding=2>\n";
  
  out <<
    "        <TR><TH>Item ID:</TH><TH>Binary Data</TH><TH>Icon</TH>"
    "<TD><INPUT type=\"reset\" value=\"Reset\"/></TD></TR>\n"
    "        <TR>\n";
  
  out << "          <TD><INPUT type=\"text\" name=\"item\" value=\"" 
      << itemNumber 
      << "\" size=\"5\"/></TD>\n";
  
  out << 
    "          <TD>"
    "<INPUT type=\"checkbox\" name=\"displayBinary\" value=\"y\"";
  
  if (displayBinaryData)
    out << " checked";
  out << "/>Display</TD>\n";
  
  out << 
    "          <TD>";
  
  if (displayIcon)
    out << "<INPUT type=\"checkbox\" name=\"hideIcon\" value=\"y\" unchecked>"
	<< "Hide</TD>\n";
  else
    out << "<INPUT type=\"hidden\" name=\"hideIcon\" value=\"y\">"
	<< "<INPUT type=\"checkbox\" name=\"showIcon\" value=\"y\" unchecked>"
	<< "Show</TD>\n";
  
  
  // Submission button
  out << 
    "          <TD>"
    "<INPUT type=\"submit\" value=\"Search\"/></TD>\n";
  
  out << "        </TR>\n";
  out << "      </TABLE>\n";
  out << "    </FORM>\n";
  
  
  // if no item number was passed in, then just return
  if (itemNumber.isEmpty())
  {
    // ask the user to enter an ItemID
    out << "<H1>Please Enter an ItemID!</H1>\n";
    
    // close the document
    out << footer;
    
    return 0;
  }
  
  // convert the passed in item number to a short
  uint16_t currentItemNr = itemNumber.toShort();
  
  EQItemDB* itemDB = new EQItemDB;
  QString nameString;
  QString loreString;
  bool hasEntry = false;
  EQItemDBEntry* entry = NULL;
  
  hasEntry = itemDB->GetItemData(currentItemNr, &entry);
  
  out << "<H1>Information on ItemID: " << currentItemNr << "</H1>\n";
  
  if (hasEntry)
  {
    loreString = entry->GetLoreName();
    nameString = entry->GetName();
    
    if (displayIcon)
      out << "<P><IMG src=\"" << ICON_DIR << entry->GetIconNr() 
	  << ".png\" alt=\"Icon: " << entry->GetIconNr() << "\"/></P>";
    
    if (!nameString.isEmpty())
    {
      out << "<H2>" << nameString << "</H2>\n";
      out << "<P><B>Lore:</B> " << loreString << "<BR></P>\n";
    }
    else
    {
      out << "<H2>Lore: " << loreString << "</H2>\n";
    } 
    
    out << "<P>\n";
    time_t updated = entry->GetUpdated();
    out << "<B>Last Updated:</B> " << ctime(&updated) << "<BR>\n";
    out << "<B>Icon Number:</B> " << entry->GetIconNr() << "<BR>\n";
    out << "<B>Model:</B> " << entry->GetIdFile() << "<BR>\n";
    out << "<B>ItemType:</B> " << QString::number(entry->GetItemType())
	<< "<BR>\n";
    out << "<B>Weight:</B> " << (int)entry->GetWeight() << "<BR>\n";
    out << "<B>Flags:</B> ";
    if (entry->IsBook())
      out << " BOOK";
    if (entry->IsContainer())
      out << " CONTAINER";
    if (entry->GetNoDrop() == 0)
      out << " NO-DROP";
    if (entry->GetNoRent() == 0)
      out << " NO-RENT";
    if (entry->GetMagic() == 1)
      out << " MAGIC";
    if (entry->GetLoreFlag())
      out << " LORE";
    else if (entry->GetSummonedFlag())
      out << " SUMMONED";
    else if (entry->GetArtifactFlag())
      out << " ARTIFACT";
    else if (entry->GetPendingLoreFlag())
      out << " PENDING-LORE";

    out << "<BR>\n";
    out << "<B>Size:</B> " << size_name(entry->GetSize()) << "<BR>\n";
    out << "<B>Slots:</B> " << print_slot(entry->GetSlots()) << "<BR>\n";
    out << "<B>Base Price:</B> " << reformatMoney(entry->GetCost()) 
	<< "<BR>\n";
    if (entry->GetSTR())
      out << "<B>Str:</B> " << (int)entry->GetSTR() << "<BR>\n";
    if (entry->GetSTA())
      out << "<B>Sta:</B> " << (int)entry->GetSTA() << "<BR>\n";
    if (entry->GetCHA())
      out << "<B>Cha:</B> " << (int)entry->GetCHA() << "<BR>\n";
    if (entry->GetDEX())
      out << "<B>Dex:</B> " << (int)entry->GetDEX() << "<BR>\n";
    if (entry->GetINT())
      out << "<B>Int:</B> " << (int)entry->GetINT() << "<BR>\n";
    if (entry->GetAGI())
      out << "<B>Agi:</B> " << (int)entry->GetAGI() << "<BR>\n";
    if (entry->GetWIS())
      out << "<B>Wis:</B> " << (int)entry->GetWIS() << "<BR>\n";
    if (entry->GetMR())
      out << "<B>Magic:</B> " << (int)entry->GetMR() << "<BR>\n";
    if (entry->GetFR())
      out << "<B>Fire:</B> " << (int)entry->GetFR() << "<BR>\n";
    if (entry->GetCR())
      out << "<B>Cold:</B> " << (int)entry->GetCR() << "<BR>\n";
    if (entry->GetDR())
      out << "<B>Disease:</B> " << (int)entry->GetDR() << "<BR>\n";
    if (entry->GetPR())
      out << "<B>Poison:</B> " << (int)entry->GetPR() << "<BR>\n";
    if (entry->GetHP())
      out << "<B>HP:</B> " << (int)entry->GetHP() << "<BR>\n";
    if (entry->GetMana())
      out << "<B>Mana:</B> " << (int)entry->GetMana() << "<BR>\n";
    if (entry->GetAC())
      out << "<B>AC:</B> " << (int)entry->GetAC() << "<BR>\n";
    if (entry->GetLight())
      out << "<B>Light:</B> " << (int)entry->GetLight() << "<BR>\n";
    if (entry->GetDelay())
      out << "<B>Delay:</B> " << (int)entry->GetDelay() << "<BR>\n";
    
    if (entry->GetDamage())
    {
      out << "<B>Damage:</B> " << (int)entry->GetDamage() << "<BR>\n";
      QString qsTemp = print_skill (entry->GetSkill());
      out << "<B>Skill:</B> ";
      
      if (qsTemp.find("U0x") == -1)
	out << qsTemp << "<BR>\n";
      else
	out << "Unknown (ID: " << qsTemp << ")<BR>\n";
    }
    
    if (entry->GetRange())
      out << "<B>Range:</B> " << (int)entry->GetRange() << "<BR>\n";
    
    if (entry->GetMaterial())
    {
      out << "<B>Material:</B> " << QString::number(entry->GetMaterial(), 16)
	  << " (" << print_material (entry->GetMaterial()) << ")<BR>\n";
      
      out << "<B>Color:</B> " << QString::number(entry->GetColor(), 16) << "<BR>\n";
      
      out << ((entry->GetColor())  ?
	      (QString("<TABLE><TR><TD bgcolor=\"#%1\">").arg(entry->GetColor(), 6, 16)) :
	      ("<TABLE><TR><TD bgcolor=\"#ffffff\">"))
	  << "&nbsp;&nbsp;###&nbsp;SAMPLE&nbsp;###&nbsp;&nbsp</TD></TR></TABLE>\n";
    }
    else
      out << "<B>Material:</B> 0 (None)<BR>\n";

    if (entry->GetStackable() != -1)
      out << "<B>Stackable:</B> " 
	  << ((entry->GetStackable() == 1) ? "yes" : "no?") << "<BR>\n";

    if (entry->GetEffectType() != -1)
      out << "<B>Effect Type:</B> " 
	  << entry->GetEffectTypeString() << "<BR>\n";
    if (entry->GetSpellId() != ITEM_SPELLID_NOSPELL)
    {
      out << "<B>Spell Effect:</B> " << spell_name (entry->GetSpellId()) 
	  << "<BR>\n";
      if (entry->GetLevel())
	out << "<B>Casting Level:</B> " << (int)entry->GetLevel() << "<BR>\n";
      if (entry->GetCharges())
      {
	out << "<B>Charges:</B> ";
	
	if (entry->GetCharges() == -1)
	  out << "Unlimited<BR>\n";
	else
	  out << (int)entry->GetCharges() << "<BR>\n";
      }
      if (entry->GetCastTime())
	out << "<B>Casting Time:</B> " << (int)entry->GetCastTime() 
	    << "<BR>\n";
    }
    out << "<B>Class:</B> " << print_classes (entry->GetClasses()) << "<BR>\n";
    out << "<B>Race:</B> " << print_races (entry->GetRaces()) << "<BR>\n";
    if (entry->GetSkillModId())
      out << "<B>Skill Modifier:</B> " << skill_name(entry->GetSkillModId())
	  << " " << (int)entry->GetSkillModPercent() << "% <BR>\n";
    if (entry->IsContainer())
    {
      if (entry->GetNumSlots())
        out << "<B>Container Slots:</B> " << (int)entry->GetNumSlots() 
	    << "<BR>\n";
      if (entry->GetSizeCapacity())
        out << "<B>Size Capacity:</B> " << size_name(entry->GetSizeCapacity())
	    << "<BR>\n";
      if (entry->GetWeightReduction())
        out << "<B>% Weight Reduction:</B> " << (int)entry->GetWeightReduction()
	    << "<BR>\n";
    }
    out << "</P>\n";
  }
  
  int size = 0;
  
  if (displayBinaryData)
  {
    time_t updated;
    char* rawData = NULL;
    size = itemDB->GetItemRawData(currentItemNr, updated, &rawData);

    if ((size > 0) && (rawData != NULL))
    {
      out << "<P><B>Raw data: (" << size << " octets) last updated: " 
	  << ctime(&updated) << "</B></P>\n";
      out << "</P>";
      out << "<PRE>\n";
      printdata (out, size, rawData);
      out << "</PRE>\n";
      delete [] rawData;
    }
  }

  if (!hasEntry && nameString.isEmpty() && loreString.isEmpty() &&
      (size == 0))
    out << "<P>Item " << currentItemNr << " not found</P>\n";

  // close the document with the footer
  out << footer;

  // delete DB entry 
  delete entry;

  // shutdown the ItemDB instance
  itemDB->Shutdown();

  // delete the ItemDB instance
  delete itemDB;

  return 0;
}

void printdata (Q3TextStream& out, int len, char *data)
{
  char asc[1024];
  uint8_t fieldCount = 1;
  uint8_t beginField = fieldCount;
  uint8_t col = 0;
  size_t fieldWidth;
  const char* curPos = data;
  const char* endField;
  QString tempStr;

  asc[0] = '\0';
  while (*curPos != '\0')
  {
    endField = curPos;
    while ((*endField != '|') && (*endField != '\0'))
      endField++;
    fieldCount++;
    
    fieldWidth = endField - curPos + 1;
    if (col + fieldWidth >= 75)
    {
      tempStr.sprintf("<SPAN class=\"head\">%.3d | %s</SPAN>\n",
		      beginField, asc);

      out << tempStr;
      asc[0] = '\0';
      col = fieldWidth;
      beginField = fieldCount;
    }
    else
    {
      strncat(asc, curPos, fieldWidth);
      col += fieldWidth;
    }
      
    curPos = endField;
    if (*curPos != '\0')
      curPos++;
  }

  tempStr.sprintf("<SPAN class=\"head\">%.3d | %s</SPAN>\n",
		  beginField, asc);

  out << tempStr;
}
