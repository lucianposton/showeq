/*
 * player.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include "player.h"
#include "util.h"
#include "packetcommon.h"
#include "diagnosticmessages.h"
#include "guild.h"
#include "zonemgr.h"
#include "main.h"

#include <stdio.h>
#include <unistd.h>

#include <qdir.h>
#include <qfile.h>
#include <qdatastream.h>


//#define DEBUG_PLAYER

//----------------------------------------------------------------------
// constants
static const char magicStr[5] = "plr2"; // magic is the size of uint32_t + a null
static const uint32_t* magic = (uint32_t*)magicStr;

static const char* conColorBasePrefNames[] =
{
  "GreenBase",
  "CyanBase",
  "BlueBase",
  "Even",
  "YellowBase",
  "RedBase",
  "Unknown"
};


Player::Player (QObject* parent,
		ZoneMgr* zoneMgr,
		GuildMgr* guildMgr,
		const char* name)
  : QObject(parent, name),
    Spawn(),
    m_zoneMgr(zoneMgr),
    m_guildMgr(guildMgr)
{
#ifdef DEBUG_PLAYER
  debug("Player()");
#endif

  connect(m_zoneMgr, SIGNAL(zoneBegin(const ServerZoneEntryStruct*, size_t, uint8_t)),
          this, SLOT(zoneBegin(const ServerZoneEntryStruct*)));
  connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
          this, SLOT(zoneChanged()));
  
  m_NPC = SPAWN_SELF;

  QString section = "Defaults";
  m_useAutoDetectedSettings = 
    pSEQPrefs->getPrefBool("useAutoDetectedSettings", section, true);
  m_defaultName = pSEQPrefs->getPrefString("DefaultName", section, "You");
  m_defaultLastName = pSEQPrefs->getPrefString("DefaultLastName", section, "");
  m_defaultLevel = pSEQPrefs->getPrefInt("DefaultLevel", section, 1);
  m_defaultRace = pSEQPrefs->getPrefInt("DefaultRace", section, 1);
  m_defaultClass = pSEQPrefs->getPrefInt("DefaultClass", section, 1);
  m_defaultDeity = pSEQPrefs->getPrefInt("DefaultDeity", section, DEITY_AGNOSTIC);
  
  setUseDefaults(true);
  
  // set the name to the default name
  Spawn::setName(m_defaultName);

  m_conColorBases[tGreenSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tGreenSpawn],
			    "Player",
			    QColor(0, 95, 0));
  m_conColorBases[tCyanSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tCyanSpawn],
			    "Player",
			    QColor(0, 255, 255));
  m_conColorBases[tBlueSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tBlueSpawn],
			    "Player",
			    QColor(0, 0, 160));
  m_conColorBases[tEvenSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tEvenSpawn],
			    "Player",
			    QColor(255, 255, 255));
  m_conColorBases[tYellowSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tYellowSpawn],
			    "Player",
			    QColor(255, 255, 0));
  m_conColorBases[tRedSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tRedSpawn],
			    "Player",
			    QColor(127, 0, 0));
  m_conColorBases[tUnknownSpawn] = 
    pSEQPrefs->getPrefColor(conColorBasePrefNames[tUnknownSpawn],
			    "Player",
			    gray);
						 
  // restore the player state if the user requested it...
  if (showeq_params->restorePlayerState)
    restorePlayerState();
  else
  {
    reset();
    clear();
  }
}

Player::~Player()
{
}

void Player::clear()
{
  m_plusMana = 0; 
  m_plusHP = 0;
  m_maxSTR = 0;
  m_maxSTA = 0;
  m_maxCHA = 0;
  m_maxDEX = 0;
  m_maxINT = 0;
  m_maxAGI = 0;
  m_maxWIS = 0;
  m_maxMana = 0;
  m_maxHP = 0;
  m_curHP = 0;
  m_food = 0;
  m_water = 0;
  m_fatigue = 0;

  m_validStam = false;
  m_validMana = false;
  m_validHP = false;
  m_validAttributes = false;

  m_lastSpawnKilledName = "unknown";
  m_lastSpawnKilledLevel = 0;
  m_freshKill = false;

  m_heading = 0;
  m_headingDegrees = 360 - ((m_heading * 360) >> 11);
  
  setID(0);
  setPoint(0,0,0);
  m_validPos = false;

  updateLastChanged();
}

void Player::reset()
{
  setUseDefaults(true);

  m_currentAltExp = 0;
  m_currentExp = 0;
  m_minExp = calc_exp(level() - 1, race(), classVal());
  m_maxExp = calc_exp(level(), race(), classVal ());
  m_tickExp = (m_maxExp - m_minExp) / 330;

  for (int a = 0; a < MAX_KNOWN_SKILLS; a++)
    m_playerSkills[a] = 255; // indicate an invalid value

  for (int a = 0; a < MAX_KNOWN_LANGS; a++)
    m_playerLanguages[a] = 255; // indicate an invalid value
  
  m_mana = 0;
  setLevel (1);
  setRace (1);
  setClassVal (1);

  emit deleteSkills();
  emit deleteLanguages();

  m_validExp = false;
  m_validAttributes = false;

  // update the con table
  fillConTable();

  updateLastChanged();
}

void Player::setUseAutoDetectedSettings(bool enable)
{
  m_useAutoDetectedSettings = enable;
  pSEQPrefs->setPrefBool("useAutoDetectedSettings", "Defaults", enable);
  fillConTable();
}

void Player::setDefaultName(const QString& name)
{
  m_defaultName = name;
  pSEQPrefs->setPrefString("DefaultName", "Defaults", name);
}

void Player::setDefaultLastname(const QString& lastName)
{
  m_defaultLastName = lastName;
  pSEQPrefs->setPrefString("DefaultLastName", "Defaults", lastName);
}

void Player::setDefaultLevel(uint8_t level)
{
  m_defaultLevel = level;
  pSEQPrefs->setPrefInt("DefaultLevel", "Defaults", level);
  if (!m_useAutoDetectedSettings || m_useDefaults)
    fillConTable();
}

void Player::setDefaultRace(uint16_t race)
{
  m_defaultRace = race;
  pSEQPrefs->setPrefInt("DefaultRace", "Defaults", race);
}

void Player::setDefaultClass(uint8_t classVal)
{
  m_defaultClass = classVal;
  pSEQPrefs->setPrefInt("DefaultClass", "Defaults", classVal);
}

void Player::setDefaultDeity(uint16_t deity)
{
  m_defaultDeity = deity;
  pSEQPrefs->setPrefInt("DefaultDeity", "Defaults", deity);
}

void Player::player(const uint8_t* data)
{
  const charProfileStruct* player = (const charProfileStruct*)data;
  QString messag;

  if (m_name != player->name)
    emit newPlayer();
  
  // fill in base Spawn class
  // set the characteristics that probably haven't changed.
  setNPC(SPAWN_SELF);
  setGender(player->gender);
  setRace(player->race);
  setClassVal(player->class_);
  setLevel(player->level);
  m_curHP = player->curHp;

  // save the raw name
  setTypeflag(1);

  Spawn::setName(player->name);

  // if it's got a last name add it
  setLastName(player->lastName);

  // set the player level
  setLevel(player->level);

  // Stats hanling
  setUseDefaults(false);
  setDeity(player->deity);
  setGuildID(player->guildID);
  setGuildTag(m_guildMgr->guildIdToName(guildID()));
  emit guildChanged();

#if 1 // ZBTEMP
  seqDebug("charProfile(%f/%f/%f - %f)",
	   player->x, player->y, player->z, player->heading);
#endif
  setPos((int16_t)lrintf(player->x), 
         (int16_t)lrintf(player->y), 
         (int16_t)lrintf(player->z),
	 showeq_params->walkpathrecord,
	 showeq_params->walkpathlength
        );
  setDeltas(0,0,0);
#if 1 // ZBTEMP
  seqDebug("Player::backfill(): Pos (%f/%f/%f) Heading: %f",
	   player->x, player->y, player->z, player->heading);
  seqDebug("Player::backfill(bind): Pos (%f/%f/%f) Heading: %f",
	   player->bind_x[0], player->bind_y[0], player->bind_z[0], 
       player->bind_heading[0]);
#endif // ZBTEMP  
  setHeading((int8_t)lrintf(player->heading), 0);
  m_headingDegrees = 360 - ((((int8_t)lrintf(player->heading)) * 360) >> 11);
  m_validPos = true;
  emit headingChanged(m_headingDegrees);
  emit posChanged(x(), y(), z(), 
		  deltaX(), deltaY(), deltaZ(), m_headingDegrees);

  // Due to the delayed decode, we must reset
  // maxplayer on zone and accumulate all totals.
  m_maxSTR += player->STR;
  m_maxSTA += player->STA;
  m_maxCHA += player->CHA;
  m_maxDEX += player->DEX;
  m_maxINT += player->INT;
  m_maxAGI += player->AGI;
  m_maxWIS += player->WIS;
  
  emit statChanged (LIST_STR, m_maxSTR, m_maxSTR);
  emit statChanged (LIST_STA, m_maxSTA, m_maxSTA);
  emit statChanged (LIST_CHA, m_maxCHA, m_maxCHA);
  emit statChanged (LIST_DEX, m_maxDEX, m_maxDEX);
  emit statChanged (LIST_INT, m_maxINT, m_maxINT);
  emit statChanged (LIST_AGI, m_maxAGI, m_maxAGI);
  emit statChanged (LIST_WIS, m_maxWIS, m_maxWIS);
  
  m_mana = player->MANA;

  m_maxMana = calcMaxMana( m_maxINT, m_maxWIS,
                           m_class, m_level
			 ) + m_plusMana;
  
  emit manaChanged(m_mana, m_maxMana);  // need max mana

  
  // Merge in our new skills...
  for (int a = 0; a < MAX_KNOWN_SKILLS; a++)
  {
    m_playerSkills[a] = player->skills[a];

    emit addSkill (a, m_playerSkills[a]);
  }

  // Merge in our new languages...
  for (int a = 0; a < MAX_KNOWN_LANGS; a++)
  {
    m_playerLanguages[a] = player->languages[a];
    
    emit addLanguage (a, m_playerLanguages[a]);
  }

  // copy in the spell book
  memcpy (&m_spellBookSlots[0], &player->sSpellBook[0], sizeof(m_spellBookSlots));

  // Move 
  m_validAttributes = true;
  m_validMana = true;
  m_validExp = true;

  // update the con table
  fillConTable();

  // Exp handling
  m_minExp = calc_exp(m_level-1, m_race, m_class);
  m_maxExp = calc_exp(m_level, m_race, m_class);
  m_tickExp = (m_maxExp - m_minExp) / 330;

  m_currentExp = player->exp;
  m_currentAltExp = player->expAA;
  m_currentAApts = player->aa_spent;
  
  emit expChangedInt (m_currentExp, m_minExp, m_maxExp);
  emit expAltChangedInt(m_currentAltExp, 0, 15000000);

  emit setAltExp(m_currentAltExp, 15000000, 15000000/330, m_currentAApts);

  if (showeq_params->savePlayerState)
    savePlayerState();

  updateLastChanged();

  emit changeItem(this, tSpawnChangedALL);

  QDir tmp("/tmp/");
  tmp.rename(QString("bankfile.") + QString::number(getpid()),
	     QString("bankfile.") + player->name);

  //Added by Halcyon
  int buffnumber;
  const struct spellBuff *buff;
  for (buffnumber=0;buffnumber<MAX_BUFFS;buffnumber++)
  {
    if (player->buffs[buffnumber].spellid && player->buffs[buffnumber].duration)
    {
      buff = &(player->buffs[buffnumber]);
      emit buffLoad(buff);
    }
  }
}

#if 0 // ZBTEMP
void Player::wearItem(const playerItemStruct* itemp)
{
  const itemItemStruct* item = &itemp->item;

  if (item->equipSlot < 22)
  {
    bool manaAdjust = false;

    if (item->STR != 0)
    {
      m_maxSTR += item->STR;
      emit statChanged (LIST_STR, m_maxSTR, m_maxSTR);
    }
    
    if (item->STA != 0)
    {
      m_maxSTA += item->STA;
      emit statChanged (LIST_STA, m_maxSTA, m_maxSTA);
    }

    if (item->CHA != 0)
    {
      m_maxCHA += item->CHA;
      emit statChanged (LIST_CHA, m_maxCHA, m_maxCHA);
    }

    if (item->DEX != 0)
    {
      m_maxDEX += item->DEX;
      emit statChanged (LIST_DEX, m_maxDEX, m_maxDEX);
    }

    if (item->INT != 0)
    {
      m_maxINT += item->INT;
      emit statChanged (LIST_INT, m_maxINT, m_maxINT);
      manaAdjust = true;
    }

    if (item->AGI != 0)
    {
      m_maxAGI += item->AGI;
      emit statChanged (LIST_AGI, m_maxAGI, m_maxAGI);
    }

    if (item->WIS != 0)
    {
      m_maxWIS += item->WIS;
      emit statChanged (LIST_WIS, m_maxWIS, m_maxWIS);
      manaAdjust = true;
    }

    m_plusHP += item->HP;

    if (item->MANA != 0)
    {
      m_plusMana += item->MANA;
      manaAdjust = true;
    }

    if (manaAdjust)
    {
      m_maxMana = calcMaxMana( m_maxINT,
			       m_maxWIS,
			       classVal(),
			       level()
			       )  +  m_plusMana;
      
      emit manaChanged(m_mana, m_maxMana);  /* Needs max mana */

      m_validMana = true;
    }

    if (showeq_params->savePlayerState)
      savePlayerState();
  }
}

