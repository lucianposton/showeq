/*
 *  combatlog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net
 */

#include "combatlog.h"
#include "player.h"
#include "util.h"
#include "diagnosticmessages.h"

#include <algorithm>

#include <qgrid.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <stdio.h>
#include <time.h>

#define DEBUGCOMBAT

#undef DEBUGCOMBAT

namespace {

static DamageCategory damageCategory(int iType)
{
    switch(iType)
    {
        case 0:		// 1H Blunt
        case 1:		// 1H Slashing
        case 2:		// 2H Blunt
        case 3:		// 2H Slashing
        case 28:	// Hand To Hand
        case 36:	// Piercing
            {
                return DAMAGE_CATEGORY_MELEE;
            }
        case 7:		// Archery
        case 51:	// Throwing
            {
                return DAMAGE_CATEGORY_MELEE_RANGED;
            }
        case 10:	// Bash
        case 30:	// Kick
            {
                return DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC;
            }
        case 8:		// Backstab
            {
                return DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB;
            }
        case 21:	// Dragon Punch
        case 23:	// Eagle Strike
        case 26:	// Flying Kick
        case 38:	// Round Kick
        case 52:	// Tiger Claw
            {
                return DAMAGE_CATEGORY_MELEE_SPECIAL_MONK;
            }
        case 231:       // Non Melee Damage e.g. spells
            {
                return DAMAGE_CATEGORY_NONMELEE;
            }
        case 250: // Lava.
        case 251: // Drowning. In OP_Death, type=23 instead
        case 252: // Falling. In OP_Death, type=23 instead
        case 253: // Physical Trap.
        //case ?: // Magical Trap.
            {
                return DAMAGE_CATEGORY_ENVIRONMENT;
            }
        case 4: // pet kills self, /pet get lost
            {
                return DAMAGE_CATEGORY_OTHER;
            }
        default:        // Damage Shield?
            {
                // 245 Mark of Retribution
                // 248 Flameshield of Ro? (45pt) (mage)
                // -11 Killing Blow with MoR
                // -8  Killing Blow with Ro? (45pt) (mage)
                return DAMAGE_CATEGORY_DAMAGE_SHIELD;
            }
    }
}

static bool isNonMeleeDamage(DamageCategory c, int iDamage)
{
    // Checking iDamage > 0 avoids buff spells
    return c == DAMAGE_CATEGORY_NONMELEE && iDamage > 0;
}

static bool isMelee(DamageCategory c)
{
    return c == DAMAGE_CATEGORY_MELEE
        || c == DAMAGE_CATEGORY_MELEE_RANGED
        || c == DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC
        || c == DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB
        || c == DAMAGE_CATEGORY_MELEE_SPECIAL_MONK;
}

static bool isDamageShield(DamageCategory c)
{
    return c == DAMAGE_CATEGORY_DAMAGE_SHIELD;
}

static bool isIgnoredDamageCategory(DamageCategory c)
{
    return c == DAMAGE_CATEGORY_ENVIRONMENT || c == DAMAGE_CATEGORY_OTHER;
}

template <class T>
void selectRecordsById(SEQListView* listView, QList<T>& records, int id)
{
    listView->clearSelection();
    Record *pRecord;
    for (pRecord = records.first(); pRecord != 0; pRecord = records.next())
    {
        if (pRecord->matchesForSelection(id))
        {
            SEQListViewItem<>* viewItem = pRecord->getViewItem();
            if (viewItem)
                listView->setSelected(viewItem, true);
        }
    }
}

} // namespace


Record::Record() :
    m_isDirty(true),
    m_viewItem(NULL)
{
}

Record::~Record()
{
}

void Record::clear()
{
    clearImpl();
    m_isDirty = true;
    m_viewItem = NULL;
}

void Record::addHit(int iDamage, DamageCategory category,
        int iTarget, int iSource)
{
    addHitImpl(iDamage, category, iTarget, iSource);
    m_isDirty = true;
}

void Record::addMiss(int iMissReason)
{
    addMissImpl(iMissReason);
    m_isDirty = true;
}

void Record::update(QListView* parent, int last_consider_id)
{
    if (!m_isDirty)
        return;
    m_isDirty = false;

    updateImpl(parent, last_consider_id);
}

void Record::updateViewItem(
        QListView* parent,
        int last_consider_id,
        const QString& l0,
        const QString& l1,
        const QString& l2,
        const QString& l3,
        const QString& l4,
        const QString& l5,
        const QString& l6,
        const QString& l7,
        const QString& l8,
        const QString& l9,
        const QString& l10,
        const QString& l11)
{
    if (m_viewItem == NULL)
    {
        m_viewItem = new SEQListViewItem<>(
                parent,
                l0,
                l1,
                l2,
                l3,
                l4,
                l5,
                l6,
                l7);
        if (!l8.isNull())
            m_viewItem->setText(8, l8);
        if (!l9.isNull())
            m_viewItem->setText(9, l9);
        if (!l10.isNull())
            m_viewItem->setText(10, l10);
        if (!l11.isNull())
            m_viewItem->setText(11, l11);
        initializeViewItem(m_viewItem);
        parent->insertItem(m_viewItem);
        if (last_consider_id && matchesForSelection(last_consider_id))
            parent->setSelected(m_viewItem, true);
    }
    else
    {
        m_viewItem->setText(0, l0);
        m_viewItem->setText(1, l1);
        m_viewItem->setText(2, l2);
        m_viewItem->setText(3, l3);
        m_viewItem->setText(4, l4);
        m_viewItem->setText(5, l5);
        m_viewItem->setText(6, l6);
        m_viewItem->setText(7, l7);
        m_viewItem->setText(8, l8);
        m_viewItem->setText(9, l9);
        m_viewItem->setText(10, l10);
        m_viewItem->setText(11, l11);
    }
}


////////////////////////////////////////////
//  CombatOffenseRecord implementation
////////////////////////////////////////////
CombatOffenseRecord::CombatOffenseRecord( int iType, const Player* p, int iSpell) :
    Record(),
	m_iType(iType),
	m_iSpell(iSpell),
	m_player(p),
	m_iHits(0),
	m_iMisses(0),
	m_iMinDamage(0),
	m_iMaxDamage(0),
	m_iTotalDamage(0),
	m_dAverage(0.0),
	m_dM2(0.0)
{
}

void CombatOffenseRecord::clearImpl()
{
    m_iHits = 0;
    m_iMisses = 0;
    m_iMinDamage = 0;
    m_iMaxDamage = 0;
    m_iTotalDamage = 0;
    m_dAverage = 0.0;
    m_dM2 = 0.0;
}

void CombatOffenseRecord::addHitImpl(int iDamage, DamageCategory, int, int)
{
	if(iDamage <= 0)
		return;

	m_iHits++;
	m_iTotalDamage += iDamage;

	if(iDamage < m_iMinDamage || !m_iMinDamage)
		m_iMinDamage = iDamage;

	if(iDamage > m_iMaxDamage)
		m_iMaxDamage = iDamage;

    const double delta1 = iDamage - m_dAverage;
    m_dAverage += delta1 / m_iHits;
    const double delta2 = iDamage - m_dAverage;
    m_dM2 += delta1 * delta2;
}

void CombatOffenseRecord::addMissImpl(int)
{
    m_iMisses += 1;
}

void CombatOffenseRecord::updateImpl(QListView* parent, int last_consider_id)
{
    const int iType = getType();
    const int iSpell = getSpell();
    const int iHits = getHits();
    const int iMisses = getMisses();
    const int iMinDamage = getMinDamage();
    const int iMaxDamage = getMaxDamage();
    const int iDamage = getTotalDamage();
    const double dSD = getStandardDeviation();

    const double dAvgDamage = (double)iDamage / (double)iHits;
    const double dAccuracy = (double)iHits / (double)(iMisses+iHits);

    QString s_type;
    const DamageCategory category = damageCategory(iType);
    switch(category)
    {
        case DAMAGE_CATEGORY_MELEE:
        case DAMAGE_CATEGORY_MELEE_RANGED:
        case DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC:
        case DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB:
        case DAMAGE_CATEGORY_MELEE_SPECIAL_MONK:
            {
                s_type.sprintf("%s(%d)", (const char*)skill_name(iType), iType);
                break;
            }
        case DAMAGE_CATEGORY_NONMELEE:
            {
                s_type.sprintf("Spell: %s(%d)", (const char*)spell_name(iSpell), iSpell);
                break;
            }
        case DAMAGE_CATEGORY_DAMAGE_SHIELD:
            {
                s_type.sprintf("Damage Shield: (%d)", iType);
                break;
            }
        case DAMAGE_CATEGORY_ENVIRONMENT:
        case DAMAGE_CATEGORY_OTHER:
            {
                seqWarn("CombatOffenseRecord::updateImpl: Unknown s_type. unexpected category=%d", category);
                break;
            }
    }

    QString s_hits;
    s_hits.setNum(iHits);
    QString s_misses;
    QString s_accuracy;
    if (category != DAMAGE_CATEGORY_DAMAGE_SHIELD)
    {
        s_misses.setNum(iMisses);
        s_accuracy = QString::number(dAccuracy, 'f', 2);
    }
    const QString s_avgdamage = doubleToQString(dAvgDamage, 0);
    const QString s_sd = doubleToQString(dSD, 0, true);
    const QString s_mindamage = intToQString(iMinDamage);
    const QString s_maxdamage = intToQString(iMaxDamage);
    QString s_damage;
    s_damage.setNum(iDamage);

    updateViewItem(
            parent,
            last_consider_id,
            s_type,
            s_hits,
            s_misses,
            s_accuracy,
            s_avgdamage,
            s_sd,
            s_mindamage,
            s_maxdamage,
            s_damage);
}

void CombatOffenseRecord::initializeViewItem(SEQListViewItem<>* pItem)
{
    pItem->setColComparator(1, SEQListViewItemCompareInt);
    pItem->setColComparator(2, SEQListViewItemCompareInt);
    pItem->setColComparator(3, SEQListViewItemCompareDouble);
    pItem->setColComparator(4, SEQListViewItemCompareDouble);
    pItem->setColComparator(5, SEQListViewItemCompareDouble);
    pItem->setColComparator(6, SEQListViewItemCompareInt);
    pItem->setColComparator(7, SEQListViewItemCompareInt);
    pItem->setColComparator(8, SEQListViewItemCompareInt);
}


////////////////////////////////////////////
//  PetOffenseRecord implementation
////////////////////////////////////////////
PetOffenseRecord::PetOffenseRecord(int iPetID, const QString& iPetName, int iType, Player* p, int iSpell) :
	CombatOffenseRecord(iType, p, iSpell),
	m_iPetID(iPetID),
	m_iPetName(iPetName)
{
}

void PetOffenseRecord::updateImpl(QListView* parent, int last_consider_id)
{
    const QString iPetName = getPetName();
    const int iPetID = getPetID();
    const int iType = getType();
    const int iSpell = getSpell();
    const int iHits = getHits();
    const int iMisses = getMisses();
    const int iMinDamage = getMinDamage();
    const int iMaxDamage = getMaxDamage();
    const int iDamage = getTotalDamage();
    const double dSD = getStandardDeviation();

    const double dAvgDamage = (double)iDamage / (double)iHits;
    const double dAccuracy = (double)iHits / (double)(iMisses+iHits);

    QString s_type;
    const DamageCategory category = damageCategory(iType);
    switch(category)
    {
        case DAMAGE_CATEGORY_MELEE:
        case DAMAGE_CATEGORY_MELEE_RANGED:
        case DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC:
        case DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB:
        case DAMAGE_CATEGORY_MELEE_SPECIAL_MONK:
            {
                s_type.sprintf("Pet: %s(%d): %s(%d)",
                        (const char*)iPetName, iPetID,
                        (const char*)skill_name(iType), iType);
                break;
            }
        case DAMAGE_CATEGORY_NONMELEE:
            {
                s_type.sprintf("Pet: %s(%d): Spell: %s(%d)",
                        (const char*)iPetName, iPetID,
                        (const char*)spell_name(iSpell), iSpell);
                break;
            }
        case DAMAGE_CATEGORY_DAMAGE_SHIELD:
            {
                s_type.sprintf("Pet: %s(%d): Damage Shield: (%d)", (const char*)iPetName, iPetID, iType);
                break;
            }
        case DAMAGE_CATEGORY_ENVIRONMENT:
        case DAMAGE_CATEGORY_OTHER:
            {
                seqWarn("PetOffenseRecord::updateImpl: Unknown pet s_type. unexpected category=%d", category);
                break;
            }
    }

    QString s_hits;
    s_hits.setNum(iHits);
    QString s_misses;
    QString s_accuracy;
    if (category != DAMAGE_CATEGORY_DAMAGE_SHIELD)
    {
        s_misses.setNum(iMisses);
        s_accuracy = QString::number(dAccuracy, 'f', 2);
    }
    const QString s_avgdamage = doubleToQString(dAvgDamage, 0);
    const QString s_sd = doubleToQString(dSD, 0, true);
    const QString s_mindamage = intToQString(iMinDamage);
    const QString s_maxdamage = intToQString(iMaxDamage);
    QString s_damage;
    s_damage.setNum(iDamage);

    updateViewItem(
            parent,
            last_consider_id,
            s_type,
            s_hits,
            s_misses,
            s_accuracy,
            s_avgdamage,
            s_sd,
            s_mindamage,
            s_maxdamage,
            s_damage);
}

void PetOffenseRecord::initializeViewItem(SEQListViewItem<>* pItem)
{
    pItem->setColComparator(1, SEQListViewItemCompareInt);
    pItem->setColComparator(2, SEQListViewItemCompareInt);
    pItem->setColComparator(3, SEQListViewItemCompareDouble);
    pItem->setColComparator(4, SEQListViewItemCompareDouble);
    pItem->setColComparator(5, SEQListViewItemCompareDouble);
    pItem->setColComparator(6, SEQListViewItemCompareInt);
    pItem->setColComparator(7, SEQListViewItemCompareInt);
    pItem->setColComparator(8, SEQListViewItemCompareInt);
}


////////////////////////////////////////////
//  DotOffenseRecord implementation
////////////////////////////////////////////
DotOffenseRecord::DotOffenseRecord(const Player* p, const QString& iSpellName) :
    CombatOffenseRecord(231, p, ITEM_SPELLID_NOSPELL), // assume spell, so 231
    m_iSpellName(iSpellName)
{
}

void DotOffenseRecord::updateImpl(QListView* parent, int last_consider_id)
{
    const int iTicks = getHits();
    const int iMinDamage = getMinDamage();
    const int iMaxDamage = getMaxDamage();
    const int iDamage = getTotalDamage();
    const double dSD = getStandardDeviation();

    const double dAvgDamage = (double)iDamage / (double)iTicks;

    QString s_type;
    s_type.sprintf("DoT: %s", (const char*)getSpellName());
    QString s_hits;
    s_hits.setNum(iTicks);
    QString s_misses;
    QString s_accuracy;
    const QString s_avgdamage = doubleToQString(dAvgDamage, 0);
    const QString s_sd = doubleToQString(dSD, 0, true);
    const QString s_mindamage = intToQString(iMinDamage);
    const QString s_maxdamage = intToQString(iMaxDamage);
    QString s_damage;
    s_damage.setNum(iDamage);

    updateViewItem(
            parent,
            last_consider_id,
            s_type,
            s_hits,
            s_misses,
            s_accuracy,
            s_avgdamage,
            s_sd,
            s_mindamage,
            s_maxdamage,
            s_damage);
}

void DotOffenseRecord::initializeViewItem(SEQListViewItem<>* pItem)
{
    pItem->setColComparator(1, SEQListViewItemCompareInt);
    pItem->setColComparator(2, SEQListViewItemCompareInt);
    pItem->setColComparator(3, SEQListViewItemCompareDouble);
    pItem->setColComparator(4, SEQListViewItemCompareDouble);
    pItem->setColComparator(5, SEQListViewItemCompareDouble);
    pItem->setColComparator(6, SEQListViewItemCompareInt);
    pItem->setColComparator(7, SEQListViewItemCompareInt);
    pItem->setColComparator(8, SEQListViewItemCompareInt);
}


