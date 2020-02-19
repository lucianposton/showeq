/*
 * spawn.cpp
 *
 * ShowEQ Distributed under GPL
 * http://sourceforge.net/projects/seq/
 */

/*
 * Adapted from spawnlist.h - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date   - 7/31/2001
 */

// Major rework of SpawnShell{} classes - Zaphod (dohpaz@users.sourceforge.net)
//   Based on stuff from CJD's SpawnShell{} and stuff from SpawnList and
//   from Map.  Some optimization ideas adapted from SINS.

#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include <limits.h>
#include <math.h>

#include <qregexp.h>

#include "spawnshell.h"
#include "fixpt.h"
#include "util.h"

//----------------------------------------------------------------------
// constants
const char * Spawn_Corpse_Designator = "'s corpse";

// fix point q format to use for spawn optimizations
const int qFormat = 14;

// used to calculate where the mob/player should be while animating
// 1.3 was figured empiraccly.. feel free to change it..  It seems to 
// be preety close though.
const float animationCoefficient = 0.0013;

// fixed point animation coefficient
const int animationCoefficientFixPt = 
   fixPtToFixed<int, float>(animationCoefficient, qFormat);

//----------------------------------------------------------------------
// Handy utility functions
// static 
QString print_item (uint16_t item)
{
  // sparse array of item names, some are NULL
  static const char*  itemnames[] = 
  {
#include "weapons.h"
  };

  // sparse array of item names (in 0x01 range), some are NULL
  static const char*  itemnames1[] = 
  {
#include "weapons1.h"
  };

  // sparse array of item names (in 0x27 range), some are NULL
  static const char*  itemnames27[] = 
  {
#include "weapons27.h"
  };

  // sparse array of item names (in 0x28 range), some are NULL
  static const char*  itemnames28[] = 
  {
#include "weapons28.h"
  };

  // sparse array of item names (in 0x29 range), some are NULL
  static const char*  itemnames29[] = 
  {
#include "weapons29.h"
  };

  // sparse array of item names (in 0x2a range), some are NULL
  static const char*  itemnames2a[] = 
  {
#include "weapons2a.h"
  };

  // sparse array of item names (in 0x2b range), some are NULL
  static const char*  itemnames2b[] = 
  {
#include "weapons2b.h"
  };

  // assume no material name found
  const char *itemStr = NULL;

  uint32_t itemLo = item & 0x00ff;
  uint32_t itemHi = (item & 0xff00) >> 8;

  if (itemHi == 0x00)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames) / sizeof (char*)))
      itemStr = itemnames[itemLo];
  }
  else if (itemHi == 0x01)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames1) / sizeof (char*)))
      itemStr = itemnames1[itemLo];
  }
  else if (itemHi == 0x27)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames27) / sizeof (char*)))
      itemStr = itemnames27[itemLo];
  }
  else if (itemHi == 0x28)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames28) / sizeof (char*)))
      itemStr = itemnames28[itemLo];
  }
  else if (itemHi == 0x29)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames29) / sizeof (char*)))
      itemStr = itemnames29[itemLo];
  }
  else if (itemHi == 0x2a)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames2a) / sizeof (char*)))
      itemStr = itemnames2a[itemLo];
  }
  else if (itemHi == 0x2b)
  {
    // retrieve pointer to item name
    if (itemLo < (sizeof(itemnames2b) / sizeof (char*)))
      itemStr = itemnames2b[itemLo];
  }

  // if race name exists, then return it, otherwise return a number string
  if (itemStr != NULL)
    return itemStr;
  else
  {
    QString item_str;
    item_str.sprintf("%#.2x", item);
    return item_str;
  }
}

//----------------------------------------------------------------------
// Item
Item::Item(spawnItemType t, uint16_t id)
  : m_filterFlags(0),
    m_runtimeFilterFlags(0),
    m_ID(id),
    m_NPC(99), // random bogus value
    m_type(t)
{
  m_spawnTime.start();
  m_lastUpdate.start();
  updateLastChanged();
}

