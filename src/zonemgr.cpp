/*
 * zonemgr.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001,2007 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
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
#include "netstream.h"

#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>

//----------------------------------------------------------------------
// constants
static const char magicStr[5] = "zon2"; // magic is the size of uint32_t + a null
static const uint32_t* magic = (uint32_t*)magicStr;
const float defaultZoneExperienceMultiplier = 0.75;

// Sequence of signals on initial entry into eq from character select screen
// EQPacket                              ZoneMgr                       isZoning
// ----------                            -------                       --------
// zoneEntry(ClientZoneEntryStruct)      zoneBegin()                   true
// PlayerProfile(charProfileStruct)      zoneBegin(shortName)          false
// zoneNew(newZoneStruct)                zoneEnd(shortName, longName)  false
//
// Sequence of signals on when zoning from zone A to zone B
// EQPacket                              ZoneMgr                       isZoning
// ----------                            -------                       --------
// zoneChange(zoneChangeStruct, client)                                true
// zoneChange(zoneChangeStruct, server)  zoneChanged(shortName)        true
// zoneEntry(ClientZoneEntryStruct)      zoneBegin()                   false
// PlayerProfile(charProfileStruct)      zoneBegin(shortName)          false
// zoneNew(newZoneStruct)                zoneEnd(shortName, longName)  false
//
ZoneMgr::ZoneMgr(QObject* parent, const char* name)
  : QObject(parent, name),
    m_zoning(false),
    m_zone_exp_multiplier(defaultZoneExperienceMultiplier),
    m_zonePointCount(0),
    m_zonePoints(0)
{
  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zoning = false;
  m_dzID = 0;

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

void ZoneMgr::zoneEntryClient(const uint8_t* data, size_t len, uint8_t dir)
{
  const ClientZoneEntryStruct* zsentry = (const ClientZoneEntryStruct*)data;

  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zone_exp_multiplier = defaultZoneExperienceMultiplier;
  m_zoning = false;

  emit zoneBegin();
  emit zoneBegin(zsentry, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

int32_t ZoneMgr::fillProfileStruct(charProfileStruct *player, const uint8_t *data, size_t len, bool checkLen)
{
  /*
  This reads data from the variable-length charPlayerProfile struct
  */
  NetStream netStream(data, len);
  int32_t retVal;
  QString name;

  player->checksum = netStream.readUInt32NC();

  // Unknown  
  netStream.skipBytes(12);
  
  player->profile.gender = netStream.readUInt8();
  player->profile.race = netStream.readUInt32();
  player->profile.class_ = netStream.readUInt32();
  player->profile.level = netStream.readUInt8();
  player->profile.level1 = netStream.readUInt8();

  // Really, everything after the level is not critical for operation.  If 
  // needed, skip the rest to get up and running quickly after patch day.

  // Bind points
  int bindCount = netStream.readUInt32NC();
  for (int i = 0; i < bindCount; i++) {
    memcpy(&player->profile.binds[i], netStream.pos(), sizeof(player->profile.binds[i]));
    netStream.skipBytes(sizeof(player->profile.binds[i]));
  }

  player->profile.deity = netStream.readUInt32NC();
  player->profile.intoxication = netStream.readUInt32NC();

  // Spell slot refresh
  int spellRefreshCount = netStream.readUInt32NC();
  for (int i = 0; i < spellRefreshCount; i++) {
    player->profile.spellSlotRefresh[i] = netStream.readUInt32NC();
  }

  // Equipment
  int equipCount = netStream.readUInt32NC();
  for (int i = 0; i < equipCount; i++) {
    memcpy(&player->profile.equipment[i], netStream.pos(), sizeof(player->profile.equipment[i]));
    netStream.skipBytes(sizeof(player->profile.equipment[i]));
  }
   
  // Something (9 ints)
  int sCount = netStream.readUInt32NC();
  for (int i = 0; i < sCount; i++) {	
    netStream.skipBytes(20);
  }

  // Something (9 ints)
  int sCount1 = netStream.readUInt32NC();
  for (int i = 0; i < sCount1; i++) {
    netStream.skipBytes(4);
  }

  // Something (9 ints)
  int sCount2 = netStream.readUInt32NC();
  for (int i = 0; i < sCount2; i++) {
    netStream.skipBytes(4);
  }

  // Looks like face, haircolor, beardcolor, eyes, etc. Skipping over it.
  netStream.skipBytes(52);

  player->profile.points = netStream.readUInt32NC();
  player->profile.MANA = netStream.readUInt32NC();
  player->profile.curHp = netStream.readUInt32NC();
  player->profile.STR = netStream.readUInt32NC();
  player->profile.STA = netStream.readUInt32NC();
  player->profile.CHA = netStream.readUInt32NC();
  player->profile.DEX = netStream.readUInt32NC();
  player->profile.INT = netStream.readUInt32NC();
  player->profile.AGI = netStream.readUInt32NC();
  player->profile.WIS = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(28);

  // AAs
  int aaCount = netStream.readUInt32NC();
  for (int i = 0; i < aaCount; i++) {
    player->profile.aa_array[i].AA = netStream.readUInt32NC();
    player->profile.aa_array[i].value = netStream.readUInt32NC();
    player->profile.aa_array[i].unknown008 = netStream.readUInt32NC();
  }

  // Something (100 ints)
  int sCount3 = netStream.readUInt32NC();
  for (int i = 0; i < sCount3; i++) {
    netStream.skipBytes(4);
  }

  // Something (25 ints)
  int sCount4 = netStream.readUInt32NC();
  for (int i = 0; i < sCount4; i++) {
    netStream.skipBytes(4);
  }

  // Something (300 ints)
  int sCount5 = netStream.readUInt32NC();
  for (int i = 0; i < sCount5; i++) {
    netStream.skipBytes(4);
  }

  // Something (20 ints)
  int sCount6 = netStream.readUInt32NC();
  for (int i = 0; i < sCount6; i++) {
    netStream.skipBytes(4);
  }

  // Unknown
  netStream.skipBytes (4);

  // Something (20 floats)
  int sCount7 = netStream.readUInt32NC();
  for (int i = 0; i < sCount7; i++) {
    netStream.skipBytes(4);
  }

  // Something (100 floats)
  int sCount8 = netStream.readUInt32NC();
  for (int i = 0; i < sCount8; i++) {
    netStream.skipBytes(4);
  }

  // Spellbook
  int spellBookSlots = netStream.readUInt32NC();
  for (int i = 0; i < spellBookSlots; i++) {
    player->profile.sSpellBook[i] = netStream.readInt32();
  }

  // Mem Spell Slots
  int spellMemSlots = netStream.readUInt32NC();
  for (int i = 0; i < spellMemSlots; i++) {
    player->profile.sMemSpells[i] = netStream.readInt32();
  }

  // Something (13 ints)
  int sCount9 = netStream.readUInt32NC();
  for (int i = 0; i < sCount9; i++) {
    netStream.skipBytes(4);
  }

  // Unknown
  netStream.skipBytes(1);

  int buffCount = netStream.readUInt32NC();
  for (int i = 0; i < buffCount; i++) {
    memcpy(&player->profile.buffs[i], netStream.pos(), sizeof(player->profile.buffs[i]));
    netStream.skipBytes(sizeof(player->profile.buffs[i]));
  }

  player->profile.platinum = netStream.readUInt32NC();
  player->profile.gold = netStream.readUInt32NC();
  player->profile.silver = netStream.readUInt32NC();
  player->profile.copper = netStream.readUInt32NC();

  player->profile.platinum_cursor = netStream.readUInt32NC();
  player->profile.gold_cursor = netStream.readUInt32NC();
  player->profile.silver_cursor = netStream.readUInt32NC();
  player->profile.copper_cursor = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(20);

  player->profile.aa_spent = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(4);

  player->profile.aa_assigned = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(20);

  player->profile.aa_unspent = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(2);