////////////////////////////////////////////
//  CombatDefenseRecord implementation
////////////////////////////////////////////
CombatDefenseRecord::CombatDefenseRecord(const Spawn* s) :
    Record(),
    m_displayString(createRecordIDString(s->name(), s->id(),
                s->classString(), s->level()))
{
  clear();
}

QString CombatDefenseRecord::createRecordIDString(
        const QString& name, int id, const QString& classname, int level)
{
    QString result;
    result.sprintf("%s [%d %s] (%d)",
            (const char*)name, level, (const char*)classname, id);
    return result;
}

void CombatDefenseRecord::clearImpl()
{
  m_iHits = 0;
  m_iMisses = 0;
  m_iBlocks = 0;
  m_iParries = 0;
  m_iRipostes = 0;
  m_iDodges = 0;
  m_iInvulnerables= 0;
  m_iShieldAbsorbs = 0;
  m_iTotalDamage = 0;
  m_iTotalAttacks = 0;

  m_dMeleeM2 = 0.0;
  m_dMeleeAverage = 0.0;
  m_iMeleeHits = 0;
  m_iMeleeTotalDamage = 0;
  m_iMeleeMinDamage = 0;
  m_iMeleeMaxDamage = 0;

  m_dRangedM2 = 0.0;
  m_dRangedAverage = 0.0;
  m_iRangedHits = 0;
  m_iRangedTotalDamage = 0;
  m_iRangedMinDamage = 0;
  m_iRangedMaxDamage = 0;

  m_dSpecialM2 = 0.0;
  m_dSpecialAverage = 0.0;
  m_iSpecialHits = 0;
  m_iSpecialTotalDamage = 0;
  m_iSpecialMinDamage = 0;
  m_iSpecialMaxDamage = 0;

  m_dBackstabM2 = 0.0;
  m_dBackstabAverage = 0.0;
  m_iBackstabHits = 0;
  m_iBackstabTotalDamage = 0;
  m_iBackstabMinDamage = 0;
  m_iBackstabMaxDamage = 0;

  m_dMonkM2 = 0.0;
  m_dMonkAverage = 0.0;
  m_iMonkHits = 0;
  m_iMonkTotalDamage = 0;
  m_iMonkMinDamage = 0;
  m_iMonkMaxDamage = 0;

  m_dNonmeleeM2 = 0.0;
  m_dNonmeleeAverage = 0.0;
  m_iNonmeleeHits = 0;
  m_iNonmeleeTotalDamage = 0;
  m_iNonmeleeMinDamage = 0;
  m_iNonmeleeMaxDamage = 0;

  m_dDamageShieldM2 = 0.0;
  m_dDamageShieldAverage = 0.0;
  m_iDamageShieldHits = 0;
  m_iDamageShieldTotalDamage = 0;
  m_iDamageShieldMinDamage = 0;
  m_iDamageShieldMaxDamage = 0;
}

