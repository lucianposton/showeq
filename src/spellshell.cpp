/*
 * spellshell.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Crazy Joe Divola (cjd1@users.sourceforge.net)
 * September 5, 2001
 *
 */

#include "spellshell.h"
#include "util.h"
#include "spawnshell.h"

SpellItem::SpellItem(SpawnShell* spawnShell, 
		     uint16_t casterId,
		     const startCastStruct *c)
{
   m_spellId = 0;
   m_casterId = 0;
   m_targetId = 0;
   m_duration = 0;
   m_target = false;

   UpdateSpell(spawnShell, casterId, c);
}

//SpellItem::SpellItem(struct beginCastStruct *b)
//{
//}

void SpellItem::UpdateSpell(SpawnShell* spawnShell,
			    uint16_t casterId,
			    const startCastStruct *c)
{
   if (c) {
      m_spellId = c->spellId;
      m_targetId = c->targetId;
      m_targetName = QString("N/A");
      m_casterId = casterId;
      m_casterName = QString("N/A");
      m_spellName = spell_name(c->spellId);
      //printf("Update:: %s\n", m_spellName.latin1());

      struct spellInfoStruct *info = spell_info(m_spellId);
      m_duration = info->duration;
      m_target = info->target;

      const Item *s;
      if (m_casterId)
         if ((s = spawnShell->findID(tSpawn, m_casterId)))
            m_casterName = s->name();

      if ( (m_targetId) && (m_target) )
	if ((s = spawnShell->findID(tSpawn, m_targetId)))
            m_targetName = s->name();

      struct timezone tz;
      gettimeofday(&m_castTime,&tz);
   }
}

//SpellItem::UpdateSpell(struct beginCastStruct *b)
//{
//}

int SpellItem::spellId() const
{
   return m_spellId;
}

int SpellItem::targetId() const
{
   if (m_target)
      return m_targetId;
   else
      return 0;
}

int SpellItem::casterId() const
{
   return m_casterId;
}

time_t SpellItem::castTime() const
{
   return m_castTime.tv_sec;
}

QString SpellItem::castTimeStr() const
{
   QString text;
   // using system_spawntime for now...
   if (showeq_params->systime_spawntime)
      text = QString("%1").arg(castTime());
   else {
      /* Friendlier format courtesy of Daisy */
      struct tm *CreationLocalTime = localtime( &(m_castTime.tv_sec) );
      /* tzname should be set by localtime() but this doesn't seem to
         work.  cpphack */
      char buff[256];
      sprintf(buff, "%02d:%02d:%02d", CreationLocalTime->tm_hour,
              CreationLocalTime->tm_min, CreationLocalTime->tm_sec);
      text = QString(buff);
   }
   return text;
}

int SpellItem::duration() const
{
   return m_duration;
}

QString SpellItem::durationStr() const
{
   QString text;
   int d = m_duration;
   if (d < 0)
      d = 0;
   int h = d / 3600;
   d %= 3600;

   text.sprintf("%02d:%02d:%02d", h, d / 60, d % 60);
   return text;
}

void SpellItem::setDuration(int d)
{
   m_duration = d;
}

const QString SpellItem::spellName() const
{
   return m_spellName;
}

const QString SpellItem::targetName() const
{
   return m_targetName;
}

const QString SpellItem::casterName() const
{
   return m_casterName;
}

//struct startCastStruct* SpellItem::cast()
//{
//   return &m_cast;
//}

SpellShell::SpellShell(Player* player, SpawnShell* spawnshell)
  : QObject(NULL, "spellshell"),
  m_player(player), 
  m_spawnShell(spawnshell)
{
   m_timer = new QTimer(this);
   m_spellList.clear();
   m_deleteList.clear();
   connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
}

SpellItem* SpellShell::FindSpell(int spell_id, int caster_id, int target_id)
{
   for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
         it != m_spellList.end(); it++) {
      SpellItem *i = *it;
      if ((i->spellId() == spell_id) && (i->casterId() == caster_id)) {
         struct spellInfoStruct *info;
         info = spell_info(spell_id);
         if ( (info->target) && (target_id) ) {
            if (i->targetId() == target_id)
               return i;
         } else return i;
      }
   }
   return NULL;
}

void SpellShell::UpdateSpell(const startCastStruct *c)
{
   if (c) {
      SpellItem *item = FindSpell(c->spellId, m_player->id(),
         c->targetId);
      item->UpdateSpell(m_spawnShell, m_player->id(), c);
      emit changeSpell(item);
   }
}