Item::~Item()
{
}

QString Item::name() const
{
  return m_name;
}

QString Item::transformedName() const
{
  return name();
}

uint16_t Item::race() const
{
  return 0;
}

QString Item::raceString() const
{
  return "Unknown";
}

uint8_t Item::classVal() const
{
  return 0; 
}

QString Item::classString() const
{
  return "Unknown";
}

QString Item::info(const char* locationDelimiter) const
{
  return "";
}

QString Item::filterString() const
{
  QString buff;
  buff.sprintf("Name:%s:Race:%s:Class:%s:NPC:%d:X:%d:Y:%d:Z:%d",
	       (const char*)transformedName().utf8(),
	       (const char*)raceString(),
	       (const char*)classString(),
	       NPC(), x(), y(), z());
  return buff;
}

QString Item::dumpString() const
{
  return QString("ID:") + QString::number(id()) + ":" + filterString();
}

void Item::setPos(int16_t x, int16_t y, int16_t z)
{
  // set the item position
  setPoint(x, y, z);
}

void Item::setDistanceToPlayer(double dist)
{
    m_fdist = dist;
    m_idist = (uint32_t)m_fdist;
}

void Item::setDistanceToPlayer(uint32_t dist)
{
    m_idist = dist;
}

//----------------------------------------------------------------------
// Spawn
Spawn::Spawn()
  : Item(tSpawn, 0)
{
  m_name = "fake";
  m_lastName = "";
  setNPC(SPAWN_NPC_UNKNOWN);

  Item::setPos(0, 0, 0);
  setDeltas(0, 0, 0);
  setHeading(0, 0);
  setAnimation(0);
  setPetOwnerID(0);
  setLight(0);
  setGender(0);
  setDeity(0);
  setRace(0);
  setClassVal(0);
  setHP(0);
  setMaxHP(0);
  setGuildID(0xffff);
  setGuildTag(NULL);
  setLevel(0);
  setTypeflag(0);
  setGM(0);
  setIsPlayersGuildmate(0);
  setIsInPvPLevelRangeToPlayer(0);
  for (int i = 0; i < tNumWearSlots; i++)
    setEquipment(i, 0);

  // just clear the considred flag since data would be outdated
  setConsidered(false);

  // finally, note when this update occurred.
  updateLast();
}

Spawn::Spawn(const spawnStruct* s)
  : Item(tSpawn, s->spawnId)
{
  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);

  // have update initialize everything
  update(s);
}

Spawn::Spawn(uint16_t id, 
	     int16_t x, int16_t y, int16_t z,
	     int16_t deltaX, int16_t deltaY, int16_t deltaZ,
	     int8_t heading, int8_t deltaHeading,
	     uint8_t animation) 
  : Item(tSpawn, id)
{
  // apply the unknown mob values
  m_name = "unknown";
  m_lastName = "";
  setNPC(SPAWN_NPC_UNKNOWN);

  // set what is known
  setPos(x, y, z);
  setDeltas(deltaX, deltaY, deltaZ);
  setHeading(heading, deltaHeading);
  setAnimation(animation);
  
  // initialize what isn't to 0
  setPetOwnerID(0);
  setLight(0);
  setGender(0);
  setDeity(0);
  setRace(0);
  setClassVal(0);
  setHP(0);
  setMaxHP(0);
  setLevel(0);
  setGuildID(0xffff);
  setGuildTag(NULL);
  for (int i = 0; i < tNumWearSlots; i++)
    setEquipment(i, 0);
  setTypeflag(0);
  setGM(0);
  setConsidered(false);
  setIsPlayersGuildmate(0);
  setIsInPvPLevelRangeToPlayer(0);

  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);

  // Finally, note when this update ocurred
  updateLast();
}
		  
