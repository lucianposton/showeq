/*
 * interface.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include "interface.h"
#include "util.h"
#include "main.h"
#include "editor.h"
#include "packet.h"
#include "zonemgr.h"
#include "compassframe.h"
#include "map.h"
#include "experiencelog.h"
#include "combatlog.h"
#include "filtermgr.h"
#include "spellshell.h"
#include "spawnlist.h"
#include "spelllist.h"
#include "player.h"
#include "skilllist.h"
#include "statlist.h"
#include "group.h"
#include "netdiag.h"
#include "spawnmonitor.h"
#include "spawnpointlist.h"
#include "spawnlist2.h"
#include "logger.h"
#include "spawnlog.h"
#include "packetlog.h"
#include "bazaarlog.h"
#include "category.h"
#include "itemdb.h"
#include "guild.h"
#include "guildshell.h"
#include "guildlist.h"
#include "spells.h"
#include "datetimemgr.h"
#include "datalocationmgr.h"
#include "eqstr.h"
#include "messagefilter.h"
#include "messages.h"
#include "messageshell.h"
#include "messagewindow.h"
#include "terminal.h"
#include "filteredspawnlog.h"
#include "messagefilterdialog.h"
#include "diagnosticmessages.h"
#include "filternotifications.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <qfont.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qstatusbar.h>
#include <qvaluelist.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qaccel.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qinputdialog.h>
#include <qfontdialog.h>
#include <qcolordialog.h>
#include <qdockarea.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qsgistyle.h>

// this define is used to diagnose the order with which zone packets are rcvd
#define ZONE_ORDER_DIAG

/* The main interface widget */
EQInterface::EQInterface(DataLocationMgr* dlm, 
			 QWidget * parent, const char *name) 
  : QMainWindow (parent, name),
    m_player(0),
    m_dataLocationMgr(dlm),
    m_mapMgr(0),
    m_spawnList(0),
    m_spawnList2(0),
    m_spellList(0),
    m_skillList(0),
    m_statList(0),
    m_spawnPointList(0),
    m_packet(0),
    m_zoneMgr(0),
    m_filterMgr(0),
    m_categoryMgr(0),
    m_spawnShell(0),
    m_spellShell(0),
    m_groupMgr(0),
    m_spawnMonitor(0),
    m_itemDB(0),
    m_guildmgr(0),
    m_guildShell(0),
    m_dateTimeMgr(0),
    m_eqStrings(0),
    m_messageFilters(0),
    m_messages(0),
    m_messageShell(0),
    m_terminal(0),
    m_filteredSpawnLog(0),
    m_filterNotifications(0),
    m_spawnLogger(0),
    m_globalLog(0),
    m_worldLog(0),
    m_zoneLog(0),
    m_bazaarLog(0),
    m_unknownZoneLog(0),
    m_opcodeMonitorLog(0),
    m_selectedSpawn(0),
    m_compass(0),
    m_expWindow(0),
    m_combatWindow(0),
    m_netDiag(0),
    m_messageFilterDialog(0),
    m_guildListWindow(0)
{
  // disable the dock menu
  setDockMenuEnabled(false);

  setCentralWidget(new QWidget(this, "filler"));
  
  setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum, false));

  for (int l = 0; l < maxNumMaps; l++)
    m_map[l] = 0;

  for (int l = 0; l < maxNumMessageWindows; l++)
    m_messageWindow[l] = 0;

  QString tempStr;
  QString section = "Interface";

  m_selectOnConsider = pSEQPrefs->getPrefBool("SelectOnCon", section, false);
  m_selectOnTarget = pSEQPrefs->getPrefBool("SelectOnTarget", section, false);


  const char* player_classes[] = {"Warrior", "Cleric", "Paladin", "Ranger",
				  "Shadow Knight", "Druid", "Monk", "Bard",
				  "Rogue", "Shaman", "Necromancer", "Wizard",
				  "Magician", "Enchanter", "Beastlord"
  };
  const char* player_races[] = {"Human", "Barbarian", "Erudite", "Wood elf",
				"High Elf", "Dark Elf", "Half Elf", "Dwarf",
				"Troll", "Ogre", "Halfling", "Gnome", "Iksar",
				"Vah Shir"
  };

   // set the applications default font
   if (pSEQPrefs->isPreference("Font", section))
   {
     QFont appFont = pSEQPrefs->getPrefFont("Font", section, qApp->font());
     qApp->setFont( appFont, true );
   }

   // initialize packet count
   m_initialcount = 0;
   m_packetStartTime = 0;

   // Create the date/time manager
   m_dateTimeMgr = new DateTimeMgr(this, "datetimemgr");

   // Create Message Filters object
   m_messageFilters = new MessageFilters(this, "messagefilters");

   // Create Messages storage
   m_messages = new Messages(m_dateTimeMgr, m_messageFilters, 
			     this, "messages");

   // Create the terminal object
   m_terminal = new Terminal(m_messages, this, "terminal");

   QString fileName, fileName2;
   QFileInfo fileInfo, fileInfo2;

   // Create the packet object
   section = "Network";
   QString vpsection = "VPacket";

   fileName = pSEQPrefs->getPrefString("WorldOPCodes", section, 
				       "worldopcodes.xml");

   fileInfo = m_dataLocationMgr->findExistingFile(".", fileName);

   fileName2 = pSEQPrefs->getPrefString("ZoneOPCodes", section, 
					"zoneopcodes.xml");

   fileInfo2 = m_dataLocationMgr->findExistingFile(".", fileName2);

   m_packet = new EQPacket(fileInfo.absFilePath(),
			   fileInfo2.absFilePath(),
			   pSEQPrefs->getPrefInt("ArqSeqGiveUp", section, 512),
			   pSEQPrefs->getPrefString("Device", section, "eth0"),
			   pSEQPrefs->getPrefString("IP", section,
						    AUTOMATIC_CLIENT_IP),
			   pSEQPrefs->getPrefString("MAC", section, "0"),
			   pSEQPrefs->getPrefBool("RealTimeThread", section,
						  false),
			   pSEQPrefs->getPrefBool("SessionTracking", 
						  section, false),
			   pSEQPrefs->getPrefBool("Record", vpsection, false),
			   pSEQPrefs->getPrefBool("Playback", vpsection,
						  false),
			   pSEQPrefs->getPrefInt("PlaybackRate", vpsection, 
						 false),
			   this, "packet");
   
  ipstr[0] = m_packet->ip();	//Retrieves last IP used in previous session 
  for( int i = 1; i < 5; i++) 
  ipstr[i] = "0.0.0.0";

  macstr[0] = m_packet->mac();	//Retrieves last MAC used in previous session
  for( int i = 1; i < 5; i++)  
  macstr[i] = "00:00:00:00:00:00";

  
   // setup the user directory
   m_dataLocationMgr->setupUserDirectory();

   section = "Interface";
			      
   // Create the Spells object
   fileName = pSEQPrefs->getPrefString("SpellsFile", section,
					       "spells_us.txt");

   fileInfo = m_dataLocationMgr->findExistingFile(".", fileName);
   
   m_spells = new Spells(fileInfo.absFilePath());
   
   // Create the EQStr storage
   m_eqStrings = new EQStr(8009); // increase if the number of stings exeeds

   // Create the Zone Manager
   m_zoneMgr = new ZoneMgr(this, "zonemgr");

   // Create GuildMgr object
   fileName = pSEQPrefs->getPrefString("GuildsFile", "Interface",
				       "guilds2.dat");
   
   fileInfo = m_dataLocationMgr->findWriteFile("tmp", fileName);

   m_guildmgr = new GuildMgr(fileInfo.absFilePath(), 
			     this, "guildmgr");

   // Create our player object
   m_player = new Player(this, m_zoneMgr, m_guildmgr);

   section = "ItemDB";
   if (pSEQPrefs->getPrefBool("Enabled", section, false))
   {
     seqDebug("ItemDB: Enabled");

     // Create an instance of the ItemDB
     m_itemDB = new EQItemDB(m_dataLocationMgr);

     // make it's parameters match those set via the config file and 
     // command line
     fileName = pSEQPrefs->getPrefString("DataDBFilename", section, 
					 "itemdata2");

     fileInfo = m_dataLocationMgr->findWriteFile(".", fileName);

     m_itemDB->SetDBFile(EQItemDB::DATA_DB, fileInfo.absFilePath());

     fileName = pSEQPrefs->getPrefString("RawDataDBFilename", section, 
					 "itemrawdata2");

     m_itemDB->SetDBFile(EQItemDB::RAW_DATA_DB, fileInfo.absFilePath());
     m_itemDB->SetEnabledDBTypes(pSEQPrefs->getPrefInt("DatabasesEnabled",
						       section, 
						       (EQItemDB::DATA_DB|EQItemDB::RAW_DATA_DB)));
     m_itemDB->SetItemLogging(pSEQPrefs->getPrefBool("ItemLogging", "ItemDB", false));
     m_itemDB->SetItemPacketLogging(pSEQPrefs->getPrefBool("ItemPackingLogging", "ItemDB", false));
     // Make sure the databases are upgraded to the current format
     m_itemDB->Upgrade();
   }
   else
     seqDebug("ItemDB: Disabled");

   // Create the filter manager
   m_filterMgr = new FilterMgr(m_dataLocationMgr,
			       pSEQPrefs->getPrefString("FilterFile", 
							section, "global.xml"),
			       pSEQPrefs->getPrefBool("IsCaseSensitive", 
						      section, false));

   // if there is a short zone name already, try to load its filters
   QString shortZoneName = m_zoneMgr->shortZoneName();
   if (!shortZoneName.isEmpty())
     m_filterMgr->loadZone(shortZoneName);
   
   m_guildShell = new GuildShell(m_zoneMgr, this, "GuildShell");

   // Create the spawn shell
   m_spawnShell = new SpawnShell(*m_filterMgr, m_zoneMgr, m_player, m_guildmgr);

   // Create the Category manager
   m_categoryMgr = new CategoryMgr();

   // Create the map manager
   m_mapMgr = new MapMgr(m_dataLocationMgr, m_spawnShell, m_player, m_zoneMgr,
			 this);

   // Create the spell shell
   m_spellShell = new SpellShell(m_player, m_spawnShell, m_spells);

   // Create the Spawn Monitor
   m_spawnMonitor = new SpawnMonitor(m_dataLocationMgr, 
				     m_zoneMgr, m_spawnShell);

   // Create the Group Manager
   m_groupMgr = new GroupMgr(m_spawnShell, m_player, this, "groupmgr");

   // Create the message shell
   m_messageShell = new MessageShell(m_messages, m_eqStrings, m_spells, 
				     m_zoneMgr, m_spawnShell, m_player,
				     this, "messageshell");

   // Create the Filter Notifications object
   m_filterNotifications = new FilterNotifications(this, "filternotifications");
   
   // Create log objects as necessary
   if (pSEQPrefs->getPrefBool("LogAllPackets", "PacketLogging", false))
     createGlobalLog();

   if (pSEQPrefs->getPrefBool("LogZonePackets", "PacketLogging", false))
     createZoneLog();

   if (pSEQPrefs->getPrefBool("LogBazaarPackets", "PacketLogging", false))
     createBazaarLog();

   if (pSEQPrefs->getPrefBool("LogWorldPackets", "PacketLogging", false))
     createWorldLog();

   if (pSEQPrefs->getPrefBool("LogUnknownZonePackets", "PacketLogging", false))
     createUnknownZoneLog();

   section = "OpCodeMonitoring";
   if (pSEQPrefs->getPrefBool("Enable", section, false))
     createOPCodeMonitorLog(pSEQPrefs->getPrefString("OpCodeList", section, ""));

   // create the filtered spawn log object if any filters are to be logged
   uint32_t filters = pSEQPrefs->getPrefInt("Log", "Filters", 0);
   if (filters)
   {
     // create the filtered spawn log object
     createFilteredSpawnLog();

     // set the filters to log
     m_filteredSpawnLog->setFilters(filters);
   }

   // if the user wants spawns logged, create the spawn logger
   if (pSEQPrefs->getPrefBool("LogSpawns", "Misc", false))
     createSpawnLog();

   section = "Interface";

   // create window menu
   m_windowMenu = new QPopupMenu;

   // Initialize the experience window;
   m_expWindow = new ExperienceWindow(m_dataLocationMgr, m_player, m_groupMgr,
				      m_zoneMgr);
   setDockEnabled(m_expWindow, 
		  pSEQPrefs->getPrefBool("DockableExperienceWindow",
					 section, false));
   Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					   m_expWindow->preferenceName(),
					   DockUnmanaged);
   addDockWindow(m_expWindow, edge, false);
   m_expWindow->undock();

   m_expWindow->restoreSize();

   // move window to new position
   if (pSEQPrefs->getPrefBool("UseWindowPos", section, true))
     m_expWindow->restorePosition();
   
   if (pSEQPrefs->getPrefBool("ShowExpWindow", section, false))
     m_expWindow->show();

    // insert its menu into the window menu
   insertWindowMenu(m_expWindow);

   // Initialize the combat window
   m_combatWindow = new CombatWindow(m_player);
   setDockEnabled(m_combatWindow, 
		  pSEQPrefs->getPrefBool("DockableCombatWindow",
					 section, false));
   edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
				      m_combatWindow->preferenceName(),
				      DockUnmanaged);
   addDockWindow(m_combatWindow, edge, false);
   m_combatWindow->undock();

   m_combatWindow->restoreSize();

   // move window to new position
   if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", true))
     m_combatWindow->restorePosition();

   if (pSEQPrefs->getPrefBool("ShowCombatWindow", section, false))
     m_combatWindow->show();

    // insert its menu into the window menu
   insertWindowMenu(m_combatWindow);

/////////////////
// Main widgets
   // Make a VBox to use as central widget
   // Create/display the Map(s)
   for (int i = 0; i < maxNumMaps; i++)
   {
     QString tmpPrefSuffix = "";
     if (i > 0)
       tmpPrefSuffix = QString::number(i + 1);
     
     // construct the preference name
     QString tmpPrefName = QString("DockedMap") + tmpPrefSuffix;

     // retrieve if the map should be docked
     m_isMapDocked[i] = pSEQPrefs->getPrefBool(tmpPrefName, section, (i == 0));

     // construct the preference name
     tmpPrefName = QString("ShowMap") + tmpPrefSuffix;

     // and as appropriate, craete the map
     if (pSEQPrefs->getPrefBool(tmpPrefName, section, (i == 0)))
       showMap(i);
   }

   // Create/display the MessageWindow(s)
   for (int i = 0; i < maxNumMessageWindows; i++)
   {
     QString tmpPrefSuffix = "";
     if (i > 0)
       tmpPrefSuffix = QString::number(i + 1);
     
     // construct the preference name
     QString tmpPrefName = QString("DockedMessageWindow") + tmpPrefSuffix;

     // retrieve if the message window should be docked
     m_isMessageWindowDocked[i] = 
       pSEQPrefs->getPrefBool(tmpPrefName, section, false);

     // construct the preference name
     tmpPrefName = QString("ShowMessageWindow") + tmpPrefSuffix;

     // and as appropriate, craete the message window
     if (pSEQPrefs->getPrefBool(tmpPrefName, section, false))
       showMessageWindow(i);
   }

   // should the compass be docked if it's created
   m_isCompassDocked = pSEQPrefs->getPrefBool("DockedCompass", section, true);

   //
   // Create the Player Skills listview (ONLY CREATED WHEN NEEDED FLOYD!!!!)
   //
   m_isSkillListDocked = pSEQPrefs->getPrefBool("DockedPlayerSkills", section, true);
   if (pSEQPrefs->getPrefBool("ShowPlayerSkills", section, true))
     showSkillList();

   //
   // Create the Player Status listview (ONLY CREATED WHEN NEEDED FLOYD!!!!)
   //
   m_isStatListDocked = pSEQPrefs->getPrefBool("DockedPlayerStats", section, true);
   if (pSEQPrefs->getPrefBool("ShowPlayerStats", section, true))
     showStatList();

   //
   // Create the compass as required
   //
   if (pSEQPrefs->getPrefBool("ShowCompass", section, false))
       showCompass();

   //
   // Create the spells listview as required (dynamic object)
   //
   m_isSpellListDocked = pSEQPrefs->getPrefBool("DockedSpellList", section, true);
   if (pSEQPrefs->getPrefBool("ShowSpellList", section, false))
       showSpellList();


   //
   // Create the Spawn List listview (always exists, just hidden if not specified)
   //
   m_isSpawnListDocked = pSEQPrefs->getPrefBool("DockedSpawnList", section, true);
   if (pSEQPrefs->getPrefBool("ShowSpawnList", section, false))
     showSpawnList();

   //
   // Create the Spawn List2 listview (always exists, just hidden if not specified)
   //
   m_isSpawnList2Docked = pSEQPrefs->getPrefBool("DockedSpawnList2", section, true);
   if (pSEQPrefs->getPrefBool("ShowSpawnList2", section, true))
     showSpawnList2();

   //
   // Create the Spawn List listview (always exists, just hidden if not specified)
   //
   m_isSpawnPointListDocked = pSEQPrefs->getPrefBool("DockedSpawnPointList", section, false);
   if (pSEQPrefs->getPrefBool("ShowSpawnPointList", section, false))
     showSpawnPointList();

   //
   // Create the Net Statistics window as required
   // 
   if (pSEQPrefs->getPrefBool("ShowNetStats", section, false))
     showNetDiag();

   //
   // Create the Guild member List window as required
   if (pSEQPrefs->getPrefBool("ShowGuildList", section, false))
     showGuildList();

