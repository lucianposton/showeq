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

#include <qtextstream.h>

#include "cgiconv.h"
#include "util.h"
#include "itemdb.h"

// forward declarations
void printdata (QTextStream& out, int len, unsigned char *data);

int main (int argc, char *argv[])
{
  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

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
    
    if (cgiconv.getParamValue("displayIcon") == "y")
      displayIcon = true;
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

  // beware Netscap4 style sheet brain death
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
    "          <TD>"
    "<INPUT type=\"checkbox\" name=\"displayIcon\" value=\"y\"";

  if (displayIcon)
    out << " checked";
  out << "/>Display</TD>\n",

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
    out << "<B>Icon Number:</B> " << entry->GetIconNr() << "<BR>\n";
    out << "<B>Model:</B> " << entry->GetIdFile() << "<BR>\n";
    out << "<B>flag:</B> " << QString::number(entry->GetFlag(), 16)
	<< "<BR>\n";
    out << "<B>Weight:</B> " << (int)entry->GetWeight() << "<BR>\n";
    out << "<B>Flags:</B> ";
    if (entry->IsBook())
      out << " BOOK";
    if (entry->IsContainer())
      out << " CONTAINER";
    if (entry->GetNoDrop() == 0)
      out << " NO-DROP";
    if (entry->GetNoSave() == 0)
      out << " NO-SAVE";
    if (entry->GetMagic() == 1)
      out << " MAGIC";
    if (loreString[0] == '*')
      out << " LORE";
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
    out << "<B>Material:</B> " << QString::number(entry->GetMaterial(), 16)
	<< " (" << print_material (entry->GetMaterial()) << ")<BR>\n";
    out << "<B>Color:</B> " << QString::number(entry->GetColor(), 16) 
	<< QString(" <B style=\"color: #%1\">###SAMPLE###</B><BR>\n")
      .arg(entry->GetColor(), 6, 16);
    if (entry->GetSpellId0() != ITEM_SPELLID_NOSPELL)
      out << "<B>Effect1:</B> " << spell_name (entry->GetSpellId0()) 
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
    if (entry->GetSpellId() != ITEM_SPELLID_NOSPELL)
      out << "<B>Effect2:</B> " << spell_name (entry->GetSpellId()) 
	  << "<BR>\n";
    out << "<B>Class:</B> " << print_classes (entry->GetClasses()) << "<BR>\n";
    out << "<B>Race:</B> " << print_races (entry->GetRaces()) << "<BR>\n";
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
    unsigned char* rawData = NULL;
    size = itemDB->GetItemRawData(currentItemNr, &rawData);

    if ((size > 0) && (rawData != NULL))
    {
      out << "<P><B>Packet data: (" << size << " octets)</B>\n";
      if (size != sizeof(itemStruct))
	out << "<BR></BR><B class=\"warning\">Warning: ("
	    << size << " octets) != sizeof(itemStruct) (" 
	    << sizeof(itemStruct) 
	    << " octets): Data alignment is suspect!</B></FONT>\n";
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

void printdata (QTextStream& out, int len, unsigned char *data)
{
  char hex[1024];
  char asc[1024];
  char tmp[1024];

  const char* redUnknown = "</SPAN><SPAN class=\"unknown\">";
  const char* green      = "</SPAN><SPAN class=\"known1\">";
  const char* blue       = "</SPAN><SPAN class=\"known2\">";
  const char* col = green;
  hex[0] = 0;
  asc[0] = 0;
  int c;
  QString tempStr;

  for (c = 0; c < len; c++)
    {
      if ((!(c % 16)) && c)
	{
	  tempStr.sprintf("<SPAN class=\"head\">%03d | "
			  "%s</SPAN><SPAN class=\"head\"> | %s </SPAN>\n",
			  c - 16, hex, asc);
	  out << tempStr;
	  hex[0] = 0;
	  asc[0] = 0;
	}

      sprintf (tmp, "%02x ", data[c]);
      // switches which color next text to be displayed in based on
      //   RED = Unknown fields
      //   BLUE and GREEN = known fields
      switch (c)
	{
	case 35:
	  col = blue;
	  break;
	case 95:
	  col = green;
	  break;
	case 101:
	  col = blue;
	  break;
	case 103:
	  col = redUnknown;
	  break;
	case 125:
	  col = blue;
	  break;
	case 126:
	  col = green;
	  break;
	case 127:
	  col = blue;
	  break;
	case 128:
	  col = green;
	  break;
	case 129:
	  col = redUnknown;
	  break;
	case 130:
	  col = green;
	  break;
	case 132:
	  col = blue;
	  break;
	case 134:
	  col = green;
	  break;
	case 136:
	  col = blue;
	  break;
	case 140:
	  col = green;
	  break;
	case 144:
	  col = redUnknown;
	  break;
	case 172:
	  col = green;
	  break;
	case 173:
	  col = blue;
	  break;
	case 174:
	  col = green;
	  break;
	case 175:
	  col = blue;
	  break;
	case 176:
	  col = green;
	  break;
	case 177:
	  col = blue;
	  break;
	case 178:
	  col = green;
	  break;
	case 179:
	  col = blue;
	  break;
	case 180:
	  col = green;
	  break;
	case 181:
	  col = blue;
	  break;
	case 182:
	  col = green;
	  break;
	case 183:
	  col = blue;
	  break;
	case 184:
	  col = green;
	  break;
	case 185:
	  col = blue;
	  break;
	case 186:
	  col = green;
	  break;
	case 187:
	  col = redUnknown;
	  break;
	case 189:
	  col = green;
	  break;
	case 190:
	  col = blue;
	  break;
	case 191:
	  col = green;
	  break;
	case 192:
	  col = redUnknown;
	  break;
	case 193:
	  col = green;
	  break;
	case 194:
	  col = blue;
	  break;
	case 195:
	  col = green;
	  break;
	case 196:
	  col = blue;
	  break;
	case 197:
	  col = green;
	  break;
	case 198:
	  col = redUnknown;
	  break;
	case 200:
	  col = blue;
	  break;
	case 204:
	  col = redUnknown;
	  break;
	case 206:
	  col = green;
	  break;
	case 208:
	  col = blue;
	  break;
	case 210:
	  col = redUnknown;
	  break;
	case 212:
	  col = blue;
	  break;
	case 214:
	  col = redUnknown;
	  break;
	case 215:
	  col = blue;
	  break;
	case 216:
	  col = green;
	  break;
	case 217:
	  col = blue;
	  break;
	case 218:
	  col = green;
	  break;
	case 219:
	  col = redUnknown;
	  break;
	case 220:
	  col = blue;
	  break;
	case 222:
	  col = redUnknown;
	  break;
	}
      strcat (hex, col);
      strcat (hex, tmp);
      strcat (asc, col);

      // is the character printable
      if ((data[c] >= 32) && (data[c] <= 126))
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
      else // if it's not printable, then just insert a '.'
	strcpy (tmp, ".");
      strcat (asc, tmp);

    }
  if (c % 16)
    c = c - (c % 16);
  else
    c -= 16;

  tempStr.sprintf("<SPAN class=\"head\">%03d | "
		  "%-48s</SPAN><SPAN class=\"head\"> | %s </SPAN>\n\n",
		  c, hex, asc);

  out << tempStr;
}
