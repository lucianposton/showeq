/*
 * zonemgr.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 * modified by Fee (fee@users.sourceforge.net)
 *
 */

#include "zonemgr.h"
#include "packet.h"
#include "main.h"
#include "everquest.h"
#include "diagnosticmessages.h"

#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>

#define DEBUGZONEMGR
#undef DEBUGZONEMGR

//----------------------------------------------------------------------
// constants
static const char magicStr[5] = "zon2"; // magic is the size of uint32_t + a null
static const uint32_t* magic = (uint32_t*)magicStr;

// Sequence of signals on initial entry into eq from character select screen
// EQPacket         ZoneMgr                ZoneMgr event                    isZoning
// ----------       --------               --------------                   --------
// OP_ZoneEntry     zoneEntryClient()      zoneBegin()                      true
//                                         zoneBegin(ClientZoneEntryStruct) true
// OP_PlayerProfile zonePlayer()           zoneBegin(shortName)             false
// OP_ZoneEntry     zoneEntryServer()      zoneBegin(ServerZoneEntryStruct) false
// OP_NewZone       zoneNew()              zoneEnd(shortName, longName)     false
//
// Sequence of signals on when zoning from zone A to zone B
// EQPacket         ZoneMgr                ZoneMgr event                    isZoning
// ----------       --------               --------------                   --------
// OP_ZoneChange    zoneChange(dir=client) zoneChanging(zoneChangeStruct)   false
// OP_ZoneChange    zoneChange(dir=server) zoneChanging(zoneChangeStruct)   false
//                                         zoneChanging(shortName)          false
// OP_ZoneEntry     zoneEntryClient()      zoneBegin()                      true
//                                         zoneBegin(ClientZoneEntryStruct) true
// OP_PlayerProfile zonePlayer()           zoneBegin(shortName)             false
// OP_ZoneEntry     zoneEntryServer()      zoneBegin(ServerZoneEntryStruct) false
// OP_NewZone       zoneNew()              zoneEnd(shortName, longName)     false
//
// Logout, /camp
// EQPacket         ZoneMgr                ZoneMgr event                    isZoning
// ----------       --------               --------------                   --------
// OP_LogoutReply   logoutReply()          logOut()                         false
ZoneMgr::ZoneMgr(QObject* parent, const char* name)
  : QObject(parent, name),
    m_zoning(false),
    m_isAwaitingZoneInExp(false),
    m_zone_exp_multiplier(BASE_ZEM_VALUE),
    m_zonePointCount(0),
    m_zonePoints(0)
{
  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zoning = false;

  if (showeq_params->restoreZoneState)
    restoreZoneState();
}

ZoneMgr::~ZoneMgr()
{
  if (m_zonePoints)
    delete [] m_zonePoints;
}

struct ZoneNames
{
  const char* shortName;
  const char* longName;
};

static const ZoneNames zoneNames[] =
{
#include "zones.h"
};

QString ZoneMgr::zoneNameFromID(uint16_t zoneId)
{
   const char* zoneName = NULL;
   if (zoneId < (sizeof(zoneNames) / sizeof (ZoneNames)))
       zoneName = zoneNames[zoneId].shortName;

   if (zoneName != NULL)
      return zoneName;

   QString tmpStr;
   tmpStr.sprintf("unk_zone_%d", zoneId);
   return tmpStr;
}

QString ZoneMgr::zoneLongNameFromID(uint16_t zoneId)
{

   const char* zoneName = NULL;
   if (zoneId < (sizeof(zoneNames) / sizeof (ZoneNames)))
       zoneName = zoneNames[zoneId].longName;

   if (zoneName != NULL)
      return zoneName;

   QString tmpStr;
   tmpStr.sprintf("unk_zone_%d", zoneId);
   return tmpStr;
}

const zonePointStruct* ZoneMgr::zonePoint(uint32_t zoneTrigger)
{
  if (!m_zonePoints)
    return 0;

  for (size_t i = 0; i < m_zonePointCount; i++)
    if (m_zonePoints[i].zoneTrigger == zoneTrigger)
      return &m_zonePoints[i];

  return 0;
}

void ZoneMgr::saveZoneState(void)
{
  QFile keyFile(showeq_params->saveRestoreBaseFilename + "Zone.dat");
  if (keyFile.open(IO_WriteOnly))
  {
    QDataStream d(&keyFile);
    // write the magic string
    d << *magic;

    d << m_longZoneName;
    d << m_shortZoneName;
  }
}

