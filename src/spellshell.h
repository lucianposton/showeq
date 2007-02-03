/*
 * spellshell.h
 *
 * ShowEQ Distributed under GPL
 * http://sourceforge.net/projects/seq/
 * 
 * Portions Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

/*
 * Orig Author - Crazy Joe Divola
 * Date - 9/7/00
 */

#ifndef SPELLSHELL_H
#define SPELLSHELL_H

#include <qtimer.h>
#include <q3valuelist.h>
#include <q3listview.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "everquest.h"

class Player;
class SpawnShell;
class Spells;
class Spell;
class Item;

/* 
 * SpellItem
 *
 * SpellItem is class intended to store information about an EverQuest
 * spell.
 *
 */
 
/* classes from everquest.h
 * spellBuff, used in playerProfileStruct
 * castOnStruct, source and target IDs
 * castStruct, when you start to cast a spell
 * beginCastStruct, spell action struct
 * interruptCastStruct, interrupt casting
 */
class SpellItem
{
 public:
  SpellItem();
  
  // get accessors
  uint16_t spellId() const;
  uint16_t targetId() const;
  uint16_t casterId() const;
  time_t castTime() const;
  QString castTimeStr() const;
  int duration() const;
  QString durationStr() const;
  const QString spellName() const;
  const QString targetName() const;
  const QString casterName() const;
  void setDuration(int);
  
  // set accessors
  void setSpellId(uint16_t spellid);
  void setTargetId(uint16_t target);
  void setCasterId(uint16_t caster);
  void setSpellName(const QString& name);
  void setCasterName(const QString& name);
  void setTargetName(const QString& name);
  void updateCastTime();

  void update(uint16_t spellId, const Spell* spell, int duration,
	      uint16_t casterId, const QString& casterName,
	      uint16_t targetId, const QString& targetName);
  
 private:
  QString m_spellName;
  QString m_casterName;
  QString m_targetName;
  int m_duration;
  timeval m_castTime;

  uint16_t m_spellId; 
  uint16_t m_casterId; 
  uint16_t m_targetId;
  
  struct startCastStruct m_cast; // Needed?
};


inline uint16_t SpellItem::spellId() const 
{
  return m_spellId;
}

inline uint16_t SpellItem::targetId() const
{
  return m_targetId;
}

inline uint16_t SpellItem::casterId() const
{
  return m_casterId;
}

inline time_t SpellItem::castTime() const
{
  return m_castTime.tv_sec;
}

inline int SpellItem::duration() const
{
  return m_duration;
}

inline void SpellItem::setDuration(int d)
{
  m_duration = d;
}

inline const QString SpellItem::spellName() const
{
  return m_spellName;
}

inline const QString SpellItem::targetName() const
{
  return m_targetName;
}

inline const QString SpellItem::casterName() const
{
  return m_casterName;
}

inline void SpellItem::setSpellId(uint16_t spellid)
{
  m_spellId = spellid;
}

inline void SpellItem::setTargetId(uint16_t target)
{
  m_targetId = target;
}

inline void SpellItem::setCasterId(uint16_t caster)
{
  m_casterId = caster;
}

inline void SpellItem::setSpellName(const QString& name)
{
  m_spellName = name;
}

inline void SpellItem::setCasterName(const QString& name)
{
  m_casterName = name;
}

inline void SpellItem::setTargetName(const QString& name)
{ 
  m_targetName = name;
}

class SpellShell : public QObject
{
  Q_OBJECT
 public:
  SpellShell(Player* player, SpawnShell* spawnshell, Spells* spells);
  virtual ~SpellShell();
  void deleteSpell(const SpellItem*);
  
 signals:
  void addSpell(const SpellItem *); // done
  void delSpell(const SpellItem *); // done
  void changeSpell(const SpellItem *); // done
  void clearSpells(); // done
  
 public slots:
  void clear();

  // slots received from EQPacket...
  void selfStartSpellCast(const uint8_t*);
  void buffLoad(const spellBuff*);
  void buff(const uint8_t*, size_t, uint8_t);
  void action(const uint8_t*, size_t, uint8_t);
  void simpleMessage(const uint8_t* cmsg, size_t, uint8_t);
  void spellMessage(QString&);
  void zoneChanged(void);
  void killSpawn(const Item* deceased);
  void timeout();

 protected:
  void deleteSpell(SpellItem *);
  SpellItem* findSpell(uint16_t spellId, 
		       uint16_t targetId, const QString& targetName);
  SpellItem* findSpell(int spell_id);
  SpellItem* FindSpell(int spell_id, int target_id);
  
 private:
  Player* m_player;
  SpawnShell* m_spawnShell;
  Spells* m_spells;
  Q3ValueList<SpellItem *> m_spellList;
  SpellItem* m_lastPlayerSpell;
  QTimer *m_timer;
};

#endif
