/*
 * player.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <stdio.h>

#include "player.h"
#include "util.h"

//#define DEBUG_PLAYER


// how many kills by others without receiving experience until 
// the last kill is no longer considered valid
const int staleKillCutoff = 5;

EQPlayer::EQPlayer (int level = 0, 
		    int race = 1, 
		    int class_ = 1, 
		    int deity = DEITY_AGNOSTIC)
: QObject (NULL, "player")
{
#ifdef DEBUG_PLAYER
    debug("EQPlayer()");
#endif
    m_playerLevel = level;
    m_playerRace  = race;
    m_playerClass = class_;
    m_playerDeity = deity;
    
    setDefaults();
    setUseDefaults(true);
    reset();
    clear();
}

void EQPlayer::backfill(const charProfileStruct* player)
{
  QString messag;

  printf("EQPlayer::backfill():\n");

  
  messag.sprintf("Zone: Name='%s' Last='%s'\n", 
 		 player->name, player->lastName);
  emit msgReceived(messag);
  
  messag.sprintf("Zone: Level: %d\n", player->level);
  emit msgReceived(messag);
  
  messag.sprintf("Zone: PlayerMoney: P=%d G=%d S=%d C=%d\n",
		 player->platinum, player->gold, 
		 player->silver, player->copper);
  emit msgReceived(messag);
  
  messag.sprintf("Zone: BankMoney: P=%d G=%d S=%d C=%d\n",
		 player->platinumBank, player->goldBank, 
		 player->silverBank, player->copperBank);
  emit msgReceived(messag);


// Stats hanling
  memcpy(&m_thePlayer, player, sizeof(charProfileStruct));
  
  m_playerLevel = player->level;
  m_playerRace = player->race;
  m_playerClass = player->class_;
  
  setUseDefaults(false);
  setPlayerName(player->name);
  setPlayerLastName(player->lastName);
  setPlayerLevel(player->level);
  setPlayerRace(player->race);
  setPlayerClass(player->class_);
  setPlayerDeity(player->deity);
  
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
  
  m_maxMana = calcMaxMana( m_maxINT, m_maxWIS,
                           m_playerClass, m_playerLevel
			 ) + m_plusMana;
  
  emit manaChanged(m_thePlayer.MANA, m_maxMana);  // need max mana

  
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

  m_maxExp = calc_exp(m_playerLevel, m_playerRace, m_playerClass);
  minexp  = calc_exp(m_playerLevel-1, m_playerRace, m_playerClass);

  if(m_currentExp < player->exp);
  {
     m_currentExp = player->exp;
     m_currentAltExp = player->altexp;
     m_currentAApts = player->aapoints;

     emit expChangedStr (messag);
     emit expChangedInt (m_currentExp, minexp, m_maxExp);

     emit expAltChangedStr (messag);
     emit expAltChangedInt(m_currentAltExp, 0, 15050000);

     printf("charpro_backfill: exp_debug: Setting m_current values %d %d %d\n", m_currentExp, m_currentAltExp, m_currentAApts);
  }

  messag = "Exp: " + Commanate(player->exp);
  emit msgReceived(messag);

  messag = "ExpAA: " + Commanate(player->altexp);
  emit msgReceived(messag);

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
  m_currentHP = 0;
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

  m_heading = -1;
  m_playerID = 10;
}

void EQPlayer::reset()
{
  memset(&m_thePlayer, 0, sizeof(charProfileStruct));

  setUseDefaults(true);

  m_currentAltExp = 0;
  m_currentExp = 0;
  m_maxExp = calc_exp(getPlayerLevel(), getPlayerRace(), getPlayerClass());

  for (int a = 0; a < MAX_KNOWN_SKILLS; a++)
    m_playerSkills[a] = 255; // indicate an invalid value

  for (int a = 0; a < MAX_KNOWN_LANGS; a++)
    m_playerLanguages[a] = 255; // indicate an invalid value
  
  setPlayerLevel (1);
  setPlayerRace (1);
  setPlayerClass (1);
  
  emit deleteSkills();
  emit deleteLanguages();

  m_validExp = false;
  m_validAttributes = false;

  // update the con table
  fillConTable();
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
      
      emit manaChanged(m_thePlayer.MANA, m_maxMana);  /* Needs max mana */

      m_validMana = true;
    }
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
    
      emit manaChanged(m_thePlayer.MANA, m_maxMana);  /* Needs max mana */

      m_validMana = true;
    }
  }
}

