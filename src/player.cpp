/*
 * player.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <stdio.h>

#include <qfile.h>
#include <qdatastream.h>

#include "player.h"
#include "util.h"

//#define DEBUG_PLAYER


// how many kills by others without receiving experience until 
// the last kill is no longer considered valid
const int staleKillCutoff = 5;

EQPlayer::EQPlayer (QObject* parent,
		    ZoneMgr* zoneMgr,
		    const char* name,
		    int level, 
		    uint16_t race, 
		    int class_, 
		    int deity)
  : QObject (parent, name),
    m_zoneMgr(zoneMgr)
{
#ifdef DEBUG_PLAYER
    debug("EQPlayer()");
#endif
    setLevel(level);
    setRace(race);
    setClassVal(class_);
    setDeity(deity);
    
    setDefaults();
    setUseDefaults(true);
    // restore the player state if the user requested it...
    if (showeq_params->restorePlayerState)
      restorePlayerState();
    else
    {
      reset();
      clear();
    }
}

void EQPlayer::backfill(const charProfileStruct* player)
{
  QString messag;

  printf("EQPlayer::backfill():\n");

  
  messag.sprintf("EQPlayer: Name: '%s' Last: '%s'\n", 
 		 player->name, player->lastName);
  emit msgReceived(messag);
  
  messag.sprintf("EQPlayer: Level: %d\n", player->level);
  emit msgReceived(messag);
  
  messag.sprintf("EQPlayer: PlayerMoney: P=%d G=%d S=%d C=%d\n",
		 player->platinum, player->gold, 
		 player->silver, player->copper);
  emit msgReceived(messag);
  
  messag.sprintf("EQPlayer: BankMoney: P=%d G=%d S=%d C=%d\n",
		 player->platinumBank, player->goldBank, 
		 player->silverBank, player->copperBank);
  emit msgReceived(messag);

  // fill in base Spawn class
  Spawn::backfill(player);

  // Stats hanling
  setUseDefaults(false);
  setPlayerName(player->name);
  setPlayerLastName(player->lastName);
  setLevel(player->level);
  setRace(player->race);
  setClassVal(player->class_);
  setDeity(player->deity);

  // update the con table
  fillConTable();
  
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

  messag = "EQPlayer: Exp =" + Commanate(player->exp);
  emit msgReceived(messag);

  messag = "EQPlayer: ExpAA =" + Commanate(player->altexp);
  emit msgReceived(messag);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void EQPlayer::clear()
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

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void EQPlayer::reset()
{
  setUseDefaults(true);

  m_currentAltExp = 0;
  m_currentExp = 0;
  m_maxExp = calc_exp(getPlayerLevel(), getPlayerRace(), getPlayerClass());

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

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void EQPlayer::wearItem(const playerItemStruct* itemp)
{
  const itemStruct* item = &itemp->item;

  if ((item->equipSlot < 22) && !isItemBook(*item))
  {
    bool manaAdjust = false;

    if (item->common.STR != 0)
    {
      m_maxSTR += item->common.STR;
      emit statChanged (LIST_STR, m_maxSTR, m_maxSTR);
    }
    
    if (item->common.STA != 0)
    {
      m_maxSTA += item->common.STA;
      emit statChanged (LIST_STA, m_maxSTA, m_maxSTA);
    }

    if (item->common.CHA != 0)
    {
      m_maxCHA += item->common.CHA;
      emit statChanged (LIST_CHA, m_maxCHA, m_maxCHA);
    }

    if (item->common.DEX != 0)
    {
      m_maxDEX += item->common.DEX;
      emit statChanged (LIST_DEX, m_maxDEX, m_maxDEX);
    }

    if (item->common.INT != 0)
    {
      m_maxINT += item->common.INT;
      emit statChanged (LIST_INT, m_maxINT, m_maxINT);
      manaAdjust = true;
    }

    if (item->common.AGI != 0)
    {
      m_maxAGI += item->common.AGI;
      emit statChanged (LIST_AGI, m_maxAGI, m_maxAGI);
    }

    if (item->common.WIS != 0)
    {
      m_maxWIS += item->common.WIS;
      emit statChanged (LIST_WIS, m_maxWIS, m_maxWIS);
      manaAdjust = true;
    }

    m_plusHP += item->common.HP;

    if (item->common.MANA != 0)
    {
      m_plusMana += item->common.MANA;
      manaAdjust = true;
    }

    if (manaAdjust)
    {
      m_maxMana = calcMaxMana( m_maxINT,
			       m_maxWIS,
			       getPlayerClass(),
			       getPlayerLevel()
			       )  +  m_plusMana;
      
      emit manaChanged(m_mana, m_maxMana);  /* Needs max mana */

      m_validMana = true;
    }

    if (showeq_params->savePlayerState)
      savePlayerState();
  }
}

