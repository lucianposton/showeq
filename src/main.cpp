/*
 * main.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

/* main.cpp is the entrypoint to ShowEQ, it parses the commandline
 * options and initializes the application
 */

#ifdef __linux__
#include <linux/version.h>
#endif

#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qapplication.h>
#include <sys/stat.h>
#include <qwindowsstyle.h>
#ifdef __GNU_LIBRARY__
#include <getopt.h>            // for long GNU-style options
#else
#define __GNU_LIBRARY__
#include <getopt.h>
#undef __GNU_LIBRARY__
#endif

#include <qaccel.h>

#include "interface.h"
#include "main.h"
#include "packetcommon.h"
#include "xmlpreferences.h"      // prefrence file class
#include "conf.h"              // autoconf/configure definitions
#include "itemdb.h"

static const char *id="$Id$";

/* **********************************
   defines used for option processing
   ********************************** */
#define OPTION_LIST "i:rf:g::j:::s:aeo:CncFAKSVvPNbtL:xWX:Y:Z:"

/* For long options without any short (single letter) equivalent, we'll
   assign single char nonprinting character equivalents, as is common
   for many GNU utilities.  */

#define   VERSION_OPTION                10
#define   IPADDR_OPTION                 11
#define   MACADDR_OPTION                12
#define   GLOBAL_LOG_FILENAME_OPTION    13
#define   ZONE_LOG_FILENAME_OPTION      14
#define   UNKNOWN_LOG_FILENAME_OPTION   15
#define   GLOBAL_LOG_OPTION             16
#define   ZONE_LOG_OPTION               17
#define   UNKNOWN_ZONE_LOG_OPTION       18
#define   PLAYBACK_SPEED_OPTION         19
#define   SHOW_MAP_POINTS_OPTION        20
#define   RAW_LOG_OPTION                21
#define   SYSTIME_SPAWNTIME_OPTION      22
#define   SPAWNLOG_FILENAME_OPTION      23
#define   DISABLE_SPAWNLOG_OPTION       24
#define   NO_BANK_INFO                  25
#define   ITEMDB_LORE_FILENAME_OPTION   26
#define   ITEMDB_NAME_FILENAME_OPTION   27
#define   ITEMDB_DATA_FILENAME_OPTION   28
#define   ITEMDB_RAW_FILENAME_OPTION    29
#define   ITEMDB_DATABASES_ENABLED      30
#define   WORLD_LOG_FILENAME_OPTION     31
#define   ITEMDB_DISABLE                3
#define   STATUS_FONT_SIZE              4
#define   RESTORE_PLAYER_STATE          6
#define   RESTORE_ZONE_STATE            7
#define   RESTORE_SPAWNS                8
#define   RESTORE_ALL                   9

/* Note that ASCII 32 is a space, best to stop at 31 and pick up again
   at 128 or higher
*/


/* 
   Add your options here.  Format:

   <long option name>,

   <no_argument | optional_argument | required_argument>,

   NULL,

   <single option char or unprintable number like VERSION_OPTION>
*/