/////////////////////
// QMenuBar

   // The first call to menuBar() makes it exist
   menuBar()->setSeparator(QMenuBar::InWindowsStyle);

   // File Menu
   //pFileMenu = new QPopupMenu;
   QPopupMenu* pFileMenu = new QPopupMenu;
   menuBar()->insertItem("&File", pFileMenu);
   pFileMenu->insertItem("&Save Preferences", this, SLOT(savePrefs()), CTRL+Key_S);
   pFileMenu->insertItem("Open &Map", m_mapMgr, SLOT(loadMap()), Key_F1);
   pFileMenu->insertItem("&Import &Map", m_mapMgr, SLOT(importMap()));
   pFileMenu->insertItem("Sa&ve Map", m_mapMgr, SLOT(saveMap()), Key_F2);
   pFileMenu->insertItem("Save SOE Map", m_mapMgr, SLOT(saveSOEMap()));
   pFileMenu->insertItem("Reload Guilds File", m_guildmgr, SLOT(readGuildList()));
   pFileMenu->insertItem("Add Spawn Category", this, SLOT(addCategory()) , ALT+Key_C);
   pFileMenu->insertItem("Rebuild SpawnList", this, SLOT(rebuildSpawnList()) , ALT+Key_R);
   pFileMenu->insertItem("Reload Categories", this, SLOT(reloadCategories()) , CTRL+Key_R);
   pFileMenu->insertItem("Select Next", this, SLOT(selectNext()), CTRL+Key_Right);
   pFileMenu->insertItem("Select Prev", this, SLOT(selectPrev()), CTRL+Key_Left);
   pFileMenu->insertItem("Save Selected Spawns Path", 
			 this, SLOT(saveSelectedSpawnPath(void)));
   pFileMenu->insertItem("Save NPC Spawn Paths",
			 this, SLOT(saveSpawnPaths(void)));
   if (m_packet->playbackPackets())
   {
     pFileMenu->insertItem("Inc Playback Speed", m_packet, SLOT(incPlayback()), CTRL+Key_X);
     pFileMenu->insertItem("Dec Playback Speed", m_packet, SLOT(decPlayback()), CTRL+Key_Z);
   }
   pFileMenu->insertItem("&Quit", qApp, SLOT(quit()));

   // View menu
   QPopupMenu* pViewMenu = new QPopupMenu;
   menuBar()->insertItem("&View", pViewMenu);
   pViewMenu->setCheckable(true);
   m_id_view_ExpWindow = pViewMenu->insertItem("Experience Window", this, SLOT(toggle_view_ExpWindow()));
   m_id_view_CombatWindow = pViewMenu->insertItem("Combat Window", this, SLOT(toggle_view_CombatWindow()));
   pViewMenu->insertSeparator(-1);
   m_id_view_SpellList = pViewMenu->insertItem("Spell List", this, SLOT(toggle_view_SpellList()));
   m_id_view_SpawnList = pViewMenu->insertItem("Spawn List", this, SLOT(toggle_view_SpawnList()));
   m_id_view_SpawnList2 = pViewMenu->insertItem("Spawn List 2", this, SLOT(toggle_view_SpawnList2()));
   m_id_view_SpawnPointList = pViewMenu->insertItem("Spawn Point List", this, SLOT(toggle_view_SpawnPointList()));
   m_id_view_PlayerStats = pViewMenu->insertItem("Player Stats", this, SLOT(toggle_view_PlayerStats()));
   m_id_view_PlayerSkills = pViewMenu->insertItem("Player Skills", this,SLOT(toggle_view_PlayerSkills()));
   m_id_view_Compass = pViewMenu->insertItem("Compass", this, SLOT(toggle_view_Compass()));
   menuBar()->setItemChecked(m_id_view_PlayerStats, (m_statList != 0));

   QPopupMenu* subMenu = new QPopupMenu;
   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map ";
        if (i > 0)
            mapName += QString::number(i + 1);
        m_id_view_Map[i] = subMenu->insertItem(mapName, this, SLOT(toggle_view_Map(int)));
        subMenu->setItemParameter(m_id_view_Map[i], i);
        subMenu->setItemChecked(m_id_view_Map[i], (m_map[i] != 0));
   }
   pViewMenu->insertItem("Maps", subMenu);

   subMenu = new QPopupMenu;
   QString messageWindowName;
   for (int i = 0; i < maxNumMessageWindows; i++)
   {     
        messageWindowName = "Channel Messages ";
        if (i > 0)
	  messageWindowName += QString::number(i + 1);
        m_id_view_MessageWindow[i] = subMenu->insertItem(messageWindowName, this, SLOT(toggle_view_ChannelMsgs(int)));
        subMenu->setItemParameter(m_id_view_MessageWindow[i], i);
        subMenu->setItemChecked(m_id_view_MessageWindow[i],
				(m_messageWindow[i] != 0));
   }
   pViewMenu->insertItem("Channel Mesages", subMenu);


   m_id_view_NetDiag = pViewMenu->insertItem("Network Diagnostics", this, SLOT(toggle_view_NetDiag()));

   m_id_view_GuildListWindow = 
     pViewMenu->insertItem("Guild Member List", this, 
			   SLOT(toggle_view_GuildList()));

   pViewMenu->insertSeparator(-1);

   // View -> PlayerStats
   m_statWinMenu = new QPopupMenu;
   m_id_view_PlayerStats_Options = pViewMenu->insertItem( "&Player Stats", m_statWinMenu);
   m_statWinMenu->setCheckable(TRUE);

   m_id_view_PlayerStats_Stats[LIST_HP] = m_statWinMenu->insertItem("Hit Points");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_HP], LIST_HP);
    
   m_id_view_PlayerStats_Stats[LIST_MANA] = m_statWinMenu->insertItem("Mana");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_MANA], LIST_MANA);
    
   m_id_view_PlayerStats_Stats[LIST_STAM] = m_statWinMenu->insertItem("Stamina");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_STAM], LIST_STAM);
    
   m_id_view_PlayerStats_Stats[LIST_EXP] = m_statWinMenu->insertItem("Experience");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_EXP], LIST_EXP);
    
   m_id_view_PlayerStats_Stats[LIST_ALTEXP] = m_statWinMenu->insertItem("Alt Experience");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_ALTEXP], LIST_ALTEXP);

   m_id_view_PlayerStats_Stats[LIST_FOOD] = m_statWinMenu->insertItem("Food");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_FOOD], LIST_FOOD);
    
   m_id_view_PlayerStats_Stats[LIST_WATR] = m_statWinMenu->insertItem("Water");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_WATR], LIST_WATR);
   
   m_statWinMenu->insertSeparator(-1);
    
   m_id_view_PlayerStats_Stats[LIST_STR] = m_statWinMenu->insertItem("Strength");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_STR], LIST_STR);
    
   m_id_view_PlayerStats_Stats[LIST_STA] = m_statWinMenu->insertItem("Stamina");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_STA], LIST_STA);
    
   m_id_view_PlayerStats_Stats[LIST_CHA] = m_statWinMenu->insertItem("Charisma");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_CHA], LIST_CHA);
    
   m_id_view_PlayerStats_Stats[LIST_DEX] = m_statWinMenu->insertItem("Dexterity");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_DEX], LIST_DEX);
    
   m_id_view_PlayerStats_Stats[LIST_INT] = m_statWinMenu->insertItem("Intelligence");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_INT], LIST_INT);
    
   m_id_view_PlayerStats_Stats[LIST_AGI] = m_statWinMenu->insertItem("Agility");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_AGI], LIST_AGI);
    
   m_id_view_PlayerStats_Stats[LIST_WIS] = m_statWinMenu->insertItem("Wisdom");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_WIS], LIST_WIS);
    
   m_statWinMenu->insertSeparator(-1);
    
   m_id_view_PlayerStats_Stats[LIST_MR] = m_statWinMenu->insertItem("Magic Res");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_MR], LIST_MR);
   m_statWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_MR], false);
    
   m_id_view_PlayerStats_Stats[LIST_FR] = m_statWinMenu->insertItem("Fire Res");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_FR], LIST_FR);
   m_statWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_FR], false);
    
   m_id_view_PlayerStats_Stats[LIST_CR] = m_statWinMenu->insertItem("Cold Res");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_CR], LIST_CR);
   m_statWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_CR], false);
    
   m_id_view_PlayerStats_Stats[LIST_DR] = m_statWinMenu->insertItem("Disease Res");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_DR], LIST_DR);
   m_statWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_DR], false);
    
   m_id_view_PlayerStats_Stats[LIST_PR] = m_statWinMenu->insertItem("Poison Res");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_PR], LIST_PR);
   m_statWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_PR], false);

   m_statWinMenu->insertSeparator(-1);
    
   m_id_view_PlayerStats_Stats[LIST_AC] = m_statWinMenu->insertItem("Armor Class");
   m_statWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_AC], LIST_AC);
   m_statWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_AC], false);
   connect (m_statWinMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_StatWin(int)));

   // View -> PlayerSkills
   m_skillWinMenu = new QPopupMenu;
   m_id_view_PlayerSkills_Options = pViewMenu->insertItem( "Player &Skills", m_skillWinMenu);
   pViewMenu->setItemEnabled(m_id_view_PlayerSkills_Options, (m_skillList != 0));
   m_skillWinMenu->setCheckable(TRUE);
   
   m_id_view_PlayerSkills_Languages = m_skillWinMenu->insertItem("&Langauges");
   m_skillWinMenu->setItemParameter(m_id_view_PlayerSkills_Languages,0);

   connect (m_skillWinMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_SkillWin(int)));

   // View -> SpawnList
    m_spawnListMenu = new QPopupMenu;
    m_id_view_SpawnList_Options = pViewMenu->insertItem( "Spawn &List", m_spawnListMenu);
    pViewMenu->setItemEnabled(m_id_view_SpawnList_Options, (m_spawnList != 0));
    m_spawnListMenu->setCheckable(TRUE);

    m_id_view_SpawnList_Cols[tSpawnColName] = m_spawnListMenu->insertItem("&Name");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColName], tSpawnColName);

    m_id_view_SpawnList_Cols[tSpawnColLevel] = m_spawnListMenu->insertItem("&Level");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColLevel], tSpawnColLevel);

    m_id_view_SpawnList_Cols[tSpawnColHP] = m_spawnListMenu->insertItem("&HP");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColHP], tSpawnColHP);

    m_id_view_SpawnList_Cols[tSpawnColGuildID] = m_spawnListMenu->insertItem("Guild Tag");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColGuildID], tSpawnColGuildID);
    m_id_view_SpawnList_Cols[tSpawnColMaxHP] = m_spawnListMenu->insertItem("&Max HP");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColMaxHP], tSpawnColMaxHP);

    m_id_view_SpawnList_Cols[tSpawnColXPos] = m_spawnListMenu->insertItem("Coord &1");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColXPos], tSpawnColXPos);

    m_id_view_SpawnList_Cols[tSpawnColYPos] = m_spawnListMenu->insertItem("Coord &2");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColYPos], tSpawnColYPos);

    m_id_view_SpawnList_Cols[tSpawnColZPos] = m_spawnListMenu->insertItem("Coord &3");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColZPos], tSpawnColZPos);

    m_id_view_SpawnList_Cols[tSpawnColID] = m_spawnListMenu->insertItem("I&D");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColID], tSpawnColID);

    m_id_view_SpawnList_Cols[tSpawnColDist] = m_spawnListMenu->insertItem("&Dist");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColDist], tSpawnColDist);

    m_id_view_SpawnList_Cols[tSpawnColRace] = m_spawnListMenu->insertItem("&Race");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColRace], tSpawnColRace);

    m_id_view_SpawnList_Cols[tSpawnColClass] = m_spawnListMenu->insertItem("&Class");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColClass], tSpawnColClass);

    m_id_view_SpawnList_Cols[tSpawnColInfo] = m_spawnListMenu->insertItem("&Info");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColInfo], tSpawnColInfo);

    m_id_view_SpawnList_Cols[tSpawnColSpawnTime] = m_spawnListMenu->insertItem("Spawn &Time");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[tSpawnColSpawnTime], tSpawnColSpawnTime);

    connect (m_spawnListMenu, SIGNAL(activated(int)), 
	     this, SLOT(toggle_view_SpawnListCol(int)));

   pViewMenu->insertSeparator(-1);

   int x;
   // View -> DockedWin
   m_dockedWinMenu = new QPopupMenu;
   pViewMenu->insertItem( "&Docked", m_dockedWinMenu);
   m_dockedWinMenu->setCheckable(true);
    
   x = m_dockedWinMenu->insertItem("Spawn &List");
   m_dockedWinMenu->setItemParameter(x, 0);
   m_dockedWinMenu->setItemChecked(x, m_isSpawnListDocked);

   x = m_dockedWinMenu->insertItem("Spawn &List 2");
   m_dockedWinMenu->setItemParameter(x, 6);
   m_dockedWinMenu->setItemChecked(x, m_isSpawnListDocked);
    
   x = m_dockedWinMenu->insertItem("Spawn P&oint List");
   m_dockedWinMenu->setItemParameter(x, 5);
   m_dockedWinMenu->setItemChecked(x, m_isSpawnPointListDocked);
    
   x = m_dockedWinMenu->insertItem("&Player Stats");
   m_dockedWinMenu->setItemParameter(x, 1);
   m_dockedWinMenu->setItemChecked(x, m_isStatListDocked);
    
   x = m_dockedWinMenu->insertItem("Player &Skills");
   m_dockedWinMenu->setItemParameter(x, 2);
   m_dockedWinMenu->setItemChecked(x, m_isSkillListDocked);
    
   x = m_dockedWinMenu->insertItem("Sp&ell List");
   m_dockedWinMenu->setItemParameter(x, 3);
   m_dockedWinMenu->setItemChecked(x, m_isSpellListDocked);
    
   x = m_dockedWinMenu->insertItem("&Compass");
   m_dockedWinMenu->setItemParameter(x, 4);
   m_dockedWinMenu->setItemChecked(x, m_isCompassDocked);

   // insert Map docking options 
   // NOTE: Always insert Map docking options at the end of the Docked menu
   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map ";
        if (i > 0)
            mapName += QString::number(i + 1);
        x = m_dockedWinMenu->insertItem(mapName);
        m_dockedWinMenu->setItemParameter(x, i + mapDockBase);
        m_dockedWinMenu->setItemChecked(x, m_isMapDocked[i]);
   }
    
   connect (m_dockedWinMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_DockedWin(int)));

   // View -> DockableWin
   m_dockableWinMenu = new QPopupMenu;
   pViewMenu->insertItem( "&Dockable", m_dockableWinMenu);
   m_dockableWinMenu->setCheckable(true);
    
   x = m_dockableWinMenu->insertItem("Spawn &List");
   m_dockableWinMenu->setItemParameter(x, 0);
   m_dockableWinMenu->setItemChecked(x, 
				     pSEQPrefs->getPrefBool("DockableSpawnList",
							    section, true));

   x = m_dockableWinMenu->insertItem("Spawn &List 2");
   m_dockableWinMenu->setItemParameter(x, 6);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableSpawnList2",
							    section, true));
    
   x = m_dockableWinMenu->insertItem("Spawn P&oint List");
   m_dockableWinMenu->setItemParameter(x, 5);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableSpawnPointList",
							    section, true));
    
   x = m_dockableWinMenu->insertItem("&Player Stats");
   m_dockableWinMenu->setItemParameter(x, 1);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockablePlayerStats",
							    section, true));
    
   x = m_dockableWinMenu->insertItem("Player &Skills");
   m_dockableWinMenu->setItemParameter(x, 2);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockablePlayerSkills",
							    section, true));
    
   x = m_dockableWinMenu->insertItem("Sp&ell List");
   m_dockableWinMenu->setItemParameter(x, 3);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableSpellList",
							    section, true));
    
   x = m_dockableWinMenu->insertItem("&Compass");
   m_dockableWinMenu->setItemParameter(x, 4);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableCompass",
							    section, true));

   x = m_dockableWinMenu->insertItem("E&xperience Window");
   m_dockableWinMenu->setItemParameter(x, 7);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableExperienceWindow",
							    section, false));

   x = m_dockableWinMenu->insertItem("Com&bat Window");
   m_dockableWinMenu->setItemParameter(x, 8);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableCombatWindow",
							    section, false));

   x = m_dockableWinMenu->insertItem("&Guild List");
   m_dockableWinMenu->setItemParameter(x, 9);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableGuildListWindow",
							    section, true));

   x = m_dockableWinMenu->insertItem("&Net Diag");
   m_dockableWinMenu->setItemParameter(x, 10);
   m_dockableWinMenu->setItemChecked(x,  
				     pSEQPrefs->getPrefBool("DockableNetDiag",
							    section, true));

   // insert Map docking options 
   subMenu = new QPopupMenu;
   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map ";
	QString mapPrefName = "Map";
        if (i > 0)
	{
            mapName += QString::number(i + 1);
	    mapPrefName + QString::number(i + 1);
	}
        x = subMenu->insertItem(mapName);
        subMenu->setItemParameter(x, i + mapDockBase);
	subMenu->setItemChecked(x, pSEQPrefs->getPrefBool(QString("Dockable")
							  + mapName,
							  section, true));
   }
   connect (subMenu, SIGNAL(activated(int)), this, 
	    SLOT(toggle_view_DockableWin(int)));
   m_dockableWinMenu->insertItem("Maps", subMenu);

   // insert Message Window docking options 
   subMenu = new QPopupMenu;
   QString messagePrefName = "DockableMessageWindow";
   for (int i = 0; i < maxNumMessageWindows; i++)
   {     
        QString messageWindowName = "Channel Messages ";
	if (i > 0)
	  messageWindowName += QString::number(i + 1);
        x = subMenu->insertItem(messageWindowName);
        subMenu->setItemParameter(x, i + messageWindowDockBase);
	subMenu->setItemChecked(x, pSEQPrefs->getPrefBool(messagePrefName + 
							  QString::number(i),
							  section, false));
   }
   connect (subMenu, SIGNAL(activated(int)), this, 
	    SLOT(toggle_view_DockableWin(int)));
   m_dockableWinMenu->insertItem("Channel Messages", subMenu);
    
   connect (m_dockableWinMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_DockableWin(int)));


   // view menu checks are set by init_view_menu
  connect(pViewMenu, SIGNAL(aboutToShow()),
	  this, SLOT(init_view_menu()));


   // Options Menu
   QPopupMenu* pOptMenu = new QPopupMenu;
   menuBar()->insertItem("&Options", pOptMenu);
   pOptMenu->setCheckable(TRUE);
   m_id_opt_Fast     = pOptMenu->insertItem("Fast Machine?", this, SLOT(toggle_opt_Fast()));
   m_id_opt_ConSelect = pOptMenu->insertItem("Select on Consider?", this, SLOT(toggle_opt_ConSelect()));
   m_id_opt_TarSelect = pOptMenu->insertItem("Select on Target?", this, SLOT(toggle_opt_TarSelect()));
   m_id_opt_KeepSelectedVisible =
                  pOptMenu->insertItem("Keep Selected Visible?"  , this, SLOT(toggle_opt_KeepSelectedVisible()));
   m_id_opt_LogSpawns = pOptMenu->insertItem("Log Spawns", this, SLOT(toggle_opt_LogSpawns()));
   m_id_opt_BazaarData    = pOptMenu->insertItem("Bazaar Searches", this, SLOT(toggle_opt_BazaarData()), Key_F11);
   menuBar()->setItemChecked (m_id_opt_BazaarData, (m_bazaarLog != 0));
   m_id_opt_ResetMana = pOptMenu->insertItem("Reset Max Mana", this, SLOT(resetMaxMana()));
   m_id_opt_PvPTeams  = pOptMenu->insertItem("PvP Teams", this, SLOT(toggle_opt_PvPTeams()));
   m_id_opt_PvPDeity  = pOptMenu->insertItem("PvP Deity", this, SLOT(toggle_opt_PvPDeity()));
   x = pOptMenu->insertItem("Create Unknown Spawns", 
			    this, SLOT(toggle_opt_CreateUnknownSpawns(int)));
   menuBar()->setItemChecked (x, showeq_params->createUnknownSpawns);
   x = pOptMenu->insertItem("Use EQ Retarded Coordinates", 
			    this, SLOT(toggle_opt_RetardedCoords(int)));
   menuBar()->setItemChecked (x, showeq_params->retarded_coords);
   x = pOptMenu->insertItem("Use Unix System Time for Spawn Time", 
			    this, SLOT(toggle_opt_SystimeSpawntime(int)));
   menuBar()->setItemChecked (x, showeq_params->systime_spawntime);
   x = pOptMenu->insertItem("Record Spawn Walk Paths", 
			    this, SLOT(toggle_opt_WalkPathRecord(int)));
   menuBar()->setItemChecked (x, showeq_params->walkpathrecord);

   subMenu = new QPopupMenu;
   QSpinBox* walkPathLengthSpinBox = new QSpinBox(0, 8192, 1, subMenu);
   walkPathLengthSpinBox->setValue(showeq_params->walkpathlength);
   connect(walkPathLengthSpinBox, SIGNAL(valueChanged(int)),
	   this, SLOT(set_opt_WalkPathLength(int)));
   subMenu->insertItem(walkPathLengthSpinBox);
   pOptMenu->insertItem("Walk Path Length", 
			subMenu);
   
   menuBar()->setItemChecked (m_id_opt_Fast, showeq_params->fast_machine);
   menuBar()->setItemChecked (m_id_opt_ConSelect, m_selectOnConsider);
   menuBar()->setItemChecked (m_id_opt_TarSelect, m_selectOnTarget);
   menuBar()->setItemChecked (m_id_opt_KeepSelectedVisible, showeq_params->keep_selected_visible);
   menuBar()->setItemChecked (m_id_opt_LogSpawns, (m_spawnLogger != 0));
   menuBar()->setItemChecked (m_id_opt_PvPTeams, showeq_params->pvp);
   menuBar()->setItemChecked (m_id_opt_PvPDeity, showeq_params->deitypvp);

   // SaveState SubMenu
   QPopupMenu* pSaveStateMenu = new QPopupMenu;
   pOptMenu->insertItem("&Save State", pSaveStateMenu);
   pSaveStateMenu->setCheckable(true);
   x = pSaveStateMenu->insertItem("&Player", this, 
				  SLOT(toggle_opt_save_PlayerState(int)));
   pSaveStateMenu->setItemChecked(x, showeq_params->savePlayerState);
   x = pSaveStateMenu->insertItem("&Zone", this, 
				  SLOT(toggle_opt_save_ZoneState(int)));
   pSaveStateMenu->setItemChecked(x, showeq_params->saveZoneState);
   x = pSaveStateMenu->insertItem("&Spawns", this, 
				  SLOT(toggle_opt_save_Spawns(int)));
   pSaveStateMenu->setItemChecked(x, showeq_params->saveSpawns);
   pSaveStateMenu->insertItem("Base &Filename...", this, 
			      SLOT(set_opt_save_BaseFilename(void)));

   pSaveStateMenu->insertSeparator(-1);
   subMenu = new QPopupMenu;
   QSpinBox* saveFrequencySpinBox = new QSpinBox(1, 320, 1, subMenu);
   saveFrequencySpinBox->setValue(showeq_params->saveSpawnsFrequency / 1000);
   connect(saveFrequencySpinBox, SIGNAL(valueChanged(int)),
	   this, SLOT(set_opt_save_SpawnFrequency(int)));
   subMenu->insertItem(saveFrequencySpinBox);
   pSaveStateMenu->insertItem("Spawn Save Frequency (s)", 
			      subMenu);

   pOptMenu->insertItem("Clear Channel Messages", 
			this, SLOT(opt_clearChannelMsgs(int)));

   // Con Color base menu
   QPopupMenu* conColorBaseMenu = new QPopupMenu;
   x = conColorBaseMenu->insertItem("Green Spawn Base...");
   conColorBaseMenu->setItemParameter(x, tGreenSpawn);
   x = conColorBaseMenu->insertItem("Cyan Spawn Base...");
   conColorBaseMenu->setItemParameter(x, tCyanSpawn);
   x = conColorBaseMenu->insertItem("Blue Spawn Base...");
   conColorBaseMenu->setItemParameter(x, tBlueSpawn);
   x = conColorBaseMenu->insertItem("Even Spawn...");
   conColorBaseMenu->setItemParameter(x, tEvenSpawn);
   x = conColorBaseMenu->insertItem("Yellow Spawn Base...");
   conColorBaseMenu->setItemParameter(x, tYellowSpawn);
   x = conColorBaseMenu->insertItem("Red Spawn Base...");
   conColorBaseMenu->setItemParameter(x, tRedSpawn);
   x = conColorBaseMenu->insertItem("Unknown Spawn...");
   conColorBaseMenu->setItemParameter(x, tUnknownSpawn);
   connect(conColorBaseMenu, SIGNAL(activated(int)),
	   this, SLOT(select_opt_conColorBase(int)));
   pOptMenu->insertItem("Con &Colors", conColorBaseMenu);

   // Network Menu
   m_netMenu = new QPopupMenu;
   menuBar()->insertItem("&Network", m_netMenu);
   m_netMenu->insertItem("Monitor &Next EQ Client Seen", this, SLOT(set_net_monitor_next_client()));
   m_netMenu->insertItem("Monitor EQ Client &IP Address...", this, SLOT(set_net_client_IP_address()));
   m_netMenu->insertItem("Monitor EQ Client &MAC Address...", this, SLOT(set_net_client_MAC_address()));
   m_netMenu->insertItem("Set &Device...", this, SLOT(set_net_device()));
   m_id_net_sessiontrack = m_netMenu->insertItem("Session Tracking", this, SLOT(toggle_net_session_tracking()));
   m_netMenu->setItemChecked(m_id_net_sessiontrack, m_packet->session_tracking());
   x = m_netMenu->insertItem("&Real Time Thread", this, SLOT(toggle_net_real_time_thread(int)));
   m_netMenu->setItemChecked(x, m_packet->realtime());

   m_netMenu->insertSeparator(-1);

   // Log menu
   QPopupMenu* pLogMenu = new QPopupMenu;
   m_netMenu->insertItem("Lo&g", pLogMenu);
   pLogMenu->setCheckable(true);
   m_id_log_AllPackets  = pLogMenu->insertItem("All Packets", this, SLOT(toggle_log_AllPackets()), Key_F5);
   m_id_log_WorldData   = pLogMenu->insertItem("World Data", this, SLOT(toggle_log_WorldData()), Key_F6);
   m_id_log_ZoneData    = pLogMenu->insertItem("Zone Data", this, SLOT(toggle_log_ZoneData()), Key_F7);
   m_id_log_UnknownData = pLogMenu->insertItem("Unknown Zone Data", this, SLOT(toggle_log_UnknownData()), Key_F8);
   m_id_view_UnknownData = pLogMenu->insertItem("View Unknown Data", this, 
						 SLOT(toggle_view_UnknownData()) , 
						 Key_F9);
   m_id_log_RawData     = pLogMenu->insertItem("Raw Data", this, SLOT(toggle_log_RawData()), Key_F10);
   if (m_itemDB)			      
   {
     m_id_log_Items  = pLogMenu->insertItem("Item Data", this, SLOT(toggle_log_ItemData()));
     m_id_log_ItemPackets   = pLogMenu->insertItem("Item Packet Data", this, SLOT(toggle_log_ItemPacketData()));
   }
   menuBar()->setItemChecked (m_id_log_AllPackets, (m_globalLog != 0));
   menuBar()->setItemChecked (m_id_log_WorldData, (m_worldLog != 0));
   menuBar()->setItemChecked (m_id_log_ZoneData, (m_zoneLog != 0));
   menuBar()->setItemChecked (m_id_log_UnknownData, (m_unknownZoneLog != 0));
   menuBar()->setItemChecked(m_id_view_UnknownData,
			     pSEQPrefs->getPrefBool("ViewUnknown", 
						    "PacketLogging", 
						    false));
   menuBar()->setItemChecked (m_id_log_RawData,
			      pSEQPrefs->getPrefBool("LogRawPackets", 
						     "PacketLogging",
						     false));

   if (m_itemDB)			      
   {
     menuBar()->setItemChecked (m_id_log_Items, m_itemDB->GetItemLogging());
     menuBar()->setItemChecked (m_id_log_ItemPackets, m_itemDB->GetItemPacketLogging());
   }

   // OpCode Monitor
   QPopupMenu* pOpCodeMenu = new QPopupMenu;
   m_netMenu->insertItem("OpCode Monitor", pOpCodeMenu);
   x = pOpCodeMenu->insertItem("&OpCode Monitoring", this,
			      SLOT(toggle_opcode_monitoring(int)), CTRL+ALT+Key_O);
   
   pOpCodeMenu->setItemChecked(x, (m_opcodeMonitorLog != 0));
   pOpCodeMenu->insertItem("&Reload Monitored OpCode List..", this,
			  SLOT(set_opcode_monitored_list()), CTRL+ALT+Key_R);
  section = "OpCodeMonitoring";
   x =pOpCodeMenu->insertItem("&View Monitored OpCode Matches", this,
			      SLOT(toggle_opcode_view(int)));
   pOpCodeMenu->setItemChecked(x, pSEQPrefs->getPrefBool("View", 
							 section,
							 false));
   x =pOpCodeMenu->insertItem("&Log Monitored OpCode Matches", this,
			      SLOT(toggle_opcode_log(int)));
   pOpCodeMenu->setItemChecked(x, pSEQPrefs->getPrefBool("Log", 
							 section,
							 false));
   pOpCodeMenu->insertItem("Log &Filename...", this,
			  SLOT(select_opcode_file()));
   m_netMenu->insertSeparator(-1);

   section = "Interface";

   // Advanced menu
   subMenu = new QPopupMenu;
   QPopupMenu* subSubMenu = new QPopupMenu;
   QSpinBox* arqSeqGiveUpSpinBox = new QSpinBox(32, 1024, 8, subSubMenu);
   arqSeqGiveUpSpinBox->setValue(m_packet->arqSeqGiveUp());
   connect(arqSeqGiveUpSpinBox, SIGNAL(valueChanged(int)),
	   this, SLOT(set_net_arq_giveup(int)));
   subSubMenu->insertItem(arqSeqGiveUpSpinBox);
   subMenu->insertItem("Arq Seq Give Up", 
			 subSubMenu);
   m_netMenu->insertItem("Advanced", subMenu);

   // Character Menu 
   m_charMenu = new QPopupMenu;
   menuBar()->insertItem("&Character", m_charMenu);
   int yx = m_charMenu->insertItem("Use Auto Detected Settings", this, 
				   SLOT(toggleAutoDetectPlayerSettings(int)));
   m_charMenu->setItemChecked(yx, m_player->useAutoDetectedSettings());

   // Character -> Level
   m_charLevelMenu = new QPopupMenu;
   m_charMenu->insertItem("Choose &Level", m_charLevelMenu);
   m_levelSpinBox = new QSpinBox(1, 70, 1, this, "m_levelSpinBox");
   m_charLevelMenu->insertItem( m_levelSpinBox );
   m_levelSpinBox->setWrapping( true );
   m_levelSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
   m_levelSpinBox->setPrefix("Level: ");
   connect(m_levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetDefaultCharacterLevel(int)));
   m_levelSpinBox->setValue(m_player->defaultLevel());

   // Character -> Class
   m_charClassMenu = new QPopupMenu;
   m_charMenu->insertItem("Choose &Class", m_charClassMenu);
   for( int i = 0; i < PLAYER_CLASSES; i++)
   {
       char_ClassID[i] = m_charClassMenu->insertItem(player_classes[i]);
       m_charClassMenu->setItemParameter(char_ClassID[i],i+1);
       if(i+1 == m_player->defaultClass())
          m_charMenu->setItemChecked(char_ClassID[i], true);
   }
   connect (m_charClassMenu, SIGNAL(activated(int)), this, SLOT(SetDefaultCharacterClass(int)));

   // Character -> Race
   m_charRaceMenu = new QPopupMenu;
   m_charMenu->insertItem("Choose &Race", m_charRaceMenu);
   for( int i = 0; i < PLAYER_RACES; i++)
   {
       char_RaceID[i] = m_charRaceMenu->insertItem(player_races[i]);
       if(i != 12 || i != 13)
          m_charRaceMenu->setItemParameter(char_RaceID[i],i+1);
       if(i == 12)
          m_charRaceMenu->setItemParameter(char_RaceID[i],128);
       else if(i == 13)
          m_charRaceMenu->setItemParameter(char_RaceID[i],130);

       if(m_charRaceMenu->itemParameter(char_RaceID[i]) == m_player->defaultRace())
          m_charRaceMenu->setItemChecked(char_RaceID[i], true);
   }
   connect (m_charRaceMenu, SIGNAL(activated(int)), this, SLOT(SetDefaultCharacterRace(int)));

   // Filters Menu
   QPopupMenu* filterMenu = new QPopupMenu;
   menuBar()->insertItem( "Fi&lters" , filterMenu);
   filterMenu->setCheckable(true);

   filterMenu->insertItem("&Reload Filters", m_filterMgr, SLOT(loadFilters()), Key_F3);
   filterMenu->insertItem("&Save Filters", m_filterMgr, SLOT(saveFilters()), Key_F4);
   if (!shortZoneName.isEmpty())
      filterMenu->insertItem("Save &Zone Filter", this, SLOT(save_as_filter_file()));
   filterMenu->insertItem("&Edit Filters", this, SLOT(launch_editor_filters()));
   filterMenu->insertItem("Select Fil&ter File", this, SLOT(select_filter_file()));

   filterMenu->insertItem("Reload &Zone Filters", m_filterMgr, SLOT(loadZoneFilters()), SHIFT+Key_F3);
   filterMenu->insertItem("S&ave Zone Filters", m_filterMgr, SLOT(saveZoneFilters()), SHIFT+Key_F4);
   filterMenu->insertItem("Edit Zone Fi&lters", this, SLOT(launch_editor_zoneFilters()));

   filterMenu->insertItem("Re&filter Spawns", m_spawnShell, SLOT(refilterSpawns()));
   x = filterMenu->insertItem("&Is Case Sensitive", this, SLOT(toggle_filter_Case(int)));
   filterMenu->setItemChecked(x, m_filterMgr->caseSensitive());
   x = filterMenu->insertItem("&Display Alert Info", this, SLOT(toggle_filter_AlertInfo(int)));
   filterMenu->setItemChecked(x, 
			      pSEQPrefs->getPrefBool("AlertInfo", "Filters"));
   x = filterMenu->insertItem("&Use System Beep", this, SLOT(toggle_filter_UseSystemBeep(int)));
   filterMenu->setItemChecked(x, m_filterNotifications->useSystemBeep());
   x = filterMenu->insertItem("Use &Commands", this, SLOT(toggle_filter_UseCommands(int)));
   filterMenu->setItemChecked(x, m_filterNotifications->useCommands());

   // Filter -> Log
   QPopupMenu* filterLogMenu = new QPopupMenu;
   filterLogMenu->setCheckable(true);
   filterMenu->insertItem("&Log", filterLogMenu);
   x = filterLogMenu->insertItem( "Alerts");
   filterLogMenu->setItemParameter(x, FILTER_FLAG_ALERT);
   filterLogMenu->setItemChecked(x, ((filters & FILTER_FLAG_ALERT) != 0));
   x = filterLogMenu->insertItem( "Locates");
   filterLogMenu->setItemParameter(x, FILTER_FLAG_LOCATE);
   filterLogMenu->setItemChecked(x, ((filters & FILTER_FLAG_LOCATE) != 0));
   x = filterLogMenu->insertItem( "Hunts");
   filterLogMenu->setItemParameter(x, FILTER_FLAG_HUNT);
   filterLogMenu->setItemChecked(x, ((filters & FILTER_FLAG_HUNT) != 0));
   x = filterLogMenu->insertItem( "Cautions");
   filterLogMenu->setItemParameter(x, FILTER_FLAG_CAUTION);
   filterLogMenu->setItemChecked(x, ((filters & FILTER_FLAG_CAUTION) != 0));
   x = filterLogMenu->insertItem( "Dangers");
   filterLogMenu->setItemParameter(x, FILTER_FLAG_DANGER);
   filterLogMenu->setItemChecked(x, ((filters & FILTER_FLAG_DANGER) != 0));
   connect(filterLogMenu, SIGNAL(activated(int)),
	   this, SLOT(toggle_filter_Log(int)));

   // Filter -> Commands
   QPopupMenu* filterCmdMenu = new QPopupMenu;
   filterMenu->insertItem("&Audio Commands", filterCmdMenu);
   x = filterCmdMenu->insertItem( "Spawn...");
   filterCmdMenu->setItemParameter(x, 1);
   x = filterCmdMenu->insertItem( "DeSpawn...");
   filterCmdMenu->setItemParameter(x, 2);
   x = filterCmdMenu->insertItem( "Death...");
   filterCmdMenu->setItemParameter(x, 3);
   x = filterCmdMenu->insertItem( "Locate...");
   filterCmdMenu->setItemParameter(x, 4);
   x = filterCmdMenu->insertItem( "Caution...");
   filterCmdMenu->setItemParameter(x, 5);
   x = filterCmdMenu->insertItem( "Hunt...");
   filterCmdMenu->setItemParameter(x, 6);
   x = filterCmdMenu->insertItem( "Danger...");
   filterCmdMenu->setItemParameter(x, 7);
   connect(filterCmdMenu, SIGNAL(activated(int)),
	   this, SLOT(set_filter_AudioCommand(int)));

   // Interface Menu
   QPopupMenu* pInterfaceMenu = new QPopupMenu;
   menuBar()->insertItem( "&Interface" , pInterfaceMenu);

   pInterfaceMenu->insertItem("Hide MenuBar", this, SLOT(toggle_view_menubar()));

   // Interface -> Style
   //pStyleMenu = new QPopupMenu;
   QPopupMenu* pStyleMenu = new QPopupMenu;
   pInterfaceMenu->insertItem( "&Style", pStyleMenu);
   pStyleMenu->setCheckable(TRUE);
   x = pStyleMenu->insertItem( "Platinum (Macintosh)");
   pStyleMenu->setItemParameter(x, 1);
   IDList_StyleMenu.append(x);
   x = pStyleMenu->insertItem( "Windows (Default)");
   pStyleMenu->setItemParameter(x, 2);
   IDList_StyleMenu.append(x);
   x = pStyleMenu->insertItem( "CDE");
   pStyleMenu->setItemParameter(x, 3);
   IDList_StyleMenu.append(x);
   x = pStyleMenu->insertItem( "CDE Polished");
   pStyleMenu->setItemParameter(x, 4);
   IDList_StyleMenu.append(x);
   x = pStyleMenu->insertItem( "Motif");
   pStyleMenu->setItemParameter(x, 5);
   IDList_StyleMenu.append(x);
   x = pStyleMenu->insertItem( "SGI");
   pStyleMenu->setItemParameter(x, 6);
   IDList_StyleMenu.append(x);
   connect (pStyleMenu, SIGNAL(activated(int)), this, SLOT(selectTheme(int)));

   setTheme(pSEQPrefs->getPrefInt("Theme", section, 2));

   // Interface -> Status Bar
   QPopupMenu* statusBarMenu = new QPopupMenu;
   statusBarMenu->setCheckable(true);
   pInterfaceMenu->insertItem("&Status Bar", statusBarMenu);
   statusBarMenu->insertItem("Show/Hide", 
			      this, SLOT(toggle_view_statusbar()));
   x = statusBarMenu->insertItem( "Status");
   statusBarMenu->setItemParameter(x, 1);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowStatus",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "Zone");
   statusBarMenu->setItemParameter(x, 2);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowZone",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "Spawns");
   statusBarMenu->setItemParameter(x, 3);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowSpawns",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "Experience");
   statusBarMenu->setItemParameter(x, 4);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowExp",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "AA Experience");
   statusBarMenu->setItemParameter(x, 5);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowExpAA",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "Packet Counter");
   statusBarMenu->setItemParameter(x, 6);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowPacketCounter",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "EQ Time");
   statusBarMenu->setItemParameter(x, 7);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowEQTime",
							   "Interface_StatusBar", false));
   x = statusBarMenu->insertItem( "Run Speed");
   statusBarMenu->setItemParameter(x, 8);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowSpeed",
							   "Interface_StatusBar", false));
   // ZEM code
   x = statusBarMenu->insertItem( "ZEM");
   statusBarMenu->setItemParameter(x, 9);
   statusBarMenu->setItemChecked(x, pSEQPrefs->getPrefBool("ShowZEM",
							   "Interface_StatusBar", false));
    
   connect (statusBarMenu, SIGNAL(activated(int)), 
	    this, SLOT(toggle_main_statusbar_Window(int)));

   m_terminalMenu = new QPopupMenu;
   pInterfaceMenu->insertItem("&Terminal", m_terminalMenu);

   m_terminalTypeFilterMenu = new QPopupMenu;
   m_terminalMenu->insertItem("MessageTypeFilter", m_terminalTypeFilterMenu);
   m_terminalTypeFilterMenu->insertItem("&Enable All", 
					this, SLOT(enableAllTypeFilters()), 0, 64);
   m_terminalTypeFilterMenu->insertItem("&Disable All", 
					this, SLOT(disableAllTypeFilters()), 0, 65);
  
   m_terminalTypeFilterMenu->insertSeparator(-1);

   QString typeName;
   uint64_t enabledTypes = m_terminal->enabledTypes();

   // iterate over the message types, filling in various menus and getting 
   // font color preferences
   for (int i = MT_Guild; i <= MT_Max; i++)
   {
     typeName = MessageEntry::messageTypeString((MessageType)i);
     if (!typeName.isEmpty())
     {
       m_terminalTypeFilterMenu->insertItem(typeName, i);
       m_terminalTypeFilterMenu->setItemChecked(i, (((uint64_t(1) << i) & enabledTypes) != 0));
     }
   }

   connect(m_terminalTypeFilterMenu, SIGNAL(activated(int)),
	   this, SLOT(toggleTypeFilter(int)));

  m_terminalShowUserFilterMenu = new QPopupMenu;
  m_terminalMenu->insertItem("User Message Filter - &Show", m_terminalShowUserFilterMenu);

  m_terminalShowUserFilterMenu->insertItem("&Enable All", 
			       this, SLOT(enableAllShowUserFilters()), 0, 66);
  m_terminalShowUserFilterMenu->insertItem("&Disable All", 
			       this, SLOT(disableAllShowUserFilters()), 0, 67);
  m_terminalShowUserFilterMenu->insertSeparator(-1);

  m_terminalHideUserFilterMenu = new QPopupMenu;
  m_terminalMenu->insertItem("User Message Filter - &Hide", m_terminalHideUserFilterMenu);

  m_terminalHideUserFilterMenu->insertItem("&Enable All", 
			       this, SLOT(enableAllHideUserFilters()), 0, 66);
  m_terminalHideUserFilterMenu->insertItem("&Disable All", 
			       this, SLOT(disableAllHideUserFilters()), 0, 67);
  m_terminalHideUserFilterMenu->insertSeparator(-1);

  uint32_t enabledShowUserFilters = m_terminal->enabledShowUserFilters();
  uint32_t enabledHideUserFilters = m_terminal->enabledHideUserFilters();
  const MessageFilter* filter;
  for(int i = 0; i < maxMessageFilters; i++)
  {
    filter = m_messageFilters->filter(i);
    if (filter)
    {
      m_terminalShowUserFilterMenu->insertItem(filter->name(), i);
      m_terminalShowUserFilterMenu->setItemChecked(i, (1 << i) & enabledShowUserFilters);

      m_terminalHideUserFilterMenu->insertItem(filter->name(), i);
      m_terminalHideUserFilterMenu->setItemChecked(i, (1 << i) & enabledHideUserFilters);
    }
  }

  connect(m_terminalShowUserFilterMenu, SIGNAL(activated(int)),
	  this, SLOT(toggleShowUserFilter(int)));
  connect(m_terminalHideUserFilterMenu, SIGNAL(activated(int)),
	  this, SLOT(toggleHideUserFilter(int)));
  m_terminalMenu->insertItem("Edit User &Message Filters...", 
			     this, SLOT(showMessageFilterDialog()));

   m_terminalMenu->insertSeparator(-1);

   x = m_terminalMenu->insertItem("&Display Type", this, SLOT(toggleDisplayType(int)));
   m_terminalMenu->setItemChecked(x, m_terminal->displayType());
   x = m_terminalMenu->insertItem("Display T&ime/Date", this,
				  SLOT(toggleDisplayTime(int)));
   m_terminalMenu->setItemChecked(x, m_terminal->displayDateTime());
   x = m_terminalMenu->insertItem("Display &EQ Date/Time", this,
				  SLOT(toggleEQDisplayTime(int)));
   m_terminalMenu->setItemChecked(x, m_terminal->displayEQDateTime());
   x = m_terminalMenu->insertItem("&Use Color", this,
				  SLOT(toggleUseColor(int)));
   m_terminalMenu->setItemChecked(x, m_terminal->useColor());


   pInterfaceMenu->insertItem( "Formatted Messages File...", 
			       this, SLOT(select_main_FormatFile(int)));
   pInterfaceMenu->insertItem( "Spells File...", 
			       this, SLOT(select_main_SpellsFile(int)));

   // insert Window menu
   menuBar()->insertItem("&Window", m_windowMenu);

   // All of the Window menu items that don't automatically get inserted
   // have to be manually placed in the right positions.

   // Interface -> WindowCaption
   m_windowCaptionMenu = new QPopupMenu;
   m_windowMenu->insertItem( "Window &Caption", m_windowCaptionMenu, -1, 0);
   
   x = m_windowCaptionMenu->insertItem("&Main Window...");
   m_windowCaptionMenu->setItemParameter(x, 5);
    
   x = m_windowCaptionMenu->insertItem("Spawn &List...");
   m_windowCaptionMenu->setItemParameter(x, 0);
    
   x = m_windowCaptionMenu->insertItem("Spawn List &2...");
   m_windowCaptionMenu->setItemParameter(x, 10);
    
   x = m_windowCaptionMenu->insertItem("Spawn P&oint List...");
   m_windowCaptionMenu->setItemParameter(x, 9);
    
   x = m_windowCaptionMenu->insertItem("&Player Stats...");
   m_windowCaptionMenu->setItemParameter(x, 1);
    
   x = m_windowCaptionMenu->insertItem("Player &Skills...");
   m_windowCaptionMenu->setItemParameter(x, 2);
    
   x = m_windowCaptionMenu->insertItem("Spell L&ist...");
   m_windowCaptionMenu->setItemParameter(x, 3);
    
   x = m_windowCaptionMenu->insertItem("&Compass...");
   m_windowCaptionMenu->setItemParameter(x, 4);
    
   x = m_windowCaptionMenu->insertItem("&Experience Window...");
   m_windowCaptionMenu->setItemParameter(x, 6);
    
   x = m_windowCaptionMenu->insertItem("Comb&at Window...");
   m_windowCaptionMenu->setItemParameter(x, 7);
    
   x = m_windowCaptionMenu->insertItem("&Network Diagnostics...");
   m_windowCaptionMenu->setItemParameter(x, 8);

   // insert Map docking options 
   // NOTE: Always insert Map docking options at the end of the Docked menu
   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map";
        if (i > 0)
            mapName += QString::number(i + 1);
        x = m_windowCaptionMenu->insertItem(mapName);
        m_windowCaptionMenu->setItemParameter(x, i + mapCaptionBase);
   }
    
   connect (m_windowCaptionMenu, SIGNAL(activated(int)), 
	    this, SLOT(set_main_WindowCaption(int)));

   // Interface -> Window Font
   QPopupMenu* windowFontMenu = new QPopupMenu;
   m_windowMenu->insertItem( "&Font", windowFontMenu, -1, 1);
    
   windowFontMenu->insertItem( "&Applicatoin Default...", 
			       this, SLOT(set_main_Font(int)));

   windowFontMenu->insertItem( "Main Window Status Font...", 
			      this, SLOT(set_main_statusbar_Font(int)));
   //   x = windowFontMenu->insertItem("&Main Window");
   //   windowFontMenu->setItemParameter(x, 5);
    
   x = windowFontMenu->insertItem("Spawn &List...");
   windowFontMenu->setItemParameter(x, 0);
    
   x = windowFontMenu->insertItem("Spawn List &2...");
   windowFontMenu->setItemParameter(x, 10);
    
   x = windowFontMenu->insertItem("Spawn P&oint List...");
   windowFontMenu->setItemParameter(x, 9);
    
   x = windowFontMenu->insertItem("&Player Stats...");
   windowFontMenu->setItemParameter(x, 1);
    
   x = windowFontMenu->insertItem("Player &Skills...");
   windowFontMenu->setItemParameter(x, 2);
    
   x = windowFontMenu->insertItem("Spell L&ist...");
   windowFontMenu->setItemParameter(x, 3);
    
   x = windowFontMenu->insertItem("&Compass...");
   windowFontMenu->setItemParameter(x, 4);
    
   x = windowFontMenu->insertItem("&Experience Window...");
   windowFontMenu->setItemParameter(x, 6);
    
   x = windowFontMenu->insertItem("Comb&at Window...");
   windowFontMenu->setItemParameter(x, 7);
    
   x = windowFontMenu->insertItem("&Network Diagnostics...");
   windowFontMenu->setItemParameter(x, 8);

   // insert Map docking options 
   // NOTE: Always insert Map docking options at the end of the Docked menu
   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map";
        if (i > 0)
            mapName += QString::number(i + 1);
        x = m_windowCaptionMenu->insertItem(mapName);
        m_windowCaptionMenu->setItemParameter(x, i + mapCaptionBase);
   }

   connect (windowFontMenu, SIGNAL(activated(int)), 
	    this, SLOT(set_main_WindowFont(int)));


   x = m_windowMenu->insertItem("Save Window Sizes && Positions", 
				  this, SLOT(toggle_main_SavePosition(int)),
				  0, -1, 2);
   m_windowMenu->setItemChecked (x, pSEQPrefs->getPrefBool("SavePosition", 
							"Interface",
							true));
   x = m_windowMenu->insertItem("Restore Window Positions", 
				  this, SLOT(toggle_main_UseWindowPos(int)),
				  0, -1, 3);
   m_windowMenu->setItemChecked (x, pSEQPrefs->getPrefBool("UseWindowPos", 
							"Interface",
							true));

   m_windowMenu->insertSeparator(4);

   // Debug menu
   QPopupMenu* pDebugMenu = new QPopupMenu;
   menuBar()->insertItem("&Debug", pDebugMenu);
   pDebugMenu->insertItem("List I&nterface", this, SLOT(listInterfaceInfo()));
   pDebugMenu->insertItem("List S&pawns", this, SLOT(listSpawns()), ALT+CTRL+Key_P);
   pDebugMenu->insertItem("List &Drops", this, SLOT(listDrops()), ALT+CTRL+Key_D);
   pDebugMenu->insertItem("List &Map Info", this, SLOT(listMapInfo()), ALT+CTRL+Key_M);
   pDebugMenu->insertItem("List G&uild Info", m_guildmgr, SLOT(listGuildInfo()));
   pDebugMenu->insertItem("List &Group", this, SLOT(listGroup()), ALT+CTRL+Key_G);
   pDebugMenu->insertItem("List Guild M&embers", this, SLOT(listGuild()), ALT+CTRL+Key_E);
   pDebugMenu->insertItem("Dump Spawns", this, SLOT(dumpSpawns()), ALT+SHIFT+CTRL+Key_P);
   pDebugMenu->insertItem("Dump Drops", this, SLOT(dumpDrops()), ALT+SHIFT+CTRL+Key_D);
   pDebugMenu->insertItem("Dump Map Info", this, SLOT(dumpMapInfo()), ALT+SHIFT+CTRL+Key_M);
   pDebugMenu->insertItem("Dump Guild Info", this , SLOT(dumpGuildInfo()));
   pDebugMenu->insertItem("Dump SpellBook Info", this , SLOT(dumpSpellBook()));
   pDebugMenu->insertItem("Dump Group", this, SLOT(dumpGroup()), ALT+CTRL+SHIFT+Key_G);
   pDebugMenu->insertItem("Dump Guild Members", this, SLOT(dumpGuild()), ALT+CTRL+SHIFT+Key_E);
   pDebugMenu->insertItem("List &Filters", m_filterMgr, SLOT(listFilters()), ALT+CTRL+Key_F);
   pDebugMenu->insertItem("List &Zone Filters", m_filterMgr, SLOT(listZoneFilters()));