void SpellShell::clear()
{
   for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
         it != m_spellList.end(); it++)
      delete (*it);

   m_spellList.clear();
   m_timer->stop();
   emit clearSpells();
}

SpellItem* SpellShell::InsertSpell(const startCastStruct *c)
{
   if (c) {
      SpellItem *item = FindSpell(c->spellId, m_player->id(),
         c->targetId);
      if (item) { // exists
         UpdateSpell(c);
         return item;
      } else { // new spell
         item = new SpellItem(m_spawnShell, m_player->id(), c);
         m_spellList.append(item);
         if ((m_spellList.count() > 0) && (!m_timer->isActive()))
            m_timer->start(1000 *
               pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
         emit addSpell(item);
	 return item;
      }
   }
   return NULL;
}

void SpellShell::DeleteSpell(SpellItem *item)
{
   if (item) {
      m_spellList.remove(item);
      if (m_spellList.count() == 0)
         m_timer->stop();
      emit delSpell(item);
      delete item;
   }
}

// slots
void SpellShell::selfStartSpellCast(const startCastStruct *c)
{
   printf("selfStartSpellCast - id=%d\n", c->spellId);
   InsertSpell(c);
}

//void SpellShell::spellStartCast(struct beginCastStruct *b)
//{
//}

void SpellShell::interruptSpellCast(const badCastStruct *icast)
{
   // Check the last spell in the list, if spawnId and casterId match,
   // reset spell.

   //printf("SpellShell::interruptSpellCast()\n");
   if (icast) 
   {
      // At times this segfaults.  Stepping through this in GDB at a crash,
      // and it never hits this call, so item is null
      // TODO - check if this happens on interrupts by player casting.

      //SpellItem *item = m_spellList.last();
     // if ( (item) && (icast->spawnId == item->casterId()) ) {
     //    printf("Interrupt %d by %d\n", item->spellId(), icast->spawnId);
     //    item->setDuration(-20);
     // }
   }
}

void SpellShell::selfFinishSpellCast(const memSpellStruct *b)
{
  if (b->param2 != 3)
    return;

   printf("selfFinishSpellCast - id=%d, by=%d\n", b->spellId, b->spawnId);
   SpellItem *item = FindSpell(b->spellId, m_player->id(), b->spawnId);
   if (item) {
      struct spellInfoStruct *info = spell_info(b->spellId);
      item->setDuration(info->duration);
   }
}

void SpellShell::spellMessage(QString &str)
{
   QString spell = str.right(str.length() - 7); // drop 'Spell: '
   bool b = false;
   // Your xxx has worn off.
   // Your target resisted the xxx spell.
   // Your spell fizzles.
   printf("*** spellMessage *** %s\n", spell.latin1());
   if (spell.left(25) == QString("Your target resisted the ")) {
      spell = spell.right(spell.length() - 25);
      spell = spell.left(spell.length() - 7);
      printf("RESIST: '%s'\n", spell.latin1());
      b = true;
   } else if (spell.right(20) == QString(" spell has worn off.")) {
      spell = spell.right(spell.length() - 5);
      spell = spell.left(spell.length() - 20);
      printf("WORE OFF: '%s'\n", spell.latin1());
      b = true;
   }

   if (b) {
      // Can't really tell which spell/target, so just delete the last one
      for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
         it != m_spellList.end(); it++) {
         if ((*it)->spellName() == spell) {
            (*it)->setDuration(0);
            break;
         }
      }
   }
}

void SpellShell::timeout()
{
   int count = 0;
   SpellItem *delList[256];

   for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
      it != m_spellList.end(); it++) {
      if (*it) {
         int d = (*it)->duration() -
            pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6);
         // check if target have despawned
         /*
         if ( (d > 0) && (*it)->targetId() && (!m_spawnShell->findID(
               tSpawn, (*it)->targetId())) ) {
            printf("timeout - caster/target died, stopping '%s'.\n", (*it)->spellName().latin1());
            d = d > 0 ? 0 : d;
         }
         */
         if (d > -20) {
            (*it)->setDuration(d);
            emit changeSpell(*it);
         } else {
            printf("SpellItem '%s' finished.\n", (*it)->spellName().latin1());
            delList[count++] = *it;
         }
      }
   }
   while (count)
      DeleteSpell(delList[--count]);
}