Spawn::Spawn(QDataStream& d, uint16_t id)
  : Item(tSpawn, id)
{
  // restore Spawn info
  d.readRawBytes((char*)this, sizeof(EQPoint));
  d.readRawBytes((char*)&m_lastUpdate, 
		 ((char*)this + sizeof(Item)) - (char*)&m_lastUpdate);
  d.readRawBytes((char*)&m_petOwnerID,
		 ((char*)this + sizeof(Spawn)) - (char*)&m_petOwnerID);
  d >> m_name;
  d >> m_lastName;

  // calculate race/deity team info
  calcRaceTeam();
  calcDeityTeam();
  
  // don't trust old movement data (minimize walkoffs causing scaling)
  setDeltas(0, 0, 0);
  setHeading(0, 0);

  // even if it had been considered, mark it as not
  setConsidered(false);
}

Spawn::Spawn(Spawn& s, uint16_t id)
  : Item(tSpawn, id)
{
  setName(s.name());
  setLastName(s.lastName());
  Item::setPoint(s.x(), s.y(), s.z());
  setPetOwnerID(s.petOwnerID());
  setLight(s.light());
  setGender(s.gender());
  setDeity(s.deity());
  setRace(s.race());
  setClassVal(s.classVal());
  setHP(s.HP());
  setMaxHP(s.maxHP());
  setGuildID(s.guildID());
  setGuildTag(s.guildTag());
  setLevel(s.level());
  for (int i = 0; i <= tLastCoreWearSlot; i++)
    setEquipment(i, s.equipment(i));
  setTypeflag(s.typeflag());
  setGM(s.gm());
  setNPC(s.NPC());
  setAnimation(s.animation());
  setDeltas(s.deltaX(), s.deltaY(), s.deltaZ());
  setHeading(s.heading(), s.deltaHeading());
  setConsidered(s.considered());
  setIsPlayersGuildmate(s.isPlayersGuildmate());
  setIsInPvPLevelRangeToPlayer(s.isInPvPLevelRangeToPlayer());

  // the new copy will own the spawn track list
  m_spawnTrackList.setAutoDelete(false);
  m_spawnTrackList = s.m_spawnTrackList;
  s.m_spawnTrackList.setAutoDelete(false);
  m_spawnTrackList.setAutoDelete(true);
}

Spawn::~Spawn()
{
  // clear out the spawn track list
  m_spawnTrackList.clear();
}

void Spawn::update(const spawnStruct* s)
{
  if (m_name.find(Spawn_Corpse_Designator) == -1)
  {
    setName(s->name);
    setLastName(s->lastName);
  }

  setSize(s->size);
  setPos(s->x >> 3, s->y >> 3, s->z >> 3);
  setPetOwnerID(s->petOwnerId);
  setLight(s->light);
  setGender(s->gender);
  setDeity(s->deity);
  setRace(s->race);
  setClassVal(s->class_);
  setHP(s->curHp);
  //setMaxHP(s->maxHp);
  setMaxHP(s->curHp); //maxHp is no longer part of the struct
  setGuildID(s->guildID);
  setLevel(s->level);
  for (int i = 0; i <= tLastCoreWearSlot; i++)
    setEquipment(i, s->equipment[i]);

  setTypeflag(s->bodytype);
  setGM(s->gm);

  if (s->NPC == SPAWN_PLAYER && s->deity == DEITY_UNKNOWN
          && s->race == 72 && s->bodytype == 5)
      setNPC(SPAWN_NPC); // Set to NPC if is a boat
  else
    setNPC(s->NPC); // otherwise it is what it is

  setAnimation(s->animation);

  // only non corpses and things with animation != 66 move
  if (!isCorpse() && (s->animation != 66))
  {
    setDeltas(s->deltaX >> 2, s->deltaY >> 2, s->deltaZ >> 2);
    setHeading(s->heading, s->deltaHeading);
  }
  else
  {
    setDeltas(0, 0, 0);
    setHeading(0, 0);
  }


  // just clear the considred flag since data would be outdated
  setConsidered(false);

  // finally, note when this update occurred.
  updateLast();
}

