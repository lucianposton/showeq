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
static 
QString print_weapon (uint16_t weapon)
{
  // sparse array of weapon names, some are NULL
  static const char*  weaponnames[] = 
  {
#include "weapons.h"
  };

  // sparse array of weapon names (in 0x27 range), some are NULL
  static const char*  weaponnames27[] = 
  {
#include "weapons27.h"
  };

  // sparse array of weapon names (in 0x28 range), some are NULL
  static const char*  weaponnames28[] = 
  {
#include "weapons28.h"
  };

  // sparse array of weapon names (in 0x2b range), some are NULL
  static const char*  weaponnames2b[] = 
  {
#include "weapons2b.h"
  };

  // assume no material name found
  const char *weaponStr = NULL;

  uint8_t weaponLo = weapon & 0x00ff;
  uint8_t weaponHi = (weapon & 0xff00) >> 8;

  if (weaponHi == 0x00)
  {
    // retrieve pointer to weapon name
    if (weaponLo < (sizeof(weaponnames) / sizeof (char*)))
      weaponStr = weaponnames[weaponLo];
  }
  else if (weaponHi == 0x27)
  {
    // retrieve pointer to weapon name
    if (weaponLo < (sizeof(weaponnames27) / sizeof (char*)))
      weaponStr = weaponnames27[weaponLo];
  }
  else if (weaponHi == 0x28)
  {
    // retrieve pointer to weapon name
    if (weaponLo < (sizeof(weaponnames28) / sizeof (char*)))
      weaponStr = weaponnames28[weaponLo];
  }
  else if (weaponHi == 0x2b)
  {
    // retrieve pointer to weapon name
    if (weaponLo < (sizeof(weaponnames2b) / sizeof (char*)))
      weaponStr = weaponnames2b[weaponLo];
  }

  // if race name exists, then return it, otherwise return a number string
  if (weaponStr != NULL)
    return weaponStr;
  else
  {
    QString weapon_str;
    weapon_str.sprintf("U%04x", weapon);
    return weapon_str;
  }
}

//----------------------------------------------------------------------
// Item
Item::Item(itemType t, uint16_t id)
  : m_filterFlags(0),
    m_runtimeFilterFlags(0),
    m_ID(id),
    m_NPC(99), // random bogus value
    m_type(t)
{
  m_spawnTime.start();
  m_lastUpdate.start();
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
  return m_name;
}

uint8_t Item::race() const
{
  return 0;
}

QString Item::raceName() const
{
  return "Unknown";
}

uint8_t Item::classVal() const
{
  return 0; 
}

QString Item::className() const
{
  return "Unknown";
}

QString Item::info() const
{
  return "";
}

QString Item::filterString() const
{
  return QString("Name:") + transformedName() + ":Race:" + raceName()
    + ":Class:" + className() + ":NPC:" + QString::number(NPC())
    + ":X:" + QString::number(xPos()) + ":Y:" + QString::number(yPos())
    + ":Z:" + QString::number(zPos());
}

QString Item::dumpString() const
{
  return QString("ID:") + QString::number(id()) + ":" + filterString();
}

void Item::setPos(int16_t xPos, int16_t yPos, int16_t zPos)
{
  setPoint(xPos, yPos, zPos);
}

//----------------------------------------------------------------------
// Spawn
Spawn::Spawn()
  : Item(tSpawn, 0)
{
  m_name = "fake";
  m_rawName = m_name;
  setNPC(SPAWN_NPC_UNKNOWN);

  Item::setPos(0, 0, 0);
  setDeltas(0, 0, 0);
  setHeading(0, 0);
  setPetOwnerID(0);
  setLight(0);
  setGender(0);
  setDeity(0);
  setRace(0);
  setClassVal(0);
  setHP(0);
  setMaxHP(0);
  setLevel(0);
  for (int i = 0; i < 9; i++)
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

  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);
}

Spawn::Spawn(uint16_t id, const spawnStruct* s)
  : Item(tSpawn, id)
{
  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);

  // have update initialize everything
  update(s);

  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);
}

