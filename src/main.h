/*
 * main.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef _SHOWEQ_MAIN_H
#define _SHOWEQ_MAIN_H

#ifdef __cplusplus

#include <stdint.h>
#include <stdlib.h>
#include <deque>

#include "preferences.h"
extern class PreferenceFile *pSEQPrefs;

#include "itemdb.h"
#endif

#include "../conf.h"

//----------------------------------------------------------------------
// Macros
#ifndef LOGDIR
#define LOGDIR "../logs"
#endif
  
#ifndef MAPDIR
#define MAPDIR "../maps"
#endif
  
#ifndef SPAWNFILE
#define SPAWNFILE        LOGDIR "/spawn.db"
#endif

#define AUTOMATIC_CLIENT_IP "127.0.0.0"

struct ShowEQParams
{
  const char    *device;
  const char    *ip;
  const char    *mac_address;
  bool           session_tracking;
  bool           realtime;
  uint16_t       arqSeqGiveUp;
  const char    *spawnfilter_filterfile;
  const char    *spawnfilter_spawnfile;
  const char    *filterfile;
  bool           despawnalert;
  bool           deathalert;
  bool           spawnfilter_regexp;
  bool           spawnfilter_case;
  bool           spawnfilter_audio;
  int            fontsize;
  unsigned char  statusfontsize;
  bool           retarded_coords; // Verant style YXZ instead of XYZ
  bool           fast_machine;
  bool           spawn_alert_plus_plus;
  bool           showUnknownSpawns;
  bool           con_select;
  bool           tar_select;
  bool           keep_selected_visible;
  bool           promisc;
  bool           net_stats;
  bool           recordpackets;
  bool           playbackpackets;
  char           playbackspeed; // Should be signed since -1 is pause
  bool           pvp;
  bool		 deitypvp;
  bool           broken_decode;
  bool           walkpathrecord;
  uint32_t            walkpathlength;
  bool           logSpawns;
  bool           logItems;
  bool           spawnfilter_loglocates;
  bool           spawnfilter_logcautions;
  bool           spawnfilter_logdangers;
  bool           spawnfilter_loghunts;
  bool           systime_spawntime;
  bool           showRealName;
  const char    *SpawnLogFilename;
  bool           spawnlog_enabled;
  
  bool           logAllPackets;
  bool           logZonePackets;
  bool           logUnknownZonePackets;
  bool           logEncrypted;
  const char    *GlobalLogFilename;
  const char    *ZoneLogFilename;
  const char    *UnknownZoneLogFilename;
  const char    *EncryptedLogFilenameBase;
  const char    *PktLoggerMask;
  char          *PktLoggerFilename;
  char          *CharProfileCodeFilename;
  char          *NewSpawnCodeFilename;
  char          *ZoneSpawnsCodeFilename;
 
  bool           AutoDetectCharSettings;
  QString        defaultName;
  QString        defaultLastName;
  unsigned char  defaultLevel;
  unsigned char  defaultRace;
  unsigned char  defaultClass;
  unsigned char  defaultDeity;

  
  int            walkpathmanymode;

  
  bool           showSpellMsgs;
  bool           no_bank;
  bool           showEQTime;

  // OpCode monitoring Variables
  bool           monitorOpCode_Usage;
  const char    *monitorOpCode_List;

  int            ItemDBTypes;
  QString        ItemLoreDBFilename;
  QString        ItemNameDBFilename;
  QString        ItemDataDBFilename;
  QString        ItemRawDataDBFileName;
  bool           ItemDBEnabled;

  bool           saveDecodeKey;
  bool           savePlayerState;
  bool           saveSpawns;
  uint8_t        saveSpawnsFrequency;
  bool           restoreDecodeKey;
  bool           restorePlayerState;
  bool           restoreSpawns;
};
 
extern struct ShowEQParams *showeq_params;
extern struct ShowEQShell *pSEQShell;
extern class EQItemDB* pItemDB;

int keyPref(QString pref, QString section, QString def);
  
#endif