void Spawn::killSpawn()
{
  setDeltas(0, 0, 0);
  setHeading(0, 0);
  setHP(0);
  setMaxHP(0);
  if ((NPC() == SPAWN_PLAYER) || (NPC() == SPAWN_SELF) || (NPC() == SPAWN_PC_CORPSE))
    setNPC(SPAWN_PC_CORPSE);
  else
    setNPC(SPAWN_NPC_CORPSE);

   //setName(realName() + Spawn_Corpse_Designator);
}

void Spawn::setPos(int16_t x, int16_t y, int16_t z,
		   bool walkpathrecord, size_t walkpathlength)
{
  Item::setPos(x, y, z);
 
  if (walkpathrecord)
  {
    uint32_t count = m_spawnTrackList.count();

    // if this is the self spawn and this is the first spawn point, 
    // don't add it to the track list
    if ((m_NPC == SPAWN_SELF) && (count == 0) && 
	(x == 0) && (y == 0) && (z == 0))
      return;

    // only insert if the change includes either an x or y change, not just z
    if ((count == 0) ||
	((m_spawnTrackList.getLast()->x() != x) ||
	 (m_spawnTrackList.getLast()->y() != y)))
    {
      // if the walk path length is limited, make sure not to exceed the limit
      if ((walkpathlength > 0) && 
	  (count > 2) && (count > walkpathlength))
	m_spawnTrackList.removeFirst();

      // append the new entry to the end of the list
      m_spawnTrackList.append(new SpawnTrackPoint(x, y, z));
    }
  }
}

void Spawn::setDeltas(int16_t deltaX, int16_t deltaY, int16_t deltaZ)
{
  m_deltaX = deltaX; 
  m_deltaY = deltaY; 
  m_deltaZ = deltaZ; 
  m_cookedDeltaXFixPt = fixPtMulI(animationCoefficientFixPt, qFormat, 
				  m_deltaX);
  m_cookedDeltaYFixPt = fixPtMulI(animationCoefficientFixPt, qFormat, 
				  m_deltaY);
  m_cookedDeltaZFixPt = fixPtMulI(animationCoefficientFixPt, qFormat, 
				  m_deltaZ);
}

QString Spawn::lightName() const
{
#if 0
  // a non-sparse array of lightnames
  static const char*  lightnames[] = 
  {
    "",    // 0  - No light 
    "CDL", // 1  - Candle 
    "TR",  // 2  - Torch 
    "TGS", // 3  - Tiny Glowing Skull 
    "SL",  // 4  - Small Lantern 
    "SoM", // 5  - Stein of Moggok 
    "LL",  // 6  - Large Lantern 
    "FL",  // 7  - Flameless lantern, Halo of Light 
    "GOS", // 8  - Globe of stars 
    "LG",  // 9  - Light Globe
    "LS",  // 10 - Lightstone, Burnt-out lightstone, wispstone 
    "GLS", // 11 - Greater lightstone 
    "FBE", // 12 - Fire Beatle Eye, Firefly Globe
    "CL",  // 13 - Coldlight 
  };

  // return light name from list if it's within range
  if (light() < (sizeof(lightnames) / sizeof (char*)))
    return lightnames[light()];
  else
#endif
    return QString::number(light());
}

QString Spawn::equipmentStr(uint8_t wearingSlot) const
{
  if (wearingSlot <= tLastMaterial)
    return print_material(equipment(wearingSlot));
  else if (wearingSlot <= tLastWeapon)
    return print_item(equipment(wearingSlot));
  else if (wearingSlot < tNumWearSlots)
    return print_material(equipment(wearingSlot));
  else
    return "";
}

QString Spawn::genderName() const
{
  if (m_gender == 0)
    return "Male";
  if (m_gender == 1)
    return "Female";
  return "Neuter";
}

