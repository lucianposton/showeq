/*
 * interface.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "interface.h"
#include "util.h"
#include "main.h"
#include "editor.h"
#include "logger.h"

#include <qfont.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvaluelist.h>
#include <qstatusbar.h>
#include <qvaluelist.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qaccel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qinputdialog.h>

#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qsgistyle.h>

// this define is used to diagnose the order with which zone packets are rcvd
#define ZONE_ORDER_DIAG
#undef DEBUG

/* The main interface widget */
EQInterface::EQInterface (QWidget * parent, const char *name) 
  : QMainWindow (parent, name),
    m_formattedMessageStrings(3001)
{
   QString tempStr;
   QString section = "Interface";

   const char* player_classes[] = {"Warrior", "Cleric", "Ranger", "Paladin",
                                   "Shadow Knight", "Druid", "Monk", "Bard",
                                   "Rogue", "Shaman", "Necromancer", "Wizzard",
                                   "Magician", "Enchanter", "Beastlord"
                                  };
   const char* player_races[] = {"Human", "Barbarian", "Erudite", "Wood elf",
                                 "High Elf", "Dark Elf", "Half Elf", "Dwarf",
                                 "Troll", "Ogre", "Halfling", "Gnome", "Iksar",
                                 "Vah Shir"
                                };

   // initialize packet count
   m_initialcount = 0;
   m_packetStartTime = 0;

   // Create our player object
   m_player = new EQPlayer(this);

   // Create the filter manager
   m_filterMgr = new FilterMgr();

   // if there is a short zone name already, try to load its filters
   QString shortZoneName = m_player->getShortZoneName();
   if (!shortZoneName.isEmpty())
     m_filterMgr->loadZone(shortZoneName);
   
   // Create the Category manager
   m_categoryMgr = new CategoryMgr();

   // Create the spawn shell
   m_spawnShell = new SpawnShell(*m_filterMgr, m_player);

   // Create the map manager
   m_mapMgr = new MapMgr(m_spawnShell, m_player, this);

   // Create the spell shell
   m_spellShell = new SpellShell(m_player, m_spawnShell);

   // Create the packet object
   m_packet = new EQPacket (this, "packet");

   // Create the Group Manager
   m_groupMgr = new GroupMgr(m_spawnShell, m_player, "groupmgr");

   // Initialize the experience window;
   m_expWindow = new ExperienceWindow(m_player, m_groupMgr);

   // Initialize the combat window
   m_combatWindow = new CombatWindow(m_player);

   // create the packet logger
   m_pktLogger = new PktLogger(showeq_params->PktLoggerFilename,
			       showeq_params->PktLoggerMask);

/////////////////
// Main widgets
   if (pSEQPrefs->getPrefString("Font", "Interface", "default") == "default")
       QFont appFont = QApplication::font();
   else
       QFont appFont(pSEQPrefs->getPrefString("Font", "Interface", "default"), showeq_params->fontsize, QFont::Normal, FALSE);
   qApp->setFont( appFont, TRUE );
   // Make a VBox to use as central widget
   QVBox* pCentralBox = new QVBox(this);
   setCentralWidget(pCentralBox);
 
   // Make the horizontal splitter deviding the map from the other objects
   // and add it to the main window
   m_splitH =new QSplitter(QSplitter::Horizontal,pCentralBox,"SplitH");
   m_splitH->setOpaqueResize(TRUE);

   // make a splitter between the spawnlist and other objects
   m_splitV = new QSplitter(QSplitter::Vertical,m_splitH,"SplitV");
   m_splitV->setOpaqueResize(TRUE);

   // Make a horizontal splitter for the skilllist/statlist/compass
   m_splitT = new QSplitter(QSplitter::Horizontal,m_splitV,"SplitT");
   m_splitT->setOpaqueResize(TRUE);


   // Create/display the Map(s)
   for (int i = 0; i < maxNumMaps; i++)
   {
     // first clear the variable
     m_map[i] = NULL;

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
   
   // should the compass be docked if it's created
   m_isCompassDocked = pSEQPrefs->getPrefBool("DockedCompass", section, true);

   //
   // Create the compass as required
   //
   m_compass = NULL;
   if (pSEQPrefs->getPrefBool("ShowCompass", section, false))
       showCompass();


   //
   // Create the spells listview as required (dynamic object)
   //
   m_spellList = NULL;
   m_isSpellListDocked = pSEQPrefs->getPrefBool("DockedSpellList", section, true);
   if (pSEQPrefs->getPrefBool("ShowSpellList", section, true))
       showSpellList();


   //
   // Create the Player Skills listview (ONLY CREATED WHEN NEEDED FLOYD!!!!)
   //
   m_skillList = NULL;
   m_isSkillListDocked = pSEQPrefs->getPrefBool("DockedPlayerSkills", section, true);
   if (pSEQPrefs->getPrefBool("ShowPlayerSkills", section, true))
     showSkillList();

   //
   // Create the Player Status listview (ONLY CREATED WHEN NEEDED FLOYD!!!!)
   //
   m_statList = NULL;
   m_isStatListDocked = pSEQPrefs->getPrefBool("DockedPlayerStats", section, true);
   if (pSEQPrefs->getPrefBool("ShowPlayerStats", section, true))
     showStatList();

   //
   // Create the Spawn List listview (always exists, just hidden if not specified)
   //
   m_spawnList = NULL;
   m_isSpawnListDocked = pSEQPrefs->getPrefBool("DockedSpawnList", section, true);
   if (pSEQPrefs->getPrefBool("ShowSpawnList", section, true))
     showSpawnList();

   //
   // Create the Net Statistics window as required
   // 
   m_netDiag = NULL;
   if (showeq_params->net_stats)
     showNetDiag();

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
   pFileMenu->insertItem("Sa&ve Map", m_mapMgr, SLOT(saveMap()), Key_F2);
   pFileMenu->insertItem("&Reload Filters", m_filterMgr, SLOT(loadFilters()), Key_F3);
   pFileMenu->insertItem("&Save Filters", m_filterMgr, SLOT(saveFilters()), Key_F4);
   pFileMenu->insertItem("Edit Filters", this, SLOT(launch_editor_filters()));
   pFileMenu->insertItem("Select Filter File", this, SLOT(select_filter_file()));
   pFileMenu->insertItem("Add Spawn Category", this, SLOT(addCategory()) , ALT+Key_C);
   pFileMenu->insertItem("Rebuild SpawnList", this, SLOT(rebuildSpawnList()) , ALT+Key_R);
   pFileMenu->insertItem("Reload Categories", this, SLOT(reloadCategories()) , CTRL+Key_R);
   pFileMenu->insertItem("Create MessageBox", this, SLOT(createMessageBox()), Key_F11);
   pFileMenu->insertItem("Select Next", this, SLOT(selectNext()), CTRL+Key_Right);
   pFileMenu->insertItem("Select Prev", this, SLOT(selectPrev()), CTRL+Key_Left);
   if (showeq_params->playbackpackets)
   {
     pFileMenu->insertItem("Inc Playback Speed", m_packet, SLOT(incPlayback()), CTRL+Key_X);
     pFileMenu->insertItem("Dec Playback Speed", m_packet, SLOT(decPlayback()), CTRL+Key_Z);
   }
   pFileMenu->insertItem("&Quit", qApp, SLOT(quit()));

   // Debug menu
   //pDebugMenu = new QPopupMenu;
   QPopupMenu* pDebugMenu = new QPopupMenu;
   menuBar()->insertItem("&Debug", pDebugMenu);
   pDebugMenu->insertItem("List &Spawns", this, SLOT(listSpawns()), ALT+CTRL+Key_S);
   pDebugMenu->insertItem("List &Drops", this, SLOT(listDrops()), ALT+CTRL+Key_D);
   pDebugMenu->insertItem("List &Coins", this, SLOT(listCoins()), ALT+CTRL+Key_C);
   pDebugMenu->insertItem("List &Map Info", this, SLOT(listMapInfo()), ALT+CTRL+Key_M);
   pDebugMenu->insertItem("Dump &Spawns", this, SLOT(dumpSpawns()), ALT+SHIFT+CTRL+Key_S);
   pDebugMenu->insertItem("Dump &Drops", this, SLOT(dumpDrops()), ALT+SHIFT+CTRL+Key_D);
   pDebugMenu->insertItem("Dump &Coins", this, SLOT(dumpCoins()), ALT+SHIFT+CTRL+Key_C);
   pDebugMenu->insertItem("Dump Map &Info", this, SLOT(dumpMapInfo()), ALT+SHIFT+CTRL+Key_M);
   pDebugMenu->insertItem("&List Filters", m_filterMgr, SLOT(listFilters()), ALT+Key_I);

   // Log menu
   QPopupMenu* pLogMenu = new QPopupMenu;
   menuBar()->insertItem("Lo&g", pLogMenu);
   pLogMenu->setCheckable(true);
   m_id_log_AllPackets = pLogMenu->insertItem("All Packets", this, SLOT(toggle_log_AllPackets()), Key_F5);
   m_id_log_ZoneData   = pLogMenu->insertItem("Zone Data", this, SLOT(toggle_log_ZoneData()), Key_F6);
   m_id_log_UnknownData= pLogMenu->insertItem("Unknown Zone Data", this, SLOT(toggle_log_UnknownData()), Key_F7);
   menuBar()->setItemChecked (m_id_log_AllPackets , showeq_params->logAllPackets);
   menuBar()->setItemChecked (m_id_log_ZoneData   , showeq_params->logZonePackets);
   menuBar()->setItemChecked (m_id_log_UnknownData, showeq_params->logUnknownZonePackets);


   // View menu
   QPopupMenu* pViewMenu = new QPopupMenu;
   menuBar()->insertItem("&View", pViewMenu);
   pViewMenu->setCheckable(true);
   m_id_view_ChannelMsgs = pViewMenu->insertItem("Channel Messages", this, SLOT(toggle_view_ChannelMsgs()));
   m_id_view_ExpWindow = pViewMenu->insertItem("Experience Window", this, SLOT(toggle_view_ExpWindow()));
   m_id_view_CombatWindow = pViewMenu->insertItem("Combat Window", this, SLOT(toggle_view_CombatWindow()));
   pViewMenu->insertSeparator(-1);
   m_id_view_SpellList = pViewMenu->insertItem("Spell List", this, SLOT(toggle_view_SpellList()));
   m_id_view_SpawnList = pViewMenu->insertItem("Spawn List", this, SLOT(toggle_view_SpawnList()));
   m_id_view_PlayerStats = pViewMenu->insertItem("Player Stats", this, SLOT(toggle_view_PlayerStats()));
   m_id_view_PlayerSkills = pViewMenu->insertItem("Player Skills", this,SLOT(toggle_view_PlayerSkills()));
   m_id_view_Compass = pViewMenu->insertItem("Compass", this, SLOT(toggle_view_Compass()));
   menuBar()->setItemChecked(m_id_view_PlayerStats, (m_statList != NULL));

   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map ";
        if (i > 0)
            mapName += QString::number(i + 1);
        m_id_view_Map[i] = pViewMenu->insertItem(mapName, this, SLOT(toggle_view_Map(int)));
        pViewMenu->setItemParameter(m_id_view_Map[i], i);
        menuBar()->setItemChecked(m_id_view_Map[i], (m_map[i] != NULL));
   }
   m_id_view_NetDiag = pViewMenu->insertItem("Network Diagnostics", this, SLOT(toggle_view_NetDiag()));

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
   pViewMenu->setItemEnabled(m_id_view_PlayerSkills_Options, (m_skillList != NULL));
   m_skillWinMenu->setCheckable(TRUE);
   
   m_id_view_PlayerSkills_Languages = m_skillWinMenu->insertItem("&Langauges");
   m_skillWinMenu->setItemParameter(m_id_view_PlayerSkills_Languages,0);

   connect (m_skillWinMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_SkillWin(int)));

   // View -> SpawnList
    m_spawnListMenu = new QPopupMenu;
    m_id_view_SpawnList_Options = pViewMenu->insertItem( "Spawn &List", m_spawnListMenu);
    pViewMenu->setItemEnabled(m_id_view_SpawnList_Options, (m_spawnList != NULL));
    m_spawnListMenu->setCheckable(TRUE);

    m_id_view_SpawnList_Cols[SPAWNCOL_NAME] = m_spawnListMenu->insertItem("&Name");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_NAME], SPAWNCOL_NAME);

    m_id_view_SpawnList_Cols[SPAWNCOL_LEVEL] = m_spawnListMenu->insertItem("&Level");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_LEVEL], SPAWNCOL_LEVEL);

    m_id_view_SpawnList_Cols[SPAWNCOL_HP] = m_spawnListMenu->insertItem("&HP");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_HP], SPAWNCOL_HP);

    m_id_view_SpawnList_Cols[SPAWNCOL_MAXHP] = m_spawnListMenu->insertItem("&Max HP");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_MAXHP], SPAWNCOL_MAXHP);

    m_id_view_SpawnList_Cols[SPAWNCOL_XPOS] = m_spawnListMenu->insertItem("Coord &1");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_XPOS], SPAWNCOL_XPOS);

    m_id_view_SpawnList_Cols[SPAWNCOL_YPOS] = m_spawnListMenu->insertItem("Coord &2");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_YPOS], SPAWNCOL_YPOS);

    m_id_view_SpawnList_Cols[SPAWNCOL_ZPOS] = m_spawnListMenu->insertItem("Coord &3");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_ZPOS], SPAWNCOL_ZPOS);

    m_id_view_SpawnList_Cols[SPAWNCOL_ID] = m_spawnListMenu->insertItem("I&D");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_ID], SPAWNCOL_ID);

    m_id_view_SpawnList_Cols[SPAWNCOL_DIST] = m_spawnListMenu->insertItem("&Dist");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_DIST], SPAWNCOL_DIST);

    m_id_view_SpawnList_Cols[SPAWNCOL_RACE] = m_spawnListMenu->insertItem("&Race");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_RACE], SPAWNCOL_RACE);

    m_id_view_SpawnList_Cols[SPAWNCOL_CLASS] = m_spawnListMenu->insertItem("&Class");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_CLASS], SPAWNCOL_CLASS);

    m_id_view_SpawnList_Cols[SPAWNCOL_INFO] = m_spawnListMenu->insertItem("&Info");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_INFO], SPAWNCOL_INFO);

    m_id_view_SpawnList_Cols[SPAWNCOL_SPAWNTIME] = m_spawnListMenu->insertItem("Spawn &Time");
    m_spawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_SPAWNTIME], SPAWNCOL_SPAWNTIME);

    connect (m_spawnListMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_SpawnListCol(int)));

   pViewMenu->insertSeparator(-1);

   int x;
   // View -> DockedWin
   m_dockedWinMenu = new QPopupMenu;
   pViewMenu->insertItem( "&Docked", m_dockedWinMenu);
   m_dockedWinMenu->setCheckable(true);
    
   x = m_dockedWinMenu->insertItem("Spawn &List");
   m_dockedWinMenu->setItemParameter(x, 0);
   m_dockedWinMenu->setItemChecked(x, m_isSpawnListDocked);
    
   x = m_dockedWinMenu->insertItem("&Player Stats");
   m_dockedWinMenu->setItemParameter(x, 1);
   m_dockedWinMenu->setItemChecked(x, m_isStatListDocked);
    
   x = m_dockedWinMenu->insertItem("&Player Skills");
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
        QString mapName = "Map";
        if (i > 0)
            mapName += QString::number(i + 1);
        x = m_dockedWinMenu->insertItem(mapName);
        m_dockedWinMenu->setItemParameter(x, i + mapDockBase);
        m_dockedWinMenu->setItemChecked(x, m_isMapDocked[i]);
   }
    
   connect (m_dockedWinMenu, SIGNAL(activated(int)), this, SLOT(toggle_view_DockedWin(int)));

   menuBar()->setItemChecked(m_id_view_PlayerSkills, (pSEQPrefs->getPrefBool("ShowPlayerSkills", section, true)));
   menuBar()->setItemChecked(m_id_view_PlayerStats, (pSEQPrefs->getPrefBool("ShowPlayerStats", section, true)));
   menuBar()->setItemChecked(m_id_view_SpawnList, (pSEQPrefs->getPrefBool("ShowSpawnList", section, true)));

   // only check for non-NULL for the following options, because they 
   // are only non-NULL if they are to be visible, and isVisble() 
   // won't be set until after show() is called on the top level window
   menuBar()->setItemChecked(m_id_view_Compass, (m_compass != NULL));
   for (int i = 0; i < maxNumMaps; i++)
        menuBar()->setItemChecked(m_id_view_Map[i], (m_map[i] != NULL));
   menuBar()->setItemChecked(m_id_view_NetDiag, (m_netDiag != NULL));
   
   // set initial view options
   if (pSEQPrefs->getPrefBool("ShowExpWindow", section, false))
       toggle_view_ExpWindow();
   if (pSEQPrefs->getPrefBool("ShowCombatWindow", section, false))
       toggle_view_CombatWindow();
   if (pSEQPrefs->getPrefBool("ShowSpellList", section, false))
       toggle_view_SpellList();

   

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
   m_id_opt_ResetMana = pOptMenu->insertItem("Reset Max Mana", this, SLOT(resetMaxMana()));
   m_id_opt_PvPTeams  = pOptMenu->insertItem("PvP Teams", this, SLOT(toggle_opt_PvPTeams()));
   m_id_opt_PvPDeity  = pOptMenu->insertItem("PvP Deity", this, SLOT(toggle_opt_PvPDeity()));
   x = pOptMenu->insertItem("Create Unknown Spawns", this, SLOT(toggle_opt_CreateUnknownSpawns(int)));
   menuBar()->setItemChecked (x, showeq_params->createUnknownSpawns);
   
   menuBar()->setItemChecked (m_id_opt_Fast, showeq_params->fast_machine);
   menuBar()->setItemChecked (m_id_opt_ConSelect, showeq_params->con_select);
   menuBar()->setItemChecked (m_id_opt_TarSelect, showeq_params->tar_select);
   menuBar()->setItemChecked (m_id_opt_KeepSelectedVisible, showeq_params->keep_selected_visible);
   menuBar()->setItemChecked (m_id_opt_LogSpawns, showeq_params->logSpawns);
   menuBar()->setItemChecked (m_id_opt_PvPTeams, showeq_params->pvp);
   menuBar()->setItemChecked (m_id_opt_PvPDeity, showeq_params->deitypvp);

   // SaveState SubMenu
   QPopupMenu* pSaveStateMenu = new QPopupMenu;
   pOptMenu->insertItem("&Save State", pSaveStateMenu);
   pSaveStateMenu->setCheckable(true);
   x = pSaveStateMenu->insertItem("&Decode Key", this, 
				  SLOT(toggle_opt_save_DecodeKey(int)));
   pSaveStateMenu->setItemChecked(x, showeq_params->saveDecodeKey);
   x = pSaveStateMenu->insertItem("&Player", this, 
				  SLOT(toggle_opt_save_PlayerState(int)));
   pSaveStateMenu->setItemChecked(x, showeq_params->savePlayerState);
   x = pSaveStateMenu->insertItem("&Spawns", this, 
				  SLOT(toggle_opt_save_Spawns(int)));
   pSaveStateMenu->setItemChecked(x, showeq_params->saveSpawns);
   pSaveStateMenu->insertItem("Base &Filename...", this, 
			      SLOT(set_opt_save_BaseFilename(void)));

   pSaveStateMenu->insertSeparator(-1);
   QPopupMenu* subMenu = new QPopupMenu;
   QSpinBox* saveFrequencySpinBox = new QSpinBox(1, 320, 1, subMenu);
   saveFrequencySpinBox->setValue(showeq_params->saveSpawnsFrequency / 1000);
   connect(saveFrequencySpinBox, SIGNAL(valueChanged(int)),
	   this, SLOT(set_opt_save_SpawnFrequency(int)));
   subMenu->insertItem(saveFrequencySpinBox);
   pSaveStateMenu->insertItem("Spawn Save Frequency (s)", 
			      subMenu);

   // Network Menu
   m_netMenu = new QPopupMenu;
   menuBar()->insertItem("&Network", m_netMenu);
   m_netMenu->insertItem("Monitor &Next EQ Client Seen", this, SLOT(set_net_monitor_next_client()));
   m_netMenu->insertItem("Monitor EQ Client &IP Address", this, SLOT(set_net_client_IP_address()));
   m_netMenu->insertItem("Monitor EQ Client &MAC Address", this, SLOT(set_net_client_MAC_address()));
   m_id_net_sessiontrack = m_netMenu->insertItem("Session Tracking", this, SLOT(toggle_net_session_tracking()));
   m_netMenu->setItemChecked(m_id_net_sessiontrack, showeq_params->session_tracking);
   x = m_netMenu->insertItem("&Real Time Thread", this, SLOT(toggle_net_real_time_thread(int)));
   m_netMenu->setItemChecked(x, showeq_params->realtime);
   x = m_netMenu->insertItem("&Broken Decode", this, SLOT(toggle_net_broken_decode(int)));
   m_netMenu->setItemChecked(x, showeq_params->broken_decode);
   m_netMenu->insertSeparator(-1);
   subMenu = new QPopupMenu;
   QPopupMenu* subSubMenu = new QPopupMenu;
   QSpinBox* arqSeqGiveUpSpinBox = new QSpinBox(32, 256, 8, subSubMenu);
   arqSeqGiveUpSpinBox->setValue(showeq_params->arqSeqGiveUp);
   connect(arqSeqGiveUpSpinBox, SIGNAL(valueChanged(int)),
	   this, SLOT(set_net_arq_giveup(int)));
   subSubMenu->insertItem(arqSeqGiveUpSpinBox);
   subMenu->insertItem("Arq Seq Give Up", 
			 subSubMenu);
   m_netMenu->insertItem("Advanced", subMenu);
   if(showeq_params->session_tracking)
      m_packet->session_tracking();

   
   // Character Menu 
   m_charMenu = new QPopupMenu;
   menuBar()->insertItem("&Character", m_charMenu);
   int yx = m_charMenu->insertItem("Auto Detect Settings", this, SLOT(toggleAutoDetectCharSettings(int)));
   m_charMenu->setItemChecked(yx, showeq_params->AutoDetectCharSettings);

   // Character -> Level
   m_charLevelMenu = new QPopupMenu;
   m_charMenu->insertItem("Choose &Level", m_charLevelMenu);
   m_levelSpinBox = new QSpinBox(1, 60, 1, this, "m_levelSpinBox");
   m_charLevelMenu->insertItem( m_levelSpinBox );
   m_levelSpinBox->setWrapping( true );
   m_levelSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
   m_levelSpinBox->setPrefix("Level: ");
   connect(m_levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetDefaultCharacterLevel(int)));
   m_levelSpinBox->setValue(showeq_params->defaultLevel);

   // Character -> Class
   m_charClassMenu = new QPopupMenu;
   m_charMenu->insertItem("Choose &Class", m_charClassMenu);
   for( int i = 0; i < PLAYER_CLASSES; i++)
   {
       char_ClassID[i] = m_charClassMenu->insertItem(player_classes[i]);
       m_charClassMenu->setItemParameter(char_ClassID[i],i+1);
       if(i+1 == showeq_params->defaultClass)
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

       if(m_charRaceMenu->itemParameter(char_RaceID[i]) == showeq_params->defaultRace)
          m_charRaceMenu->setItemChecked(char_RaceID[i], true);
   }
   connect (m_charRaceMenu, SIGNAL(activated(int)), this, SLOT(SetDefaultCharacterRace(int)));

   // OpCode Monitor
   QPopupMenu* pOpCodeMenu = new QPopupMenu;
   menuBar()->insertItem("OpCode Monitor", pOpCodeMenu);
   if (showeq_params->monitorOpCode_Usage == true)
       pOpCodeMenu->insertItem("Disable &OpCode Monitoring", this,
			      SLOT(ToggleOpCodeMonitoring(int)), CTRL+ALT+Key_O);
   else
       pOpCodeMenu->insertItem("Enable &OpCode Monitoring", this,
			      SLOT(ToggleOpCodeMonitoring(int)), CTRL+ALT+Key_O);
   pOpCodeMenu->insertItem("&Reload Monitored OpCode List", this,
			  SLOT(ReloadMonitoredOpCodeList()), CTRL+ALT+Key_R);
   m_id_view_UnknownData = pOpCodeMenu->insertItem("Unknown Data", this, SLOT(toggle_view_UnknownData()) , Key_F8);
   viewUnknownData = false;
   menuBar()->setItemChecked(m_id_view_UnknownData, viewUnknownData);
   
   // Interface Menu
   //pInterfaceMenu = new QPopupMenu;
   QPopupMenu* pInterfaceMenu = new QPopupMenu;
   menuBar()->insertItem( "&Interface" , pInterfaceMenu);

   pInterfaceMenu->insertItem("Hide MenuBar", this, SLOT(toggle_view_menubar()));
   pInterfaceMenu->insertItem("Hide/Unhide StatusBar", this, SLOT(toggle_view_statusbar()));

   // Interface -> Style
   //pStyleMenu = new QPopupMenu;
   QPopupMenu* pStyleMenu = new QPopupMenu;
   pInterfaceMenu->insertItem( "&Style", pStyleMenu);
   pStyleMenu->setCheckable(TRUE);
   pStyleMenu->insertItem( "Platinum (Macintosh)");
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

   // Interface -> WindowCaption
   m_windowCaptionMenu = new QPopupMenu;
   pInterfaceMenu->insertItem( "Window &Caption", m_windowCaptionMenu);
    
   x = m_windowCaptionMenu->insertItem("&Main Window");
   m_windowCaptionMenu->setItemParameter(x, 5);
    
   x = m_windowCaptionMenu->insertItem("Spawn &List");
   m_windowCaptionMenu->setItemParameter(x, 0);
    
   x = m_windowCaptionMenu->insertItem("&Player Stats");
   m_windowCaptionMenu->setItemParameter(x, 1);
    
   x = m_windowCaptionMenu->insertItem("&Player Skills");
   m_windowCaptionMenu->setItemParameter(x, 2);
    
   x = m_windowCaptionMenu->insertItem("Sp&ell List");
   m_windowCaptionMenu->setItemParameter(x, 3);
    
   x = m_windowCaptionMenu->insertItem("&Compass");
   m_windowCaptionMenu->setItemParameter(x, 4);
    
   x = m_windowCaptionMenu->insertItem("&Experience Window");
   m_windowCaptionMenu->setItemParameter(x, 6);
    
   x = m_windowCaptionMenu->insertItem("&Combat Window");
   m_windowCaptionMenu->setItemParameter(x, 7);
    
   x = m_windowCaptionMenu->insertItem("&Network Diagnostics");
   m_windowCaptionMenu->setItemParameter(x, 8);

   // insert Map docking options 
   // NOTE: Always insert Map docking options at the end of the Docked menu
   for (int i = 0; i < maxNumMaps; i++)
   {     
        QString mapName = "Map";
        if (i > 0)
            mapName += QString::number(i + 1);
        x = m_windowCaptionMenu->insertItem(mapName);
        m_windowCaptionMenu->setItemParameter(x, i + mapDockBase);
   }
    
   connect (m_windowCaptionMenu, SIGNAL(activated(int)), this, SLOT(set_interface_WindowCaption(int)));

