/*
 * spellshell.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Crazy Joe Divola (cjd1@users.sourceforge.net)
 * September 5, 2001
 * 
 * Portions Copyright 2003-2007 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

#include "spellshell.h"
#include "util.h"
#include "player.h"
#include "spawnshell.h"
#include "spells.h"
#include "packetcommon.h"
#include "spawn.h"
#include "diagnosticmessages.h"
//Added by qt3to4:
#include <Q3ValueList>

//#define DIAG_SPELLSHELL 1 

SpellItem::SpellItem()
  : m_duration(0),
    m_spellId(0),
    m_casterId(0),
    m_targetId(0)
{
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

void SpellItem::update(uint16_t spellId, const Spell* spell, int duration,
		       uint16_t casterId, const QString& casterName,
		       uint16_t targetId, const QString& targetName)
{
     setSpellId(spellId);

     setDuration(duration);

     if (spell)
     {
       setSpellName(spell->name());

       if (spell->targetType() != 0x06)
	 setTargetId(targetId);
     }
     else
     {
       setSpellName(spell_name(spellId));
       setTargetId(targetId);
     }

     setCasterId(casterId);

     if (!casterName.isEmpty())
       setCasterName(casterName);
     else
       setCasterName(QString("N/A"));

     if (!targetName.isEmpty())
       setTargetName(targetName);
     else
       setTargetName(QString("N/A"));

     updateCastTime();
}


SpellShell::SpellShell(Player* player, SpawnShell* spawnshell, Spells* spells)
: QObject(NULL, "spellshell"),
  m_player(player), 
  m_spawnShell(spawnshell),
  m_spells(spells),
  m_lastPlayerSpell(0)
{
   m_timer = new QTimer(this);
   m_spellList.clear();
   connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
}

SpellShell::~SpellShell()
{
  clear();
}

SpellItem* SpellShell::findSpell(uint16_t spellId, 
				 uint16_t targetId, const QString& targetName)
{
  for(Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin();
      it != m_spellList.end(); 
      it++) 
  {
    SpellItem *i = *it;
    if (i->spellId() == spellId)
    {
      if ((i->targetId() == targetId) || 
	  ((i->targetId() == 0) && (i->targetName() == targetName)))
	return i;
    }
  }

  return NULL;
}

SpellItem* SpellShell::findSpell(int spell_id)
{
  for(Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin(); 
      it != m_spellList.end(); 
      it++)
  {      
    SpellItem *si = *it;

    if (si->spellId() == spell_id)
      return si;
  }

  return NULL;
}

void SpellShell::clear()
{
   emit clearSpells();

   m_lastPlayerSpell = 0;
   for(Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin();
         it != m_spellList.end(); it++)
      delete (*it);

   m_spellList.clear();
   m_timer->stop();
}

// this is just the public way of doing this
void SpellShell::deleteSpell(const SpellItem* item)
{
  deleteSpell((SpellItem*)item);
}

void SpellShell::deleteSpell(SpellItem *item)
{
   if (item) 
   {
      if (m_lastPlayerSpell == item)
	m_lastPlayerSpell = 0;
      m_spellList.remove(item);
      if (m_spellList.count() == 0)
         m_timer->stop();
      emit delSpell(item);
      delete item;
   }
}

// slots

void SpellShell::selfStartSpellCast(const uint8_t* data)
{
  const startCastStruct *c = (const startCastStruct *)data;
#ifdef DIAG_SPELLSHELL
  seqDebug("selfStartSpellCast - id=%d (slot=%d, inv=%d) on spawnid=%d", 
	   c->spellId, c->slot, c->inventorySlot, c->targetId);
#endif // DIAG_SPELLSHELL

  // get the target 
  const Item* s;
  QString targetName;
  int duration = 0;
  const Spell* spell = m_spells->spell(c->spellId);
  SpellItem *item;
  if (spell)
    duration = spell->calcDuration(m_player->level()) * 6;

  if (!spell || spell->targetType() != 6)
  {
    if (c->targetId && 
	((s = m_spawnShell->findID(tSpawn, c->targetId))))
      targetName = s->name();
    
    item = findSpell(c->spellId, c->targetId, targetName);
  }
  else
  {
    targetName = m_player->name();
    item = findSpell(c->spellId);
  }

  if (item) 
  { // exists
    item->update(c->spellId, spell, duration,
		 m_player->id(), m_player->name(),
		 c->targetId, targetName);
    emit changeSpell(item);
  } 
  else 
  { // new spell
    item = new SpellItem();
    item->update(c->spellId, spell, duration,
		 m_player->id(), m_player->name(),
		 c->targetId, targetName);
    m_spellList.append(item);
    if ((m_spellList.count() > 0) && (!m_timer->isActive()))
      m_timer->start(1000 *
		     pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
    emit addSpell(item);
    m_lastPlayerSpell = item;
  }
}

//slot for loading buffs when main char struct is loaded
void SpellShell::buffLoad(const spellBuff* c)
{
#ifdef DIAG_SPELLSHELL
  seqDebug("Loading buff - id=%d.",c->spellid);
#endif // DIAG_SPELLSHELL

  const Spell* spell = m_spells->spell(c->spellid);
  int duration = c->duration * 6;
  SpellItem *item = findSpell(c->spellid, m_player->id(), m_player->name());
  if (item) 
  { // exists
    item->update(c->spellid, spell, duration, 
		 0, "Buff", m_player->id(), m_player->name());
    emit changeSpell(item);
  } 
  else 
  { // new spell
    item = new SpellItem();
    item->update(c->spellid, spell, duration, 
		 0, "Buff", m_player->id(), m_player->name());
    m_spellList.append(item);
    if ((m_spellList.count() > 0) && (!m_timer->isActive()))
      m_timer->start(1000 *
		     pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
    emit addSpell(item);
  }
}

void SpellShell::buff(const uint8_t* data, size_t, uint8_t dir)
{
  // we only care about the server
  if (dir == DIR_Client)
    return;

  const buffStruct* b = (const buffStruct*)data;

  // if this is the second server packet then ignore it
  if (b->spellid == 0xffffffff)
    return;

#ifdef DIAG_SPELLSHELL
  seqDebug("Changing buff - id=%d from spawn=%d", b->spellid, b->spawnid);
#endif // DIAG_SPELLSHELL

  const Spell* spell = m_spells->spell(b->spellid);

  // find the spell item
  SpellItem* item;
  const Item* s;
  QString targetName;
  if (!spell || spell->targetType() != 6)
  {
    if (b->spawnid && 
	((s = m_spawnShell->findID(tSpawn, b->spawnid))))
      targetName = s->name();
    
    item = findSpell(b->spellid, b->spawnid, targetName);
  }
  else
    item = findSpell(b->spellid);

  if (!item)
    return;

  if (b->changetype == 0x01) // removing buff
    deleteSpell(item);
  else if (b->changetype == 0x02)
  {
    // right now we only know how to find the updated duration
    item->setDuration(b->duration * 6);
    emit changeSpell(item);
  }
}

void SpellShell::action(const uint8_t* data, size_t, uint8_t)
{
  const actionStruct* a = (const actionStruct*)data;

  if (a->type != 0xe7) // only things to do if action is a spell
    return;

  const Item* s;
  QString targetName;

  if (a->target && 
      ((s = m_spawnShell->findID(tSpawn, a->target))))
    targetName = s->name();

  SpellItem *item = findSpell(a->spell, a->target, targetName);

  if (item || (a->target == m_player->id()))
  {
    int duration = 0;
    const Spell* spell = m_spells->spell(a->spell);
    if (spell)
      duration = spell->calcDuration(a->level) * 6;
    
    QString casterName;
    if (a->source && 
	((s = m_spawnShell->findID(tSpawn, a->source))))
      casterName = s->name();

    if (item)
    {
#ifdef DIAG_SPELLSHELL
      seqDebug("action - found - source=%d (lvl: %d) cast id=%d on target=%d causing %d damage", 
	       a->source, a->level, a->spell, a->target, a->damage);
#endif // DIAG_SPELLSHELL
      
      item->update(a->spell, spell, duration, 
		   a->source, casterName, a->target, targetName);
      emit changeSpell(item);
    }
    else
    {
      // otherwise check for spells cast on us
#ifdef DIAG_SPELLSHELL
      seqDebug("action - new - source=%d (lvl: %d) cast id=%d on target=%d causing %d damage", 
	       a->source, a->level, a->spell, a->target, a->damage);
#endif // DIAG_SPELLSHELL
      
      // only way to get here is if there wasn't an existing spell, so...
      item = new SpellItem();
      item->update(a->spell, spell, duration, 
		   a->source, casterName, a->target, targetName);
      m_spellList.append(item);
      if ((m_spellList.count() > 0) && (!m_timer->isActive()))
	m_timer->start(1000 *
		       pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
      emit addSpell(item);
    }    
  }
}

void SpellShell::simpleMessage(const uint8_t* data, size_t, uint8_t)
{
  // if no spell cast by the player recently, then nothing to do.
  if (!m_lastPlayerSpell)
    return;

  const simpleMessageStruct* smsg = (const simpleMessageStruct*)data;
  switch(smsg->messageFormat)
  {
  case 191: // Your target has no mana to affect
  case 239: // Your target cannot be mesmerized.
  case 240: // Your target cannot be mesmerized (with this spell).
  case 242: // Your target is immune to changes in its attack speed.
  case 243: // Your target is immune to fear spells.
  case 244: // Your target is immune to changes in its run speed.
  case 245: // You are unable to change form here.
  case 248: // Your target is too high of a level for your charm spell.
  case 251: // That spell can not affect this target NPC.
  case 253: // This pet may not be made invisible.
  case 255: // You do not have a pet.
  case 263: // Your spell did not take hold.
  case 264: // Your target has resisted your attempt to mesmerize it.
  case 268: // Your target looks unaffected.
  case 269: // Stick to singing until you learn to play this instrument.
  case 271: // Your spell would not have taken hold on your target.
  case 272: // You are missing some required spell components.
  case 439: // Your spell is interrupted.
  case 3285: // Your target is too powerful to be Castigated in this manner.
  case 9035: // Your target is too high of a level for your fear spell.
  case 9036: // This spell only works in the Planes of Power.
    // delete the last player spell
    deleteSpell(m_lastPlayerSpell);
    m_lastPlayerSpell = 0;
    break;
  default:
    break;
  }
}


void SpellShell::spellMessage(QString &str)
{
   QString spell = str.right(str.length() - 7); // drop 'Spell: '
   bool b = false;
   // Your xxx has worn off.
   // Your target resisted the xxx spell.
   // Your spell fizzles.
   seqInfo("*** spellMessage *** %s", spell.latin1());
   if (spell.left(25) == QString("Your target resisted the ")) {
      spell = spell.right(spell.length() - 25);
      spell = spell.left(spell.length() - 7);
      seqInfo("RESIST: '%s'", spell.latin1());
      b = true;
   } else if (spell.right(20) == QString(" spell has worn off.")) {
      spell = spell.right(spell.length() - 5);
      spell = spell.left(spell.length() - 20);
      seqInfo("WORE OFF: '%s'", spell.latin1());
      b = true;
   }

   if (b) {
      // Can't really tell which spell/target, so just delete the last one
      for(Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin();
         it != m_spellList.end(); it++) {
         if ((*it)->spellName() == spell) {
            (*it)->setDuration(0);
            break;
         }
      }
   }
}

void SpellShell::zoneChanged(void)
{
  m_lastPlayerSpell = 0;
  SpellItem* spell;
  for(Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin();
      it != m_spellList.end(); it++) 
  {
    spell = *it;

    // clear all the invalidated spawn ids
    spell->setTargetId(0);
    spell->setCasterId(0);
  }
}

void SpellShell::killSpawn(const Item* deceased)
{
    uint16_t id = deceased->id();

    if (id == m_player->id())
    {
        // We're dead. No more buffs for us.
        clear();
    }
    else
    {
        SpellItem* spell;

        if (m_lastPlayerSpell && (m_lastPlayerSpell->targetId() == id))
        {
            m_lastPlayerSpell = 0;
        }

        Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin();
        while(it != m_spellList.end())
        {
            spell = *it;
            if (spell->targetId() == id)
            {
                it = m_spellList.remove(it);
                emit delSpell(spell);
                delete spell;
            }
            else
            {
                ++it;
            }
        }

        if (m_spellList.count() == 0)
        {
            m_timer->stop();
        }
    }

}

void SpellShell::timeout()
{
  SpellItem* spell;

  Q3ValueList<SpellItem*>::Iterator it = m_spellList.begin();
  while (it != m_spellList.end()) 
  {
    spell = *it;

    int d = spell->duration() -
      pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6);
    if (d > -6) 
    {
      spell->setDuration(d);
      emit changeSpell(spell);
      it++;
    } 
    else 
    {
      seqInfo("SpellItem '%s' finished.", (*it)->spellName().latin1());
      if (m_lastPlayerSpell == spell)
	m_lastPlayerSpell = 0;
      emit delSpell(spell);
      it = m_spellList.remove(it);
      delete spell;
    }
   }

  if (m_spellList.count() == 0)
    m_timer->stop();
}

#ifndef QMAKEBUILD
#include "spellshell.moc"
#endif