QString Spawn::deityName() const
{
  // make sure people don't add to list without modifying code to support
  static const char*  deitynames[(DEITY_VEESHAN - DEITY_BERT) + 1] = 
  {
#include "deity.h"
  };

  // if it's an NPC, return quickly
  if (deity() == DEITY_UNKNOWN)
    return "NPC";

  // if agnostic return it
  if (deity() == DEITY_AGNOSTIC)
    return "Agnostic";
  
  // if it is a deity in the table, retrieve and return it
  if ((deity() >= DEITY_BERT) && (deity() <= DEITY_VEESHAN))
  {
    // subtract out lowest #'d deity in list to give 0 offset
    int deityIndex = deity() - DEITY_BERT;
    
    // return deity name
    return deitynames[deityIndex];
  }

  // all else failed, so return a number
  return QString::number(deity());
}

void Spawn::calcDeityTeam()
{
  m_deityTeam = DTEAM_OTHER;

  switch(deity())
  {
    //Good
  case DEITY_EROL:
  case DEITY_MITH:
  case DEITY_RODCET:
  case DEITY_QUELLIOUS:
  case DEITY_TUNARE:
    m_deityTeam = DTEAM_GOOD;
    break;
    //Neutral
  case DEITY_BRELL:
  case DEITY_BRISTLE:
  case DEITY_KARANA:
  case DEITY_PREXUS:
  case DEITY_SOLUSEK:
  case DEITY_TRIBUNAL:
  case DEITY_VEESHAN:
    m_deityTeam = DTEAM_NEUTRAL;
    break;
    //Evil
  case DEITY_BERT:
  case DEITY_CAZIC:
  case DEITY_INNY:
  case DEITY_RALLOS:
    m_deityTeam = DTEAM_EVIL;
    break;
  }
}

void Spawn::calcRaceTeam()
{
  m_raceTeam = RTEAM_OTHER;

  switch(race())
  {
  case 1: // Human
  case 2: // Barb
  case 3:  // Erudite
  case 130: // Vah Shir
    m_raceTeam = RTEAM_HUMAN;
    break;
    
  case 4: // Wood Elf
  case 5: // High Elf
  case 7: // Half Elf
    m_raceTeam = RTEAM_ELF;
    break;
    
  case 6:  // Dark Elf
  case 9:  // Troll
  case 10: // Ogre
  case 128: // Iksar
    m_raceTeam = RTEAM_DARK;
    break;
    
  case 8:  // Dwarf
  case 11:  // Halfling 
  case 12:  // Gnome 
    m_raceTeam = RTEAM_SHORT;
    break;
  }
}

QString Spawn::lastName() const
{
  return m_lastName;
}

uint8_t Spawn::level() const 
{ 
  return m_level; 
}

uint16_t Spawn::deity() const 
{ 
  return m_deity; 
}

QString Spawn::cleanedName() const
{
  QString newName = name();
  newName.replace(QRegExp("[0-9]"), "");
  newName.replace(QRegExp("_"), " ");
  return newName;
}

QString Spawn::transformedName() const
{
  QString         temp = cleanedName();
  QString         article;
  bool            leading_hash = false;

  if (temp.startsWith( "#", false ))
  {
    temp.remove( 0, 1 );
    leading_hash = true;
  }

  if (temp.startsWith( "a ", false ))
  {
    temp.remove( 0, 2 );
    article = "a";
  }
  else if (temp.startsWith( "an ", false ))
  {
    temp.remove( 0, 3 );
    article = "an";
  }
  else if (temp.startsWith( "the ", false ))
  {
    temp.remove( 0, 4 );
    article = "the";
  }

  if (!article.isEmpty())
  {
    temp.append( ", " );
    temp.append( article );
  }

  if (leading_hash)
  {
    temp.prepend('#');
  }

  return temp;
}

uint16_t Spawn::race() const
{
  return m_race;
}

