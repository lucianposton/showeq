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
#include "util.h"
#include "diagnosticmessages.h"

#include <stdio.h>
#include <math.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <q3ptrqueue.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>

static inline int16_t min(const int16_t& __a,  const int16_t& __b)
{
  if (__b < __a) return __b; return __a;
}

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
  QStringList spellInfo = QStringList::split("^", spells_enLine, true);

  // I'll add support for the rest of the fields later
  m_spell = spellInfo[0].toUShort();
  m_name = spellInfo[1];
  m_buffDurationFormula = spellInfo[16].toUShort();
  m_buffDurationArgument = spellInfo[17].toUShort();
  m_targetType = uint8_t(spellInfo[98].toUShort());

  for (size_t i = 0; i < playerClasses; i++)
    m_classLevels[i] = uint8_t(spellInfo[104 + i].toUShort());
#if 0 // ZBTEMP
  seqDebug("Spell: %d  Fields: %d", m_spell, 
	   spellInfo.count());
#endif 
}

Spell::~Spell()
{
}

int16_t Spell::calcDuration(uint8_t level) const
{
  using namespace std;
  switch(m_buffDurationFormula)
  {
  case 0:
    return 0;
  case 1:
  case 6:
    return min(lroundf(float(level) / 2), m_buffDurationArgument);
  case 3:
  case 4:
  case 11:
  case 12:
    return m_buffDurationArgument;
  case 2:
    return min(lroundf(float(level) * 0.6), m_buffDurationArgument);
  case 5:
    return 3;
  case 7:
    return min(level, m_buffDurationArgument);
  case 8:
    return min(level + 10, m_buffDurationArgument);
  case 9:
    return min(level * 2 + 10, m_buffDurationArgument);
  case 10:
    return min(level * 3 + 10, m_buffDurationArgument);
  case 50:
    return 65535; // as close to permanent as I can get
  case 51:
    return m_buffDurationArgument; // auras? What to do?
  case 3600:
    return 3600;
  default:
    seqInfo("Spell::calcDuration(): Unknown formula for spell %.04x (%d)",
	    m_spell, m_buffDurationArgument);
    return m_buffDurationArgument;
  }
}

uint8_t Spell::level(uint8_t class_) const
{
  if ((class_ > 0) && (class_ <= PLAYER_CLASSES))
    return m_classLevels[class_ - 1];
  else
    return 255;
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
  if (spellsFile.open(QIODevice::ReadOnly))
  {
    // QPtrQueue to temporarily store our Spells until we know the maxSpell
    Q3PtrQueue<Spell> spellQueue;
    spellQueue.setAutoDelete(false);
    
    // allocate memory in a QCString to hold the entire file contents
    Q3CString textData(spellsFile.size() + 1);

    // read the file as one big chunk
    spellsFile.readBlock(textData.data(), textData.size());

    // construct a regex to deal with either style line termination
    QRegExp lineTerm("[\r\n]{1,2}");

    uint16_t unicodeIndicator = *(uint16_t*)textData.data();
    QString text;

    if ((unicodeIndicator != 0xfffe) && (unicodeIndicator != 0xfeff))
      text = textData;
    else
      text = QString::fromUcs2((uint16_t*)textData.data());

    // split the file into at the line termination
    QStringList lines = QStringList::split(lineTerm,
					   text, false);

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

    seqInfo("Loaded %d spells from '%s' maxSpell=%#.04x",
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
      
      // insert into table. Make sure we don't clobber and lose memory
      if (m_spells[newSpell->spell()] != NULL)
      {
        delete m_spells[newSpell->spell()];
      }
      m_spells[newSpell->spell()] = newSpell;
    }
  }
  else
    seqWarn("Spells: Failed to open: '%s'",
	    spellsFileName.latin1());
}

void Spells::unloadSpells(void)
{
  // if a spell list has been allocated, delete it's elements and then itself
  if (m_spells)
  {
    for (int i = 0; i <= m_maxSpell; i++)
    {
      if (m_spells[i] != NULL)
      {
        delete m_spells[i];
      }
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