////////////////////
// QStatusBar creation
   
   QString statusBarSection = "Interface_StatusBar";
   int sts_widget_count = 0; // total number of widgets visible on status bar

   //Status widget
     m_stsbarStatus = new QLabel(statusBar(), "Status");
     m_stsbarStatus->setMinimumWidth(80);
     m_stsbarStatus->setText(QString("ShowEQ %1").arg(VERSION));
     statusBar()->addWidget(m_stsbarStatus, 8);

   //Zone widget
     m_stsbarZone = new QLabel(statusBar(), "Zone");
     m_stsbarZone->setText("Zone: [unknown]");
     statusBar()->addWidget(m_stsbarZone, 2);

   //Mobs widget
     m_stsbarSpawns = new QLabel(statusBar(), "Mobs");
     m_stsbarSpawns->setText("Mobs:");
     statusBar()->addWidget(m_stsbarSpawns, 1);

   //Exp widget
     m_stsbarExp = new QLabel(statusBar(), "Exp");
     m_stsbarExp->setText("Exp [unknown]");
     statusBar()->addWidget(m_stsbarExp, 2);

   //ExpAA widget
     m_stsbarExpAA = new QLabel(statusBar(), "ExpAA");
     m_stsbarExpAA->setText("ExpAA [unknown]");
     statusBar()->addWidget(m_stsbarExpAA, 2);
   
   //Pkt widget
     m_stsbarPkt = new QLabel(statusBar(), "Pkt");
     m_stsbarPkt->setText("Pkt 0");
     statusBar()->addWidget(m_stsbarPkt, 1);

   //EQTime widget
     m_stsbarEQTime = new QLabel(statusBar(), "EQTime");
     m_stsbarEQTime->setText("EQTime");
     statusBar()->addWidget(m_stsbarEQTime, 1);

   // Run Speed widget
     m_stsbarSpeed = new QLabel(statusBar(), "Speed");
     m_stsbarSpeed->setText("Run Speed:");
     statusBar()->addWidget(m_stsbarSpeed, 1);

   // ZEM code
   // Zone Exp Mult widget
     m_stsbarZEM = new QLabel(statusBar(), "ZEM");
     m_stsbarZEM->setText("ZEM: [unknown]");
     statusBar()->addWidget(m_stsbarZEM, 1);

     // setup the status fonts correctly
     restoreStatusFont();

   if (!pSEQPrefs->getPrefBool("ShowStatus", statusBarSection, true))
     m_stsbarStatus->hide();
   else
     sts_widget_count++;
  
   if (!pSEQPrefs->getPrefBool("ShowZone", statusBarSection, true))
     m_stsbarZone->hide();
   else
     sts_widget_count++;

   if (!pSEQPrefs->getPrefBool("ShowSpawns", statusBarSection, false))
     m_stsbarSpawns->hide();
   else
     sts_widget_count++;

   if (!pSEQPrefs->getPrefBool("ShowExp", statusBarSection, false))
     m_stsbarExp->hide();
   else
     sts_widget_count++;

   if (!pSEQPrefs->getPrefBool("ShowExpAA", statusBarSection, false))
     m_stsbarExpAA->hide();
   else
     sts_widget_count++;

   if (!pSEQPrefs->getPrefBool("ShowPacketCounter", statusBarSection, false))
     m_stsbarPkt->hide();
   else
     sts_widget_count++;

   if (!pSEQPrefs->getPrefBool("ShowEQTime", statusBarSection, true))
     m_stsbarEQTime->hide();
   else
     sts_widget_count++;

   if (!pSEQPrefs->getPrefBool("ShowSpeed", statusBarSection, false))
     m_stsbarSpeed->hide();
   else
     sts_widget_count++;

   // ZEM code
   if (!pSEQPrefs->getPrefBool("ShowZEM", statusBarSection, false))
     m_stsbarZEM->hide();
   else
     sts_widget_count++;


   //hide the statusbar if no visible widgets
   if (!sts_widget_count || !pSEQPrefs->getPrefBool("StatusBarActive", statusBarSection, 1))
      statusBar()->hide();


