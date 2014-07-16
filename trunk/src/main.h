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

#include "config.h"

//----------------------------------------------------------------------
// bogus structure that should die soon
struct ShowEQParams
{
  bool           retarded_coords; // Verant style YXZ instead of XYZ
  bool           fast_machine;
  bool           createUnknownSpawns;
  bool           keep_selected_visible;
  bool           pvp;
  bool		 deitypvp;
  bool           walkpathrecord;
  uint32_t       walkpathlength;
  bool           systime_spawntime;
  bool           showRealName;
  
  bool           saveZoneState;
  bool           savePlayerState;
  bool           saveSpawns;
  uint32_t       saveSpawnsFrequency;
  bool           restorePlayerState;
  bool           restoreZoneState;
  bool           restoreSpawns;
  QString        saveRestoreBaseFilename;
  bool           useUpdateRadius;
  uint8_t        filterZoneDataLog;
};
 
extern struct ShowEQParams *showeq_params;

#endif
