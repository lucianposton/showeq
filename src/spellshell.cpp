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

       if (spell->targetType() != Spell::SELF)
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
  m_spells(spells)
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

void SpellShell::updateOrAdd(SpellItem* item,
        uint16_t spellid, const Spell* spell, int duration,
        uint16_t casterId, const QString& casterName,
        uint16_t targetId, const QString& targetName,
        bool isPlayerBuff, const QString& nameSuffix)
{
    if (item)
    {
        item->update(spellid, spell, duration,
                casterId, casterName,
                targetId, targetName,
                isPlayerBuff, nameSuffix);
        emit changeSpell(item);
    }
    else
    {
        item = new SpellItem();
        item->update(spellid, spell, duration,
                casterId, casterName,
                targetId, targetName,
                isPlayerBuff, nameSuffix);
        m_spellList.append(item);
        if ((m_spellList.count() > 0) && (!m_timer->isActive()))
            m_timer->start(1000 *
                    pSEQPrefs->getPrefInt("SpellTimer", "SpellList", 6));
        emit addSpell(item);
    }
}

void SpellShell::clear()
{
   emit clearSpells();

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

    if (spell && (spell->targetType() == Spell::SELF))
    {
        return sourceID == player_id;
    }

    return false;
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
  updateOrAdd(item,
          c->spellid, spell, duration,
          0, "",
          m_player->id(), m_player->name(),
          true, nameSuffix);
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

void SpellShell::translocate(const uint8_t* data, size_t, uint8_t dir)
{
    const Translocate_Struct* a = (const Translocate_Struct*)data;
    const uint32_t spellid = a->SpellID;
    const Spell* spell = m_spells->spell(spellid);
    SpellItem *item = findPlayerBuff(spellid);
#ifdef DIAG_SPELLSHELL
    seqDebug("SpellShell::translocate(), spell->name()=%s item->spellName()=%s",
            (const char*)spell->name(), item?(const char*)item->spellName():"n/a");
#endif // DIAG_SPELLSHELL
    const int duration = 50 * 6; // Seems hardcoded in client
    updateOrAdd(item,
            spellid, spell, duration,
            0, "",
            m_player->id(), m_player->name(),
            true);
}

void SpellShell::resurrect(const uint8_t* data, size_t, uint8_t dir)
{
    const Resurrect_Struct* a = (const Resurrect_Struct*)data;
    const uint32_t spellid = a->spellid;
    const Spell* spell = m_spells->spell(spellid);
    SpellItem *item = findPlayerBuff(spellid);
#ifdef DIAG_SPELLSHELL
    seqDebug("SpellShell::resurrect(), spell->name()=%s item->spellName()=%s",
            (const char*)spell->name(), item?(const char*)item->spellName():"n/a");
#endif // DIAG_SPELLSHELL
    const int duration = 50 * 6; // Seems hardcoded in client
    updateOrAdd(item,
            spellid, spell, duration,
            0, "",
            m_player->id(), m_player->name(),
            true);
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

#ifdef DIAG_SPELLSHELL
  seqDebug("-SpellShell::action(): %s spell, is_player_buff=%d",
          item?"found":"new", is_player_buff);
#endif // DIAG_SPELLSHELL

  updateOrAdd(item,
          a->spell, spell, duration,
          a->source, casterName,
          is_player_buff ? m_player->id() : a->target,
          is_player_buff ? m_player->name() : targetName,
          is_player_buff);
}

void SpellShell::zoneChanged(void)
{
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
      emit delSpell(spell);
      it = m_spellList.remove(it);
      delete spell;
    }
   }

  if (m_spellList.count() == 0)
    m_timer->stop();
}

#include "spellshell.moc"