/////////////////
// interface connections
   // connect EQInterface slots to its own signals
   connect(this, SIGNAL(restoreFonts(void)),
	   this, SLOT(restoreStatusFont(void)));

   // connect MapMgr slots to interface signals
   connect(this, SIGNAL(saveAllPrefs(void)),
	   m_mapMgr, SLOT(savePrefs(void)));

   // connect CategoryMgr slots to interface signals
   connect(this, SIGNAL(saveAllPrefs(void)),
	   m_categoryMgr, SLOT(savePrefs(void)));

   if (m_itemDB != 0)
   {
     // connect ItemDB slots to EQPacket signals
     m_packet->connect2("OP_ItemPacket", SP_Zone, DIR_Server,
			"itemPacketStruct", SZC_None,
			m_itemDB, SLOT(item(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_ItemPlayerPacket", SP_Zone, DIR_Server,
			"char", SZC_None, m_itemDB, 
			SLOT(playerItem(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_ItemLinkResponse", SP_Zone, DIR_Server,
			"itemInfoStruct", SZC_None, m_itemDB, 
			SLOT(itemInfo(const uint8_t*, size_t, uint8_t)));
   }

   if (m_groupMgr != 0)
   {
     m_packet->connect2("OP_PlayerProfile", SP_Zone, DIR_Server,
			"charProfileStruct", SZC_Match,
			m_groupMgr, SLOT(player(const uint8_t*)));
     m_packet->connect2("OP_GroupUpdate", SP_Zone, DIR_Server,
			"groupUpdateStruct", SZC_Match,
			m_groupMgr, SLOT(groupUpdate(const uint8_t*, size_t)));
     m_packet->connect2("OP_GroupUpdate", SP_Zone, DIR_Server,
			"groupFullUpdateStruct", SZC_Match,
			m_groupMgr, SLOT(groupUpdate(const uint8_t*, size_t)));
     // connect GroupMgr slots to SpawnShell signals
     connect(m_spawnShell, SIGNAL(addItem(const Item*)),
	     m_groupMgr, SLOT(addItem(const Item*)));
     // connect GroupMgr slots to SpawnShell signals
     connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	     m_groupMgr, SLOT(delItem(const Item*)));
     // connect GroupMgr slots to SpawnShell signals
     connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	     m_groupMgr, SLOT(killSpawn(const Item*)));
   }

   if (m_dateTimeMgr)
   {
     // connect DateTimeMgr slots to EQPacket signals
     m_packet->connect2("OP_TimeOfDay", SP_Zone, DIR_Server,
			"timeOfDayStruct", SZC_Match, 
			m_dateTimeMgr, SLOT(timeOfDay(const uint8_t*)));

     // connect interface slots to DateTimeMgr signals
     connect(m_dateTimeMgr, SIGNAL(updatedDateTime(const QDateTime&)),
	     this, SLOT(updatedDateTime(const QDateTime&)));

     connect(m_dateTimeMgr, SIGNAL(syncDateTime(const QDateTime&)),
	     this, SLOT(syncDateTime(const QDateTime&)));
   }

   if (m_zoneMgr)
   {
     m_packet->connect2("OP_ZoneEntry", SP_Zone, DIR_Client,
			"ClientZoneEntryStruct", SZC_Match,
			m_zoneMgr, SLOT(zoneEntryClient(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_ZoneEntry", SP_Zone, DIR_Server,
			"ServerZoneEntryStruct", SZC_Match,
			m_zoneMgr, SLOT(zoneEntryServer(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_PlayerProfile", SP_Zone, DIR_Server,
			"charProfileStruct", SZC_Match,
			m_zoneMgr, SLOT(zonePlayer(const uint8_t*)));
     m_packet->connect2("OP_ZoneChange", SP_Zone, DIR_Client|DIR_Server,
			"zoneChangeStruct", SZC_Match,
			m_zoneMgr, SLOT(zoneChange(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_NewZone", SP_Zone, DIR_Server,
			"newZoneStruct", SZC_Match,
			m_zoneMgr, SLOT(zoneNew(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_SendZonePoints", SP_Zone, DIR_Server,
			"zonePointsStruct", SZC_None,
			m_zoneMgr, SLOT(zonePoints(const uint8_t*, size_t, uint8_t)));
   }

   if (m_filterMgr)
   {
     connect(m_zoneMgr, SIGNAL(zoneBegin(const QString&)),
	     m_filterMgr, SLOT(loadZone(const QString&)));
     connect(m_zoneMgr, SIGNAL(zoneEnd(const QString&, const QString&)),
	     m_filterMgr, SLOT(loadZone(const QString&)));
     connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
	     m_filterMgr, SLOT(loadZone(const QString&)));
   }

   if (m_guildmgr)
   {
     m_packet->connect2("OP_GuildList", SP_World, DIR_Server, 
			"worldGuildListStruct", SZC_Match,
			m_guildmgr, 
			SLOT(worldGuildList(const uint8_t*, size_t)));

     connect(this, SIGNAL(guildList2text(QString)),
	     m_guildmgr, SLOT(guildList2text(QString)));
   }

   if (m_guildShell)
   {
     m_packet->connect2("OP_GuildMemberList", SP_Zone, DIR_Server,
			"uint8_t", SZC_None,
			m_guildShell,
			SLOT(guildMemberList(const uint8_t*, size_t)));
     m_packet->connect2("OP_GuildMemberUpdate", SP_Zone, DIR_Server,
			"GuildMemberUpdate", SZC_Match,
			m_guildShell,
			SLOT(guildMemberUpdate(const uint8_t*, size_t)));
   }

   if (m_messageShell)
   {
     m_packet->connect2("OP_CommonMessage", SP_Zone, DIR_Client|DIR_Server,
			"channelMessageStruct", SZC_None,
			m_messageShell,
			SLOT(channelMessage(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_FormattedMessage", SP_Zone, DIR_Server,
			"formattedMessageStruct", SZC_None,
			m_messageShell,
			SLOT(formattedMessage(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_SimpleMessage", SP_Zone, DIR_Server,
			"simpleMessageStruct", SZC_None,
			m_messageShell,
			SLOT(simpleMessage(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_SpecialMesg", SP_Zone, DIR_Server,
			"specialMessageStruct", SZC_None,
			m_messageShell,
			SLOT(specialMessage(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_GuildMOTD", SP_Zone, DIR_Server,
			"guildMOTDStruct", SZC_None,
			m_messageShell,
			SLOT(guildMOTD(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_RandomReq", SP_Zone, DIR_Client,
			"randomReqStruct", SZC_Match,
			m_messageShell, SLOT(randomRequest(const uint8_t*)));
     m_packet->connect2("OP_RandomReply", SP_Zone, DIR_Server,
			"randomStruct", SZC_Match,
			m_messageShell, SLOT(random(const uint8_t*)));
     m_packet->connect2("OP_Emote", SP_Zone, DIR_Server|DIR_Client,
			"emoteTextStruct", SZC_None,
			m_messageShell, SLOT(emoteText(const uint8_t*)));
     m_packet->connect2("OP_InspectAnswer", SP_Zone, DIR_Server,
			"inspectDataStruct", SZC_Match,
			m_messageShell, SLOT(inspectData(const uint8_t*)));
     m_packet->connect2("OP_MoneyOnCorpse", SP_Zone, DIR_Server,
			"moneyOnCorpseStruct", SZC_Match,
			m_messageShell, SLOT(moneyOnCorpse(const uint8_t*)));
     m_packet->connect2("OP_Logout", SP_Zone, DIR_Server,
			"none", SZC_Match,
			m_messageShell, SLOT(logOut(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_NewZone", SP_Zone, DIR_Server,
			"newZoneStruct", SZC_Match,
			m_messageShell, SLOT(zoneNew(const uint8_t*, size_t, uint8_t)));
     connect(m_zoneMgr, SIGNAL(zoneBegin(const ClientZoneEntryStruct*, size_t, uint8_t)),
	     m_messageShell, SLOT(zoneEntryClient(const ClientZoneEntryStruct*)));
     connect(m_zoneMgr, SIGNAL(zoneBegin(const ServerZoneEntryStruct*, size_t, uint8_t)),
	     m_messageShell, SLOT(zoneEntryServer(const ServerZoneEntryStruct*)));
     connect(m_zoneMgr, SIGNAL(zoneChanged(const zoneChangeStruct*, size_t, uint8_t)),
	     m_messageShell, SLOT(zoneChanged(const zoneChangeStruct*, size_t, uint8_t)));
     connect(m_zoneMgr, SIGNAL(zoneBegin(const QString&)),
	     m_messageShell, SLOT(zoneBegin(const QString&)));
     connect(m_zoneMgr, SIGNAL(zoneEnd(const QString&, const QString&)),
	     m_messageShell, SLOT(zoneEnd(const QString&, const QString&)));
     connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
	     m_messageShell, SLOT(zoneChanged(const QString&)));

     m_packet->connect2("OP_MOTD", SP_World, DIR_Server,
			"worldMOTDStruct", SZC_None,
			m_messageShell, SLOT(worldMOTD(const uint8_t*)));
     m_packet->connect2("OP_MemorizeSpell", SP_Zone, DIR_Server|DIR_Client,
			"memSpellStruct", SZC_Match,
			m_messageShell, SLOT(handleSpell(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_BeginCast", SP_Zone, DIR_Server|DIR_Client,
			"beginCastStruct", SZC_Match,
			m_messageShell, SLOT(beginCast(const uint8_t*)));
     m_packet->connect2("OP_BuffFadeMsg", SP_Zone, DIR_Server|DIR_Client,
			"spellFadedStruct", SZC_None,
			m_messageShell, SLOT(spellFaded(const uint8_t*)));
     m_packet->connect2("OP_CastSpell", SP_Zone, DIR_Server|DIR_Client,
			"startCastStruct", SZC_Match,
			m_messageShell, SLOT(startCast(const uint8_t*)));
     m_packet->connect2("OP_PlayerProfile", SP_Zone, DIR_Server,
			"charProfileStruct", SZC_Match,
			m_messageShell, SLOT(player(const uint8_t*)));
     m_packet->connect2("OP_SkillUpdate", SP_Zone, DIR_Server,
			"skillIncStruct", SZC_Match,
			m_messageShell, SLOT(increaseSkill(const uint8_t*)));
     m_packet->connect2("OP_LevelUpdate", SP_Zone, DIR_Server,
			"levelUpUpdateStruct", SZC_Match,
			m_messageShell, SLOT(updateLevel(const uint8_t*)));

     m_packet->connect2("OP_Consider", SP_Zone, DIR_Server,
			"considerStruct", SZC_Match,
			m_messageShell, SLOT(consMessage(const uint8_t*, size_t, uint8_t)));

     connect(m_player, SIGNAL(setExp(uint32_t, uint32_t, uint32_t, uint32_t, 
				     uint32_t)),
	     m_messageShell, SLOT(setExp(uint32_t, uint32_t, uint32_t, 
					 uint32_t, uint32_t)));
     connect(m_player, SIGNAL(newExp(uint32_t, uint32_t, uint32_t, uint32_t, 
				     uint32_t, uint32_t)),
	     m_messageShell, SLOT(newExp(uint32_t, uint32_t, uint32_t,  
					 uint32_t, uint32_t, uint32_t)));
     connect(m_player, SIGNAL(setAltExp(uint32_t, uint32_t, uint32_t, uint32_t)),
	     m_messageShell, SLOT(setAltExp(uint32_t, uint32_t, uint32_t, uint32_t)));
     connect(m_player, SIGNAL(newAltExp(uint32_t, uint32_t, uint32_t, uint32_t,
					uint32_t, uint32_t)),
	     m_messageShell, SLOT(newAltExp(uint32_t, uint32_t, uint32_t, uint32_t,
					    uint32_t, uint32_t)));

     connect(m_spawnShell, SIGNAL(addItem(const Item*)),
	     m_messageShell, SLOT(addItem(const Item*)));
     connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	     m_messageShell, SLOT(delItem(const Item*)));
     connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	     m_messageShell, SLOT(killSpawn(const Item*)));

     connect(m_dateTimeMgr, SIGNAL(syncDateTime(const QDateTime&)),
	     m_messageShell, SLOT(syncDateTime(const QDateTime&)));

     m_packet->connect2("OP_GroupUpdate", SP_Zone, DIR_Server,
			"groupUpdateStruct", SZC_Match,
			m_messageShell, SLOT(groupUpdate(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_GroupUpdate", SP_Zone, DIR_Server,
			"groupFullUpdateStruct", SZC_Match,
			m_messageShell, SLOT(groupUpdate(const uint8_t*, size_t, uint8_t)));
     m_packet->connect2("OP_GroupInvite", SP_Zone, DIR_Server|DIR_Client,
			"groupInviteStruct", SZC_Match,
			m_messageShell, SLOT(groupInvite(const uint8_t*)));
     m_packet->connect2("OP_GroupInvite2", SP_Zone, DIR_Server|DIR_Client,
			"groupInviteStruct", SZC_Match,
			m_messageShell, SLOT(groupInvite(const uint8_t*)));
     m_packet->connect2("OP_GroupFollow", SP_Zone, DIR_Server|DIR_Client,
			"groupFollowStruct", SZC_Match,
			m_messageShell, SLOT(groupFollow(const uint8_t*)));
     m_packet->connect2("OP_GroupDisband", SP_Zone, DIR_Server|DIR_Client,
			"groupDisbandStruct", SZC_Match,
			m_messageShell, SLOT(groupDisband(const uint8_t*, size_t, uint8_t)));
#if 0 // ZBTEMP
     connect(m_packet, SIGNAL(groupDecline(const uint8_t*, size_t, uint8_t)),
	     m_messageShell, SLOT(groupDecline(const uint8_t*)));
#endif // ZBTEMP
   }

   if (m_filterNotifications)
   {
     connect(m_spawnShell, SIGNAL(addItem(const Item*)),
	     m_filterNotifications, SLOT(addItem(const Item*)));
     connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	     m_filterNotifications, SLOT(delItem(const Item*)));
     connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	     m_filterNotifications, SLOT(killSpawn(const Item*)));
     connect(m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
	     m_filterNotifications, 
	     SLOT(changeItem(const Item*, uint32_t)));
   }

   // connect interface slots to Packet signals
   m_packet->connect2("OP_TargetMouse", SP_Zone, DIR_Client|DIR_Server,
		      "clientTargetStruct", SZC_Match,
		      this, SLOT(clientTarget(const uint8_t*)));
#if 0 // ZBTEMP
   connect(m_packet, SIGNAL(attack2Hand1(const uint8_t*, size_t, uint8_t)), 
	   this, SLOT(attack2Hand1(const uint8_t*)));
#endif
   m_packet->connect2("OP_Action2", SP_Zone, DIR_Client|DIR_Server,
		      "action2Struct", SZC_Match,
		      this, SLOT(action2Message(const uint8_t*)));
   m_packet->connect2("OP_Death", SP_Zone, DIR_Server,
		      "newCorpseStruct", SZC_Match,
		      this, SLOT(combatKillSpawn(const uint8_t*)));
#if 0 // ZBTEMP
   connect(m_packet, SIGNAL(interruptSpellCast(const uint8_t*, size_t, uint8_t)),
	   this, SLOT(interruptSpellCast(const uint8_t*)));
   connect(m_packet, SIGNAL(moneyUpdate(const uint8_t*, size_t, uint8_t)),
	   this, SLOT(moneyUpdate(const uint8_t*)));
   connect(m_packet, SIGNAL(moneyThing(const uint8_t*, size_t, uint8_t)),
	   this, SLOT(moneyThing(const uint8_t*)));
#endif // ZBTEMP

   connect(m_packet, SIGNAL(toggle_session_tracking()),
	   this, SLOT(toggle_net_session_tracking()));
   
   // connect EQInterface slots to ZoneMgr signals
  connect(m_zoneMgr, SIGNAL(zoneBegin(const QString&)),
	  this, SLOT(zoneBegin(const QString&)));
  connect(m_zoneMgr, SIGNAL(zoneEnd(const QString&, const QString&)),
	  this, SLOT(zoneEnd(const QString&, const QString&)));
  connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
	  this, SLOT(zoneChanged(const QString&)));

   // connect the SpellShell slots to EQInterface signals
   connect(this, SIGNAL(spellMessage(QString&)),
	   m_spellShell, SLOT(spellMessage(QString&)));

   // connect EQInterface slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(addItem(const Item*)),
	   this, SLOT(addItem(const Item*)));
   connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	   this, SLOT(delItem(const Item*)));
   connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	   this, SLOT(killSpawn(const Item*)));
   connect(m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SLOT(changeItem(const Item*)));
   connect(m_spawnShell, SIGNAL(spawnConsidered(const Item*)),
	   this, SLOT(spawnConsidered(const Item*)));

   // connect the SpawnShell slots to Packet signals
   m_packet->connect2("OP_GroundSpawn", SP_Zone, DIR_Server,
		      "makeDropStruct", SZC_Match,
		      m_spawnShell, SLOT(newGroundItem(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_ClickObject", SP_Zone, DIR_Server,
		      "remDropStruct", SZC_Match,
		      m_spawnShell, SLOT(removeGroundItem(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_SpawnDoor", SP_Zone, DIR_Server,
		      "doorStruct", SZC_Modulus,
		      m_spawnShell, SLOT(newDoorSpawns(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_NewSpawn", SP_Zone, DIR_Server,
		      "spawnStruct", SZC_Match,
		      m_spawnShell, SLOT(newSpawn(const uint8_t*)));
   m_packet->connect2("OP_MobUpdate", SP_Zone, DIR_Server|DIR_Client,
		      "spawnPositionUpdate", SZC_Match,
		      m_spawnShell, SLOT(updateSpawns(const uint8_t*)));
   m_packet->connect2("OP_WearChange", SP_Zone, DIR_Server|DIR_Client,
		      "SpawnUpdateStruct", SZC_Match,
		      m_spawnShell, SLOT(updateSpawnInfo(const uint8_t*)));
   m_packet->connect2("OP_HPUpdate", SP_Zone, DIR_Server|DIR_Client,
		      "hpNpcUpdateStruct", SZC_Match,
		      m_spawnShell, SLOT(updateNpcHP(const uint8_t*)));
   m_packet->connect2("OP_DeleteSpawn", SP_Zone, DIR_Server|DIR_Client,
		      "deleteSpawnStruct", SZC_Match,
		      m_spawnShell, SLOT(deleteSpawn(const uint8_t*)));
   m_packet->connect2("OP_Death", SP_Zone, DIR_Server,
		      "newCorpseStruct", SZC_Match,
		      m_spawnShell, SLOT(killSpawn(const uint8_t*)));
#if 0 // ZBTEMP
   connect(m_packet, SIGNAL(spawnWearingUpdate(const uint8_t*, size_t, uint8_t)),
	   m_spawnShell, SLOT(spawnWearingUpdate(const uint8_t*)));
#endif
   m_packet->connect2("OP_Consider", SP_Zone, DIR_Server|DIR_Client,
		      "considerStruct", SZC_Match,
		      m_spawnShell, SLOT(consMessage(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_ClientUpdate", SP_Zone, DIR_Server,
		      "playerSpawnPosStruct", SZC_Match,
		      m_spawnShell, SLOT(playerUpdate(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_CorpseLocResponse", SP_Zone, DIR_Server,
		      "corpseLocStruct", SZC_Match,
		      m_spawnShell, SLOT(corpseLoc(const uint8_t*)));
   m_packet->connect2("OP_ZoneSpawns", SP_Zone, DIR_Server,
		      "spawnStruct", SZC_Modulus,
		      m_spawnShell, SLOT(zoneSpawns(const uint8_t*, size_t)));

   // connect the SpellShell slots to ZoneMgr signals
   connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
	   m_spellShell, SLOT(zoneChanged()));
   
   // connect the SpellShell slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	   m_spellShell, SLOT(killSpawn(const Item*)));
   
   // connect the SpellShell slots to Player signals
   connect(m_player, SIGNAL(newPlayer(void)),
	   m_spellShell, SLOT(clear()));
   connect(m_player, SIGNAL(buffLoad(const spellBuff *)), 
	   m_spellShell, SLOT(buffLoad(const spellBuff *)));

   // connect the SpellShell slots to EQPacket signals
   m_packet->connect2("OP_CastSpell", SP_Zone, DIR_Server|DIR_Client,
		      "startCastStruct", SZC_Match,
		      m_spellShell, SLOT(selfStartSpellCast(const uint8_t*)));
   m_packet->connect2("OP_Buff", SP_Zone, DIR_Server|DIR_Client,
		      "buffStruct", SZC_Match,
		      m_spellShell, SLOT(buff(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_Action", SP_Zone, DIR_Server|DIR_Client,
		      "actionStruct", SZC_Match,
		      m_spellShell, SLOT(action(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_SimpleMessage", SP_Zone, DIR_Server,
		      "simpleMessageStruct", SZC_None,
		      m_spellShell,
		      SLOT(simpleMessage(const uint8_t*, size_t, uint8_t)));


   // connect Player slots to EQPacket signals
   m_packet->connect2("OP_PlayerProfile", SP_Zone, DIR_Server,
		      "charProfileStruct", SZC_Match,
		      m_player, SLOT(player(const uint8_t*)));
   m_packet->connect2("OP_SkillUpdate", SP_Zone, DIR_Server,
		      "skillIncStruct", SZC_Match,
		      m_player, SLOT(increaseSkill(const uint8_t*)));
   m_packet->connect2("OP_ManaChange", SP_Zone, DIR_Server,
		      "manaDecrementStruct", SZC_Match,
		      m_player, SLOT(manaChange(const uint8_t*)));
   m_packet->connect2("OP_ClientUpdate", SP_Zone, DIR_Server|DIR_Client,
		      "playerSelfPosStruct", SZC_Match,
		      m_player, SLOT(playerUpdateSelf(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_ExpUpdate", SP_Zone, DIR_Server,
		      "expUpdateStruct", SZC_Match,
		      m_player, SLOT(updateExp(const uint8_t*)));
   m_packet->connect2("OP_LevelUpdate", SP_Zone, DIR_Server,
		      "levelUpUpdateStruct", SZC_Match,
		      m_player, SLOT(updateLevel(const uint8_t*)));
   m_packet->connect2("OP_HPUpdate", SP_Zone, DIR_Server|DIR_Client,
		      "hpNpcUpdateStruct", SZC_Match,
		      m_player, SLOT(updateNpcHP(const uint8_t*)));
   m_packet->connect2("OP_WearChange", SP_Zone, DIR_Server|DIR_Client,
		      "SpawnUpdateStruct", SZC_Match,
		      m_player, SLOT(updateSpawnInfo(const uint8_t*)));
   m_packet->connect2("OP_Stamina", SP_Zone, DIR_Server,
		      "staminaStruct", SZC_Match,
		      m_player, SLOT(updateStamina(const uint8_t*)));
   m_packet->connect2("OP_Consider", SP_Zone, DIR_Server|DIR_Client,
		      "considerStruct", SZC_Match,
		      m_player, SLOT(consMessage(const uint8_t*, size_t, uint8_t)));
   m_packet->connect2("OP_SwapSpell", SP_Zone, DIR_Server,
		      "tradeSpellBookSlotsStruct", SZC_Match,
	   m_player, SLOT(tradeSpellBookSlots(const uint8_t*, size_t, uint8_t)));
#if 0 // ZBTEMP
   connect(m_packet, SIGNAL(updateAltExp(const uint8_t*, size_t, uint8_t)),
	   m_player, SLOT(updateAltExp(const uint8_t*)));
#endif

   // interface statusbar slots
   connect (this, SIGNAL(newZoneName(const QString&)),
            m_stsbarZone, SLOT(setText(const QString&)));
   connect (m_packet, SIGNAL(stsMessage(const QString &, int)),
            this, SLOT(stsMessage(const QString &, int)));
   connect (m_spawnShell, SIGNAL(numSpawns(int)),
            this, SLOT(numSpawns(int)));
   connect (m_packet, SIGNAL(numPacket(int, int)),
            this, SLOT(numPacket(int, int)));
   connect (m_packet, SIGNAL(resetPacket(int, int)),
            this, SLOT(resetPacket(int, int)));
   connect (m_player, SIGNAL(newSpeed(int)),
            this, SLOT(newSpeed(int)));
   connect(m_player, SIGNAL(setExp(uint32_t, uint32_t, uint32_t, uint32_t, 
				   uint32_t)),
	   this, SLOT(setExp(uint32_t, uint32_t, uint32_t, 
			     uint32_t, uint32_t)));
   connect(m_player, SIGNAL(newExp(uint32_t, uint32_t, uint32_t, uint32_t, 
				   uint32_t, uint32_t)),
	   this, SLOT(newExp(uint32_t, uint32_t, uint32_t,  
			     uint32_t, uint32_t, uint32_t)));
   connect(m_player, SIGNAL(setAltExp(uint32_t, uint32_t, uint32_t, uint32_t)),
	   this, SLOT(setAltExp(uint32_t, uint32_t, uint32_t, uint32_t)));
   connect(m_player, SIGNAL(newAltExp(uint32_t, uint32_t, uint32_t, uint32_t,
				      uint32_t, uint32_t)),
	   this, SLOT(newAltExp(uint32_t, uint32_t, uint32_t, uint32_t,
				uint32_t, uint32_t)));
   connect(m_player, SIGNAL(levelChanged(uint8_t)),
	   this, SLOT(levelChanged(uint8_t)));

   if (m_expWindow != 0)
   {
     // connect ExperienceWindow slots to Player signals
     connect(m_player, SIGNAL(newPlayer(void)),
	     m_expWindow, SLOT(clear(void)));
     connect(m_player, SIGNAL(expGained(const QString &, int, long, QString )),
	     m_expWindow, SLOT(addExpRecord(const QString &, int, long,QString )));
     
     // connect ExperienceWindow slots to EQInterface signals
     connect(this, SIGNAL(restoreFonts(void)),
	     m_expWindow, SLOT(restoreFont(void)));
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_expWindow, SLOT(savePrefs(void)));
   }

   if (m_combatWindow != 0)
   {
     // connect CombatWindow slots to the signals
     connect(m_player, SIGNAL(newPlayer(void)),
	     m_combatWindow, SLOT(clear(void)));
     connect (this, SIGNAL(combatSignal(int, int, int, int, int, QString, QString)),
	      m_combatWindow, SLOT(addCombatRecord(int, int, int, int, int, QString, QString)));
     connect (m_spawnShell, SIGNAL(spawnConsidered(const Item*)),
	      m_combatWindow, SLOT(resetDPS()));
     connect(this, SIGNAL(restoreFonts(void)),
	     m_combatWindow, SLOT(restoreFont(void)));
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_combatWindow, SLOT(savePrefs(void)));
   }

   //
   // Geometry Configuration
   //
   
   QSize s;
   QPoint p;


   // interface components

   // set mainwindow Geometry
   section = "Interface";
   s = pSEQPrefs->getPrefSize("WindowSize", section, size());
   resize(s);
   if (pSEQPrefs->getPrefBool("UseWindowPos", section, true)) 
   {
     p = pSEQPrefs->getPrefPoint("WindowPos", section, pos());
     move(p);
   }
   show();

   QAccel *accel = new QAccel(this);
   accel->connectItem( accel->insertItem(CTRL+ALT+Key_S), this, SLOT(toggle_view_statusbar()));
   accel->connectItem( accel->insertItem(CTRL+ALT+Key_T), this, SLOT(toggle_view_menubar()));

   // load in the docking preferences if any have been saved
   QString dockPrefs = pSEQPrefs->getPrefString("DockingInfo", section, 
						QString());
   if (!dockPrefs.isEmpty())
   {
     QTextStream ts(&dockPrefs, IO_ReadOnly);
     ts >> *this;
   }

   // Set main window title
   // TODO: Add % replacement values and a signal to update, for ip address currently
   // TODO: being monitored.

   QMainWindow::setCaption(pSEQPrefs->getPrefString("Caption", section, 
						    "ShowEQ - Main (ctrl+alt+t to toggle menubar)"));

   // load the format strings for display
   loadFormatStrings();

   /* Start the packet capturing */
   m_packet->start (10);
}// end constructor
////////////////////

EQInterface::~EQInterface()
{
  if (m_netDiag != 0)
    delete m_netDiag;

  if (m_spawnPointList != 0)
    delete m_spawnPointList;

  if (m_statList != 0)
    delete m_statList;

  if (m_guildListWindow != 0)
    delete m_guildListWindow;

  if (m_skillList != 0)
    delete m_skillList;

  if (m_spellList != 0)
    delete m_spellList;

  if (m_spawnList2 != 0)
    delete m_spawnList2;

  if (m_spawnList != 0)
    delete m_spawnList;

  for (int i = 0; i < maxNumMaps; i++)
    if (m_map[i] != 0)
      delete m_map[i];

  for (int i = 0; i < maxNumMessageWindows; i++)
    if (m_messageWindow[i] != 0)
      delete m_messageWindow[i];

  if (m_combatWindow != 0)
    delete m_combatWindow;

  if (m_expWindow != 0)
    delete m_expWindow;

  if (m_spawnLogger != 0)
    delete m_spawnLogger;

  if (m_spawnMonitor != 0)
    delete m_spawnMonitor;

  if (m_groupMgr != 0)
    delete m_groupMgr;

  if (m_spellShell != 0)
    delete m_spellShell;

  if (m_mapMgr != 0)
    delete m_mapMgr;

  if (m_spawnShell != 0)
    delete m_spawnShell;

  if (m_categoryMgr != 0)
    delete m_categoryMgr;

  if (m_filterMgr != 0)
    delete m_filterMgr;

  if (m_dateTimeMgr != 0)
    delete m_dateTimeMgr;

  if (m_eqStrings != 0)
    delete m_eqStrings;

  // Shutdown the Item DB before any other cleanup
  if (m_itemDB != 0)
     m_itemDB->Shutdown();

  // delete the ItemDB before application exit
  delete m_itemDB;

  if (m_player != 0)
    delete m_player;

  if (m_guildShell != 0)
    delete m_guildShell;

  if (m_zoneMgr != 0)
    delete m_zoneMgr;
  
  if (m_packet != 0)
    delete m_packet;
}

void EQInterface::restoreStatusFont()
{
   QFont defFont;
   defFont.setPointSize(8);
   QFont statusFont = pSEQPrefs->getPrefFont("StatusFont", "Interface", 
					     defFont);
				  
   int statusFixedHeight = statusFont.pointSize() + 6;

   // set the correct font information and sizes of the status bar widgets
   m_stsbarStatus->setFont(statusFont);
   m_stsbarStatus->setFixedHeight(statusFixedHeight);
   m_stsbarZone->setFont(statusFont);
   m_stsbarZone->setFixedHeight(statusFixedHeight);
   m_stsbarSpawns->setFont(statusFont);
   m_stsbarSpawns->setFixedHeight(statusFixedHeight);
   m_stsbarExp->setFont(statusFont);
   m_stsbarExp->setFixedHeight(statusFixedHeight);
   m_stsbarExpAA->setFont(statusFont);
   m_stsbarExpAA->setFixedHeight(statusFixedHeight);
   m_stsbarPkt->setFont(statusFont);
   m_stsbarPkt->setFixedHeight(statusFixedHeight);
   m_stsbarEQTime->setFont(statusFont);
   m_stsbarEQTime->setFixedHeight(statusFixedHeight);
   m_stsbarSpeed->setFont(statusFont);
   m_stsbarSpeed->setFixedHeight(statusFixedHeight);
   // ZEM code
   m_stsbarZEM->setFont(statusFont);
   m_stsbarZEM->setFixedHeight(statusFixedHeight);
}

void EQInterface::toggle_view_StatWin( int id )
{
   int statnum;

   statnum = m_statWinMenu->itemParameter(id);

   if (m_statWinMenu->isItemChecked(id))
   {
       m_statWinMenu->setItemChecked(id, FALSE);
       if (m_statList != 0)
	 m_statList->statList()->enableStat(statnum, false);
   }
   else
   {
       m_statWinMenu->setItemChecked(id, TRUE);
       if (m_statList != 0)
	 m_statList->statList()->enableStat(statnum, true);
   }
}

void EQInterface::toggle_view_SkillWin( int id )
{
  int skillnum;

  skillnum = m_skillWinMenu->itemParameter(id);

  if (m_skillWinMenu->isItemChecked(id))
  {
    m_skillWinMenu->setItemChecked(id, FALSE);

    if ((id == m_id_view_PlayerSkills_Languages) &&
	(m_skillList != 0))
      m_skillList->skillList()->showLanguages(false);
   }
   else
   {
       m_skillWinMenu->setItemChecked(id, TRUE);

    if ((id == m_id_view_PlayerSkills_Languages) &&
	(m_skillList != 0))
      m_skillList->skillList()->showLanguages(true);
   }
}

void EQInterface::toggle_view_SpawnListCol( int id )
{
  int colnum;

  colnum = m_spawnListMenu->itemParameter(id);
  
  if (m_spawnListMenu->isItemChecked(id))
  {
    m_spawnListMenu->setItemChecked(id, FALSE);
    
    if (m_spawnList != 0)
      m_spawnList->spawnList()->setColumnVisible(colnum, false);
  }
  else
  {
    m_spawnListMenu->setItemChecked(id, TRUE);
    
    if (m_spawnList != 0)
      m_spawnList->spawnList()->setColumnVisible(colnum, true);
   }
}

void EQInterface::toggle_view_DockedWin( int id )
{
  SEQWindow* widget = 0;
  int winnum;
  QString preference;

  // get the window number parameter
  winnum = m_dockedWinMenu->itemParameter(id);

  // get the current menu item state
  bool checked = m_dockedWinMenu->isItemChecked(id);

  // flip the menu item state
  m_dockedWinMenu->setItemChecked(id, !checked);

  switch(winnum)
  {
  case 0: // Spawn List
    // note the new setting
    m_isSpawnListDocked = !checked;

    // reparent the Spawn List
    widget = m_spawnList;

    // preference
    preference = "DockedSpawnList";
    break;
  case 1: // Player Stats
    // note the new setting
    m_isStatListDocked = !checked;

    // reparent the Stat List
    widget = m_statList;

    // preference
    preference = "DockedPlayerStats";
    break;
  case 2: // Player Skills
    // note the new setting
    m_isSkillListDocked = !checked;
    
    // reparent the Skill List
    widget = m_skillList;

    // preference
    preference = "DockedPlayerSkills";
    break;
  case 3: // Spell List
    // note the new setting
    m_isSpellListDocked = !checked;
    
    // reparent the Skill List
    widget = m_spellList;

    // preference
    preference = "DockedSpellList";
    break;
  case 4: // Compass
    // note the new setting
    m_isCompassDocked = !checked;
    
    // reparent the Skill List
    widget = m_compass;

    // preference
    preference = "DockedCompass";
    break;
  case 5: // Spawn Point List
    // note the new setting
    m_isSpawnPointListDocked = !checked;

    // reparent the Spawn List
    widget = m_spawnPointList;

    // preference
    preference = "DockedSpawnPointList";
    break;
  case 6: // Spawn List 2
    // note the new setting
    m_isSpawnList2Docked = !checked;

    // reparent the Spawn List
    widget = m_spawnList2;

    // preference
    preference = "DockedSpawnList2";
    break;
  default:
    // use default for maps since the number of them can be changed via a 
    // constant (maxNumMaps)
    if ((winnum >= mapDockBase) && (winnum < (mapDockBase + maxNumMaps)))
    {
      int i = winnum - mapDockBase;

      // note the new setting
      m_isMapDocked[i] = !checked;
      
      // reparent teh appropriate map
      widget = m_map[i];

      QString tmpPrefSuffix = "";
      if (i > 0)
	tmpPrefSuffix = QString::number(i + 1);

      // preference
      preference = "DockedMap" + tmpPrefSuffix;
    }

    break;
    };

  // save new setting
  pSEQPrefs->setPrefBool(preference, "Interface", !checked);

  // attempt to undock the window
  if (widget)
  {
    if (checked)
      widget->undock();
    else
      widget->dock();

    // make the widget update it's geometry
    widget->updateGeometry();
  }
}


void EQInterface::toggle_view_DockableWin( int id )
{
  SEQWindow* widget = 0;
  int winnum;
  QString preference;

  // get the window number parameter
  winnum = m_dockableWinMenu->itemParameter(id);

  // get the current menu item state
  bool dockable = !m_dockableWinMenu->isItemChecked(id);

  // flip the menu item state
  m_dockableWinMenu->setItemChecked(id, dockable);

  switch(winnum)
  {
  case 0: // Spawn List
    widget = m_spawnList;

    // preference
    preference = "DockableSpawnList";
    break;
  case 1: // Player Stats
    widget = m_statList;

    // preference
    preference = "DockablePlayerStats";
    break;
  case 2: // Player Skills
    widget = m_skillList;

    // preference
    preference = "DockablePlayerSkills";
    break;
  case 3: // Spell List
    widget = m_spellList;

    // preference
    preference = "DockableSpellList";
    break;
  case 4: // Compass
    widget = m_compass;

    // preference
    preference = "DockableCompass";
    break;
  case 5: // Spawn Point List
    widget = m_spawnPointList;

    // preference
    preference = "DockableSpawnPointList";
    break;
  case 6: // Spawn List 2
    widget = m_spawnList2;

    // preference
    preference = "DockableSpawnList2";
    break;
  case 7: // Experience Window
    widget = m_expWindow;

    preference = "DockableExperienceWindow";
    break;
  case 8: // Combat Window
    widget = m_combatWindow;

    preference = "DockableCombatWindow";
    break;
  case 9: // Guild List Window
    widget = m_guildListWindow;

    preference = "DockableGuildListWindow";
    break;
  case 10: // NetDiag
    widget = m_netDiag;
    
    preference = "DockableNetDiag";
    break;
  default:
    // use default for maps since the number of them can be changed via a 
    // constant (maxNumMaps)
    if ((winnum >= mapDockBase) && (winnum < (mapDockBase + maxNumMaps)))
    {
      int i = winnum - mapDockBase;

      // reparent teh appropriate map
      widget = m_map[i];

      QString tmpPrefSuffix = "";
      if (i > 0)
	tmpPrefSuffix = QString::number(i + 1);

      // preference
      preference = "DockableMap" + tmpPrefSuffix;
    }
    else if ((winnum >= messageWindowDockBase) && 
	     (winnum < (messageWindowDockBase + maxNumMessageWindows)))
    {
      int i = winnum - messageWindowDockBase;

      // reparent teh appropriate map
      widget = m_messageWindow[i];

      QString tmpPrefSuffix = "";
      tmpPrefSuffix = QString::number(i);

      // preference
      preference = "DockableMessageWindow" + tmpPrefSuffix;
    }

    break;
    };

  // save new setting
  pSEQPrefs->setPrefBool(preference, "Interface", dockable);

  // attempt to undock the window
  if (widget)
    setDockEnabled(widget, dockable);
}

void EQInterface::set_main_WindowCaption( int id )
{
  QWidget* widget = 0;
  int winnum;
  QString window;

  // get the window number parameter
  winnum = menuBar()->itemParameter(id);

  switch(winnum)
  {
  case 0: // Spawn List
    widget = m_spawnList;

    window = "Spawn List";
    break;
  case 1: // Player Stats
    widget = m_statList;

    window = "Player Stats";
    break;
  case 2: // Player Skills
    widget = m_skillList;

    window = "Player Skills";
    break;
  case 3: // Spell List
    widget = m_spellList;

    window = "Spell List";
    break;
  case 4: // Compass
    widget = m_compass;

    window = "Compass";
    break;
  case 5: // Interface
    widget = this;

    window = "Main Window";
    break;
  case 6: // Experience Window
    widget = m_expWindow;

    window = "Experience Window";
    break;
  case 7: // Combat Window
    widget = m_combatWindow;

    window = "Combat Window";
    break;
  case 8: // Network Diagnostics
    widget = m_netDiag;

    window = "Network Diagnostics";
    break;
  case 9: // Spawn Point List
    widget = m_spawnPointList;

    window = "Spawn Point List";
  case 10: // Spawn List
    widget = m_spawnList2;

    window = "Spawn List 2";
    break;
  default:
    // use default for maps since the number of them can be changed via a 
    // constant (maxNumMaps)
    if ((winnum >= mapCaptionBase) && (winnum < (mapCaptionBase + maxNumMaps)))
    {
      int i = winnum - mapCaptionBase;

      widget = m_map[i];
    }

    break;
  };

  // attempt to undock the window
  if (widget != 0)
  {
    bool ok = false;
    QString caption = 
      QInputDialog::getText("ShowEQ " + window + "Caption",
			    "Enter caption for the " + window + ":",
			    QLineEdit::Normal, widget->caption(),
			    &ok, this);
					    
    // if the user entered a caption and clicked ok, set the windows caption
    if (ok)
      widget->setCaption(caption);
  }
}


void EQInterface::set_main_WindowFont( int id )
{
  int winnum;

  // get the window number parameter
  winnum = menuBar()->itemParameter(id);

  bool ok = false;
  QFont newFont;
  SEQWindow* window = 0;
  QString title;
  
  //
  // NOTE: Yeah, this sucks for now, but until the architecture gets cleaned
  // up it will have to do
  switch(winnum)
  {
  case 0: // Spawn List
    title = "Spawn List";
    
    window = m_spawnList;
    break;
  case 1: // Player Stats
    title = "Player Stats";
    
    window = m_statList;
    break;
  case 2: // Player Skills
    title = "Player Skills";

    window = m_skillList;
    break;
  case 3: // Spell List
    title = "Spell List";
    
    window = m_spellList;
    break;
  case 4: // Compass
    title = "Compass";
    
    window = m_compass;
    break;
  case 5: // Interface
    // window = "Main Window";
    break;
  case 6: // Experience Window
    title = "Experience Window";
    
    window = m_expWindow;
    break;
  case 7: // Combat Window
    title = "Combat Window";

    window = m_combatWindow;
    break;
  case 8: // Network Diagnostics
    title = "Network Diagnostics";

    window = m_netDiag;
    break;
  case 9: // Spawn Point List
    title = "Spawn Point List";

    window = m_spawnPointList;
  case 10: // Spawn List
    title = "Spawn List 2";
    
    window = m_spawnList2;
    break;
  default:
    // use default for maps since the number of them can be changed via a 
    // constant (maxNumMaps)
    if ((winnum >= mapCaptionBase) && (winnum < (mapCaptionBase + maxNumMaps)))
    {
      int i = winnum - mapCaptionBase;
      if (i)
	title.sprintf("Map %d", i);
      else
	title = "Map";

      window = m_map[i];
    }
    break;
  };

  if (window != 0)
  {
    // get a new font
    newFont = QFontDialog::getFont(&ok, window->font(), 
				   this, "ShowEQ " + title + " Font");
    
    
    // if the user entered a font and clicked ok, set the windows font
    if (ok)
      window->setWindowFont(newFont);
  }
}

void EQInterface::set_main_Font(int id)
{
  QString name = "ShowEQ - Application Font";
  bool ok = false;

  // get a new application font
  QFont newFont;
  newFont = QFontDialog::getFont(&ok, QApplication::font(), 
				 this, name);

  // if the user clicked ok and selected a valid font, set it
  if (ok)
  {
    // set the new application font
    qApp->setFont( newFont, true );

    // set the preference for future sessions
    pSEQPrefs->setPrefFont("Font", "Interface", 
			   newFont);

    // make sure the windows that override the application font, do so
    emit restoreFonts();
  }
}


void EQInterface::select_main_FormatFile(int id)
{
  QString formatFile = pSEQPrefs->getPrefString("FormatFile", "Interface", 
						"eqstr_us.txt");

  QFileInfo fileInfo = m_dataLocationMgr->findExistingFile(".", formatFile);

  QString newFormatFile = 
    QFileDialog::getOpenFileName(fileInfo.absFilePath(), "*.txt", 
				 this, "FormatFile", "Select Format File");

  // if the newFormatFile name is not empty, then the user selected a file
  if (!newFormatFile.isEmpty())
  {
    // set the new format file to use
    pSEQPrefs->setPrefString("FormatFile", "Interface", newFormatFile);
			     
    // reload the format strings
    loadFormatStrings();
  }
}

void EQInterface::select_main_SpellsFile(int id)
{
  QString spellsFile = pSEQPrefs->getPrefString("SpellsFile", "Interface", 
						"spells_us.txt");

  QFileInfo fileInfo = m_dataLocationMgr->findExistingFile(".", spellsFile);

  QString newSpellsFile = 
    QFileDialog::getOpenFileName(fileInfo.absFilePath(), "*.txt", 
				 this, "FormatFile", "Select Format File");

  // if the newFormatFile name is not empty, then the user selected a file
  if (!newSpellsFile.isEmpty())
  {
    // set the new format file to use
    pSEQPrefs->setPrefString("SpellsFile", "Interface", newSpellsFile);

    // reload the spells
    m_spells->loadSpells(newSpellsFile);
  }
}

void EQInterface::toggle_main_statusbar_Window(int id)
{
  QWidget* window = 0;
  QString preference;

  switch (menuBar()->itemParameter(id))
  {
  case 1:
    window = m_stsbarStatus;

    preference = "ShowStatus";
    break;
  case 2:
    window = m_stsbarZone;

    preference = "ShowZone";
    break;
  case 3:
    window = m_stsbarSpawns;

    preference = "ShowSpawns";
    break;
  case 4:
    window = m_stsbarExp;

    preference = "ShowExp";
    break;
  case 5:
    window = m_stsbarExpAA;

    preference = "ShowExpAA";
    break;
  case 6:
    window = m_stsbarPkt;

    preference = "ShowPacketCounter";
    break;
  case 7:
    window = m_stsbarEQTime;

    preference = "ShowEQTime";
    break;
  case 8:
    window = m_stsbarSpeed;

    preference = "ShowSpeed";
    break;
  // ZEM code
  case 9:
    window = m_stsbarZEM;

    preference = "ShowZEM";
    break;

  default:
    return;
  }

  if (window == 0)
    return;

  // should the window be visible
  bool show = !window->isVisible();

  // show or hide the window as necessary
  if (show)
    window->show();
  else
    window->hide();

  // check/uncheck the menu item
  menuBar()->setItemChecked(id, show); 

  // set the preference for future sessions
  pSEQPrefs->setPrefBool(preference, "Interface_StatusBar", show);
}

void EQInterface::set_main_statusbar_Font(int id)
{
  QString name = "ShowEQ - Status Font";
  bool ok = false;

  // setup a default new status font
  QFont newFont = QApplication::font();
  newFont.setPointSize(8);

  // get new status font
  newFont = QFontDialog::getFont(&ok, 
				 pSEQPrefs->getPrefFont("StatusFont",
							"Interface",
							newFont),
				 this, name);

  // if the user clicked ok and selected a valid font, set it
  if (ok)
  {
    // set the preference for future sessions
    pSEQPrefs->setPrefFont("StatusFont", "Interface", 
			   newFont);

    // make sure to reset the status font since the previous call may have 
    // changed it
    restoreStatusFont();
  }
}

void
EQInterface::toggle_main_SavePosition (int id)
{
    pSEQPrefs->setPrefBool("SavePosition", "Interface", 
			   !pSEQPrefs->getPrefBool("SavePosition", 
						   "Interface"));

    m_windowMenu->setItemChecked(id, pSEQPrefs->getPrefBool("SavePosition", 
							 "Interface"));
}

void
EQInterface::toggle_main_UseWindowPos (int id)
{
    pSEQPrefs->setPrefBool("UseWindowPos", "Interface", 
			   !pSEQPrefs->getPrefBool("UseWindowPos", 
						   "Interface"));
    m_windowMenu->setItemChecked(id, pSEQPrefs->getPrefBool("UseWindowPos", 
							 "Interface"));
}

//
// save prefs
//
void
EQInterface::savePrefs(void)
{
  seqDebug("==> EQInterface::savePrefs()");
   
   if( isVisible() ) 
   {
     seqDebug("\tisVisible()");
     QString section;
     QString interfaceSection = "Interface";
     QString tempStr;

     QString dockPrefs;
     QTextStream ts(&dockPrefs, IO_WriteOnly);

     ts << *this;

     pSEQPrefs->setPrefString("DockingInfo", interfaceSection, dockPrefs);

     // send savePrefs signal out
     emit saveAllPrefs();
     
     section = "Interface";
     if (pSEQPrefs->getPrefBool("SavePosition", interfaceSection, true)) 
     {
       pSEQPrefs->setPrefPoint("WindowPos", section, 
			       topLevelWidget()->pos());
       pSEQPrefs->setPrefSize("WindowSize", section, 
			      topLevelWidget()->size());
     }
     
     // save prefs to file
     pSEQPrefs->save();
   }
} // end savePrefs

void EQInterface::saveDockAreaPrefs(QDockArea* a, Dock edge)
{
  QPtrList<QDockWindow> l = a->dockWindowList();
  for (QDockWindow *dw = l.first(); dw; dw = l.next())
  {
    if (dw->inherits("SEQWindow"))
      pSEQPrefs->setPrefInt("Dock", ((SEQWindow*)dw)->preferenceName(), edge);
  }
}


void EQInterface::setCaption(const QString& text)
{
  QMainWindow::setCaption(text);

  pSEQPrefs->setPrefString("Caption", "Interface", caption());
}


void EQInterface::loadFormatStrings()
{
  // get the name of the format file
  QString formatFileName = pSEQPrefs->getPrefString("FormatFile", "Interface", 
						    "eqstr_us.txt");

  QFileInfo fileInfo = m_dataLocationMgr->findExistingFile(".", 
							   formatFileName);

  // load the strings
  m_eqStrings->load(fileInfo.absFilePath());
}

void
EQInterface::select_filter_file(void)
{
  QString filterFile = QFileDialog::getOpenFileName(m_filterMgr->filterFile(),
                                                     QString("ShowEQ Filter Files (*.xml)"),
                                                     0,
                                                     "Select Filter Config..."
                                                   );
  if (!filterFile.isEmpty())
    m_filterMgr->loadFilters(filterFile);
}

void
EQInterface::save_as_filter_file(void)
{
   QString shortZoneName = m_zoneMgr->shortZoneName();
   if (!shortZoneName.isEmpty())
      m_filterMgr->saveAsFilters(shortZoneName);
}

void EQInterface::toggle_filter_Case(int id)
{
  m_filterMgr->setCaseSensitive(!m_filterMgr->caseSensitive());
  menuBar()->setItemChecked(id, m_filterMgr->caseSensitive());
  pSEQPrefs->setPrefBool("IsCaseSensitive", "Filters", 
			 m_filterMgr->caseSensitive());
}

void EQInterface::toggle_filter_AlertInfo(int id)
{
  pSEQPrefs->setPrefBool("AlertInfo", "Filters", 
			 !pSEQPrefs->getPrefBool("AlertInfo", "Filters"));
  menuBar()->setItemChecked(id, 
			    pSEQPrefs->getPrefBool("AlertInfo", "Filters"));
}

void EQInterface::toggle_filter_UseSystemBeep(int id)
{
  m_filterNotifications->setUseSystemBeep(!m_filterNotifications->useSystemBeep());
  menuBar()->setItemChecked(id, m_filterNotifications->useSystemBeep());
}

void EQInterface::toggle_filter_UseCommands(int id)
{
  m_filterNotifications->setUseCommands(!m_filterNotifications->useCommands());
  menuBar()->setItemChecked(id, m_filterNotifications->useCommands());
}

void EQInterface::toggle_filter_Log(int id)
{
  if (!m_filteredSpawnLog)
    createFilteredSpawnLog();

  uint32_t filters = m_filteredSpawnLog->filters();
  uint32_t filter = menuBar()->itemParameter(id);
  
  if (filters & filter)
    filters &= ~filter;
  else
    filters |= filter;

  m_filteredSpawnLog->setFilters(filters);

  menuBar()->setItemChecked(id, ((filters & filter) != 0));
  pSEQPrefs->setPrefBool("Log", "Filters", filters);
}

void EQInterface::set_filter_AudioCommand(int id)
{
  QString property;
  QString prettyName;
  switch(menuBar()->itemParameter(id))
  {
  case 1:
    property = "SpawnAudioCommand";
    prettyName = "Spawn";
    break;
  case 2:
    property = "DeSpawnAudioCommand";
    prettyName = "DeSpawn";
    break;
  case 3:
    property = "DeathAudioCommand";
    prettyName = "Death";
    break;
  case 4:
    property = "LocateSpawnAudioCommand";
    prettyName = "Locate Spawn";
    break;
  case 5:
    property = "CautionSpawnAudioCommand";
    prettyName = "Caution Spawn";
    break;
  case 6:
    property = "HuntSpawnAudioCommand";
    prettyName = "Hunt Spawn";
    break;
  case 7:
    property = "DangerSpawnAudioCommand";
    prettyName = "Danger Spawn";
    break;
  default: 
    return;
  }

  QString value = pSEQPrefs->getPrefString(property, "Filters",
					   "/usr/bin/esdplay " PKGDATADIR "/spawn.wav &");

  bool ok = false;
  QString command = 
    QInputDialog::getText("ShowEQ " + prettyName + "Command",
			  "Enter command line to use for " + prettyName + "'s:",
			  QLineEdit::Normal, value,
			  &ok, this);

  if (ok)
    pSEQPrefs->setPrefString(property, "Filters", command);
}

void EQInterface::listSpawns (void)
{
#ifdef DEBUG
  debug ("listSpawns()");
#endif /* DEBUG */

  QString outText;

  // open the output data stream
  QTextStream out(&outText, IO_WriteOnly);
  
   // dump the spawns 
  m_spawnShell->dumpSpawns(tSpawn, out);

  seqInfo((const char*)outText);
}

void EQInterface::listDrops (void)
{
#ifdef DEBUG
  debug ("listDrops()");
#endif /* DEBUG */
  QString outText;

  // open the output data stream
  QTextStream out(&outText, IO_WriteOnly);

  // dump the drops
  m_spawnShell->dumpSpawns(tDrop, out);

  seqInfo((const char*)outText);
}

void EQInterface::listMapInfo(void)
{
#ifdef DEBUG
  debug ("listMapInfo()");
#endif /* DEBUG */
  QString outText;

  // open the output data stream
  QTextStream out(&outText, IO_WriteOnly);

  // dump map managers info
  m_mapMgr->dumpInfo(out);

  // iterate over all the maps
  for (int i = 0; i < maxNumMaps; i++)
  {
    // if this map has been instantiated, dump it's info
    if (m_map[i] != 0)
      m_map[i]->dumpInfo(out);
  }

  seqInfo((const char*)outText);
}

void EQInterface::listInterfaceInfo(void)
{
#ifdef DEBUG
  debug ("listMapInfo()");
#endif /* DEBUG */

  QString outText;

  // open the output data stream
  QTextStream out(&outText, IO_WriteOnly);

  out << "Map window layout info:" << endl;
  out << "-----------------------" << endl;
  out << *this;
  out << "-----------------------" << endl;


  seqInfo((const char*)outText);
}

void EQInterface::listGroup(void)
{
#ifdef DEBUG
  debug ("listGroup()");
#endif /* DEBUG */
  QString outText;

  // open the output data stream
  QTextStream out(&outText, IO_WriteOnly);

  // dump the drops
  m_groupMgr->dumpInfo(out);

  seqInfo((const char*)outText);
}


void EQInterface::listGuild(void)
{
#ifdef DEBUG
  debug ("listGuild()");
#endif /* DEBUG */
  QString outText;

  // open the output data stream
  QTextStream out(&outText, IO_WriteOnly);

  // dump the drops
  m_guildShell->dumpMembers(out);

  seqInfo((const char*)outText);
}

void EQInterface::dumpSpawns (void)
{
#ifdef DEBUG
  debug ("dumpSpawns()");
#endif /* DEBUG */
  
  QString logFile = pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
					     "dumpspawns.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  // open the output data stream
  QFile file(logFileInfo.absFilePath());
  file.open(IO_WriteOnly);
  QTextStream out(&file);
  
  // dump the spawns 
  m_spawnShell->dumpSpawns(tSpawn, out);
}

void EQInterface::dumpDrops (void)
{
#ifdef DEBUG
  debug ("dumpDrops()");
#endif /* DEBUG */

  QString logFile = pSEQPrefs->getPrefString("DumpDropsFilename", "Interface",
					     "dumpdrops.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  // open the output data stream
  QFile file(logFileInfo.absFilePath());
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the drops
  m_spawnShell->dumpSpawns(tDrop, out);
}

void EQInterface::dumpMapInfo(void)
{
#ifdef DEBUG
  debug ("dumpMapInfo()");
#endif /* DEBUG */

  QString logFile = pSEQPrefs->getPrefString("DumpMapInfoFilename", 
					     "Interface",
					     "mapinfo.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  // open the output data stream
  QFile file(logFileInfo.absFilePath());
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump map managers info
  m_mapMgr->dumpInfo(out);

  // iterate over all the maps
  for (int i = 0; i < maxNumMaps; i++)
  {
    // if this map has been instantiated, dump it's info
    if (m_map[i] != 0)
      m_map[i]->dumpInfo(out);
  }
}

void EQInterface::dumpGuildInfo(void)
{
  QString logFile = pSEQPrefs->getPrefString("GuildsDumpFile", 
					     "Interface", 
					     "guilds.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  emit guildList2text(logFileInfo.absFilePath());
}

void EQInterface::dumpSpellBook(void)
{
#ifdef DEBUG
  debug ("dumpSpellBook");
#endif /* DEBUG */

  QString logFile = pSEQPrefs->getPrefString("DumpSpellBookFilename", 
					     "Interface", 
					     "spellbook.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  // open the output data stream
  QFile file(logFileInfo.absFilePath());
  file.open(IO_WriteOnly);
  QTextStream out(&file);
  QString txt;

  seqInfo("Dumping Spell Book to '%s'\n", 
	  (const char*)file.name().utf8());
  out << "Spellbook of " << m_player->name() << " a level " 
      << m_player->level() << " " << m_player->raceString() 
      << " " << m_player->classString()
      << endl;

  uint8_t playerClass = m_player->classVal();

  uint32_t spellid;
  for (uint32_t i = 0; i < MAX_SPELLBOOK_SLOTS; i++)
  {
    spellid = m_player->getSpellBookSlot(i);
    if (spellid == 0xffffffff)
      continue;

    const Spell* spell = m_spells->spell(spellid);

    QString spellName;

    if (spell)
    {
      txt.sprintf("%.3d %.2d %.2d %#4.04x %02d\t%s", 
		  i, ((i / 8) + 1), ((i % 8) + 1), 
		  spellid, spell->level(playerClass),
		  spell->name().latin1());
    }
    else
    {
      txt.sprintf("%.3d %.2d %.2d %#4.04x   \t%s", 
		  i, ((i / 8) + 1), ((i % 8) + 1), 
		  spellid, 
		  spell_name(spellid).latin1());
    }

    out << txt << endl;
  }
}

void EQInterface::dumpGroup(void)
{
#ifdef DEBUG
  debug ("dumpGroup()");
#endif /* DEBUG */

  QString logFile = pSEQPrefs->getPrefString("DumpGroupFilename", "Interface",
					     "dumpgroup.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  // open the output data stream
  QFile file(logFileInfo.absFilePath());
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the drops
  m_groupMgr->dumpInfo(out);
}

void EQInterface::dumpGuild(void)
{
#ifdef DEBUG
  debug ("dumpGuild()");
#endif /* DEBUG */

  QString logFile = pSEQPrefs->getPrefString("DumpGuildFilename", "Interface",
					     "dumpguild.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("dumps", logFile);

  // open the output data stream
  QFile file(logFileInfo.absFilePath());
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the drops
  m_guildShell->dumpMembers(out);
}

void
EQInterface::launch_editor_filters(void)
{
  EditorWindow * ew = new EditorWindow(m_filterMgr->filterFile());
  ew->setCaption(m_filterMgr->filterFile());
  ew->show();
}

void
EQInterface::launch_editor_zoneFilters(void)
{
  EditorWindow * ew = new EditorWindow(m_filterMgr->zoneFilterFile());
  ew->setCaption(m_filterMgr->zoneFilterFile());
  ew->show();
}

void
EQInterface::toggle_opt_ConSelect (void)
{
  m_selectOnConsider = !(m_selectOnConsider);
  menuBar()->setItemChecked (m_id_opt_ConSelect, m_selectOnConsider);
  pSEQPrefs->setPrefBool("SelectOnCon", "Interface", m_selectOnConsider);
}

void
EQInterface::toggle_opt_TarSelect (void)
{
  m_selectOnTarget = !(m_selectOnTarget);
  menuBar()->setItemChecked (m_id_opt_TarSelect, m_selectOnTarget);
  pSEQPrefs->setPrefBool("SelectOnTarget", "Interface", m_selectOnTarget);
}

void
EQInterface::toggle_opt_Fast (void)
{
  showeq_params->fast_machine = !(showeq_params->fast_machine);
  menuBar()->setItemChecked (m_id_opt_Fast, showeq_params->fast_machine);

  pSEQPrefs->setPrefBool("FastMachine", "Misc", showeq_params->fast_machine);
}

void
EQInterface::toggle_opt_KeepSelectedVisible (void)
{
  showeq_params->keep_selected_visible = !(showeq_params->keep_selected_visible);
  menuBar()->setItemChecked (m_id_opt_KeepSelectedVisible, showeq_params->keep_selected_visible);
  pSEQPrefs->setPrefBool("KeepSelected", "Interface", showeq_params->keep_selected_visible);
}

/* Check and uncheck Log menu options & set EQPacket logging flags */
void EQInterface::toggle_log_AllPackets (void)
{
  if (m_globalLog)
  {
    delete m_globalLog;
    m_globalLog = 0;
  }
  else
    createGlobalLog();

  bool state = (m_globalLog != 0);
  menuBar()->setItemChecked (m_id_log_AllPackets, state);
  pSEQPrefs->setPrefBool("LogAllPackets", "PacketLogging", state);
}

void EQInterface::toggle_log_WorldData (void)
{
  if (m_worldLog)
  {
    delete m_worldLog;
    m_worldLog = 0;
  }
  else
    createWorldLog();

  bool state = (m_worldLog != 0);
  menuBar()->setItemChecked (m_id_log_WorldData, state);
  pSEQPrefs->setPrefBool("LogWorldPackets", "PacketLogging", state);
}

void EQInterface::toggle_log_ZoneData (void)
{
  if (m_zoneLog)
  {
    delete m_zoneLog;
    m_zoneLog = 0;
  }
  else
    createZoneLog();

  bool state = (m_zoneLog != 0);
  menuBar()->setItemChecked (m_id_log_ZoneData, state);
  pSEQPrefs->setPrefBool("LogZonePackets", "PacketLogging", state);
}

void EQInterface::toggle_opt_BazaarData (void)
{
  if (m_bazaarLog)
  {
    disconnect(m_bazaarLog,0,0,0);
    delete m_bazaarLog;
    m_bazaarLog = 0;
  }
  else
    createBazaarLog();

  bool state = (m_bazaarLog != 0);
  menuBar()->setItemChecked(m_id_opt_BazaarData, state);
  pSEQPrefs->setPrefBool("LogBazaarPackets", "PacketLogging", state);
}

void EQInterface::toggle_log_UnknownData (void)
{
  if (m_unknownZoneLog)
  {
    delete m_unknownZoneLog;
    m_unknownZoneLog = 0;
  }
  else
    createUnknownZoneLog();

  bool state = (m_unknownZoneLog != 0);
  menuBar()->setItemChecked (m_id_log_UnknownData, state);
  pSEQPrefs->setPrefBool("LogUnknownZonePackets", "PacketLogging", state);
}

void EQInterface::toggle_log_RawData (void)
{
  bool state = !pSEQPrefs->getPrefBool("LogRawPackets", "PacketLogging",
				       false);

  if (m_worldLog)
    m_worldLog->setRaw(state);
  
  if (m_zoneLog)
    m_zoneLog->setRaw(state);

  menuBar()->setItemChecked(m_id_log_RawData, state);
  pSEQPrefs->setPrefBool("LogRawPackets", "PacketLogging", state);
}

void EQInterface::toggle_log_ItemData (void)
{
  m_itemDB->SetItemLogging(!m_itemDB->GetItemLogging());
  menuBar()->setItemChecked (m_id_log_Items, m_itemDB->GetItemLogging());
  pSEQPrefs->setPrefBool("ItemLogging", "ItemDB", m_itemDB->GetItemLogging());
}

void EQInterface::toggle_log_ItemPacketData (void)
{
  m_itemDB->SetItemPacketLogging(!m_itemDB->GetItemPacketLogging());
  menuBar()->setItemChecked (m_id_log_ItemPackets, m_itemDB->GetItemPacketLogging());
  pSEQPrefs->setPrefBool("ItemPacketLogging", "ItemDB", m_itemDB->GetItemPacketLogging());
}


/* Check and uncheck View menu options */
void
EQInterface::toggle_view_ChannelMsgs(int id)
{
  int winNum = menuBar()->itemParameter(id);

  bool wasVisible = ((m_messageWindow[winNum] != 0) && 
		     (m_messageWindow[winNum]->isVisible()));

  if (!wasVisible)
    showMessageWindow(winNum);
  else
  {
    // save any preference changes
    m_messageWindow[winNum]->savePrefs();

    // hide it 
    m_messageWindow[winNum]->hide();

    // remove its window menu
    removeWindowMenu(m_messageWindow[winNum]);

    // then delete it
    delete m_messageWindow[winNum];

    // make sure to clear it's variable
    m_messageWindow[winNum] = 0;
  }

  QString tmpPrefSuffix = "";
  if (winNum > 0)
    tmpPrefSuffix = QString::number(winNum + 1);
  
  QString tmpPrefName = QString("ShowMessageWindow") + tmpPrefSuffix;

  pSEQPrefs->setPrefBool(tmpPrefName, "Interface", !wasVisible); 
}

void
EQInterface::toggle_view_UnknownData (void)
{
  bool state = !pSEQPrefs->getPrefBool("ViewUnknown", "PacketLogging", 
				       false);

  if (m_unknownZoneLog)
    m_unknownZoneLog->setView(state);

  menuBar()->setItemChecked (m_id_view_UnknownData, state);
  pSEQPrefs->setPrefBool("ViewUnknown", "PacketLogging", state);
}

void EQInterface::toggle_view_ExpWindow (void)
{
    if (!m_expWindow->isVisible())
       m_expWindow->show();
    else
       m_expWindow->hide();

    pSEQPrefs->setPrefBool("ShowExpWindow", "Interface",
			   m_expWindow->isVisible());
}

void EQInterface::toggle_view_CombatWindow (void)
{
  if (!m_combatWindow->isVisible())
    m_combatWindow->show();
  else
    m_combatWindow->hide();

  pSEQPrefs->setPrefBool("ShowCombatWindow", "Interface", 
			 m_combatWindow->isVisible());
}

void
EQInterface::toggle_view_SpawnList(void)
{
  bool wasVisible = ((m_spawnList != 0) && m_spawnList->isVisible());

  if (!wasVisible)
  {
    showSpawnList();

    // enable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_SpawnList_Options, true);
  }
  else 
  {
    // save it's preferences
    m_spawnList->savePrefs();

    // hide it
    m_spawnList->hide();

    // disable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_SpawnList_Options, false);

    // remove its window menu
    removeWindowMenu(m_spawnList);

    // delete the window
    delete m_spawnList;

    // make sure to clear it's variable
    m_spawnList = 0;
  }

  pSEQPrefs->setPrefBool("ShowSpawnList", "Interface", !wasVisible);
}

void
EQInterface::toggle_view_SpawnList2(void)
{
  bool wasVisible = ((m_spawnList2 != 0) && m_spawnList2->isVisible());

  if (!wasVisible)
    showSpawnList2();
  else 
  {
    // save it's preferences
    m_spawnList2->savePrefs();

    // hide it
    m_spawnList2->hide();

    // remove its window menu
    removeWindowMenu(m_spawnList2);

    // delete the window
    delete m_spawnList2;

    // make sure to clear it's variable
    m_spawnList2 = 0;
  }

  pSEQPrefs->setPrefBool("ShowSpawnList2", "Interface", !wasVisible);
}

void
EQInterface::toggle_view_SpawnPointList(void)
{
  bool wasVisible = ((m_spawnPointList != 0) && 
		     m_spawnPointList->isVisible());

  if (!wasVisible)
    showSpawnPointList();
  else 
  {
    // save it's preferences
    m_spawnPointList->savePrefs();

    // hide it
    m_spawnPointList->hide();

    // remove its window menu
    removeWindowMenu(m_spawnPointList);

    // delete the window
    delete m_spawnPointList;

    // make sure to clear it's variable
    m_spawnPointList = 0;
  }

  pSEQPrefs->setPrefBool("ShowSpawnPointList", "Interface", !wasVisible);
}

void EQInterface::toggle_view_SpellList(void)
{
  bool wasVisible = ((m_spellList != 0) && (m_spellList->isVisible()));

  if (!wasVisible)
    showSpellList();
  else
  {
    // save it's preferences
    m_spellList->savePrefs();
    
    // hide it
    m_spellList->hide();

    // remove its window menu
    removeWindowMenu(m_spellList);

    // delete it
    delete m_spellList;
    
    // make sure to clear it's variable
    m_spellList = 0;
  }

  pSEQPrefs->setPrefBool("ShowSpellList", "Interface", !wasVisible); 
}

void EQInterface::toggle_view_PlayerStats(void)
{
  bool wasVisible = ((m_statList != 0) && m_statList->isVisible());

  if (!wasVisible)
  {
    showStatList();

    // enable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_PlayerStats_Options, true);
  }
  else 
  {
    // save it's preferences
    m_statList->savePrefs();

    // hide it
    m_statList->hide();

    // disable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_PlayerStats_Options, false);

    // remove its window menu
    removeWindowMenu(m_statList);

    // then delete it
    delete m_statList;

    // make sure to clear it's variable
    m_statList = 0;
  }
  
  pSEQPrefs->setPrefBool("ShowPlayerStats", "Interface", !wasVisible);
}

void EQInterface::toggle_view_PlayerSkills(void)
{
  bool wasVisible = ((m_skillList != 0) && m_skillList->isVisible());

  if (!wasVisible)
  {
    showSkillList();

    menuBar()->setItemEnabled(m_id_view_PlayerSkills_Options, true);
  }
  else
  {
    // save any preference changes
    m_skillList->savePrefs();

    // if it's not visible, hide it
    m_skillList->hide();

    // disable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_PlayerSkills_Options, false);

    // remove its window menu
    removeWindowMenu(m_skillList);

    // then delete it
    delete m_skillList;

    // make sure to clear it's variable
    m_skillList = 0;
  }

  pSEQPrefs->setPrefBool("ShowPlayerSkills", "Interface", !wasVisible);
}

void
EQInterface::toggle_view_Compass(void)
{
  bool wasVisible = ((m_compass != 0) && (m_compass->isVisible()));

  if (!wasVisible)
    showCompass();
  else
  {
    // if it's not visible, hide it
    m_compass->hide();

    // remove its window menu
    removeWindowMenu(m_compass);

    // then delete it
    delete m_compass;

    // make sure to clear it's variable
    m_compass = 0;
  }

  pSEQPrefs->setPrefBool("ShowCompass", "Interface", !wasVisible);
}

void EQInterface::toggle_view_Map(int id)
{
  int mapNum = menuBar()->itemParameter(id);

  bool wasVisible = ((m_map[mapNum] != 0) && 
		     (m_map[mapNum]->isVisible()));

  if (!wasVisible)
    showMap(mapNum);
  else
  {
    // save any preference changes
    m_map[mapNum]->savePrefs();

    // hide it 
    m_map[mapNum]->hide();

    // remove its window menu
    removeWindowMenu(m_map[mapNum]);

    // then delete it
    delete m_map[mapNum];

    // make sure to clear it's variable
    m_map[mapNum] = 0;
  }

  QString tmpPrefSuffix = "";
  if (mapNum > 0)
    tmpPrefSuffix = QString::number(mapNum + 1);
  
  QString tmpPrefName = QString("ShowMap") + tmpPrefSuffix;

  pSEQPrefs->setPrefBool(tmpPrefName, "Interface", !wasVisible); 
}

void
EQInterface::toggle_view_NetDiag(void)
{
  bool wasVisible = ((m_netDiag != 0) && (m_netDiag->isVisible()));

  if (!wasVisible)
    showNetDiag();
  else
  {
    // if it's not visible, hide it
    m_netDiag->hide();

    // remove its window menu
    removeWindowMenu(m_netDiag);

    // then delete it
    delete m_netDiag;

    // make sure to clear it's variable
    m_netDiag = 0;
  }

  pSEQPrefs->setPrefBool("ShowNetStats", "Interface", !wasVisible);
}

void
EQInterface::toggle_view_GuildList(void)
{
  bool wasVisible = ((m_guildListWindow != 0) && 
		     (m_guildListWindow->isVisible()));

  if (!wasVisible)
    showGuildList();
  else
  {
    // if it's not visible, hide it
    m_guildListWindow->hide();

    // remove its window menu
    removeWindowMenu(m_guildListWindow);

    // then delete it
    delete m_guildListWindow;

    // make sure to clear it's variable
    m_guildListWindow = 0;
  }

  pSEQPrefs->setPrefBool("ShowGuildList", "Interface", !wasVisible);
}

bool 
EQInterface::getMonitorOpCodeList(const QString& title, 
				  QString& opCodeList)
{
  bool ok = false;
  QString newMonitorOpCode_List = 
    QInputDialog::getText(title,
			  "A list of OpCodes seperated by commas...\n"
			  "\n"
			  "Each Opcode has 4 arguments, only one of which is actually necessary...\n"
			  "They are:\n"
			  "OpCode:    16-bit HEX value of the OpCode\n"
			  "            (REQUIRED - No Default)\n"
			  "\n"
			  "Alias:     Name used when displaying the Opcode\n"
			  "            (DEFAULT: Monitored OpCode)\n"
			  "\n"
			  "Direction: 1 = Client ---> Server\n"
			  "           2 = Client <--- Server\n"
			  "           3 = Client <--> Server (BOTH)\n"
			  "            (DEFAULT: 3)\n"
			  "\n"
			  "Show known 1 = Show if OpCode is marked as known.\n"
			  "           0 = Ignore if OpCode is known.\n"
			  "            (DEFAULT: 0)\n"
			  "\n"
			  "The way which you include the arguments in the list of OpCodes is:\n"
			  "adding a ':' inbetween arguments and a ',' after the last OpCode\n"
			  "argument.\n"
			  "\n"
			  "(i.e. 7F21:Mana Changed:3:1, 7E21:Unknown Spell Event(OUT):1,\n"
			  "      7E21:Unknown Spell Event(IN):2 )\n",
			  QLineEdit::Normal,
			  opCodeList,
			  &ok, this);

  if (ok)
    opCodeList = newMonitorOpCode_List;

  return ok;
}

void
EQInterface::toggle_opcode_monitoring(int id)
{
  if(m_opcodeMonitorLog == 0)
  {
    QString section = "OpCodeMonitoring";
    QString opCodeList = pSEQPrefs->getPrefString("OpCodeList", section, "");
    bool ok = getMonitorOpCodeList("ShowEQ - Enable OpCode Monitor",
				   opCodeList);

    if (ok && !opCodeList.isEmpty())
    {
      createOPCodeMonitorLog(opCodeList);

      // set the list of monitored opcodes
      pSEQPrefs->setPrefString("OpCodeList", section, opCodeList);
      

      seqInfo("OpCode monitoring is now ENABLED...\nUsing list:\t%s", 
	     (const char*)opCodeList);
    }
  }
  else
  {
    delete m_opcodeMonitorLog;
    m_opcodeMonitorLog = 0;

    seqInfo("OpCode monitoring has been DISABLED...");
  }

  bool state = (m_opcodeMonitorLog != 0);
  menuBar()->setItemChecked(id, state);
  pSEQPrefs->setPrefBool("Enable", "OpCodeMonitoring", state);
}

void EQInterface::set_opcode_monitored_list(void)
{
  QString section = "OpCodeMonitoring";
  QString opCodeList = pSEQPrefs->getPrefString("OpCodeList", section, "");
  bool ok = getMonitorOpCodeList("ShowEQ - Reload OpCode Monitor", 
				 opCodeList);

  if (ok && m_opcodeMonitorLog)
  {
    m_opcodeMonitorLog->init(opCodeList);
    
    seqInfo("The monitored OpCode list has been reloaded...\nUsing list:\t%s", 
	   (const char*)opCodeList);
    
    // set the list of monitored opcodes
    pSEQPrefs->setPrefString("OpCodeList", section, opCodeList);
  }
}


void EQInterface::toggle_opcode_log(int id)
{
  QString section = "OpCodeMonitoring";
  bool state = !pSEQPrefs->getPrefBool("Log", section, false);

  if (m_opcodeMonitorLog)
  {
    m_opcodeMonitorLog->setLog(state);

    state = m_opcodeMonitorLog->log();
  }

  menuBar()->setItemChecked (id, state);
  pSEQPrefs->setPrefBool("Log", section, state);
}

void EQInterface::toggle_opcode_view(int id)
{
  QString section = "OpCodeMonitoring";
  bool state = !pSEQPrefs->getPrefBool("View", section, false);

  if (m_opcodeMonitorLog)
    m_opcodeMonitorLog->setView(state);

  menuBar()->setItemChecked (id, state);
  pSEQPrefs->setPrefBool("View", section, state);
}


void
EQInterface::select_opcode_file(void)
{
  QString section = "OpCodeMonitoring";

  QString logFile = pSEQPrefs->getPrefString("LogFilename",
					     section,
					     "opcodemonitor.log");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  logFile = 
    QFileDialog::getSaveFileName(logFileInfo.absFilePath(),
				 "*.log",
				 this,
				 "ShowEQ - OpCode Log File");

  // set log filename
  if (!logFile.isEmpty())
    pSEQPrefs->setPrefString("LogFilename", section, logFile);
}

void EQInterface::resetMaxMana(void)
{
  if (m_statList != 0)
    m_statList->statList()->resetMaxMana();
}

void
EQInterface::toggle_opt_LogSpawns (void)
{
  bool state = (m_spawnLogger == 0);

    if (state)
      createSpawnLog();
    else
    {
      // delete the spawn logger
      delete m_spawnLogger;

      // make sure to clear it's varialbe
      m_spawnLogger = 0;
    }

    menuBar()->setItemChecked (m_id_opt_LogSpawns, state);
    pSEQPrefs->setPrefBool("LogSpawns", "Misc", state);
}

void
EQInterface::toggle_opt_PvPTeams (void)
{
    showeq_params->pvp = !(showeq_params->pvp);
    menuBar()->setItemChecked (m_id_opt_PvPTeams, showeq_params->pvp);
    pSEQPrefs->setPrefBool("PvPTeamColoring", "Interface", showeq_params->pvp);
}

void
EQInterface::toggle_opt_PvPDeity (void)
{
    showeq_params->deitypvp = !(showeq_params->deitypvp);
    menuBar()->setItemChecked (m_id_opt_PvPDeity, showeq_params->deitypvp);
    pSEQPrefs->setPrefBool("DeityPvPTeamColoring", "Interface", showeq_params->deitypvp);
}

void
EQInterface::toggle_opt_CreateUnknownSpawns (int id)
{
    showeq_params->createUnknownSpawns = !showeq_params->createUnknownSpawns;
    menuBar()->setItemChecked(id, showeq_params->createUnknownSpawns);
    pSEQPrefs->setPrefBool("CreateUnknownSpawns", "Misc", showeq_params->createUnknownSpawns);
}

void
EQInterface::toggle_opt_WalkPathRecord (int id)
{
    showeq_params->walkpathrecord = !showeq_params->walkpathrecord;
    menuBar()->setItemChecked(id, showeq_params->walkpathrecord);
    pSEQPrefs->setPrefBool("WalkPathRecording", "Misc", showeq_params->walkpathrecord);
}

void
EQInterface::set_opt_WalkPathLength(int len)
{
  if ((len > 0) && (len <= 8192))
    showeq_params->walkpathlength = len;

    pSEQPrefs->setPrefInt("WalkPathLength", "Misc", showeq_params->walkpathlength);
}

void
EQInterface::toggle_opt_RetardedCoords (int id)
{
    showeq_params->retarded_coords = !showeq_params->retarded_coords;
    menuBar()->setItemChecked(id, showeq_params->retarded_coords);
    pSEQPrefs->setPrefBool("RetardedCoords", "Interface", showeq_params->retarded_coords);
}

void
EQInterface::toggle_opt_SystimeSpawntime (int id)
{
    showeq_params->systime_spawntime = !showeq_params->systime_spawntime;
    menuBar()->setItemChecked(id, showeq_params->systime_spawntime);
    pSEQPrefs->setPrefBool("SystimeSpawntime", "Interface", showeq_params->systime_spawntime);
}

void 
EQInterface::select_opt_conColorBase(int id)
{
  ColorLevel level = (ColorLevel)menuBar()->itemParameter(id);
  
  // get the current color
  QColor color = m_player->conColorBase(level);

  // get the new color
  QColor newColor = QColorDialog::getColor(color, this, "ShowEQ - Con Color");

  // only set if the user selected a valid color and clicked ok
  if (newColor.isValid())
  {
    // set the new con color
    m_player->setConColorBase(level, newColor);
    
    // force the spawn lists to get rebuilt with the new colors
    rebuildSpawnList();
  }
}

void EQInterface::setExp(uint32_t totalExp, uint32_t totalTick,
			  uint32_t minExpLevel, uint32_t maxExpLevel, 
			  uint32_t tickExpLevel)
{
  if (m_stsbarExp)
    m_stsbarExp->setText(QString("Exp: %1; %2 (%3/330); 1/330 = %4")
			 .arg(Commanate(totalExp))
			 .arg(Commanate(totalExp - minExpLevel))
			 .arg(totalTick).arg(tickExpLevel));
}

void EQInterface::newExp(uint32_t newExp, uint32_t totalExp, 
			 uint32_t totalTick,
			 uint32_t minExpLevel, uint32_t maxExpLevel, 
			 uint32_t tickExpLevel)
{
  uint32_t intoExp = totalExp - minExpLevel;
  uint32_t leftExp = maxExpLevel - totalExp;

  if (newExp)
  {
    uint32_t needKills = leftExp / newExp;
    // format a string for the status bar
    if (m_stsbarStatus)
      m_stsbarStatus->setText(QString("Exp: %1; %2 (%3/330); %4 left [~ %5 kills]")
			      .arg(Commanate(newExp))
			      .arg(Commanate(totalExp - minExpLevel))
			      .arg(totalTick)
			      .arg(Commanate(leftExp))
			      .arg(needKills));

    if (m_stsbarExp)
      m_stsbarExp->setText(QString("Exp: %1; %2 (%3/330) [%4]")
			   .arg(Commanate(totalExp))
			   .arg(Commanate(intoExp))
			   .arg(totalTick).arg(needKills));
  }
  else
  {
    if (m_stsbarStatus)
      m_stsbarStatus->setText(QString("Exp: <%1; %2 (%3/330); %4 left")
			      .arg(Commanate(tickExpLevel))
			      .arg(Commanate(totalExp - minExpLevel))
			      .arg(totalTick).arg(Commanate(leftExp)));

    if (m_stsbarExp)
      m_stsbarExp->setText(QString("Exp: %1; %2 (%3/330)")
			   .arg(Commanate(totalExp))
			   .arg(Commanate(intoExp))
			   .arg(totalTick));
  }
}

void EQInterface::setAltExp(uint32_t totalExp,
			    uint32_t maxExp, uint32_t tickExp, 
			    uint32_t aapoints)
{
  if (m_stsbarExpAA)
    m_stsbarExpAA->setText(QString("ExpAA: %1").arg(totalExp));
}

void EQInterface::newAltExp(uint32_t newExp, uint32_t totalExp, 
			    uint32_t totalTick, 
			    uint32_t maxExp, uint32_t tickExp, 
			    uint32_t aapoints)
{
  if (m_stsbarExpAA)
    m_stsbarExpAA->setText(QString("ExpAA: %1 (%2/330)")
			   .arg(Commanate(totalExp)).arg(totalTick));
}

void EQInterface::levelChanged(uint8_t level)
{
  QString tempStr;
  tempStr.sprintf("New Level: %u", level);
  if (m_stsbarStatus)
    m_stsbarStatus->setText(tempStr);
}

//
// TODO:  clear after timeout miliseconds
//
void
EQInterface::stsMessage(const QString &string, int timeout)
{
  if (m_stsbarStatus)
    m_stsbarStatus->setText(string);
}

void
EQInterface::numSpawns(int num)
{
  // only update once per sec
  static int lastupdate = 0;
  if ( (mTime() - lastupdate) < 1000)
    return;
  lastupdate = mTime();

   QString tempStr;
   tempStr.sprintf("Mobs: %d", num);
   m_stsbarSpawns->setText(tempStr);
}

void
EQInterface::newSpeed(int speed)
{
  // update twice per sec
  static int lastupdate = 0;
  if ( (mTime() - lastupdate) < 500)
    return;
  lastupdate = mTime();

   QString tempStr;
   tempStr.sprintf("Run Speed: %d", speed);
   m_stsbarSpeed->setText(tempStr);
}

void 
EQInterface::resetPacket(int num, int stream)
{
  if(stream != (int)zone2client);
  // if passed 0 reset the average
  m_packetStartTime = mTime();
  m_initialcount = num;
}

void
EQInterface::numPacket(int num, int stream)
{

  if(stream != (int)zone2client)
    return;
  // start the timer of not started
  if (!m_packetStartTime)
    m_packetStartTime = mTime();

  // only update once per sec
  static int lastupdate = 0;
  if ( (mTime() - lastupdate) < 1000)
    return;
  lastupdate = mTime();
  

   QString tempStr;
   int delta = mTime() - m_packetStartTime;
   num -= m_initialcount;
   if (num && delta)
     tempStr.sprintf("Pkt: %d (%2.1f)", num, (float) (num<<10) / (float) delta);
   else   
     tempStr.sprintf("Pkt: %d", num);

   m_stsbarPkt->setText(tempStr);
}

void EQInterface::attack2Hand1(const uint8_t* data)
{
  // const attack2Struct * atk2 = (const attack2Struct*)data;
}

void EQInterface::action2Message(const uint8_t* data)
{
  action2Struct *action2 = (action2Struct*)data;
  const Item* target = m_spawnShell->findID(tSpawn, action2->target);
  const Item* source = m_spawnShell->findID(tSpawn, action2->source);
  emit combatSignal(action2->target, action2->source, action2->type, action2->spell, action2->damage, 
		    (target != 0) ? target->name() : QString("Unknown"), (source != 0) ? source->name() : QString("Unknown"));
}

// belith - combatKillSpawn, fix for the combat window
//          this displays a killing shot on a mob in combat records
void EQInterface::combatKillSpawn(const uint8_t* data)
{
  const newCorpseStruct *deadspawn = (const newCorpseStruct *)data;
  // only show my kills
  if (deadspawn && deadspawn->killerId == m_player->id())
  {
    const Item* target = m_spawnShell->findID(tSpawn, deadspawn->spawnId);
    const Item* source = m_spawnShell->findID(tSpawn, deadspawn->killerId);
    emit combatSignal(deadspawn->spawnId, deadspawn->killerId,
		      (deadspawn->type == -25) ? 231 : deadspawn->type,
		      deadspawn->spellId, deadspawn->damage,
		      (target != 0) ? target->name() : QString("Unknown"),
		      (source != 0) ? source->name() : QString("Unknown"));
  }
}

void EQInterface::updatedDateTime(const QDateTime& dt)
{
  
  m_stsbarEQTime->setText(dt.toString(pSEQPrefs->getPrefString("DateTimeFormat", "Interface", "ddd MMM dd,yyyy - hh:mm ap")));
}

void EQInterface::syncDateTime(const QDateTime& dt)
{
  QString dateString = dt.toString(pSEQPrefs->getPrefString("DateTimeFormat", "Interface", "ddd MMM dd,yyyy - hh:mm ap"));

  m_stsbarEQTime->setText(dateString);
}

void EQInterface::zoneBegin(const QString& shortZoneName)
{
  emit newZoneName(shortZoneName);
  float percentZEM = ((float)(m_zoneMgr->zoneExpMultiplier()-0.75)/0.75)*100;
  QString tempStr;
  tempStr.sprintf("ZEM: %3.2f%%", percentZEM);
  if (m_stsbarZEM)
    m_stsbarZEM->setText(tempStr);
}

void EQInterface::zoneEnd(const QString& shortZoneName, 
			  const QString& longZoneName)
{
  emit newZoneName(longZoneName);
  stsMessage("");
  float percentZEM = ((float)(m_zoneMgr->zoneExpMultiplier()-0.75)/0.75)*100;
  QString tempStr;
  tempStr.sprintf("ZEM: %3.2f%%", percentZEM);
  if (m_stsbarZEM)
    m_stsbarZEM->setText(tempStr);
}

void EQInterface::zoneChanged(const QString& shortZoneName)
{
  QString tempStr;
  stsMessage("- Busy Zoning -");
  emit newZoneName(shortZoneName);
  float percentZEM = ((float)(m_zoneMgr->zoneExpMultiplier()-0.75)/0.75)*100;
  tempStr.sprintf("ZEM: %3.2f%%", percentZEM);
  if (m_stsbarZEM)
    m_stsbarZEM->setText(tempStr);
}

void EQInterface::clientTarget(const uint8_t* data)
{
  if (!m_selectOnTarget)
    return;

  const clientTargetStruct* cts = (const clientTargetStruct*)data;

  // try to find the targeted spawn in the spawn shell
  const Item* item = m_spawnShell->findID(tSpawn, cts->newTarget);

  // if found, make it the currently selected target
  if (item)
  {
    // note the new selection
    m_selectedSpawn = item;
    
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);

    // update the spawn status
    updateSelectedSpawnStatus(m_selectedSpawn);
  }
}

void EQInterface::spawnSelected(const Item* item)
{
  if (item == 0)
    return;

  // note the new selection
  m_selectedSpawn = item;
  
  // notify others of the new selected spawn
  emit selectSpawn(m_selectedSpawn);

  // update the spawn status
  updateSelectedSpawnStatus(m_selectedSpawn);
}

void EQInterface::spawnConsidered(const Item* item)
{
  if (item == 0)
    return;

  if (!m_selectOnConsider)
    return;

  // note the new selection
  m_selectedSpawn = item;
  
  // notify others of the new selected spawn
  emit selectSpawn(m_selectedSpawn);
  
  // update the spawn status
  updateSelectedSpawnStatus(m_selectedSpawn);
}

void EQInterface::addItem(const Item* item)
{
  uint32_t filterFlags = item->filterFlags();

  if (filterFlags & FILTER_FLAG_LOCATE)
  {
    // note the new selection
    m_selectedSpawn = item;
    
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);
    
    // update the spawn status
    updateSelectedSpawnStatus(m_selectedSpawn);
  } // End LOCATE Filter alerting
}


void EQInterface::delItem(const Item* item)
{
  // if this is the selected spawn, then there isn't a selected spawn anymore
  if (m_selectedSpawn == item)
  {
    m_selectedSpawn = 0;
  
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);
  }
}

void EQInterface::killSpawn(const Item* item)
{
  if (m_selectedSpawn != item)
    return;

  // update status message, notifying that selected spawn has died
  QString string = m_selectedSpawn->name() + " died";

  stsMessage(string);
}

void EQInterface::changeItem(const Item* item)
{
  // if this isn't the selected spawn, nothing more to do
  if (item != m_selectedSpawn)
    return;

  updateSelectedSpawnStatus(item);
}

void EQInterface::updateSelectedSpawnStatus(const Item* item)
{
  if (item == 0)
    return;

  const Spawn* spawn = 0;

  if ((item->type() == tSpawn) || (item->type() == tPlayer))
    spawn = (const Spawn*)item;

  // construct a message for the status message display
  QString string("");
  if (spawn != 0)
    string.sprintf("%d: %s:%d (%d/%d) Pos:", // "%d/%d/%d (%d) %s %s Item:%s", 
		   item->id(),
		   (const char*)item->name().utf8(),
		   spawn->level(), spawn->HP(),
		   spawn->maxHP());
  else
    string.sprintf("%d: %s: Pos:", // "%d/%d/%d (%d) %s %s Item:%s", 
		   item->id(),
		   (const char*)item->name().utf8());

  if (showeq_params->retarded_coords)
    string += QString::number(item->y()) + "/" 
      + QString::number(item->x()) + "/" 
      + QString::number(item->z());
  else
    string += QString::number(item->x()) + "/" 
      + QString::number(item->y()) + "/" 
      + QString::number(item->z());

  string += QString(" (") 
    + QString::number(item->calcDist(m_player->x(),
				     m_player->y(),
				     m_player->z()))
    + ") " + item->raceString() + " " + item->classString();

  // just call the status message method
  stsMessage(string);
}

void EQInterface::addCategory(void)
{
  if (m_categoryMgr)
    m_categoryMgr->addCategory();
}

void EQInterface::reloadCategories(void)
{
  if (m_categoryMgr)
    m_categoryMgr->reloadCategories();
}

void EQInterface::rebuildSpawnList()
{
  if (m_spawnList)
    m_spawnList->spawnList()->rebuildSpawnList();

  if (m_spawnList2)
    m_spawnList2->rebuildSpawnList();
}

void EQInterface::selectNext(void)
{
  if (m_spawnList)
    m_spawnList->spawnList()->selectNext();
}

void EQInterface::selectPrev(void)
{
  if (m_spawnList)
    m_spawnList->spawnList()->selectPrev();
}

void EQInterface::saveSelectedSpawnPath(void)
{
  QString fileName;
  fileName.sprintf("%s_mobpath.map", 
		   (const char*)m_zoneMgr->shortZoneName());

  QFileInfo fileInfo = m_dataLocationMgr->findWriteFile("maps", fileName, 
							false);

  QFile mobPathFile(fileInfo.absFilePath());
  if (mobPathFile.open(IO_Append | IO_WriteOnly))
  {
    QTextStream out(&mobPathFile);
    // append the selected spawns path to the end
    saveSpawnPath(out, m_selectedSpawn);

    seqInfo("Finished appending '%s'!\n", (const char*)fileName);
  }
}

void EQInterface::saveSpawnPaths(void)
{
  QString fileName;
  fileName.sprintf("%s_mobpath.map", 
		   (const char*)m_zoneMgr->shortZoneName());

  QFileInfo fileInfo = m_dataLocationMgr->findWriteFile("maps", fileName, 
							false);

  QFile mobPathFile(fileInfo.absFilePath());
  if (mobPathFile.open(IO_Truncate | IO_WriteOnly))
  {
    QTextStream out(&mobPathFile);
    // map header line
    out << m_zoneMgr->longZoneName() << ","
	<< m_zoneMgr->shortZoneName() << ",0,0" << endl;

    // iterate over the spawns adding their paths to the file
    ItemConstIterator it(m_spawnShell->getConstMap(tSpawn));
    const Item* item;
    for (item = it.current(); item != 0; item = ++it)
    {
      if ((item->NPC() == SPAWN_NPC) || 
	  (item->NPC() == SPAWN_NPC_CORPSE) ||
	  (item->NPC() == SPAWN_NPC_UNKNOWN))
	saveSpawnPath(out, it.current());
    }

    seqInfo("Finished writing '%s'!\n", (const char*)fileName);
  }
}

void EQInterface::saveSpawnPath(QTextStream& out, const Item* item)
{
  if (item == 0)
    return;

  const Spawn* spawn = spawnType(item);

  if (spawn == 0)
    return;

   const SpawnTrackList& trackList = spawn->trackList();
   SpawnTrackListIterator trackIt(spawn->trackList());
   int cnt = trackList.count();

   // only make a line if there is more then one point
   if (cnt < 2)
     return;

   const SpawnTrackPoint* trackPoint;
   
   out << "M," << spawn->realName() << ",blue," << trackList.count();
   //iterate over the track, writing out the points
   for (trackPoint = trackIt.current();
	trackPoint;
	trackPoint = ++trackIt)
     {
       out << "," << trackPoint->x() 
	   << "," <<  trackPoint->y()
	   << "," << trackPoint->z();
     }
   out << endl;
}

void EQInterface::toggle_net_real_time_thread(int id)
{
  bool realtime = !m_packet->realtime();
  m_packet->setRealtime(realtime);
   m_netMenu->setItemChecked(id, realtime);
   pSEQPrefs->setPrefBool("RealTimeThread", "Network", realtime);
}

void EQInterface::set_net_monitor_next_client()
{
  // start monitoring the next client seen
  m_packet->monitorNextClient();

  // set it as the address to monitor next session
  pSEQPrefs->setPrefString("IP", "Network", m_packet->ip());
}

void EQInterface::set_net_client_IP_address()
{
  QStringList iplst;
  for( int l = 0; l < 5; l++)
  iplst += ipstr[l];    
  bool ok = false;
  QString address = 
     QInputDialog::getItem("ShowEQ - EQ Client IP Address",
      			   "Enter IP address of EQ client",
			    iplst, 0, TRUE, &ok, this );
  if (ok)
  {
	for (int i = 4; i > 0; i--)
	ipstr[i] = ipstr[ i - 1 ];
	ipstr[0] = address;
    // start monitoring the new address
    m_packet->monitorIPClient(address);

    // set it as the address to monitor next session
    pSEQPrefs->setPrefString("IP", "Network", m_packet->ip());
  }
}

void EQInterface::set_net_client_MAC_address()
{
  QStringList maclst;
  for( int l = 0; l < 5; l++)  
  maclst += macstr[l];
  bool ok = false;
  QString address = 
     QInputDialog::getItem("ShowEQ - EQ Client MAC Address",
     			   "Enter MAC address of EQ client",
			    maclst, 0, TRUE, &ok, this );
  if (ok)
  {
    if (address.length() != 17)
    {
      seqWarn("Invalid MAC Address (%s)! Ignoring!",
	      (const char*)address);
      return;
    }
 	for (int i = 4; i > 0; i--)
	macstr[i] = macstr[ i - 1 ];
	macstr[0] = address;
    // start monitoring the new address
    m_packet->monitorMACClient(address);

    // set it as the address to monitor next session
    pSEQPrefs->setPrefString("MAC", "Network", m_packet->mac());
  }
}

void EQInterface::set_net_device()
{
  bool ok = false;
  QString dev = 
    QInputDialog::getText("ShowEQ - Device",
			  "Enter the device to sniff for EQ Packets:",
			  QLineEdit::Normal, m_packet->device(),
			  &ok, this);

  if (ok)
  {
    // start monitoring the device
    m_packet->monitorDevice(dev);

    // set it as the device to monitor next session
    pSEQPrefs->setPrefString("Device", "Network", m_packet->device());
  }
}

void EQInterface::set_net_arq_giveup(int giveup)
{
  // set the Arq Seq Give Up length
  m_packet->setArqSeqGiveUp(uint16_t(giveup));

  // set it as the value to use next session
  pSEQPrefs->setPrefInt("ArqSeqGiveUp", "Network", m_packet->arqSeqGiveUp());
}

void EQInterface::toggle_net_session_tracking()
{
  bool enable = !m_packet->session_tracking();

  m_packet->session_tracking(enable);
  m_netMenu->setItemChecked(m_id_net_sessiontrack, enable);
  pSEQPrefs->setPrefBool("SessionTracking", "Network", enable);
}

void EQInterface::toggleAutoDetectPlayerSettings (int id)
{
  m_player->setUseAutoDetectedSettings(!m_player->useAutoDetectedSettings());
  menuBar()->setItemChecked (id, m_player->useAutoDetectedSettings());
}

/* Choose the character's level */
void EQInterface::SetDefaultCharacterLevel(int level)
{
  m_player->setDefaultLevel(level);
}

/* Choose the character's class */
void EQInterface::SetDefaultCharacterClass(int id)
{
   for (int i = 0; i < PLAYER_CLASSES; i++)
       m_charClassMenu->setItemChecked(char_ClassID[i], char_ClassID[i] == id);
   m_player->setDefaultClass(m_charClassMenu->itemParameter(id));
}

/* Choose the character's race */
void EQInterface::SetDefaultCharacterRace(int id)
{   
   for (int i = 0; i < PLAYER_RACES; i++)
     m_charRaceMenu->setItemChecked(char_RaceID[i], char_RaceID[i] == id);
   m_player->setDefaultRace(m_charRaceMenu->itemParameter(id));
}

void EQInterface::toggle_view_menubar()
{
   if (menuBar()->isVisible())
       menuBar()->hide();
   else
       menuBar()->show();
}

void EQInterface::toggle_view_statusbar()
{
   if (statusBar()->isVisible())
       statusBar()->hide();
   else
       statusBar()->show();
   pSEQPrefs->setPrefBool("StatusBarActive", "Interface_StatusBar", statusBar()->isVisible());
}

void EQInterface::init_view_menu()
{
  // need to check for 0 before checking if is visible for dynamicly
  // created windows
  menuBar()->setItemChecked(m_id_view_PlayerSkills, 
			    (m_skillList != 0) && 
			    m_skillList->isVisible());
  menuBar()->setItemChecked(m_id_view_PlayerStats, 
			    (m_statList != 0) && 
			    m_statList->isVisible());
  menuBar()->setItemChecked(m_id_view_SpawnList, 
			    (m_spawnList != 0) && 
			    m_spawnList->isVisible());
  menuBar()->setItemChecked(m_id_view_SpawnList2, 
			    (m_spawnList2 != 0) && 
			    m_spawnList2->isVisible());
  menuBar()->setItemChecked(m_id_view_SpawnPointList, 
			    (m_spawnPointList != 0) &&
			    m_spawnPointList->isVisible());
  menuBar()->setItemChecked(m_id_view_Compass, 
			    (m_compass != 0) &&
			    m_compass->isVisible());
  menuBar()->setItemChecked(m_id_view_NetDiag, 
			    (m_netDiag != 0) &&
			    m_netDiag->isVisible());
  menuBar()->setItemChecked(m_id_view_GuildListWindow, 
			    (m_guildListWindow != 0) &&
			    m_guildListWindow->isVisible());
  menuBar()->setItemChecked (m_id_view_SpellList, 
			     (m_spellList != 0) &&
			     m_spellList->isVisible());

  // loop over the maps
  for (int i = 0; i < maxNumMaps; i++)
    menuBar()->setItemChecked(m_id_view_Map[i], 
			      (m_map[i] != 0) &&
			      m_map[i]->isVisible());

  // loop over the message windows
  for (int i = 0; i < maxNumMessageWindows; i++)
    menuBar()->setItemChecked(m_id_view_MessageWindow[i], 
			      (m_messageWindow[i] != 0) &&
			      m_messageWindow[i]->isVisible());

  // set the checkmarks for windows that are always created, but not always
  // visible
  menuBar()->setItemChecked(m_id_view_ExpWindow, 
			    (m_expWindow != 0) && 
			    m_expWindow->isVisible()); 
  menuBar()->setItemChecked (m_id_view_CombatWindow, 
			     (m_combatWindow != 0) &&
			     m_combatWindow->isVisible());
   
   // set initial view options
  if (m_spawnList != 0)
  {
    SEQListView* spawnList = m_spawnList->spawnList();

    // make sure the menu bar settings are correct
    for (int i = 0; i < tSpawnColMaxCols; i++)
      m_spawnListMenu->setItemChecked(m_id_view_SpawnList_Cols[i], 
				      spawnList->columnVisible(i));
  }

  if (m_statList != 0)
  {
    StatList* statList = m_statList->statList();
    // make sure the menu items are checked
    for (int i = 0; i < LIST_MAXLIST; i++)
      m_statWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[i], 
				    statList->statShown(i));

  }

  if (m_skillList != 0)
  {
    // make sure the proper menu items are checked
    menuBar()->setItemChecked(m_id_view_PlayerSkills_Languages, 
			      m_skillList->skillList()->showLanguages());

  }
}

void EQInterface::toggle_opt_save_PlayerState(int id)
{
  showeq_params->savePlayerState = !showeq_params->savePlayerState;
  menuBar()->setItemChecked(id, showeq_params->savePlayerState);
  pSEQPrefs->setPrefBool("PlayerState", "SaveState", 
			 showeq_params->savePlayerState);
}

void EQInterface::toggle_opt_save_ZoneState(int id)
{
  showeq_params->saveZoneState = !showeq_params->saveZoneState;
  menuBar()->setItemChecked(id, showeq_params->saveZoneState);
  pSEQPrefs->setPrefBool("ZoneState", "SaveState", 
			 showeq_params->saveZoneState);
}

void EQInterface::toggle_opt_save_Spawns(int id)
{
  showeq_params->saveSpawns = !showeq_params->saveSpawns;
  menuBar()->setItemChecked(id, showeq_params->saveSpawns);
  pSEQPrefs->setPrefBool("Spawns", "SaveState", 
			 showeq_params->saveSpawns);

  if (showeq_params->saveSpawns)
    m_spawnShell->saveSpawns();
}

void EQInterface::set_opt_save_SpawnFrequency(int frequency)
{
  showeq_params->saveSpawnsFrequency = frequency * 1000;
  pSEQPrefs->setPrefInt("SpawnsFrequency", "SaveState", 
			showeq_params->saveSpawnsFrequency);
}

void EQInterface::set_opt_save_BaseFilename()
{
  QString fileName = 
    QFileDialog::getSaveFileName(showeq_params->saveRestoreBaseFilename, 
				 QString::null, this, "SaveBaseFilename",
				 "Save State Base Filename");
  if (!fileName.isEmpty())
  {
    // set it to be the new base filename
    showeq_params->saveRestoreBaseFilename = fileName;
    
    // set preference to use for next session
    pSEQPrefs->setPrefString("BaseFilename", "SaveState", 
			     showeq_params->saveRestoreBaseFilename);
  }
}

void EQInterface::opt_clearChannelMsgs(int id)
{
  // clear the messages
  m_messages->clear();
}


void EQInterface::showMessageFilterDialog(void)
{
  // create the filter dialog, if necessary
  if (!m_messageFilterDialog)
    m_messageFilterDialog = new MessageFilterDialog(m_messageFilters, 
						    "ShowEQ Message Filters",
						    this, "messagefilterdialog");

  // show the message filter dialog
  m_messageFilterDialog->show();
}

void EQInterface::toggleTypeFilter(int id)
{
  uint64_t enabledTypes = m_terminal->enabledTypes();

  if (((uint64_t(1) << id) & enabledTypes) != 0)
    enabledTypes &= ~(uint64_t(1) << id);
  else
    enabledTypes |= (uint64_t(1) << id);

  m_terminal->setEnabledTypes(enabledTypes);

  // (un)check the appropriate menu item
  m_terminalTypeFilterMenu->setItemChecked(id, ((enabledTypes & (uint64_t(1) << id))));
}

void EQInterface::disableAllTypeFilters()
{
  m_terminal->setEnabledTypes(0);
  // make sure the All menu items are unchecked
  m_terminalTypeFilterMenu->setItemChecked(64, false);
  m_terminalTypeFilterMenu->setItemChecked(65, false);

  // uncheck all the menu items
  QString typeName;
  for (int i = MT_Guild; i <= MT_Max; i++)
  {
    typeName = MessageEntry::messageTypeString((MessageType)i);
    if (!typeName.isEmpty())
      m_terminalTypeFilterMenu->setItemChecked(i, false);
  }
}

void EQInterface::enableAllTypeFilters()
{
  m_terminal->setEnabledTypes(0xFFFFFFFFFFFFFFFFULL);

  // make sure the All menu items are unchecked
  m_terminalTypeFilterMenu->setItemChecked(64, false);
  m_terminalTypeFilterMenu->setItemChecked(65, false);

  // check all the menu items
  QString typeName;
  for (int i = MT_Guild; i <= MT_Max; i++)
  {
    typeName = MessageEntry::messageTypeString((MessageType)i);
    if (!typeName.isEmpty())
      m_terminalTypeFilterMenu->setItemChecked(i, true);
  }
}

void EQInterface::toggleShowUserFilter(int id)
{
  uint32_t enabledShowUserFilters = m_terminal->enabledShowUserFilters();
  // toggle whether the filter is enabled/disabled
  if (((1 << id) & enabledShowUserFilters) != 0)
    enabledShowUserFilters &= ~(1 << id);
  else
    enabledShowUserFilters |= (1 << id);

  m_terminal->setEnabledShowUserFilters(enabledShowUserFilters);
 
  // (un)check the appropriate menu item
  m_terminalShowUserFilterMenu->setItemChecked(id, ((enabledShowUserFilters & (1 << id)) != 0));
}

void EQInterface::disableAllShowUserFilters()
{
  // set and save all filters disabled setting
  m_terminal->setEnabledShowUserFilters(0);
  
  // make sure the All menu items are unchecked
  m_terminalShowUserFilterMenu->setItemChecked(66, false);
  m_terminalShowUserFilterMenu->setItemChecked(67, false);

  // uncheck all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_terminalShowUserFilterMenu->setItemChecked(i, false);
  }
}

void EQInterface::enableAllShowUserFilters()
{
  // set and save all filters enabled flag
  m_terminal->setEnabledShowUserFilters(0xFFFFFFFF);

  // make sure the All menu items are unchecked
  m_terminalShowUserFilterMenu->setItemChecked(66, false);
  m_terminalShowUserFilterMenu->setItemChecked(67, false);

  // check all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_terminalShowUserFilterMenu->setItemChecked(i, true);
  }
}

void EQInterface::toggleHideUserFilter(int id)
{
  uint32_t enabledHideUserFilters = m_terminal->enabledHideUserFilters();

  // toggle whether the filter is enabled/disabled
  if (((1 << id) & enabledHideUserFilters) != 0)
    enabledHideUserFilters &= ~(1 << id);
  else
    enabledHideUserFilters |= (1 << id);

  m_terminal->setEnabledHideUserFilters(enabledHideUserFilters);

  // (un)check the appropriate menu item
  m_terminalHideUserFilterMenu->setItemChecked(id, ((enabledHideUserFilters & (1 << id)) != 0));
}

void EQInterface::disableAllHideUserFilters()
{
  // set and save all filters disabled setting
  m_terminal->setEnabledHideUserFilters(0);
  
  // make sure the All menu items are unchecked
  m_terminalHideUserFilterMenu->setItemChecked(66, false);
  m_terminalHideUserFilterMenu->setItemChecked(67, false);

  // uncheck all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_terminalHideUserFilterMenu->setItemChecked(i, false);
  }
}

void EQInterface::enableAllHideUserFilters()
{
  // set and save all filters enabled flag
  m_terminal->setEnabledHideUserFilters(0xFFFFFFFF);

  // make sure the All menu items are unchecked
  m_terminalHideUserFilterMenu->setItemChecked(66, false);
  m_terminalHideUserFilterMenu->setItemChecked(67, false);

  // check all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_terminalHideUserFilterMenu->setItemChecked(i, true);
  }
}

void EQInterface::toggleDisplayType(int id)
{
  // toggle the display of message types
  m_terminal->setDisplayType(m_terminal->displayType());
  m_terminalMenu->setItemChecked(id, m_terminal->displayType());
}

void EQInterface::toggleDisplayTime(int id)
{
  // toggle the display of message time
  m_terminal->setDisplayDateTime(!m_terminal->displayDateTime());
  m_terminalMenu->setItemChecked(id, m_terminal->displayDateTime());  
}

void EQInterface::toggleEQDisplayTime(int id)
{
  m_terminal->setDisplayEQDateTime(!m_terminal->displayEQDateTime());
  m_terminalMenu->setItemChecked(id, m_terminal->displayEQDateTime());
}

void EQInterface::toggleUseColor(int id)
{
  m_terminal->setUseColor(!m_terminal->useColor());
  m_terminalMenu->setItemChecked(id, m_terminal->useColor());
}

int EQInterface::setTheme(int id)
{
    static QFont OrigFont = qApp->font();
    static QPalette OrigPalette = qApp->palette();;

    MenuIDList::Iterator iter;
    for ( iter = IDList_StyleMenu.begin(); iter != IDList_StyleMenu.end(); ++iter)
      menuBar()->setItemChecked( (*iter), false );

    menuBar()->setItemChecked( id, true );
    int theme = menuBar()->itemParameter(id);

    switch ( theme )
    {
    case 1: // platinum
    {
      QPalette p( QColor( 239, 239, 239 ) );
#if QT_VERSION >= 0x030000
      qApp->setStyle("platinum");
#else
      qApp->setStyle( (QStyle *) new QPlatinumStyle );
#endif
      qApp->setPalette( p, TRUE );
    }
    break;
    case 2: // windows
    {
#if QT_VERSION >= 0x030000
      qApp->setStyle("windows");
#else
      qApp->setStyle( (QStyle *) new QWindowsStyle );
#endif
      qApp->setFont( OrigFont, TRUE );
      qApp->setPalette( OrigPalette, TRUE );
    }
    break;
    case 3: // cde 
    case 4: // cde polished
    {
      QPalette p( QColor( 75, 123, 130 ) );
#if QT_VERSION >= 0x030000
      qApp->setStyle("cde");
#else
      qApp->setStyle( (QStyle *) new QCDEStyle( theme == 3 ? TRUE : FALSE ) );
#endif
      p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
      p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
      p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
      p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
      p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
      p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
      p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
      p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );
      qApp->setPalette( p, TRUE );
      qApp->setFont( QFont( "times", OrigFont.pointSize() ), TRUE );
    }
    break;
    case 5: // motif
    {
      QPalette p( QColor( 192, 192, 192 ) );
#if QT_VERSION >= 0x030000
      qApp->setStyle("motif");
#else
      qApp->setStyle( (QStyle *) new QMotifStyle );
#endif
      qApp->setPalette( p, TRUE );
      qApp->setFont( OrigFont, TRUE );
    }
    break;
    case 6: // SGI
    {
      //QPalette p( QColor( 192, 192, 192 ) );
#if QT_VERSION >= 0x030000
      qApp->setStyle("sgi");
#else
      qApp->setStyle( (QStyle *) new QSGIStyle( FALSE ) );
#endif
      qApp->setPalette( OrigPalette, TRUE );
      qApp->setFont( OrigFont, TRUE );
    }
    break;
    default: // system default
    {
      QPalette p( QColor( 192, 192, 192 ) );
#if QT_VERSION >= 0x030000
      qApp->setStyle("motif");
#else
      qApp->setStyle( (QStyle *) new QMotifStyle );
#endif
      qApp->setPalette( p, TRUE );
      qApp->setFont( OrigFont, TRUE );
      theme = 2;
    }
    break;
    }

    // make sure the windows that override the application font, do so
    emit restoreFonts();

    return theme;
}

void EQInterface::selectTheme( int id )
{
  int theme = setTheme(id);
  pSEQPrefs->setPrefInt("Theme", "Interface", theme);
}

void EQInterface::showMap(int i)
{
  if ((i > maxNumMaps) || (i < 0))
    return;

  // if it doesn't exist, create it
  if (m_map[i] == 0)
  {
    int mapNum = i + 1;
    QString mapPrefName = "Map";
    QString mapName = QString("map") + QString::number(mapNum);
    QString mapCaption = "Map ";

    if (i != 0)
    {
      mapPrefName += QString::number(mapNum);
      mapCaption += QString::number(mapNum);
    }

    m_map[i] = new MapFrame(m_filterMgr,
			    m_mapMgr,
			    m_player, 
			    m_spawnShell, 
			    m_zoneMgr,
			    m_spawnMonitor,
			    mapPrefName, 
			    mapCaption,
			    mapName, 
			    0);


   setDockEnabled(m_map[i], 
		  pSEQPrefs->getPrefBool(QString("Dockable") + mapPrefName,
					 "Interface", true));

    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", m_map[i]->preferenceName(),
					    Left);
    addDockWindow(m_map[i], mapName, edge, true);
    if (!m_isMapDocked[i])
      m_map[i]->undock();

    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_map[i], SLOT(savePrefs()));
    connect(this, SIGNAL(restoreFonts(void)),
	    m_map[i], SLOT(restoreFont(void)));
    
    // Get the map...
    Map* map = m_map[i]->map();
    
    // supply the Map slots with signals from EQInterface
    connect (this, SIGNAL(selectSpawn(const Item*)), 
	     map, SLOT(selectSpawn(const Item*)));
    
    // supply EQInterface slots with signals from Map
    connect (map, SIGNAL(spawnSelected(const Item*)),
	     this, SLOT(spawnSelected(const Item*)));

    m_map[i]->restoreSize();

    // restore it's position if necessary and practical
    if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", true))
      m_map[i]->restorePosition();

    // insert its menu into the window menu
    insertWindowMenu(m_map[i]);
  }
      
  // make sure it's visible
  m_map[i]->show();
}

void EQInterface::showMessageWindow(int i)
{
  if ((i > maxNumMessageWindows) || (i < 0))
    return;

  // if it doesn't exist, create it
  if (m_messageWindow[i] == 0)
  {
    int winNum = i + 1;
    QString prefName = "MessageWindow" + QString::number(winNum);
    QString name = QString("messageWindow") + QString::number(winNum);
    QString caption = "Channel Messages ";

    if (i != 0)
      caption += QString::number(winNum);

    m_messageWindow[i] = new MessageWindow(m_messages, m_messageFilters,
					   prefName, caption,
					   0, name);

   setDockEnabled(m_messageWindow[i], 
		  pSEQPrefs->getPrefBool(QString("Dockable") + prefName,
					 "Interface", false));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_messageWindow[i]->preferenceName(),
					    Left);
    addDockWindow(m_messageWindow[i], edge, false);
    if (!m_isMessageWindowDocked[i])
      m_messageWindow[i]->undock();

    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_messageWindow[i], SLOT(savePrefs(void)));
    connect(this, SIGNAL(restoreFonts(void)),
	    m_messageWindow[i], SLOT(restoreFont(void)));
    
    m_messageWindow[i]->restoreSize();

    // restore it's position if necessary and practical
    if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", true))
      m_messageWindow[i]->restorePosition();

    // insert its menu into the window menu
    insertWindowMenu(m_messageWindow[i]);
  }
      
  // make sure it's visible
  m_messageWindow[i]->show();
}

void EQInterface::showSpawnList(void)
{
  // if it doesn't exist, create it.
  if (m_spawnList == 0)
  {
    m_spawnList = new SpawnListWindow (m_player, m_spawnShell, m_categoryMgr,
				       0, "spawnlist");
    setDockEnabled(m_spawnList, 
		   pSEQPrefs->getPrefBool("DockableSpawnList",
					  "Interface", true));

    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_spawnList->preferenceName(),
					    Left);
    addDockWindow(m_spawnList, edge, false);
    if (m_isSpawnListDocked)
      m_spawnList->undock();

    // restore the size of the spawn list
    m_spawnList->restoreSize();

    // only do this move stuff iff the spawn list isn't docked
    // and the user set the option to do so.
    if (!m_isSpawnListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
      m_spawnList->restorePosition();

     // connections from spawn list to interface
     connect (m_spawnList->spawnList(), SIGNAL(spawnSelected(const Item*)),
	      this, SLOT(spawnSelected(const Item*)));

     // connections from interface to spawn list
     connect (this, SIGNAL(selectSpawn(const Item*)),
	      m_spawnList->spawnList(), SLOT(selectSpawn(const Item*)));
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_spawnList, SLOT(savePrefs(void)));
     connect(this, SIGNAL(restoreFonts(void)),
	     m_spawnList, SLOT(restoreFont(void)));

    // insert its menu into the window menu
    insertWindowMenu(m_spawnList);
  }

  // make sure it's visible
  m_spawnList->show();
}

void EQInterface::showSpawnList2(void)
{
  // if it doesn't exist, create it.
  if (m_spawnList2 == 0)
  {
    m_spawnList2 = new SpawnListWindow2(m_player, m_spawnShell, 
					m_categoryMgr,
					0, "spawnlist");
   setDockEnabled(m_spawnList2, 
		  pSEQPrefs->getPrefBool("DockableSpawnList2",
					 "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_spawnList2->preferenceName(),
					    Left);
    addDockWindow(m_spawnList2, edge, false);
    if (!m_isSpawnList2Docked)
      m_spawnList2->undock();

    // restore the size of the spawn list
    m_spawnList2->restoreSize();

    // only do this move stuff iff the spawn list isn't docked
    // and the user set the option to do so.
    if (!m_isSpawnList2Docked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
      m_spawnList2->restorePosition();

     // connections from spawn list to interface
     connect (m_spawnList2, SIGNAL(spawnSelected(const Item*)),
	      this, SLOT(spawnSelected(const Item*)));

     // connections from interface to spawn list
     connect (this, SIGNAL(selectSpawn(const Item*)),
	      m_spawnList2, SLOT(selectSpawn(const Item*)));
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_spawnList2, SLOT(savePrefs(void)));
     connect(this, SIGNAL(restoreFonts(void)),
	     m_spawnList2, SLOT(restoreFont(void)));

    // insert its menu into the window menu
    insertWindowMenu(m_spawnList2);
  }

  // make sure it's visible
  m_spawnList2->show();
}

void EQInterface::showSpawnPointList(void)
{
  // if it doesn't exist, create it.
  if (m_spawnPointList == 0)
  {
      m_spawnPointList = new SpawnPointWindow(m_spawnMonitor,
					      0, "spawnlist");
      setDockEnabled(m_spawnPointList, 
		     pSEQPrefs->getPrefBool("DockableSpawnPointList",
					    "Interface", true));
      Dock edge = 
	(Dock)pSEQPrefs->getPrefInt("Dock", 
				    m_spawnPointList->preferenceName(),
				    Left);
      addDockWindow(m_spawnPointList, edge, false);
      if (!m_isSpawnPointListDocked)
	m_spawnPointList->undock();

    // restore the size of the spawn list
    m_spawnPointList->restoreSize();

    // only do this move stuff iff the spawn list isn't docked
    // and the user set the option to do so.
    if (!m_isSpawnPointListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
      m_spawnPointList->restorePosition();

     // connections from interface to spawn list
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_spawnPointList, SLOT(savePrefs(void)));
     connect(this, SIGNAL(restoreFonts(void)),
	     m_spawnPointList, SLOT(restoreFont(void)));

    // insert its menu into the window menu
    insertWindowMenu(m_spawnPointList);
  }

  // make sure it's visible
  m_spawnPointList->show();
}

void EQInterface::showStatList(void)
{
  // if it doesn't exist, create it
  if (m_statList == 0)
  {
    m_statList = new StatListWindow(m_player, 0, "stats");
    setDockEnabled(m_statList, 
		   pSEQPrefs->getPrefBool("DockablePlayerStats",
					  "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_statList->preferenceName(),
					    Left);
    addDockWindow(m_statList, edge, false);
    if (!m_isStatListDocked)
      m_statList->undock();

    // connect stat list slots to interface signals
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_statList, SLOT(savePrefs(void)));
    connect(this, SIGNAL(restoreFonts(void)),
	    m_statList, SLOT(restoreFont(void)));

    // restore the size of the spawn list
    m_statList->restoreSize();

    // only do this move stuff iff the spawn list isn't docked
    // and the user set the option to do so.
    if (!m_isStatListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
      m_statList->restorePosition();

    // insert its menu into the window menu
    insertWindowMenu(m_statList);
  }

  // make sure it's visible
  m_statList->show();
}

void EQInterface::showSkillList(void)
{
  // if it doesn't exist, create it
  if (m_skillList == 0)
  {
    m_skillList = new SkillListWindow(m_player, 0, "skills");
    setDockEnabled(m_skillList, 
		   pSEQPrefs->getPrefBool("DockablePlayerSkills",
					  "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_skillList->preferenceName(),
					    Left);
    addDockWindow(m_skillList, edge, false);
    if (!m_isSkillListDocked)
      m_skillList->undock();

    // connect skill list slots to interfaces signals
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_skillList, SLOT(savePrefs(void)));
    connect(this, SIGNAL(restoreFonts(void)),
	    m_skillList, SLOT(restoreFont(void)));

    // restore the size of the spawn list
    m_skillList->restoreSize();

    // only do this move stuff iff the spawn list isn't docked
    // and the user set the option to do so.
    if (!m_isSkillListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
      m_skillList->restorePosition();

    // insert its menu into the window menu
    insertWindowMenu(m_skillList);
  }

  // make sure it's visible
  m_skillList->show();
}

void EQInterface::showSpellList(void)
{
  // if it doesn't exist, create it
  if (m_spellList == 0)
  {
    m_spellList = new SpellListWindow(m_spellShell, this, "spelllist");
    setDockEnabled(m_spellList, 
		   pSEQPrefs->getPrefBool("DockableSpellList",
					  "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_spellList->preferenceName(),
					    Left);
    addDockWindow(m_spellList, edge, false);
    if (!m_isSpellListDocked)
      m_spellList->undock();

    SpellList* spellList = m_spellList->spellList();

    // connect SpellShell to SpellList
    connect(m_spellShell, SIGNAL(addSpell(const SpellItem *)),
	    spellList, SLOT(addSpell(const SpellItem *)));
    connect(m_spellShell, SIGNAL(delSpell(const SpellItem *)),
	    spellList, SLOT(delSpell(const SpellItem *)));
    connect(m_spellShell, SIGNAL(changeSpell(const SpellItem *)),
	    spellList, SLOT(changeSpell(const SpellItem *)));
    connect(m_spellShell, SIGNAL(clearSpells()),
	    spellList, SLOT(clear()));
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_spellList, SLOT(savePrefs(void)));
    connect(this, SIGNAL(restoreFonts(void)),
	    m_spellList, SLOT(restoreFont(void)));

    // restore the size of the spell list
    m_spellList->restoreSize();

    // only do this move stuff iff the spell list isn't docked
    // and the user set the option to do so.
    if (!m_isSpellListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
      m_spellList->restorePosition();

    // insert its menu into the window menu
    insertWindowMenu(m_spellList);
  }

  // make sure it's visible
  m_spellList->show();
}

void EQInterface::showCompass(void)
{
  // if it doesn't exist, create it.
  if (m_compass == 0)
  {
    m_compass = new CompassFrame(m_player, 0, "compass");
    setDockEnabled(m_compass, 
		   pSEQPrefs->getPrefBool("DockableCompass",
					  "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_compass->preferenceName(),
					    Left);
    addDockWindow(m_compass, edge, false);
    if (!m_isCompassDocked)
      m_compass->undock();

    // supply the compass slots with EQInterface signals
    connect (this, SIGNAL(selectSpawn(const Item*)),
	     m_compass, SLOT(selectSpawn(const Item*)));
    connect(this, SIGNAL(restoreFonts(void)),
	    m_compass, SLOT(restoreFont(void)));
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_compass, SLOT(savePrefs(void)));

    m_compass->restoreSize();

    // move window to new position
    if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", true))
      m_compass->restorePosition(); 

    // insert its menu into the window menu
    insertWindowMenu(m_compass);
 }

  // make sure it's visible
  m_compass->show();
}

void EQInterface::showNetDiag()
{
  if (m_netDiag == 0)
  {
    m_netDiag = new NetDiag(m_packet, 0, "NetDiag");
    setDockEnabled(m_netDiag, 
		   pSEQPrefs->getPrefBool("DockableNetDiag",
					  "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_netDiag->preferenceName(),
					    Bottom);
    addDockWindow(m_netDiag, edge, true);
    m_netDiag->undock();

    connect(this, SIGNAL(restoreFonts(void)),
	    m_netDiag, SLOT(restoreFont(void)));
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_netDiag, SLOT(savePrefs(void)));

    m_netDiag->restoreSize();
    
    // move window to new position
    if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", true))
      m_netDiag->restorePosition(); 

    // insert its menu into the window menu
    insertWindowMenu(m_netDiag);
  }

  // make sure it's visible
  m_netDiag->show();
}

void EQInterface::showGuildList(void)
{
  if (!m_guildListWindow)
  {
    m_guildListWindow = new GuildListWindow(m_player, m_guildShell, 
					    0, "GuildList");
    setDockEnabled(m_guildListWindow, 
		   pSEQPrefs->getPrefBool("DockableGuildListWindow",
					  "Interface", true));
    Dock edge = (Dock)pSEQPrefs->getPrefInt("Dock", 
					    m_guildListWindow->preferenceName(),
					    Bottom);
    addDockWindow(m_guildListWindow, edge, true);
    m_guildListWindow->undock();

    connect(this, SIGNAL(restoreFonts(void)),
	    m_guildListWindow, SLOT(restoreFont(void)));
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_guildListWindow, SLOT(savePrefs(void)));

    m_guildListWindow->restoreSize();
    
    // move window to new position
    if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", true))
      m_guildListWindow->restorePosition(); 

    // insert its menu into the window menu
    insertWindowMenu(m_guildListWindow);
  }

  // make sure it's visible
  m_guildListWindow->show();
}

void EQInterface::createFilteredSpawnLog(void)
{
  if (m_filteredSpawnLog)
    return;

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs",
							   "filtered_spawns.log");
  
  m_filteredSpawnLog = new FilteredSpawnLog(m_dateTimeMgr, m_filterMgr,
					    logFileInfo.absFilePath());

  connect(m_spawnShell, SIGNAL(addItem(const Item*)),
	  m_filteredSpawnLog, SLOT(addItem(const Item*)));
  connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	  m_filteredSpawnLog, SLOT(delItem(const Item*)));
  connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	  m_filteredSpawnLog, SLOT(killSpawn(const Item*)));
}

void EQInterface::createSpawnLog(void)
{
  // if the spawnLogger already exists, then nothing to do... 
  if (m_spawnLogger)
    return;

  QString logFile = pSEQPrefs->getPrefString("SpawnLogFilename",
					     "Misc", 
					     "spawnlog.txt");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  logFile = logFileInfo.absFilePath();

   // create the spawn logger
   m_spawnLogger = new SpawnLog(m_dateTimeMgr, logFile);

   // initialize it with the current state
   QString shortZoneName = m_zoneMgr->shortZoneName();
   if (!shortZoneName.isEmpty())
     m_spawnLogger->logNewZone(shortZoneName);

   // Connect SpawnLog slots to ZoneMgr signals
   connect(m_zoneMgr, SIGNAL(zoneBegin(const QString&)),
	   m_spawnLogger, SLOT(logNewZone(const QString&)));

   // Connect SpawnLog slots to EQPacket signals
   m_packet->connect2("OP_ZoneSpawns", SP_Zone, DIR_Server,
		      "spawnStruct", SZC_Modulus,
		      m_spawnLogger, SLOT(logZoneSpawns(const uint8_t*, size_t)));
   m_packet->connect2("OP_NewSpawn", SP_Zone, DIR_Server,
		      "spawnStruct", SZC_Match,
		      m_spawnLogger, SLOT(logNewSpawn(const uint8_t*)));
   
   // Connect SpawnLog slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	   m_spawnLogger, SLOT(logDeleteSpawn(const Item *)));
   connect(m_spawnShell, SIGNAL(killSpawn(const Item*, const Item*, uint16_t)),
	   m_spawnLogger, SLOT(logKilledSpawn(const Item *, const Item*, uint16_t)));
}

void EQInterface::createGlobalLog(void)
{
  if (m_globalLog)
    return;

  QString logFile = pSEQPrefs->getPrefString("GlobalLogFilename",
					     "PacketLogging",
					     "global.log");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  m_globalLog = new PacketLog(*m_packet, 
			      logFileInfo.absFilePath(),
			      this, "GlobalLog");

  connect(m_packet, SIGNAL(newPacket(const EQUDPIPPacketFormat&)),
	  m_globalLog, SLOT(logData(const EQUDPIPPacketFormat&)));
}

void EQInterface::createWorldLog(void)
{
  if (m_worldLog)
    return;

  QString logFile = pSEQPrefs->getPrefString("WorldLogFilename",
					     "PacketLogging",
					     "world.log");
  
  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  m_worldLog = new PacketStreamLog(*m_packet, 
				   logFileInfo.absFilePath(),
				   this, "WorldLog");

  m_worldLog->setRaw(pSEQPrefs->getPrefBool("LogRawPackets", "PacketLogging",
					   false));

  connect(m_packet, SIGNAL(rawWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  m_worldLog, SLOT(rawStreamPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_packet, SIGNAL(decodedWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)),
	  m_worldLog, SLOT(decodedStreamPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)));
}

void EQInterface::createZoneLog(void)
{
  if (m_zoneLog)
    return;
  
  QString logFile = pSEQPrefs->getPrefString("ZoneLogFilename",
					     "PacketLogging",
					     "zone.log");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  m_zoneLog = new PacketStreamLog(*m_packet, 
				  logFileInfo.absFilePath(),
				  this, "ZoneLog");

  m_zoneLog->setRaw(pSEQPrefs->getPrefBool("LogRawPackets", "PacketLogging",
					   false));

  connect(m_packet, SIGNAL(rawZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  m_zoneLog, SLOT(rawStreamPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_packet, SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)),
	  m_zoneLog, SLOT(decodedStreamPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)));
}

void EQInterface::createBazaarLog(void)
{
  if (m_bazaarLog)
    return;
  
  QString logFile = pSEQPrefs->getPrefString("BazaarLogFilename",
					     "PacketLogging",
					     "bazaar.log");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  m_bazaarLog = new BazaarLog(*m_packet,
			      logFileInfo.absFilePath(),
			      this,
			      *m_spawnShell,
			      "BazaarLog");
  m_packet->connect2("OP_BazaarSearch", SP_Zone, DIR_Server,
		     "bazaarSearchResponseStruct", SZC_Modulus,
		     m_bazaarLog, SLOT(bazaarSearch(const uint8_t*, size_t, uint8_t)));
}

void EQInterface::createUnknownZoneLog(void)
{
  if (m_unknownZoneLog)
    return;

  QString section = "PacketLogging";

  QString logFile = pSEQPrefs->getPrefString("UnknownZoneLogFilename",
					     section,
					     "unknownzone.log");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  logFile = logFileInfo.absFilePath();

  m_unknownZoneLog = new UnknownPacketLog(*m_packet, 
					  logFile,
					  this, "UnknownLog");

  m_unknownZoneLog->setView(pSEQPrefs->getPrefBool("ViewUnknown", section, 
						   false));

  connect(m_packet, SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*, bool)),
	  m_unknownZoneLog, SLOT(packet(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*, bool)));
}

void EQInterface::createOPCodeMonitorLog(const QString& opCodeList)
{
  if (m_opcodeMonitorLog)
    return;
  
  QString section = "OpCodeMonitoring";

  QString logFile = pSEQPrefs->getPrefString("LogFilename", 
					     section, 
					     "opcodemonitor.log");

  QFileInfo logFileInfo = m_dataLocationMgr->findWriteFile("logs", logFile);
  
  logFile = logFileInfo.absFilePath();

  m_opcodeMonitorLog = new OPCodeMonitorPacketLog(*m_packet, 
						  logFile,
						  this, "OpCodeMonitorLog");
  
  m_opcodeMonitorLog->init(opCodeList);
  m_opcodeMonitorLog->setLog(pSEQPrefs->getPrefBool("Log", section, false));
  m_opcodeMonitorLog->setView(pSEQPrefs->getPrefBool("View", section, false));
  
  connect(m_packet, SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*, bool)),
	  m_opcodeMonitorLog, SLOT(packet(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*, bool)));
}


void EQInterface::insertWindowMenu(SEQWindow* window)
{
  QPopupMenu* menu = window->menu();
  if (menu)
    m_windowMenu->insertItem(window->caption(), menu, int(window));
}

void EQInterface::removeWindowMenu(SEQWindow* window)
{
  m_windowMenu->removeItem(int(window));
}

void EQInterface::setDockEnabled(QDockWindow* dw, bool enable)
{
  QMainWindow::setDockEnabled(dw, DockTop, enable);
  QMainWindow::setDockEnabled(dw, DockBottom, enable);
  QMainWindow::setDockEnabled(dw, DockLeft, enable);
  QMainWindow::setDockEnabled(dw, DockRight, enable);
}