void ZoneMgr::restoreZoneState(void)
{
  QString fileName = showeq_params->saveRestoreBaseFilename + "Zone.dat";
  QFile keyFile(fileName);
  if (keyFile.open(IO_ReadOnly))
  {
    QDataStream d(&keyFile);

    // check the magic string
    uint32_t magicTest;
    d >> magicTest;

    if (magicTest != *magic)
    {
      seqWarn("Failure loading %s: Bad magic string!",
	      (const char*)fileName);
      return;
    }

    d >> m_longZoneName;
    d >> m_shortZoneName;

    seqInfo("Restored Zone: %s (%s)!",
	    (const char*)m_shortZoneName,
	    (const char*)m_longZoneName);
  }
  else
  {
    seqWarn("Failure loading %s: Unable to open!", 
	    (const char*)fileName);
  }
}

void ZoneMgr::initialZoneSpawn(bool isInitialZoneSpawn)
{
    if (isInitialZoneSpawn)
    {
        // Received initial zone spawn list, so no longer zoning.
        m_zoning = false;
    }
}

void ZoneMgr::zoneEntryClient(const uint8_t* data, size_t len, uint8_t dir)
{
  const ClientZoneEntryStruct* zsentry = (const ClientZoneEntryStruct*)data;

  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zone_exp_multiplier = BASE_ZEM_VALUE;
  m_zoning = true;

  emit zoneBegin();
  emit zoneBegin(zsentry, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zonePlayer(const uint8_t* data)
{
  const charProfileStruct* player = (const charProfileStruct*)data;
  m_shortZoneName = zoneNameFromID(player->zoneId);
  m_longZoneName = zoneLongNameFromID(player->zoneId);
  m_zone_exp_multiplier = BASE_ZEM_VALUE;
  m_zoning = false;
  emit zoneBegin(m_shortZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneEntryServer(const uint8_t* data, size_t len, uint8_t dir)
{
  const ServerZoneEntryStruct* zsentry = (const ServerZoneEntryStruct*)data;
  m_zone_exp_multiplier = BASE_ZEM_VALUE;
  m_zoning = false;
  emit zoneBegin(zsentry, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneChange(const uint8_t* data, size_t len, uint8_t dir)
{
  const zoneChangeStruct* zoneChange = (const zoneChangeStruct*)data;
  emit zoneChanging(zoneChange, len, dir);
  if (dir == DIR_Client)
      return;

  if (zoneChange->success != 1)
      return;

  m_shortZoneName = zoneNameFromID(zoneChange->zoneId);
  m_longZoneName = zoneLongNameFromID(zoneChange->zoneId);

  emit zoneChanging(m_shortZoneName);
  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneNew(const uint8_t* data, size_t len, uint8_t dir)
{
  const newZoneStruct* zoneNew = (const newZoneStruct*)data;
  m_safePoint.setPoint(lrintf(zoneNew->safe_x), lrintf(zoneNew->safe_y),
		       lrintf(zoneNew->safe_z));
  m_zone_exp_multiplier = zoneNew->zone_exp_multiplier;

#if 0 // ZBTEMP
  // ZBNOTE: Apparently these come in with the localized names, which means we 
  //         may not wish to use them for zone short names.  
  //         An example of this is: shortZoneName 'ecommons' in German comes 
  //         in as 'OGemeinl'.  OK, now that we have figured out the zone id
  //         issue, we'll only use this short zone name if there isn't one or
  //         it is an unknown zone.
  if (m_shortZoneName.isEmpty() || m_shortZoneName.startsWith("unk"))
  {
    m_shortZoneName = zoneNew->shortName;

    // LDoN likes to append a _262 to the zonename. Get rid of it.
    QRegExp rx("_\\d+$");
    m_shortZoneName.replace( rx, "");
  }
#else
    m_shortZoneName = zoneNew->shortName;
    // LDoN likes to append a _262 to the zonename. Get rid of it.
    QRegExp rx("_\\d+$");
    m_shortZoneName.replace( rx, "");
#endif // ZBTEMP

  m_longZoneName = zoneNew->longName;
  m_zoning = false;

  
  emit zoneEnd(m_shortZoneName, m_longZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::logoutReply()
{
    emit logOut();
}

void ZoneMgr::zonePoints(const uint8_t* data, size_t len, uint8_t)
{
  const zonePointsStruct* zp = (const zonePointsStruct*)data;
  // note the zone point count
  m_zonePointCount = zp->count;

  // delete the previous zone point set
  if (m_zonePoints)
    delete [] m_zonePoints;
  
  // allocate storage for zone points
  m_zonePoints = new zonePointStruct[m_zonePointCount];

  // copy the zone point information
  memcpy((void*)m_zonePoints, zp->zonePoints, 
	 sizeof(zonePointStruct) * m_zonePointCount);
}

void ZoneMgr::zoneInExp(const uint8_t*, size_t, uint8_t dir)
{
#ifdef DEBUGZONEMGR
    seqDebug("ZoneMgr::zoneInExp(dir=%s)", dir == DIR_Server ? "DIR_Server" : "DIR_Client");
#endif
    m_isAwaitingZoneInExp = dir == DIR_Client;
}

#include "zonemgr.moc"