void EQPlayer::removeItem(const itemStruct* item)
{
  if ((item->equipSlot < 22) && !isItemBook(*item))
  {
    bool manaAdjust = false;

    if (item->common.STR != 0)
    {
      m_maxSTR -= item->common.STR;
      emit statChanged (LIST_STR, m_maxSTR, m_maxSTR);
    }
    
    if (item->common.STA != 0)
    {
      m_maxSTA -= item->common.STA;
      emit statChanged (LIST_STA, m_maxSTA, m_maxSTA);
    }

    if (item->common.CHA != 0)
    {
      m_maxCHA -= item->common.CHA;
      emit statChanged (LIST_CHA, m_maxCHA, m_maxCHA);
    }

    if (item->common.DEX != 0)
    {
      m_maxDEX -= item->common.DEX;
      emit statChanged (LIST_DEX, m_maxDEX, m_maxDEX);
    }

    if (item->common.INT != 0)
    {
      m_maxINT -= item->common.INT;
      emit statChanged (LIST_INT, m_maxINT, m_maxINT);
      manaAdjust = true;
    }

    if (item->common.AGI != 0)
    {
      m_maxAGI -= item->common.AGI;
      emit statChanged (LIST_AGI, m_maxAGI, m_maxAGI);
    }

    if (item->common.WIS != 0)
    {
      m_maxWIS -= item->common.WIS;
      emit statChanged (LIST_WIS, m_maxWIS, m_maxWIS);
      manaAdjust = true;
    }

    m_plusHP -= item->common.HP;

    if (item->common.MANA != 0)
    {
      m_plusMana -= item->common.MANA;
      manaAdjust = true;
    }

    if (manaAdjust)
    {
      m_maxMana = calcMaxMana( m_maxINT,
			     m_maxWIS,
			     getPlayerClass(),
			     getPlayerLevel()
			     )  +  m_plusMana;
    
      emit manaChanged(m_mana, m_maxMana);  /* Needs max mana */

      m_validMana = true;
    }

    if (showeq_params->savePlayerState)
      savePlayerState();
  }
}

void EQPlayer::increaseSkill(const skillIncStruct* skilli)
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

