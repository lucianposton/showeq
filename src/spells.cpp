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

#include "spells.h"
#include "everquest.h"

#include <stdio.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qptrqueue.h>

// Spell item ^ delmited fields
// 0   - SpellID
// 1   - Name
// 2   - player_1
// 3   - Teleport zone
// 4   - You Cast
// 5   - Other Cast
// 6   - Cast on You
// 7   - Cast on Other
// 8   - Spell Fades
// 9   - Range
// 10  - AOE range
// 11  - Push back
// 12  - Push up
// 13  - Cast time
// 14  - Recovery time
// 15  - Recast time
// 16  - Buff duration formula
// 17  - Buff duration argument
// 18  - Impact duration
// 19  - Mana
// 21  - Base
// 22  - Base
// 23  - Base
// 24  - Base
// 25  - Base
// 26  - Base
// 27  - Base
// 28  - Base
// 29  - Base
// 30  - Base
// 31  - Base
// 32  - Max
// 33  - Max
// 35  - Max
// 36  - Max
// 37  - Max
// 38  - Max
// 39  - Max
// 40  - Max
// 41  - Max
// 42  - Max
// 43  - Max
// 44  - Icon
// 45  - Mem Icon
// 46  - Component
// 47  - Component
// 48  - Component
// 49  - Component
// 50  - Component counts
// 51  - Component counts
// 52  - Component counts
// 53  - Component counts
// 54  - No-expend reagent
// 55  - No-expend reagent
// 56  - No-expend reagent
// 57  - No-expend reagent
// 58  - Formula
// 59  - Formula
// 60  - Formula
// 61  - Formula
// 62  - Formula
// 63  - Formula
// 64  - Formula
// 65  - Formula
// 66  - Formula
// 67  - Formula
// 68  - Formula
// 69  - Formula
// 70  - Light type
// 71  - Good effect
// 72  - Activated
// 73  - Resist Type
// 74  - Effect ID
// 75  - Effect ID
// 76  - Effect ID
// 77  - Effect ID
// 78  - Effect ID
// 79  - Effect ID
// 80  - Effect ID
// 81  - Effect ID
// 82  - Effect ID
// 83  - Effect ID
// 84  - Effect ID
// 85  - Effect ID
// 86  - Target type
// 87  - Base diff
// 88  - Skill
// 89  - Zone type
// 90  - Environmental type
// 91  - Time of day
// 92  - Class level (Warrior)
// 93  - Class level 
// 94  - Class level 
// 95  - Class level 
// 96  - Class level 
// 97  - Class level 
// 98  - Class level 
// 99  - Class level 
// 100 - Class level 
// 101 - Class level 
// 102 - Class level 
// 103 - Class level 
// 104 - Class level 
// 105 - Class level 
// 106 - Class level (Beastlord)
// 107 - Casting animation
// 108 - Target animation
// 109 - Travel type
// 110 - Spell Affect Index
// 111-133 - Unknown (Filler?)
// 134 - Resist diff
// 135 - Unknown
// 136 - Unknown
// 137 - Recourse Link
// 138- - Unknown

Spell::Spell(const QString& spells_enLine)
  : m_spell(0)
{
  // split the ^ delimited spell entry into a QStringList
  QChar sep('^');
  int flags = QString::SectionCaseInsensitiveSeps;

  // I'll add support for the rest of the fields later
  m_spell = spells_enLine.section(sep, 0, 0, flags).toUShort();
  m_name = spells_enLine.section(sep, 1, 1, flags);
  m_buffDurationFormula = spells_enLine.section(sep, 16, 16, flags).toUShort();
  m_buffDurationArgument = spells_enLine.section(sep, 17, 17, flags).toUShort();
  m_targetType = uint8_t(spells_enLine.section(sep, 85, 85, flags).toUShort());

#if 0 // ZBTEMP
  fprintf(stderr, "Spell: %d  Fields: %d\n", m_spell, 
	  QStringList::split(QChar('^'), spells_enLine).count());
#endif 
}