void CombatDefenseRecord::addHitImpl(int iDamage, DamageCategory category, int, int)
{
	if(iDamage <= 0)
		return;

	m_iTotalAttacks++;
	m_iHits++;
	m_iTotalDamage += iDamage;

    switch(category)
    {
        case DAMAGE_CATEGORY_MELEE:
            {
            m_iMeleeHits++;
            const double delta1 = iDamage - m_dMeleeAverage;
            m_dMeleeAverage += delta1 / m_iMeleeHits;
            const double delta2 = iDamage - m_dMeleeAverage;
            m_dMeleeM2 += delta1 * delta2;

            m_iMeleeTotalDamage += iDamage;
            if(iDamage < m_iMeleeMinDamage || !m_iMeleeMinDamage)
                m_iMeleeMinDamage = iDamage;
            if(iDamage > m_iMeleeMaxDamage)
                m_iMeleeMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_MELEE_RANGED:
            {
            m_iRangedHits++;
            const double delta1 = iDamage - m_dRangedAverage;
            m_dRangedAverage += delta1 / m_iRangedHits;
            const double delta2 = iDamage - m_dRangedAverage;
            m_dRangedM2 += delta1 * delta2;

            m_iRangedTotalDamage += iDamage;
            if(iDamage < m_iRangedMinDamage || !m_iRangedMinDamage)
                m_iRangedMinDamage = iDamage;
            if(iDamage > m_iRangedMaxDamage)
                m_iRangedMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC:
            {
            m_iSpecialHits++;
            const double delta1 = iDamage - m_dSpecialAverage;
            m_dSpecialAverage += delta1 / m_iSpecialHits;
            const double delta2 = iDamage - m_dSpecialAverage;
            m_dSpecialM2 += delta1 * delta2;

            m_iSpecialTotalDamage += iDamage;
            if(iDamage < m_iSpecialMinDamage || !m_iSpecialMinDamage)
                m_iSpecialMinDamage = iDamage;
            if(iDamage > m_iSpecialMaxDamage)
                m_iSpecialMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB:
            {
            m_iBackstabHits++;
            const double delta1 = iDamage - m_dBackstabAverage;
            m_dBackstabAverage += delta1 / m_iBackstabHits;
            const double delta2 = iDamage - m_dBackstabAverage;
            m_dBackstabM2 += delta1 * delta2;

            m_iBackstabTotalDamage += iDamage;
            if(iDamage < m_iBackstabMinDamage || !m_iBackstabMinDamage)
                m_iBackstabMinDamage = iDamage;
            if(iDamage > m_iBackstabMaxDamage)
                m_iBackstabMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_MELEE_SPECIAL_MONK:
            {
            m_iMonkHits++;
            const double delta1 = iDamage - m_dMonkAverage;
            m_dMonkAverage += delta1 / m_iMonkHits;
            const double delta2 = iDamage - m_dMonkAverage;
            m_dMonkM2 += delta1 * delta2;

            m_iMonkTotalDamage += iDamage;
            if(iDamage < m_iMonkMinDamage || !m_iMonkMinDamage)
                m_iMonkMinDamage = iDamage;
            if(iDamage > m_iMonkMaxDamage)
                m_iMonkMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_NONMELEE:
            {
            m_iNonmeleeHits++;
            const double delta1 = iDamage - m_dNonmeleeAverage;
            m_dNonmeleeAverage += delta1 / m_iNonmeleeHits;
            const double delta2 = iDamage - m_dNonmeleeAverage;
            m_dNonmeleeM2 += delta1 * delta2;

            m_iNonmeleeTotalDamage += iDamage;
            if(iDamage < m_iNonmeleeMinDamage || !m_iNonmeleeMinDamage)
                m_iNonmeleeMinDamage = iDamage;
            if(iDamage > m_iNonmeleeMaxDamage)
                m_iNonmeleeMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_DAMAGE_SHIELD:
            {
            m_iDamageShieldHits++;
            const double delta1 = iDamage - m_dDamageShieldAverage;
            m_dDamageShieldAverage += delta1 / m_iDamageShieldHits;
            const double delta2 = iDamage - m_dDamageShieldAverage;
            m_dDamageShieldM2 += delta1 * delta2;

            m_iDamageShieldTotalDamage += iDamage;
            if(iDamage < m_iDamageShieldMinDamage || !m_iDamageShieldMinDamage)
                m_iDamageShieldMinDamage = iDamage;
            if(iDamage > m_iDamageShieldMaxDamage)
                m_iDamageShieldMaxDamage = iDamage;
            break;
            }
        case DAMAGE_CATEGORY_OTHER:
        case DAMAGE_CATEGORY_ENVIRONMENT:
            seqWarn("CombatDefenseRecord::addHitImpl: unexpected category=%d", category);
            break;
    }
}

void CombatDefenseRecord::addMissImpl(int iMissReason)
{
	m_iTotalAttacks++;

	switch(iMissReason)
	{
		case COMBAT_MISS:
		{
			m_iMisses++;
			break;
		}
		case COMBAT_BLOCK:
		{
			m_iBlocks++;
			break;
		}
		case COMBAT_PARRY:
		{
			m_iParries++;
			break;
		}
		case COMBAT_RIPOSTE:
		{
			m_iRipostes++;
			break;
		}
		case COMBAT_DODGE:
		{
			m_iDodges++;
			break;
		}
		case COMBAT_INVULNERABLE:
		{
			m_iInvulnerables++;
			break;
		}
		case COMBAT_SHIELD_ABSORB:
		{
			m_iShieldAbsorbs++;
			break;
		}
		default:
		{
		  seqDebug("CombatDefenseRecord::addMissImpl: invalid miss reason (%d)", iMissReason);
			break;
		}
	}
}

void CombatDefenseRecord::updateImpl(QListView* parent, int last_consider_id)
{
    // Not used in a list view
}

void CombatDefenseRecord::initializeViewItem(SEQListViewItem<>* pItem)
{
    // Not used in a list view
}


////////////////////////////////////////////
//	CombatMobRecord implementation
////////////////////////////////////////////
CombatMobRecord::CombatMobRecord(int iID, const QString& mobName, Player* p) :
    Record(),
m_iID(iID),
m_iName(mobName),
m_player(p),
m_iStartTime(0),
m_iLastTime(0),
m_iDamageGiven(0),
m_dDPS(0.0),
m_iDamageTaken(0),
m_dMobDPS(0.0),
m_iPetStartTime(0),
m_iPetLastTime(0),
m_iPetDamageGiven(0),
m_dPetDPS(0.0),
m_iPetDamageTaken(0),
m_dPetMobDPS(0.0)
{
}

int CombatMobRecord::getDuration()
{
    const int iTimeElapsed = m_iLastTime - m_iStartTime;
    const int iPetTimeElapsed = m_iPetLastTime - m_iPetStartTime;
    if (iTimeElapsed && iPetTimeElapsed)
    {
        return std::max(m_iLastTime, m_iPetLastTime) - std::min(m_iStartTime, m_iPetStartTime);
    }
    else if (iTimeElapsed)
    {
        return iTimeElapsed;
    }
    else // Assume iPetTimeElapsed
    {
        return iPetTimeElapsed;
    }
};

void CombatMobRecord::addHitImpl(int iDamage, DamageCategory, int iTarget, int iSource)
{
    m_time = time(0);
    const int iPlayerID = m_player->id();
    const bool hitInvolvesPlayer = iPlayerID == iTarget || iPlayerID == iSource;
    if (hitInvolvesPlayer)
    {
        m_iLastTime = mTime();
        if (0 == m_iStartTime)
            m_iStartTime = m_iLastTime;
    }
    else // Assume pet
    {
        m_iPetLastTime = mTime();
        if (0 == m_iPetStartTime)
            m_iPetStartTime = m_iPetLastTime;
    }

    if (iDamage > 0)
    {
        if (hitInvolvesPlayer && m_iID == iTarget)
        {
            m_iDamageGiven += iDamage;
            const int iTimeElapsed = (m_iLastTime - m_iStartTime) / 1000;
            if (iTimeElapsed > 0)
                m_dDPS = (double)m_iDamageGiven / (double)iTimeElapsed;
        }
        else if (hitInvolvesPlayer && m_iID == iSource)
        {
            m_iDamageTaken += iDamage;
            const int iTimeElapsed = (m_iLastTime - m_iStartTime) / 1000;
            if (iTimeElapsed > 0)
                m_dMobDPS = (double)m_iDamageTaken / (double)iTimeElapsed;
        }
        else if (m_iID == iTarget) // Assume pet
        {
            m_iPetDamageGiven += iDamage;
            const int iTimeElapsed = (m_iPetLastTime - m_iPetStartTime) / 1000;
            if (iTimeElapsed > 0)
                m_dPetDPS = (double)m_iPetDamageGiven / (double)iTimeElapsed;
        }
        else // Assume pet
        {
            m_iPetDamageTaken += iDamage;
            const int iTimeElapsed = (m_iPetLastTime - m_iPetStartTime) / 1000;
            if (iTimeElapsed > 0)
                m_dPetMobDPS = (double)m_iPetDamageTaken / (double)iTimeElapsed;
        }
    }
}

void CombatMobRecord::addMissImpl(int)
{
}

void CombatMobRecord::clearImpl()
{
    m_iStartTime = 0;
    m_iLastTime = 0;
    m_iDamageGiven = 0;
    m_dDPS = 0.0;
    m_iDamageTaken = 0;
    m_dMobDPS = 0.0;
    m_iPetStartTime = 0;
    m_iPetLastTime = 0;
    m_iPetDamageGiven = 0;
    m_dPetDPS = 0.0;
    m_iPetDamageTaken = 0;
    m_dPetMobDPS = 0.0;
}

void CombatMobRecord::updateImpl(QListView* parent, int last_consider_id)
{
    const int iID = getID();
    const int iDuration = getDuration() / 1000;
    const int iDamageGiven = getDamageGiven();
    const double dDPS = getDPS();
    const int iDamageTaken = getDamageTaken();
    const double dMobDPS = getMobDPS();
    const int iPetDamageGiven = getPetDamageGiven();
    const double dPetDPS = getPetDPS();
    const int iPetDamageTaken = getPetDamageTaken();
    const double dPetMobDPS = getPetMobDPS();

    char s_time[64];
    const time_t timev = getTime();
    strftime(s_time, 64, "%m/%d %H:%M:%S", localtime(&timev));
    const QString s_name = getName();
    const QString s_id = QString::number(iID);
    const QString s_duration = QString::number(iDuration);
    const QString s_damagegiven = intToQString(iDamageGiven);
    const QString s_dps = doubleToQString(dDPS, 1);
    const QString s_iDamageTaken = intToQString(iDamageTaken);
    const QString s_mobdps = doubleToQString(dMobDPS, 1);
    const QString s_petdamagegiven = intToQString(iPetDamageGiven);
    const QString s_petdps = doubleToQString(dPetDPS, 1);
    const QString s_petiDamageTaken = intToQString(iPetDamageTaken);
    const QString s_petmobdps = doubleToQString(dPetMobDPS, 1);

    updateViewItem(
            parent,
            last_consider_id,
            s_time,
            s_name,
            s_id,
            s_duration,
            s_damagegiven,
            s_dps,
            s_iDamageTaken,
            s_mobdps,
            s_petdamagegiven,
            s_petdps,
            s_petiDamageTaken,
            s_petmobdps);
}

void CombatMobRecord::initializeViewItem(SEQListViewItem<>* pItem)
{
    pItem->setColComparator(2, SEQListViewItemCompareInt);
    pItem->setColComparator(3, SEQListViewItemCompareInt);
    pItem->setColComparator(4, SEQListViewItemCompareInt);
    pItem->setColComparator(5, SEQListViewItemCompareDouble);
    pItem->setColComparator(6, SEQListViewItemCompareInt);
    pItem->setColComparator(7, SEQListViewItemCompareDouble);
    pItem->setColComparator(8, SEQListViewItemCompareInt);
    pItem->setColComparator(9, SEQListViewItemCompareDouble);
    pItem->setColComparator(10, SEQListViewItemCompareInt);
    pItem->setColComparator(11, SEQListViewItemCompareDouble);
}


////////////////////////////////////////////
//	CombatOtherRecord implementation
////////////////////////////////////////////
CombatOtherRecord::CombatOtherRecord(
        int iTargetID, int iSourceID,
        const QString& tName, const QString& sName) :
    Record(),
    m_iTargetID(iTargetID),
    m_iSourceID(iSourceID),
    m_iTargetName(tName),
    m_iSourceName(sName),
    m_iStartTime(0),
    m_iLastTime(0),
    m_dDPS(0.0),
    m_iDamageTotal(0),
    m_time(0)
{
}

void CombatOtherRecord::addHitImpl(int iDamage, DamageCategory, int, int)
{
    m_time = time(0);
    m_iLastTime = mTime();
    if (0 == m_iStartTime)
        m_iStartTime = m_iLastTime;

    if (iDamage > 0)
        m_iDamageTotal += iDamage;

    const int iTimeElapsed = (m_iLastTime - m_iStartTime) / 1000;
    if (iTimeElapsed > 0)
    {
        m_dDPS = (double)m_iDamageTotal / (double)iTimeElapsed;
    }
}

void CombatOtherRecord::addMissImpl(int)
{
}

void CombatOtherRecord::clearImpl()
{
    m_iStartTime = 0;
    m_iLastTime = 0;
    m_dDPS = 0.0;
    m_iDamageTotal = 0;
    m_time = 0;
}

void CombatOtherRecord::updateImpl(QListView* parent, int last_consider_id)
{
    const int iDuration = getDuration() / 1000;
    const int iDamageTotal = getDamageTotal();
    const double dDPS = getDPS();

    char s_time[64];
    const time_t timev = getTime();
    strftime(s_time, 64, "%m/%d %H:%M:%S", localtime(&timev));
    const QString s_sourcename = getSourceName();
    const QString s_targetname = getTargetName();
    const QString s_sourceid = QString::number(getSourceID());
    const QString s_targetid = QString::number(getTargetID());
    const QString s_duration = QString::number(iDuration);
    const QString s_damagetotal = QString::number(iDamageTotal);
    const QString s_dps = doubleToQString(dDPS, 1);

    updateViewItem(
            parent,
            last_consider_id,
            s_time,
            s_sourcename,
            s_sourceid,
            s_targetname,
            s_targetid,
            s_duration,
            s_damagetotal,
            s_dps);
}

void CombatOtherRecord::initializeViewItem(SEQListViewItem<>* pItem)
{
    pItem->setColComparator(2, SEQListViewItemCompareInt);
    pItem->setColComparator(4, SEQListViewItemCompareInt);
    pItem->setColComparator(5, SEQListViewItemCompareInt);
    pItem->setColComparator(6, SEQListViewItemCompareInt);
    pItem->setColComparator(7, SEQListViewItemCompareDouble);
}


////////////////////////////////////////////
//	CombatWindow implementation
////////////////////////////////////////////

CombatWindow::~CombatWindow()
{
	if(m_combat_defense_record != 0)
	{
		delete m_combat_defense_record;
		m_combat_defense_record = 0;
	}
}

CombatWindow::CombatWindow(Player* player,
			   QWidget* parent, const char* name)
  : SEQWindow("Combat", "ShowEQ - Combat", parent, name),
    m_player(player),
    m_lastConsider(0),
    m_autoupdate_pet_defense_selection(false),
    m_iCurrentDPSTotal(0),
    m_iDPSStartTime(0),
    m_iDPSTimeLast(0),
    m_dDPS(0.0),
    m_dDPSLast(0.0),
    m_iPetCurrentDPSTotal(0),
    m_iPetDPSStartTime(0),
    m_iPetDPSTimeLast(0),
    m_dPetDPS(0.0),
    m_dPetDPSLast(0.0),
    m_iCurrentMobDPSTotal(0),
    m_iMobDPSStartTime(0),
    m_iMobDPSTimeLast(0),
    m_dMobDPS(0.0),
    m_dMobDPSLast(0.0),
    m_iPetCurrentMobDPSTotal(0),
    m_iPetMobDPSStartTime(0),
    m_iPetMobDPSTimeLast(0),
    m_dPetMobDPS(0.0),
    m_dPetMobDPSLast(0.0),
    m_last_consider_id(0),
    m_playerspell_action_sequence(0),
    m_playerspell_target_id(0),
    m_playerspell_spell_id(0),
    m_playerspell_damage(0),
    m_playerspell_target_cleaned_name(),
    m_playerspell_confirmed(false)
{
  /* Hopefully this is only called once to set up the window,
     so this is a good place to initialize some things which
     otherwise won't be. */

	m_combat_offense_list.setAutoDelete(true);
	m_pet_offense_list.setAutoDelete(true);
	m_dot_offense_list.setAutoDelete(true);
	m_combat_defense_record = new CombatDefenseRecord(player);
	m_combat_pet_defense_current_record = NULL;
	m_combat_pet_defense_list.setAutoDelete(true);
	m_combat_mob_list.setAutoDelete(true);
	m_combat_other_list.setAutoDelete(true);

	initUI();
}

void CombatWindow::initUI()
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::initUI: starting...");
#endif
	QVBoxLayout* layout = new QVBoxLayout(boxLayout());

	m_menu_bar = new QMenuBar(this);
	layout->addWidget(m_menu_bar);

	m_tab = new QTabWidget(this);
	layout->addWidget(m_tab);

	m_widget_offense = initOffenseWidget();
	m_tab->addTab(m_widget_offense, "&Offense");

	m_widget_defense = initDefenseWidget();
	m_tab->addTab(m_widget_defense, "&Defense");

	m_widget_pet_defense = initPetDefenseWidget();
	m_tab->addTab(m_widget_pet_defense, "&Pet Defense");

	m_widget_mob = initMobWidget();
	m_tab->addTab(m_widget_mob, "A&ttackers");

	m_widget_other = initOtherWidget();
	m_tab->addTab(m_widget_other, "&All");

	m_clear_menu = new QPopupMenu(this);
	m_clear_menu->insertItem("Clear All", this, SLOT(clear()));
	m_clear_menu->insertItem("Clear Offense Stats", this, SLOT(clearOffense()));
	m_clear_menu->insertItem("Clear Defense Stats", this, SLOT(clearDefense()));
	m_clear_menu->insertItem("Clear Pet Defense Stats", this, SLOT(clearPetDefense()));
	m_clear_menu->insertItem("Clear Mob Stats", this, SLOT(clearMob()));
	m_clear_menu->insertItem("Clear Other Stats", this, SLOT(clearOther()));

	m_menu_bar->insertItem("&Clear", m_clear_menu);

	updateOffense();
	updateDefense();
	updatePetDefense();
	updateMob();
	updateOther();

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::initUI: finished...");
#endif
}

QWidget* CombatWindow::initOffenseWidget()
{
	QWidget *pWidget = new QWidget(m_tab);

	m_layout_offense = new QVBoxLayout(pWidget);

	QGroupBox *listGBox = new QVGroupBox(pWidget);
	m_layout_offense->addWidget(listGBox);

	m_listview_offense = new SEQListView(preferenceName()+"_OffenseListView", listGBox);
	m_listview_offense->addColumn("Type");
	m_listview_offense->setColumnAlignment(0, Qt::AlignLeft);
	m_listview_offense->addColumn("Hit");
	m_listview_offense->setColumnAlignment(1, Qt::AlignRight);
	m_listview_offense->addColumn("Miss");
	m_listview_offense->setColumnAlignment(2, Qt::AlignRight);
	m_listview_offense->addColumn("Accuracy");
	m_listview_offense->setColumnAlignment(3, Qt::AlignRight);
	m_listview_offense->addColumn("Avg");
	m_listview_offense->setColumnAlignment(4, Qt::AlignRight);
	m_listview_offense->addColumn(QString::fromUtf8("SD (\u03c3)"));
	m_listview_offense->setColumnAlignment(5, Qt::AlignRight);
	m_listview_offense->addColumn("Min");
	m_listview_offense->setColumnAlignment(6, Qt::AlignRight);
	m_listview_offense->addColumn("Max");
	m_listview_offense->setColumnAlignment(7, Qt::AlignRight);
	m_listview_offense->addColumn("Total");
	m_listview_offense->setColumnAlignment(8, Qt::AlignRight);

	m_listview_offense->restoreColumns();

	//m_listview_offense->setMinimumSize(m_listview_offense->sizeHint().width(), 200);

	QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
	m_layout_offense->addWidget(summaryGBox);

	QGrid *summaryGrid = new QGrid(11, summaryGBox);

	new QLabel("% from Melee:", summaryGrid);
	m_label_offense_percentmelee = new QLabel(summaryGrid);
	m_label_offense_percentmelee->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Melee:", summaryGrid);
	m_label_offense_avgmelee = new QLabel(summaryGrid);
	m_label_offense_avgmelee->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% from Pet Melee:", summaryGrid);
	m_label_offense_percentpetmelee = new QLabel(summaryGrid);
	m_label_offense_percentpetmelee->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Pet Melee:", summaryGrid);
	m_label_offense_avgpetmelee = new QLabel(summaryGrid);
	m_label_offense_avgpetmelee->setAlignment(Qt::AlignRight);

	new QLabel("% from Special:", summaryGrid);
	m_label_offense_percentspecial = new QLabel(summaryGrid);
	m_label_offense_percentspecial->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Special:", summaryGrid);
	m_label_offense_avgspecial = new QLabel(summaryGrid);
	m_label_offense_avgspecial->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% from Pet Special:", summaryGrid);
	m_label_offense_percentpetspecial = new QLabel(summaryGrid);
	m_label_offense_percentpetspecial->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Pet Special:", summaryGrid);
	m_label_offense_avgpetspecial = new QLabel(summaryGrid);
	m_label_offense_avgpetspecial->setAlignment(Qt::AlignRight);

	new QLabel("% from NonMelee:", summaryGrid);
	m_label_offense_percentnonmelee = new QLabel(summaryGrid);
	m_label_offense_percentnonmelee->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg NonMelee:", summaryGrid);
	m_label_offense_avgnonmelee = new QLabel(summaryGrid);
	m_label_offense_avgnonmelee->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% from Pet NonMelee:", summaryGrid);
	m_label_offense_percentpetnonmelee = new QLabel(summaryGrid);
	m_label_offense_percentpetnonmelee->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Pet NonMelee:", summaryGrid);
	m_label_offense_avgpetnonmelee = new QLabel(summaryGrid);
	m_label_offense_avgpetnonmelee->setAlignment(Qt::AlignRight);


	new QLabel("% from DS:", summaryGrid);
	m_label_offense_percentds = new QLabel(summaryGrid);
	m_label_offense_percentds->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg DS:", summaryGrid);
	m_label_offense_avgds = new QLabel(summaryGrid);
	m_label_offense_avgds->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% from Pet DS:", summaryGrid);
	m_label_offense_percentpetds = new QLabel(summaryGrid);
	m_label_offense_percentpetds->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Pet DS:", summaryGrid);
	m_label_offense_avgpetds = new QLabel(summaryGrid);
	m_label_offense_avgpetds->setAlignment(Qt::AlignRight);


	new QLabel("% from DoT:", summaryGrid);
	m_label_offense_percentdot = new QLabel(summaryGrid);
	m_label_offense_percentdot->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg DoT Tick:", summaryGrid);
	m_label_offense_avgdottick = new QLabel(summaryGrid);
	m_label_offense_avgdottick->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("", summaryGrid);
	new QLabel("", summaryGrid);
	new QLabel("", summaryGrid);
	new QLabel("", summaryGrid);
	new QLabel("", summaryGrid);

	new QLabel("Total Damage:", summaryGrid);
	m_label_offense_totaldamage = new QLabel(summaryGrid);
	m_label_offense_totaldamage->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Current DPS:", summaryGrid);
	m_label_offense_currentdps= new QLabel(summaryGrid);
	m_label_offense_currentdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% Total from Pet:", summaryGrid);
	m_label_offense_percentpettotaldamage = new QLabel(summaryGrid);
	m_label_offense_percentpettotaldamage->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Current Pet DPS:", summaryGrid);
	m_label_offense_currentpetdps= new QLabel(summaryGrid);
	m_label_offense_currentpetdps->setAlignment(Qt::AlignRight);

	((QGridLayout *)summaryGrid->layout())->setColStretch(2, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(5, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(8, 1);
	summaryGrid->layout()->setSpacing(5);

	return pWidget;
}

QWidget* CombatWindow::initDefenseWidget()
{
	QWidget *pWidget = new QWidget(m_tab);
	m_layout_defense = new QVBoxLayout(pWidget);
	QWidget *top_third_widget = new QWidget(pWidget);
	QHBoxLayout* top_third_layout = new QHBoxLayout(top_third_widget);
	m_layout_defense->addWidget(top_third_widget);

	QGroupBox *avoidanceGBox = new QVGroupBox("Avoidance", top_third_widget);
	top_third_layout->addWidget(avoidanceGBox);

	QGrid *avoidanceGrid = new QGrid(6, avoidanceGBox);

	new QLabel("Misses:", avoidanceGrid);
	m_label_defense_avoid_misses = new QLabel(avoidanceGrid);
	m_label_defense_avoid_misses->setAlignment(Qt::AlignRight);
	new QLabel("", avoidanceGrid);
	new QLabel("Blocks:", avoidanceGrid);
	m_label_defense_avoid_block = new QLabel(avoidanceGrid);
	m_label_defense_avoid_block->setAlignment(Qt::AlignRight);
	new QLabel("", avoidanceGrid);

	new QLabel("Parries:", avoidanceGrid);
	m_label_defense_avoid_parry = new QLabel(avoidanceGrid);
	m_label_defense_avoid_parry->setAlignment(Qt::AlignRight);
	new QLabel("", avoidanceGrid);
	new QLabel("Ripostes:", avoidanceGrid);
	m_label_defense_avoid_riposte = new QLabel(avoidanceGrid);
	m_label_defense_avoid_riposte->setAlignment(Qt::AlignRight);
	new QLabel("", avoidanceGrid);

	new QLabel("Dodges:", avoidanceGrid);
	m_label_defense_avoid_dodge = new QLabel(avoidanceGrid);
	m_label_defense_avoid_dodge->setAlignment(Qt::AlignRight);
	new QLabel("", avoidanceGrid);
	new QLabel("", avoidanceGrid);
	new QLabel("", avoidanceGrid);
	new QLabel("", avoidanceGrid);

	new QLabel("Total:", avoidanceGrid);
	m_label_defense_avoid_total = new QLabel(avoidanceGrid);
	m_label_defense_avoid_total->setAlignment(Qt::AlignRight);

	((QGridLayout *)avoidanceGrid->layout())->setColStretch(2, 1);
	((QGridLayout *)avoidanceGrid->layout())->setColStretch(5, 1);
	avoidanceGrid->layout()->setSpacing(5);


	QGroupBox *preventionGBox = new QVGroupBox("Prevention", top_third_widget);
	top_third_layout->addWidget(preventionGBox);

	QGrid *preventionGrid = new QGrid(3, preventionGBox);
	new QLabel("Invulnerables:", preventionGrid);
	m_label_defense_prevented_invulnerables = new QLabel(preventionGrid);
	m_label_defense_prevented_invulnerables->setAlignment(Qt::AlignRight);
	new QLabel("", preventionGrid);

	new QLabel("Absorbs:", preventionGrid);
	m_label_defense_prevented_shield_absorb = new QLabel(preventionGrid);
	m_label_defense_prevented_shield_absorb->setAlignment(Qt::AlignRight);
	new QLabel("", preventionGrid);

	new QLabel("", preventionGrid);
	new QLabel("", preventionGrid);
	new QLabel("", preventionGrid);

	new QLabel("Total:", preventionGrid);
	m_label_defense_prevented_total = new QLabel(preventionGrid);
	m_label_defense_prevented_total->setAlignment(Qt::AlignRight);

	((QGridLayout *)preventionGrid->layout())->setColStretch(2, 1);
	preventionGrid->layout()->setSpacing(5);


	QGroupBox *mitigationGBox = new QVGroupBox("Mitigation", pWidget);
	m_layout_defense->addWidget(mitigationGBox);

	QGrid *mitigationGrid = new QGrid(18, mitigationGBox);

	new QLabel("Melee:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_melee_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_melee_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_melee_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_melee_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_melee_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_melee_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_melee_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_melee_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_melee_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_melee_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_melee_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_melee_min->setAlignment(Qt::AlignRight);

	new QLabel("Special:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_special_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_special_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_special_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_special_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_special_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_special_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_special_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_special_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_special_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_special_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_special_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_special_min->setAlignment(Qt::AlignRight);

	new QLabel("Backstab:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_backstab_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_backstab_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_backstab_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_backstab_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_backstab_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_backstab_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_backstab_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_backstab_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_backstab_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_backstab_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_backstab_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_backstab_min->setAlignment(Qt::AlignRight);

	new QLabel("Monk:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_monk_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_monk_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_monk_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_monk_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_monk_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_monk_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_monk_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_monk_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_monk_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_monk_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_monk_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_monk_min->setAlignment(Qt::AlignRight);

	new QLabel("Ranged:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_ranged_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_ranged_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_ranged_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_ranged_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_ranged_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_ranged_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_ranged_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_ranged_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_ranged_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_ranged_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_ranged_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_ranged_min->setAlignment(Qt::AlignRight);

	new QLabel("Damage Shield:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_damageshield_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_damageshield_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_damageshield_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_damageshield_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_damageshield_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_damageshield_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_damageshield_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_damageshield_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_damageshield_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_damageshield_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_damageshield_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_damageshield_min->setAlignment(Qt::AlignRight);

	new QLabel("Non-melee:", mitigationGrid);
	new QLabel("% of Total:", mitigationGrid);
	m_label_defense_mitigate_nonmelee_percent = new QLabel(mitigationGrid);
	m_label_defense_mitigate_nonmelee_percent->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg:", mitigationGrid);
	m_label_defense_mitigate_nonmelee_avg = new QLabel(mitigationGrid);
	m_label_defense_mitigate_nonmelee_avg->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
	m_label_defense_mitigate_nonmelee_sd = new QLabel(mitigationGrid);
	m_label_defense_mitigate_nonmelee_sd->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Count:", mitigationGrid);
	m_label_defense_mitigate_nonmelee_count = new QLabel(mitigationGrid);
	m_label_defense_mitigate_nonmelee_count->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_nonmelee_max = new QLabel(mitigationGrid);
	m_label_defense_mitigate_nonmelee_max->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_nonmelee_min = new QLabel(mitigationGrid);
	m_label_defense_mitigate_nonmelee_min->setAlignment(Qt::AlignRight);


	((QGridLayout *)mitigationGrid->layout())->setColStretch(3, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(6, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(9, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(12, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(15, 1);
	mitigationGrid->layout()->setSpacing(5);


	QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
	m_layout_defense->addWidget(summaryGBox);

	QGrid *summaryGrid = new QGrid(6, summaryGBox);

	new QLabel("Mob Hits:", summaryGrid);
	m_label_defense_summary_mobhits = new QLabel(summaryGrid);
	m_label_defense_summary_mobhits->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% Avoided:", summaryGrid);
	m_label_defense_summary_percentavoided = new QLabel(summaryGrid);
	m_label_defense_summary_percentavoided->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);

	new QLabel("Mob Attacks:", summaryGrid);
	m_label_defense_summary_mobattacks = new QLabel(summaryGrid);
	m_label_defense_summary_mobattacks->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("% Prevented:", summaryGrid);
	m_label_defense_summary_percentprevented = new QLabel(summaryGrid);
	m_label_defense_summary_percentprevented->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);

	new QLabel("Total Damage:", summaryGrid);
	m_label_defense_summary_totaldamage = new QLabel(summaryGrid);
	m_label_defense_summary_totaldamage->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Current Mob DPS:", summaryGrid);
	m_label_defense_currentmobdps = new QLabel(summaryGrid);
	m_label_defense_currentmobdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);

	((QGridLayout *)summaryGrid->layout())->setColStretch(2, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(5, 1);
	summaryGrid->layout()->setSpacing(5);

	return pWidget;
}

QWidget* CombatWindow::initPetDefenseWidget()
{
    QWidget *pWidget = new QWidget(m_tab);

    m_layout_pet_defense = new QVBoxLayout(pWidget);
    m_combobox_pet_defense = new QComboBox(false, pWidget);
    m_layout_pet_defense->addWidget(m_combobox_pet_defense);
    connect(m_combobox_pet_defense, SIGNAL(activated(const QString&)),
            this, SLOT(petDefenseComboboxSelectionChanged(const QString&)));

    QWidget *top_third_widget = new QWidget(pWidget);
    QHBoxLayout* top_third_layout = new QHBoxLayout(top_third_widget);
    m_layout_pet_defense->addWidget(top_third_widget);

    QGroupBox *avoidanceGBox = new QVGroupBox("Avoidance", top_third_widget);
    top_third_layout->addWidget(avoidanceGBox);

    QGrid *avoidanceGrid = new QGrid(6, avoidanceGBox);

    new QLabel("Misses:", avoidanceGrid);
    m_label_pet_defense_avoid_misses = new QLabel(avoidanceGrid);
    m_label_pet_defense_avoid_misses->setAlignment(Qt::AlignRight);
    new QLabel("", avoidanceGrid);
    new QLabel("Blocks:", avoidanceGrid);
    m_label_pet_defense_avoid_block = new QLabel(avoidanceGrid);
    m_label_pet_defense_avoid_block->setAlignment(Qt::AlignRight);
    new QLabel("", avoidanceGrid);

    new QLabel("Parries:", avoidanceGrid);
    m_label_pet_defense_avoid_parry = new QLabel(avoidanceGrid);
    m_label_pet_defense_avoid_parry->setAlignment(Qt::AlignRight);
    new QLabel("", avoidanceGrid);
    new QLabel("Ripostes:", avoidanceGrid);
    m_label_pet_defense_avoid_riposte = new QLabel(avoidanceGrid);
    m_label_pet_defense_avoid_riposte->setAlignment(Qt::AlignRight);
    new QLabel("", avoidanceGrid);

    new QLabel("Dodges:", avoidanceGrid);
    m_label_pet_defense_avoid_dodge = new QLabel(avoidanceGrid);
    m_label_pet_defense_avoid_dodge->setAlignment(Qt::AlignRight);
    new QLabel("", avoidanceGrid);
    new QLabel("", avoidanceGrid);
    new QLabel("", avoidanceGrid);
    new QLabel("", avoidanceGrid);

    new QLabel("Total:", avoidanceGrid);
    m_label_pet_defense_avoid_total = new QLabel(avoidanceGrid);
    m_label_pet_defense_avoid_total->setAlignment(Qt::AlignRight);

    ((QGridLayout *)avoidanceGrid->layout())->setColStretch(2, 1);
    ((QGridLayout *)avoidanceGrid->layout())->setColStretch(5, 1);
    avoidanceGrid->layout()->setSpacing(5);


    QGroupBox *preventionGBox = new QVGroupBox("Prevention", top_third_widget);
    top_third_layout->addWidget(preventionGBox);

    QGrid *preventionGrid = new QGrid(3, preventionGBox);
    new QLabel("Invulnerables:", preventionGrid);
    m_label_pet_defense_prevented_invulnerables = new QLabel(preventionGrid);
    m_label_pet_defense_prevented_invulnerables->setAlignment(Qt::AlignRight);
    new QLabel("", preventionGrid);

    new QLabel("Absorbs:", preventionGrid);
    m_label_pet_defense_prevented_shield_absorb = new QLabel(preventionGrid);
    m_label_pet_defense_prevented_shield_absorb->setAlignment(Qt::AlignRight);
    new QLabel("", preventionGrid);

    new QLabel("", preventionGrid);
    new QLabel("", preventionGrid);
    new QLabel("", preventionGrid);

    new QLabel("Total:", preventionGrid);
    m_label_pet_defense_prevented_total = new QLabel(preventionGrid);
    m_label_pet_defense_prevented_total->setAlignment(Qt::AlignRight);

    ((QGridLayout *)preventionGrid->layout())->setColStretch(2, 1);
    preventionGrid->layout()->setSpacing(5);


    QGroupBox *mitigationGBox = new QVGroupBox("Mitigation", pWidget);
    m_layout_pet_defense->addWidget(mitigationGBox);

    QGrid *mitigationGrid = new QGrid(18, mitigationGBox);

    new QLabel("Melee:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_melee_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_melee_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_melee_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_melee_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_melee_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_melee_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_melee_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_melee_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_melee_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_melee_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_melee_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_melee_min->setAlignment(Qt::AlignRight);

    new QLabel("Special:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_special_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_special_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_special_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_special_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_special_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_special_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_special_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_special_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_special_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_special_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_special_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_special_min->setAlignment(Qt::AlignRight);

    new QLabel("Backstab:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_backstab_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_backstab_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_backstab_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_backstab_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_backstab_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_backstab_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_backstab_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_backstab_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_backstab_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_backstab_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_backstab_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_backstab_min->setAlignment(Qt::AlignRight);

    new QLabel("Monk:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_monk_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_monk_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_monk_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_monk_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_monk_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_monk_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_monk_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_monk_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_monk_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_monk_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_monk_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_monk_min->setAlignment(Qt::AlignRight);

    new QLabel("Ranged:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_ranged_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_ranged_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_ranged_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_ranged_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_ranged_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_ranged_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_ranged_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_ranged_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_ranged_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_ranged_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_ranged_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_ranged_min->setAlignment(Qt::AlignRight);

    new QLabel("Damage Shield:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_damageshield_min->setAlignment(Qt::AlignRight);

    new QLabel("Non-melee:", mitigationGrid);
    new QLabel("% of Total:", mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_percent = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_percent->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg:", mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_avg = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_avg->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel(QString::fromUtf8("\u03c3:"), mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_sd = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_sd->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Count:", mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_count = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_count->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_max = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_max->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_min = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_nonmelee_min->setAlignment(Qt::AlignRight);


    ((QGridLayout *)mitigationGrid->layout())->setColStretch(3, 1);
    ((QGridLayout *)mitigationGrid->layout())->setColStretch(6, 1);
    ((QGridLayout *)mitigationGrid->layout())->setColStretch(9, 1);
    ((QGridLayout *)mitigationGrid->layout())->setColStretch(12, 1);
    ((QGridLayout *)mitigationGrid->layout())->setColStretch(15, 1);
    mitigationGrid->layout()->setSpacing(5);


    QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
    m_layout_pet_defense->addWidget(summaryGBox);

    QGrid *summaryGrid = new QGrid(6, summaryGBox);

    new QLabel("Mob Hits:", summaryGrid);
    m_label_pet_defense_summary_mobhits = new QLabel(summaryGrid);
    m_label_pet_defense_summary_mobhits->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);
    new QLabel("% Avoided:", summaryGrid);
    m_label_pet_defense_summary_percentavoided = new QLabel(summaryGrid);
    m_label_pet_defense_summary_percentavoided->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);

    new QLabel("Mob Attacks:", summaryGrid);
    m_label_pet_defense_summary_mobattacks = new QLabel(summaryGrid);
    m_label_pet_defense_summary_mobattacks->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);
    new QLabel("% Prevented:", summaryGrid);
    m_label_pet_defense_summary_percentprevented = new QLabel(summaryGrid);
    m_label_pet_defense_summary_percentprevented->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);

    new QLabel("Total Damage:", summaryGrid);
    m_label_pet_defense_summary_totaldamage = new QLabel(summaryGrid);
    m_label_pet_defense_summary_totaldamage->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);
    new QLabel("Current Mob DPS:", summaryGrid);
    m_label_pet_defense_currentpetmobdps = new QLabel(summaryGrid);
    m_label_pet_defense_currentpetmobdps->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);

    ((QGridLayout *)summaryGrid->layout())->setColStretch(2, 1);
    ((QGridLayout *)summaryGrid->layout())->setColStretch(5, 1);
    summaryGrid->layout()->setSpacing(5);

    return pWidget;
}

QWidget* CombatWindow::initMobWidget()
{
	QWidget *pWidget = new QWidget(m_tab);

	m_layout_mob = new QVBoxLayout(pWidget);

	QGroupBox *listGBox = new QVGroupBox(pWidget);
	m_layout_mob->addWidget(listGBox);

	m_listview_mob = new SEQListView(preferenceName()+"_MobListView", listGBox);
	m_listview_mob->addColumn("Time");
	m_listview_mob->setColumnAlignment(0, Qt::AlignLeft);
	m_listview_mob->addColumn("Name");
	m_listview_mob->setColumnAlignment(1, Qt::AlignLeft);
	m_listview_mob->addColumn("ID");
	m_listview_mob->setColumnAlignment(2, Qt::AlignHCenter);
	m_listview_mob->addColumn("Duration");
	m_listview_mob->setColumnAlignment(3, Qt::AlignRight);
	m_listview_mob->addColumn("Damage to");
	m_listview_mob->setColumnAlignment(4, Qt::AlignRight);
	m_listview_mob->addColumn("DPS");
	m_listview_mob->setColumnAlignment(5, Qt::AlignRight);
	m_listview_mob->addColumn("Damage from");
	m_listview_mob->setColumnAlignment(6, Qt::AlignRight);
	m_listview_mob->addColumn("MOB DPS");
	m_listview_mob->setColumnAlignment(7, Qt::AlignRight);
	m_listview_mob->addColumn("Pet Damage to");
	m_listview_mob->setColumnAlignment(8, Qt::AlignRight);
	m_listview_mob->addColumn("Pet DPS");
	m_listview_mob->setColumnAlignment(9, Qt::AlignRight);
	m_listview_mob->addColumn("Pet Damage from");
	m_listview_mob->setColumnAlignment(10, Qt::AlignRight);
	m_listview_mob->addColumn("Pet MOB DPS");
	m_listview_mob->setColumnAlignment(11, Qt::AlignRight);

	m_listview_mob->setSelectionMode(QListView::Multi);
	m_listview_mob->restoreColumns();

	//m_listview_mob->setMinimumSize(m_listview_mob->sizeHint().width(), 200);

	QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
	m_layout_mob->addWidget(summaryGBox);

	QGrid *summaryGrid = new QGrid(11, summaryGBox);

	new QLabel("Avg DPS:", summaryGrid);
	m_label_mob_avgdps = new QLabel(summaryGrid);
	m_label_mob_avgdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Mob DPS:", summaryGrid);
	m_label_mob_avgmobdps = new QLabel(summaryGrid);
	m_label_mob_avgmobdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Pet DPS:", summaryGrid);
	m_label_mob_avgpetdps = new QLabel(summaryGrid);
	m_label_mob_avgpetdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Avg Pet Mob DPS:", summaryGrid);
	m_label_mob_avgpetmobdps = new QLabel(summaryGrid);
	m_label_mob_avgpetmobdps->setAlignment(Qt::AlignRight);

	new QLabel("Last DPS:", summaryGrid);
	m_label_mob_lastdps = new QLabel(summaryGrid);
	m_label_mob_lastdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Last Mob DPS:", summaryGrid);
	m_label_mob_lastmobdps = new QLabel(summaryGrid);
	m_label_mob_lastmobdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Last Pet DPS:", summaryGrid);
	m_label_mob_lastpetdps = new QLabel(summaryGrid);
	m_label_mob_lastpetdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Last Pet Mob DPS:", summaryGrid);
	m_label_mob_lastpetmobdps = new QLabel(summaryGrid);
	m_label_mob_lastpetmobdps->setAlignment(Qt::AlignRight);

	new QLabel("Current DPS:", summaryGrid);
	m_label_mob_currentdps = new QLabel(summaryGrid);
	m_label_mob_currentdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Current Mob DPS:", summaryGrid);
	m_label_mob_currentmobdps = new QLabel(summaryGrid);
	m_label_mob_currentmobdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Current Pet DPS:", summaryGrid);
	m_label_mob_currentpetdps = new QLabel(summaryGrid);
	m_label_mob_currentpetdps->setAlignment(Qt::AlignRight);
	new QLabel("", summaryGrid);
	new QLabel("Current Pet Mob DPS:", summaryGrid);
	m_label_mob_currentpetmobdps = new QLabel(summaryGrid);
	m_label_mob_currentpetmobdps->setAlignment(Qt::AlignRight);

	new QLabel("Total Mobs:", summaryGrid);
	m_label_mob_totalmobs = new QLabel(summaryGrid);
	m_label_mob_totalmobs->setAlignment(Qt::AlignRight);

	((QGridLayout *)summaryGrid->layout())->setColStretch(2, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(5, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(8, 1);
	summaryGrid->layout()->setSpacing(5);


	return pWidget;
}

QWidget* CombatWindow::initOtherWidget()
{
    QWidget *pWidget = new QWidget(m_tab);

    m_layout_other = new QVBoxLayout(pWidget);

    QGroupBox *listGBox = new QVGroupBox(pWidget);
    m_layout_other->addWidget(listGBox);

    m_listview_other = new SEQListView(preferenceName()+"_OtherListView", listGBox);
    m_listview_other->addColumn("Time");
    m_listview_other->setColumnAlignment(0, Qt::AlignLeft);
    m_listview_other->addColumn("Name");
    m_listview_other->setColumnAlignment(1, Qt::AlignLeft);
    m_listview_other->addColumn("ID");
    m_listview_other->setColumnAlignment(2, Qt::AlignHCenter);
    m_listview_other->addColumn("Defender Name");
    m_listview_other->setColumnAlignment(3, Qt::AlignLeft);
    m_listview_other->addColumn("Defender ID");
    m_listview_other->setColumnAlignment(4, Qt::AlignHCenter);
    m_listview_other->addColumn("Duration");
    m_listview_other->setColumnAlignment(5, Qt::AlignRight);
    m_listview_other->addColumn("Damage");
    m_listview_other->setColumnAlignment(6, Qt::AlignRight);
    m_listview_other->addColumn("DPS");
    m_listview_other->setColumnAlignment(7, Qt::AlignRight);

    m_listview_other->setSelectionMode(QListView::Multi);
    m_listview_other->restoreColumns();

    QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
    m_layout_other->addWidget(summaryGBox);

    QGrid *summaryGrid = new QGrid(5, summaryGBox);

    new QLabel("Total Mobs:", summaryGrid);
    m_label_other_totalmobs = new QLabel(summaryGrid);
    m_label_other_totalmobs->setAlignment(Qt::AlignRight);
    new QLabel("", summaryGrid);
    new QLabel("Avg DPS:", summaryGrid);
    m_label_other_avgdps = new QLabel(summaryGrid);
    m_label_other_avgdps->setAlignment(Qt::AlignRight);

    ((QGridLayout *)summaryGrid->layout())->setColStretch(2, 1);
    ((QGridLayout *)summaryGrid->layout())->setColStretch(5, 1);
    summaryGrid->layout()->setSpacing(5);

    return pWidget;
}

void CombatWindow::savePrefs()
{
  // save the SEQWindow's prefs
  SEQWindow::savePrefs();

  // save the SEQListViews' prefs
  m_listview_other->savePrefs();
  m_listview_mob->savePrefs();
  m_listview_offense->savePrefs();
}

void CombatWindow::updateOffense()
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::updateOffense starting...");
#endif


	QString s_totaldamage;
	QString s_percentpettotaldamage;
	QString s_percentmelee;
	QString s_percentspecial;
	QString s_percentnonmelee;
	QString s_percentds;
	QString s_percentdot;
	QString s_avgmelee;
	QString s_avgspecial;
	QString s_avgnonmelee;
	QString s_avgds;
	QString s_avgdottick;
	QString s_percentpetmelee;
	QString s_percentpetspecial;
	QString s_percentpetnonmelee;
	QString s_percentpetds;
	QString s_avgpetmelee;
	QString s_avgpetspecial;
	QString s_avgpetnonmelee;
	QString s_avgpetds;

	int iTotalDamage = 0;
	int iPetTotalDamage = 0;
	//int iTotalHits = 0;
	double dPercentMelee = 0.0;
	double dPercentSpecial = 0.0;
	double dPercentNonmelee = 0.0;
	double dPercentDS = 0.0;
	double dPercentDot = 0.0;
	double dAvgMelee = 0.0;
	double dAvgSpecial = 0.0;
	double dAvgNonmelee = 0.0;
	double dAvgDS = 0.0;
	double dAvgDotTick = 0.0;

	int iMeleeDamage = 0;
	int iMeleeHits = 0;
	int iSpecialDamage = 0;
	int iSpecialHits = 0;
	int iNonmeleeDamage = 0;
	int iNonmeleeHits = 0;
	int iDSDamage = 0;
	int iDSHits = 0;
	int iDotDamage = 0;
	int iDotTicks = 0;

    double dPetPercentTotalDamage = 0.0;

    double dPetPercentMelee = 0.0;
    double dPetPercentSpecial = 0.0;
    double dPetPercentNonmelee = 0.0;
    double dPetPercentDS = 0.0;
    double dPetAvgMelee = 0.0;
    double dPetAvgSpecial = 0.0;
    double dPetAvgNonmelee = 0.0;
    double dPetAvgDS = 0.0;

    int iPetMeleeDamage = 0;
    int iPetMeleeHits = 0;
    int iPetSpecialDamage = 0;
    int iPetSpecialHits = 0;
    int iPetNonmeleeDamage = 0;
    int iPetNonmeleeHits = 0;
    int iPetDSDamage = 0;
    int iPetDSHits = 0;

	CombatOffenseRecord *pRecord;
	for(pRecord = m_combat_offense_list.first(); pRecord != 0; pRecord = m_combat_offense_list.next())
	{
        pRecord->update(m_listview_offense, m_last_consider_id);

        const int iType = pRecord->getType();
        const int iHits = pRecord->getHits();
        const int iDamage = pRecord->getTotalDamage();

		const DamageCategory category = damageCategory(iType);
		switch(category)
		{
			case DAMAGE_CATEGORY_MELEE:
			{
				iMeleeDamage += iDamage;
				iMeleeHits += iHits;
				break;
			}
			case DAMAGE_CATEGORY_MELEE_RANGED:
			case DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC:
			case DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB:
			case DAMAGE_CATEGORY_MELEE_SPECIAL_MONK:
			{
				iSpecialDamage += iDamage;
				iSpecialHits += iHits;
				break;
			}
			case DAMAGE_CATEGORY_NONMELEE:
			{
				iNonmeleeDamage += iDamage;
				iNonmeleeHits += iHits;
				break;
			}
			case DAMAGE_CATEGORY_DAMAGE_SHIELD:
			{
				iDSDamage += iDamage;
				iDSHits += iHits;
				break;
			}
			case DAMAGE_CATEGORY_ENVIRONMENT:
			case DAMAGE_CATEGORY_OTHER:
			{
				seqWarn("CombatWindow::updateOffense: Unknown damage type. unexpected category=%d", category);
				break;
			}
		}
	}

    PetOffenseRecord *petRecord;
    for(petRecord = m_pet_offense_list.first(); petRecord != 0; petRecord = m_pet_offense_list.next())
    {
        petRecord->update(m_listview_offense, m_last_consider_id);

        const int iType = petRecord->getType();
        const int iHits = petRecord->getHits();
        const int iDamage = petRecord->getTotalDamage();

        const DamageCategory category = damageCategory(iType);
        switch(category)
        {
            case DAMAGE_CATEGORY_MELEE:
                {
                    iPetMeleeDamage += iDamage;
                    iPetMeleeHits += iHits;
                    break;
                }
            case DAMAGE_CATEGORY_MELEE_RANGED:
            case DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC:
            case DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB:
            case DAMAGE_CATEGORY_MELEE_SPECIAL_MONK:
                {
                    iPetSpecialDamage += iDamage;
                    iPetSpecialHits += iHits;
                    break;
                }
            case DAMAGE_CATEGORY_NONMELEE:
                {
                    iPetNonmeleeDamage += iDamage;
                    iPetNonmeleeHits += iHits;
                    break;
                }
            case DAMAGE_CATEGORY_DAMAGE_SHIELD:
                {
                    iPetDSDamage += iDamage;
                    iPetDSHits += iHits;
                    break;
                }
            case DAMAGE_CATEGORY_ENVIRONMENT:
            case DAMAGE_CATEGORY_OTHER:
                {
                    seqWarn("CombatWindow::updateOffense: Unknown pet damage type. unexpected category=%d", category);
                    break;
                }
        }
    }

	DotOffenseRecord *dotRecord;
	for(dotRecord = m_dot_offense_list.first(); dotRecord != 0; dotRecord = m_dot_offense_list.next())
	{
        dotRecord->update(m_listview_offense, m_last_consider_id);

        iDotDamage += dotRecord->getTotalDamage();
        iDotTicks += dotRecord->getHits();
	}

	iPetTotalDamage = iPetMeleeDamage + iPetSpecialDamage + iPetNonmeleeDamage + iPetDSDamage;
	iTotalDamage = iMeleeDamage + iSpecialDamage + iNonmeleeDamage + iDotDamage + iDSDamage
		+ iPetTotalDamage;
	//iTotalHits = iMeleeHits + iSpecialHits + iNonmeleeHits;

	dPetPercentTotalDamage = ((double)iPetTotalDamage / (double)iTotalDamage) * 100.0;

	dPercentMelee = ((double)iMeleeDamage / (double)iTotalDamage) * 100.0;
	dPercentSpecial = ((double)iSpecialDamage / (double)iTotalDamage) * 100.0;
	dPercentNonmelee = ((double)iNonmeleeDamage / (double)iTotalDamage) * 100.0;
	dPercentDS = ((double)iDSDamage / (double)iTotalDamage) * 100.0;
	dPercentDot = ((double)iDotDamage / (double)iTotalDamage) * 100.0;

	dAvgMelee = (double)iMeleeDamage / (double)iMeleeHits;
	dAvgSpecial = (double)iSpecialDamage / (double)iSpecialHits;
	dAvgNonmelee = (double)iNonmeleeDamage / (double)iNonmeleeHits;
	dAvgDS = (double)iDSDamage / (double)iDSHits;
	dAvgDotTick = (double)iDotDamage / (double)iDotTicks;

	dPetPercentMelee = ((double)iPetMeleeDamage / (double)iTotalDamage) * 100.0;
	dPetPercentSpecial = ((double)iPetSpecialDamage / (double)iTotalDamage) * 100.0;
	dPetPercentNonmelee = ((double)iPetNonmeleeDamage / (double)iTotalDamage) * 100.0;
	dPetPercentDS = ((double)iPetDSDamage / (double)iTotalDamage) * 100.0;

	dPetAvgMelee = (double)iPetMeleeDamage / (double)iPetMeleeHits;
	dPetAvgSpecial = (double)iPetSpecialDamage / (double)iPetSpecialHits;
	dPetAvgNonmelee = (double)iPetNonmeleeDamage / (double)iPetNonmeleeHits;
	dPetAvgDS = (double)iPetDSDamage / (double)iPetDSHits;

	s_totaldamage.setNum(iTotalDamage);
	s_percentpettotaldamage = doubleToQString(dPetPercentTotalDamage, 1);

	s_percentmelee = doubleToQString(dPercentMelee, 1);
	s_percentspecial = doubleToQString(dPercentSpecial, 1);
	s_percentnonmelee = doubleToQString(dPercentNonmelee, 1);
	s_percentds = doubleToQString(dPercentDS, 1);
	s_percentdot = doubleToQString(dPercentDot, 1);

	s_avgmelee = doubleToQString(dAvgMelee, 0);
	s_avgspecial = doubleToQString(dAvgSpecial, 0);
	s_avgnonmelee = doubleToQString(dAvgNonmelee, 0);
	s_avgds = doubleToQString(dAvgDS, 0);
	s_avgdottick = doubleToQString(dAvgDotTick, 0);

	s_percentpetmelee = doubleToQString(dPetPercentMelee, 1);
	s_percentpetspecial = doubleToQString(dPetPercentSpecial, 1);
	s_percentpetnonmelee = doubleToQString(dPetPercentNonmelee, 1);
	s_percentpetds = doubleToQString(dPetPercentDS, 1);

	s_avgpetmelee = doubleToQString(dPetAvgMelee, 0);
	s_avgpetspecial = doubleToQString(dPetAvgSpecial, 0);
	s_avgpetnonmelee = doubleToQString(dPetAvgNonmelee, 0);
	s_avgpetds = doubleToQString(dPetAvgDS, 0);

	m_label_offense_totaldamage->setText(s_totaldamage);
	m_label_offense_percentpettotaldamage->setText(s_percentpettotaldamage);

	m_label_offense_percentmelee->setText(s_percentmelee);
	m_label_offense_percentspecial->setText(s_percentspecial);
	m_label_offense_percentnonmelee->setText(s_percentnonmelee);
	m_label_offense_percentds->setText(s_percentds);
	m_label_offense_percentdot->setText(s_percentdot);
	m_label_offense_avgmelee->setText(s_avgmelee);
	m_label_offense_avgspecial->setText(s_avgspecial);
	m_label_offense_avgnonmelee->setText(s_avgnonmelee);
	m_label_offense_avgds->setText(s_avgds);
	m_label_offense_avgdottick->setText(s_avgdottick);

	m_label_offense_percentpetmelee->setText(s_percentpetmelee);
	m_label_offense_percentpetspecial->setText(s_percentpetspecial);
	m_label_offense_percentpetnonmelee->setText(s_percentpetnonmelee);
	m_label_offense_percentpetds->setText(s_percentpetds);
	m_label_offense_avgpetmelee->setText(s_avgpetmelee);
	m_label_offense_avgpetspecial->setText(s_avgpetspecial);
	m_label_offense_avgpetnonmelee->setText(s_avgpetnonmelee);
	m_label_offense_avgpetds->setText(s_avgpetds);


#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::updateOffense finished...");
#endif

}

void CombatWindow::updateDefense()
{
	int iHits = m_combat_defense_record->getHits();
	int iMisses = m_combat_defense_record->getMisses();
	int iBlocks = m_combat_defense_record->getBlocks();
	int iParries = m_combat_defense_record->getParries();
	int iRipostes = m_combat_defense_record->getRipostes();
	int iDodges = m_combat_defense_record->getDodges();
	int iInvulnerables = m_combat_defense_record->getInvulnerables();
	int iShieldAbsorbs = m_combat_defense_record->getShieldAbsorbs();
	int iTotalDamage = m_combat_defense_record->getTotalDamage();
	int iTotalAvoid = iMisses+iBlocks+iParries+iRipostes+iDodges;
	int iTotalPrevented = iInvulnerables+iShieldAbsorbs;

	const int iMeleeHits = m_combat_defense_record->getMeleeHits();
	const int iMeleeTotalDamage = m_combat_defense_record->getMeleeTotalDamage();
	const int iMeleeMinDamage = m_combat_defense_record->getMeleeMinDamage();
	const int iMeleeMaxDamage = m_combat_defense_record->getMeleeMaxDamage();
	const double dMeleeSD = m_combat_defense_record->getMeleeStandardDeviation();

	const int iRangedHits = m_combat_defense_record->getRangedHits();
	const int iRangedTotalDamage = m_combat_defense_record->getRangedTotalDamage();
	const int iRangedMinDamage = m_combat_defense_record->getRangedMinDamage();
	const int iRangedMaxDamage = m_combat_defense_record->getRangedMaxDamage();
	const double dRangedSD = m_combat_defense_record->getRangedStandardDeviation();

	const int iSpecialHits = m_combat_defense_record->getSpecialHits();
	const int iSpecialTotalDamage = m_combat_defense_record->getSpecialTotalDamage();
	const int iSpecialMinDamage = m_combat_defense_record->getSpecialMinDamage();
	const int iSpecialMaxDamage = m_combat_defense_record->getSpecialMaxDamage();
	const double dSpecialSD = m_combat_defense_record->getSpecialStandardDeviation();

	const int iBackstabHits = m_combat_defense_record->getBackstabHits();
	const int iBackstabTotalDamage = m_combat_defense_record->getBackstabTotalDamage();
	const int iBackstabMinDamage = m_combat_defense_record->getBackstabMinDamage();
	const int iBackstabMaxDamage = m_combat_defense_record->getBackstabMaxDamage();
	const double dBackstabSD = m_combat_defense_record->getBackstabStandardDeviation();

	const int iMonkHits = m_combat_defense_record->getMonkHits();
	const int iMonkTotalDamage = m_combat_defense_record->getMonkTotalDamage();
	const int iMonkMinDamage = m_combat_defense_record->getMonkMinDamage();
	const int iMonkMaxDamage = m_combat_defense_record->getMonkMaxDamage();
	const double dMonkSD = m_combat_defense_record->getMonkStandardDeviation();

	const int iNonmeleeHits = m_combat_defense_record->getNonmeleeHits();
	const int iNonmeleeTotalDamage = m_combat_defense_record->getNonmeleeTotalDamage();
	const int iNonmeleeMinDamage = m_combat_defense_record->getNonmeleeMinDamage();
	const int iNonmeleeMaxDamage = m_combat_defense_record->getNonmeleeMaxDamage();
	const double dNonmeleeSD = m_combat_defense_record->getNonmeleeStandardDeviation();

	const int iDamageShieldHits = m_combat_defense_record->getDamageShieldHits();
	const int iDamageShieldTotalDamage = m_combat_defense_record->getDamageShieldTotalDamage();
	const int iDamageShieldMinDamage = m_combat_defense_record->getDamageShieldMinDamage();
	const int iDamageShieldMaxDamage = m_combat_defense_record->getDamageShieldMaxDamage();
	const double dDamageShieldSD = m_combat_defense_record->getDamageShieldStandardDeviation();

	const double dMeleePercent = (double)iMeleeTotalDamage / (double)iTotalDamage * 100.0;
	const double dMeleeAvg = (double)iMeleeTotalDamage / (double)iMeleeHits;
	const double dRangedPercent = (double)iRangedTotalDamage / (double)iTotalDamage * 100.0;
	const double dRangedAvg = (double)iRangedTotalDamage / (double)iRangedHits;
	const double dSpecialPercent = (double)iSpecialTotalDamage / (double)iTotalDamage * 100.0;
	const double dSpecialAvg = (double)iSpecialTotalDamage / (double)iSpecialHits;
	const double dBackstabPercent = (double)iBackstabTotalDamage / (double)iTotalDamage * 100.0;
	const double dBackstabAvg = (double)iBackstabTotalDamage / (double)iBackstabHits;
	const double dMonkPercent = (double)iMonkTotalDamage / (double)iTotalDamage * 100.0;
	const double dMonkAvg = (double)iMonkTotalDamage / (double)iMonkHits;
	const double dNonmeleePercent = (double)iNonmeleeTotalDamage / (double)iTotalDamage * 100.0;
	const double dNonmeleeAvg = (double)iNonmeleeTotalDamage / (double)iNonmeleeHits;
	const double dDamageShieldPercent = (double)iDamageShieldTotalDamage / (double)iTotalDamage * 100.0;
	const double dDamageShieldAvg = (double)iDamageShieldTotalDamage / (double)iDamageShieldHits;

	int iMobAttacks = m_combat_defense_record->getTotalAttacks();
	double dAvoided = ((double)iTotalAvoid / (double)iMobAttacks) * 100.0;
	double dPrevented = ((double)iTotalPrevented / (double)iMobAttacks) * 100.0;

	m_label_defense_avoid_misses->setText(QString::number(iMisses));
	m_label_defense_avoid_block->setText(QString::number(iBlocks));
	m_label_defense_avoid_parry->setText(QString::number(iParries));
	m_label_defense_avoid_riposte->setText(QString::number(iRipostes));
	m_label_defense_avoid_dodge->setText(QString::number(iDodges));
	m_label_defense_avoid_total->setText(QString::number(iTotalAvoid));

	m_label_defense_prevented_shield_absorb->setText(QString::number(iShieldAbsorbs));
	m_label_defense_prevented_invulnerables->setText(QString::number(iInvulnerables));
	m_label_defense_prevented_total->setText(QString::number(iTotalPrevented));

	m_label_defense_mitigate_melee_percent->setText(doubleToQString(dMeleePercent, 1));
	m_label_defense_mitigate_melee_avg->setText(doubleToQString(dMeleeAvg, 0));
	m_label_defense_mitigate_melee_min->setText(intToQString(iMeleeMinDamage));
	m_label_defense_mitigate_melee_max->setText(intToQString(iMeleeMaxDamage));
	m_label_defense_mitigate_melee_count->setText(intToQString(iMeleeHits));
	m_label_defense_mitigate_melee_sd->setText(doubleToQString(dMeleeSD, 0, true));

	m_label_defense_mitigate_ranged_percent->setText(doubleToQString(dRangedPercent, 1));
	m_label_defense_mitigate_ranged_avg->setText(doubleToQString(dRangedAvg, 0));
	m_label_defense_mitigate_ranged_min->setText(intToQString(iRangedMinDamage));
	m_label_defense_mitigate_ranged_max->setText(intToQString(iRangedMaxDamage));
	m_label_defense_mitigate_ranged_count->setText(intToQString(iRangedHits));
	m_label_defense_mitigate_ranged_sd->setText(doubleToQString(dRangedSD, 0, true));

	m_label_defense_mitigate_special_percent->setText(doubleToQString(dSpecialPercent, 1));
	m_label_defense_mitigate_special_avg->setText(doubleToQString(dSpecialAvg, 0));
	m_label_defense_mitigate_special_min->setText(intToQString(iSpecialMinDamage));
	m_label_defense_mitigate_special_max->setText(intToQString(iSpecialMaxDamage));
	m_label_defense_mitigate_special_count->setText(intToQString(iSpecialHits));
	m_label_defense_mitigate_special_sd->setText(doubleToQString(dSpecialSD, 0, true));

	m_label_defense_mitigate_backstab_percent->setText(doubleToQString(dBackstabPercent, 1));
	m_label_defense_mitigate_backstab_avg->setText(doubleToQString(dBackstabAvg, 0));
	m_label_defense_mitigate_backstab_min->setText(intToQString(iBackstabMinDamage));
	m_label_defense_mitigate_backstab_max->setText(intToQString(iBackstabMaxDamage));
	m_label_defense_mitigate_backstab_count->setText(intToQString(iBackstabHits));
	m_label_defense_mitigate_backstab_sd->setText(doubleToQString(dBackstabSD, 0, true));

	m_label_defense_mitigate_monk_percent->setText(doubleToQString(dMonkPercent, 1));
	m_label_defense_mitigate_monk_avg->setText(doubleToQString(dMonkAvg, 0));
	m_label_defense_mitigate_monk_min->setText(intToQString(iMonkMinDamage));
	m_label_defense_mitigate_monk_max->setText(intToQString(iMonkMaxDamage));
	m_label_defense_mitigate_monk_count->setText(intToQString(iMonkHits));
	m_label_defense_mitigate_monk_sd->setText(doubleToQString(dMonkSD, 0, true));

	m_label_defense_mitigate_nonmelee_percent->setText(doubleToQString(dNonmeleePercent, 1));
	m_label_defense_mitigate_nonmelee_avg->setText(doubleToQString(dNonmeleeAvg, 0));
	m_label_defense_mitigate_nonmelee_min->setText(intToQString(iNonmeleeMinDamage));
	m_label_defense_mitigate_nonmelee_max->setText(intToQString(iNonmeleeMaxDamage));
	m_label_defense_mitigate_nonmelee_count->setText(intToQString(iNonmeleeHits));
	m_label_defense_mitigate_nonmelee_sd->setText(doubleToQString(dNonmeleeSD, 0, true));

	m_label_defense_mitigate_damageshield_percent->setText(doubleToQString(dDamageShieldPercent, 1));
	m_label_defense_mitigate_damageshield_avg->setText(doubleToQString(dDamageShieldAvg, 0));
	m_label_defense_mitigate_damageshield_min->setText(intToQString(iDamageShieldMinDamage));
	m_label_defense_mitigate_damageshield_max->setText(intToQString(iDamageShieldMaxDamage));
	m_label_defense_mitigate_damageshield_count->setText(intToQString(iDamageShieldHits));
	m_label_defense_mitigate_damageshield_sd->setText(doubleToQString(dDamageShieldSD, 0, true));

	m_label_defense_summary_mobhits->setText(QString::number(iHits));
	m_label_defense_summary_mobattacks->setText(QString::number(iMobAttacks));
	m_label_defense_summary_percentavoided->setText(doubleToQString(dAvoided, 1));
	m_label_defense_summary_percentprevented->setText(doubleToQString(dPrevented, 1));
	m_label_defense_summary_totaldamage->setText(QString::number(iTotalDamage));
}

void CombatWindow::updatePetDefense()
{
    const CombatDefenseRecord *pRecord = m_combat_pet_defense_current_record;
    if(pRecord == NULL)
    {
        m_label_pet_defense_avoid_misses->clear();
        m_label_pet_defense_avoid_block->clear();
        m_label_pet_defense_avoid_parry->clear();
        m_label_pet_defense_avoid_riposte->clear();
        m_label_pet_defense_avoid_dodge->clear();
        m_label_pet_defense_avoid_total->clear();

        m_label_pet_defense_prevented_shield_absorb->clear();
        m_label_pet_defense_prevented_invulnerables->clear();
        m_label_pet_defense_prevented_total->clear();

        m_label_pet_defense_mitigate_melee_percent->clear();
        m_label_pet_defense_mitigate_melee_avg->clear();
        m_label_pet_defense_mitigate_melee_min->clear();
        m_label_pet_defense_mitigate_melee_max->clear();
        m_label_pet_defense_mitigate_melee_count->clear();
        m_label_pet_defense_mitigate_melee_sd->clear();

        m_label_pet_defense_mitigate_ranged_percent->clear();
        m_label_pet_defense_mitigate_ranged_avg->clear();
        m_label_pet_defense_mitigate_ranged_min->clear();
        m_label_pet_defense_mitigate_ranged_max->clear();
        m_label_pet_defense_mitigate_ranged_count->clear();
        m_label_pet_defense_mitigate_ranged_sd->clear();

        m_label_pet_defense_mitigate_special_percent->clear();
        m_label_pet_defense_mitigate_special_avg->clear();
        m_label_pet_defense_mitigate_special_min->clear();
        m_label_pet_defense_mitigate_special_max->clear();
        m_label_pet_defense_mitigate_special_count->clear();
        m_label_pet_defense_mitigate_special_sd->clear();

        m_label_pet_defense_mitigate_backstab_percent->clear();
        m_label_pet_defense_mitigate_backstab_avg->clear();
        m_label_pet_defense_mitigate_backstab_min->clear();
        m_label_pet_defense_mitigate_backstab_max->clear();
        m_label_pet_defense_mitigate_backstab_count->clear();
        m_label_pet_defense_mitigate_backstab_sd->clear();

        m_label_pet_defense_mitigate_monk_percent->clear();
        m_label_pet_defense_mitigate_monk_avg->clear();
        m_label_pet_defense_mitigate_monk_min->clear();
        m_label_pet_defense_mitigate_monk_max->clear();
        m_label_pet_defense_mitigate_monk_count->clear();
        m_label_pet_defense_mitigate_monk_sd->clear();

        m_label_pet_defense_mitigate_nonmelee_percent->clear();
        m_label_pet_defense_mitigate_nonmelee_avg->clear();
        m_label_pet_defense_mitigate_nonmelee_min->clear();
        m_label_pet_defense_mitigate_nonmelee_max->clear();
        m_label_pet_defense_mitigate_nonmelee_count->clear();
        m_label_pet_defense_mitigate_nonmelee_sd->clear();

        m_label_pet_defense_mitigate_damageshield_percent->clear();
        m_label_pet_defense_mitigate_damageshield_avg->clear();
        m_label_pet_defense_mitigate_damageshield_min->clear();
        m_label_pet_defense_mitigate_damageshield_max->clear();
        m_label_pet_defense_mitigate_damageshield_count->clear();
        m_label_pet_defense_mitigate_damageshield_sd->clear();

        m_label_pet_defense_summary_mobhits->clear();
        m_label_pet_defense_summary_mobattacks->clear();
        m_label_pet_defense_summary_percentavoided->clear();
        m_label_pet_defense_summary_percentprevented->clear();
        m_label_pet_defense_summary_totaldamage->clear();
        return;
    }

    const int iHits = pRecord->getHits();
    const int iMisses = pRecord->getMisses();
    const int iBlocks = pRecord->getBlocks();
    const int iParries = pRecord->getParries();
    const int iRipostes = pRecord->getRipostes();
    const int iDodges = pRecord->getDodges();
    const int iInvulnerables = pRecord->getInvulnerables();
    const int iShieldAbsorbs = pRecord->getShieldAbsorbs();
    const int iTotalDamage = pRecord->getTotalDamage();
    const int iTotalAvoid = iMisses+iBlocks+iParries+iRipostes+iDodges;
    const int iTotalPrevented = iInvulnerables+iShieldAbsorbs;

    const int iMeleeHits = pRecord->getMeleeHits();
    const int iMeleeTotalDamage = pRecord->getMeleeTotalDamage();
    const int iMeleeMinDamage = pRecord->getMeleeMinDamage();
    const int iMeleeMaxDamage = pRecord->getMeleeMaxDamage();
    const double dMeleeSD = pRecord->getMeleeStandardDeviation();

    const int iRangedHits = pRecord->getRangedHits();
    const int iRangedTotalDamage = pRecord->getRangedTotalDamage();
    const int iRangedMinDamage = pRecord->getRangedMinDamage();
    const int iRangedMaxDamage = pRecord->getRangedMaxDamage();
    const double dRangedSD = pRecord->getRangedStandardDeviation();

    const int iSpecialHits = pRecord->getSpecialHits();
    const int iSpecialTotalDamage = pRecord->getSpecialTotalDamage();
    const int iSpecialMinDamage = pRecord->getSpecialMinDamage();
    const int iSpecialMaxDamage = pRecord->getSpecialMaxDamage();
    const double dSpecialSD = pRecord->getSpecialStandardDeviation();

    const int iBackstabHits = pRecord->getBackstabHits();
    const int iBackstabTotalDamage = pRecord->getBackstabTotalDamage();
    const int iBackstabMinDamage = pRecord->getBackstabMinDamage();
    const int iBackstabMaxDamage = pRecord->getBackstabMaxDamage();
    const double dBackstabSD = pRecord->getBackstabStandardDeviation();

    const int iMonkHits = pRecord->getMonkHits();
    const int iMonkTotalDamage = pRecord->getMonkTotalDamage();
    const int iMonkMinDamage = pRecord->getMonkMinDamage();
    const int iMonkMaxDamage = pRecord->getMonkMaxDamage();
    const double dMonkSD = pRecord->getMonkStandardDeviation();

    const int iNonmeleeHits = pRecord->getNonmeleeHits();
    const int iNonmeleeTotalDamage = pRecord->getNonmeleeTotalDamage();
    const int iNonmeleeMinDamage = pRecord->getNonmeleeMinDamage();
    const int iNonmeleeMaxDamage = pRecord->getNonmeleeMaxDamage();
    const double dNonmeleeSD = pRecord->getNonmeleeStandardDeviation();

    const int iDamageShieldHits = pRecord->getDamageShieldHits();
    const int iDamageShieldTotalDamage = pRecord->getDamageShieldTotalDamage();
    const int iDamageShieldMinDamage = pRecord->getDamageShieldMinDamage();
    const int iDamageShieldMaxDamage = pRecord->getDamageShieldMaxDamage();
    const double dDamageShieldSD = pRecord->getDamageShieldStandardDeviation();

    const double dMeleePercent = (double)iMeleeTotalDamage / (double)iTotalDamage * 100.0;
    const double dMeleeAvg = (double)iMeleeTotalDamage / (double)iMeleeHits;
    const double dRangedPercent = (double)iRangedTotalDamage / (double)iTotalDamage * 100.0;
    const double dRangedAvg = (double)iRangedTotalDamage / (double)iRangedHits;
    const double dSpecialPercent = (double)iSpecialTotalDamage / (double)iTotalDamage * 100.0;
    const double dSpecialAvg = (double)iSpecialTotalDamage / (double)iSpecialHits;
    const double dBackstabPercent = (double)iBackstabTotalDamage / (double)iTotalDamage * 100.0;
    const double dBackstabAvg = (double)iBackstabTotalDamage / (double)iBackstabHits;
    const double dMonkPercent = (double)iMonkTotalDamage / (double)iTotalDamage * 100.0;
    const double dMonkAvg = (double)iMonkTotalDamage / (double)iMonkHits;
    const double dNonmeleePercent = (double)iNonmeleeTotalDamage / (double)iTotalDamage * 100.0;
    const double dNonmeleeAvg = (double)iNonmeleeTotalDamage / (double)iNonmeleeHits;
    const double dDamageShieldPercent = (double)iDamageShieldTotalDamage / (double)iTotalDamage * 100.0;
    const double dDamageShieldAvg = (double)iDamageShieldTotalDamage / (double)iDamageShieldHits;

    const int iMobAttacks = pRecord->getTotalAttacks();
    const double dAvoided = ((double)iTotalAvoid / (double)iMobAttacks) * 100.0;
    const double dPrevented = ((double)iTotalPrevented / (double)iMobAttacks) * 100.0;

    m_label_pet_defense_avoid_misses->setText(QString::number(iMisses));
    m_label_pet_defense_avoid_block->setText(QString::number(iBlocks));
    m_label_pet_defense_avoid_parry->setText(QString::number(iParries));
    m_label_pet_defense_avoid_riposte->setText(QString::number(iRipostes));
    m_label_pet_defense_avoid_dodge->setText(QString::number(iDodges));
    m_label_pet_defense_avoid_total->setText(QString::number(iTotalAvoid));

    m_label_pet_defense_prevented_shield_absorb->setText(QString::number(iShieldAbsorbs));
    m_label_pet_defense_prevented_invulnerables->setText(QString::number(iInvulnerables));
    m_label_pet_defense_prevented_total->setText(QString::number(iTotalPrevented));

    m_label_pet_defense_mitigate_melee_percent->setText(doubleToQString(dMeleePercent, 1));
    m_label_pet_defense_mitigate_melee_avg->setText(doubleToQString(dMeleeAvg, 0));
    m_label_pet_defense_mitigate_melee_min->setText(intToQString(iMeleeMinDamage));
    m_label_pet_defense_mitigate_melee_max->setText(intToQString(iMeleeMaxDamage));
    m_label_pet_defense_mitigate_melee_count->setText(intToQString(iMeleeHits));
    m_label_pet_defense_mitigate_melee_sd->setText(doubleToQString(dMeleeSD, 0, true));

    m_label_pet_defense_mitigate_ranged_percent->setText(doubleToQString(dRangedPercent, 1));
    m_label_pet_defense_mitigate_ranged_avg->setText(doubleToQString(dRangedAvg, 0));
    m_label_pet_defense_mitigate_ranged_min->setText(intToQString(iRangedMinDamage));
    m_label_pet_defense_mitigate_ranged_max->setText(intToQString(iRangedMaxDamage));
    m_label_pet_defense_mitigate_ranged_count->setText(intToQString(iRangedHits));
    m_label_pet_defense_mitigate_ranged_sd->setText(doubleToQString(dRangedSD, 0, true));

    m_label_pet_defense_mitigate_special_percent->setText(doubleToQString(dSpecialPercent, 1));
    m_label_pet_defense_mitigate_special_avg->setText(doubleToQString(dSpecialAvg, 0));
    m_label_pet_defense_mitigate_special_min->setText(intToQString(iSpecialMinDamage));
    m_label_pet_defense_mitigate_special_max->setText(intToQString(iSpecialMaxDamage));
    m_label_pet_defense_mitigate_special_count->setText(intToQString(iSpecialHits));
    m_label_pet_defense_mitigate_special_sd->setText(doubleToQString(dSpecialSD, 0, true));

    m_label_pet_defense_mitigate_backstab_percent->setText(doubleToQString(dBackstabPercent, 1));
    m_label_pet_defense_mitigate_backstab_avg->setText(doubleToQString(dBackstabAvg, 0));
    m_label_pet_defense_mitigate_backstab_min->setText(intToQString(iBackstabMinDamage));
    m_label_pet_defense_mitigate_backstab_max->setText(intToQString(iBackstabMaxDamage));
    m_label_pet_defense_mitigate_backstab_count->setText(intToQString(iBackstabHits));
    m_label_pet_defense_mitigate_backstab_sd->setText(doubleToQString(dBackstabSD, 0, true));

    m_label_pet_defense_mitigate_monk_percent->setText(doubleToQString(dMonkPercent, 1));
    m_label_pet_defense_mitigate_monk_avg->setText(doubleToQString(dMonkAvg, 0));
    m_label_pet_defense_mitigate_monk_min->setText(intToQString(iMonkMinDamage));
    m_label_pet_defense_mitigate_monk_max->setText(intToQString(iMonkMaxDamage));
    m_label_pet_defense_mitigate_monk_count->setText(intToQString(iMonkHits));
    m_label_pet_defense_mitigate_monk_sd->setText(doubleToQString(dMonkSD, 0, true));

    m_label_pet_defense_mitigate_nonmelee_percent->setText(doubleToQString(dNonmeleePercent, 1));
    m_label_pet_defense_mitigate_nonmelee_avg->setText(doubleToQString(dNonmeleeAvg, 0));
    m_label_pet_defense_mitigate_nonmelee_min->setText(intToQString(iNonmeleeMinDamage));
    m_label_pet_defense_mitigate_nonmelee_max->setText(intToQString(iNonmeleeMaxDamage));
    m_label_pet_defense_mitigate_nonmelee_count->setText(intToQString(iNonmeleeHits));
    m_label_pet_defense_mitigate_nonmelee_sd->setText(doubleToQString(dNonmeleeSD, 0, true));

    m_label_pet_defense_mitigate_damageshield_percent->setText(doubleToQString(dDamageShieldPercent, 1));
    m_label_pet_defense_mitigate_damageshield_avg->setText(doubleToQString(dDamageShieldAvg, 0));
    m_label_pet_defense_mitigate_damageshield_min->setText(intToQString(iDamageShieldMinDamage));
    m_label_pet_defense_mitigate_damageshield_max->setText(intToQString(iDamageShieldMaxDamage));
    m_label_pet_defense_mitigate_damageshield_count->setText(intToQString(iDamageShieldHits));
    m_label_pet_defense_mitigate_damageshield_sd->setText(doubleToQString(dDamageShieldSD, 0, true));

    m_label_pet_defense_summary_mobhits->setText(QString::number(iHits));
    m_label_pet_defense_summary_mobattacks->setText(QString::number(iMobAttacks));
    m_label_pet_defense_summary_percentavoided->setText(doubleToQString(dAvoided, 1));
    m_label_pet_defense_summary_percentprevented->setText(doubleToQString(dPrevented, 1));
    m_label_pet_defense_summary_totaldamage->setText(QString::number(iTotalDamage));
}

void CombatWindow::updateMob()
{
	double dDPSSum = 0.0;
	double dPetDPSSum = 0.0;
	double dMobDPSSum = 0.0;
	double dPetMobDPSSum = 0.0;

	CombatMobRecord *pRecord;

	for(pRecord = m_combat_mob_list.first(); pRecord != 0; pRecord = m_combat_mob_list.next())
	{
        pRecord->update(m_listview_mob, m_last_consider_id);

        dDPSSum += pRecord->getDPS();
        dPetDPSSum += pRecord->getPetDPS();
        dMobDPSSum += pRecord->getMobDPS();
        dPetMobDPSSum += pRecord->getPetMobDPS();
	}

    const int iTotalMobs = m_combat_mob_list.count();

    const double dAvgDPS = dDPSSum / (double)iTotalMobs;
    const double dAvgMobDPS = dMobDPSSum / (double)iTotalMobs;
    const double dAvgPetDPS = dPetDPSSum / (double)iTotalMobs;
    const double dAvgPetMobDPS = dPetMobDPSSum / (double)iTotalMobs;

	m_label_mob_totalmobs->setText(QString::number(iTotalMobs));
	m_label_mob_avgdps->setText(doubleToQString(dAvgDPS, 1));
	m_label_mob_avgmobdps->setText(doubleToQString(dAvgMobDPS, 1));
	m_label_mob_avgpetdps->setText(doubleToQString(dAvgPetDPS, 1));
	m_label_mob_avgpetmobdps->setText(doubleToQString(dAvgPetMobDPS, 1));
}

void CombatWindow::updateOther()
{
    double dDPSSum = 0.0;

    CombatOtherRecord *pRecord;
    for(pRecord = m_combat_other_list.first(); pRecord != 0;
            pRecord = m_combat_other_list.next())
    {
        pRecord->update(m_listview_other, m_last_consider_id);
        dDPSSum += pRecord->getDPS();
    }

    const int iTotalMobs = m_combat_other_list.count();
    const double dAvgDPS = dDPSSum / (double)iTotalMobs;

    m_label_other_totalmobs->setText(QString::number(iTotalMobs));
    m_label_other_avgdps->setText(doubleToQString(dAvgDPS, 1));
}

void CombatWindow::addNonMeleeHit(const QString& iTargetName, const int iDamage)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addNonMeleeHit starting...");
#endif

    if (iTargetName.isEmpty())
        return;

    m_playerspell_target_cleaned_name = iTargetName;
    m_playerspell_damage = iDamage;

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addNonMeleeHit finished...");
#endif
}

void CombatWindow::addDotTick(
        const QString& iTargetName, const QString& iSpellName, const int iDamage)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addDotTickstarting...");
#endif

    if (iSpellName.isEmpty())
        return;

    addDotOffenseRecord(iSpellName, iDamage);
    updateOffense();
    updateDPS(iDamage);

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addDotTickfinished...");
#endif
}

void CombatWindow::addDotOffenseRecord(const QString& iSpellName, const int iDamage)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addDotOffenseRecord starting...");
#endif

    bool bFoundRecord = false;
    DotOffenseRecord *pRecord;
    for(pRecord = m_dot_offense_list.first();
            pRecord != 0; pRecord = m_dot_offense_list.next())
    {
        if(pRecord->getSpellName() == iSpellName)
        {
            bFoundRecord = true;
            break;
        }
    }

    if(!bFoundRecord)
    {
        pRecord = new DotOffenseRecord(m_player, iSpellName);
        m_dot_offense_list.prepend(pRecord);
    }

    pRecord->addHit(iDamage, DAMAGE_CATEGORY_NONMELEE, 0, 0);

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addDotOffenseRecord finished...");
#endif
}

void CombatWindow::addCombatRecord(
        int iTargetID, const Spawn* target,
        int iSourceID, const Spawn* source,
        int iType, int iSpell, int iDamage, bool isKillingBlow,
        uint32_t action_sequence)
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addCombatRecord starting...");
	seqDebug("target=%d, source=%d, type=%d, spell=%d, damage=%d, isKillingBlow=%d, "
            "action_sequence=%u",
			iTargetID, iSourceID, iType, iSpell, iDamage, isKillingBlow,
            action_sequence);
#endif

    const DamageCategory category = damageCategory(iType);
    if (isIgnoredDamageCategory(category))
        return;

    // Damage shields show up as negative damage, so making positive
    if (isDamageShield(category))
        iDamage = -iDamage;

    const int iPlayerID = m_player->id();
    const int iSourcePetOwnerID = (source == NULL) ? -1 : source->petOwnerID();
    const int iTargetPetOwnerID = (target == NULL) ? -1 : target->petOwnerID();
    QString tName;
    if (target != NULL)
        tName = target->name();
    else if (iTargetID == 0)
        tName = "Pain and suffering";
    else // if (target == NULL)
        tName.sprintf("Unknown(%d)", iTargetID);

    QString sName;
    if (source != NULL)
        sName = source->name();
    else if (iSourceID == 0)
        sName = "Pain and suffering";
    else // if (source == NULL)
        sName.sprintf("Unknown(%d)", iSourceID);

#ifdef DEBUGCOMBAT
    if (isDamageShield(category) && iDamage <= 0)
    {
        seqDebug("CombatWindow::addCombatRecord: UNEXPECTED iType (%d,%d,%d) "
                "(%s->%s) "
                "(%d[%d]->%d[%d])",
                iType, iDamage, iSpell,
                (const char*)sName, (const char*)tName,
                iSourceID, iSourcePetOwnerID, iTargetID, iTargetPetOwnerID);
    }
    else if (damageCategory(iType) == DAMAGE_CATEGORY_ENVIRONMENT)
    {
        seqDebug("CombatWindow::addCombatRecord: ENVIRONMENT iType (%d,%d,%d) "
                "(%s->%s) "
                "(%d[%d]->%d[%d])",
                iType, iDamage, iSpell,
                (const char*)sName, (const char*)tName,
                iSourceID, iSourcePetOwnerID, iTargetID, iTargetPetOwnerID);
    }
#endif

	//	The one case we won't handle (for now) is where the Target
	//	and Source are the same.

    int actualPlayerNonmeleeDamage = 0;
    bool shouldRecordPlayerNonMeleeDamage = false;
	if(iTargetID == iPlayerID && iSourceID != iPlayerID)
	{
		if (isMelee(category) || isDamageShield(category) || isNonMeleeDamage(category, iDamage))
		{
			addDefenseRecord(iDamage, category);
			updateDefense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, category, iDamage, tName, sName);
			updateMob();
			updateMobDPS(iDamage);
		}
	}
	else if(iSourceID == iPlayerID && iTargetID != iPlayerID)
	{
		// For the player, non-melee only has positive damage on killing blows
		// (OP_Death), which are captured by isNonMeleeDamage below.
		if (isMelee(category) || isDamageShield(category) || isNonMeleeDamage(category, iDamage))
		{
			addOffenseRecord(iType, category, iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, category, iDamage, tName, sName);
			updateMob();
			updateDPS(iDamage);
		}
        else
        {
            // For the player, non-killing blows (OP_Damage) always have 0 damage,
            // which makes them difficult to distinguish from buffs.
            // isPlayerSpellDamage does the work of determining whether
            // a spell has caused non-melee damage.
            shouldRecordPlayerNonMeleeDamage = isPlayerSpellDamage(
                    category, iSpell, isKillingBlow, iDamage,
                    iTargetID, target, action_sequence,
                    actualPlayerNonmeleeDamage);
            if (shouldRecordPlayerNonMeleeDamage)
            {
                addOffenseRecord(iType, category, actualPlayerNonmeleeDamage, iSpell);
                updateOffense();
                addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID,
                        iSourcePetOwnerID, category, actualPlayerNonmeleeDamage,
                        tName, sName);
                updateMob();
                updateDPS(actualPlayerNonmeleeDamage);
            }
        }
	}
	else if (iPlayerID == iTargetPetOwnerID)
	{
		if (isMelee(category) || isDamageShield(category) || isNonMeleeDamage(category, iDamage))
		{
			addPetDefenseRecord(target, iDamage, category);
			updatePetDefense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, category, iDamage, tName, sName);
			updateMob();
			updatePetMobDPS(iDamage);
		}
	}
	else if (iPlayerID == iSourcePetOwnerID)
	{
		if (isMelee(category) || isDamageShield(category) || isNonMeleeDamage(category, iDamage))
		{
			addPetOffenseRecord(iSourceID, sName, iType, category, iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, category, iDamage, tName, sName);
			updateMob();
			updatePetDPS(iDamage);
		}
	}

	if (iTargetID != iPlayerID || iSourceID != iPlayerID)
	{
		if (isMelee(category) || isDamageShield(category) || isNonMeleeDamage(category, iDamage))
		{
			addOtherRecord(category, iTargetID, iSourceID, iDamage, tName, sName, isKillingBlow);
			updateOther();
		}
        else if (shouldRecordPlayerNonMeleeDamage)
        {
            addOtherRecord(category, iTargetID, iSourceID,
                    actualPlayerNonmeleeDamage, tName, sName, isKillingBlow);
            updateOther();
        }
	}

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addCombatRecord finished...");
#endif
}

void CombatWindow::addOffenseRecord(int iType, DamageCategory category,
        int iDamage, int iSpell)
{

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addOffenseRecord starting...");
#endif

	bool bFoundRecord = false;

	CombatOffenseRecord *pRecord;

	for(pRecord = m_combat_offense_list.first(); pRecord != 0; pRecord = m_combat_offense_list.next())
	{
		if(pRecord->getType() == iType
			&& (isMelee(category) || pRecord->getSpell() == iSpell))
		{
			bFoundRecord = true;
			break;
		}
	}

	if(!bFoundRecord)
	{
		pRecord = new CombatOffenseRecord(iType, m_player, iSpell);
		m_combat_offense_list.prepend(pRecord);
	}

	if(iDamage > 0)
	{
		pRecord->addHit(iDamage, category, 0, 0);
	}
	else if (isMelee(category))
	{
		pRecord->addMiss(iDamage);
	}

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addOffenseRecord finished...");
#endif
}

void CombatWindow::addPetOffenseRecord(int petID, const QString& petName, int iType, DamageCategory category, int iDamage, int iSpell)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addPetOffenseRecord starting...");
#endif

	bool bFoundRecord = false;

	PetOffenseRecord *pRecord;

	for(pRecord = m_pet_offense_list.first(); pRecord != 0; pRecord = m_pet_offense_list.next())
	{
		if(pRecord->getType() == iType && pRecord->getPetID() == petID
			&& (isMelee(category) || pRecord->getSpell() == iSpell))
		{
			bFoundRecord = true;
			break;
		}
	}

	if(!bFoundRecord)
	{
		pRecord = new PetOffenseRecord(petID, petName, iType, m_player, iSpell);
		m_pet_offense_list.prepend(pRecord);
	}

	if(iDamage > 0)
	{
		pRecord->addHit(iDamage, category, 0, 0);
	}
	else if (isMelee(category))
	{
		pRecord->addMiss(iDamage);
	}

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addPetOffenseRecord finished...");
#endif
}

void CombatWindow::addDefenseRecord(int iDamage, DamageCategory category)
{
	if(iDamage > 0)
		m_combat_defense_record->addHit(iDamage, category, 0, 0);
	else
		m_combat_defense_record->addMiss(iDamage);

}

void CombatWindow::addPetDefenseRecord(const Spawn* s, int iDamage, DamageCategory category)
{
    if (!s)
    {
        seqWarn("Failed to add pet defense record for unknown spawn");
        return;
    }

    const QString pet_id_string = CombatDefenseRecord::createRecordIDString(
            s->name(), s->id(), s->classString(), s->level());
    bool bFoundRecord = false;
    CombatDefenseRecord *pRecord;
    for(pRecord = m_combat_pet_defense_list.first(); pRecord != 0; pRecord = m_combat_pet_defense_list.next())
    {
        if(pRecord->displayString() == pet_id_string)
        {
            bFoundRecord = true;
            break;
        }
    }

    if(!bFoundRecord)
    {
        pRecord = new CombatDefenseRecord(s);
        m_combat_pet_defense_list.prepend(pRecord);
        m_combobox_pet_defense->insertItem(pet_id_string, 0);
    }

    if (m_autoupdate_pet_defense_selection || m_combat_pet_defense_current_record == NULL)
    {
        m_autoupdate_pet_defense_selection = false;
        bool foundText = false;
        for (int i = 0; i < m_combobox_pet_defense->count() ; ++i)
        {
            if (m_combobox_pet_defense->text(i) == pet_id_string)
            {
                foundText = true;
                m_combobox_pet_defense->setCurrentItem(i);
                break;
            }
        }
        if (foundText)
            m_combat_pet_defense_current_record = pRecord;
        else
            seqWarn("Combobox item missing for pet defense record. id=%s",
                    (const char*)pet_id_string);
    }

    if(iDamage > 0)
        pRecord->addHit(iDamage, category, 0, 0);
    else
        pRecord->addMiss(iDamage);
}

void CombatWindow::addMobRecord(int iTargetID, int iTargetPetOwnerID,
        int iSourceID, int iSourcePetOwnerID, DamageCategory category,
        int iDamage, const QString& tName, const QString& sName)
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addMobRecord starting...");
#endif

	const int iPlayerID = m_player->id();
	int iMobID;
	QString mobName;

	if(iPlayerID == iTargetID || iPlayerID == iTargetPetOwnerID)
	{
		iMobID = iSourceID;
		mobName = sName;
	}
	else if(iPlayerID == iSourceID || iPlayerID == iSourcePetOwnerID)
	{
		iMobID = iTargetID;
		mobName = tName;
	}
	else
	{
		seqWarn("CombatWindow::addMobRecord: Invalid record");
		return;
	}


	bool bFoundRecord = false;

	CombatMobRecord *pRecord;

	for(pRecord = m_combat_mob_list.first(); pRecord != 0; pRecord = m_combat_mob_list.next())
	{
		if(pRecord->getID() == iMobID)
		{
			bFoundRecord = true;
			break;
		}
	}

	if(!bFoundRecord)
	{
		pRecord = new CombatMobRecord(iMobID, mobName, m_player);
		m_combat_mob_list.prepend(pRecord);
	}
	pRecord->addHit(iDamage, category, iTargetID, iSourceID);

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addMobRecord finished...");
#endif
}

void CombatWindow::addOtherRecord(
        DamageCategory category, int iTargetID, int iSourceID,
        int iDamage, const QString& tName, const QString& sName,
        bool isKillingBlow)
{
    bool bFoundRecord = false;
    CombatOtherRecord *pRecord;
    for (pRecord = m_combat_other_list.first(); pRecord != 0;
            pRecord = m_combat_other_list.next())
    {
        if(pRecord->getSourceID() == iSourceID
                && pRecord->getTargetID() == iTargetID
                && pRecord->getSourceName() == sName
                && pRecord->getTargetName() == tName)
        {
            bFoundRecord = true;
            break;
        }
    }

    if (!bFoundRecord)
    {
        // Ignore killing blows, because death packets (OP_Death) have no
        // range limit while regular hits (OP_Damage) do. Allow OP_Death where
        // the player is the killer, since these are obviously within range of
        // the player.
        if (isKillingBlow && iSourceID != m_player->id())
            return;

        pRecord = new CombatOtherRecord(iTargetID, iSourceID, tName, sName);
        m_combat_other_list.prepend(pRecord);
    }

    pRecord->addHit(iDamage, category, iTargetID, iSourceID);
}

void CombatWindow::updateDPS(int iDamage)
{
    // < 0 damage are misses e.g. dodge, block, etc
    if (iDamage < 0)
        iDamage = 0;

    const int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iDPSTimeLast + 10000))
	{
		//	reset DPS
		m_dDPSLast = m_dDPS;
		m_dDPS = 0.0;
		m_iDPSStartTime = iTimeNow;
		m_iCurrentDPSTotal = 0;
	}

	m_iDPSTimeLast = iTimeNow;
	m_iCurrentDPSTotal += iDamage;

	int iTimeElapsed = (iTimeNow - m_iDPSStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dDPS = (double)m_iCurrentDPSTotal / (double)iTimeElapsed;
	}

	const QString sDPS = doubleToQString(m_dDPS, 1);
	const QString sDPSLast = doubleToQString(m_dDPSLast, 1);

	m_label_offense_currentdps->setText(sDPS);
	m_label_mob_currentdps->setText(sDPS);
	m_label_mob_lastdps->setText(sDPSLast);
}


void CombatWindow::updatePetDPS(int iDamage)
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::updatePetDPS...");
#endif

    // < 0 damage are misses e.g. dodge, block, etc
    if (iDamage < 0)
        iDamage = 0;

    const int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iPetDPSTimeLast + 10000))
	{
		//	reset DPS
		m_dPetDPSLast = m_dPetDPS;
		m_dPetDPS = 0.0;
		m_iPetDPSStartTime = iTimeNow;
		m_iPetCurrentDPSTotal = 0;
	}

	m_iPetDPSTimeLast = iTimeNow;
	m_iPetCurrentDPSTotal += iDamage;

	const int iTimeElapsed = (iTimeNow - m_iPetDPSStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dPetDPS = (double)m_iPetCurrentDPSTotal / (double)iTimeElapsed;
	}

	const QString sPetDPS = doubleToQString(m_dPetDPS, 1);
	const QString sPetDPSLast = doubleToQString(m_dPetDPSLast, 1);

	m_label_offense_currentpetdps->setText(sPetDPS);
	m_label_mob_currentpetdps->setText(sPetDPS);
	m_label_mob_lastpetdps->setText(sPetDPSLast);
}


void CombatWindow::updateMobDPS(int iDamage)
{
    // < 0 damage are misses e.g. dodge, block, etc
    if (iDamage < 0)
        iDamage = 0;

    const int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iMobDPSTimeLast + 10000))
	{
		m_dMobDPSLast = m_dDPS;
		m_dMobDPS = 0.0;
		m_iMobDPSStartTime = iTimeNow;
		m_iCurrentMobDPSTotal = 0;
	}

	m_iMobDPSTimeLast = iTimeNow;
	m_iCurrentMobDPSTotal += iDamage;

	int iTimeElapsed = (iTimeNow - m_iMobDPSStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dMobDPS = (double)m_iCurrentMobDPSTotal / (double)iTimeElapsed;
	}

	const QString sMobDPS = doubleToQString(m_dMobDPS, 1);
	const QString sMobDPSLast = doubleToQString(m_dMobDPSLast, 1);

	m_label_defense_currentmobdps->setText(sMobDPS);
	m_label_mob_currentmobdps->setText(sMobDPS);
	m_label_mob_lastmobdps->setText(sMobDPSLast);
}


void CombatWindow::updatePetMobDPS(int iDamage)
{
    // < 0 damage are misses e.g. dodge, block, etc
    if (iDamage < 0)
        iDamage = 0;

    const int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iPetMobDPSTimeLast + 10000))
	{
		m_dPetMobDPSLast = m_dPetDPS;
		m_dPetMobDPS = 0.0;
		m_iPetMobDPSStartTime = iTimeNow;
		m_iPetCurrentMobDPSTotal = 0;
	}

	m_iPetMobDPSTimeLast = iTimeNow;
	m_iPetCurrentMobDPSTotal += iDamage;

	const int iTimeElapsed = (iTimeNow - m_iPetMobDPSStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dPetMobDPS = (double)m_iPetCurrentMobDPSTotal / (double)iTimeElapsed;
	}

	const QString sPetMobDPS = doubleToQString(m_dPetMobDPS, 1);
	const QString sPetMobDPSLast = doubleToQString(m_dPetMobDPSLast, 1);

	m_label_pet_defense_currentpetmobdps->setText(sPetMobDPS);
	m_label_mob_currentpetmobdps->setText(sPetMobDPS);
	m_label_mob_lastpetmobdps->setText(sPetMobDPSLast);
}