static struct option option_list[] = {
  {"net-interface",                required_argument,  NULL,  'i'},
  {"realtime",                     no_argument,        NULL,  'r'},
  {"filter-file",                  required_argument,  NULL,  'f'},
  {"playback-filename",            optional_argument,  NULL,  'j'},
  {"playback-speed",               required_argument,  NULL,  PLAYBACK_SPEED_OPTION},
  {"record-filename",              optional_argument,  NULL,  'g'},
  {"enlightenment-audio",          no_argument,        NULL,  'a'},
  {"filter-case-sensitive",        no_argument,        NULL,  'C'},
  {"use-retarded-coords",          no_argument,        NULL,  'c'},
  {"fast-machine",                 no_argument,        NULL,  'F'},
  {"spawn-alert",                  no_argument,        NULL,  'A'},
  {"create-unknown-spawns",        no_argument,        NULL,  'K'},
  {"show-unknown-spawns",          no_argument,        NULL,  'K'},
  {"select-on-consider",           no_argument,        NULL,  'S'},
  {"select-on-target",             no_argument,        NULL,  'e'},
  {"no-promiscuous",               no_argument,        NULL,  'P'},
  {"show-packet-numbers",          no_argument,        NULL,  'N'},
  {"broken-decode",                no_argument,        NULL,  'b'},
  {"show-selected",                no_argument,        NULL,  't'},
  {"spawn-path-length",            required_argument,  NULL,  'L'},
  {"log-spawns",                   no_argument,        NULL,  'x'},
  {"dont-autodetectcharsettings",  no_argument,        NULL,  'W'},
  {"default-level",                required_argument,  NULL,  'X'},
  {"default-race",                 required_argument,  NULL,  'Y'},
  {"default-class",                required_argument,  NULL,  'Z'},
  {"help",                         no_argument,        NULL,  'h'},
  {"version",                      no_argument,        NULL,  'v'},
  {"ip-address",                   required_argument,  NULL,  IPADDR_OPTION},
  {"mac-address",                  required_argument,  NULL,  MACADDR_OPTION},
  {"global-log-filename",          required_argument,  NULL,  GLOBAL_LOG_FILENAME_OPTION},
  {"world-log-filename",           required_argument,  NULL,  WORLD_LOG_FILENAME_OPTION},
  {"zone-log-filename",            required_argument,  NULL,  ZONE_LOG_FILENAME_OPTION},
  {"unknown-zone-log-filename",    required_argument,  NULL,  UNKNOWN_LOG_FILENAME_OPTION},
  {"log-all",                      no_argument,        NULL,  GLOBAL_LOG_OPTION},
  {"log-zone",                     no_argument,        NULL,  ZONE_LOG_OPTION},
  {"log-unknown-zone",             no_argument,        NULL,  UNKNOWN_ZONE_LOG_OPTION},
  {"log-raw",                      no_argument,        NULL,  RAW_LOG_OPTION},
  {"systime-spawntime",            no_argument,        NULL,  SYSTIME_SPAWNTIME_OPTION},
  {"spawnlog-filename",            required_argument,  NULL,  SPAWNLOG_FILENAME_OPTION},
  {"no-bank",                      no_argument,        NULL,  NO_BANK_INFO},    
  {"itemdb-lore-filename",         required_argument,  NULL, ITEMDB_LORE_FILENAME_OPTION},
  {"itemdb-name-filename",         required_argument,  NULL, ITEMDB_NAME_FILENAME_OPTION},
  {"itemdb-data-filename",         required_argument,  NULL, ITEMDB_DATA_FILENAME_OPTION},
  {"itemdb-raw-data-filename",     required_argument,  NULL, ITEMDB_RAW_FILENAME_OPTION},
  {"itemdb-databases-enabled",     required_argument,  NULL, ITEMDB_DATABASES_ENABLED},
  {"itemdb-disable",               no_argument,        NULL, ITEMDB_DISABLE},
  {"restore-player-state",         no_argument,        NULL, RESTORE_PLAYER_STATE},
  {"restore-zone",                 no_argument,        NULL, RESTORE_ZONE_STATE},
  {"restore-spawns",               no_argument,        NULL, RESTORE_SPAWNS},
  {"restore-all",                  no_argument,        NULL, RESTORE_ALL},
  {0,                              0,                  0,     0}
};

/* Global parameters, so all parts of ShowEQ has access to it */
struct ShowEQParams *showeq_params;
XMLPreferences      *pSEQPrefs = NULL; 

void displayVersion(void);
void displayOptions(const char* progName);

