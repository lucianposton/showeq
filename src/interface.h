/*
 * interface.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQINT_H
#define EQINT_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qmainwindow.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qsplitter.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qptrlist.h>
#include <qmessagebox.h>
#include <qtabdialog.h>
#include <qspinbox.h>
#include <qintdict.h>

#include "everquest.h"
#include "spawnlist.h"
#include "spawnshell.h"

//--------------------------------------------------
// forward declarations
class Player;
class MapMgr;
class SpawnListWindow;
class SpawnListWindow2;
class SpellListWindow;
class SkillListWindow;
class StatListWindow;
class SpawnPointWindow;
class EQPacket;
class ZoneMgr;
class FilterMgr;
class CategoryMgr;
class SpawnShell;
class SpellShell;
class GroupMgr;
class SpawnMonitor;
class SpawnLog;
class FilteredSpawnLog;
class FilterNotifications;
class Item;
class CompassFrame;
class MapFrame;
class ExperienceWindow;
class CombatWindow;
class NetDiag;
class GuildMgr;
class Spells;
class DateTimeMgr;
class PacketLog;
class PacketStreamLog;
class UnknownPacketLog;
class OPCodeMonitorPacketLog;
class DataLocationMgr;
class EQStr;
class MessageFilters;
class Messages;
class MessageShell;
class MessageWindow;
class Terminal;
class MessageFilterDialog;

//--------------------------------------------------
// typedefs
typedef QValueList<int> MenuIDList;

//--------------------------------------------------
// constants
// maximum number of maps
const int maxNumMaps = 5; 

// This is the base number where the map dock options appear in the
// Docked menu
const int mapDockBase = 7; 

// This is the base number where the map caption options appear in the
// Window caption menu
const int mapCaptionBase = 11; 

// maximum number of message windows
const int maxNumMessageWindows = 10;

//--------------------------------------------------
// EQInterface
class EQInterface:public QMainWindow
{
   Q_OBJECT

 public:
   EQInterface(DataLocationMgr* dlm, 
	       QWidget * parent = 0, const char *name = 0);
   ~EQInterface();

   QFont appFont;

 public slots:
   void stsMessage(const QString &, int timeout = 0);
   void numSpawns(int);
   void setExp(uint32_t totalExp, uint32_t totalTick,
	       uint32_t minExpLevel, uint32_t maxExpLevel, 
	       uint32_t tickExpLevel);
   void newExp(uint32_t newExp, uint32_t totalExp, 
	       uint32_t totalTick,
	       uint32_t minExpLevel, uint32_t maxExpLevel, 
	       uint32_t tickExpLevel);
   void setAltExp(uint32_t totalExp,
		  uint32_t maxExp, uint32_t tickExp, uint32_t aapoints);
   void newAltExp(uint32_t newExp, uint32_t totalExp, uint32_t totalTick, 
		  uint32_t maxExp, uint32_t tickExp, uint32_t aapoints);
   void levelChanged(uint8_t level);
   void newSpeed(int);
   void numPacket(int, int);
   void resetPacket(int, int);
   void attack2Hand1(const uint8_t*);
   void action2Message(const uint8_t *);
   void combatKillSpawn(const uint8_t*);
   void updatedDateTime(const QDateTime&);
   void syncDateTime(const QDateTime&);
   void clientTarget(const uint8_t* cts);

   void zoneBegin(const QString& shortZoneName);
   void zoneEnd(const QString& shortZoneName, const QString& longZoneName);
   void zoneChanged(const QString& shortZoneName);

   void spawnSelected(const Item* item);
   void spawnConsidered(const Item* item);
   void addItem(const Item* item);
   void delItem(const Item* item);
   void killSpawn(const Item* item);
   void changeItem(const Item* item);

   void updateSelectedSpawnStatus(const Item* item);

   void savePrefs(void);
   void saveDockAreaPrefs(QDockArea* a, Dock edge);

   void addCategory(void);
   void reloadCategories(void);
   void rebuildSpawnList();
   void selectNext(void);
   void selectPrev(void);
   void saveSelectedSpawnPath(void);
   void saveSpawnPaths(void);
   void saveSpawnPath(QTextStream& out, const Item* item);
   void toggle_log_AllPackets();
   void toggle_log_WorldData();
   void toggle_log_ZoneData();
   void toggle_log_UnknownData();
   void toggle_log_RawData();
   void toggle_log_ItemData();
   void toggle_log_ItemPacketData();
   void listSpawns(void);
   void listDrops(void);
   void listMapInfo(void);
   void listInterfaceInfo(void);
   void listGroup(void);
   void dumpSpawns(void);
   void dumpDrops(void);
   void dumpMapInfo(void);
   void dumpGuildInfo(void);
   void dumpSpellBook(void);
   void dumpGroup(void);
   void launch_editor_filters(void);
   void launch_editor_zoneFilters(void);
   void toggleAutoDetectPlayerSettings(int id);
   void SetDefaultCharacterClass(int id);
   void SetDefaultCharacterRace(int id);
   void SetDefaultCharacterLevel (int id);
   void toggle_view_StatWin(int id);
   void toggle_view_SkillWin(int id);
   void toggle_view_SpawnListCol( int id );
   void toggle_view_DockedWin( int id );
   
   void selectTheme(int id);
   void toggle_opcode_monitoring (int id);
   void set_opcode_monitored_list (void);
   void toggle_opcode_view(int id);
   void toggle_opcode_log(int id);
   void select_opcode_file(void);
   void toggle_net_session_tracking(void);
   void toggle_net_real_time_thread(int id);
   void set_net_monitor_next_client();
   void set_net_client_IP_address();
   void set_net_client_MAC_address();
   void set_net_device();
   void set_net_arq_giveup(int giveup);
   virtual void setCaption(const QString&);
   void restoreStatusFont();
   void showMessageFilterDialog(void);

 signals:
   void guildList2text(QString);
   void loadFileMap();
   void selectSpawn(const Item* item);
   void saveAllPrefs(void);
   void newZoneName (const QString &); 
   void spellMessage(QString&);
   void restoreFonts();

   // Decoder signals
   void theKey(uint64_t);
   void backfillPlayer(charProfileStruct *);

   void combatSignal(int, int, int, int, int, QString, QString);

 private slots:
   void toggle_opt_Fast();
   void toggle_view_UnknownData();
   void toggle_view_ChannelMsgs(int id);
   void toggle_view_ExpWindow();
   void toggle_view_CombatWindow();
   void toggle_opt_ConSelect();
   void toggle_opt_TarSelect();
   void toggle_opt_KeepSelectedVisible();
   void toggle_opt_LogSpawns();
   void toggle_opt_PvPTeams();
   void toggle_opt_PvPDeity();
   void toggle_opt_CreateUnknownSpawns(int);
   void toggle_opt_WalkPathRecord(int);
   void set_opt_WalkPathLength(int);
   void toggle_opt_RetardedCoords(int);
   void toggle_opt_SystimeSpawntime(int);
   void select_opt_conColorBase(int);
   void toggle_view_SpawnList();
   void toggle_view_SpawnList2();
   void toggle_view_SpawnPointList();
   void toggle_view_SpellList();
   void toggle_view_PlayerStats();
   void toggle_view_Compass();
   void toggle_view_PlayerSkills();
   void toggle_view_Map(int id);
   void toggle_view_NetDiag();
   void resetMaxMana();
   void select_filter_file();
   void save_as_filter_file();
   void toggle_filter_Case(int id);
   void toggle_filter_AlertInfo(int id);
   void toggle_filter_UseSystemBeep(int id);
   void toggle_filter_UseCommands(int id);
   void toggle_filter_Log(int id);
   void set_filter_AudioCommand(int id);
   void toggle_view_menubar();
   void toggle_view_statusbar();
   void set_main_WindowCaption(int id);
   void set_main_WindowFont(int id);
   void set_main_Font(int id);
   void select_main_FormatFile(int id);
   void select_main_SpellsFile(int id);
   void toggle_main_statusbar_Window(int id);
   void set_main_statusbar_Font(int id);
   void toggle_main_SavePosition(int id);
   void toggle_main_UseWindowPos(int id);
   void toggle_opt_save_PlayerState(int id);
   void toggle_opt_save_ZoneState(int id);
   void toggle_opt_save_Spawns(int id);
   void set_opt_save_SpawnFrequency(int frequency);
   void set_opt_save_BaseFilename();
   void opt_clearChannelMsgs(int id);
   void init_view_menu();

   void toggleTypeFilter(int);
   void disableAllTypeFilters();
   void enableAllTypeFilters();
   void toggleShowUserFilter(int);
   void disableAllShowUserFilters();
   void enableAllShowUserFilters();
   void toggleHideUserFilter(int);
   void disableAllHideUserFilters();
   void enableAllHideUserFilters();
   void toggleDisplayType(int);
   void toggleDisplayTime(int);
   void toggleEQDisplayTime(int);
   void toggleUseColor(int);

 protected:
   bool getMonitorOpCodeList(const QString& title, QString& opcodeList);
   int setTheme(int id);
   void loadFormatStrings();
   void showMap(int mapNum);
   void showMessageWindow(int winNum);
   void showSpawnList(void);
   void showSpawnList2(void);
   void showSpawnPointList(void);
   void showStatList(void);
   void showSkillList(void);
   void showSpellList(void);
   void showCompass(void);
   void showNetDiag(void);
   void createFilteredSpawnLog(void);
   void createSpawnLog(void);
   void createGlobalLog(void);
   void createWorldLog(void);
   void createZoneLog(void);
   void createUnknownZoneLog(void);
   void createOPCodeMonitorLog(const QString&);
   void insertWindowMenu(SEQWindow* window);
   void removeWindowMenu(SEQWindow* window);

 public:
   Player* m_player;
   MapMgr* mapMgr(void) { return m_mapMgr; }

 private:
   DataLocationMgr* m_dataLocationMgr;
   MapMgr* m_mapMgr;
   SpawnListWindow* m_spawnList;
   SpawnListWindow2* m_spawnList2;
   SpellListWindow* m_spellList;
   SkillListWindow* m_skillList;
   StatListWindow* m_statList;
   SpawnPointWindow* m_spawnPointList;
   EQPacket* m_packet;
   ZoneMgr* m_zoneMgr;
   FilterMgr* m_filterMgr;
   CategoryMgr* m_categoryMgr;
   SpawnShell* m_spawnShell;
   Spells* m_spells;
   SpellShell* m_spellShell;
   GroupMgr* m_groupMgr;
   SpawnMonitor* m_spawnMonitor;
   EQItemDB* m_itemDB;
   GuildMgr* m_guildmgr;
   DateTimeMgr* m_dateTimeMgr;
   EQStr* m_eqStrings;
   MessageFilters* m_messageFilters;
   Messages* m_messages;
   MessageShell* m_messageShell;
   Terminal* m_terminal;
   FilteredSpawnLog* m_filteredSpawnLog;
   FilterNotifications* m_filterNotifications;
   SpawnLog *m_spawnLogger;

   PacketLog* m_globalLog;
   PacketStreamLog* m_worldLog;
   PacketStreamLog* m_zoneLog;
   UnknownPacketLog* m_unknownZoneLog;
   OPCodeMonitorPacketLog* m_opcodeMonitorLog;

   const Item* m_selectedSpawn;
   
   QPopupMenu* m_netMenu;
   QPopupMenu* m_decoderMenu;
   QPopupMenu* m_statWinMenu;
   QPopupMenu* m_skillWinMenu;
   QPopupMenu* m_spawnListMenu;
   QPopupMenu* m_dockedWinMenu;
   QPopupMenu* m_windowCaptionMenu;
   QPopupMenu* m_charMenu;
   QPopupMenu* m_charLevelMenu;
   QSpinBox* m_levelSpinBox;
   QPopupMenu* m_charClassMenu;
   QPopupMenu* m_charRaceMenu;
   QPopupMenu* m_terminalMenu;
   QPopupMenu* m_terminalTypeFilterMenu;
   QPopupMenu* m_terminalShowUserFilterMenu;
   QPopupMenu* m_terminalHideUserFilterMenu;
   QPopupMenu* m_windowMenu;

   CompassFrame* m_compass;
   MessageWindow* m_messageWindow[maxNumMessageWindows];
   MapFrame*  m_map[maxNumMaps];
   ExperienceWindow* m_expWindow;
   CombatWindow* m_combatWindow;
   NetDiag* m_netDiag;
   MessageFilterDialog* m_messageFilterDialog;

   QLabel* m_stsbarSpawns;
   QLabel* m_stsbarStatus;
   QLabel* m_stsbarZone;
   QLabel* m_stsbarID;
   QLabel* m_stsbarExp;
   QLabel* m_stsbarExpAA;
   QLabel* m_stsbarPkt;
   QLabel* m_stsbarEQTime;
   QLabel* m_stsbarSpeed;
   // ZEM code
   QLabel* m_stsbarZEM;

   QIntDict<QString> m_formattedMessageStrings;

   int char_ClassID[PLAYER_CLASSES];
   int char_RaceID[PLAYER_RACES];
   int  m_id_log_AllPackets;
   int  m_id_log_WorldData;
   int  m_id_log_ZoneData;
   int  m_id_log_UnknownData;
   int  m_id_log_RawData;
   int  m_id_log_Items;
   int  m_id_log_ItemPackets;
   int  m_id_opt_OptionsDlg;
   int  m_id_opt_Fast;
   int  m_id_opt_ResetMana;
   int  m_id_view_UnknownData;
   int  m_id_view_ExpWindow;
   int  m_id_view_CombatWindow;
   int  m_id_view_SpawnList;
   int  m_id_view_SpawnList2;
   int  m_id_view_SpawnPointList;
   int  m_id_view_PlayerStats;
   int  m_id_view_PlayerSkills;
   int  m_id_view_Compass;
   int  m_id_view_Map[maxNumMaps];
   int  m_id_view_MessageWindow[maxNumMessageWindows];
   int  m_id_view_NetDiag;
   int  m_id_view_SpellList;
   int  m_id_view_PlayerStats_Options;
   int  m_id_view_PlayerStats_Stats[LIST_MAXLIST];
   int  m_id_view_PlayerSkills_Options;
   int  m_id_view_PlayerSkills_Languages;
   int  m_id_view_SpawnList_Options;
   int  m_id_view_SpawnList_Cols[tSpawnColMaxCols];
   int  m_id_opt_ConSelect;
   int  m_id_opt_TarSelect;
   int  m_id_opt_KeepSelectedVisible;
   int  m_id_opt_LogSpawns;
   int  m_id_opt_PvPTeams;
   int  m_id_opt_PvPDeity;
   int  m_id_net_sessiontrack;
   int  m_packetStartTime;
   int  m_initialcount;

   MenuIDList IDList_StyleMenu;

   QStringList m_StringList;
   QDialog *dialogbox;

   bool m_isSkillListDocked;
   bool m_isStatListDocked;
   bool m_isMapDocked[maxNumMaps];
   bool m_isMessageWindowDocked[maxNumMessageWindows];
   bool m_isSpawnListDocked;
   bool m_isSpawnList2Docked;
   bool m_isSpawnPointListDocked;
   bool m_isSpellListDocked;
   bool m_isCompassDocked;

   bool m_selectOnConsider;
   bool m_selectOnTarget;
};

#endif // EQINT_H

