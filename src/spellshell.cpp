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
#include "player.h"
#include "spawnshell.h"
#include "spells.h"

// #define DIAG_SPELLSHELL 1 

SpellItem::SpellItem()
{
   m_spellId = 0;
   m_casterId = 0;
   m_targetId = 0;
   m_duration = 0;
}

void SpellItem::updateCastTime()
{
  struct timezone tz;
  gettimeofday(&m_castTime,&tz);
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

SpellShell::SpellShell(Player* player, SpawnShell* spawnshell, Spells* spells)
: QObject(NULL, "spellshell"),
  m_player(player), 
  m_spawnShell(spawnshell),
  m_spells(spells)
{
   m_timer = new QTimer(this);
   m_spellList.clear();
   m_deleteList.clear();
   connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
}

SpellItem* SpellShell::FindSpell(int spell_id, int caster_id, int target_id)
{
  bool target = true;
  const Spell* spell = m_spells->spell(spell_id);
  if (spell)
    target = (spell->targetType() != 6);

  for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
      it != m_spellList.end(); it++) {
    SpellItem *i = *it;
    //loop to trap a spell being added that is already cast on self.
    if ((i->spellId() == spell_id) && (i->casterId() == i->targetId()))
      if (caster_id == target_id)
	return i;
	
    if ((i->spellId() == spell_id) && (i->targetId() == target_id)) 
    {
      if ( (target) && (target_id) ) 
      {
	if (i->targetId() == target_id)
	  return i;
      }
      else return i;
    }
  }
  return NULL;
}

SpellItem* SpellShell::FindSpell(int spell_id, int target_id)
{
  bool target = true;
  const Spell* spell = m_spells->spell(spell_id);
  if (spell)
    target = (spell->targetType() != 6);

  for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
      it != m_spellList.end(); it++) {
    SpellItem *i = *it;

    //loop to trap a spell being added that is already cast on self.
    if (i->spellId() == spell_id)
    {
      // if it's a targeted spell, check target, else just return the item
      if (target) 
      {
	// if the target id is non-zero, then check it, otherwise return it
	if (i->targetId())
	{
	  // if target id matches then return it
	  if (i->targetId() == target_id)
	    return i;
	}
	else
	  return i; // no target id, return item
      }
      else 
	return i; // non-targeted spell, return item
    }
  }
  return NULL;
}

SpellItem* SpellShell::FindSpell(int spell_id)
{
  for(QValueList<SpellItem*>::Iterator it = m_spellList.begin(); it != m_spellList.end(); it++)
  {      
    if ((*it) != NULL)
    {
      if ((*it)->spellId() == spell_id)
	return (*it);
    }
  }
  return NULL;
}

void SpellShell::UpdateSpell(SpellItem* item, const startCastStruct *c)
{
   if (c && item) 
   {
     item->setSpellId(c->spellId);
     item->setTargetName(QString("N/A"));
     item->setCasterId(m_player->id());
     item->setCasterName(QString("N/A"));

     const Spell* spell = m_spells->spell(c->spellId);
     
     bool target = true;
     if (spell)
     {
       item->setSpellName(spell->name());
       item->setDuration(spell->calcDuration(m_player->level()) * 6);
       target = (spell->targetType() != 0x06);
     }
     
     if (target)
       item->setTargetId(c->targetId);

     const Item* s;
     if (target && item->targetId() && 
	 ((s = m_spawnShell->findID(tSpawn, item->targetId()))))
       item->setTargetName(s->name());
     
     item->updateCastTime();
   }
}

//Overloaded function for spellBuff
void SpellShell::UpdateSpell(SpellItem* item, const spellBuff *c)
{
   if (c && item) 
   {
     item->setSpellId(c->spellid);
     item->setTargetName(m_player->name());
     item->setTargetId(m_player->id());
     item->setCasterName(QString("Buff"));
     item->setDuration(c->duration * 6);
     
     const Spell* spell = m_spells->spell(c->spellid);

     if (spell)
       item->setSpellName(spell->name());
     
     item->updateCastTime();
   }
}