int main (int argc, char **argv)
{
   int           opt;
   int           temp_int;
   int           option_index = 0;
   
   bool          bOptionHelp = false;
        
   /* Create application instance */
   //   QApplication::setStyle( new QWindowsStyle );
   QApplication qapp (argc, argv);

   /* Initialize the parameters with default values */
   char *configfile = LOGDIR "/showeq.xml";

   // scan command line arguments for a specified config file
   int i = 1;
   while (i < argc)
   {
      if ((argv[i][0] == '-') && (argv[i][1] == 'o'))
         configfile = strdup(argv[i + 1]);

      i ++;
   }

   /* NOTE: See preferencefile.cpp for info on how to use prefrences class */
   printf("Using config file '%s'\n", configfile);
   pSEQPrefs = new XMLPreferences(LOGDIR "/seqdef.xml", configfile);

   showeq_params = new ShowEQParams;

   QString section;

   /* TODO: Add some sanity checks to the MAC address option.  cpphack */
   section = "Network";
   showeq_params->device = pSEQPrefs->getPrefString("Device", section, "eth0");
   showeq_params->ip = strdup(pSEQPrefs->getPrefString("IP", section,
						       AUTOMATIC_CLIENT_IP));
   showeq_params->mac_address = strdup(pSEQPrefs->getPrefString("MAC", section, "0"));
   showeq_params->realtime = pSEQPrefs->getPrefBool("RealTimeThread", section,   false);
   showeq_params->promisc = pSEQPrefs->getPrefBool("NoPromiscuous", section, true);
   showeq_params->arqSeqGiveUp = pSEQPrefs->getPrefInt("ArqSeqGiveUp", section, 96);
   showeq_params->session_tracking = pSEQPrefs->getPrefBool("SessionTracking", section, 0);
   showeq_params->broken_decode = pSEQPrefs->getPrefBool("BrokenDecode", section, 0);

   if (showeq_params->broken_decode)
      printf("Disabling decoder due to showeq.xml preferences\n");

   section = "Interface";
   /* Allow map depth filtering */
   showeq_params->retarded_coords  = pSEQPrefs->getPrefBool("RetardedCoords", section, 0);
   showeq_params->con_select = pSEQPrefs->getPrefBool("SelectOnCon", section, false);
   showeq_params->tar_select = pSEQPrefs->getPrefBool("SelectOnTarget", section, false);
   showeq_params->net_stats = pSEQPrefs->getPrefBool("ShowNetStats", section, false);
   showeq_params->systime_spawntime = pSEQPrefs->getPrefBool("SystimeSpawntime", section, false);
   showeq_params->pvp = pSEQPrefs->getPrefBool("PvPTeamColoring", section, false);
   showeq_params->deitypvp = pSEQPrefs->getPrefBool("DeityPvPTeamColoring", section, false);
   showeq_params->keep_selected_visible = pSEQPrefs->getPrefBool("KeepSelected", section, true);

   showeq_params->no_bank = pSEQPrefs->getPrefBool("NoBank", section, true);

   section = "Interface_StatusBar";
   showeq_params->showEQTime = pSEQPrefs->getPrefBool("ShowEQTime",section,false);
   section = "Misc";
   showeq_params->fast_machine = pSEQPrefs->getPrefBool("FastMachine", section, true);
   showeq_params->createUnknownSpawns = pSEQPrefs->getPrefBool("CreateUnknownSpawns", section, true);

   showeq_params->walkpathrecord = pSEQPrefs->getPrefBool("WalkPathRecording", section, false);
   showeq_params->walkpathlength = pSEQPrefs->getPrefInt("WalkPathLength", section, 25);
   /* Tells SEQ whether or not to display casting messages (Turn this off if you're on a big raid) */
   showeq_params->showSpellMsgs = pSEQPrefs->getPrefBool("ShowSpellMessages", section, true);

   section = "Filters";
   showeq_params->filterfile = pSEQPrefs->getPrefString("FilterFile", section, LOGDIR "/filters.conf");
   showeq_params->spawnfilter_audio = pSEQPrefs->getPrefBool("Audio", section, false);
   showeq_params->spawnfilter_loglocates = pSEQPrefs->getPrefBool("LogLocates", section, 0);
   showeq_params->spawnfilter_logcautions = pSEQPrefs->getPrefBool("LogCautions", section, 0);
   showeq_params->spawnfilter_loghunts = pSEQPrefs->getPrefBool("LogHunts", section, 0);
   showeq_params->spawnfilter_logdangers = pSEQPrefs->getPrefBool("LogDangers", section, 0);
   showeq_params->spawnfilter_case = pSEQPrefs->getPrefBool("IsCaseSensitive", section, 0);
   showeq_params->spawn_alert_plus_plus = pSEQPrefs->getPrefBool("AlertInfo", section, 0);

   /* Default Level / Race / Class preferences */
   section = "Defaults";
   showeq_params->AutoDetectCharSettings = pSEQPrefs->getPrefBool("AutoDetectCharSettings", section, 1);
   showeq_params->defaultName = pSEQPrefs->getPrefString("DefaultName", section, "You");
   showeq_params->defaultLastName = pSEQPrefs->getPrefString("DefaultLastName", section, "");
   showeq_params->defaultLevel = pSEQPrefs->getPrefInt("DefaultLevel", section, 1);
   showeq_params->defaultRace = pSEQPrefs->getPrefInt("DefaultRace", section, 1);
   showeq_params->defaultClass = pSEQPrefs->getPrefInt("DefaultClass", section, 1);
   showeq_params->defaultDeity = pSEQPrefs->getPrefInt("DefaultDeity", section, DEITY_AGNOSTIC);

   /* VPacket (Packet Recording / Playback) */
   section = "VPacket";
   showeq_params->playbackpackets = pSEQPrefs->getPrefBool("Playback", section,  false);
   showeq_params->recordpackets = pSEQPrefs->getPrefBool("Record", section, false);
   showeq_params->playbackspeed = pSEQPrefs->getPrefBool("PlaybackRate", section, false);

   section = "SpawnList";
   showeq_params->showRealName = pSEQPrefs->getPrefBool("ShowRealName", section, false);

   /* Different files for different kinds of raw data */

   // item database parameters
   section = "ItemDB";
   showeq_params->ItemLoreDBFilename = pSEQPrefs->getPrefString("LoreDBFilename", section, LOGDIR "/itemlore");
   showeq_params->ItemNameDBFilename = pSEQPrefs->getPrefString("NameDBFilename", section, LOGDIR "/itemname");
   showeq_params->ItemDataDBFilename = pSEQPrefs->getPrefString("DataDBFilename", section, LOGDIR "/itemdata");
   showeq_params->ItemRawDataDBFileName = pSEQPrefs->getPrefString("RawDataDBFilename", section, LOGDIR "/itemrawdata");
   showeq_params->ItemDBTypes = pSEQPrefs->getPrefInt("DatabasesEnabled", section, (EQItemDB::DATA_DB|EQItemDB::RAW_DATA_DB));
   showeq_params->ItemDBEnabled = pSEQPrefs->getPrefBool("Enabled", section, 1);

   section = "SaveState";
   showeq_params->saveZoneState = 
     pSEQPrefs->getPrefBool("ZoneState", section, 1);
   showeq_params->savePlayerState = 
     pSEQPrefs->getPrefBool("PlayerState", section, 1);
   showeq_params->saveSpawns = pSEQPrefs->getPrefBool("Spawns", section, false);
   showeq_params->saveSpawnsFrequency = 
     pSEQPrefs->getPrefInt("SpawnsFrequency", section, (120 * 1000));
   showeq_params->restorePlayerState = false;
   showeq_params->restoreZoneState = false;
   showeq_params->restoreSpawns = false;
   showeq_params->saveRestoreBaseFilename = pSEQPrefs->getPrefString("BaseFilename", section, LOGDIR "/last");

   /* Parse the commandline for commandline parameters */
   while ((opt = getopt_long( argc,
                              argv,
                              OPTION_LIST,
                              option_list,
                              &option_index
                             ))               != -1
          )
   {
      switch (opt)
      {
         /* Set the request to use a despawn list based off the spawn alert list. */

         /* Set the interface */
         case 'i':
         {            
            showeq_params->device = optarg;
            break;
         }


         /* Set pcap thread to realtime */
         case 'r':
         {  
            showeq_params->realtime = 1;
            break;
         }


         /* Set the spawn filter file */
         case 'f':
         {
            showeq_params->filterfile             = optarg;
            
            break;
         }


         /* Packet playback mode */
         case 'j':
         {            
            if (optarg)
               pSEQPrefs->setPrefString("Filename", "VPacket", optarg, 
					XMLPreferences::Runtime);

            showeq_params->playbackpackets = 1;
            showeq_params->recordpackets   = 0;
            
            break;
         }

         /* Packet record mode */
         case 'g':
         {
            if (optarg)
               pSEQPrefs->setPrefString("Filename", "VPacket", optarg, 
					XMLPreferences::Runtime);

            showeq_params->recordpackets   = 1;
            showeq_params->playbackpackets = 0;
            
            break;
         }


         /* Config file was already taken care of, ignore */
         case 'o':
            break;


         /* Enable use of enlightenment audio */
         case 'a':
         {
            showeq_params->spawnfilter_audio = 1;
            break;
         }


         /* Make filter case sensitive */
         case 'C':
         {
            showeq_params->spawnfilter_case = 1;
            break;
         }

         /* Use retarded coordinate system yxz */
         case 'c':
         {
            showeq_params->retarded_coords = 1;
            break;
         }


         /* Fast machine updates.. framerate vs packet based */
         case 'F':
         {
            showeq_params->fast_machine = 1;
            break;
         }

         /* Cool spawn alert */
         case 'A':
         {
            showeq_params->spawn_alert_plus_plus = 1;
            break;
         }


         /* Show unknown spawns */
         case 'K':
         {
            showeq_params->createUnknownSpawns = 1;
            break;
         }


         /* Select spawn on 'Consider' */
         case 'S':
         {
            showeq_params->con_select = 1;
            break;
         }


         /* Select spawn on 'Target' */
         case 'e':
         {
            showeq_params->tar_select = 1;
            break;
         }


         /* Don't use Promiscuous mode on sniffing */
         case 'P':
         {
            showeq_params->promisc = 0;
            break;
         }


         /* Show net info */
         case 'N':
         {
            showeq_params->net_stats = 1;
            break;
         }


         /* 'b'roken decode -- don't deal with spawn packets */
         case 'b':
         {
            showeq_params->broken_decode = 1;
            
            printf("Disabling decoder due to command-line parameter\n");
            break;
         }


         /* 't'rack pathing for mobs */
         case 't':
         {
            showeq_params->walkpathrecord = 1;
            break;
         }


         /* Maximum spawn path tracking length  */
         case 'L':
         {
            showeq_params->walkpathlength = atoi(optarg);
            break;
         }

         
         /* Log spawns! */
         case 'x':
         {
	   pSEQPrefs->setPrefBool("LogSpawns", "Misc", true, 
				  XMLPreferences::Runtime);
	   break;
         }


         /* Display the version info... */
         case 'V':
         case 'v':
         {
	   displayVersion();
	   exit(0);
	   break;
         }

         /* Don't autodetect character settings */
         case 'W':
         {
            showeq_params->AutoDetectCharSettings = 0;
            break;
         }


         /* Set default player level */
         case 'X':
         {
            temp_int = atoi(optarg);
            
            if (temp_int < 1 || temp_int > 60)
            {
               printf ("Invalid default level.  Valid range is 1 - 60.\n");
               exit(0);
            }
            
            showeq_params->defaultLevel = temp_int;           
            break;
         }


         /* Set default player race */
         case 'Y':
         {
            temp_int = atoi(optarg);
            
            if ((temp_int < 1 || temp_int > 12) && temp_int != 128)
            {
               printf ("Invalid default race, please use showeq -h to list valid race options.\n");
               exit(0);
            }
            
            showeq_params->defaultRace = temp_int;
            break;
         }


         /* Set default player class */
         case 'Z':
         {
            temp_int = atoi(optarg);
            
            if (temp_int < 1 || temp_int > 14)
            {
               printf ("Invalid default class, please use showeq -h to list valid class options.\n");
               exit(0);
            }

            showeq_params->defaultClass = temp_int;
            break;
         }

         /* IP address to track */
         case IPADDR_OPTION:
         {
            showeq_params->ip = strdup(optarg);
            break;
         }


         /* MAC address to track for those on DHCP */
         case MACADDR_OPTION:
         {
            showeq_params->mac_address = optarg;
            break;
         }


         /* Filename for logging all packets */
         case GLOBAL_LOG_FILENAME_OPTION:
         {
	   pSEQPrefs->setPrefString("GlobalLogFilename", "PacketLogging", 
				    optarg, XMLPreferences::Runtime);
	   break;
         }


         /* Filename for logging world change packets */
         case WORLD_LOG_FILENAME_OPTION:
         {
	   pSEQPrefs->setPrefString("WorldLogFilename", "PacketLogging", 
				    optarg, XMLPreferences::Runtime);
	   break;
         }

         /* Filename for logging zone change packets */
         case ZONE_LOG_FILENAME_OPTION:
         {
	   pSEQPrefs->setPrefString("ZoneLogFilename", "PacketLogging", 
				    optarg, XMLPreferences::Runtime);
	   break;
         }


         /* Filename for logging unknown zone change packets */
         case UNKNOWN_LOG_FILENAME_OPTION:
         {
	   pSEQPrefs->setPrefString("UnknownZoneLogFilename", 
				    "PacketLogging", optarg, 
				    XMLPreferences::Runtime);
	   break;
         }


         /* Log everything */
         case GLOBAL_LOG_OPTION:
         {
	   pSEQPrefs->setPrefBool("LogAllPackets", "PacketLogging", true,
				  XMLPreferences::Runtime);
	   break;
         }


         /* Log all zone change packets */
         case ZONE_LOG_OPTION:
         {
	   pSEQPrefs->setPrefBool("LogZonePackets", "PacketLogging", true,
				  XMLPreferences::Runtime);
	   break;
         }


         /* Log only unfamiliar zone change packets */
         case UNKNOWN_ZONE_LOG_OPTION:
         {
	   pSEQPrefs->setPrefBool("LogUnknownZonePackets", "PacketLogging", true,
				  XMLPreferences::Runtime);
	   break;
         }


         case PLAYBACK_SPEED_OPTION:
         {
            showeq_params->playbackspeed = atoi(optarg);
            break;
         }


         /* Enable logging of raw packets... */
         case RAW_LOG_OPTION:
         {
	   pSEQPrefs->setPrefBool("LogRawPackets", "PacketLogging", true,
				  XMLPreferences::Runtime);
	   break;
         }

         /* Display spawntime in UNIX time (time_t) instead of hh:mm format */
         case SYSTIME_SPAWNTIME_OPTION:
         {
            showeq_params->systime_spawntime = 1;
            break;
         }

         case SPAWNLOG_FILENAME_OPTION:
         {
	   pSEQPrefs->setPrefString("SpawnLogFilename", "Misc", optarg,
				  XMLPreferences::Runtime);
	   break;
         }

         case NO_BANK_INFO:
         {
         	showeq_params->no_bank = TRUE;
         	break;
         }
         
         case ITEMDB_LORE_FILENAME_OPTION:
         {
            showeq_params->ItemLoreDBFilename = optarg;
            break;
         }
         
         case ITEMDB_NAME_FILENAME_OPTION:
         {
            showeq_params->ItemNameDBFilename = optarg;
            break;
         }
         
         case ITEMDB_DATA_FILENAME_OPTION:
         {
            showeq_params->ItemDataDBFilename = optarg;
            break;
         }
         
         case ITEMDB_RAW_FILENAME_OPTION:
         {
            showeq_params->ItemRawDataDBFileName = optarg;
            break;
         }

         case ITEMDB_DATABASES_ENABLED:
         {
            showeq_params->ItemDBTypes = atoi(optarg);
            break;
         }
	 
         case ITEMDB_DISABLE:
	 {
	   showeq_params->ItemDBEnabled = false;
	   break;
	 }

         case RESTORE_PLAYER_STATE:
	 {
	   showeq_params->restorePlayerState = true;
	   break;
	 }
         case RESTORE_ZONE_STATE:
	 {
	   showeq_params->restoreZoneState = true;
	   break;
	 }
         case RESTORE_SPAWNS:
	 {
	   showeq_params->restoreSpawns = true;
	   break;
	 }
         case RESTORE_ALL:
	 {
	   showeq_params->restorePlayerState = true;
	   showeq_params->restoreZoneState = true;
	   showeq_params->restoreSpawns = true;
	   break;
	 }


         /* Spit out the help */
         case 'h': /* Fall through */
         default:
         {
            bOptionHelp = true;
            break;
         }
      }
   }

   /* Print the version number */
   displayVersion();

   if (bOptionHelp)
   {
     displayOptions(argv[0]);
     exit (0);
   }

   /* Set up individual files for logging selected packet types based on
      a common filename base.   The types to log were found by following
      where pre_worked was a precondition for further analysis.
   */

   if (showeq_params->broken_decode)   
    printf( "***DECRYPTION DISABLED***\n\n"
    
            "Decoder has been manually disabled by either -b, the BrokenDecode option,\n"
            "or a missing libEQ.cpp.\n\n"
            
            "(There should be a more detailed message above)\n"
          );

   int ret;

   // just to add a scope to better control when the main interface gets 
   // destroyed
   if  (1)
   {
     /* The main interface widget */
     EQInterface intf (0, "interface");
     qapp.setMainWidget (&intf);
   
     /* Start the main loop */
     ret = qapp.exec ();
   }

   // delete the preferences data
   delete pSEQPrefs;

   // delete the showeq_params data
   delete showeq_params;

   return ret;
}