/*
  // Bandolier
  netStream.skipBytes(996);

  // Potion Belt
  netStream.skipBytes(153);
*/

  int bandolierCount = netStream.readUInt32NC();
  for (int i = 0; i < bandolierCount; i++) {
    name = netStream.readText();
    if(name.length()) {
      strncpy(player->profile.bandoliers[i].bandolierName, name.latin1(), 32);
    }

    // Mainhand
    name = netStream.readText();
    if(name.length()) {
      strncpy(player->profile.bandoliers[i].mainHand.itemName, name.latin1(), 64);
    }
    player->profile.bandoliers[i].mainHand.itemId = netStream.readUInt32NC();
    player->profile.bandoliers[i].mainHand.icon = netStream.readUInt32NC();

    // Offhand
    name = netStream.readText();
    if(name.length()) {
      strncpy(player->profile.bandoliers[i].offHand.itemName, name.latin1(), 64);
    }
    player->profile.bandoliers[i].offHand.itemId = netStream.readUInt32NC();
    player->profile.bandoliers[i].offHand.icon = netStream.readUInt32NC();

    // Range
    name = netStream.readText();
    if(name.length()) {
      strncpy(player->profile.bandoliers[i].range.itemName, name.latin1(), 64);
    }
    player->profile.bandoliers[i].range.itemId = netStream.readUInt32NC();
    player->profile.bandoliers[i].range.icon = netStream.readUInt32NC();

    // Ammo
    name = netStream.readText();
    if(name.length()) {
      strncpy(player->profile.bandoliers[i].ammo.itemName, name.latin1(), 64);
    }
    player->profile.bandoliers[i].ammo.itemId = netStream.readUInt32NC();
    player->profile.bandoliers[i].ammo.icon = netStream.readUInt32NC();
  }

  int potionCount = netStream.readUInt32NC();
  for (int i = 0; i < potionCount; i++) {
    name = netStream.readText();
    if(name.length()) {
      strncpy(player->profile.potionBelt[i].itemName, name.latin1(), 64);
    }
    player->profile.potionBelt[i].itemId = netStream.readUInt32NC();
    player->profile.potionBelt[i].icon = netStream.readUInt32NC();
  }

  // Unknown
  netStream.skipBytes(84);

  player->profile.endurance = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(12);

  memcpy(player->name, netStream.pos(), 64);
  netStream.skipBytes(64);

  // Unknown
  netStream.skipBytes(4);

  memcpy(player->lastName, netStream.pos(), 32);
  netStream.skipBytes(32);

  player->birthdayTime = netStream.readUInt32NC();
  player->accountCreateDate = netStream.readUInt32NC();
  player->lastSaveTime = netStream.readUInt32NC();
  player->timePlayedMin = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(8);