Spawn::Spawn(Spawn* item, uint16_t id)
  : Item(tSpawn, id)
{
  // copy over all the values but the ID.
  setName(item->realName());
  m_rawName = item->rawName();
  Item::setPos(item->xPos(), item->yPos(), item->zPos());
  setDeltas(item->deltaX(), item->deltaY(), item->deltaZ());
  setHeading(item->heading(), item->deltaHeading());
  setPetOwnerID(item->petOwnerID());
  setLight(item->light());
  setGender(item->gender());
  setDeity(item->deity());
  setRace(item->race());
  setClassVal(item->classVal());
  setHP(item->HP());
  setMaxHP(item->maxHP());
  setLevel(item->level());
  
  for (int i = 0; i < 9; i++)
    setEquipment(i, item->equipment(i));

  setNPC(item->NPC());
  setConsidered(item->considered());

  // tell the old object not to auto delete it's track list
  item->m_spawnTrackList.setAutoDelete(false);

  // copy the track list
  m_spawnTrackList = item->trackList();

  // clear the old objects copy of the track list
  item->m_spawnTrackList.clear();
  
  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);
}

Spawn::Spawn(uint16_t id, 
	     const QString& name, 
	     const QString& lastName,
	     uint8_t race, uint8_t classVal,
	     uint8_t level, uint8_t deity)
  : Item(tSpawn, id)
{
  // set what is known
  m_rawName = name;
  m_name = name;
  if (!lastName.isEmpty())
    m_name += QString (" (") + lastName + ")";

  setNPC(SPAWN_SELF);
  setRace(race);
  setClassVal(classVal);
  setLevel(level);
  setDeity(deity);

  // initialize what isn't to 0
  Item::setPos(0, 0, 0);
  setDeltas(0, 0, 0);
  setHeading(0, 0);
  setPetOwnerID(0);
  setLight(0);
  setGender(0);
  setHP(0);
  setMaxHP(0);
  for (int i = 0; i < 9; i++)
    setEquipment(i, 0);
  setConsidered(false);

  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);
}

Spawn::Spawn(const playerProfileStruct* player, 
	     uint8_t deity)
  : Item(tSpawn, 0)
{
  // set what's known 
  setNPC(SPAWN_SELF);

  setRace(player->race);
  setClassVal(player->class_);
  setLevel(player->level);
  setGender(player->gender);
  setDeity(deity);
  
  // save the raw name
  m_rawName = player->name;

  // start with the first name
  m_name = player->name;

  // if it's got a last name add it
  if (player->lastName[0] != 0)
    m_name += QString (" (") + player->lastName + ")";

  // initialize what isn't to 0
  Item::setPos(0, 0, 0);
  setDeltas(0, 0, 0);
  setHeading(0, 0);
  setPetOwnerID(0);
  setLight(0);
  setHP(0);
  setMaxHP(0);
  for (int i = 0; i < 9; i++)
    setEquipment(i, 0);
  setConsidered(false);

  // turn on auto delete for the track list
  m_spawnTrackList.setAutoDelete(true);
}