void EQPlayer::increaseSkill(const skillIncStruct* skilli)
{
  // save the new skill value
  m_playerSkills[skilli->skillId] = skilli->value;
  m_thePlayer.skills[skilli->skillId] = skilli->value;

  // notify others of the new value
  emit changeSkill (skilli->skillId, skilli->value);

  QString tempStr;
  tempStr.sprintf("Skill: %s has increased (%d)",
		  (const char*)skill_name(skilli->skillId),
		  skilli->value);

  emit msgReceived(tempStr);
  emit stsMessage(tempStr);
}

void EQPlayer::manaChange(const manaDecrementStruct *mana)
{
  // update the players mana
  m_thePlayer.MANA = mana->newMana;

  m_validMana = true;

  // notify others of this change
  emit manaChanged(m_thePlayer.MANA, m_maxMana);  // Needs max mana
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

  realexp = altexp->altexp * altexp->percent * (15050000 / 33000);
  aapoints = altexp->aapoints;

  if (m_currentAApts != aapoints)
  {
      m_currentAApts = aapoints;
      m_currentAltExp = realexp;
  }
  if (m_currentAltExp > realexp)
      realexp = m_currentAltExp;

  totalAltExp = Commanate(realexp);
  leftAltExp = Commanate(15050000 - realexp);
  incrementAltExp = Commanate(15050000/330);

  emit expAltChangedInt(realexp, 0, 15050000);

  tempStr = QString("ExpAA: %1 (%2/330)").arg(totalAltExp).arg(tempStr2.sprintf("%u",altexp->altexp));
  emit expAltChangedStr(tempStr);
  tempStr = QString("ExpAA: %1 (%2/330) left %3 - 1/330 = %4").arg(totalAltExp).arg(tempStr2.sprintf("%u",altexp->altexp)).arg(leftAltExp).arg(incrementAltExp);
  emit msgReceived(tempStr);

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

printf("exp_debug: fract %d, min %d, max %d, diff %d, real %d, inc %d\n", fractexp, minexp, maxexp, diffexp, realexp, diffexp/330);

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
                     m_longZoneName);
      
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
                     m_longZoneName);
  }
  else if (m_currentExp == 0) 
  {

  }
  else
     emit expGained( "Unknown", // Randomly blessed with xp?
                     0, // don't know what gave it so, level 0
		     realexp - m_currentExp,
		     m_longZoneName
		   );
  
  m_currentExp = realexp;
  m_validExp = true;
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
  m_playerLevel  = levelup->level;

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
}

void EQPlayer::updateSpawnHP(const hpUpdateStruct *hpupdate)
{
  if (hpupdate->spawnId != m_playerID)
    return;

  m_currentHP = m_plusHP + hpupdate->curHp;
  m_maxHP = m_plusHP + hpupdate->maxHp;

  m_validHP = true;

  emit hpChanged(m_currentHP, m_maxHP);
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

}

void EQPlayer::setLastKill(const QString& name, uint8_t level)
{
  // note the last spawn this player killed
  m_lastSpawnKilledName = name;
  m_lastSpawnKilledLevel = level;
  m_freshKill = true;
}

void EQPlayer::setLastSpell(uint16_t spellId)
{
  m_lastSpellOnId = spellId;
}

void EQPlayer::zoneEntry(const ClientZoneEntryStruct* zsentry)
{
  if (m_playerName != zsentry->name)
     reset();
}

void EQPlayer::zoneEntry(const ServerZoneEntryStruct* zsentry)
{
  m_shortZoneName = zsentry->zoneShortName;
  clear();
  setPlayerName(zsentry->name);
  setPlayerDeity(zsentry->deity);
  setPlayerLevel(zsentry->level);
  setPlayerClass(zsentry->class_);
  setPlayerRace(zsentry->race);
}

void EQPlayer::zoneChange(const zoneChangeStruct* zoneChange, bool client)
{
  m_shortZoneName = zoneChange->zoneName;
}

void EQPlayer::zoneNew(const newZoneStruct* zoneNew, bool client)
{
  m_shortZoneName = zoneNew->shortName;
  m_longZoneName = zoneNew->longName;
}


