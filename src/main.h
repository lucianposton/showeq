/*
 * main.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef _SHOWEQ_MAIN_H
#define _SHOWEQ_MAIN_H

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif
#include <stdlib.h>
#include <deque>

#include "xmlpreferences.h"
extern class XMLPreferences *pSEQPrefs;

class EQItemDB;

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

struct ShowEQParams
{
  QString        device;
  QString        ip;
  QString        mac_address;
  bool           session_tracking;
  bool           realtime;
  uint16_t       arqSeqGiveUp;
  QString        filterfile;
  bool           spawnfilter_case;
  bool           spawnfilter_audio;
  bool           retarded_coords; // Verant style YXZ instead of XYZ
  bool           fast_machine;
  bool           spawn_alert_plus_plus;
  bool           createUnknownSpawns;
  bool           con_select;
  bool           tar_select;
  bool           keep_selected_visible;
  bool           promisc;
  bool           net_stats;
  bool           recordpackets;
  bool           playbackpackets;
  int8_t         playbackspeed; // Should be signed since -1 is pause
  bool           pvp;
  bool		 deitypvp;
  bool           broken_decode;
  bool           walkpathrecord;
  uint32_t       walkpathlength;
  bool           spawnfilter_loglocates;
  bool           spawnfilter_logcautions;
  bool           spawnfilter_logdangers;
  bool           spawnfilter_loghunts;
  bool           systime_spawntime;
  bool           showRealName;
  
  bool           AutoDetectCharSettings;
  QString        defaultName;
  QString        defaultLastName;
  unsigned char  defaultLevel;
  unsigned char  defaultRace;
  unsigned char  defaultClass;
  unsigned char  defaultDeity;

  bool           showSpellMsgs;
  bool           no_bank;
  bool           showEQTime;

  int            ItemDBTypes;
  QString        ItemLoreDBFilename;
  QString        ItemNameDBFilename;
  QString        ItemDataDBFilename;
  QString        ItemRawDataDBFileName;
  bool           ItemDBEnabled;

  bool           saveZoneState;
  bool           savePlayerState;
  bool           saveSpawns;
  uint32_t       saveSpawnsFrequency;
  bool           restorePlayerState;
  bool           restoreZoneState;
  bool           restoreSpawns;
  QString        saveRestoreBaseFilename;
};
 
extern struct ShowEQParams *showeq_params;

#endif