QString Spawn::raceString() const
{
  // sparse array of racenames, some are NULL
  static const char*  racenames[] = 
  {
#include "races.h"
  };

  // assume no racename found
  const char *raceString = NULL;

  // retrieve pointer to race name
  if (race() < (sizeof(racenames) / sizeof (char*)))
    raceString = racenames[race()];

  // if race name exists, then return it, otherwise return a number string
  if (raceString != NULL)
    return raceString;
  else
    return QString::number(race());
}

uint8_t Spawn::classVal() const
{
  return m_class;
}

QString Spawn::classString() const
{
  return ::classString(classVal());
}

QString Spawn::info(const char* locationDelimiter) const
{
  // Head, Chest, Arms, Waist, Gloves, Legs, Feet, Primary, Secondary
  static const char* locs[]={"H","C","A","W","G","L","F","1","2"};
  int i;
  QString temp = "";

 // Worn weapons
  for (i = tFirstWeapon; i <= tLastWeapon; i++)
    if (equipment(i))
      temp += QString(locs[i]) + ":" +  + print_item(equipment(i)) + locationDelimiter;

  // Worn stuff
  for (i = tFirstMaterial; i <= tLastMaterial ; i++)
    if (equipment(i))
      temp += QString(locs[i]) + ":" + print_material(equipment(i)) + locationDelimiter;

  // Add the light source to the list if it has one
  if (light())
    temp += QString("Light:") + lightName() + locationDelimiter;

  return temp;
}

QString Spawn::typeString() const
{
  static const char*  typenames[] =
  {
#include "typenames.h"
  };

  if ( (typeflag() < (sizeof(typenames) / sizeof (char*))) && (typenames[typeflag()] != NULL) )
    return typenames[typeflag()];
  else
    return QString::number(typeflag());
}

QString Spawn::filterString() const
{
  QString name = transformedName();

  QString buff;
  buff.sprintf("Name:%s:Level:%d:Race:%s:Class:%s:NPC:%d:IsPet:%d:X:%d:Y:%d:Z:%d:"
	       "Deity:%s:RTeam:%d:DTeam:%d:Type:%s:LastName:%s:Guild:%s:"
           "IsGuildmate:%d:InPvPLevelRange:%d:Size:%f:Equipment:%s:",
	       (const char*)name.utf8(),
	       level(),
	       (const char*)raceString(),
	       (const char*)classString(),
	       ((NPC() == 10) ? 0 : NPC()), 
           isPet(),
	       x(), y(), z(),
	       (const char*)deityName(),
	       raceTeam(), 
	       deityTeam(),
	       (const char*)typeString(),
	       (const char*)lastName().utf8(),
               (const char*)guildTag().utf8(),
               isPlayersGuildmate(),
               isInPvPLevelRangeToPlayer(),
               size(),
               (const char*)info(":"));

  if (gm())
    buff += QString("GM:") + QString::number(gm()) + ":";

  return buff;
}

QString Spawn::dumpString() const
{
  return QString("ID:") + QString::number(id()) 
    + ":Name:" + transformedName() 
    + ":Level:" + QString::number(level())
    + ":HP:" + QString::number(HP())
    + ":MaxHP:" + QString::number(maxHP())
    + ":Race:" + raceString()
    + ":Class:" + classString() 
    + ":NPC:" + QString::number(NPC())
    + ":IsPet:" + QString::number(isPet())
    + ":X:" + QString::number(x()) 
    + ":Y:" + QString::number(y())
    + ":Z:" + QString::number(z()) 
    + ":Deity:" + deityName() 
    + ":RTeam:" + QString::number(raceTeam())
    + ":DTeam:" + QString::number(deityTeam())
    + ":Type:" + typeString()
    + ":LastName:" + lastName() 
    + ":Guild:" + guildTag()
    + ":IsGuildmate:" + QString::number(isPlayersGuildmate())
    + ":InPvPLevelRange:" + QString::number(isInPvPLevelRangeToPlayer())
    + ":Size:" + size()
    + ":Equipment:" + ((const char*)info(":"))
    + ":FilterFlags:" + QString::number(filterFlags(),2)
    + ":";
}


