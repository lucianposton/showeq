/*
 * player.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <stdio.h>
#include <unistd.h>

#include <qdir.h>
#include <qfile.h>
#include <qdatastream.h>

#include "player.h"
#include "util.h"

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
		const char* name)
  : QObject(parent, name),
    Spawn(),
    m_zoneMgr(zoneMgr)
{
#ifdef DEBUG_PLAYER
  debug("Player()");
#endif

  connect(m_zoneMgr, SIGNAL(zoneBegin(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
          this, SLOT(zoneBegin(const ServerZoneEntryStruct*)));
  connect(m_zoneMgr, SIGNAL(zoneChanged(const QString&)),
          this, SLOT(zoneChanged()));
  
  m_NPC = SPAWN_SELF;
  
  setDefaults();
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

void Player::backfill(const charProfileStruct* player)
{
  QString messag;

  printf("Player::backfill():\n");

  messag.sprintf("Player: Name: '%s' Last: '%s'\n", 
 		 player->name, player->lastName);
  emit msgReceived(messag);
  
  messag.sprintf("Player: Level: %d\n", player->level);
  emit msgReceived(messag);
  
  messag.sprintf("Player: PlayerMoney: P=%d G=%d S=%d C=%d\n",
		 player->platinum, player->gold, 
		 player->silver, player->copper);
  emit msgReceived(messag);
  
  messag.sprintf("Player: BankMoney: P=%d G=%d S=%d C=%d\n",
		 player->platinumBank, player->goldBank, 
		 player->silverBank, player->copperBank);
  emit msgReceived(messag);

  // fill in base Spawn class
  // set the characteristics that probably haven't changed.
  setNPC(SPAWN_SELF);
  setGender(player->gender);
  setRace(player->race);
  setClassVal(player->class_);
  setLevel(player->level);

  // save the raw name
  setTypeflag(1);

  Spawn::setName(player->name);
  setLastName(player->lastName);

  // if it's got a last name add it
  if (level() < player->level)
    setLevel(player->level);

  // Stats hanling
  setUseDefaults(false);
  setDeity(player->deity);
  setGuildID(player->guildID);

  emit getPlayerGuildTag();

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
    if ((m_playerSkills[a] == 255) || // not valid
	(player->skills[a] > m_playerSkills[a])) // or a higher value
      m_playerSkills[a] = player->skills[a];

    emit addSkill (a, m_playerSkills[a]);
  }

  // Merge in our new languages...
  for (int a = 0; a < MAX_KNOWN_LANGS; a++)
  {
    if ((m_playerLanguages[a] == 255) ||
	(player->languages[a] > m_playerLanguages[a]))
      m_playerLanguages[a] = player->languages[a];
    
    emit addLanguage (a, m_playerLanguages[a]);
  }

  m_validAttributes = true;
  m_validMana = true;
  m_validExp = true;

  // update the con table
  fillConTable();

  // Exp handling
  uint32_t minexp;

  m_maxExp = calc_exp(m_level, m_race, m_class);
  minexp  = calc_exp(m_level-1, m_race, m_class);

  if(m_currentExp < player->exp);
  {
     m_currentExp = player->exp;
     m_currentAltExp = player->altexp;
     m_currentAApts = player->aapoints;

     emit expChangedInt (m_currentExp, minexp, m_maxExp);

     messag = "Exp: " + Commanate(player->exp);
     emit expChangedStr(messag);

     emit expAltChangedInt(m_currentAltExp, 0, 15000000);

     messag = "ExpAA: " + Commanate(player->altexp);
     emit expAltChangedStr(messag);

  }

  if (showeq_params->savePlayerState)
    savePlayerState();

  messag = "Player: Exp =" + Commanate(player->exp);
  emit msgReceived(messag);

  messag = "Player: ExpAA =" + Commanate(player->altexp);
  emit msgReceived(messag);

  updateLastChanged();

  emit changeItem(this, tSpawnChangedALL);

  QDir tmp("/tmp/");
  tmp.rename(QString("bankfile.") + QString::number(getpid()),
	     QString("bankfile.") + player->name);
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
  m_headingDegrees = 360 - (m_heading * 360) / 256;
  
  setID(0);
  setPoint(0,0,0);

  updateLastChanged();
}

void Player::reset()
{
  setUseDefaults(true);

  m_currentAltExp = 0;
  m_currentExp = 0;
  m_maxExp = calc_exp(level(), race(), classVal());

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

void Player::increaseSkill(const skillIncStruct* skilli)
{
  // save the new skill value
  m_playerSkills[skilli->skillId] = skilli->value;

  // notify others of the new value
  emit changeSkill (skilli->skillId, skilli->value);

  QString tempStr;
  tempStr.sprintf("Skill: %s has increased (%d)",
		  (const char*)skill_name(skilli->skillId),
		  skilli->value);

  emit msgReceived(tempStr);
  emit stsMessage(tempStr);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::manaChange(const manaDecrementStruct *mana)
{
  // update the players mana
  m_mana = mana->newMana;

  m_validMana = true;

  // notify others of this change
  emit manaChanged(m_mana, m_maxMana);  // Needs max mana

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateAltExp(const altExpUpdateStruct* altexp)
{
  QString totalAltExp;
  QString leftAltExp;
  QString incrementAltExp;
  QString tempStr;
  QString tempStr2;
  uint32_t realexp;
  uint16_t aapoints;

  realexp = altexp->altexp * altexp->percent * (15000000 / 33000);
  aapoints = altexp->aapoints;

  if (m_currentAApts != aapoints)
  {
      m_currentAApts = aapoints;
      m_currentAltExp = realexp;
  }
  if (m_currentAltExp > realexp)
      realexp = m_currentAltExp;

  totalAltExp = Commanate(realexp);
  leftAltExp = Commanate(15000000 - realexp);
  incrementAltExp = Commanate(15000000/330);

  emit expAltChangedInt(realexp, 0, 15000000);

  tempStr = QString("ExpAA: %1 (%2/330)").arg(totalAltExp).arg(tempStr2.sprintf("%u",altexp->altexp));
  emit expAltChangedStr(tempStr);
  tempStr = QString("ExpAA: %1 (%2/330) left %3 - 1/330 = %4").arg(totalAltExp).arg(tempStr2.sprintf("%u",altexp->altexp)).arg(leftAltExp).arg(incrementAltExp);
  emit msgReceived(tempStr);


  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateExp(const expUpdateStruct* exp)
{
  QString totalExp;
  QString incrementExp;
  QString leftExp;
  QString needKills;
  QString tempStr;
  QString tempStr2;
  uint32_t realexp;
  uint32_t minexp;
  uint32_t maxexp;
  uint32_t diffexp;
  uint16_t fractexp;

  fractexp =  exp->exp;
  minexp = calc_exp(level() - 1, race(), classVal());
  maxexp = calc_exp(level(), race(), classVal());
  diffexp = maxexp - minexp;
  realexp = (diffexp / 330) * fractexp + minexp;
  incrementExp = Commanate(diffexp/330);


  totalExp  = Commanate(realexp - minexp);
  leftExp = Commanate(maxexp - realexp);
  needKills = Commanate(((maxexp - realexp) / (realexp > m_currentExp ? realexp - m_currentExp : 1)) + 1 );

  tempStr = QString("Exp: %1 (%2/330) [%3]").arg(totalExp).arg(tempStr2.sprintf("%u",fractexp)).arg(needKills);
  emit expChangedStr(tempStr);
  emit expChangedInt (realexp, minexp, maxexp);
    
  tempStr = QString("Exp: %1 (%2/330) left %3 - 1/330 = %4").arg(totalExp).arg(tempStr2.sprintf("%u",fractexp)).arg(leftExp).arg(incrementExp);
  emit msgReceived(tempStr);
  emit stsMessage(tempStr);

  if(m_freshKill)
  {
     emit expGained( m_lastSpawnKilledName,
                     m_lastSpawnKilledLevel,
                     realexp - m_currentExp,
                     m_zoneMgr->longZoneName());
      
     // have gained experience for the kill, it's no longer fresh
     m_freshKill = false;
  }
  else if ((m_lastSpellOnId == 0x0184) || // Resuscitate
	   (m_lastSpellOnId == 0x0187) || // Revive (does it or don't it?)
	   (m_lastSpellOnId == 0x0188) || // Resurrection
	   (m_lastSpellOnId == 0x02f4) || // Resurrection Effects
	   (m_lastSpellOnId == 0x02f5) || // Resurrection Effect
	   (m_lastSpellOnId == 0x03e2) || // Customer Service Resurrection
	   (m_lastSpellOnId == 0x05f4)) // Reviviscence
  {
     emit expGained( spell_name(m_lastSpellOnId),
                     0, // level of caster would only confuse things further
                     realexp - m_currentExp,
                     m_zoneMgr->longZoneName());
  }
  else if (m_currentExp != 0) 
     emit expGained( "Unknown", // Randomly blessed with xp?
                     0, // don't know what gave it so, level 0
		     realexp - m_currentExp,
		     m_zoneMgr->longZoneName()
		   );
  
  m_currentExp = realexp;
  m_validExp = true;

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateLevel(const levelUpUpdateStruct *levelup)
{
  QString totalExp;
  QString gainedExp;
  QString leftExp;
  QString needKills;
  QString tempStr;

  totalExp = Commanate(levelup->exp);
  gainedExp = Commanate((uint32_t) (levelup->exp - m_currentExp));
  
  needKills = Commanate(((calc_exp( levelup->level,
				    race  (),
				    classVal ()
				    ) - levelup->exp
			  )          /  ( levelup->exp > m_currentExp    ?
					  levelup->exp - m_currentExp :
					  1
					  )
			 )
			);
  
  tempStr = QString("Exp: %1 (%2) [%3]").arg(totalExp).arg(gainedExp).arg(needKills);
  
  emit expChangedStr (tempStr);

  m_defaultLevel = levelup->level;
  m_level  = levelup->level;

  m_maxExp = calc_exp( level (),
		     race  (),
		     classVal ()
		     );
  
  emit expChangedInt( levelup->exp,
		      calc_exp( level () - 1,
				race  (),
				classVal ()
				),
		      calc_exp( level (),
				race  (),
				classVal ()
				)
		      );
  
  m_currentExp = levelup->exp;

  m_validExp = true;

  // update the con table
  fillConTable();

  if (showeq_params->savePlayerState)
    savePlayerState();

  updateLastChanged();

  emit levelChanged(m_level);
  emit changeItem(this, tSpawnChangedLevel);
}

void Player::updateSpawnMaxHP(const SpawnUpdateStruct *su)
{
  if (su->spawnId != id())
    return;

  m_curHP = su->arg1;

  m_validHP = true;

  updateLastChanged();

  emit changeItem(this, tSpawnChangedHP);

  emit hpChanged(m_curHP, m_maxHP);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void Player::updateStamina(const staminaStruct *stam)
{
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
  setPos((int16_t)lrintf(zsentry->x), 
         (int16_t)lrintf(zsentry->y), 
         (int16_t)lrintf(zsentry->z),
	 showeq_params->walkpathrecord,
	 showeq_params->walkpathlength
        );
  setDeltas(0,0,0);
  setHeading((int8_t)lrintf(zsentry->heading), 0);

  m_headingDegrees = 360 - (((int8_t)lrintf(zsentry->heading)) * 360) / 256;
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

void Player::playerUpdate(const playerPosStruct *pupdate, uint32_t, uint8_t dir)
{
  if ((dir != DIR_CLIENT) && (pupdate->spawnId != id()))
    return;
  else if (dir == DIR_CLIENT)
    setPlayerID(pupdate->spawnId);
  
  int16_t py = pupdate->y / 8;
  int16_t px = pupdate->x / 8;
  int16_t pz = pupdate->z / 8;
  int16_t pdeltaX = pupdate->deltaX / 64;
  int16_t pdeltaY = pupdate->deltaY / 64;
  int16_t pdeltaZ = pupdate->deltaZ / 64;
 
  setPos(px, py, pz, showeq_params->walkpathrecord, showeq_params->walkpathlength);
  setDeltas(pdeltaX, pdeltaY, pdeltaZ);
  setHeading(pupdate->heading, pupdate->deltaHeading);
  updateLast();

//  m_headingDegrees = 360 - (pupdate->heading * 360) / 256;
  m_headingDegrees = 360 - (pupdate->heading * 360) / 2048;
  emit headingChanged(m_headingDegrees);

  emit posChanged(x(), y(), z(), 
		  deltaX(), deltaY(), deltaZ(), m_headingDegrees);

  updateLastChanged();
  emit changeItem(this, tSpawnChangedPosition);

  emit newSpeed((int)lrint(hypot( hypot( pupdate->deltaX, pupdate->deltaY ), pupdate->deltaZ)));

  static uint8_t count = 0;

  // only save player position every 64 updates
  if (showeq_params->savePlayerState)
  {
    count++;
    if (count % 64)
      savePlayerState();
  }
}

void Player::consMessage(const considerStruct * con, uint32_t, uint8_t dir)
{
  if (dir == DIR_CLIENT)
    return;

  if (con->playerid == con->targetid) 
    setPlayerID(con->playerid);
}

// setPlayer* only updates player*.  If you want to change and use the defaults you
// must change showeq_params->default* and emit an checkDefaults signal.
void Player::setPlayerID(uint16_t playerID)
{
  if (id() != playerID)
  {
     printf("Your player's id is %i\n", playerID);
     setID(playerID);
     emit changedID(id());
     updateLastChanged();
     emit changeItem(this, tSpawnChangedALL);
  }
}

// Set our internal defaults equal to the showeq_params defaults.
void Player::setDefaults(void)
{
    m_defaultName     = showeq_params->defaultName;
    m_defaultLastName = showeq_params->defaultLastName;
    m_defaultLevel    = showeq_params->defaultLevel;
    m_defaultRace     = showeq_params->defaultRace;
    m_defaultClass    = showeq_params->defaultClass;
    m_defaultDeity    = showeq_params->defaultDeity;
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
// *OLD* This is the info we have to work with
// Level Range		Green		Red
// 1-12				-4			+3
// 13-22			-6			+3
// 23-24			-7			+3
// 25-34			-8			+3
// 35-40			-10			+3
// 41-42			-11			+3
// 43-44			-12			+3
// 45-48			-13			+3
// 49-51			-14			+3
// 52-54			-15			+3
// 55-57			-16			+3
// 58-60			-17			+3

// *NEW* 
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
    greenRange = -7;
    cyanRange = -5;
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
  else if (level() < 61)
  { // 57 - 60
    greenRange = -21;
    cyanRange = -16;
  }
  else if (level() == 61) //61+
  {
    greenRange = -19;
    cyanRange = -14;
  }
  else if (level() == 62)
  {
    greenRange = -18;
    cyanRange = -13;
  }
  else if (level() == 63)
  {
    greenRange = -17;
    cyanRange = -12;
  }
  else if (level() < 66) //64 - 65
  {
    greenRange = -16;
    cyanRange = -11;
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
  return (!showeq_params->AutoDetectCharSettings || m_useDefaults ?
	m_defaultName : m_name);
}

QString Player::lastName() const
{
  return (!showeq_params->AutoDetectCharSettings || m_useDefaults ?
	m_defaultLastName : m_lastName);
}

uint16_t Player::deity() const 
{ 
  return ((!showeq_params->AutoDetectCharSettings || m_useDefaults) ? 
	  m_defaultDeity : m_deity); 
}

uint8_t Player::level() const 
{ 
  return (!showeq_params->AutoDetectCharSettings || m_useDefaults ? 
	  m_defaultLevel : m_level);
}

uint16_t Player::race() const
{
  return ((!showeq_params->AutoDetectCharSettings || m_useDefaults) ? 
	  m_defaultRace : m_race);
}

uint8_t Player::classVal() const
{
  return ((!showeq_params->AutoDetectCharSettings || m_useDefaults) ? 
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
      fprintf(stderr, 
	      "Failure loading %s: Bad magic string!\n",
	      (const char*)fileName);
      reset();
      clear();
      return;
    }

    // check the test value at the top of the file
    d >> testVal;
    if (testVal != sizeof(charProfileStruct))
    {
      fprintf(stderr, 
	      "Failure loading %s: Bad player size!\n", 
	      (const char*)fileName);
      reset();
      clear();
      return;
    }

    d >> testVal;
    if (testVal != MAX_KNOWN_SKILLS)
    {
      fprintf(stderr, 
	      "Failure loading %s: Bad known skills!\n", 
	      (const char*)fileName);
      reset();
      clear();
      return;
    }

    d >> testVal;
    if (testVal != MAX_KNOWN_LANGS)
    {
      fprintf(stderr, 
	      "Failure loading %s: Bad known langs!\n", 
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
      fprintf(stderr,
	      "\aWARNING: Restoring player state for potentially incorrect zone (%s != %s)!\n",
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

    fprintf(stderr, "Restored PLAYER: %s (%s)!\n",
	    (const char*)m_name,
	    (const char*)m_lastName);
  }
  else
  {
    fprintf(stderr,
	    "Failure loading %s: Unable to open!\n", 
	    (const char*)fileName);
    reset();
    clear();
  }
}