void Player::removeItem(const itemItemStruct* item)
{
  if (item->equipSlot < 22)
  {
    bool manaAdjust = false;

    if (item->STR != 0)
    {
      m_maxSTR -= item->STR;
      emit statChanged (LIST_STR, m_maxSTR, m_maxSTR);
    }
    
    if (item->STA != 0)
    {
      m_maxSTA -= item->STA;
      emit statChanged (LIST_STA, m_maxSTA, m_maxSTA);
    }

    if (item->CHA != 0)
    {
      m_maxCHA -= item->CHA;
      emit statChanged (LIST_CHA, m_maxCHA, m_maxCHA);
    }

    if (item->DEX != 0)
    {
      m_maxDEX -= item->DEX;
      emit statChanged (LIST_DEX, m_maxDEX, m_maxDEX);
    }

    if (item->INT != 0)
    {
      m_maxINT -= item->INT;
      emit statChanged (LIST_INT, m_maxINT, m_maxINT);
      manaAdjust = true;
    }

    if (item->AGI != 0)
    {
      m_maxAGI -= item->AGI;
      emit statChanged (LIST_AGI, m_maxAGI, m_maxAGI);
    }

    if (item->WIS != 0)
    {
      m_maxWIS -= item->WIS;
      emit statChanged (LIST_WIS, m_maxWIS, m_maxWIS);
      manaAdjust = true;
    }

    m_plusHP -= item->HP;

    if (item->MANA != 0)
    {
      m_plusMana -= item->MANA;
      manaAdjust = true;
    }

    if (manaAdjust)
    {
      m_maxMana = calcMaxMana( m_maxINT,
			     m_maxWIS,
			     classVal(),
			     level()
			     )  +  m_plusMana;
    
      emit manaChanged(m_mana, m_maxMana);  /* Needs max mana */

      m_validMana = true;
    }

    if (showeq_params->savePlayerState)
      savePlayerState();
  }
}
#endif // ZBTEMP