Spell::~Spell()
{
}

int16_t Spell::calcDuration(uint8_t level) const
{
  switch(m_buffDurationFormula)
  {
  case 0:
    return 0;
  case 1:
  case 6:
    return level / 2;
  case 2:
    return level / 2 + 1;
  case 3:
  case 4:
  case 11:
  case 12:
    return m_buffDurationArgument;
  case 5:
    return 3;
  case 7:
    return level * 10;
  case 8: 
    return level * 10 + 10;
  case 9:
    return level * 2 + 10;
  case 10:
    return level * 3 + 10;
  case 3600:
    return 3600;
  case 50:
    return 65535; // as close to permanent as I can get
  default:
    fprintf(stderr, "Spell::calcDuration(): Unknown formula for spell %.04x\n",
	    m_spell);
    return 0;
  }
}

uint8_t Spell::level(uint8_t class_) const
{
  if (class_ < PLAYER_CLASSES)
    return uint8_t(m_spellInfo[92 + class_ - 1].toUShort());
  else
    return 255;
}

QString Spell::spellField(uint8_t field) const
{
  if (field < m_spellInfo.count())
    return m_spellInfo[field];
  
  return "";
}

Spells::Spells(const QString& spellsFileName)
  : m_maxSpell(0), 
    m_spells(NULL)
{
  loadSpells(spellsFileName);
}

Spells::~Spells()
{
  unloadSpells();
}

void Spells::loadSpells(const QString& spellsFileName)
{
  // unload any previously loaded spells
  unloadSpells();

  // create a QFile on the past in spell file
  QFile spellsFile(spellsFileName);

  // open the spell file if possible
  if (spellsFile.open(IO_ReadOnly))
  {
    // QPtrQueue to temporarily store our Spells until we know the maxSpell
    QPtrQueue<Spell> spellQueue;
    spellQueue.setAutoDelete(false);
    
    // allocate memory in a QCString to hold the entire file contents
    QCString textData(spellsFile.size() + 1);

    // read the file as one big chunk
    spellsFile.readBlock(textData.data(), textData.size());

    // split the file into CR LF terminated lines
    QStringList lines = QStringList::split(QString("\r\n"), textData, false);

    Spell* newSpell;

    // iterate over the lines and process the spell entries therein.
    for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
    {
      newSpell = new Spell(*it);
	
      // calculate the maximum spell ID
      if (newSpell->spell() > m_maxSpell)
	m_maxSpell = newSpell->spell();
      
      // enqueue the new spell entry for later insertion
      spellQueue.enqueue(newSpell);
    }

    fprintf(stderr, 
	    "Loaded %d spells from '%s' maxSpell=%.04x\n",
	    spellQueue.count(), spellsFileName.latin1(), m_maxSpell);

    // allocate the spell array 
    // Notes:  Yeah, it is slightly sparse, but as of this writing there are 
    // only 126 empty entries, so allocating this way for fastest access
    m_spells = new Spell*[m_maxSpell + 1];

    memset((void*)m_spells, 0, sizeof(Spell*) * (m_maxSpell+1));

    // iterate over the queue, removing spells from it and inserting them into 
    // the spells table
    while (!spellQueue.isEmpty())
    {
      // remove from queue
      newSpell = spellQueue.dequeue();
      
      // insert into table
      m_spells[newSpell->spell()] = newSpell;
    }
  }
  else
    fprintf(stderr, "Spells::Spells(): Failed to open: '%s'\n",
	    spellsFileName.latin1());
}

void Spells::unloadSpells(void)
{
  // if a spell list has been allocated, delete it's elements and then itself
  if (m_spells)
  {
    for (int i = 0; i <= m_maxSpell; i++)
    {
      delete m_spells[i];
    }
    
    delete [] m_spells;

    m_spells = NULL;
  }
}

const Spell* Spells::spell(uint16_t spell) const
{
  // make sure the spell is within range
  if (spell >= m_maxSpell)
    return NULL;

  // return the appropriate spell
  return m_spells[spell];
}