////////////////////
// QStatusBar creation
   
   QString statusBarSection = "Interface_StatusBar";
   int sts_widget_count = 0; // total number of widgets visible on status bar

   //Status widget
     m_stsbarStatus = new QLabel(statusBar(), "Status");
     m_stsbarStatus->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarStatus->setFixedHeight(showeq_params->statusfontsize + 6);
     m_stsbarStatus->setMinimumWidth(80);
     m_stsbarStatus->setText(QString("ShowEQ %1").arg(VERSION));
     statusBar()->addWidget(m_stsbarStatus, 8);

   //Zone widget
     m_stsbarZone = new QLabel(statusBar(), "Zone");
     m_stsbarZone->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarZone->setText("Zone: [unknown]");
     m_stsbarZone->setFixedHeight(showeq_params->statusfontsize + 6);
     statusBar()->addWidget(m_stsbarZone, 2);

   //Mobs widget
     m_stsbarSpawns = new QLabel(statusBar(), "Mobs");
     m_stsbarSpawns->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarSpawns->setText("Mobs:");
     m_stsbarSpawns->setFixedHeight(showeq_params->statusfontsize + 6);
     statusBar()->addWidget(m_stsbarSpawns, 1);

   //Exp widget
     m_stsbarExp = new QLabel(statusBar(), "Exp");
     m_stsbarExp->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarExp->setText("Exp [unknown]");
     m_stsbarExp->setFixedHeight(showeq_params->statusfontsize + 6);
     statusBar()->addWidget(m_stsbarExp, 2);

   //ExpAA widget
     m_stsbarExpAA = new QLabel(statusBar(), "ExpAA");
     m_stsbarExpAA->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarExpAA->setText("ExpAA [unknown]");
     m_stsbarExpAA->setFixedHeight(showeq_params->statusfontsize + 6);
     statusBar()->addWidget(m_stsbarExpAA, 2);
   
   //Pkt widget
     m_stsbarPkt = new QLabel(statusBar(), "Pkt");
     m_stsbarPkt->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarPkt->setText("Pkt 0");
     m_stsbarPkt->setFixedHeight(showeq_params->statusfontsize + 6);
     statusBar()->addWidget(m_stsbarPkt, 1);

   //EQTime widget
     m_stsbarEQTime = new QLabel(statusBar(), "EQTime");
     m_stsbarEQTime->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarEQTime->setText("EQTime [UNKNOWN]");
     m_stsbarEQTime->setFixedHeight(showeq_params->statusfontsize + 6);
     statusBar()->addWidget(m_stsbarEQTime, 1);

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

   //hide the statusbar if no visible widgets
   if (!sts_widget_count || !pSEQPrefs->getPrefBool("StatusBarActive", statusBarSection, 1))
      statusBar()->hide();