Spawn::Spawn(uint16_t id, 
	     int16_t xPos, int16_t yPos, int16_t zPos,
	     int16_t deltaX, int16_t deltaY, int16_t deltaZ,
	     int8_t heading, int8_t deltaHeading) 
  : Item(tSpawn, id)
{
  // apply the unknown mob values
  m_rawName = "unknown";
  m_name = "unknown";
  setNPC(SPAWN_NPC_UNKNOWN);

  // set what is known
  setPos(xPos, yPos, zPos);
  setDeltas(deltaX, deltaY, deltaZ);
  setHeading(heading, deltaHeading);
  
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
  for (int i = 0; i < 9; i++)
    setEquipment(i, 0);
  setConsidered(false);

  // turn on auto delete for the track list
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
    m_rawName = s->name;
    m_name = s->name;
    if (s->lastname[0] != 0)
      m_name += QString (" (") + s->lastname + ")";
  }

  setPos(s->xPos, s->yPos, s->zPos);
  setPetOwnerID(s->petOwnerId);
  setLight(s->light);
  setGender(s->Gender);
  setDeity(s->deity);
  setRace(s->race);
  setClassVal(s->class_);
  setHP(s->curHp);
  setMaxHP(s->maxHp);
  setLevel(s->level);
  for (int i = 0; i < 9; i++)
    setEquipment(i, s->equipment[i]);

  // If it is a corpse with Unknown (NPC) religion.
  if ((s->NPC == SPAWN_PC_CORPSE) && (s->deity == DEITY_UNKNOWN))
    setNPC(SPAWN_NPC_CORPSE); // it's a dead monster
  else
    setNPC(s->NPC); // otherwise it is what it is

  // only non corpses move
  if (!isCorpse())
  {
    setDeltas(s->deltaX, s->deltaY, s->deltaZ);
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

void Spawn::backfill(const spawnStruct* s)
{
  int i;

  // set the characteristics that probably haven't changed.
  setGender(s->Gender);
  setDeity(s->deity);
  setRace(s->race);
  setClassVal(s->class_);

  // don't know how we'd find out if this changed, but it may, currently 
  // no-check
  setPetOwnerID(s->petOwnerId);

  // preserve the NPC setting, have they died since the spawn info
  if ((m_NPC == SPAWN_PC_CORPSE) ||
      (m_NPC == SPAWN_NPC_CORPSE))
  {
    // No hit points for the dead, sorry.  You're not undead yet.
    setHP(0);

    // Set whether this is a player or monster corpse
    if ((s->NPC == SPAWN_PLAYER) || (s->NPC == SPAWN_SELF))
      setNPC(SPAWN_PC_CORPSE); // Player corpse
    else
      setNPC(SPAWN_NPC_CORPSE); // Monster corpse
    
    //If it is a corpse with Unknown (NPC) religion.
    if (s->NPC == SPAWN_PC_CORPSE && s->deity == DEITY_UNKNOWN)
      setNPC(SPAWN_NPC_CORPSE);  //It is a dead monster.
  }  // only change NPC value if it's unknown
  else if (m_NPC == SPAWN_NPC_UNKNOWN)
  {
    // If it is a corpse with Unknown (NPC) religion.
    if ((s->NPC == SPAWN_PC_CORPSE) && (s->deity == DEITY_UNKNOWN))
      setNPC(SPAWN_NPC_CORPSE); // it's a dead monster
    else
      setNPC(s->NPC); // otherwise it is what it is
  }

  // start with the first name
  m_rawName = s->name;
  m_name = s->name;

  // if it's got a last name add it
  if (s->lastname[0] != 0)
    m_name += QString (" (") + s->lastname + ")";

  // if it's dead,  append the corpse designator and make sure it's not moving
  if (isCorpse())
  {
    m_name += Spawn_Corpse_Designator;
    m_rawName += Spawn_Corpse_Designator;
    setDeltas(0, 0, 0);
    setHeading(0, 0);
  }

  // only change unknown equipment
  for (i = 0; i < 9; i++)
    if (equipment(i) == 0)
      setEquipment(i, s->equipment[i]);

  // only change unknown or no light
  if (light() == 0)
    setLight(s->light);

  // only set the level if it's higher (not perfect I know)
  if (m_level < s->level)
    setLevel(s->level);
}

void Spawn::backfill(const playerProfileStruct* player)
{
  // set the characteristics that probably haven't changed.
  setNPC(SPAWN_SELF);
  setGender(player->gender);
  setRace(player->race);
  setClassVal(player->class_);
  setLevel(player->level);

  // save the raw name
  m_rawName = player->name;

  // start with the first name
  m_name = player->name;

  // if it's got a last name add it
  if (player->lastName[0] != 0)
    m_name += QString (" (") + player->lastName + ")";

  if (level() < player->level)
    setLevel(player->level);
}

void Spawn::killSpawn()
{
  setDeltas(0, 0, 0);
  setHeading(0, 0);
  setHP(0);
  setMaxHP(0);
  if ((NPC() == SPAWN_PLAYER) || (NPC() == SPAWN_SELF))
    setNPC(SPAWN_PC_CORPSE);
  else
    setNPC(SPAWN_NPC_CORPSE);
  
  setName(realName() + Spawn_Corpse_Designator);
}

void Spawn::setPos(int16_t xPos, int16_t yPos, int16_t zPos,
			     bool walkpathrecord, size_t walkpathlength)
{
  Item::setPos(xPos, yPos, zPos);
 
  if (walkpathrecord)
  {
    uint32_t count = m_spawnTrackList.count();

    // if this is the self spawn and this is the first spawn point, 
    // don't add it to the track list
    if ((m_NPC == SPAWN_SELF) && (count == 0) && 
	(xPos == 0) && (yPos == 0) && (zPos == 0))
      return;

    // only insert if the change includes either an x or y change, not just z
    if ((count == 0) ||
	((m_spawnTrackList.getLast()->xPos() != xPos) ||
	 (m_spawnTrackList.getLast()->yPos() != yPos)))
    {
      // if the walk path length is limited, make sure not to exceed the limit
      if ((walkpathlength > 0) && 
	  (count > 2) && (count > walkpathlength))
	m_spawnTrackList.removeFirst();

      // append the new entry to the end of the list
      m_spawnTrackList.append(new SpawnTrackPoint(xPos, yPos, zPos));
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
    return QString::number(light());
}

QString Spawn::equipmentStr(uint8_t wearingSlot) const
{
  if (wearingSlot < 7)
    return print_material(equipment(wearingSlot));
  else if (wearingSlot < wearingSlot)
    return print_weapon(equipment(wearingSlot));
  else
    return "";
}


QString Spawn::deityName() const
{
  // make sure people don't add to list without modifying code to support
  static const char*  deitynames[(DEITY_VEESHAN - DEITY_BERT) + 1] = 
  {
#include "deity.h"
  };

  // if it's an NPC, return quickly
  if (deity() == 0)
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
    m_raceTeam = RTEAM_DARK;
    break;
    
  case 8:  // Dwarf
  case 11:  // Halfling 
  case 12:  // Gnome 
    m_raceTeam = RTEAM_SHORT;
    break;
  }
}

QString Spawn::cleanedName() const
{
  QString newName = m_name;
  newName.replace(QRegExp("[0-9]"), "");
  newName.replace(QRegExp("_"), " ");
  return newName;
}

QString Spawn::transformedName() const
{
  QString         temp = cleanedName();
  QString         article;
  
  if (temp.startsWith( "a " ))
  {
    temp = temp.mid( 2 );
    article = "a";
  }
  else if (temp.startsWith( "an " ))
  {
    temp = temp.mid( 3 );
    article = "an";
  }
  else if (temp.startsWith( "the " ))
  {
    temp = temp.mid( 4 );
    article = "the";
  }

  if (!article.isEmpty())
  {
    temp += ", ";
    temp += article;
  }

  return temp;
}

uint8_t Spawn::race() const
{
  return m_race;
}

QString Spawn::raceName() const
{
  // sparse array of racenames, some are NULL
  static const char*  racenames[] = 
  {
#include "races.h"
  };

  // assume no racename found
  const char *raceName = NULL;

  // retrieve pointer to race name
  if (race() < (sizeof(racenames) / sizeof (char*)))
    raceName = racenames[race()];

  // if race name exists, then return it, otherwise return a number string
  if (raceName != NULL)
    return raceName;
  else
    return QString::number(race());
}

uint8_t Spawn::classVal() const
{
  return m_class;
}

QString Spawn::className() const
{
  // a non-sparse array of class names
  static const char*  classnames[] = 
  {
#include "classes.h"
  };

  // return class name from list if it's within range
  if (classVal() < (sizeof(classnames) / sizeof (char*)))
    return classnames[classVal()];
  else
    return QString::number(classVal());
}

QString Spawn::info() const
{
  static const char* locs[]={"H","C","A","W","G","L","F","1","2"};
  int i;
  QString temp = "";
  
  // Add the light source to the list if it has one
  if (light())
    temp += QString("Light:") + lightName() + " ";

  // Worn stuff
  for (i = 0; i < 7 ; i++)
    if (equipment(i))
      temp += QString(locs[i]) + print_material(equipment(i)) + " ";

  // Worn weapons
  for (i = 7; i < 9; i++)
    if (equipment(i))
      temp += QString(locs[i]) + print_weapon(equipment(i)) + " ";

  return temp;
}

QString Spawn::filterString() const
{
  return QString("Name:") + transformedName() 
    + ":Level:" + QString::number(level())
    + ":HP:" + QString::number(HP())
    + ":MaxHP:" + QString::number(maxHP())
    + ":Race:" + raceName()
    + ":Class:" + className() 
    + ":NPC:" + QString::number((NPC() == 10) ? 0 : NPC())
    + ":X:" + QString::number(xPos()) 
    + ":Y:" + QString::number(yPos())
    + ":Z:" + QString::number(zPos()) 
    + ":Light:" + lightName()
    + ":Deity:" + deityName() 
    + ":RTeam:" + QString::number(raceTeam())
    + ":DTeam:" + QString::number(deityTeam())
    + ":";
}

QString Spawn::dumpString() const
{
  return QString("ID:") + QString::number(id()) 
    + ":Name:" + transformedName() 
    + ":Level:" + QString::number(level())
    + ":HP:" + QString::number(HP())
    + ":MaxHP:" + QString::number(maxHP())
    + ":Race:" + raceName()
    + ":Class:" + className() 
    + ":NPC:" + QString::number(NPC())
    + ":X:" + QString::number(xPos()) 
    + ":Y:" + QString::number(yPos())
    + ":Z:" + QString::number(zPos()) 
    + ":Deity:" + deityName() 
    + ":RTeam:" + QString::number(raceTeam())
    + ":DTeam:" + QString::number(deityTeam())
    + ":FilterFlags:" + QString::number(filterFlags())
    + ":";
}


bool Spawn::approximatePosition(bool animating, 
				const QTime& curTime,
				EQPoint& newPos) const
{
  // default is the current location of the spawn
  newPos.setPoint(*this);

  // if animating calculate the current predicted position
  if (animating)
  {
    // get the amount of time since last update
    int msec = m_lastUpdate.msecsTo(curTime); 
    
    if (msec < 0) // if passed midnight, adjust time accordingly
      msec += 86400 * 1000;
    
    // if it's been over 90 seconds, then don't adjust position
    if (msec < (90 * 1000))
    {
      newPos.addPoint(fixPtMulII(m_cookedDeltaXFixPt, qFormat, msec),
		      fixPtMulII(m_cookedDeltaYFixPt, qFormat, msec),
		      fixPtMulII(m_cookedDeltaZFixPt, qFormat, msec));

      return true;
    }
    else
      return false;
  }

  return true;
}

//----------------------------------------------------------------------
// Coin
Coin::Coin(const dropCoinsStruct* c)
  : Item(tCoins, c->dropId)
{
  m_NPC = SPAWN_COINS;

  update(c);
}

Coin::~Coin()
{
}

void Coin::update(const dropCoinsStruct* c)
{
  QString temp;
  setPos((int16_t)(c->xPos), 
	 (int16_t)(c->yPos), 
	 (int16_t)(c->zPos));
  setAmount(c->amount);
  setCoinType(c->type[0]);
  m_name.sprintf("Coints: %c %d", c->type[0], c->amount);
  updateLast();
}

QString Coin::raceName() const
{
  return "Coins";
}

QString Coin::className() const
{
  return "Thing";
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
  setPos((int16_t)(d->xPos), 
	 (int16_t)(d->yPos), 
	 (int16_t)(d->zPos));
  m_name.sprintf("Door: %s (%d) ", d->name, d->doorId);
  updateLast();
}

QString Door::raceName() const
{
  return "Door";
}

QString Door::className() const
{
  return "Thing";
}

//----------------------------------------------------------------------
// Drop
Drop::Drop(const dropThingOnGround* d, const QString& name)
  : Item(tDrop, d->dropId)
{
  m_NPC = SPAWN_DROP;

  update(d, name);
}

Drop::~Drop()
{
}

void Drop::update(const dropThingOnGround* d, const QString& name)
{
  int itemId;
  QString buff;

  // set the position
  setPos((int16_t)d->xPos, 
	 (int16_t)d->yPos, 
	 (int16_t)d->zPos);

  // set the drop specific info
  setItemNr(d->itemNr);
  setIdFile(d->idFile);
  
  // calculate the drop name
  if (name.isEmpty())
  {
    if (d->idFile[0] == 'I' && d->idFile[1] == 'T')
      buff = (d->idFile + 2);
    else
      buff = d->idFile;
    
    itemId = buff.toInt();
    
    buff = "Drop: ";
    if (itemId > 0) 
      buff.append(print_weapon(itemId));
    else 
      buff.append(d->idFile);
  }
  else
    buff = QString("Drop: '") + name + "'";
  
  // set the name 
  setName(buff);

  updateLast();
}

QString Drop::raceName() const
{
  return "Drop";
}

QString Drop::className() const 
{
  return "Thing";
}
