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
 */

#include <qfile.h>
#include <qdatastream.h>

#include "zonemgr.h"
#include "packet.h"
#include "main.h"

//----------------------------------------------------------------------
// constants
static const char magicStr[5] = "zon2"; // magic is the size of uint32_t + a null
static const uint32_t* magic = (uint32_t*)magicStr;


// Sequence of signals on initial entry into eq from character select screen
// EQPacket                              ZoneMgr                       isZoning
// ----------                            -------                       --------
// zoneEntry(ClientZoneEntryStruct)      zoneBegin()                   true
// zoneEntry(ServerZoneEntryStruct)      zoneBegin(shortName)          false
// zoneNew(newZoneStruct)                zoneEnd(shortName, longName)  false
//
// Sequence of signals on when zoning from zone A to zone B
// EQPacket                              ZoneMgr                       isZoning
// ----------                            -------                       --------
// zoneChange(zoneChangeStruct, client)                                true
// zoneChange(zoneChangeStruct, server)  zoneChanged(shortName)        true
// zoneEntry(ClientZoneEntryStruct)      zoneBegin()                   true
// zoneEntry(ServerZoneEntryStruct)      zoneBegin(shortName)          false
// zoneNew(newZoneStruct)                zoneEnd(shortName, longName)  false
//
ZoneMgr::ZoneMgr(EQPacket* packet, QObject* parent, const char* name)
  : QObject(parent, name)
{
  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zoning = false;

  connect(packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneEntry(const ClientZoneEntryStruct*)));
  connect(packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
  connect(packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)));
  connect(packet, SIGNAL(zoneNew(const newZoneStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneNew(const newZoneStruct*, uint32_t, uint8_t)));

  if (showeq_params->restoreZoneState)
    restoreZoneState();
}

QString ZoneMgr::zoneNameFromID(uint32_t zoneId)
{
   static const char* zoneNames[] =
   {
#include "zones.h"
   };

   const char* zoneName = NULL;
   if (zoneId < (sizeof(zoneNames) / sizeof (char*)))
       zoneName = zoneNames[zoneId];

   if (zoneName != NULL)
      return zoneName;

   QString tmpStr;
   tmpStr.sprintf("unk_zone_%d", zoneId);
   return tmpStr;
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
      fprintf(stderr, 
	      "Failure loading %s: Bad magic string!\n",
	      (const char*)fileName);
      return;
    }

    d >> m_longZoneName;
    d >> m_shortZoneName;

    fprintf(stderr, "Restored Zone: %s (%s)!\n",
	    (const char*)m_shortZoneName,
	    (const char*)m_longZoneName);
  }
  else
  {
    fprintf(stderr,
	    "Failure loading %s: Unable to open!\n", 
	    (const char*)fileName);
  }
}

void ZoneMgr::zoneEntry(const ClientZoneEntryStruct* zsentry)
{
  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";

  m_zoning = true;

  emit zoneBegin();

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneEntry(const ServerZoneEntryStruct* zsentry)
{
  m_shortZoneName = zoneNameFromID(zsentry->zoneId);

  m_zoning = false;

  emit zoneBegin(m_shortZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneChange(const zoneChangeStruct* zoneChange, uint32_t, uint8_t dir)
{
  m_shortZoneName = zoneNameFromID(zoneChange->zoneId);
  m_zoning = true;

  if (dir == DIR_SERVER)
    emit zoneChanged(m_shortZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneNew(const newZoneStruct* zoneNew, uint32_t, uint8_t dir)
{
  m_shortZoneName = zoneNew->shortName;
  m_longZoneName = zoneNew->longName;
  m_zoning = false;

  emit zoneEnd(m_shortZoneName, m_longZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

