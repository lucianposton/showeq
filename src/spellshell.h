/*
 * spawnshell.h
 *
 * ShowEQ Distributed under GPL
 * http://sourceforge.net/projects/seq/
 */

/*
 * Orig Author - Crazy Joe Divola
 * Date - 9/7/00
 */

#ifndef SPELLSHELL_H
#define SPELLSHELL_H

#include <qtimer.h>
#include <qvaluelist.h>
#include <qlistview.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "everquest.h"

#include "player.h"
#include "spawnshell.h"

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
      SpellItem(SpawnShell* spawnShell, 
		uint16_t casterId,
		const startCastStruct *c = NULL);
      //SpellItem(struct beginCastStruct *b = NULL);

      void UpdateSpell(SpawnShell* spawnShell, 
		       uint16_t casterId,
		       const startCastStruct *c = NULL);
      //void UpdateSpell(struct beginCastStruct *b = NULL);
      
      int spellId() const;
      int targetId() const;
      int casterId() const;
      time_t castTime() const;
      QString castTimeStr() const;
      int duration() const;
      QString durationStr() const;
      void setDuration(int);
      const QString spellName() const;
      const QString targetName() const;
      const QString casterName() const;
      //struct castStruct* cast();

   private:
      int m_spellId;
      int m_casterId; //CJD TODO - track others casting spells?
      int m_targetId;
      int m_duration;
      bool m_target;
      timeval m_castTime;
      
      QString m_spellName;
      QString m_casterName;
      QString m_targetName;

      struct startCastStruct m_cast; // Needed?
};

class SpellShell : public QObject
{
   Q_OBJECT
   public:
      SpellShell(EQPlayer* player, SpawnShell* spawnshell);
      SpellItem* InsertSpell(const startCastStruct *c);
      void UpdateSpell(const startCastStruct *);
      //void UpdateSpell(struct beginCastStruct *);
      void DeleteSpell(SpellItem *);
      void clear();
      SpellItem* FindSpell(int spell_id, int caster_id, int target_id=0);

   signals:
      void addSpell(SpellItem *); // done
      void delSpell(SpellItem *); // done
      void changeSpell(SpellItem *); // done
      //void selectSpell(SpellItem *);
      void clearSpells(); // done

   public slots:
      // slots received from EQPacket...
      void selfStartSpellCast(const startCastStruct *);
      //void spellStartCast(struct beginCastStruct *);
      void interruptSpellCast(const badCastStruct *);
      void selfFinishSpellCast(const memSpellStruct *);
      void spellMessage(QString&);
      void timeout();

   private:
      EQPlayer* m_player;
      SpawnShell* m_spawnShell;
      QValueList<SpellItem *> m_spellList, m_deleteList;
      QTimer *m_timer;
};

#endif