bool Spawn::approximatePosition(bool animating, 
				const QTime& curTime,
				EQPoint& newPos) const
{
  // default is the current location of the spawn
  newPos.setPoint(*this);

  // get the amount of time since last update
  int msec = m_lastUpdate.msecsTo(curTime);

  if (msec < 0) // if passed midnight, adjust time accordingly
      msec += 86400 * 1000;

  // if it's been over 18 seconds, then don't adjust position
  if (msec > (18 * 1000))
  {
      return false;
  }

  // if animating calculate the current predicted position
  if (animating)
  {
      newPos.addPoint(fixPtMulII(m_cookedDeltaXFixPt, qFormat, msec),
              fixPtMulII(m_cookedDeltaYFixPt, qFormat, msec),
              fixPtMulII(m_cookedDeltaZFixPt, qFormat, msec));
  }

  return true;
}

void Spawn::saveSpawn(QDataStream& d)
{
  // dump spawn info
  // write out the raw spawn structure, skipping over the QStrings,
  // and SpawnTrackList (which can't be persisted in this fashion),
  // and the data we don't wan't copied over (heading/delta info).
  d.writeRawBytes((const char*)this, sizeof(EQPoint));
  d.writeRawBytes((const char*)&m_lastUpdate, 
		  ((char*)this + sizeof(Item)) 
		  - (char*)&m_lastUpdate);
  d.writeRawBytes((const char*)&m_petOwnerID,
		  ((char*)this + sizeof(Spawn)) - (char*)&m_petOwnerID);
  d << m_name;
  d << m_lastName;
}

//----------------------------------------------------------------------
// Door
Door::Door(const doorStruct* d)
  : Item(tDoors, d->doorId)
{
  m_NPC = SPAWN_DOOR;

  update(d);
}

Door::~Door()
{
}

void Door::update(const doorStruct* d)
{
  QString temp;
  setPos((int16_t)(d->x), 
	 (int16_t)(d->y), 
	 (int16_t)(d->z * 10.0));
  setHeading((int8_t)lrintf(d->heading));
  m_name.sprintf("Door: %s (%d) ", d->name, d->doorId);
  setZonePoint(d->zonePoint);
  updateLast();
}

QString Door::raceString() const
{
  return "Door";
}

QString Door::classString() const
{
  return "Thing";
}

//----------------------------------------------------------------------
// Drop
Drop::Drop(const makeDropStruct* d, const QString& name)
  : Item(tDrop, d->dropId)
{
  m_NPC = SPAWN_DROP;

  update(d, name);
}

Drop::~Drop()
{
}

void Drop::update(const makeDropStruct* d, const QString& name)
{
  int itemId;
  QString buff;

  // set the position
  setPos((int16_t)d->x, 
	 (int16_t)d->y, 
	 (int16_t)d->z);
  setHeading((int8_t)lrintf(d->heading));

  // set the drop specific info
  //setItemNr(d->itemNr);
  setIdFile(d->idFile);
  
  // calculate the drop name
  if (name.isEmpty())
  {
    if (d->idFile[0] == 'I' && d->idFile[1] == 'T')
    {
      buff = (d->idFile + 2);
      buff = buff.section('_', 0, 0, QString::SectionCaseInsensitiveSeps);
    }
    else
      buff = d->idFile;

    
    itemId = buff.toInt();
    
    buff = "Drop: ";
    if (itemId > 0) 
      buff.append(print_item(itemId));
    else 
      buff.append(d->idFile);
  }
  else
    buff = QString("Drop: '") + name + "'";
  
  // set the name 
  setName(buff);

  updateLast();
}

QString Drop::raceString() const
{
  return "Drop";
}

QString Drop::classString() const 
{
  return "Thing";
}
