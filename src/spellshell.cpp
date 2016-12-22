/*
 * spellshell.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Crazy Joe Divola (cjd1@users.sourceforge.net)
 * September 5, 2001
 * 
 * Portions Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
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

// #define DIAG_SPELLSHELL 1 

SpellItem::SpellItem()
  : m_duration(0),
    m_isPlayerBuff(0),
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
		       uint16_t targetId, const QString& targetName,
               bool isPlayerBuff, const QString& nameSuffix)
{
     setSpellId(spellId);

     setDuration(duration);

     QString spellName;
     if (spell)
     {
       spellName = spell->name();

       if (spell->targetType() != Spell::SELF && spell->targetType() != Spell::GROUP)
           setTargetId(targetId);
     }
     else
     {
       seqWarn("SpellItem::update(): unknown spellId %d", spellId);
       spellName = spell_name(spellId);
       setTargetId(targetId);
     }

     if (!nameSuffix.isEmpty())
     {
         spellName.append(nameSuffix);
     }
     setSpellName(spellName);

     setCasterId(casterId);

     if (!casterName.isEmpty() || isPlayerBuff)
       setCasterName(casterName);
     else
       setCasterName(QString("N/A"));

     if (!targetName.isEmpty())
       setTargetName(targetName);
     else
       setTargetName(QString("N/A"));

     setIsPlayerBuff(isPlayerBuff);

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

SpellItem* SpellShell::findPlayerBuff(uint16_t spellId)
{
    for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
            it != m_spellList.end();
            it++)
    {
        SpellItem *i = *it;
        if (i->spellId() == spellId && i->isPlayerBuff())
        {
            return i;
        }
    }

    return NULL;
}

SpellItem* SpellShell::findSpell(uint16_t spellId, 
				 uint16_t targetId, const QString& targetName)
{
  for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
      it != m_spellList.end(); 
      it++) 
  {
    SpellItem *i = *it;
    if (i->spellId() == spellId)
    {
      if ((i->targetId() == targetId) || 
              (targetId == m_player->id() && i->isPlayerBuff()) ||
	  ((i->targetId() == 0) && (i->targetName() == targetName)))
	return i;
    }
  }

  return NULL;
}

SpellItem* SpellShell::findSpell(int spell_id)
{
  for(QValueList<SpellItem*>::Iterator it = m_spellList.begin(); 
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
   for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
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

bool SpellShell::isPlayerBuff(const Spell* spell, uint32_t sourceID, uint32_t targetID) const
{
    const uint32_t player_id = m_player->id();
    if (targetID == player_id)
    {
        return true;
    }

    if (spell && (spell->targetType() == Spell::SELF || spell->targetType() == Spell::GROUP))
    {
        return sourceID == player_id;
    }

    return false;
}

// slots

void SpellShell::selfStartSpellCast(const uint8_t* data)
{
  const startCastStruct *c = (const startCastStruct *)data;
#ifdef DIAG_SPELLSHELL
  seqDebug("SpellShell::selfStartSpellCast(): id=%d (me->%d) (slot=%d, inv=%d)",
          c->spellId, c->targetId, c->slot, c->inventorySlot);
#endif // DIAG_SPELLSHELL

  // get the target 
  const Item* s;
  QString targetName;
  int duration = 0;
  const Spell* spell = m_spells->spell(c->spellId);
  SpellItem *item;
  if (spell)
    duration = spell->calcDuration(m_player->level()) * 6;

  if (isPlayerBuff(spell, m_player->id(), c->targetId))
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::selfStartSpellCast(): DROPPED self-buff (id=%d).",
              c->targetId);
#endif // DIAG_SPELLSHELL
      return;
  }

  if (c->targetId && ((s = m_spawnShell->findID(tSpawn, c->targetId))))
      targetName = s->name();

  item = findSpell(c->spellId, c->targetId, targetName);
  if (item) 
  { // exists
    item->update(c->spellId, spell, duration,
		 m_player->id(), m_player->name(),
		 c->targetId, targetName, false);
    emit changeSpell(item);
  } 
  else 
  { // new spell
    item = new SpellItem();
    item->update(c->spellId, spell, duration,
		 m_player->id(), m_player->name(),
		 c->targetId, targetName, false);
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
  seqDebug("SpellShell::buffLoad(): id=%d, duration=%d, dmg_rem=%d, persistant=%d, reserved=%d, playerId=%d, m_player->id()=%d.",
          c->spellid, c->duration, c->dmg_shield_remaining, c->persistant_buff, c->reserved, c->playerId, m_player->id());
#endif // DIAG_SPELLSHELL

  const Spell* spell = m_spells->spell(c->spellid);
  int duration = c->duration * 6;
  QString nameSuffix = c->dmg_shield_remaining ? QString(" - %1").arg(c->dmg_shield_remaining) : "";
  SpellItem *item = findPlayerBuff(c->spellid);
  if (item) 
  { // exists
    item->update(c->spellid, spell, duration, 
		 0, "", m_player->id(), m_player->name(), true, nameSuffix);
    emit changeSpell(item);
  } 
  else 
  { // new spell
    item = new SpellItem();
    item->update(c->spellid, spell, duration, 
		 0, "", m_player->id(), m_player->name(), true, nameSuffix);
    m_spellList.append(item);
    if ((m_spellList.count() > 0) && (!m_timer->isActive()))
      m_timer->start(1000 *
		     pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
    emit addSpell(item);
  }
}

void SpellShell::buff(const uint8_t* data, size_t, uint8_t dir)
{
  const buffStruct* b = (const buffStruct*)data;
#ifdef DIAG_SPELLSHELL
  seqDebug("SpellShell::buff(): spellID=%d (%d->%d) duration=%d changetype=%d"
           " spellslot=%d effect_type=%d level=%d counters=%d unknown003=%d"
           " dir=%d",
          b->spellid, b->playerId, b->spawnid, b->duration, b->changetype,
          b->spellslot, b->effect_type, b->level, b->counters, b->unknown003,
          dir);
#endif // DIAG_SPELLSHELL

  // we only care about the server
  if (dir == DIR_Client)
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::buff(): DROPPING DIR_Client");
#endif // DIAG_SPELLSHELL
      return;
  }

  // if this is the second server packet then ignore it
  if (b->spellid == 0xffffffff)
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::buff(): DROPPING spellId=0xffffffff");
#endif // DIAG_SPELLSHELL
      return;
  }

  const Spell* spell = m_spells->spell(b->spellid);

  // find the spell item
  SpellItem* item;
  const Item* s;
  QString targetName;
  if (!isPlayerBuff(spell, b->playerId, b->spawnid))
  {
    if (b->spawnid && 
	((s = m_spawnShell->findID(tSpawn, b->spawnid))))
      targetName = s->name();
    
    item = findSpell(b->spellid, b->spawnid, targetName);
  }
  else
  {
    item = findPlayerBuff(b->spellid);
  }

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

void SpellShell::action(const uint8_t* data, size_t, uint8_t dir)
{
  const actionStruct* a = (const actionStruct*)data;

  if (a->type != 0xe7) // only things to do if action is a spell
    return;

#ifdef DIAG_SPELLSHELL
  seqDebug("SpellShell::action(): id=%d (%d->%d) (lvl: %d) causing %d damage. sequence=%d. buff=%d. dir=%d",
          a->spell, a->source, a->target, a->level, a->damage, a->sequence, a->make_buff_icon, dir);
#endif // DIAG_SPELLSHELL

  if (a->make_buff_icon != 4)
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::action(): make_buff_icon != 4");
#endif // DIAG_SPELLSHELL
      return;
  }

  const Item* s;
  QString targetName;

  if (a->target && 
      ((s = m_spawnShell->findID(tSpawn, a->target))))
    targetName = s->name();

  SpellItem *item = findSpell(a->spell, a->target, targetName);
  const Spell* spell = m_spells->spell(a->spell);
  const bool is_player_buff = isPlayerBuff(spell, a->source, a->target);

  // Drop actions that do not involve the player or are not updates to
  // existing spells
  if (!item && !is_player_buff && a->source != m_player->id())
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::action(): DROP unimportant spell");
#endif // DIAG_SPELLSHELL
      return;
  }

  int duration = 0;
  if (spell)
      duration = spell->calcDuration(a->level) * 6;

  if (duration == 0)
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::action(): DROP 0 duration spell");
#endif // DIAG_SPELLSHELL
      return;
  }

  QString casterName;
  if (a->source && ((s = m_spawnShell->findID(tSpawn, a->source))))
      casterName = s->name();

  if (item)
  {
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::action(): found");
#endif // DIAG_SPELLSHELL

      item->update(a->spell, spell, duration, 
              a->source, casterName,
              is_player_buff ? m_player->id() : a->target,
              is_player_buff ? m_player->name() : targetName,
              is_player_buff);
      emit changeSpell(item);
  }
  else if (is_player_buff)
  {
      // otherwise check for spells cast on us
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::action(): new player buff");
#endif // DIAG_SPELLSHELL

      // only way to get here is if there wasn't an existing spell, so...
      item = new SpellItem();
      item->update(a->spell, spell, duration, 
              a->source, casterName,
              m_player->id(), m_player->name(), is_player_buff);
      m_spellList.append(item);
      if ((m_spellList.count() > 0) && (!m_timer->isActive()))
          m_timer->start(1000 *
                  pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
      emit addSpell(item);
  }
  else if (a->source == m_player->id())
  {
      // otherwise check for spells cast by us
#ifdef DIAG_SPELLSHELL
      seqDebug("-SpellShell::action(): new");
#endif // DIAG_SPELLSHELL

      // only way to get here is if there wasn't an existing spell, so...
      item = new SpellItem();
      item->update(a->spell, spell, duration,
              a->source, casterName,
              a->target, targetName, false);
      m_spellList.append(item);
      if ((m_spellList.count() > 0) && (!m_timer->isActive()))
          m_timer->start(1000 *
                  pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
      emit addSpell(item);
  }
  else
  {
      seqWarn("-SpellShell::action(): unexpected case");
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
#ifdef DIAG_SPELLSHELL
    seqDebug("SpellShell::simpleMessage(): deleting m_lastPlayerSpell. messageFormat=%d",
            smsg->messageFormat);
#endif // DIAG_SPELLSHELL
    // delete the last player spell
    deleteSpell(m_lastPlayerSpell);
    m_lastPlayerSpell = 0;
    break;
  default:
#ifdef DIAG_SPELLSHELL
    seqDebug("SpellShell::simpleMessage(): ignoring messageFormat=%d",
            smsg->messageFormat);
#endif // DIAG_SPELLSHELL
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
      for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
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
  for(QValueList<SpellItem*>::Iterator it = m_spellList.begin();
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
  SpellItem* spell;

  if (m_lastPlayerSpell && (m_lastPlayerSpell->targetId() == id))
    m_lastPlayerSpell = 0;

  QValueList<SpellItem*>::Iterator it = m_spellList.begin();
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
      ++it;
  }

  if (m_spellList.count() == 0)
    m_timer->stop();
}

void SpellShell::timeout()
{
  SpellItem* spell;

  QValueList<SpellItem*>::Iterator it = m_spellList.begin();
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

#include "spellshell.moc"