void displayVersion(void)
{
   printf ("ShowEQ %s, released under the GPL.\n", VERSION);
   printf ("  SINS 0.5, released under the GPL.\n");
   printf ("All ShowEQ source code is Copyright (C) 2000-2003 by the respective ShowEQ Developers\n");
  
  printf ("ShowEQ comes with NO WARRANTY.\n\n");
  
  printf ("You may redistribute copies of ShowEQ under the terms of\n");
  printf ("The GNU General Public License.\n");
  printf ("See: http://www.gnu.org/copyleft/gpl.html for more details...\n\n");
  
  printf ("For updates and information, please visit http://seq.sourceforge.net/\n");

  /////////////////////////////////
  // Display build information
  printf ("ShowEQ %s, Built from '%s' on %s at %s\n", VERSION,
	  __FILE__, __DATE__, __TIME__);
  printf ("\tCVS: %s\n", id);
#ifdef __GNUC__
#ifdef __GNUC_PATCHLEVEL__
  printf ("\t\tUsing GCC version: %d.%d.%d\n", 
	  __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
  printf ("\t\tUsing GCC version: %d.%d\n", 
	  __GNUC__, __GNUC_MINOR__);
#endif
#elif defined(__VERSION__)
  printf ("\t\tUsing c++ version %s\n", __VERSION__);
#endif

#ifdef __KCC
  printf ("\t\tUsing a KAI compiler\n");
#endif

#ifdef __GLIBC__
  printf ("\t\tUsing glibc version: %d.%d\n", __GLIBC__, __GLIBC_MINOR__);
#endif

#ifdef QT_VERSION_STR
  printf ("\t\tUsing Qt version: %s\n", QT_VERSION_STR);
#endif
#ifdef __linux__
  printf ("\t\tUsing headers from linux version: %s\n",
	  UTS_RELEASE);
#endif
  printf ("\t\tUsing EQItemDB: %s\n", EQItemDB::Version());
  printf ("\tUsing GDBM: %s\n", GDBMConvenience::Version());
#ifdef USE_DB3
  printf ("\tUsing DB3: %s\n", DB3Convenience::Version());
#endif
  /////////////////////////////////
  // Display current system environment information
  struct utsname utsbuff;
  if (uname(&utsbuff) == 0)
    printf ("\tRunning on %s release %s for processor %s\n",
	    utsbuff.sysname, utsbuff.release, 
	    utsbuff.machine);

  printf ("\n");
}

void displayOptions(const char* progName)
{
  /* The default help text */
  printf ("Usage:\n  %s [<options>] [<client IP address>]\n\n", progName);
  
  printf ("  -h, --help                            Shows this help\n");
  printf ("  -o CONFIGFILE                         Alternate showeq.xml pathname\n");
  printf ("  -V, --version                         Prints ShowEQ version number\n");
  printf ("  -i, --net-interface=DEVICE            Specify which network device to bind to\n");
  printf ("  -r, --realtime                        Set the network thread realtime\n");
  printf ("  -f, --filter-file=FILENAME            Sets spawn filter file\n");
  printf ("  -s, --spawn-file=FILENAME             Sets spawn alert file\n");
  printf ("  -C, --filter-case-sensitive           Spawn alert and filter is case sensitive\n");
  printf ("  -a, --enlightenment-audio             Use ESD to play alert during spawn alert\n");
  printf ("  -c, --use-retarded-coords             Use \"retarded\" YXZ coordinates\n");
  printf ("  -F, --fast-machine                    Fast machine - perform more accurate vs. \n");
  printf ("                                        less accurate calculations.\n");
  printf ("  -A, --spawn-alert                     Use name/race/class/light/equipment for \n");
  printf ("                                        spawn matching\n");
  printf ("  -K, --create-unknown-spawns           create unknown spawns\n");
  printf ("  -S, --select-on-consider              Select the spawn considered\n");
  printf ("  -e, --select-on-target                Select the spawn targetted\n");
  printf ("  -P, --no-promiscuous                  Don't sniff the network in promiscuous\n");
  printf ("                                        mode (Don't process packets bound for\n");
  printf ("                                        other machines).\n");
  printf ("  -N, --show-packet-numbers             Show network info dialog\n");
  printf ("  -j, --playback-file=FILENAME          Playback packets in FILENAME, previously\n");
  printf ("                                        recorded with -g option\n");
  printf ("      --playback-speed=SPEED            -1 = Paused, 0 = Max, 1 = Slow, 9 = Fast\n");
  printf ("  -g, --record-file=FILENAME            Record packets to FILENAME to playback\n");
  printf ("                                        with the -j option\n");
  printf ("  -b, --broken-decode                   Broken decode -- Don't attempt to decode\n");
  printf ("                                        the spawn packets (i.e. Your CPU is VERY\n");
  printf ("                                        slow)\n");
  printf ("  -t, --show-selected                   Track spawn movements (no path trace)\n");
  printf ("  -L, --spawn-path-length=###           Track spawn maximum track length (min:3)\n");
  printf ("  -x, --log-spawns                      Log spawns into spawns.db\n");
  printf ("      --systime-spawntime               Show spawn time using UNIX systtem time\n");
  printf ("      --ip-address=IP                   Client IP address\n");
  printf ("      --mac-address=MAC                 Client MAC address as 00:00:00:00:00:00\n");
  printf ("      --log-all                         Log all packets to global logfile\n");
  printf ("      --global-log-filename=FILE        Use FILE for above packet logging\n");
  printf ("      --log-zone                        Like --log-all, but only zone data\n");
  printf ("      --zone-log-filename=FILE          Use FILE for above packet logging\n");
  printf ("      --log-unknown-zone                Log only unrecognized zone data\n");
  printf ("      --unknown-zone-log-filename=FILE  Use FILE for above packet logging\n");
  printf ("      --log-raw                   Log some unprocessed raw data\n");
  printf ("      --spawnlog-filename=FILE          Use FILE instead of spawnlog.txt\n");
  printf ("      --itemdb-lore-filename=FILE       Use FILE instead of itemlore\n");
  printf ("      --itemdb-name-filename=FILE       Use FILE instead of itemname\n");
  printf ("      --itemdb-data-filename=FILE       Use FILE instead of itemdata\n");
  printf ("      --itemdb-raw-data-filename=FILE   Use FILE instead of itemrawdata\n");
  printf ("      --itemdb-databases-enabled=DBS    Use DBS to enable different item\n");
  printf ("                                        databases.\n");
  printf ("      --itemdb-disable                  Disable use of the item DB.\n");
  printf ("  -W, --dont-autodetectcharsettings     Don't auto-detect your character's\n");
  printf ("                                        Level/Race/Class.\n");
  printf ("  -X, --default-level=##                Default player level. (1-60)\n");
  printf ("  -Y, --default-race=##                 Default player race:");
  printf ("\n\t                                    HUM 1,  BAR 2,  ERU 3");
  printf ("\n\t                                    ELF 4,  HIE 5,  DEF 6");
  printf ("\n\t                                    HEF 7,  DWF 8,  TRL 9");
  printf ("\n\t                                    OGR 10, HFL 11, GNM 12");
  printf ("\n\t                                    GNM 12, IKS 128\n");
  printf ("  -Z, --default-class=##                Default player class:");
  printf ("\n\t                                    WAR 1,  CLR 2,  PAL 3");
  printf ("\n\t                                    RNG 4,  SHD 5,  DRU 6");
  printf ("\n\t                                    MNK 7,  BRD 8,  ROG 9");
  printf ("\n\t                                    SHM 10, NEC 11, WIZ 12");
  printf ("\n\t                                    MAG 13, ENC 14\n");
  
  printf ("                                                                   \n");
  printf ("                                                                   \n");
  printf (" The following four options should be used with extreme care!         \n");
  printf ("      --restore-player-state            Restores the player state\n");
  printf ("                                        from a previous session    \n");
  printf ("      --restore-zone                    Restores the zone state\n");
  printf ("                                        from a previous session    \n");
  printf ("      --restore-spawns                  Restores the spawns\n");
  printf ("                                        from a previous session    \n");
  printf ("      --restore-all                     Restores , \n");
  printf ("                                        player state, and spawns   \n");
  printf ("                                        from a previous session    \n");
}