void CombatWindow::resetDPS()
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::resetDPS");
#endif
	//	we'll let updateDPS do all the work
	//	by simply setting m_iDPSTimeLast to 0

	m_iDPSTimeLast = 0;

	updateDPS(0);

	m_iPetDPSTimeLast = 0;

	updatePetDPS(0);

	m_iMobDPSTimeLast = 0;

	updateMobDPS(0);

	m_iPetMobDPSTimeLast = 0;

	updatePetMobDPS(0);
}

void CombatWindow::clearOther()
{
    m_combat_other_list.clear();
    m_listview_other->clear();
    updateOther();
}

void CombatWindow::clearMob()
{
    m_combat_mob_list.clear();
    m_listview_mob->clear();
    updateMob();
}

void CombatWindow::clearOffense()
{
    m_combat_offense_list.clear();
    m_pet_offense_list.clear();
    m_dot_offense_list.clear();
    m_listview_offense->clear();
    updateOffense();
}

void CombatWindow::clearDefense()
{
    m_combat_defense_record->clear();
    updateDefense();
}

void CombatWindow::clearPetDefense()
{
    m_combobox_pet_defense->clear();
    m_combat_pet_defense_current_record = NULL;
    m_combat_pet_defense_list.clear();
    updatePetDefense();
}

