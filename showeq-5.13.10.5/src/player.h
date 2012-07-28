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
#include "spawn.h"

//----------------------------------------------------------------------
// forward declarations
class GuildMgr;
class ZoneMgr;

//----------------------------------------------------------------------
// constants
const int maxSpawnLevel = 100;

enum ColorLevel
{
  tGraySpawn = 0,
  tGreenSpawn = 1,
  tCyanSpawn = 2,
  tBlueSpawn = 3,
  tEvenSpawn = 4,
  tYellowSpawn = 5,
  tRedSpawn = 6,
  tUnknownSpawn = 7,
  tMaxColorLevels = 8
};

//----------------------------------------------------------------------
// Player
class Player : public QObject, public Spawn
{
  Q_OBJECT 
public:
  Player (QObject* parent,
	  ZoneMgr* zoneMgr,
	  GuildMgr* guildMgr,
	  const char* name = "player");
  virtual ~Player();

 public slots:
   void clear();
   void reset();
   void setUseAutoDetectedSettings(bool enable);
   void setDefaultName(const QString&);
   void setDefaultLastname(const QString&);
   void setDefaultLevel(uint8_t);
   void setDefaultRace(uint16_t);
   void setDefaultClass(uint8_t);
   void setDefaultDeity(uint16_t);

   void player(const charProfileStruct* player); 
   void loadProfile(const playerProfileStruct& player);
   void increaseSkill(const uint8_t* skilli);
   void manaChange(const uint8_t* mana);
   void updateExp(const uint8_t* exp);
   void updateAltExp(const uint8_t* altexp);
   void updateLevel(const uint8_t* levelup);
   void updateNpcHP(const uint8_t* hpupdate);
   void updateSpawnInfo(const uint8_t* su);
   void updateStamina(const uint8_t* stam);
   void setLastKill(const QString& name, uint8_t level);
   void zoneChanged(void);
   void playerUpdateSelf(const uint8_t* pupdate, size_t, uint8_t);
   void consMessage(const uint8_t* con, size_t, uint8_t dir);
   void tradeSpellBookSlots(const uint8_t*, size_t, uint8_t);

   void setPlayerID(uint16_t playerID);
   void savePlayerState(void);
   void restorePlayerState(void);
   void setUseDefaults(bool bdefaults) { m_useDefaults = bdefaults; }

 public:
   virtual QString name() const;
   virtual QString lastName() const;
   virtual uint8_t level() const;
   virtual uint16_t deity() const;
   virtual uint16_t race() const;
   virtual uint8_t classVal() const;

   bool useAutoDetectedSettings() const { return m_useAutoDetectedSettings; }
   QString defaultName() const { return m_defaultName; }
   QString defaultLastName() const { return m_defaultLastName; }
   uint8_t defaultLevel() const { return m_defaultLevel; }
   uint16_t defaultDeity() const { return m_defaultDeity; }
   uint16_t defaultRace() const { return m_defaultRace; }
   uint8_t defaultClass() const { return m_defaultClass; }
   QString realName() const { return m_realName; }
   void setRealName(const QString& name) { m_realName = name; }

   virtual void killSpawn();

   // ZBTEMP: compatibility code
   uint16_t getPlayerID() const { return id(); }
   int16_t headingDegrees() const { return m_headingDegrees; }
   bool validPos() const { return m_validPos; }

   uint32_t getSkill(uint8_t skillId) { return m_playerSkills[skillId]; }
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
   uint32_t getSpellBookSlot(uint32_t slotid) { return m_spellBookSlots[slotid]; }

   uint32_t getCurrentExp() { return m_currentExp; }
   uint32_t getMaxExp() { return m_maxExp; }
   
   const QColor& conColorBase(ColorLevel level);
   void setConColorBase(ColorLevel level, const QColor& color);
   const QColor& pickConColor(int otherSpawnLevel) const;


   bool getStatValue(uint8_t stat,
		     uint32_t& curValue, 
		     uint32_t& maxValue);

 signals:
   void newPlayer(void);
   void buffLoad(const spellBuff*); 
   void newSpeed               (double speed);
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

   void setExp(uint32_t totalExp, uint32_t totalTick,
	       uint32_t minExpLevel, uint32_t maxExpLevel, 
	       uint32_t tickExpLevel);

   void newExp(uint32_t newExp, uint32_t totalExp, uint32_t totalTick,
	       uint32_t minExpLevel, uint32_t maxExpLevel, 
	       uint32_t tickExpLevel);
   void setAltExp(uint32_t totalExp,
		  uint32_t maxExp, uint32_t tickExp, uint32_t aapoints);
   void newAltExp(uint32_t newExp, uint32_t totalExp, uint32_t totalTick, 
		  uint32_t maxExp, uint32_t tickExp, uint32_t aapoints);
   void expAltChangedInt       (int, int, int);
   void expChangedInt          (int, int, int);
                               
   void expGained              ( const QString &,
                                 int,
                                 long,
                                 QString
                               );
                               
   void manaChanged            ( uint32_t,
                                 uint32_t
                               );
                               
   void stamChanged            ( int, int, int, int);
  void hpChanged(int16_t, int16_t);
  void changedID(uint16_t playerID);
  void posChanged(int16_t x, int16_t y, int16_t z,
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ,
		  int32_t heading);
  void changeItem(const Item* item, uint32_t changeType);
  void headingChanged(int32_t heading);
  void levelChanged(uint8_t level);
  void guildChanged();

 protected:
  void fillConTable();
  
 private:
  ZoneMgr* m_zoneMgr;
  GuildMgr* m_guildMgr;

  // The default values are set either by info showeq_params.
  // We keep a second copy in case the player levels while playing.
  QString m_defaultName;
  QString m_defaultLastName;
  QString m_realName;
  uint16_t m_mana;
  uint16_t m_defaultRace;
  uint16_t m_defaultDeity;
  uint8_t m_defaultClass;
  uint8_t m_defaultLevel;
  uint32_t m_playerSkills[MAX_KNOWN_SKILLS];
  uint8_t m_playerLanguages[MAX_KNOWN_LANGS];
  
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
  uint32_t m_minExp;
  uint32_t m_maxExp;
  uint32_t m_tickExp;
  
  uint32_t m_spellBookSlots[MAX_SPELLBOOK_SLOTS];
  
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
  // Wether or not we use defaults, determined by wether or not we could 
  // decode the zone loading data.  
  bool m_useDefaults;
  
  // Whether or not to use auto-detected character settings
  bool m_useAutoDetectedSettings;
  
  // which things are valid
  bool m_validStam;
  bool m_validMana;
  bool m_validHP;
  bool m_validExp;
  bool m_validAttributes;
  bool m_validPos;
};

inline
const QColor& Player::pickConColor(int otherSpawnLevel) const
{
  return m_conTable[otherSpawnLevel];
}

#endif	// EQPLAYER_H
