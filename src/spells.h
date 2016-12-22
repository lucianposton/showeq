/*
 * spells.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 * 
 */

#ifndef SPELLS_H_
#define SPELLS_H_

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif
#include <qstringlist.h>
#include <qstring.h>

const size_t playerClasses = 15;

class Spell
{
 public:
  enum TargetType
  {
      // See eqemu common/spdat.h for more
      // http://wiki.shardsofdalaya.com/index.php/Spells/Parsing/code:Main.java
      // http://wiki.shardsofdalaya.com/index.php/Spells/Parsing/code:Tables.java
      GROUP_TELEPORT = 3,
      POINT_BLANK_AE = 4,
      SINGLE = 5,
      SELF = 6,
      TARGET_AE = 8,
      SINGLE_ANIMAL = 9,
      SINGLE_UNDEAD = 10,
      SINGLE_SUMMONED = 11,
      SINGLE_LIFETAP = 13,
      PET = 14,
      SINGLE_CORSE = 15,
      SINGLE_PLANT = 16,
      SINGLE_GIANT = 17,
      SINGLE_DRAGON = 18,
      GROUP = 41,
  };

  Spell(const QString& spells_enLine);
  ~Spell();

  // accessors
  uint16_t spell() const { return m_spell; }
  const QString& name() const { return m_name; }
  uint8_t level(uint8_t class_) const;
  uint8_t targetType() const { return m_targetType; }

  QString spellField(uint8_t field) const;

  int16_t calcDuration(uint8_t level) const;

 private:
  QString m_name;
  uint16_t m_spell;
  int16_t m_buffDurationFormula;
  int16_t m_buffDurationArgument;
  uint8_t m_targetType;
  uint8_t m_classLevels[playerClasses];
};

class Spells
{
 public:
  Spells(const QString& spellsFile);
  ~Spells();
  void loadSpells(const QString& spellsFileName);
  void unloadSpells(void);
    
  const Spell* spell(uint16_t spell) const;
  uint16_t maxSpell() const { return m_maxSpell; }

 private:
  uint16_t m_maxSpell;
  Spell** m_spells;
};

#endif // SPELLS_H_