void CombatWindow::clear(void)
{
  clearOffense();
  clearDefense();
  clearPetDefense();
  clearOther();
  clearMob();
  resetDPS();
}

void CombatWindow::petDefenseComboboxSelectionChanged(const QString& selected)
{
    bool bFoundRecord = false;
    const CombatDefenseRecord *pRecord;
    for(pRecord = m_combat_pet_defense_list.first(); pRecord != 0; pRecord = m_combat_pet_defense_list.next())
    {
        if(pRecord->displayString() == selected)
        {
            bFoundRecord = true;
            break;
        }
    }

    if(!bFoundRecord)
    {
        seqWarn("Failed to select pet defense record. Unknown selection=%s",
                (const char*)selected);
        return;
    }

    m_combat_pet_defense_current_record = pRecord;
    m_autoupdate_pet_defense_selection = false;
    updatePetDefense();
}

void CombatWindow::charmUpdate(const uint8_t* data)
{
    const Charm_Struct* update = (const Charm_Struct*)data;

    if (update->owner_id == m_player->id() && update->command)
    {
        m_autoupdate_pet_defense_selection = true;
    }
}

void CombatWindow::newSpawn(const uint8_t* data)
{
    const spawnStruct* s = (const spawnStruct*)data;
    if (s->petOwnerId == m_player->id())
    {
        m_autoupdate_pet_defense_selection = true;
    }
}