void Player::increaseSkill(const uint8_t* data)
{
  const skillIncStruct* skilli = (const skillIncStruct*)data;
  // save the new skill value
  m_playerSkills[skilli->skillId] = skilli->value;

  // notify others of the new value
  emit changeSkill (skilli->skillId, skilli->value);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::manaChange(const uint8_t* data)
{
  const manaDecrementStruct *mana = (const manaDecrementStruct*)data;
  // update the players mana
  m_mana = mana->newMana;

  m_validMana = true;

  // notify others of this change
  emit manaChanged(m_mana, m_maxMana);  // Needs max mana

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateAltExp(const uint8_t* data)
{
  const altExpUpdateStruct* altexp = (const altExpUpdateStruct*)data;

  uint32_t realExp = altexp->altexp * altexp->percent * (15000000 / 33000);
  uint32_t expIncrement;

  if (realExp > m_currentExp)
    expIncrement = realExp - m_currentAltExp;
  else
    expIncrement = 0;

  m_currentAApts = altexp->aapoints;
  m_currentAltExp = realExp;

  emit expAltChangedInt(m_currentAltExp, 0, 15000000);

  emit newAltExp(expIncrement, m_currentAltExp, altexp->altexp,
		 15000000, 15000000/330, m_currentAApts);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateExp(const uint8_t* data)
{
  const expUpdateStruct* exp = (const expUpdateStruct*)data;

  // if this is just setting the percentage, then do nothing (use info from
  //   player packet).
  if (exp->type == 0) 
  {
    // signal the setting of experience
    emit setExp(m_currentExp, exp->exp, m_minExp, m_maxExp, m_tickExp);

    // nothing more to do.
    return;
  }

  uint32_t realExp = (m_tickExp * exp->exp) + m_minExp;
  uint32_t expIncrement;
  
  // if realExperience is greater then current expereince, calculate the 
  // increment, otherwise this was a < 1/330'th kill and/or the calculated
  // real experience is in that funky rounding place that EQ has...
  if (realExp > m_currentExp)
    expIncrement = realExp - m_currentExp;
  else 
    expIncrement = 0;
  
  m_currentExp = realExp;
  m_validExp = true;

  // signal the new experience
  emit newExp(expIncrement, realExp, exp->exp, 
	      m_minExp, m_maxExp, m_tickExp);
  
  emit expChangedInt (realExp, m_minExp, m_maxExp);
    
  if(m_freshKill)
  {
     emit expGained( m_lastSpawnKilledName,
                     m_lastSpawnKilledLevel,
                     expIncrement,
                     m_zoneMgr->longZoneName());
      
     // have gained experience for the kill, it's no longer fresh
     m_freshKill = false;
  }
  else
     emit expGained( "Unknown", // Randomly blessed with xp?
                     0, // don't know what gave it so, level 0
		     expIncrement,
		     m_zoneMgr->longZoneName());

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateLevel(const uint8_t* data)
{
  const levelUpUpdateStruct *levelup = (const levelUpUpdateStruct *)data;

  // cache previous experience for later calculations
  uint32_t prevExp = m_currentExp;

  // save the new level information
  m_level  = levelup->level;

  // calculate the new experience information
  m_minExp = calc_exp(level() - 1, race(), classVal());
  m_maxExp = calc_exp(level(), race(), classVal ());
  m_tickExp = (m_maxExp - m_minExp) / 330;
  m_currentExp = (m_tickExp * levelup->exp) + m_minExp;
  m_validExp = true;

  // calculate the increment in experience between the current experience and
  // the previous experience
  uint32_t expIncrement =  m_currentExp - prevExp;
  
  emit newExp(expIncrement, m_currentExp, levelup->exp, 
	      m_minExp, m_maxExp, m_tickExp);

  if(m_freshKill)
  {
     emit expGained( m_lastSpawnKilledName,
                     m_lastSpawnKilledLevel,
                     expIncrement,
                     m_zoneMgr->longZoneName());
      
     // have gained experience for the kill, it's no longer fresh
     m_freshKill = false;
  }
  else
     emit expGained( "Unknown", // Randomly blessed with xp?
                     0, // don't know what gave it so, level 0
		     expIncrement,
		     m_zoneMgr->longZoneName());

  emit expChangedInt( m_currentExp, m_minExp, m_maxExp);

  // update the con table
  fillConTable();

  if (showeq_params->savePlayerState)
    savePlayerState();

  updateLastChanged();

  // signal that the level changed
  emit levelChanged(m_level);
  emit changeItem(this, tSpawnChangedLevel);
}

void Player::updateNpcHP(const uint8_t* data)
{
  const hpNpcUpdateStruct* hpupdate = (const hpNpcUpdateStruct*)data;

  if (hpupdate->spawnId != id())
    return;

  m_curHP = hpupdate->curHP;
  m_maxHP = hpupdate->maxHP;

  m_validHP = true;

  updateLastChanged();

  emit changeItem(this, tSpawnChangedHP);

  emit hpChanged(m_curHP, m_maxHP);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateSpawnInfo(const uint8_t* data)
{
  const SpawnUpdateStruct *su = (const SpawnUpdateStruct *)data;
  if (su->spawnId != id())
    return;

  if (su->subcommand != 17)
    return;

  m_curHP = su->arg1;

  m_validHP = true;

  updateLastChanged();

  emit changeItem(this, tSpawnChangedHP);

  emit hpChanged(m_curHP, m_maxHP);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateStamina(const uint8_t* data)
{
  const staminaStruct *stam = (const staminaStruct *)data;
  m_food = stam->food;
  m_water = stam->water;
  m_fatigue = stam->fatigue;
  m_validStam = true;

  emit stamChanged( 100 - m_fatigue,
		    100,
		    m_food,
		    127,
		    m_water,
		    127
		    );


  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::setLastKill(const QString& name, uint8_t level)
{
  // note the last spawn this player killed
  m_lastSpawnKilledName = name;
  m_lastSpawnKilledLevel = level;
  m_freshKill = true;
}

void Player::zoneChanged()
{
  reset();
  clear();
}

void Player::zoneBegin(const ServerZoneEntryStruct* zsentry)
{
  Spawn::setName(zsentry->name);
  Spawn::setLastName(zsentry->lastName);
  setDeity(zsentry->deity);
  setLevel(zsentry->level);
  setClassVal(zsentry->class_);
  setRace(zsentry->race);
  setGender(zsentry->gender);
  setGuildID(zsentry->guildId);
  setGuildTag(m_guildMgr->guildIdToName(guildID()));
  emit guildChanged();
  setPos(zsentry->x >> 3, 
         zsentry->y >> 3, 
         zsentry->z >> 3,
	 showeq_params->walkpathrecord,
	 showeq_params->walkpathlength);
  seqDebug("Player::zoneBegin(): Pos (%f/%f/%f)",
	   float(zsentry->x)/8.0, float(zsentry->y)/8.0, float(zsentry->z)/8.0);
  setHeading(zsentry->heading, 0);
  m_validPos = true;

  m_headingDegrees = 360 - ((((int8_t)lrintf(zsentry->heading)) * 360) >> 11);
  emit headingChanged(m_headingDegrees);
  emit posChanged(x(), y(), z(), 
		  deltaX(), deltaY(), deltaZ(), m_headingDegrees);

  setUseDefaults(false);
  
  if (showeq_params->savePlayerState)
    savePlayerState();

  // update the con table
  fillConTable();

  emit changeItem(this, tSpawnChangedALL);
}

void Player::playerUpdateSelf(const uint8_t* data, size_t, uint8_t dir)
{
  const playerSelfPosStruct *pupdate = (const playerSelfPosStruct*)data;

  if ((dir != DIR_Client) && (pupdate->spawnId != id()))
    return;
  else if (dir == DIR_Client)
    setPlayerID(pupdate->spawnId);
  
  int16_t py = int16_t(pupdate->y);
  int16_t px = int16_t(pupdate->x);
  int16_t pz = int16_t(pupdate->z);
  int16_t pdeltaX = int16_t(pupdate->deltaX);
  int16_t pdeltaY = int16_t(pupdate->deltaY);
  int16_t pdeltaZ = int16_t(pupdate->deltaZ);

  setPos(px, py, pz, showeq_params->walkpathrecord, showeq_params->walkpathlength);
  setDeltas(pdeltaX, pdeltaY, pdeltaZ);
  setHeading(pupdate->heading, pupdate->deltaHeading);
  m_validPos = true;
  updateLast();

  m_headingDegrees = 360 - ((pupdate->heading * 360) >> 11);
  emit headingChanged(m_headingDegrees);

  emit posChanged(x(), y(), z(), 
		  deltaX(), deltaY(), deltaZ(), m_headingDegrees);

  updateLastChanged();
  emit changeItem(this, tSpawnChangedPosition);

  emit newSpeed((int)lrint(hypot( hypot( (pupdate->deltaX*80), 
					 (pupdate->deltaY*80)), 
				  (pupdate->deltaZ*80))));

  static uint8_t count = 0;

  // only save player position every 64 updates
  if (showeq_params->savePlayerState)
  {
    count++;
    if (count % 64)
      savePlayerState();
  }
}

void Player::consMessage(const uint8_t* data, size_t, uint8_t dir)
{
  if (dir == DIR_Client)
    return;

  const considerStruct * con = (const considerStruct*)data;

  if (con->playerid == con->targetid) 
    setPlayerID(con->playerid);
}

void Player::tradeSpellBookSlots(const uint8_t* data, size_t, uint8_t dir)
{
  const tradeSpellBookSlotsStruct* tsb = (const tradeSpellBookSlotsStruct*)data;

  seqDebug("tradeSpellBookSlots(dir=%d): Swapping %d (%04x) with %d (%04x)",
	  dir,
	  tsb->slot1, m_spellBookSlots[tsb->slot1],
	  tsb->slot2, m_spellBookSlots[tsb->slot2]);

  if (dir != DIR_Server)
    return;

  uint32_t spell1 = m_spellBookSlots[tsb->slot1];
  m_spellBookSlots[tsb->slot1] = m_spellBookSlots[tsb->slot2];
  m_spellBookSlots[tsb->slot2] = spell1;
}

// setPlayer* only updates player*.  If you want to change and use the defaults you
// must change showeq_params->default* and emit an checkDefaults signal.
void Player::setPlayerID(uint16_t playerID)
{
  if (id() != playerID)
  {
     seqInfo("Your player's id is %i", playerID);
     setID(playerID);
     emit changedID(id());
     updateLastChanged();
     emit changeItem(this, tSpawnChangedALL);
  }
}

bool Player::getStatValue(uint8_t stat,
			    uint32_t& curValue, 
			    uint32_t& maxValue)
{
  bool valid = false;
  curValue = 0;
  maxValue = 0;

  switch (stat)
  {
  case LIST_HP:
    curValue = m_curHP;
    maxValue = m_maxHP;
    valid = m_validHP;
    break;
  case LIST_MANA:
    curValue = m_mana;
    maxValue = m_maxMana;
    valid = m_validMana;
    break;
  case LIST_STAM:
    curValue = 100 - m_fatigue;
    maxValue = 100;
    valid = m_validStam;
    break;
  case LIST_EXP:
    curValue = m_currentExp;
    maxValue = m_maxExp;
    valid = m_validExp;
    break;
  case LIST_FOOD:
    curValue = m_food;
    maxValue = 127;
    valid = m_validStam;
    break;
  case LIST_WATR:
    curValue = m_water;
    maxValue = 127;
    valid = m_validStam;
    break;
  case LIST_STR:
    curValue = m_maxSTR;
    maxValue = m_maxSTR;
    valid = m_validAttributes;
    break;
  case LIST_STA:
    curValue = m_maxSTA;
    maxValue = m_maxSTA;
    valid = m_validAttributes;
    break;
  case LIST_CHA:
    curValue = m_maxCHA;
    maxValue = m_maxCHA;
    valid = m_validAttributes;
    break;
  case LIST_DEX:
    curValue = m_maxDEX;
    maxValue = m_maxDEX;
    valid = m_validAttributes;
    break;
  case LIST_INT:
    curValue = m_maxINT;
    maxValue = m_maxINT;
    valid = m_validAttributes;
    break;
  case LIST_AGI:
    curValue = m_maxAGI;
    maxValue = m_maxAGI;
    valid = m_validAttributes;
    break;
  case LIST_WIS:
    curValue = m_maxWIS;
    maxValue = m_maxWIS;
    valid = m_validAttributes;
    break;
  case LIST_MR:
  case LIST_FR:
  case LIST_CR:
  case LIST_DR:
  case LIST_PR:
    // don't know how to get these values
    break;
  default:
    break;
  };

  return valid;
}


const QColor& Player::conColorBase(ColorLevel level)
{
  static const QColor invalidColor;

  // only retrieve valid color levels
  if (level < tMaxColorLevels)
    return m_conColorBases[level];
  else
    return invalidColor;
}

void Player::setConColorBase(ColorLevel level, const QColor& color)
{
  // only set valid color levels
  if (level >= tMaxColorLevels)
    return;

  // note the new color base
  m_conColorBases[level] = color;

  // set the color in the preferences
  pSEQPrefs->setPrefColor(conColorBasePrefNames[level], "Player", color);

  // refill the con table
  fillConTable();
}

void Player::fillConTable()
{
// keep around for historical giggles
//
// to make changes here, simply alter greenRange and cyanRange
//
// Levels	Green	Cyan    Red
// 1-7		-4      n/a	+3
// 8-?          -5      -4      +3
// 11-?         -6      -5      +3
// 13-22	-7      -5	+3
// 23-24	-10?    -7?  	+3
// 25-34	-13     -10	+3
// 35-40	-14?	-10?	+3
// 41-42	-15?	-11?	+3
// 43-44	-16?	-12?	+3
// 45-48	-17?	-13?	+3
// 49-51	-18	-14?    +3
// 52-54        -19?	-15?    +3
// 55-57	-20?	-16?	+3
// 58-60        -21     -16     +3
// 61           -13	-17	+3

  int greenRange = 0; 
  int cyanRange = 0; 

  if (level() < 8) 
  { // 1 - 7 
    greenRange = -4;
    cyanRange = -8;
  } 
  else if (level() < 13) 
  { // 8 - 12 
    greenRange = -5;
    cyanRange = -4;
  }
  else if (level() < 23) 
  { // 
    greenRange = -8;
    cyanRange = -6;
  }
  else if (level() < 27) 
  { //
    greenRange = -10;
    cyanRange = -8;
  }
  else if (level() < 29)
  { //
    greenRange = -11;
    cyanRange = -8;
  }
  else if (level() < 34) 
  { // 
    greenRange = -12;
    cyanRange = -9;
  }
  else if (level() < 37) 
  { // 
    greenRange = -13;
    cyanRange = -10;
  }
  else if (level() < 41)
  { // 37 - 40
  	greenRange = -14;
	cyanRange = -11;
  }
  else if (level() < 45)
  { // 41 - 44
  	greenRange = -16;
	cyanRange = -12;
  }
  else if (level() < 49) 
  { // 45 - 48
    greenRange = -17;
    cyanRange = -13;
  }
  else if (level() < 53) 
  { // 49 - 52
    greenRange = -18;
    cyanRange = -14;
  }
  else if (level() < 55) 
  { // 52 - 54
    greenRange = -19;
    cyanRange = -15;
  }
  else if (level() < 57) 
  { // 55 - 56
    greenRange = -20;
    cyanRange = -15;
  }
  else if (level() < 71)
  { //57 - 70
    greenRange = -21;
    cyanRange = -16;
  }

  uint8_t spawnLevel = 1; 
  uint8_t scale;

  uint8_t greenBase = m_conColorBases[tGreenSpawn].green();
  uint8_t greenScale = 255 - greenBase;
  for (; spawnLevel <= (greenRange + level()); spawnLevel++)
  { // this loop handles all GREEN cons
    if (spawnLevel <= (greenRange + level() - 5))
      m_conTable[spawnLevel] = m_conColorBases[tGreenSpawn];
    else
    {
      scale = greenScale/(greenRange + level() - spawnLevel + 1);
      m_conTable[spawnLevel] = QColor(m_conColorBases[tGreenSpawn].red(), 
				 (greenBase + scale), 
				 m_conColorBases[tGreenSpawn].blue());
    }
  }

  for (; spawnLevel <= cyanRange + level(); spawnLevel++)
  { // light blue cons, no need to gradient a small range
    m_conTable[spawnLevel] = m_conColorBases[tCyanSpawn];
  }

  uint8_t blueBase = m_conColorBases[tBlueSpawn].blue();
  uint8_t blueScale = 255 - blueBase;
  for (; spawnLevel < level(); spawnLevel++)
  { // blue cons here
    scale = blueScale/(level() - spawnLevel);
    m_conTable[spawnLevel] = QColor(m_conColorBases[tBlueSpawn].red(), 
			       m_conColorBases[tBlueSpawn].green(), 
			       (blueBase + scale));
  }

  m_conTable[spawnLevel++] = m_conColorBases[tEvenSpawn]; // even con
  m_conTable[spawnLevel++] = m_conColorBases[tYellowSpawn];   // yellow con
  m_conTable[spawnLevel++] = QColor(m_conColorBases[tYellowSpawn].red(), 
			       m_conColorBases[tYellowSpawn].green() - 30,
			       m_conColorBases[tYellowSpawn].blue());   // yellow con
  
  uint8_t redBase = m_conColorBases[tRedSpawn].red();
  uint8_t redScale = 255 - redBase;
  for (; spawnLevel < maxSpawnLevel; spawnLevel++)
  { // red cons
    if (spawnLevel > level() + 13) 
      m_conTable[spawnLevel] = m_conColorBases[tRedSpawn];
    else
    {
      scale = redScale/(spawnLevel - level() - 2);
      m_conTable[spawnLevel] = QColor((redBase + scale), 
				 m_conColorBases[tRedSpawn].green(), 
				 m_conColorBases[tRedSpawn].blue());
    }
  }

  // level 0 is unknown, and thus gray
  m_conTable[0] = m_conColorBases[tUnknownSpawn];
}

QString Player::name() const
{
  return (!m_useAutoDetectedSettings || m_useDefaults ?
	m_defaultName : m_name);
}

QString Player::lastName() const
{
  return (!m_useAutoDetectedSettings || m_useDefaults ?
	m_defaultLastName : m_lastName);
}

uint16_t Player::deity() const 
{ 
  return ((!m_useAutoDetectedSettings || m_useDefaults) ? 
	  m_defaultDeity : m_deity); 
}

uint8_t Player::level() const 
{ 
  return (!m_useAutoDetectedSettings || m_useDefaults ? 
	  m_defaultLevel : m_level);
}

uint16_t Player::race() const
{
  return ((!m_useAutoDetectedSettings || m_useDefaults) ? 
	  m_defaultRace : m_race);
}

uint8_t Player::classVal() const
{
  return ((!m_useAutoDetectedSettings || m_useDefaults) ? 
	  m_defaultClass : m_class);
}

void Player::savePlayerState(void)
{
  QFile keyFile(showeq_params->saveRestoreBaseFilename + "Player.dat");
  if (keyFile.open(IO_WriteOnly))
  {
    QDataStream d(&keyFile);

    int i;

    // write the magic string
    d << *magic;

    // write a test value at the top of the file for a validity check
    size_t testVal = sizeof(charProfileStruct);
    d << testVal;
    d << MAX_KNOWN_SKILLS;
    d << MAX_KNOWN_LANGS;

    d << m_zoneMgr->shortZoneName().lower();

    // write out the rest
    d << m_name;
    d << m_lastName;
    d << m_level;
    d << m_class;
    d << m_deity;
    d << m_ID;
    d << m_x;
    d << m_y;
    d << m_z;
    d << m_deltaX;
    d << m_deltaY;
    d << m_deltaZ;
    d << m_heading;
    d << m_headingDegrees;

    for (i = 0; i < MAX_KNOWN_SKILLS; ++i)
      d << m_playerSkills[i];

    for (i = 0; i < MAX_KNOWN_LANGS; ++i)
      d << m_playerLanguages[i];

    d << m_plusMana;
    d << m_plusHP;
    d << m_curHP;

    d << m_maxMana;
    d << m_maxSTR;
    d << m_maxSTA;
    d << m_maxCHA;
    d << m_maxDEX;
    d << m_maxINT;
    d << m_maxAGI;
    d << m_maxWIS;
    d << m_maxHP;

    d << m_food;
    d << m_water;
    d << m_fatigue;

    d << m_currentAltExp;
    d << m_currentAApts;
    d << m_currentExp;
    d << m_maxExp;

    uint8_t flags = 0;
    if (m_validStam)
      flags |= 0x01;
    if (m_validMana)
      flags |= 0x02;
    if (m_validHP)
      flags |= 0x04;
    if (m_validExp)
      flags |= 0x08;
    if (m_validAttributes)
      flags |= 0x10;
    if (m_useDefaults)
      flags |= 0x20;

    d << flags;
  }
}

void Player::restorePlayerState(void)
{
  QString fileName = showeq_params->saveRestoreBaseFilename + "Player.dat";
  QFile keyFile(fileName);
  if (keyFile.open(IO_ReadOnly))
  {
    int i;
    size_t testVal;
    QDataStream d(&keyFile);

    // check the magic string
    uint32_t magicTest;
    d >> magicTest;

    if (magicTest != *magic)
    {
      seqWarn("Failure loading %s: Bad magic string!",
	      (const char*)fileName);
      reset();
      clear();
      return;
    }

    // check the test value at the top of the file
    d >> testVal;
    if (testVal != sizeof(charProfileStruct))
    {
      seqWarn("Failure loading %s: Bad player size!", 
	      (const char*)fileName);
      reset();
      clear();
      return;
    }

    d >> testVal;
    if (testVal != MAX_KNOWN_SKILLS)
    {
      seqWarn("Failure loading %s: Bad known skills!", 
	      (const char*)fileName);
      reset();
      clear();
      return;
    }

    d >> testVal;
    if (testVal != MAX_KNOWN_LANGS)
    {
      seqWarn("Failure loading %s: Bad known langs!", 
	      (const char*)fileName);
      reset();
      clear();
      return;
    }
    
    // attempt to validate that the info is from the current zone
    QString zoneShortName;
    d >> zoneShortName;
    if (zoneShortName != m_zoneMgr->shortZoneName().lower())
    {
      seqWarn("\aWARNING: Restoring player state for potentially incorrect zone (%s != %s)!",
	      (const char*)zoneShortName, 
	      (const char*)m_zoneMgr->shortZoneName().lower());
    }

    // read in the rest
    d >> m_name;
    d >> m_lastName;
    d >> m_level;
    d >> m_class;
    d >> m_deity;
    d >> m_ID;
    d >> m_x;
    d >> m_y;
    d >> m_z;
    d >> m_deltaX;
    d >> m_deltaY;
    d >> m_deltaZ;
    d >> m_heading;
    d >> m_headingDegrees;

    for (i = 0; i < MAX_KNOWN_SKILLS; ++i)
      d >> m_playerSkills[i];

    for (i = 0; i < MAX_KNOWN_LANGS; ++i)
      d >> m_playerLanguages[i];

    d >> m_plusMana;
    d >> m_plusHP;
    d >> m_curHP;

    d >> m_maxMana;
    d >> m_maxSTR;
    d >> m_maxSTA;
    d >> m_maxCHA;
    d >> m_maxDEX;
    d >> m_maxINT;
    d >> m_maxAGI;
    d >> m_maxWIS;
    d >> m_maxHP;

    d >> m_food;
    d >> m_water;
    d >> m_fatigue;

    d >> m_currentAltExp;
    d >> m_currentAApts;
    d >> m_currentExp;
    d >> m_maxExp;

    uint8_t flags;
    d >> flags;

    if (flags & 0x01)
      m_validStam = true;
    if (flags & 0x02)
      m_validMana = true;
    if (flags & 0x04)
      m_validHP = true;
    if (flags & 0x08)
      m_validExp = true;
    if (flags & 0x10)
      m_validAttributes = true;
    if (flags & 0x20)
      m_useDefaults = true;
    else 
      m_useDefaults = false;

    // now fill out the con table
    fillConTable();

    seqInfo("Restored PLAYER: %s (%s)!",
	    (const char*)m_name,
	    (const char*)m_lastName);
  }
  else
  {
    seqWarn("Failure loading %s: Unable to open!", 
	    (const char*)fileName);
    reset();
    clear();
  }
}

#include "player.moc"
