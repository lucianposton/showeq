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
#include "zonemgr.h"
#include "main.h"
#include "spawn.h"

//----------------------------------------------------------------------
// constants
const int maxSpawnLevel = 100;

enum ColorLevel
{
  tGreenSpawn = 0,
  tCyanSpawn = 1,
  tBlueSpawn = 2,
  tEvenSpawn = 3,
  tYellowSpawn = 4,
  tRedSpawn = 5,
  tUnknownSpawn = 6,
  tMaxColorLevels = 7
};

//----------------------------------------------------------------------
// Player
class Player : public QObject, public Spawn
{
  Q_OBJECT 
public:
  Player (QObject* parent,
	  ZoneMgr* zoneMgr,
	  const char* name = "player");
  virtual ~Player();

 public slots:
   void backfill(const charProfileStruct* player); 
   void clear();
   void reset();
   void wearItem(const playerItemStruct* itemp);
   void removeItem(const itemItemStruct* item);
   void increaseSkill(const skillIncStruct* skilli);
   void manaChange(const manaDecrementStruct* mana);
   void updateExp(const expUpdateStruct* exp);
   void updateAltExp(const altExpUpdateStruct* altexp);
   void updateLevel(const levelUpUpdateStruct* levelup);
   void updateSpawnHP(const hpUpdateStruct* hpupdate);
   void updateStamina(const staminaStruct* stam);
   void setLastKill(const QString& name, uint8_t level);
   void zoneEntry(const ServerZoneEntryStruct* zsentry);
   void playerUpdate(const playerPosStruct* pupdate, uint32_t, uint8_t);
   void consMessage(const considerStruct * con, uint32_t, uint8_t dir);

   void setPlayerID(uint16_t playerID);
   void checkDefaults(void) { setDefaults(); } // Update our default values
   void setUseDefaults (bool bdefaults) { m_useDefaults = bdefaults; }
   void savePlayerState(void);
   void restorePlayerState(void);

 public:
   virtual QString name() const;
   virtual QString lastName() const;
   virtual uint8_t level() const;
   virtual uint16_t deity() const;
   virtual uint16_t race() const;
   virtual uint8_t classVal() const;
   
   // ZBTEMP: compatibility code
   uint16_t getPlayerID() const { return id(); }
   int16_t headingDegrees() const { return m_headingDegrees; }
   
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
   uint16_t getMana() { return m_mana; }

   uint32_t getCurrentExp() { return m_currentExp; }
   uint32_t getMaxExp() { return m_maxExp; }
   
   const QColor& conColorBase(ColorLevel level);
   void setConColorBase(ColorLevel level, const QColor& color);
   const QColor& pickConColor(int otherSpawnLevel) const;


   bool getStatValue(uint8_t stat,
		     uint32_t& curValue, 
		     uint32_t& maxValue);
   void setDefaults(void);

 signals:
   void newSpeed               (int speed);
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
  void changedID(uint16_t playerID);
  void posChanged(int16_t x, int16_t y, int16_t z,
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ,
		  int32_t heading);
  void changeItem(const Item* item, uint32_t changeType);
  void headingChanged(int32_t heading);
  void levelChanged(uint8_t level);
  
  void getPlayerGuildTag(void);

 protected:
  void fillConTable();
  
 private:
  ZoneMgr* m_zoneMgr;
  
   // The default values are set either by info showeq_params.
   // We keep a second copy in case the player levels while playing.
   QString m_defaultName;
   QString m_defaultLastName;
   uint16_t m_mana;
   uint16_t m_defaultRace;
   uint16_t m_defaultDeity;
   uint8_t m_defaultClass;
   uint8_t m_defaultLevel;
   uint8_t m_playerSkills [MAX_KNOWN_SKILLS];
   uint8_t m_playerLanguages [MAX_KNOWN_LANGS];

   uint16_t m_plusMana;
   uint16_t m_plusHP;

   uint16_t m_maxMana;
   uint8_t m_maxSTR;
   uint8_t m_maxSTA;
   uint8_t m_maxCHA;
   uint8_t m_maxDEX;
   uint8_t m_maxINT;
   uint8_t m_maxAGI;
   uint8_t m_maxWIS;

   uint16_t m_food;
   uint16_t m_water;
   uint16_t m_fatigue;

   // ExperienceWindow needs this
   uint32_t m_currentAltExp;
   uint16_t m_currentAApts;
   uint32_t m_currentExp;
   uint32_t m_maxExp;

   // con color bases
   QColor m_conColorBases[tMaxColorLevels];

   // con color table
   QColor m_conTable[maxSpawnLevel];

   // last spawn this player killed
   QString m_lastSpawnKilledName;
   uint8_t m_lastSpawnKilledLevel;

   // is the kill information fresh
   bool m_freshKill;

   // last spell cast on this player
   uint16_t m_lastSpellOnId;
   
   int16_t m_headingDegrees;
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
const QColor& Player::pickConColor(int otherSpawnLevel) const
{
  return m_conTable[otherSpawnLevel];
}

#endif	// EQPLAYER_H