/*
  // Something (32 ints)  Think this is MAX_KNOWN_LANGS - cannot get this to skip the 36 bytes so added skip below
  int sCount10 = netStream.readUInt8();
  for (int i = 0; i < sCount10; i++) {
  }
*/

  // Unknown
  netStream.skipBytes(36);

  player->zoneId = netStream.readUInt16NC();
  player->zoneInstance = netStream.readUInt16NC();

  memcpy(&player->x, netStream.pos(), sizeof(player->x));
  netStream.skipBytes(sizeof(player->x));

  memcpy(&player->y, netStream.pos(), sizeof(player->y));
  netStream.skipBytes(sizeof(player->y));

  memcpy(&player->z, netStream.pos(), sizeof(player->z));
  netStream.skipBytes(sizeof(player->z));

  memcpy(&player->heading, netStream.pos(), sizeof(player->heading));
  netStream.skipBytes(sizeof(player->heading));

/*  
  player->standState = netStream.readUInt8();
*/

  netStream.skipBytes(4);
  
  player->guildID = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(19);

  player->platinum_bank = netStream.readUInt32NC();
  player->gold_bank = netStream.readUInt32NC();
  player->silver_bank = netStream.readUInt32NC();
  player->copper_bank = netStream.readUInt32NC();
  player->platinum_shared = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(12);

  // Something (164 ints)
  int sCount11 = netStream.readUInt32NC();
  for (int i = 0; i < sCount11; i++) {
    netStream.skipBytes(8);
  }

  // Unknown
  netStream.skipBytes(8);

  player->careerTribute = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(4);

  player->currentTribute = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(6);

  // Tributes
  int tributeCount = netStream.readUInt32NC();
  for (int i = 0; i < tributeCount; i++) {
    memcpy(&player->tributes[i], netStream.pos(), sizeof(player->tributes[i]));
    netStream.skipBytes(sizeof(player->tributes[i]));
  }

  // Unknown
  netStream.skipBytes(229);

  player->currentRadCrystals = netStream.readUInt32NC();
  player->careerRadCrystals = netStream.readUInt32NC();
  player->currentEbonCrystals = netStream.readUInt32NC();
  player->careerEbonCrystals = netStream.readUInt32NC();

  // Unknown
  netStream.skipBytes(149);

  player->ldon_guk_points = netStream.readUInt32NC();
  player->ldon_mir_points = netStream.readUInt32NC();
  player->ldon_mmc_points = netStream.readUInt32NC();
  player->ldon_ruj_points = netStream.readUInt32NC();
  player->ldon_tak_points = netStream.readUInt32NC();
  player->ldon_avail_points = netStream.readUInt32NC();


  // Below are the structs still not found in the new playerpacket

