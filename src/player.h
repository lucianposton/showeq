/*
 * player.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQPLAYER_H
#define EQPLAYER_H

#include <qobject.h>
#include <qcolor.h>

#include "everquest.h"
#include "main.h"

//----------------------------------------------------------------------
// constants
const int maxSpawnLevel = 100;

//----------------------------------------------------------------------
// EQPlayer
class EQPlayer : public QObject
{
  Q_OBJECT 
public:
  EQPlayer (int level = 0, 
	    int race = 1, 
	    int class_ = 1, 
	    int deity = DEITY_AGNOSTIC);

 public slots:
   void backfill(const playerProfileStruct* player); 
   void clear();
   void reset();
   void wearItem(const itemPlayerStruct* itemp);
   void removeItem(const itemStruct* item);
   void increaseSkill(const skillIncreaseStruct* skilli);
   void manaChange(const manaDecrementStruct* mana);
   void updateExp(const expUpdateStruct* exp);
   void updateAltExp(const expAltUpdateStruct* altexp);
   void updateLevel(const levelUpStruct* levelup);
   void updateSpawnHP(const spawnHpUpdateStruct* hpupdate);
   void updateStamina(const staminaStruct* stam);
   void setLastKill(const QString& name, uint8_t level);
   void setLastSpell(uint16_t spellId);
   void zoneEntry(const ClientZoneEntryStruct* zsentry);
   void zoneEntry(const ServerZoneEntryStruct* zsentry);
   void zoneChange(const zoneChangeStruct* zoneChange, bool client);
   void zoneNew(const newZoneStruct* zoneNew, bool client);
   void playerUpdate(const playerUpdateStruct* pupdate, bool client);

   void setPlayerName(const QString& playerName);
   void setPlayerLastName(const QString& playerLastName);
   void setPlayerLevel(uint8_t newlevel);
   void setPlayerRace(uint8_t newrace);
   void setPlayerClass(uint8_t newclass);
   void setPlayerDeity(uint8_t newdeity);
   void setPlayerID(uint16_t playerID);
   void checkDefaults(void) { setDefaults(); } // Update our default values
   void setUseDefaults (bool bdefaults) { m_useDefaults = bdefaults; }

 public:
   QString getPlayerName(void) const 
     { return (!showeq_params->AutoDetectCharSettings || m_useDefaults ?
	       m_defaultName : m_playerName);}
   QString getPlayerLastName(void) const 
     { return (!showeq_params->AutoDetectCharSettings || m_useDefaults ? 
	       m_defaultLastName : m_playerLastName);}
   uint8_t getPlayerLevel(void) const 
     { return (!showeq_params->AutoDetectCharSettings || m_useDefaults ? 
	       m_defaultLevel : m_playerLevel);}
   uint8_t getPlayerRace(void) const 
     { return (!showeq_params->AutoDetectCharSettings || m_useDefaults ? 
	       m_defaultRace : m_playerRace);}
   uint8_t getPlayerClass(void) const 
     { return (!showeq_params->AutoDetectCharSettings || m_useDefaults ? 
	       m_defaultClass : m_playerClass);}
   uint8_t getPlayerDeity(void) const 
     { return (!showeq_params->AutoDetectCharSettings || m_useDefaults ? 
	       m_defaultDeity : m_playerDeity);}
   uint16_t getPlayerID() const { return m_playerID; }
   uint16_t getPlayerX() const { return m_xPos; }
   uint16_t getPlayerY() const { return m_yPos; }
   uint16_t getPlayerZ() const { return m_zPos; }
   uint16_t getPlayerDeltaX() const { return m_deltaX; }
   uint16_t getPlayerDeltaY() const { return m_deltaY; }
   uint16_t getPlayerDeltaZ() const { return m_deltaZ; }
   int16_t getPlayerHeading() const { return m_heading; } 
   void getPlayerPos(uint16_t& xPos, uint16_t yPos, uint16_t& zPos)
     { xPos = m_xPos; yPos = m_yPos; zPos = m_zPos; }
   
   uint8_t getSkill(uint8_t skillId) { return m_playerSkills[skillId]; }
   uint8_t getLanguage(uint8_t langId) { return m_playerLanguages[langId]; }

   int getPlusHP() { return m_plusHP; }
   int getPlusMana() { return m_plusMana; }

   uint8_t getMaxSTR() { return m_maxSTR; }
   uint8_t getMaxSTA() { return m_maxSTA; }
   uint8_t getMaxCHA() { return m_maxCHA; }
   uint8_t getMaxDEX() { return m_maxDEX; }
   uint8_t getMaxINT() { return m_maxINT; }
   uint8_t getMaxAGI() { return m_maxAGI; }
   uint8_t getMaxWIS() { return m_maxWIS; }
   uint16_t getMaxMana() { return m_maxMana; }
   uint16_t getMana() { return m_thePlayer.MANA; }

   uint32_t getCurrentExp() { return m_currentExp; }
   uint32_t getMaxExp() { return m_maxExp; }
   const QColor& pickConColor(int otherSpawnLevel) const;

   QString getShortZoneName() { return m_shortZoneName; }
   QString getLongZoneName() { return m_longZoneName; }

   bool getStatValue(uint8_t stat,
		     uint32_t& curValue, 
		     uint32_t& maxValue);
   void setDefaults(void);

 signals:
   void msgReceived            (const QString &);
   void stsMessage             ( const QString &,
                                 int              = 0
                               );
   void statChanged            ( int statNum,
                                 int val,
                                 int max
                               );
   void addSkill               ( int,
                                 int
                               );
                               
   void changeSkill            ( int,
                                 int
                               );
   void deleteSkills();
   void addLanguage            ( int,
                                 int
                               );
   void changeLanguage         ( int,
                                 int
                               );
   void deleteLanguages();

   void expAltChangedStr       (const QString &);
   void expAltChangedInt       (int, int, int);
   void expChangedStr          (const QString &);
   void expChangedInt          (int, int, int);
                               
   void expGained              ( const QString &,
                                 int,
                                 long,
                                 QString
                               );
                               
   void manaChanged            ( uint32_t,
                                 uint32_t
                               );
                               
   void stamChanged            ( int,
                                 int,
                                 int,
                                 int,
                                 int,
                                 int
                               );
  void hpChanged(uint16_t, uint16_t);
  void setID(uint16_t playerID);
  void posChanged(int16_t xPos, int16_t yPos, int16_t zPos,
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ,
		  int32_t heading);
  void headingChanged(int32_t heading);

 protected:
  void fillConTable();
  
 private:
   // The default values are set either by info showeq_params.
   // We keep a second copy in case the player levels while playing.
   QString m_defaultName;
   QString m_defaultLastName;
   int m_defaultLevel;
   uint8_t m_defaultRace;
   uint8_t m_defaultClass;
   uint8_t m_defaultDeity;
   // The actual values are set by info from EQPacket.
   QString m_playerName;
   QString m_playerLastName;
   uint8_t m_playerLevel;
   uint8_t m_playerRace;
   uint8_t m_playerClass;
   uint8_t m_playerDeity;

   uint16_t m_playerID;
   int16_t m_xPos;
   int16_t m_yPos;
   int16_t m_zPos;
   uint16_t m_deltaX;
   uint16_t m_deltaY;
   uint16_t m_deltaZ;
   int16_t  m_heading;

   struct playerProfileStruct m_thePlayer; 
   uint8_t m_playerSkills [MAX_KNOWN_SKILLS];
   uint8_t m_playerLanguages [MAX_KNOWN_LANGS];

   uint16_t m_plusMana;
   uint16_t m_plusHP;
   uint16_t m_currentHP;

   uint16_t m_maxMana;
   uint8_t m_maxSTR;
   uint8_t m_maxSTA;
   uint8_t m_maxCHA;
   uint8_t m_maxDEX;
   uint8_t m_maxINT;
   uint8_t m_maxAGI;
   uint8_t m_maxWIS;
   uint16_t m_maxHP;

   uint16_t m_food;
   uint16_t m_water;
   uint16_t m_fatigue;

   // ExperienceWindow needs this
   uint32_t m_currentAltExp;
   uint16_t m_currentAApts;
   uint32_t m_currentExp;
   uint32_t m_maxExp;

   // con color table
   QColor m_conTable[maxSpawnLevel];

   // last spawn this player killed
   QString m_lastSpawnKilledName;
   uint8_t m_lastSpawnKilledLevel;

   // is the kill information fresh
   bool m_freshKill;

   // last spell cast on this player
   uint16_t m_lastSpellOnId;
   
   // what zone this player is in.
   QString m_longZoneName;
   QString m_shortZoneName;

   // Wether or not we use defaults, determined by wether or not we could decode the zone
   // loading data.  Used alongside showeq_params->forceDefaults
   bool m_useDefaults;
   
   // which things are valid
   bool m_validStam;
   bool m_validMana;
   bool m_validHP;
   bool m_validExp;
   bool m_validAttributes;
};

inline
const QColor& EQPlayer::pickConColor(int otherSpawnLevel) const
{
  return m_conTable[otherSpawnLevel];
}

#endif	// EQPLAYER_H