//////////////////
// ToolTips

   // The first call to tooTipGroup() makes it exist
   // toolTipGroup()->addWidget();

/////////////////
// interface connections

   // connect MapMgr slots to interface signals
   connect(this, SIGNAL(saveAllPrefs(void)),
	   m_mapMgr, SLOT(savePrefs(void)));

   // connect GroupMgr slots to EQPacket signals
   connect(m_packet, SIGNAL(groupInfo(const groupMemberStruct*, uint32_t, uint8_t)),
	   m_groupMgr, SLOT(handleGroupInfo(const groupMemberStruct*)));

   // connect GroupMgr slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	   m_groupMgr, SLOT(delItem(const Item*)));

   // connect interface slots to Packet signals
   connect(m_packet, SIGNAL(clientTarget(const clientTargetStruct*, uint32_t, uint8_t)),
	   this, SLOT(clientTarget(const clientTargetStruct*)));
   connect(m_packet, SIGNAL(attack2Hand1(const attack2Struct*, uint32_t, uint8_t)), 
	   this, SLOT(attack2Hand1(const attack2Struct*)));
   connect(m_packet, SIGNAL(action2Message(const action2Struct*, uint32_t, uint8_t)),
           this, SLOT(action2Message(const action2Struct*)));
   connect(m_packet, SIGNAL(wearItem(const playerItemStruct*, uint32_t, uint8_t)),
	   this, SLOT(wearItem(const playerItemStruct*)));
   connect(m_packet, SIGNAL(itemShop(const itemInShopStruct*, uint32_t, uint8_t)),
	   this, SLOT(itemShop(const itemInShopStruct*)));
   connect(m_packet, SIGNAL(moneyOnCorpse(const moneyOnCorpseStruct*, uint32_t, uint8_t)),
	   this, SLOT(moneyOnCorpse(const moneyOnCorpseStruct*)));
   connect(m_packet, SIGNAL(itemPlayerReceived(const itemOnCorpseStruct*, uint32_t, uint8_t)),
	   this, SLOT(itemPlayerReceived(const itemOnCorpseStruct*)));
   connect(m_packet, SIGNAL(tradeItemOut(const tradeItemOutStruct*, uint32_t, uint8_t)),
	   this, SLOT(tradeItemOut(const tradeItemOutStruct*)));
   connect(m_packet, SIGNAL(tradeItemIn(const tradeItemInStruct*, uint32_t, uint8_t)),
	   this, SLOT(tradeItemIn(const tradeItemInStruct*)));
   connect(m_packet, SIGNAL(channelMessage(const channelMessageStruct*, uint32_t, uint8_t)),
	   this, SLOT(channelMessage(const channelMessageStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(formattedMessage(const formattedMessageStruct*, uint32_t, uint8_t)),
	   this, SLOT(formattedMessage(const formattedMessageStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(random(const randomStruct*, uint32_t, uint8_t)),
	   this, SLOT(random(const randomStruct*)));
   connect(m_packet, SIGNAL(emoteText(const emoteTextStruct*, uint32_t, uint8_t)),
	   this, SLOT(emoteText(const emoteTextStruct*)));
   connect(m_packet, SIGNAL(playerBook(const playerBookStruct*, uint32_t, uint8_t)),
	   this, SLOT(playerBook(const playerBookStruct*)));
   connect(m_packet, SIGNAL(playerContainer(const playerContainerStruct*, uint32_t, uint8_t)),
	   this, SLOT(playerContainer(const playerContainerStruct*)));
   connect(m_packet, SIGNAL(inspectData(const inspectDataStruct*, uint32_t, uint8_t)),
	   this, SLOT(inspectData(const inspectDataStruct*)));
   connect(m_packet, SIGNAL(spMessage(const spMesgStruct*, uint32_t, uint8_t)),
	   this, SLOT(spMessage(const spMesgStruct*)));
   connect(m_packet, SIGNAL(handleSpell(const memSpellStruct*, uint32_t, uint8_t)),
	   this, SLOT(handleSpell(const memSpellStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(beginCast(const beginCastStruct*, uint32_t, uint8_t)),
	   this, SLOT(beginCast(const beginCastStruct*)));
   connect(m_packet, SIGNAL(interruptSpellCast(const badCastStruct *, uint32_t, uint8_t)),
	   this, SLOT(interruptSpellCast(const badCastStruct *)));
   connect(m_packet, SIGNAL(startCast(const startCastStruct*, uint32_t, uint8_t)),
	   this, SLOT(startCast(const startCastStruct*)));
   connect(m_packet, SIGNAL(systemMessage(const sysMsgStruct*, uint32_t, uint8_t)),
	   this, SLOT(systemMessage(const sysMsgStruct*)));
   connect(m_packet, SIGNAL(moneyUpdate(const moneyUpdateStruct*, uint32_t, uint8_t)),
	   this, SLOT(moneyUpdate(const moneyUpdateStruct*)));
   connect(m_packet, SIGNAL(moneyThing(const moneyThingStruct*, uint32_t, uint8_t)),
	   this, SLOT(moneyThing(const moneyThingStruct*)));
   connect(m_packet, SIGNAL(groupInfo(const groupMemberStruct*, uint32_t, uint8_t)),
	   this, SLOT(groupInfo(const groupMemberStruct*)));
   connect(m_packet, SIGNAL(summonedItem(const summonedItemStruct*, uint32_t, uint8_t)),
	   this, SLOT(summonedItem(const summonedItemStruct*)));
   connect(m_packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)),
	   this, SLOT(zoneEntry(const ClientZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	   this, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	   this, SLOT(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, uint32_t, uint8_t)),
	   this, SLOT(zoneNew(const newZoneStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(toggle_session_tracking()),
	   this, SLOT(toggle_net_session_tracking()));

   // connect EQInterface slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(addItem(const Item*)),
	   this, SLOT(addItem(const Item*)));
   connect(m_spawnShell, SIGNAL(handleAlert(const Item*, alertType)),
	   this, SLOT(handleAlert(const Item*, alertType)));
   connect(m_spawnShell, SIGNAL(spawnConsidered(const Item*)),
	   this, SLOT(spawnConsidered(const Item*)));
   connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	   this, SLOT(delItem(const Item*)));
   connect(m_spawnShell, SIGNAL(killSpawn(const Item*)),
	   this, SLOT(killSpawn(const Item*)));
   connect(m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SLOT(changeItem(const Item*)));

   // connect the SpellShell slots to EQInterface signals
   connect(this, SIGNAL(spellMessage(QString&)),
	   m_spellShell, SLOT(spellMessage(QString&)));

   // connect MapMgr slots to Packet signals
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	   m_mapMgr, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	   m_mapMgr, SLOT(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, uint32_t, uint8_t)),
	   m_mapMgr, SLOT(zoneNew(const newZoneStruct*, uint32_t, uint8_t)));

   // connect the SpawnShell slots to Packet signals
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(clear(void)));
   connect(m_packet, SIGNAL(newGroundItem(const makeDropStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(newGroundItem(const makeDropStruct *)));
   connect(m_packet, SIGNAL(removeGroundItem(const remDropStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(removeGroundItem(const remDropStruct *)));
   connect(m_packet, SIGNAL(newCoinsItem(const dropCoinsStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(newCoinsItem(const dropCoinsStruct *)));
   connect(m_packet, SIGNAL(removeCoinsItem(const removeCoinsStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(removeCoinsItem(const removeCoinsStruct *)));
   connect(m_packet, SIGNAL(compressedDoorSpawn(const cDoorSpawnsStruct *, uint32_t, uint8_t)),
           m_spawnShell, SLOT(compressedDoorSpawn(const cDoorSpawnsStruct*)));
   connect(m_packet, SIGNAL(newSpawn(const newSpawnStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(newSpawn(const newSpawnStruct*)));
   connect(m_packet, SIGNAL(timeOfDay(const timeOfDayStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(timeOfDay(const timeOfDayStruct *)));
   connect(m_packet, SIGNAL(updateSpawns(const mobUpdateStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(updateSpawns(const mobUpdateStruct *)));
   connect(m_packet, SIGNAL(updateSpawnHP(const hpUpdateStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(updateSpawnHP(const hpUpdateStruct *)));
   connect(m_packet, SIGNAL(deleteSpawn(const deleteSpawnStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(deleteSpawn(const deleteSpawnStruct*)));
   connect(m_packet, SIGNAL(killSpawn(const newCorpseStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(killSpawn(const newCorpseStruct*)));
   connect(m_packet, SIGNAL(backfillSpawn(const newSpawnStruct *, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(backfillSpawn(const newSpawnStruct *)));
   connect(m_packet, SIGNAL(backfillZoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(backfillZoneSpawns(const zoneSpawnsStruct*, uint32_t)));
   connect(m_packet, SIGNAL(backfillPlayer(const charProfileStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(backfillPlayer(const charProfileStruct*)));
   connect(m_packet, SIGNAL(spawnWearingUpdate(const wearChangeStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(spawnWearingUpdate(const wearChangeStruct*)));
   connect(m_packet, SIGNAL(consMessage(const considerStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(consMessage(const considerStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(playerUpdate(const playerPosStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(playerUpdate(const playerPosStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(corpseLoc(const corpseLocStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(corpseLoc(const corpseLocStruct*)));
   connect(m_packet, SIGNAL(zoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t)),
	   m_spawnShell, SLOT(zoneSpawns(const zoneSpawnsStruct*, uint32_t)));

   // connect the SpellShell slots to EQPacket signals
   connect(m_packet, SIGNAL(startCast(const startCastStruct *, uint32_t, uint8_t)),
	   m_spellShell, SLOT(selfStartSpellCast(const startCastStruct *)));
   connect(m_packet, SIGNAL(handleSpell(const memSpellStruct *, uint32_t, uint8_t)),
	   m_spellShell, SLOT(selfFinishSpellCast(const memSpellStruct *)));
//   connect(m_packet, SIGNAL(beginCast(struct beginCastStruct *)),
//      m_spellShell, SLOT(otherStartSpellCast(struct beginCastStruct *)));
   connect(m_packet, SIGNAL(interruptSpellCast(const badCastStruct *, uint32_t, uint8_t)),
	   m_spellShell, SLOT(interruptSpellCast(const badCastStruct *)));

   // connect EQPlayer slots to EQPacket signals
   connect(m_packet, SIGNAL(backfillPlayer(const charProfileStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(backfill(const charProfileStruct*)));
   connect(m_packet, SIGNAL(increaseSkill(const skillIncStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(increaseSkill(const skillIncStruct*)));
   connect(m_packet, SIGNAL(manaChange(const manaDecrementStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(manaChange(const manaDecrementStruct*)));
   connect(m_packet, SIGNAL(playerUpdate(const playerPosStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(playerUpdate(const playerPosStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(setPlayerID(uint16_t)), 
	   m_player, SLOT(setPlayerID(uint16_t)));
   connect(m_packet, SIGNAL(updateAltExp(const altExpUpdateStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(updateAltExp(const altExpUpdateStruct*)));
   connect(m_packet, SIGNAL(updateExp(const expUpdateStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(updateExp(const expUpdateStruct*)));
   connect(m_packet, SIGNAL(updateLevel(const levelUpUpdateStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(updateLevel(const levelUpUpdateStruct*)));
   connect(m_packet, SIGNAL(updateSpawnHP(const hpUpdateStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(updateSpawnHP(const hpUpdateStruct*)));
   connect(m_packet, SIGNAL(updateStamina(const staminaStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(updateStamina(const staminaStruct*)));
   connect(m_packet, SIGNAL(wearItem(const playerItemStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(wearItem(const playerItemStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)));
   connect(m_packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(zoneEntry(const ClientZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, uint32_t, uint8_t)),
	   m_player, SLOT(zoneNew(const newZoneStruct*, uint32_t, uint8_t)));

   // connect EQInterface slots to EQPacket signals
   connect (m_packet, SIGNAL(toggle_log_AllPackets()),
	    this, SLOT(toggle_log_AllPackets()));
   connect (m_packet, SIGNAL(toggle_log_ZoneData()),  
	    this, SLOT(toggle_log_ZoneData()));
   connect (m_packet, SIGNAL(toggle_log_UnknownData()),
	    this, SLOT(toggle_log_UnknownData()));

   // interface statusbar slots
   connect (this, SIGNAL(newZoneName(const QString&)),
            m_stsbarZone, SLOT(setText(const QString&)));
   connect (m_player, SIGNAL(expChangedStr(const QString&)),
            m_stsbarExp, SLOT(setText(const QString&)));
   connect (m_player, SIGNAL(expAltChangedStr(const QString&)),
            m_stsbarExpAA, SLOT(setText(const QString&)));
   connect (m_packet, SIGNAL(eqTimeChangedStr(const QString&)),
            m_stsbarEQTime, SLOT(setText(const QString&)));
   connect (m_packet, SIGNAL(stsMessage(const QString &, int)),
            this, SLOT(stsMessage(const QString &, int)));
   connect (m_player, SIGNAL(stsMessage(const QString &, int)),
            this, SLOT(stsMessage(const QString &, int)));
   connect (m_spawnShell, SIGNAL(numSpawns(int)),
            this, SLOT(numSpawns(int)));
   connect (m_packet, SIGNAL(numPacket(int)),
            this, SLOT(numPacket(int)));
   connect (m_packet, SIGNAL(resetPacket(int)),
            this, SLOT(resetPacket(int)));
   
   // connect ExperienceWindow slots to EQPlayer signals
   connect (m_player, SIGNAL(expGained(const QString &, int, long, QString )),
	    m_expWindow, SLOT(addExpRecord(const QString &, int, long,QString )));

   // connect CombatWindow slots to the signals
   connect (this, SIGNAL(combatSignal(int, int, int, int, int)),
            m_combatWindow, SLOT(addCombatRecord(int, int, int, int, int)));
   connect (m_spawnShell, SIGNAL(spawnConsidered(const Item*)),
   	    m_combatWindow, SLOT(resetDPS()));

   // connect PktLogger to EQPacket signals
   connect (m_packet, SIGNAL(zoneServerInfo(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logZoneServerInfo(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(cPlayerItems(const cPlayerItemsStruct *, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCPlayerItems(const cPlayerItemsStruct *, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(itemShop(const itemInShopStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logItemInShop(const itemInShopStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(moneyOnCorpse(const moneyOnCorpseStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logMoneyOnCorpse(const moneyOnCorpseStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(itemPlayerReceived(const itemOnCorpseStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logItemOnCorpse(const itemOnCorpseStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(tradeItemOut(const tradeItemOutStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logTradeItemOut(const tradeItemOutStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(tradeItemIn(const tradeItemInStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logTradeItemIn(const tradeItemInStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(wearItem(const playerItemStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logPlayerItem(const playerItemStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(summonedItem(const summonedItemStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logSummonedItem(const summonedItemStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(killSpawn(const newCorpseStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logNewCorpse(const newCorpseStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(deleteSpawn(const deleteSpawnStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logDeleteSpawn(const deleteSpawnStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(channelMessage(const channelMessageStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logChannelMessage(const channelMessageStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(newSpawn(const newSpawnStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logNewSpawn(const newSpawnStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(zoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logZoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(timeOfDay(const timeOfDayStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logTimeOfDay(const timeOfDayStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(bookText(const bookTextStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logBookText(const bookTextStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(random(const randomStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logRandom(const randomStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(emoteText(const emoteTextStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logEmoteText(const emoteTextStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(corpseLoc(const corpseLocStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCorpseLoc(const corpseLocStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(playerBook(const playerBookStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logPlayerBook(const playerBookStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(playerContainer(const playerContainerStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logPlayerContainer(const playerContainerStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(inspectData(const inspectDataStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logInspectData(const inspectDataStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(updateSpawnHP(const hpUpdateStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logHPUpdate(const hpUpdateStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(spMessage(const spMesgStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logSPMesg(const spMesgStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(handleSpell(const memSpellStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logMemSpell(const memSpellStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(beginCast(const beginCastStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logBeginCast(const beginCastStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(startCast(const startCastStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logStartCast(const startCastStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(updateSpawns(const mobUpdateStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logMobUpdate(const mobUpdateStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(updateExp(const expUpdateStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logExpUpdate(const expUpdateStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(updateAltExp(const altExpUpdateStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logAltExpUpdate(const altExpUpdateStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(updateLevel(const levelUpUpdateStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logLevelUpUpdate(const levelUpUpdateStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(increaseSkill(const skillIncStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logSkillInc(const skillIncStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(doorOpen(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logDoorOpen(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(illusion(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logIllusion(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(interruptSpellCast(const badCastStruct *, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logBadCast(const badCastStruct *, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(systemMessage(const sysMsgStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logSysMsg(const sysMsgStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logZoneChange(const zoneChangeStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logZoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logZoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(zoneNew(const newZoneStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logNewZone(const newZoneStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(playerUpdate(const playerPosStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logPlayerPos(const playerPosStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(spawnWearingUpdate(const wearChangeStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logWearChange(const wearChangeStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(action2Message(const action2Struct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logAction(const action2Struct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(castOn(const castOnStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCastOn(const castOnStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(manaChange(const manaDecrementStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logManaDecrement(const manaDecrementStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(updateStamina(const staminaStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logStamina(const staminaStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(newGroundItem(const makeDropStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logMakeDrop(const makeDropStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(removeGroundItem(const remDropStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logRemDrop(const remDropStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(newCoinsItem(const dropCoinsStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logDropCoins(const dropCoinsStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(removeCoinsItem(const removeCoinsStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logRemoveCoins(const removeCoinsStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(openVendor(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logOpenVendor(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(closeVendor(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCloseVendor(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(openGM(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logOpenGM(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(closeGM(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCloseGM(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(spawnAppearance(const spawnAppearanceStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logSpawnAppearance(const spawnAppearanceStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(attack2Hand1(const attack2Struct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logAttack2(const attack2Struct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(consMessage(const considerStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logConsider(const considerStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(newGuildInZone(const newGuildInZoneStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logNewGuildInZone(const newGuildInZoneStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(moneyUpdate(const moneyUpdateStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logMoneyUpdate(const moneyUpdateStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(moneyThing(const moneyThingStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logMoneyThing(const moneyThingStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(clientTarget(const clientTargetStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logClientTarget(const clientTargetStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(bindWound(const bindWoundStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logBindWound(const bindWoundStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(compressedDoorSpawn(const cDoorSpawnsStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCDoorSpawns(const cDoorSpawnsStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(groupInfo(const groupMemberStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logGroupInfo(const groupMemberStruct*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(unknownOpcode(const uint8_t*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logUnknownOpcode(const uint8_t*, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(backfillPlayer(const charProfileStruct *, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logCharProfile(const charProfileStruct *, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(backfillSpawn(const newSpawnStruct *, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logNewSpawn(const newSpawnStruct *, uint32_t, uint8_t)));
   connect (m_packet, SIGNAL(backfillZoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t)),
	    m_pktLogger, SLOT(logZoneSpawns(const zoneSpawnsStruct*, uint32_t, uint8_t)));

   // Create message boxes defined in config preferences
   QString title;
   int i = 0;
   int j = 0;
   MsgDialog* pMsgDlg;
   QString msgSection;
   for(i = 1; i < 15; i++)
   {
      // attempt to pull a button title from the preferences
     msgSection.sprintf("MessageBox%d", i);
     if (pSEQPrefs->isPreference("Caption", msgSection))
     {
       m_viewChannelMsgs = true;
       menuBar()->setItemChecked (m_id_view_ChannelMsgs, m_viewChannelMsgs);
       title = pSEQPrefs->getPrefString("Caption", msgSection);
//        pMsgDlg = new MsgDialog(topLevelWidget(), title, m_StringList);
//        pMsgDlg = new MsgDialog(this, title, m_StringList);
        // using the parentWidget makes this messagebox isolated from the
        // main application
        pMsgDlg = new MsgDialog(parentWidget(), title, m_StringList);
        m_msgDialogList.append(pMsgDlg);

        // connect signal for new messages
        connect (this, SIGNAL (newMessage(int)),
           pMsgDlg, SLOT (newMessage(int)));
	connect (pMsgDlg, SIGNAL(toggle_view_ChannelMsgs()),
            this, SLOT(toggle_view_ChannelMsgs()));

        // set Additive mode
        pMsgDlg->setAdditive(pSEQPrefs->getPrefBool("Additive", msgSection));

        // set control mode
        pMsgDlg->showControls(!pSEQPrefs->getPrefBool("HideControls", msgSection));

        // set Msg Type mode
        pMsgDlg->showMsgType(pSEQPrefs->getPrefBool("ShowMsgType", msgSection));

        // Configure buttons
        for(j = 1; j < 15; j++)
        {
          // attempt to pull button description from the preferences
          tempStr.sprintf("Button%dName", j);
          QString buttonname(pSEQPrefs->getPrefString(tempStr, msgSection));
          tempStr.sprintf("Button%dFilter", j);
          QString buttonfilter(pSEQPrefs->getPrefString(tempStr, msgSection));
          tempStr.sprintf("Button%dColor", j);
	  QColor buttoncolor(pSEQPrefs->getPrefColor(tempStr, msgSection, 
						     QColor("black")));
          tempStr.sprintf("Button%dActive", j);

          // if we have a name and filter string
          if (!buttonname.isEmpty() && !buttonfilter.isEmpty())
          {
            // Add the button
            pMsgDlg->addButton(buttonname, buttonfilter,
                  buttoncolor, pSEQPrefs->getPrefBool(tempStr, msgSection));
          }
          else
          {
             if (!buttonname.isEmpty() || !buttonfilter.isEmpty())
             {
               fprintf(stderr, "Error: Incomplete definition of Button '%s'\n",
		       (const char*)title);
             }
// allow skipped numbers
//             break; // no more buttons
          }

        } // end for buttons

        // set Geometry
        QSize s = pSEQPrefs->getPrefSize("WindowSize", msgSection, 
					 pMsgDlg->size());
        pMsgDlg->resize(s);
        pMsgDlg->show();
        if (pSEQPrefs->getPrefBool("UseWindowPos", section, 0))
	{
	  QPoint p = pSEQPrefs->getPrefPoint("WindowPos", msgSection, 
					     pMsgDlg->pos());
          pMsgDlg->move(p);
	}
      } // end if dialog config section found

// allow skipped numbers
//      else
//        break;

   } // for all message boxes defined in pref file

   // connect signals for receiving string messages
   connect (m_packet, SIGNAL (msgReceived(const QString &)),
            this, SLOT (msgReceived(const QString &)));
   connect (m_player, SIGNAL (msgReceived(const QString &)),
            this, SLOT (msgReceived(const QString &)));
   connect (m_spawnShell, SIGNAL (msgReceived(const QString &)),
            this, SLOT (msgReceived(const QString &)));



   //
   // Geometry Configuration
   //
   
   QSize s;
   QPoint p;


   //  Add Category filters to spanwlist from config
   reloadCategories();

   // interface components

   // Restore splitter sizes
   QValueList<int> list;
   i = 0;
   for(;;) 
   {
      i++;
      tempStr.sprintf("SplitVSize%d", i);
      if (pSEQPrefs->isPreference(tempStr, section)) {
         x = pSEQPrefs->getPrefInt(tempStr, section);
         list.append(x);
      } else break;
   }
   m_splitV->setSizes(list);

   list.clear();
   i = 0;
   for(;;) 
   {
      i++;
      tempStr.sprintf("SplitHSize%d", i);
      if (pSEQPrefs->isPreference(tempStr, section)) {
         x = pSEQPrefs->getPrefInt(tempStr, section);
         list.append(x);
      } else break;
   }
   m_splitH->setSizes(list);

   list.clear();
   i = 0;
   for(;;) 
   {
      i++;
      tempStr.sprintf("SplitTSize%d", i);
      if (pSEQPrefs->isPreference(tempStr, section)) 
      {
         x = pSEQPrefs->getPrefInt(tempStr, section);
         list.append(x);
      } else break;
   }
   m_splitT->setSizes(list);


   // set mainwindow Geometry
   s = pSEQPrefs->getPrefSize("WindowSize", section, size());
#ifdef DEBUG
   printf("Resizing %d/%d\n", s.width(), s.height());
#endif
   resize(s);
   if (pSEQPrefs->getPrefBool("UseWindowPos", section, 0)) 
   {
     p = pSEQPrefs->getPrefPoint("WindowPos", section, pos());
#ifdef DEBUG
     printf("Moving window to %d/%d\n", p.x(), p.y());
#endif
      move(p);
   }
   show();

   QAccel *accel = new QAccel(this);
   accel->connectItem( accel->insertItem(CTRL+ALT+Key_S), this, SLOT(toggle_view_statusbar()));
   accel->connectItem( accel->insertItem(CTRL+ALT+Key_T), this, SLOT(toggle_view_menubar()));
   
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
  if (m_packet != NULL)
    delete m_packet;
}

void EQInterface::toggle_view_StatWin( int id )
{
   int statnum;

   statnum = m_statWinMenu->itemParameter(id);

   if (m_statWinMenu->isItemChecked(id))
   {
       m_statWinMenu->setItemChecked(id, FALSE);
       if (m_statList != NULL)
	 m_statList->enableStat(statnum, false);
   }
   else
   {
       m_statWinMenu->setItemChecked(id, TRUE);
       if (m_statList != NULL)
	 m_statList->enableStat(statnum, true);
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
	(m_skillList != NULL))
      m_skillList->showLanguages(false);
   }
   else
   {
       m_skillWinMenu->setItemChecked(id, TRUE);

    if ((id == m_id_view_PlayerSkills_Languages) &&
	(m_skillList != NULL))
      m_skillList->showLanguages(true);
   }
}

void EQInterface::toggle_view_SpawnListCol( int id )
{
  int colnum;

  colnum = m_spawnListMenu->itemParameter(id);
  
  if (m_spawnListMenu->isItemChecked(id))
  {
    m_spawnListMenu->setItemChecked(id, FALSE);
    
    if (m_spawnList != NULL)
      m_spawnList->setColumnVisible(colnum, false);
  }
  else
  {
    m_spawnListMenu->setItemChecked(id, TRUE);
    
    if (m_spawnList != NULL)
      m_spawnList->setColumnVisible(colnum, true);
   }
}

void EQInterface::toggle_view_DockedWin( int id )
{
  QPoint point;
  QWidget* widget = NULL;
  QWidget* newParent = NULL;
  int winnum;
  QString preference;

  // get the window number parameter
  winnum = m_dockedWinMenu->itemParameter(id);

  // get the current menu item state
  bool checked = m_dockedWinMenu->isItemChecked(id);

  // flip the menu item state
  m_dockedWinMenu->setItemChecked(id, !checked);

  // new parent is none, or the vertical T splitter, 
  // unless theres an override in a specific case
  newParent = checked ? NULL : m_splitT;

  switch(winnum)
  {
  case 0: // Spawn List
    // new parent is none, or the vertical splitter
    newParent = checked ? NULL : m_splitV;

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
    // new parent is none, or the vertical splitter
    newParent = checked ? NULL : m_splitV;

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
  default:
    // use default for maps since the number of them can be changed via a 
    // constant (maxNumMaps)
    if ((winnum >= mapDockBase) && (winnum < (mapDockBase + maxNumMaps)))
    {
      // new parent is none, or the horizontal splitter
      newParent = checked ? NULL : m_splitH;
      
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
  if (widget != NULL)
  {
    // fist hide the widget
    widget->hide();

    // then reparent the widget
    widget->reparent(newParent, point, true);

    // then show the widget again
    widget->show();

    // make the widget update it's geometry
    widget->updateGeometry();
  }
}

void EQInterface::set_interface_WindowCaption( int id )
{
  QWidget* widget = NULL;
  int winnum;
  QString window;

  // get the window number parameter
  winnum = m_windowCaptionMenu->itemParameter(id);

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
  if (widget != NULL)
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

//
// save prefs
//
void
EQInterface::savePrefs(void)
{
   printf("==> EQInterface::savePrefs()\n");
   if( isVisible() ) {
     QString section = "Interface";
     QString tempStr;

      // send savePrefs signal out
      emit saveAllPrefs();

      // save experience window location
      if(m_expWindow) 
      {
	section = "Experience";
	if (pSEQPrefs->getPrefBool("SavePosition", section, false)) 
	{
	  pSEQPrefs->setPrefPoint("WindowPos", section, m_expWindow->pos());
	  pSEQPrefs->setPrefSize("WindowSize", section, m_expWindow->size());
	}
      }

      // save combat window location
      if (m_combatWindow) 
      {
	section = "Combat";
	if (pSEQPrefs->getPrefBool("SavePosition", section, false)) 
	{
	  pSEQPrefs->setPrefPoint("WindowPos", section, 
				m_combatWindow->pos());
	  pSEQPrefs->setPrefSize("WindowSize", section, 
				 m_combatWindow->size());
	}
      }
		
      if (m_spawnList) 
      {
	section = "SpawnList";
	if (pSEQPrefs->getPrefBool("SavePosition", section, false)) 
	{
	  pSEQPrefs->setPrefPoint("WindowPos", section, 
				m_spawnList->pos());
	  pSEQPrefs->setPrefSize("WindowSize", section, 
				 m_spawnList->size());
	}
      }

      // save message dialog geometries
      MsgDialog* diag;
      int i = 0;

      QString msgSection;
      for (diag=m_msgDialogList.first(); diag != 0; diag=m_msgDialogList.next() )
      {
         // determine the message box number from the config file
         for(i=1; i<15; i++) {
	   msgSection.sprintf("MessageBox%d", i);
	   if (!strcmp(pSEQPrefs->getPrefString("Caption", msgSection, ""), diag->name()) )
               break;
         }

	 if (pSEQPrefs->getPrefBool("SavePosition", msgSection, false)) 
	 {
	   pSEQPrefs->setPrefPoint("WindowPos", msgSection, 
				 diag->pos());
	   pSEQPrefs->setPrefSize("WindowSize", msgSection, 
				  diag->size());
	 }
      }

      section = "Interface";
      if (pSEQPrefs->getPrefBool("SavePosition", section, false)) 
      {
	pSEQPrefs->setPrefPoint("WindowPos", section, 
			      topLevelWidget()->pos());
	pSEQPrefs->setPrefSize("WindowSize", section, 
			       topLevelWidget()->size());
      }

      QValueList<int> list = m_splitV->sizes();
      QValueList<int>::Iterator it = list.begin();
      i = 0;
      while(it != list.end()) {
         i++;
         tempStr.sprintf("SplitVSize%d", i);
         pSEQPrefs->setPrefInt(tempStr, section, (*it));
         ++it;
      }

      list = m_splitH->sizes();
      it = list.begin();
      i = 0;
      while(it != list.end()) {
         i++;
         tempStr.sprintf("SplitHSize%d", i);
         pSEQPrefs->setPrefInt(tempStr, section, (*it));
         ++it;
      }

      list = m_splitT->sizes();
      it = list.begin();
      i = 0;
      while(it != list.end()) {
         i++;
         tempStr.sprintf("SplitT_Size%d", i);
         pSEQPrefs->setPrefInt(tempStr, section, (*it));
         ++it;
      }

      // save prefs to file
      pSEQPrefs->save();
   }
} // end savePrefs

void EQInterface::setCaption(const QString& text)
{
  QMainWindow::setCaption(text);

  pSEQPrefs->setPrefString("Caption", "Interface", caption());
}


void EQInterface::loadFormatStrings()
{
  // clear out any existing contents
  m_formattedMessageStrings.setAutoDelete(true);
  m_formattedMessageStrings.clear();

  // get the name of the format file
  QString formatFileName = pSEQPrefs->getPrefString("FormatFile", "Interface", 
						    LOGDIR "/eqstr_en.txt");

  // create a QFile on the file
  QFile formatFile(formatFileName);

  // open the file read only
  if (formatFile.open(IO_ReadOnly))
  {
    // create a text stream on the file
    QTextStream fft(&formatFile);

    // read in the magic id string
    QString magicString = fft.readLine();
    int tmp;
    int count;
    int formatId;
    QString formatString;

    // read in the number of entries
    fft >> tmp >> count;

    //fprintf(stderr, "Reading %d formatted strings\n", count);
    
    // read count format strings in
    while (count--)
    {
      // read in the format string id
      fft >> formatId >> ws;

      // read in the format string
      formatString = fft.readLine();
      
      // insert a message format indexed by the formatId
      m_formattedMessageStrings.insert(formatId, new QString(formatString));
    }
  }
}

/* Capture resize events and reset the geometry */
void
EQInterface::resizeEvent (QResizeEvent *e)
{
}

void
EQInterface::select_filter_file(void)
{
  QString filterFile = QFileDialog::getOpenFileName( QString(LOGDIR),
                                                     QString("ShowEQ Filter Files (*.conf)"),
                                                     0,
                                                     "Select Filter Config..."
                                                   );
  if (!filterFile.isEmpty())
    m_filterMgr->loadFilters(filterFile);
}

void EQInterface::listSpawns (void)
{
#ifdef DEBUG
  debug ("listSpawns()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);
  
   // dump the coin spawns 
  m_spawnShell->dumpSpawns(tSpawn, out);
}

void EQInterface::listDrops (void)
{
#ifdef DEBUG
  debug ("listDrops()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tDrop, out);
}

void EQInterface::listCoins (void)
{
#ifdef DEBUG
  debug ("listCoins()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tCoins, out);
}

void EQInterface::listMapInfo(void)
{
#ifdef DEBUG
  debug ("listMapInfo()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

  // dump map managers info
  m_mapMgr->dumpInfo(out);

  // iterate over all the maps
  for (int i = 0; i < maxNumMaps; i++)
  {
    // if this map has been instantiated, dump it's info
    if (m_map[i] != NULL)
      m_map[i]->dumpInfo(out);
  }
}

void EQInterface::dumpSpawns (void)
{
#ifdef DEBUG
  debug ("dumpSpawns()");
#endif /* DEBUG */
  
  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
				      LOGDIR "/dumpspawns.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);
  
  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tSpawn, out);
}

void EQInterface::dumpDrops (void)
{
#ifdef DEBUG
  debug ("dumpDrops()");
#endif /* DEBUG */
  
  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
				      LOGDIR "/dumpdrops.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tDrop, out);
}

void EQInterface::dumpCoins (void)
{
#ifdef DEBUG
  debug ("dumpCoins()");
#endif /* DEBUG */

  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
				      LOGDIR "/dumpcoins.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tCoins, out);
}

void EQInterface::dumpMapInfo(void)
{
#ifdef DEBUG
  debug ("dumpMapInfo()");
#endif /* DEBUG */

  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpMapInfoFilename", "Interface",
				      LOGDIR "/mapinfo.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump map managers info
  m_mapMgr->dumpInfo(out);

  // iterate over all the maps
  for (int i = 0; i < maxNumMaps; i++)
  {
    // if this map has been instantiated, dump it's info
    if (m_map[i] != NULL)
      m_map[i]->dumpInfo(out);
  }
}

void
EQInterface::launch_editor_filters(void)
{
  EditorWindow * ew = new EditorWindow(m_filterMgr->filterFile());
  ew->setCaption(m_filterMgr->filterFile());
  ew->show();
}

void
EQInterface::toggle_opt_ConSelect (void)
{
  showeq_params->con_select = !(showeq_params->con_select);
  menuBar()->setItemChecked (m_id_opt_ConSelect, showeq_params->con_select);
  pSEQPrefs->setPrefBool("SelectOnCon", "Interface", showeq_params->con_select);
}

void
EQInterface::toggle_opt_TarSelect (void)
{
  showeq_params->tar_select = !(showeq_params->tar_select);
  menuBar()->setItemChecked (m_id_opt_TarSelect, showeq_params->tar_select);
  pSEQPrefs->setPrefBool("SelectOnTarget", "Interface", showeq_params->tar_select);
}

void
EQInterface::toggle_opt_Fast (void)
{
  showeq_params->fast_machine = !(showeq_params->fast_machine);
  menuBar()->setItemChecked (m_id_opt_Fast, showeq_params->fast_machine);

  pSEQPrefs->setPrefBool("FastMachine", "Interface", showeq_params->fast_machine);
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
    showeq_params->logAllPackets = !showeq_params->logAllPackets;
    menuBar()->setItemChecked (m_id_log_AllPackets, showeq_params->logAllPackets);
  pSEQPrefs->setPrefBool("LogAllPackets", "PacketLogging", showeq_params->logAllPackets);
}

void EQInterface::toggle_log_ZoneData (void)
{
    showeq_params->logZonePackets = !showeq_params->logZonePackets;
    menuBar()->setItemChecked (m_id_log_ZoneData, showeq_params->logZonePackets);
  pSEQPrefs->setPrefBool("LogZonePackets", "PacketLogging", showeq_params->logZonePackets);
}

void EQInterface::toggle_log_UnknownData (void)
{
    showeq_params->logUnknownZonePackets = !showeq_params->logUnknownZonePackets;
    menuBar()->setItemChecked (m_id_log_UnknownData, showeq_params->logUnknownZonePackets);
  pSEQPrefs->setPrefBool("LogUnknownZonePackets", "PacketLogging", showeq_params->logUnknownZonePackets);
}


/* Check and uncheck View menu options */
void
EQInterface::toggle_view_ChannelMsgs (void)
{
    m_viewChannelMsgs = !m_viewChannelMsgs;
    menuBar()->setItemChecked (m_id_view_ChannelMsgs, m_viewChannelMsgs);
    /* From Daisy, hide Channel Messages if the view flag is false */
    for(MsgDialog *diag=m_msgDialogList.first(); diag != 0; diag=m_msgDialogList.next() ) {
      if (m_viewChannelMsgs) {
	diag->show();
      }
      else {
	diag->hide();
      }
    }
}

void
EQInterface::toggle_view_UnknownData (void)
{
    viewUnknownData = !viewUnknownData;
    menuBar()->setItemChecked (m_id_view_UnknownData, viewUnknownData);
    m_packet->setViewUnknownData (viewUnknownData);
}

void EQInterface::toggle_view_ExpWindow (void)
{
    if (!m_expWindow->isVisible())
    {
       menuBar()->setItemChecked (m_id_view_ExpWindow,1);
       m_expWindow->show();
       // set exp window location
       QString section = "Experience";
       QSize s = pSEQPrefs->getPrefSize("WindowSize", section,
					m_expWindow->size());

       m_expWindow->resize(s);

       // move window to new position
       if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0))
       {
	 QPoint p = pSEQPrefs->getPrefPoint("WindowPos", section, 
					    m_expWindow->pos());
	 m_expWindow->move(p);
       }
    }
    else
    {
       menuBar()->setItemChecked (m_id_view_ExpWindow,0); 
       m_expWindow->hide();
    }
}

void EQInterface::toggle_view_CombatWindow (void)
{
    if (!m_combatWindow->isVisible())
    {
       menuBar()->setItemChecked (m_id_view_CombatWindow, 1);
       m_combatWindow->show();
       QString section = "Combat";

       // set combat window location
       QSize s = pSEQPrefs->getPrefSize("WindowSize", section,
					m_combatWindow->size());

       m_combatWindow->resize(s);

       // move window to new position
       if (pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0))
       {
	 QPoint p = pSEQPrefs->getPrefPoint("WindowPos", section,
					    m_combatWindow->pos());

	 m_combatWindow->move(p);
       }
    }
    else
    {
       menuBar()->setItemChecked (m_id_view_CombatWindow, 0);
       m_combatWindow->hide();
    }
}


void
EQInterface::toggle_view_SpawnList(void)
{
  bool wasVisible = ((m_spawnList != NULL) && m_spawnList->isVisible());

  menuBar()->setItemChecked (m_id_view_SpawnList, !wasVisible);

  if (!wasVisible)
  {
    showSpawnList();

    for (int i = 0; i < SPAWNCOL_MAXCOLS; i++)
      m_spawnListMenu->setItemChecked(m_id_view_SpawnList_Cols[i], 
				     m_spawnList->columnWidth(i) != 0);

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

    // delete the window
    delete m_spawnList;

    // make sure to clear it's variable
    m_spawnList = NULL;
  }

  pSEQPrefs->setPrefBool("ShowSpawnList", "Interface", !wasVisible);
}

void EQInterface::toggle_view_SpellList(void)
{
  bool wasVisible = ((m_spellList != NULL) && (m_spellList->isVisible()));

  menuBar()->setItemChecked (m_id_view_SpellList, !wasVisible);
  
  if (!wasVisible)
  {
    showSpellList();

    QString section = "SpellList";
    QSize s = pSEQPrefs->getPrefSize("WindowSize", section, 
				     m_spellList->size());
    m_spellList->resize(s);
    
    // only do this move stuff iff the spell list isn't docked
    // and the user set the option to do so.
    if (!m_isSpellListDocked && 
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
    {
      // Set window location
      QPoint p = pSEQPrefs->getPrefPoint("WindowPos", section, 
					 m_spellList->pos());
      
      // Move window to new position
      m_spellList->move(p);
    }
  }
  else
  {
    // save it's preferences
    m_spellList->savePrefs();
    
    // hide it
    m_spellList->hide();

    // delete it
    delete m_spellList;
    
    // make sure to clear it's variable
    m_spellList = NULL;
  }

  pSEQPrefs->setPrefBool("ShowSpellList", "Interface", !wasVisible); 
}

void EQInterface::toggle_view_PlayerStats(void)
{
  bool wasVisible = ((m_statList != NULL) && m_statList->isVisible());

  menuBar()->setItemChecked (m_id_view_PlayerStats, !wasVisible);

  if (!wasVisible)
  {
    showStatList();

    for (int i = 0; i < LIST_MAXLIST; i++)
      m_statWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[i], 
				   m_statList->statShown(i));

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

    // then delete it
    delete m_statList;

    // make sure to clear it's variable
    m_statList = NULL;
  }
  
  pSEQPrefs->setPrefBool("ShowPlayerStats", "Interface", !wasVisible);
}

void EQInterface::toggle_view_PlayerSkills(void)
{
  bool wasVisible = ((m_skillList != NULL) && m_skillList->isVisible());

  menuBar()->setItemChecked (m_id_view_PlayerSkills, !wasVisible);

  if (!wasVisible)
  {
    showSkillList();

    menuBar()->setItemChecked(m_id_view_PlayerSkills_Languages, 
			      m_skillList->showLanguages());
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

    // then delete it
    delete m_skillList;

    // make sure to clear it's variable
    m_skillList = NULL;
  }

  pSEQPrefs->setPrefBool("ShowPlayerSkills", "Interface", !wasVisible);
}

void
EQInterface::toggle_view_Compass(void)
{
  bool wasVisible = ((m_compass != NULL) && (m_compass->isVisible()));

  menuBar()->setItemChecked (m_id_view_Compass, !wasVisible);

  if (!wasVisible)
    showCompass();
  else
  {
    // if it's not visible, hide it
    m_compass->hide();

    // then delete it
    delete m_compass;

    // make sure to clear it's variable
    m_compass = NULL;
  }

  pSEQPrefs->setPrefBool("ShowCompass", "Interface", !wasVisible);
}

void EQInterface::toggle_view_Map(int id)
{
  int mapNum = menuBar()->itemParameter(id);

  bool wasVisible = ((m_map[mapNum] != NULL) && 
		     (m_map[mapNum]->isVisible()));

  menuBar()->setItemChecked(m_id_view_Map[mapNum], !wasVisible);
  
  if (!wasVisible)
    showMap(mapNum);
  else
  {
    // save any preference changes
    m_map[mapNum]->savePrefs();

    // hide it 
    m_map[mapNum]->hide();

    // then delete it
    delete m_map[mapNum];

    // make sure to clear it's variable
    m_map[mapNum] = NULL;
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
  bool wasVisible = ((m_netDiag != NULL) && (m_netDiag->isVisible()));

  menuBar()->setItemChecked (m_id_view_NetDiag, !wasVisible);

  if (!wasVisible)
    showNetDiag();
  else
  {
    // if it's not visible, hide it
    m_netDiag->hide();

    // then delete it
    delete m_netDiag;

    // make sure to clear it's variable
    m_netDiag = NULL;
  }
}

void
EQInterface::ToggleOpCodeMonitoring (int id)
{
  if(!showeq_params->monitorOpCode_Usage)
  {
    if (!showeq_params->monitorOpCode_List.isEmpty())
    {
        if (m_packet->m_bOpCodeMonitorInitialized == false)
          m_packet->InitializeOpCodeMonitor();

        pSEQPrefs->setPrefBool ("Enable", "OpCodeMonitoring", 1 );
        showeq_params->monitorOpCode_Usage = true;

        printf("OpCode monitoring is now ENABLED...\nUsing list:\t%s\n", 
	       (const char*)showeq_params->monitorOpCode_List);

        menuBar()->changeItem( id, "Disable &OpCode Monitoring" );
    }

    else
      QMessageBox::critical(this, "ERROR", "Unable to enable OpCode monitoring!!!\n\n"
                                               "It would appear as if you have not yet set the\n"
                                               "List value under the [OpCodeMonitoring] section\n"
                                               "of your ShowEQ.conf file...\n\n"
                                               "Please reffer to ShowEQ's instructions for more details...\0"
                               );
  }

  else
  {
    pSEQPrefs->setPrefBool ("Enable", "OpCodeMonitoring", 0);
    showeq_params->monitorOpCode_Usage = false;

    printf("OpCode monitoring has been DISABLED...\n");

    menuBar()->changeItem( id, "Enable &OpCode Monitoring" );
  }
}

void EQInterface::ReloadMonitoredOpCodeList (void)
{
  pSEQPrefs->revert();
  showeq_params->monitorOpCode_List = 
    pSEQPrefs->getPrefString ("List", "OpCodeMonitoring", "");
  m_packet->InitializeOpCodeMonitor();
  printf("The monitored OpCode list has been reloaded...\nUsing list:\t%s\n", 
	 (const char*)showeq_params->monitorOpCode_List);
}

void EQInterface::resetMaxMana(void)
{
  if (m_statList != NULL)
    m_statList->resetMaxMana();
}

void
EQInterface::toggle_opt_LogSpawns (void)
{
    showeq_params->logSpawns = !(showeq_params->logSpawns);
    menuBar()->setItemChecked (m_id_opt_LogSpawns, showeq_params->logSpawns);
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
EQInterface::createMessageBox(void)
{
  MsgDialog* pMsgDlg = new MsgDialog(parentWidget(), 
                                "MessageBox", m_StringList);
  m_msgDialogList.append(pMsgDlg);

  // connect signal for new messages
  connect (this, SIGNAL (newMessage(int)), pMsgDlg, SLOT (newMessage(int)));

  pMsgDlg->show();
}


void
EQInterface::msgReceived(const QString &instring)
{
  //
  // getting reports of a lot of blank lines being spammed about....
  // thinking its CR's in the msgs themselves.  Putting a CR/LF stripper here
  // WARNING:  If you send a msg with a CR or LF in the middle of it, this will
  // replace it with a space.  So far, I don't think we do that anywhere.
  //  - Maerlyn 3/28
  QString string(instring);
  int index = 0;

  if (string.left(28) != "System: Players in EverQuest") {
  while( -1 != (index = string.find('\n')) )
    string.replace(index, 1, " ");
  //  string.remove(index);
  while( -1 != (index = string.find('\r')) )
    string.replace(index, 1, " ");
  //  string.remove(index);
   }

  if (pSEQPrefs->getPrefBool("UseStdout", "Interface")) 
  {
    	if (!strncmp(string.ascii(), "Guild", 5)) fprintf(stdout, "\e[1;32m"); // Light Green
	if (!strncmp(string.ascii(), "Group", 5)) fprintf(stdout, "\e[0;36m"); // Cyan
	if (!strncmp(string.ascii(), "Shout", 5)) fprintf(stdout, "\e[1;31m"); // Light Red
	if (!strncmp(string.ascii(), "Aucti", 5)) fprintf(stdout, "\e[1;42m"); // Light White on Green Background
	if (!strncmp(string.ascii(), "OOC",   3)) fprintf(stdout, "\e[0;32m"); // Green
	if (!strncmp(string.ascii(), "Tell",  4)) fprintf(stdout, "\e[0;35m"); // Magenta
	if (!strncmp(string.ascii(), "Say",   3)) fprintf(stdout, "\e[1;37m"); // White
	if (!strncmp(string.ascii(), "GM-Te", 5)) fprintf(stdout, "\e[5;31m"); // Blinking Red

	fprintf(stdout, "%s", string.ascii());
	fprintf(stdout, "\e[0;0m\n"); // Shut off all ANSI
    	fflush(stdout);
  }

  m_StringList.append(string);

  emit 
    newMessage(m_StringList.count() - 1);
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
EQInterface::resetPacket(int num)
{
  // if passed 0 reset the average
  m_packetStartTime = mTime();
  m_initialcount = num;
}

void
EQInterface::numPacket(int num)
{
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

void EQInterface::attack2Hand1(const attack2Struct * atk2)
{
}

void EQInterface::action2Message(const action2Struct *action2)
{
	emit combatSignal(action2->target, action2->source, action2->type, action2->spell, action2->damage);
}


void EQInterface::itemShop(const itemInShopStruct* items)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&items->item);
  
  tempStr = QString("Item Shop: ") + items->item.lore + "(" 
    + QString::number(items->item.itemNr) + "), Value: "
    + reformatMoney(items->item.cost);
  
  emit msgReceived(tempStr);
  
  if (items->itemType == 1)
  {
    tempStr = QString("Item Shop: Container: Slots: ") 
      + QString::number(items->item.container.numSlots)
      + ", Size Capacity: " 
      + size_name(items->item.container.sizeCapacity)
      + ", Weight Reduction: "
      + QString::number(items->item.container.weightReduction)
      + "%";
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::moneyOnCorpse(const moneyOnCorpseStruct* money)
{
  QString tempStr;

  if( money->platinum || money->gold || money->silver || money->copper )
  {
    bool bneedComma = false;
    
    tempStr = "Money: You receive ";
    
    if(money->platinum)
    {
      tempStr += QString::number(money->platinum) + " platinum";
      bneedComma = true;
    }
    
    if(money->gold)
    {
      if(bneedComma)
	tempStr += ", ";
      
      tempStr += QString::number(money->gold) + " gold";
      bneedComma = true;
    }
    
    if(money->silver)
    {
      if(bneedComma)
	tempStr += ", ";
      
      tempStr += QString::number(money->silver) + " silver";
      bneedComma = true;
    }
    
    if(money->copper)
      {
	if(bneedComma)
	  tempStr += ", ";
	
	tempStr += QString::number(money->copper) + " copper";
      }
    
    tempStr += " from the corpse";
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::itemPlayerReceived(const itemOnCorpseStruct* itemc)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemc->item);
  
  tempStr = QString("Item Looted: ") + itemc->item.lore
    + "(" + QString::number(itemc->item.itemNr)
    + "), Value: " + reformatMoney(itemc->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::tradeItemOut(const tradeItemOutStruct* itemt)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemt->item);
  
  tempStr = QString("Item Trade [OUT]: ") + itemt->item.lore
    + "(" + QString::number(itemt->item.itemNr)
    + "), Value: "
    + reformatMoney(itemt->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::tradeItemIn(const tradeItemInStruct* itemr)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemr->item);
  
  tempStr = QString("Item Trade [IN]: ")
    + itemr->item.lore
    + "(" + QString::number(itemr->item.itemNr)
    + "), Value: "
    + reformatMoney(itemr->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::wearItem(const playerItemStruct* itemp)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemp->item);
  
  if (!showeq_params->no_bank)
  {
    tempStr = QString("Item: ") + itemp->item.lore
      + "(" + QString::number(itemp->item.itemNr)
      + "), Slot: " + QString::number(itemp->item.equipSlot)
      + ", Value: " + reformatMoney(itemp->item.cost);
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::summonedItem(const summonedItemStruct* itemsum)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemsum->item);
  
  tempStr = QString("ITEMSUM: ") + itemsum->item.lore
    + "(" + QString::number(itemsum->item.itemNr)
    + "), Value: " + reformatMoney(itemsum->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::channelMessage(const channelMessageStruct* cmsg, uint32_t, uint8_t dir)
{
  QString tempStr;

  // avoid client chatter and do nothing if not viewing channel messages
  if ((dir == DIR_CLIENT) || !m_viewChannelMsgs)
    return;

  bool target = false;
  switch (cmsg->chanNum)
  {
  case 0:
    tempStr.sprintf("Guild");
    break;
    
  case 2:
    tempStr.sprintf("Group");
    break;
    
  case 3:
    tempStr.sprintf("Shout");
    break;
    
  case 4:
    tempStr.sprintf("Auction");
    break;
    
  case 5:
    tempStr.sprintf("OOC");
    break;
    
  case 7:
    tempStr.sprintf("Tell");
    target = true;
    break;
    
  case 8:
    tempStr.sprintf("Say");
    target = true;
    break;
    
  case 14:
    tempStr.sprintf("GM-Tell");
    target = true;
    break;
    
  default:
    tempStr.sprintf("Chan%02x", cmsg->chanNum);
    target = true;
    break;
  }
  
  if (cmsg->language)
  {
    if ((cmsg->target[0] != 0) && target)
    {
      tempStr.sprintf( "%s: '%s' -> '%s' - %s {%s}",
		       tempStr.ascii(),
		       cmsg->sender,
		       cmsg->target,
		       cmsg->message,
		       (const char*)language_name(cmsg->language)
		       );
    }
    else
    {
      tempStr.sprintf( "%s: '%s' - %s {%s}",
		       tempStr.ascii(),
		       cmsg->sender,
		       cmsg->message,
		       (const char*)language_name(cmsg->language)
		       );
    }
  }
  else // Don't show common, its obvious
  {
    if ((cmsg->target[0] != 0) && target)
    {
      tempStr.sprintf( "%s: '%s' -> '%s' - %s",
		       tempStr.ascii(),
		       cmsg->sender,
		       cmsg->target,
		       cmsg->message
		       );
    }
    else
    {
      tempStr.sprintf( "%s: '%s' - %s",
		       tempStr.ascii(),
		       cmsg->sender,
		       cmsg->message
		       );
    }
  }
  
  emit msgReceived(tempStr);
}

void EQInterface::formattedMessage(const formattedMessageStruct* fmsg, uint32_t len, uint8_t dir)
{
  QString tempStr;

  // avoid client chatter and do nothing if not viewing channel messages
  if ((dir == DIR_CLIENT) || !m_viewChannelMsgs)
    return;

  int messagesLen = 
    len
    - ((uint8_t*)&fmsg->messages[0] - (uint8_t*)fmsg) 
    - sizeof(fmsg->unknownXXXX);

  QString* formatStringRes = m_formattedMessageStrings.find(fmsg->messageFormat);

  if (formatStringRes == NULL)
  {
    tempStr.sprintf( "Formatted: %04x: %s",
		     fmsg->messageFormat,
		     fmsg->messages);

    int totalArgsLen = strlen(fmsg->messages) + 1;
    
    const char* curMsg;
    while (totalArgsLen < messagesLen)
    {
      curMsg = fmsg->messages + totalArgsLen;
      tempStr += QString(", ") + curMsg;
      totalArgsLen += strlen(curMsg) + 1;
    }
  }
  else
  {
    tempStr = QString("Formatted: ") + *formatStringRes;
    int totalArgsLen = 0;
    const char* curMsg;
    while (totalArgsLen < messagesLen)
    {
      curMsg = fmsg->messages + totalArgsLen;
      tempStr = tempStr.arg(curMsg);
      totalArgsLen += strlen(curMsg) + 1;
    }
  }

  emit msgReceived(tempStr);
}

void EQInterface::random(const randomStruct* randr)
{
  QString tempStr;

  tempStr.sprintf ( "RANDOM: Request random number between %d and %d\n",
		    randr->bottom,
		    randr->top);
  
  emit msgReceived(tempStr);
}

void EQInterface::emoteText(const emoteTextStruct* emotetext)
{
  QString tempStr;

  if (!m_viewChannelMsgs)
    return;

  tempStr.sprintf("Emote: %s", emotetext->text);

  emit msgReceived(tempStr);
}

void EQInterface::playerBook(const playerBookStruct* bookp)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&bookp->item);
  
  if (!showeq_params->no_bank)
  {
    tempStr = QString("Item: Book: ") + bookp->item.name
      + ", " + bookp->item.lore
      + ", " + bookp->item.book.file
      + ", Value: " + reformatMoney(bookp->item.cost);
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::playerContainer(const playerContainerStruct *containp)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&containp->item);
  
  if (!showeq_params->no_bank)
  {
    tempStr = QString("Item: Container: ") + containp->item.lore
      + "(" + QString::number(containp->item.itemNr)
      + "), Slot: " + QString::number(containp->item.equipSlot)
      + ", Value: " + reformatMoney(containp->item.cost);
    
    emit msgReceived(tempStr);
    
    tempStr = QString("Item: Container: Slots: ") 
      + QString::number(containp->item.container.numSlots)
      + ", Size Capacity: " 
      + size_name(containp->item.container.sizeCapacity)
      + ", Weight Reduction: " 
      + QString::number(containp->item.container.weightReduction)
      + "%";
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::inspectData(const inspectDataStruct *inspt)
{
  for (int inp = 0; inp < 21; inp ++)
    printf("He has %s (icn:%i)\n", inspt->itemNames[inp], inspt->icons[inp]);
  
  printf("His info: %s\n", inspt->mytext);
}

void EQInterface::spMessage(const spMesgStruct *spmsg)
{
  QString tempStr;

  // Seems to be lots of blanks
  if (!spmsg->message[0])
    return;
  
  //printf("== msgType=%d, msg='%s'\n", spmsg->msgType, spmsg->message);
  // This seems to be several type of message...
  // CJD - why was there no breaks in each case?
  switch(spmsg->msgType)
  {
  case 13:
  {
    if (!strncmp(spmsg->message, "Your target is too far away", 20))
      tempStr.sprintf("Attack: %s", spmsg->message);
    
    else if (!strncmp(spmsg->message, "You can't see your target", 18))
      tempStr.sprintf("Attack: %s", spmsg->message);
    
    // CJD - is this resist any longer type 13, or was it
    // moved to 289 (what I see from some brief logging)?
    else if (!strncmp(spmsg->message, "Your target resisted", 18))
    {
      printf("== let cjd1@users.sourceforge.net know if you ever see this message.\n");
      tempStr.sprintf("Spell: %s", spmsg->message);
    }
    else
      tempStr.sprintf("Special: %s (%d)", spmsg->message, spmsg->msgType);
    break;
  }
  case 15:
    tempStr.sprintf("\e[0;33mExp: %s\e[0;0m", spmsg->message);
    break;
    
    // CJD TODO - make these signals themselves? or just one
    // spellMessage(QString&) and let spellshell split them up...
  case 284: // Your xxx spell has worn off.
    // CJD - No way with this to tell which spell wore off
    // if the same spell is on two different targets...
    tempStr.sprintf("Spell: %s", spmsg->message);
    break;
    
    // Your target resisted the xxx spell.  OR
    // Your spell fizzles.
  case 289:
    tempStr.sprintf("Spell: %s", spmsg->message);
    break;
    
  default:
    tempStr.sprintf("Special: %s (%d)", spmsg->message, spmsg->msgType);
  }
  
  if (tempStr.left(6) == "Spell:")
    emit spellMessage(tempStr);
  
  emit msgReceived(tempStr);
}

void EQInterface::handleSpell(const memSpellStruct* mem, uint32_t, uint8_t dir)
{
  QString tempStr;

  if (!showeq_params->showSpellMsgs)
    return;
  
  bool client = (dir == DIR_CLIENT);

  tempStr = "";
  
  switch (mem->param2)
  {
  case 0:
    {
      if (!client)
	tempStr = "You have finished scribing '";
      break;
    }
    
  case 1:
    {
      if (!client)
	tempStr = "You have finished memorizing '";
      break;
    }
    
  case 2:
    {
      if (!client)
	tempStr = "You forget '";
      break;
    }
    
  case 3:
    {
      if (!client)
	tempStr = "You finish casting '";
      break;
    }
    
  default:
    {
      tempStr.sprintf( "Unknown Spell Event ( %s ) - '",
		       client  ?
		     "Client --> Server"   :
		       "Server --> Client"
		       );
      break;
    }
  }
  
  
  if (!tempStr.isEmpty())
  {
    if (mem->param2 != 3)
      tempStr.sprintf("SPELL: %s%s', slot %d.", 
		      tempStr.ascii(), 
		      (const char*)spell_name (mem->spellId), 
		      mem->spawnId);
    else 
    {
      tempStr.sprintf("SPELL: %s%s'.", 
		      tempStr.ascii(), 
		      (const char*)spell_name (mem->spellId));
    }

    emit msgReceived(tempStr);
  }
}

void EQInterface::beginCast(const beginCastStruct *bcast)
{
  QString tempStr;

  if (!showeq_params->showSpellMsgs)
    return;
  
  tempStr = "";

  if (bcast->spawnId == m_player->getPlayerID())
    tempStr = "You begin casting '";
  else
  {
    const Item* item = m_spawnShell->findID(tSpawn, bcast->spawnId);
    if (item != NULL)
      tempStr = item->name();
    
    if (tempStr == "" || tempStr.isEmpty())
      tempStr.sprintf("UNKNOWN (ID: %d)", bcast->spawnId);
    
    tempStr += " has begun casting '";
  }
  float casttime = ((float)bcast->param1 / 1000);
  tempStr.sprintf( "SPELL: %s%s' - Casting time is %g Second%s", tempStr.ascii(),
		   (const char*)spell_name(bcast->spellId), casttime,
		   casttime == 1 ? "" : "s"
		   );
  emit msgReceived(tempStr);
}

void EQInterface::interruptSpellCast(const badCastStruct *icast)
{
  const Item* item = m_spawnShell->findID(tSpawn, icast->spawnId);

  if (item != NULL)
    printf("SPELL: %s(%d): %s\n", 
	   (const char*)item->name(), icast->spawnId, icast->message);
  else
    printf("SPELL: spawn(%d): %s\n", 
	   icast->spawnId, icast->message);
}

void EQInterface::startCast(const startCastStruct* cast)
{
  printf("SPELL: You begin casting %s.  Current Target is ", 
	 (const char*)spell_name(cast->spellId) );
  
  const Item* item = m_spawnShell->findID(tSpawn, cast->targetId);

  if (item != NULL)
    printf("%s(%d)", (const char*)item->name(), cast->targetId);
  else
    printf("spawn(%d)", cast->targetId);
  
  printf("\n");
}

void EQInterface::systemMessage(const sysMsgStruct* smsg)
{
  QString tempStr;

  // Seems to be lots of blanks
  if (!smsg->message[0])
    return;
  
  // This seems to be several type of message...
  if (!strncmp(smsg->message, "Your faction", 12))
    tempStr.sprintf("Faction: %s", smsg->message);
  else
    tempStr.sprintf("System: %s", smsg->message);
  
  emit msgReceived(tempStr);
}

void EQInterface::newGroundItem(const makeDropStruct* adrop, uint32_t, uint8_t dir)
{
  QString tempStr;

  if (dir != DIR_CLIENT)
    return;

  /* If the packet is outbound  ( Client --> Server ) then it
     should not be added to the spawn list... The server will
     send the client a packet when it has actually placed the
     item on the ground.
  */
  if (pItemDB != NULL) 
    tempStr = pItemDB->GetItemLoreName(adrop->itemNr);
  else
    tempStr = "";
  
  if (tempStr != "")
  {
    tempStr.prepend("Item: Drop: You have dropped your '");
    tempStr.append("' on the ground!");
  }
  else
    tempStr = QString("Item: Drop: You have dropped your *UNKNOWN ITEM* (ID: %1)  on the ground!\nNOTE:\tIn order for ShowEQ to know the name of the item you dropped it is suggested that you pickup and drop the item again...").arg(adrop->itemNr);
  
  emit msgReceived(tempStr);
}

void EQInterface::moneyUpdate(const moneyUpdateStruct* money)
{
  emit msgReceived("Money: Update");
}

void EQInterface::moneyThing(const moneyThingStruct* money)
{
  emit msgReceived("Money: Thing");
}

void EQInterface::groupInfo(const groupMemberStruct* gmem)
{
  printf ("Member: %s - %s (%i)\n", 
	  gmem->yourname, gmem->membername, gmem->oper);
}

void EQInterface::zoneEntry(const ClientZoneEntryStruct* zsentry)
{
  QString tempStr;
#ifdef ZONE_ORDER_DIAG
  tempStr = "Zone: EntryCode: Client";
  emit msgReceived(tempStr);
#endif
}

void EQInterface::zoneEntry(const ServerZoneEntryStruct* zsentry)
{
  QString tempStr;

#ifdef ZONE_ORDER_DIAG
  tempStr = "Zone: EntryCode: Server, Zone: ";
  tempStr += zone_name(zsentry->zoneId);
  emit msgReceived(tempStr);
#endif
  tempStr = QString("Zone: Zoning, Please Wait...\t(Zone: '")
    + zone_name(zsentry->zoneId) + "')";
  emit msgReceived(tempStr);
  emit newZoneName(zone_name(zsentry->zoneId));

  m_filterMgr->loadZone(zone_name(zsentry->zoneId));
}

void EQInterface::zoneChange(const zoneChangeStruct* zoneChange, uint32_t, uint8_t dir)
{
  QString tempStr;

  stsMessage("- Busy Zoning -");
  emit newZoneName(zone_name(zoneChange->zoneId));

  if (dir == DIR_CLIENT)
  {
#ifdef ZONE_ORDER_DIAG
    tempStr = "Zone: ChangeCode: Client, Zone: ";
    tempStr += zone_name(zoneChange->zoneId);
    emit msgReceived(tempStr);
#endif
  }
  else
  {
    printf("Loading, Please Wait...\t(Zone: \'%s\')\n", (const char*)zone_name(zoneChange->zoneId));
#ifdef ZONE_ORDER_DIAG
    tempStr = "Zone: ChangeCode: Server, Zone:";
    tempStr += zone_name(zoneChange->zoneId);
    emit msgReceived(tempStr);
#endif
    tempStr = QString("Zone: Zoning, Please Wait...\t(Zone: '")
      + zone_name(zoneChange->zoneId) + "')";
    emit msgReceived(tempStr);

    m_filterMgr->loadZone(zone_name(zoneChange->zoneId));
  }
}

void EQInterface::zoneNew(const newZoneStruct* zoneNew, uint32_t, uint8_t dir)
{
  QString tempStr;

#ifdef ZONE_ORDER_DIAG
  tempStr = "Zone: NewCode: Zone: ";
  tempStr += QString(zoneNew->shortName) + " ("
    + zoneNew->longName + ")";
  emit msgReceived(tempStr);
#endif
  tempStr = QString("Zone: Entered: ShortName = '") + zoneNew->shortName +
                    "' LongName = " + zoneNew->longName;
  emit msgReceived(tempStr);

   if (pSEQPrefs->getPrefBool("UseStdout", "Interface"))
       printf("Loading Complete...\t(Zone: '%s')\n", zoneNew->shortName);

  emit newZoneName(zoneNew->longName);
  stsMessage("");

  m_filterMgr->loadZone(zoneNew->shortName);
}

void EQInterface::clientTarget(const clientTargetStruct* cts)
{
  if (!showeq_params->tar_select)
    return;

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
  if (item == NULL)
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
  if (item == NULL)
    return;

  if (!showeq_params->con_select)
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
    printf ("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    printf ("LocateSpawn: %s spawned LOC %dy, %dx, %5.1fz at %s",
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->displayZPos(),
	    (const char*)item->spawnTimeStr());
    printf ("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    
    if (showeq_params->spawnfilter_loglocates)
      logFilteredSpawn(item, FILTER_FLAG_LOCATE);

    // Deside how to handle audio alerts
    //
    makeNoise(item, "LocateSpawnAudioCommand", "Locate Spawned");
    
    // note the new selection
    m_selectedSpawn = item;
    
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);
    
    // update the spawn status
    updateSelectedSpawnStatus(m_selectedSpawn);
  } // End LOCATE Filter alerting
  
  if (filterFlags & FILTER_FLAG_CAUTION)
  {
    printf ("CautionSpawn: %s spawned LOC %dy, %dx, %5.1fz at %s\n", 
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->displayZPos(),
	    (const char*)item->spawnTimeStr());
    
    if (showeq_params->spawnfilter_logcautions)
      logFilteredSpawn(item, FILTER_FLAG_CAUTION);

    // Deside how to handle audio alerts
    //
    makeNoise(item, "CautionSpawnAudioCommand", "Caution Spawned");

  } // End CAUTION Filter alerting
  
  if (filterFlags & FILTER_FLAG_HUNT)
  {
    printf ("HuntSpawn: %s spawned LOC %dy, %dx, %5.1fz at %s\n", 
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->displayZPos(),
	    (const char*)item->spawnTimeStr());
    
    if (showeq_params->spawnfilter_loghunts)
      logFilteredSpawn(item, FILTER_FLAG_HUNT);

    // Deside how to handle audio alerts
    //
    makeNoise(item, "HuntSpawnAudioCommand", "Hunt Spawned");

  } // End HUNT Filter alerting
  
  if (filterFlags & FILTER_FLAG_DANGER)
  {
    printf ("DangerSpawn: %s spawned LOC %dy, %dx, %5.1fz at %s", 
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->displayZPos(),
	    (const char*)item->spawnTimeStr());
    
    if (showeq_params->spawnfilter_logdangers)
      logFilteredSpawn(item, FILTER_FLAG_DANGER);

    // Deside how to handle audio alerts
    //
    makeNoise(item, "DangerSpawnAudioCommand", "Danger Spawned");
  } // End DANGER Filter alerting
}


void EQInterface::delItem(const Item* item)
{
  // if this is the selected spawn, then there isn't a selected spawn anymore
  if (m_selectedSpawn == item)
  {
    m_selectedSpawn = NULL;
  
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);
  }
}

void EQInterface::killSpawn(const Item* item)
{
  if (item == NULL)
    return;

  if (item->id() == m_player->getPlayerID())
    printf("Player died at y:%d, x:%d, z:%d\n", item->yPos(), item->xPos(),
      item->zPos());

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

void EQInterface::handleAlert(const Item* item, 
			      alertType type)
{
  QString prefix;
  switch (type)
    {
    case tNewSpawn:
      prefix = "Spawn:";
      break;
    case tFilledSpawn:
      prefix = "Filled:";
      break;
    case tKillSpawn:
      prefix = "Died:";
      break;
    case tDelSpawn:
      prefix = "DeSpawn:";
      break;
    default:
      prefix = "WTF:";
    }

  QString msg;
  if (pSEQPrefs->getPrefBool("AlertInfo", "Filters"))
  {
    long timeval;
    struct tm *tp;
    
    time(&timeval);
    tp=localtime(&timeval);

    QString temp;

    msg = prefix + item->name() + "/" + item->raceName() 
      + "/" + item->className();

    const Spawn* spawn = spawnType(item);

    if (spawn)
      msg += QString("/") + spawn->lightName();

    // aditional info or new spawns
    if (type == tNewSpawn)
    {
      if (spawn)
	temp.sprintf(" [%d-%d-%d %d:%d:%d] (%d,%d,%5.1f) LVL %d, HP %d/%d", 
		     1900 + tp->tm_year, tp->tm_mon, tp->tm_mday,
		     tp->tm_hour, tp->tm_min, tp->tm_sec,
		     item->xPos(), item->yPos(), item->displayZPos(),
		     spawn->level(), spawn->HP(), spawn->maxHP());
      else
	temp.sprintf(" [%d-%d-%d %d:%d:%d] (%d,%d,%5.1f)", 
		     1900 + tp->tm_year, tp->tm_mon, tp->tm_mday,
		     tp->tm_hour, tp->tm_min, tp->tm_sec,
		     item->xPos(), item->yPos(), item->displayZPos());
    }
    else
      temp.sprintf(" [%d-%d-%d %d:%d:%d]", 
		    1900 + tp->tm_year, tp->tm_mon, tp->tm_mday,
		    tp->tm_hour, tp->tm_min, tp->tm_sec);
      
    msg += temp;
  }
  else
    msg = prefix + item->name();

  emit msgReceived(msg);

  // Gereric system beep for those without a soundcard
  //
  if (!pSEQPrefs->getPrefBool("SpawnFilterAudio", "Filters"))
	{
    fprintf( stderr, "\a");
	fflush( stderr);
	}
  else
  {
    // execute a command
    QString command;
    QString audioCmd;
    QString audioCue;
    switch (type)
    {
    case tNewSpawn:
      audioCmd = "SpawnAudioCommand";
      audioCue = "Spawned";
      break;
    case tFilledSpawn:
      audioCmd = "SpawnAudioCommand";
      audioCue = "Filled";
      break;
    case tKillSpawn:
      audioCmd = "DeathAudioCommand";
      audioCue = "Died";
      break;
    case tDelSpawn:
      audioCmd = "DeSpawnAudioCommand";
      audioCue = "Despawned";
      break;
    }
    
    doAlertCommand(item, pSEQPrefs->getPrefString(audioCmd, "Filters"), audioCue);
  }
}

// Deside how to handle audio alerts
//
void EQInterface::makeNoise( const Item* item, 
			     char* szAudioCmd, 
			     char* szSoundType)
{
  if (!showeq_params->spawnfilter_audio)
  {
    fprintf(stderr,"\a");
    fflush(stderr);
  }
  else
    doAlertCommand(item, 
		   pSEQPrefs->getPrefString(szAudioCmd, "Filters"),
		   szSoundType);
}

void EQInterface::doAlertCommand(const Item* item, 
				 const QString& rawCommand, 
				 const QString& audioCue)
{
  if (rawCommand.isEmpty())
    return;

  // time to cook the command line
  QString command = rawCommand;

  QRegExp nameExp("%n");
  QRegExp cueExp("%c");
  
  // get the name, and replace all occurrances of %n with the name
  QString name = item->transformedName();
  command.replace(nameExp, name);
  
  // now, replace all occurrances of %c with the audio cue
  command.replace(cueExp, audioCue);
  
  // fire off the command
  system ((const char*)command);
}

void EQInterface::logFilteredSpawn(const Item* item, uint32_t flag)
{
  FILE *rar;
  rar = fopen("/usr/local/share/showeq/filtered.spawns","at");
  if (rar) 
  {
    fprintf (rar, "%s %s spawned LOC %dy, %dx, %dz at %s", 
	     (const char*)m_filterMgr->filterString(flag),
	     (const char*)item->name(), 
	     item->yPos(), item->xPos(), item->zPos(),
	     (const char*)item->spawnTimeStr());
    fclose(rar);
  }
}

void EQInterface::updateSelectedSpawnStatus(const Item* item)
{
  if (item == NULL)
    return;

  const Spawn* spawn = spawnType(item);

  // construct a message for the status message display
  QString string("");
  if (spawn != NULL)
    string.sprintf("%d: %s:%d (%d/%d) Pos:", // "%d/%d/%d (%d) %s %s Item:%s", 
		   item->id(),
		   (const char*)item->name(), 
		   spawn->level(), spawn->HP(),
		   spawn->maxHP());
  else
    string.sprintf("%d: %s: Pos:", // "%d/%d/%d (%d) %s %s Item:%s", 
		   item->id(),
		   (const char*)item->name());

  if (showeq_params->retarded_coords)
    string += QString::number(item->yPos()) + "/" 
      + QString::number(item->xPos()) + "/" 
      + QString::number(item->displayZPos(), 'f', 1);
  else
    string += QString::number(item->xPos()) + "/" 
      + QString::number(item->yPos()) + "/" 
      + QString::number(item->displayZPos(), 'f', 1);

  string += QString(" (") 
    + QString::number(item->calcDist(m_player->getPlayerX(),
				     m_player->getPlayerY(),
				     m_player->getPlayerZ()))
    + ") " + item->raceName() + " " + item->className();

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
    m_spawnList->rebuildSpawnList();
}

void EQInterface::selectNext(void)
{
  if (m_spawnList)
    m_spawnList->selectNext();
}

void EQInterface::selectPrev(void)
{
  if (m_spawnList)
    m_spawnList->selectPrev();
}

void EQInterface::toggle_net_real_time_thread(int id)
{
   showeq_params->realtime = !showeq_params->realtime;
   m_netMenu->setItemChecked(id, showeq_params->realtime);
   pSEQPrefs->setPrefBool("RealTimeThread", "Network", showeq_params->realtime);
}

void EQInterface::toggle_net_broken_decode(int id)
{
   showeq_params->broken_decode = !showeq_params->broken_decode;
   m_netMenu->setItemChecked(id, showeq_params->broken_decode);
   pSEQPrefs->setPrefBool("RealTimeThread", "Network", showeq_params->broken_decode);
}

void EQInterface::set_net_monitor_next_client()
{
  // start monitoring the next client seen
  m_packet->monitorNextClient();

  // set it as the address to monitor next session
  pSEQPrefs->setPrefString("IP", "Network", showeq_params->ip);
}

void EQInterface::set_net_client_IP_address()
{
  bool ok = false;
  QString address = 
    QInputDialog::getText("ShowEQ - EQ Client IP Address",
			  "Enter IP address of EQ client:",
			  QLineEdit::Normal, showeq_params->ip,
			  &ok, this);

  if (ok)
  {
    // start monitoring the new address
    m_packet->monitorIPClient(address);

    // set it as the address to monitor next session
    pSEQPrefs->setPrefString("IP", "Network", showeq_params->ip);
  }
}

void EQInterface::set_net_client_MAC_address()
{
  bool ok = false;
  QString address = 
    QInputDialog::getText("ShowEQ - Client MAC Address",
			  "Enter MAC address of EQ client:",
			  QLineEdit::Normal, showeq_params->mac_address,
			  &ok, this);

  if (ok)
  {
    if (address.length() != 17)
    {
      fprintf(stderr, "Invalid MAC Address (%s)! Ignoring!",
	      (const char*)address);
      return;
    }

    // start monitoring the new address
    m_packet->monitorMACClient(address);

    // set it as the address to monitor next session
    pSEQPrefs->setPrefString("MAC", "Network", showeq_params->mac_address);
  }
}

void EQInterface::set_net_arq_giveup(int giveup)
{
  // set the Arq Seq Give Up length
  m_packet->setArqSeqGiveUp(giveup);

  // set it as the value to use next session
  pSEQPrefs->setPrefInt("ArqSeqGiveUp", "Network", showeq_params->arqSeqGiveUp);
}

void EQInterface::toggle_net_session_tracking()
{
   showeq_params->session_tracking = !showeq_params->session_tracking;
   m_netMenu->setItemChecked(m_id_net_sessiontrack, showeq_params->session_tracking);
   pSEQPrefs->setPrefBool("SessionTracking", "Network", showeq_params->session_tracking);
   m_packet->session_tracking();
}

void EQInterface::toggleAutoDetectCharSettings (int id)
{
    showeq_params->AutoDetectCharSettings = !showeq_params->AutoDetectCharSettings;
    menuBar()->setItemChecked (id, showeq_params->AutoDetectCharSettings);
    pSEQPrefs->setPrefBool("AutoDetectCharSettings", "Defaults", showeq_params->AutoDetectCharSettings);
}

/* Choose the character's level */
void EQInterface::SetDefaultCharacterLevel(int level)
{
    showeq_params->defaultLevel = level;
    pSEQPrefs->setPrefInt("DefaultLevel", "Defaults", level);
}

/* Choose the character's class */
void EQInterface::SetDefaultCharacterClass(int id)
{
   for (int i = 0; i < PLAYER_CLASSES; i++)
       m_charClassMenu->setItemChecked(char_ClassID[i], false );
   m_charClassMenu->setItemChecked(id, true);
   showeq_params->defaultClass = m_charClassMenu->itemParameter(id);
   pSEQPrefs->setPrefInt("DefaultClass", "Defaults", m_charClassMenu->itemParameter(id));
}

/* Choose the character's race */
void EQInterface::SetDefaultCharacterRace(int id)
{   
   for (int i = 0; i < PLAYER_RACES; i++)
       m_charRaceMenu->setItemChecked(char_RaceID[i], false );
   m_charRaceMenu->setItemChecked(id, true);
   showeq_params->defaultRace = m_charRaceMenu->itemParameter(id);
   pSEQPrefs->setPrefInt("DefaultRace", "Defaults", m_charRaceMenu->itemParameter(id));
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

void EQInterface::toggle_opt_save_DecodeKey(int id)
{
  showeq_params->saveDecodeKey = !showeq_params->saveDecodeKey;
  menuBar()->setItemChecked(id, showeq_params->saveDecodeKey);
  pSEQPrefs->setPrefBool("DecodeKey", "SaveState", 
			 showeq_params->saveDecodeKey);
}

void EQInterface::toggle_opt_save_PlayerState(int id)
{
  showeq_params->savePlayerState = !showeq_params->savePlayerState;
  menuBar()->setItemChecked(id, showeq_params->savePlayerState);
  pSEQPrefs->setPrefBool("PlayerState", "SaveState", 
			 showeq_params->savePlayerState);
}

void EQInterface::toggle_opt_save_Spawns(int id)
{
  printf ("toggle_opt_save_Spawns(id=%d)\n ", id);
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
      qApp->setStyle( (QStyle *) new QPlatinumStyle );
      qApp->setPalette( p, TRUE );
    }
    break;
    case 2: // windows
    {
      qApp->setStyle( (QStyle *) new QWindowsStyle );
      qApp->setFont( OrigFont, TRUE );
      qApp->setPalette( OrigPalette, TRUE );
    }
    break;
    case 3: // cde 
    case 4: // cde polished
    {
      QPalette p( QColor( 75, 123, 130 ) );
      qApp->setStyle( (QStyle *) new QCDEStyle( theme == 3 ? TRUE : FALSE ) );
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
      qApp->setStyle( (QStyle *) new QMotifStyle );
      qApp->setPalette( p, TRUE );
      qApp->setFont( OrigFont, TRUE );
    }
    break;
    case 6: // SGI
    {
      //QPalette p( QColor( 192, 192, 192 ) );
      qApp->setStyle( (QStyle *) new QSGIStyle( FALSE ) );
      qApp->setPalette( OrigPalette, TRUE );
      qApp->setFont( OrigFont, TRUE );
    }
    break;
    default: // system default
    {
      QPalette p( QColor( 192, 192, 192 ) );
      qApp->setStyle( (QStyle *) new QMotifStyle );
      qApp->setPalette( p, TRUE );
      qApp->setFont( OrigFont, TRUE );
      theme = 2;
    }
    break;
    }

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
  if (m_map[i] == NULL)
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

    if (m_isMapDocked[i])
      m_map[i] = new MapFrame(m_filterMgr,
			      m_mapMgr,
			      m_player, 
			      m_spawnShell, 
			      mapPrefName, 
			      mapCaption,
			      mapName, 
			      m_splitH);
    else
      m_map[i] = new MapFrame(m_filterMgr,
			      m_mapMgr,
			      m_player, 
			      m_spawnShell, 
			      mapPrefName, 
			      mapCaption,
			      mapName, 
			      NULL);
      
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_map[i], SLOT(savePrefs(void)));
    
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
    if (!m_isMapDocked[i] && 
	(pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0) != 0))
      m_map[i]->restorePosition();
  }
      
  // make sure it's visible
  m_map[i]->show();
}

void EQInterface::showSpawnList(void)
{
  // if it doesn't exist, create it.
  if (m_spawnList == NULL)
  {
    if (m_isSpawnListDocked)
      m_spawnList = new CSpawnList (m_player, m_spawnShell, m_categoryMgr,
				    m_splitV, "spawnlist");
    else
      m_spawnList = new CSpawnList (m_player, m_spawnShell, m_categoryMgr,
				    NULL, "spawnlist");

    QString section = "SpawnList";
    QSize s = pSEQPrefs->getPrefSize("WindowSize", section, 
				     m_spawnList->size());
    m_spawnList->resize(s);

    // only do this move stuff iff the spawn list isn't docked
    // and the user set the option to do so.
    if (!m_isSpawnListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
    {
      // Set window location
      QPoint p = pSEQPrefs->getPrefPoint("WindowPos", section, 
					 m_spawnList->pos());

      // Move window to new position
      m_spawnList->move(p);
    }
     // connections from spawn list to interface
     connect (m_spawnList, SIGNAL(spawnSelected(const Item*)),
	      this, SLOT(spawnSelected(const Item*)));

     // connections from interface to spawn list
     connect (this, SIGNAL(selectSpawn(const Item*)),
	      m_spawnList, SLOT(selectSpawn(const Item*)));
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_spawnList, SLOT(savePrefs(void)));
  }

  // make sure it's visible
  m_spawnList->show();
}

void EQInterface::showStatList(void)
{
  // if it doesn't exist, create it
  if (m_statList == NULL)
  {
    if (m_isStatListDocked)
      m_statList = new EQStatList(m_player, m_splitT, "stats");
    else
      m_statList = new EQStatList(m_player, NULL, "stats");

    // connect stat list slots to interface signals
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_statList, SLOT(savePrefs(void)));
  }

  // make sure it's visible
  m_statList->show();
}

void EQInterface::showSkillList(void)
{
  // if it doesn't exist, create it
  if (m_skillList == NULL)
  {
    if (m_isSkillListDocked)
      m_skillList = new EQSkillList(m_player, m_splitT, "skills");
    else
      m_skillList = new EQSkillList(m_player, NULL, "skills");

    // connect skill list slots to interfaces signals
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_skillList, SLOT(savePrefs(void)));
  }

  // make sure it's visible
  m_skillList->show();
}

void EQInterface::showSpellList(void)
{
  // if it doesn't exist, create it
  if (m_spellList == NULL)
  {
    if (m_isSpellListDocked)
      m_spellList = new SpellList(m_splitV, "spelllist");
    else
      m_spellList = new SpellList(NULL, "spelllist");

    // connect SpellShell to SpellList
    connect(m_spellShell, SIGNAL(addSpell(SpellItem *)),
	    m_spellList, SLOT(addSpell(SpellItem *)));
    connect(m_spellShell, SIGNAL(delSpell(SpellItem *)),
	    m_spellList, SLOT(delSpell(SpellItem *)));
    connect(m_spellShell, SIGNAL(changeSpell(SpellItem *)),
	    m_spellList, SLOT(changeSpell(SpellItem *)));
    connect(m_spellShell, SIGNAL(clearSpells()),
	    m_spellList, SLOT(clear()));
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_spellList, SLOT(savePrefs(void)));

  }

  // make sure it's visible
  m_spellList->show();
}

void EQInterface::showCompass(void)
{
  // if it doesn't exist, create it.
  if (m_compass == NULL)
  {
    if (m_isCompassDocked)
      m_compass = new CompassFrame(m_player, m_splitT, "compass");
    else
      m_compass = new CompassFrame(m_player, NULL, "compass");

    // supply the compass slots with EQInterface signals
     connect (this, SIGNAL(selectSpawn(const Item*)),
	      m_compass, SLOT(selectSpawn(const Item*)));
  }

  // make sure it's visible
  m_compass->show();
}

void EQInterface::showNetDiag()
{
  if (m_netDiag == NULL)
    m_netDiag = new NetDiag(m_packet, NULL, "NetDiag");

  // make sure it's visible
  m_netDiag->show();
}