/*
  int skillCount = netStream.readUInt32NC();
  for (int i = 0; i < skillCount; i++) {
    player->profile.skills[i] = netStream.readUInt32NC();
  }

  int innateSkillCount = netStream.readUInt32NC();
  for (int i = 0; i < innateSkillCount; i++) {
    player->profile.innateSkills[i] = netStream.readUInt32NC();
  }

  player->profile.toxicity = netStream.readUInt32NC();
  player->profile.thirst = netStream.readUInt32NC();
  player->profile.hunger = netStream.readUInt32NC();

  int disciplineCount = netStream.readUInt32NC();
  for (int i = 0; i < disciplineCount; i++) {
    player->profile.disciplines[i] = netStream.readUInt32NC();
  }

  int recastTypes = netStream.readUInt32NC();
  for (int i = 0; i < recastTypes; i++) {
    player->profile.recastTimers[i] = netStream.readUInt32NC();
  }

  player->pvp = netStream.readUInt8();
  player->anon = netStream.readUInt8();
  player->gm = netStream.readUInt8();
  player->guildstatus = netStream.readInt8();
  player->exp = netStream.readUInt32NC();

  int langCount = netStream.readUInt32NC();
  for (int i = 0; i < langCount; i++) {
    player->languages[i] = netStream.readUInt8();
  }

   // Unknown (41)
  int doubleIntCount = netStream.readUInt32NC();
  for (int i = 0; i < doubleIntCount; i++) {
    int something = netStream.readUInt32NC();
    int somethingElse = netStream.readUInt32NC();
  }

  // Unknown (64)
  int byteCount = netStream.readUInt32NC();
  for (int i = 0; i < byteCount; i++) {
    char something = netStream.readUInt8();
  }
  
  player->expansions = netStream.readUInt32NC();
  player->autosplit = netStream.readUInt8();

*/

  retVal = netStream.pos() - netStream.data();
  if (checkLen && (int32_t)len != retVal)
  {
    seqDebug("SpawnShell::fillProfileStruct - expected length: %d, read: %d for player '%s'", len, retVal, player->name);
  }

  return retVal;
}


void ZoneMgr::zonePlayer(const uint8_t* data, size_t len)
{
  charProfileStruct *player = new charProfileStruct;

  memset(player,0,sizeof(charProfileStruct));

  fillProfileStruct(player,data,len,false); // don't bother checking the length since it's always going to not match up

  m_shortZoneName = zoneNameFromID(player->zoneId);
  m_longZoneName = zoneLongNameFromID(player->zoneId);
  m_zone_exp_multiplier = defaultZoneExperienceMultiplier;
  m_zoning = false;

  emit zoneBegin(m_shortZoneName);
  emit playerProfile(player);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneChange(const uint8_t* data, size_t len, uint8_t dir)
{
  const zoneChangeStruct* zoneChange = (const zoneChangeStruct*)data;
  m_shortZoneName = zoneNameFromID(zoneChange->zoneId);
  m_longZoneName = zoneLongNameFromID(zoneChange->zoneId);
  m_zone_exp_multiplier = defaultZoneExperienceMultiplier;
  m_zoning = true;

  if (dir == DIR_Server)
    emit zoneChanged(m_shortZoneName);
    emit zoneChanged(zoneChange, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneNew(const uint8_t* data, size_t len, uint8_t dir)
{
  const newZoneStruct* zoneNew = (const newZoneStruct*)data;
  m_safePoint.setPoint(lrintf(zoneNew->safe_x), lrintf(zoneNew->safe_y),
		       lrintf(zoneNew->safe_z));
  m_zone_exp_multiplier = zoneNew->zone_exp_multiplier;

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

  m_longZoneName = zoneNew->longName;
  m_zoning = false;

#if 1 // ZBTEMP
  seqDebug("Welcome to lovely downtown '%s' with an experience multiplier of %f",
	 zoneNew->longName, zoneNew->zone_exp_multiplier);
  seqDebug("Safe Point (%f, %f, %f)", 
	 zoneNew->safe_x, zoneNew->safe_y, zoneNew->safe_z);
#endif // ZBTEMP
  
//   seqDebug("zoneNew: m_short(%s) m_long(%s)",
//      (const char*)m_shortZoneName,
//      (const char*)m_longZoneName);
  
  emit zoneEnd(m_shortZoneName, m_longZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
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

void ZoneMgr::dynamicZonePoints(const uint8_t *data, size_t len, uint8_t)
{
   const dzSwitchInfo *dz = (const dzSwitchInfo*)data;

   if(len == sizeof(dzSwitchInfo))
   {
      m_dzPoint.setPoint(lrintf(dz->x), lrintf(dz->y), lrintf(dz->z));
      m_dzID = dz->zoneID;
      m_dzLongName = zoneLongNameFromID(m_dzID);
      if(dz->type != 1 && dz->type > 2 && dz->type <= 5)
         m_dzType = 0; // green
      else
         m_dzType = 1; // pink
   }
   else if(len == 8)
   {
      // we quit the expedition
      m_dzPoint.setPoint(0, 0, 0);
      m_dzID = 0;
      m_dzLongName = "";
   }
}

void ZoneMgr::dynamicZoneInfo(const uint8_t* data, size_t len, uint8_t)
{
   const dzInfo *dz = (const dzInfo*)data;

   if(!dz->newDZ)
   {
      m_dzPoint.setPoint(0, 0, 0);
      m_dzID = 0;
      m_dzLongName = "";
   }
}

#ifndef QMAKEBUILD
#include "zonemgr.moc"
#endif