void EQPlayer::playerUpdate(const playerPosStruct *pupdate, bool client)
{
  if (!client && (pupdate->spawnId != m_playerID))
    return;
  else if (client)
    setPlayerID(pupdate->spawnId);
 
  m_xPos = pupdate->xPos;
  m_yPos = pupdate->yPos;
  m_zPos = pupdate->zPos;
  m_deltaX = pupdate->deltaX;
  m_deltaY = pupdate->deltaY;
  m_deltaZ = pupdate->deltaZ;
  m_heading = 360 - (pupdate->heading * 360) / 256;
  
  emit headingChanged(m_heading);
  emit posChanged(m_xPos, m_yPos, m_zPos, 
		  m_deltaX, m_deltaY, m_deltaZ, m_heading);
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

void EQPlayer::setPlayerLevel(uint8_t newlevel)
{
  m_playerLevel = newlevel;

  // update the con table
  fillConTable();

}

void EQPlayer::setPlayerRace(uint8_t newrace)
{
    m_playerRace = newrace;
}

void EQPlayer::setPlayerClass(uint8_t newclass)
{
    m_playerClass = newclass;
}

void EQPlayer::setPlayerDeity(uint16_t newdeity)
{
    //printf("New Deity: %i\n", newdeity);
    m_playerDeity = newdeity;
}

void EQPlayer::setPlayerID(uint16_t playerID)
{
  if (m_playerID != playerID)
  {
     printf("Your player's id is %i\n", playerID);
     m_playerID = playerID;
     emit setID(m_playerID);
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
    curValue = m_currentHP;
    maxValue = m_maxHP;
    valid = m_validHP;
    break;
  case LIST_MANA:
    curValue = m_thePlayer.MANA;
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

void EQPlayer::fillConTable()
{
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

// *NEW* <Malakin 2001-11-15>
// Levels	Green		Cyan    Red
// 1-12			-6       -4			  +3
// 13-22			-7       -5			  +3
// 23-24			-10?    -7?  	     +3
// 25-34			-13     -10			  +3
// 35-40			-14?   -10?		  +3
// 41-42			-15?   -11?		  +3
// 43-44			-16?   -12?		  +3
// 45-48			-17?   -13?		  +3
// 49-51			-18		 -14?     +3
// 52-54       -19?   -15?    +3
// 55-57			-20?	 -16?	  +3
// 58-60       -21     -16     +3

  for (uint8_t level = 1; level < maxSpawnLevel; level++)
  {	
    // all levels have red as +3, yellow as +1 and +2
    // so lets get the those and even con out of the way first
    if  ( m_playerLevel == level )
      m_conTable[ level ] = QColor( 255, 255, 255 );
    else if ( m_playerLevel == ( level - 1 ) )
      m_conTable[ level ] = QColor( 255, 255, 0 );
    else if ( m_playerLevel == ( level - 2 ) )
      m_conTable[ level ] = QColor( 255, 190, 0 );
    else if ( m_playerLevel <= ( level - 3 ) )
    {
      // do the red shading
      if ( m_playerLevel <= ( level - 13 ) )
	m_conTable[ level ] = QColor( 95, 0, 0 );
      else
      {
	int scale = ( level - m_playerLevel ) - 3;
	int redval = 255 - scale * 16;
	m_conTable[ level ] = QColor( redval, 0, 0 );
      }
    }
    else 
    {
      // 1 - 12
      int greenRange = -6;
      int cyanRange = -4;
      
      if (m_playerLevel < 23) 
      { // 13 - 22
	greenRange = -7;
	cyanRange = -5;
      }
      else if (m_playerLevel < 25) 
      { // 23 - 24
	greenRange = -10;
	cyanRange = -7;
      }
      else if (m_playerLevel < 35)
      { // 25 - 34
	greenRange = -13;
	cyanRange = -10;
      }
      else if (m_playerLevel < 41) 
      { // 35 - 40
	greenRange = -14;
	cyanRange = -10;
      }
      else if (m_playerLevel < 43) 
      { // 41 - 42
	greenRange = -15;
	cyanRange = -11;
      }
      else if (m_playerLevel < 45) 
      { // 43 - 44
	greenRange = -16;
	cyanRange = -12;
      }
      else if (m_playerLevel < 49) 
      { // 45 - 48
	greenRange = -17;
	cyanRange = -13;
      }
      else if (m_playerLevel < 52) 
      { // 49 - 51
	greenRange = -18;
	cyanRange = -14;
      }
      else if (m_playerLevel < 55) 
      { // 52 - 54
	greenRange = -19;
	cyanRange = -15;
      }
      else if (m_playerLevel < 58) 
      { // 55 - 57
	greenRange = -21;
	cyanRange = -16;
      }
      else
      { // 58 - 60
	greenRange = -20;
	cyanRange = -16;
      }
      
      calcPickConColor(greenRange, cyanRange, level - m_playerLevel,
		       m_conTable[level]);
    }
  }

  // level 0 is unknown, and thus gray
  m_conTable[0] = gray;
}