void SpellShell::UpdateSpell(SpellItem* item, const actionStruct *a)
{
   if (a && item) 
   {
     item->setSpellId(a->spell);
     item->setTargetName(QString("N/A"));
     item->setCasterId(a->source);
     item->setCasterName(QString("N/A"));

     const Spell* spell = m_spells->spell(a->spell);
     
     bool target = true;
     if (spell)
     {
       item->setSpellName(spell->name());
       item->setDuration(spell->calcDuration(a->level) * 6);
       target = (spell->targetType() != 0x06);
     }

     const Item* s;

     if (item->casterId() && 
	 ((s = m_spawnShell->findID(tSpawn, item->casterId()))))
       item->setCasterName(s->name());

     if (target)
       item->setTargetId(a->target);
     
     if (target && item->targetId() && 
	 ((s = m_spawnShell->findID(tSpawn, item->targetId()))))
       item->setTargetName(s->name());
     
     item->updateCastTime();
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
   if (c) 
   {
     SpellItem *item = FindSpell(c->spellId, m_player->id(),
				 c->targetId);
     if (item) 
     { // exists
       UpdateSpell(item, c);
       emit changeSpell(item);
       return item;
     } 
     else 
     { // new spell
       item = new SpellItem();
       UpdateSpell(item, c);
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
//Overloaded InsertSpell for buffLoad
SpellItem* SpellShell::InsertSpell(const spellBuff *c)
{
   if (c) 
   {
      SpellItem *item = FindSpell(c->spellid);
      if (item) 
      { // exists
	UpdateSpell(item, c);
	emit changeSpell(item);
	return item;
      } 
      else 
      { // new spell
	item = new SpellItem();
	UpdateSpell(item, c);
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
#ifdef DIAG_SPELLSHELL
   printf("selfStartSpellCast - id=%d on spawnid=%d\n", 
	  c->spellId, c->targetId);
#endif // DIAG_SPELLSHELL

   InsertSpell(c);
}

//slot for loading buffs when main char struct is loaded
void SpellShell::buffLoad(const spellBuff* c)
{
#ifdef DIAG_SPELLSHELL
   printf("Loading buff - id=%d.\n",c->spellid);
#endif // DIAG_SPELLSHELL

   InsertSpell(c);
}

void SpellShell::buffDrop(const buffDropStruct* bd, uint32_t, uint8_t dir)
{
  // we only care about the server
  if (dir == DIR_CLIENT)
    return;

  // if this is the second server packet then ignore it
  if (bd->spellid == 0xffffffff)
    return;

#ifdef DIAG_SPELLSHELL
  printf("Dropping buff - id=%d from spawn=%d\n", bd->spellid, bd->spawnid);
#endif // DIAG_SPELLSHELL

  // find the spell item
  SpellItem* item = FindSpell(bd->spellid, bd->spawnid);

  // if the spell item was found, then delete it
  if (item)
    DeleteSpell(item);
}

void SpellShell::action(const actionStruct* a, uint32_t, uint8_t)
{
  if (a->type != 0xe7) // only things to do if action is a spell
    return;

  SpellItem* item;
  
  // find a spell with a matching spellid and target - used for updating
  // buffs that we had cast previously that are now be updated by someone
  // else.
  item = FindSpell(a->spell, a->target);
  if (item)
  {
#ifdef DIAG_SPELLSHELL
    printf("action - source=%d (lvl: %d) cast id=%d on target=%d causing %d damage\n", 
	   a->source, a->level, a->spell, a->target, a->damage);
#endif // DIAG_SPELLSHELL

    UpdateSpell(item, a);
    emit changeSpell(item);
    return;
  }

  // otherwise check for spells cast on us
  if (a->target == m_player->id())
  {
#ifdef DIAG_SPELLSHELL
    printf("action - source=%d (lvl: %d) cast id=%d on target=%d causing %d damage\n", 
	   a->source, a->level, a->spell, a->target, a->damage);
#endif // DIAG_SPELLSHELL

    // only way to get here is if there wasn't an existing spell, so...
    item = new SpellItem();
    UpdateSpell(item, a);
    m_spellList.append(item);
    if ((m_spellList.count() > 0) && (!m_timer->isActive()))
      m_timer->start(1000 *
		     pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
    emit addSpell(item);

  }    
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
   printf("selfFinishSpellCast - id=%d, by=%d\n", b->spellId, b->slotId);
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