void EQPlayer::manaChange(const manaDecrementStruct *mana)
{
  // update the players mana
  m_mana = mana->newMana;

  m_validMana = true;

  // notify others of this change
  emit manaChanged(m_mana, m_maxMana);  // Needs max mana

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void EQPlayer::updateAltExp(const altExpUpdateStruct* altexp)
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

void EQPlayer::updateExp(const expUpdateStruct* exp)
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
  minexp = calc_exp(getPlayerLevel() - 1, getPlayerRace(), getPlayerClass());
  maxexp = calc_exp(getPlayerLevel(), getPlayerRace(), getPlayerClass());
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

void EQPlayer::updateLevel(const levelUpUpdateStruct *levelup)
{
  QString totalExp;
  QString gainedExp;
  QString leftExp;
  QString needKills;
  QString tempStr;

  totalExp = Commanate(levelup->exp);
  gainedExp = Commanate((uint32_t) (levelup->exp - m_currentExp));
  
  needKills = Commanate(((calc_exp( levelup->level,
				    getPlayerRace  (),
				    getPlayerClass ()
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

  m_maxExp = calc_exp( getPlayerLevel (),
		     getPlayerRace  (),
		     getPlayerClass ()
		     );
  
  emit expChangedInt( levelup->exp,
		      calc_exp( getPlayerLevel () - 1,
				getPlayerRace  (),
				getPlayerClass ()
				),
		      calc_exp( getPlayerLevel (),
				getPlayerRace  (),
				getPlayerClass ()
				)
		      );
  
  m_currentExp = levelup->exp;

  m_validExp = true;

  // update the con table
  fillConTable();

  if (showeq_params->savePlayerState)
    savePlayerState();

  emit levelChanged(m_level);
}

void EQPlayer::updateSpawnHP(const hpUpdateStruct *hpupdate)
{
  if (hpupdate->spawnId != id())
    return;

  m_curHP = m_plusHP + hpupdate->curHp;
  m_maxHP = m_plusHP + hpupdate->maxHp;

  m_validHP = true;

  emit hpChanged(m_curHP, m_maxHP);

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void EQPlayer::updateStamina(const staminaStruct *stam)
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

void EQPlayer::setLastKill(const QString& name, uint8_t level)
{
  // note the last spawn this player killed
  m_lastSpawnKilledName = name;
  m_lastSpawnKilledLevel = level;
  m_freshKill = true;
}

void EQPlayer::zoneEntry(const ServerZoneEntryStruct* zsentry)
{
  clear();
  setPlayerName(zsentry->name);
  setDeity(zsentry->deity);
  setLevel(zsentry->level);
  setClassVal(zsentry->class_);
  setRace(zsentry->race);

  // update the con table
  fillConTable();

  if (showeq_params->savePlayerState)
    savePlayerState();
}

void EQPlayer::playerUpdate(const playerPosStruct *pupdate, uint32_t, uint8_t dir)
{
  if ((dir != DIR_CLIENT) && (pupdate->spawnId != id()))
    return;
  else if (dir == DIR_CLIENT)
    setPlayerID(pupdate->spawnId);
 
  setPos(pupdate->xPos, pupdate->yPos, pupdate->zPos,
	 showeq_params->walkpathrecord,
	 showeq_params->walkpathlength);
  setDeltas(pupdate->deltaX, pupdate->deltaY, 
	    pupdate->deltaZ);
  setHeading(pupdate->heading, pupdate->deltaHeading);
  updateLast();

  m_headingDegrees = 360 - (pupdate->heading * 360) / 256;

  emit headingChanged(m_headingDegrees);

  emit posChanged(x(), y(), z(), 
		  deltaX(), deltaY(), deltaZ(), m_headingDegrees);

  static uint8_t count = 0;

  // only save player position every 64 updates
  if (showeq_params->savePlayerState)
  {
    count++;
    if (count % 64)
      savePlayerState();
  }
}

void EQPlayer::consMessage(const considerStruct * con, uint32_t, uint8_t dir)
{
  if (dir == DIR_CLIENT)
    return;

  if (con->playerid == con->targetid) 
    setPlayerID(con->playerid);
}

// setPlayer* only updates player*.  If you want to change and use the defaults you
// must change showeq_params->default* and emit an checkDefaults signal.
void EQPlayer::setPlayerName(const QString& newplayername)
{
    m_playerName = newplayername;
}

void EQPlayer::setPlayerLastName(const QString& newplayerlastname)
{
    m_playerLastName = newplayerlastname;
}

void EQPlayer::setPlayerID(uint16_t playerID)
{
  if (id() != playerID)
  {
     printf("Your player's id is %i\n", playerID);
     setID(playerID);
     emit changedID(id());
  }
}

// Set our internal defaults equal to the showeq_params defaults.
void EQPlayer::setDefaults(void)
{
    m_defaultName     = showeq_params->defaultName;
    m_defaultLastName = showeq_params->defaultLastName;
    m_defaultLevel    = showeq_params->defaultLevel;
    m_defaultRace     = showeq_params->defaultRace;
    m_defaultClass    = showeq_params->defaultClass;
    m_defaultDeity    = showeq_params->defaultDeity;
}

bool EQPlayer::getStatValue(uint8_t stat,
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


void EQPlayer::fillConTable()
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

      int greenRange; 
      int cyanRange; 

      if (m_level < 8) 
      { // 1 - 7 
         greenRange = -4;
         cyanRange = -8;
      } 
      else if (m_level < 13) 
      { // 8 - 12 
	greenRange = -5;
	cyanRange = -4;
      }
      else if (m_level < 23) 
      { // 
	greenRange = -7;
	cyanRange = -5;
      }
      else if (m_level < 27) 
      { //
	greenRange = -10;
	cyanRange = -8;
      }
      else if (m_level < 29)
      { //
	greenRange = -11;
	cyanRange = -8;
      }
      else if (m_level < 34) 
      { // 
	greenRange = -12;
	cyanRange = -9;
      }
      else if (m_level < 37) 
      { // 
	greenRange = -13;
	cyanRange = -10;
      }
      else if (m_level < 40) 
      { // 43 - 44
	greenRange = -14;
	cyanRange = -11;
      }
      else if (m_level < 49) 
      { // 45 - 48
	greenRange = -17;
	cyanRange = -13;
      }
      else if (m_level < 53) 
      { // 49 - 52
	greenRange = -18;
	cyanRange = -14;
      }
      else if (m_level < 55) 
      { // 52 - 54
	greenRange = -19;
	cyanRange = -15;
      }
      else if (m_level < 57) 
      { // 55 - 56
	greenRange = -20;
	cyanRange = -15;
      }
      else
      { // 57 - 60
	greenRange = -21;
	cyanRange = -16;
      }
      
  uint8_t level = 1; 
  uint8_t scale;

  for (; level <= (greenRange + m_level); level++)
  { // this loop handles all GREEN cons
      if (level <= (greenRange + m_level - 5))
         m_conTable[level] = QColor(0, 95, 0);
      else
      {
         scale = 160/(greenRange + m_level - level + 1);
         m_conTable[level] = QColor(0, (95 + scale), 0);
      }
  }

  for (; level <= cyanRange + m_level; level++)
  { // light blue cons, no need to gradient a small range
      m_conTable[level] = QColor(0, 255, 255);
  }

  for (; level < m_level; level++)
  { // blue cons here
    scale = 95/(m_level - level);
    m_conTable[level] = QColor(0, 0, (160 + scale));
  }

  m_conTable[level++] = QColor(255, 255, 255); // even con
  m_conTable[level++] = QColor(255, 255, 0);   // yellow con
  m_conTable[level++] = QColor(255, 200, 0);   // yellow con
  
  for (; level < maxSpawnLevel; level++)
  { // red cons
      if (level > m_level + 13) 
         m_conTable[level] = QColor(127, 0, 0);
      else
      {
         scale = 128/(level - m_level - 2);
         m_conTable[level] = QColor((127 + scale), 0, 0);
      }
  }

  // level 0 is unknown, and thus gray
  m_conTable[0] = gray;
}
    
#if 0  // this function no longer in use
void calcPickConColor(int greenRange, int cyanRange, int levelDif, 
		      QColor& color)
{
  // do the greens
  if ( greenRange > levelDif )
  {
    if ( ( greenRange - 10 ) >= levelDif )
      color = QColor( 0, 95, 0 );
    else
    {
      int scale = greenRange - levelDif;
      int greenval = 255 - ( scale * 16 );
      color = QColor( 0, greenval, 0 );
    }
  }
  else if ( cyanRange >= levelDif) // do the cyans
  {
    int middleValue = ( cyanRange + ( ( greenRange - cyanRange)  / 2 ) );
    
    if ( levelDif < middleValue ) // is cyan partially green?
    {
      int greenSteps = middleValue - greenRange - 1;
      int greenValue = 207 + ( 48 / (greenSteps ? greenSteps : 1)) * ( middleValue - levelDif - 1 );
      
      color = QColor ( 0, greenValue, 191);
    }
    else // cyan must be partially blue then.
    {
      int blueSteps = cyanRange - middleValue;
      int blueValue = 207 + ( 48 / (blueSteps ? blueSteps : 1)) * ( levelDif - middleValue );	
      
      color = QColor ( 0, 191, blueValue);
    }	
  }
  else
    // ok, now we know its blue
  {
    if ( ( greenRange / 2 ) < levelDif )
    {
      // white scale
      int scalemax = (greenRange / 2 ) * -1;
      int scale = ( levelDif + 1 ) * -1;
      int colorval = 127 - ( scale * 127 / scalemax );
      color = QColor( colorval, colorval, 255 );
      // 127, 127, 255 --- 0, 0, 255
    }
    else if ( ( greenRange / 2 ) > levelDif )
    {
      // green scale
      int scalemax = ( greenRange / 2 ) * -1;
      int scale = levelDif - ( greenRange + 1 );
      int greenval = 127 - ( scale * 127 / scalemax );
      int blueval = 127 + ( scale * 127 / scalemax );
      color = QColor( 0, greenval, blueval );
      // 0, 127, 127 --- 0, 0, 255
    }
    else
      color = QColor( 0, 0, 255 );
  }
}
#endif

uint16_t EQPlayer::race() const
{
  return ((!showeq_params->AutoDetectCharSettings || m_useDefaults) ? 
	  m_defaultRace : m_race);
}

uint8_t EQPlayer::classVal() const
{
  return ((!showeq_params->AutoDetectCharSettings || m_useDefaults) ? 
	  m_defaultClass : m_class);
}

void EQPlayer::savePlayerState(void)
{
  QFile keyFile(showeq_params->saveRestoreBaseFilename + "Player.dat");
  if (keyFile.open(IO_WriteOnly))
  {
    QDataStream d(&keyFile);

    int i;

    // write a test value at the top of the file for a validity check
    size_t testVal = sizeof(charProfileStruct);
    d << testVal;
    d << MAX_KNOWN_SKILLS;
    d << MAX_KNOWN_LANGS;

    // write out the rest
    d << m_playerName;
    d << m_playerLastName;
    d << m_level;
    d << m_class;
    d << m_deity;
    d << m_ID;
    d << m_xPos;
    d << m_yPos;
    d << m_zPos;
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

void EQPlayer::restorePlayerState(void)
{
  QString fileName = showeq_params->saveRestoreBaseFilename + "Player.dat";
  QFile keyFile(fileName);
  if (keyFile.open(IO_ReadOnly))
  {
    int i;
    size_t testVal;
    QDataStream d(&keyFile);

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

    // read in the rest
    d >> m_playerName;
    d >> m_playerLastName;
    d >> m_level;
    d >> m_class;
    d >> m_deity;
    d >> m_ID;
    d >> m_xPos;
    d >> m_yPos;
    d >> m_zPos;
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
	    (const char*)m_playerName,
	    (const char*)m_playerLastName);
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