void CombatWindow::considerSpawn(const uint8_t* data)
{
    const considerStruct* con = (const considerStruct*)data;
    m_last_consider_id = con->targetid;
    selectRecordsById(m_listview_mob, m_combat_mob_list, con->targetid);
    selectRecordsById(m_listview_other, m_combat_other_list, con->targetid);

    // clear all if it's been less than .2 seconds since last consider
    const int iTimeNow = mTime();
    if(m_lastConsider && iTimeNow < (m_lastConsider + 200))
    {
        clear();
    }

    m_lastConsider = iTimeNow;
    resetDPS();
}

void CombatWindow::action(const uint8_t* data)
{
    const actionStruct* a = (const actionStruct*)data;
    // only handle player spell actions
    if (a->type != 0xe7 || a->source != m_player->id())
        return;

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::action(): spell_id=%d (%d->%d) type=%d (lvl: %d) "
            "causing %d damage. sequence=%d. buff=%d. "
            "instrument_mod=%d. bard_focus_id=%d.",
            a->spell, a->source, a->target, a->type, a->level,
            a->damage, a->sequence, a->make_buff_icon,
            a->instrument_mod, a->bard_focus_id);
#endif

    if (a->make_buff_icon == 0)
    {
        // Reset action session by recording action data for later comparison
        resetPlayerSpellActionState();
        m_playerspell_action_sequence = a->sequence;
        m_playerspell_target_id = a->target;
        m_playerspell_spell_id = a->spell;
        return;
    }

    if (a->make_buff_icon != 4)
    {
        seqWarn("CombatWindow::action(): Unexpected make_buff_icon value (%d)",
                a->make_buff_icon);
        return;
    }

    if (m_playerspell_action_sequence != a->sequence ||
            m_playerspell_target_id != a->target ||
            m_playerspell_spell_id != a->spell)
    {
        // Clear saved action state because it doesn't match this confirmation action
        resetPlayerSpellActionState();
        return;
    }

    m_playerspell_confirmed = true;
}

bool CombatWindow::isPlayerSpellDamage(DamageCategory category, int iSpell,
        bool isKillingBlow, int iDamage, int iTargetID, const Spawn* target,
        uint32_t sequence, int& actualPlayerNonmeleeDamage)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::isPlayerSpellDamage(): "
            "confirmed=%d category=%d isKillingBlow=%d iDamage=%d iSpell=%d "
            "m_playerspell_spell_id=%d iTargetID=%d m_playerspell_target_id=%d "
            "sequence=%u m_playerspell_action_sequence=%u "
            "target->cleanedName()=%s m_playerspell_target_cleaned_name=%s ",
            m_playerspell_confirmed, category, isKillingBlow, iDamage, iSpell,
            m_playerspell_spell_id, iTargetID, m_playerspell_target_id,
            sequence, m_playerspell_action_sequence,
            target == NULL ? "NULL" : (const char*)target->cleanedName(),
            (const char*)m_playerspell_target_cleaned_name);
#endif

    if (m_playerspell_confirmed
            && category == DAMAGE_CATEGORY_NONMELEE
            && !isKillingBlow
            && iDamage == 0
            && iSpell == m_playerspell_spell_id
            && iTargetID == m_playerspell_target_id
            && sequence == m_playerspell_action_sequence
            && target != NULL
            && target->cleanedName() == m_playerspell_target_cleaned_name)
    {
#ifdef DEBUGCOMBAT
        seqDebug("   CombatWindow::isPlayerSpellDamage(): TRUE");
#endif
        actualPlayerNonmeleeDamage = m_playerspell_damage;
        resetPlayerSpellActionState();
        return true;
    }

#ifdef DEBUGCOMBAT
    seqDebug("   CombatWindow::isPlayerSpellDamage(): TRUE");
#endif
    resetPlayerSpellActionState();
    return false;
}

void CombatWindow::resetPlayerSpellActionState()
{
    m_playerspell_action_sequence = 0;
    m_playerspell_target_id = 0;
    m_playerspell_spell_id = 0;
    m_playerspell_damage = 0;
    m_playerspell_target_cleaned_name.truncate(0);
    m_playerspell_confirmed = false;
}

#include "combatlog.moc"
