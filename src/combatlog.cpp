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

inline bool isNanOrZero(double d)
{
    // c++98, so using double != double, instead of std::isnan from c++11
    return d == 0.0 || d != d;
}

QString doubleToQString(double d, int precision)
{
    QString result;
    if (!isNanOrZero(d))
    {
        result.setNum(d, 'f', precision);
    }
    return result;
}

QString intToQString(int i)
{
    QString result;
    if (i != 0)
    {
        result.setNum(i);
    }
    return result;
}

enum DamageCategory
{
    DAMAGE_CATEGORY_MELEE,
    DAMAGE_CATEGORY_MELEE_SPECIAL,
    DAMAGE_CATEGORY_NONMELEE,
    DAMAGE_CATEGORY_ENVIRONMENT,
    DAMAGE_CATEGORY_DAMAGE_SHIELD
};

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
        case 8:		// Backstab
        case 10:	// Bash
        case 21:	// Dragon Punch
        case 23:	// Eagle Strike
        case 26:	// Flying Kick
        case 30:	// Kick
        case 38:	// Round Kick
        case 51:	// Throwing
        case 52:	// Tiger Claw
            {
                return DAMAGE_CATEGORY_MELEE_SPECIAL;
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

static bool isNonMeleeDamage(int iType, int iDamage)
{
    // Checking iDamage > 0 avoids buff spells
    return damageCategory(iType) == DAMAGE_CATEGORY_NONMELEE && iDamage > 0;
}

static bool isMelee(int iType)
{
    const DamageCategory c = damageCategory(iType);
    return c == DAMAGE_CATEGORY_MELEE || c == DAMAGE_CATEGORY_MELEE_SPECIAL;
}

static bool isDamageShield(int iType)
{
    return damageCategory(iType) == DAMAGE_CATEGORY_DAMAGE_SHIELD;
}

} // namespace


////////////////////////////////////////////
//  CombatOffenseRecord implementation
////////////////////////////////////////////
CombatOffenseRecord::CombatOffenseRecord( int iType, Player* p, int iSpell) :
	m_iType(iType),
	m_iSpell(iSpell),
	m_player(p),
	m_iHits(0),
	m_iMisses(0),
	m_iMinDamage(0),
	m_iMaxDamage(0),
	m_iTotalDamage(0)
{

}


void CombatOffenseRecord::addHit(int iDamage)
{
	if(iDamage <= 0)
		return;

	m_iHits++;
	m_iTotalDamage += iDamage;

	if(iDamage > 0 && (iDamage < m_iMinDamage || !m_iMinDamage))
		m_iMinDamage = iDamage;

	if(iDamage > m_iMaxDamage)
		m_iMaxDamage = iDamage;

}


////////////////////////////////////////////
//  NonMeleeOffenseRecord implementation
////////////////////////////////////////////
NonMeleeOffenseRecord::NonMeleeOffenseRecord() :
    CombatOffenseRecord(231, NULL, ITEM_SPELLID_NOSPELL) // assume non-melee is spell, so 231
{
}

void NonMeleeOffenseRecord::clear()
{
    m_iHits = 0;
    m_iMisses = 0;
    m_iMinDamage = 0;
    m_iMaxDamage = 0;
    m_iTotalDamage = 0;
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


////////////////////////////////////////////
//  DotOffenseRecord implementation
////////////////////////////////////////////
DotOffenseRecord::DotOffenseRecord(const Player* p, const QString& iSpellName) :
    m_iSpellName(iSpellName),
    m_player(p),
    m_iTicks(0),
    m_iMinDamage(0),
    m_iMaxDamage(0),
    m_iTotalDamage(0)
{
}

void DotOffenseRecord::addTick(int iDamage)
{
    if(iDamage <= 0)
        return;

    m_iTicks++;
    m_iTotalDamage += iDamage;

    if(iDamage > 0 && (iDamage < m_iMinDamage || !m_iMinDamage))
        m_iMinDamage = iDamage;

    if(iDamage > m_iMaxDamage)
        m_iMaxDamage = iDamage;
}


////////////////////////////////////////////
//  CombatDefenseRecord implementation
////////////////////////////////////////////
CombatDefenseRecord::CombatDefenseRecord(const Spawn* s) :
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

void CombatDefenseRecord::clear(void)
{
  m_iHits = 0;
  m_iMisses = 0;
  m_iBlocks = 0;
  m_iParries = 0;
  m_iRipostes = 0;
  m_iDodges = 0;
  m_iInvulnerables= 0;
  m_iShieldAbsorbs = 0;
  m_iMinDamage = 0;
  m_iMaxDamage = 0;
  m_iTotalDamage = 0;
  m_iTotalAttacks = 0;
}

void CombatDefenseRecord::addHit(int iDamage)
{
	if(iDamage <= 0)
		return;

	m_iTotalAttacks++;
	m_iHits++;
	m_iTotalDamage += iDamage;

	if(iDamage > 0 && (iDamage < m_iMinDamage || !m_iMinDamage))
		m_iMinDamage = iDamage;

	if(iDamage > m_iMaxDamage)
		m_iMaxDamage = iDamage;

}

void CombatDefenseRecord::addMiss(int iMissReason)
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
		  seqDebug("CombatDefenseRecord::addMiss: invalid miss reason (%d)", iMissReason);
			break;
		}
	}

}


////////////////////////////////////////////
//	CombatMobRecord implementation
////////////////////////////////////////////
CombatMobRecord::CombatMobRecord(int iID, const QString& mobName, Player* p) :
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

double CombatMobRecord::getDPS()
{
	int iTimeElapsed = (m_iLastTime - m_iStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dDPS = (double)m_iDamageGiven / (double)iTimeElapsed;
	}

	return m_dDPS;
}

double CombatMobRecord::getMobDPS()
{
	int iTimeElapsed = (m_iLastTime - m_iStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dMobDPS = (double)m_iDamageTaken / (double)iTimeElapsed;
	}

	return m_dMobDPS;
}

double CombatMobRecord::getPetDPS()
{
    const int iTimeElapsed = (m_iPetLastTime - m_iPetStartTime) / 1000;

    if (iTimeElapsed > 0)
    {
        m_dPetDPS = (double)m_iPetDamageGiven / (double)iTimeElapsed;
    }

    return m_dPetDPS;
}

double CombatMobRecord::getPetMobDPS()
{
    const int iTimeElapsed = (m_iPetLastTime - m_iPetStartTime) / 1000;

    if (iTimeElapsed > 0)
    {
        m_dPetMobDPS = (double)m_iPetDamageTaken / (double)iTimeElapsed;
    }

    return m_dPetMobDPS;
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

void CombatMobRecord::addHit(int iTarget, int iSource, int iDamage)
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
        }
        else if (hitInvolvesPlayer && m_iID == iSource)
        {
            m_iDamageTaken += iDamage;
        }
        else if (m_iID == iTarget) // Assume pet
        {
            m_iPetDamageGiven += iDamage;
        }
        else // Assume pet
        {
            m_iPetDamageTaken += iDamage;
        }
    }
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
	if(m_nonmelee_offense_record != 0)
	{
		delete m_nonmelee_offense_record;
		m_nonmelee_offense_record = 0;
	}
}

CombatWindow::CombatWindow(Player* player,
			   QWidget* parent, const char* name)
  : SEQWindow("Combat", "ShowEQ - Combat", parent, name),
    m_player(player),
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
    m_dPetMobDPSLast(0.0)
{
  /* Hopefully this is only called once to set up the window,
     so this is a good place to initialize some things which
     otherwise won't be. */

	m_combat_offense_list.setAutoDelete(true);
	m_pet_offense_list.setAutoDelete(true);
	m_dot_offense_list.setAutoDelete(true);
	m_nonmelee_offense_record = new NonMeleeOffenseRecord;
	m_combat_defense_record = new CombatDefenseRecord(player);
	m_combat_pet_defense_current_record = NULL;
	m_combat_pet_defense_list.setAutoDelete(true);
	m_combat_mob_list.setAutoDelete(true);

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
	m_tab->addTab(m_widget_mob, "&Mobs");

	m_clear_menu = new QPopupMenu(this);
	m_clear_menu->insertItem("Clear All", this, SLOT(clear()));
	m_clear_menu->insertItem("Clear Offense Stats", this, SLOT(clearOffense()));
	m_clear_menu->insertItem("Clear Defense Stats", this, SLOT(clearDefense()));
	m_clear_menu->insertItem("Clear Pet Defense Stats", this, SLOT(clearPetDefense()));
	m_clear_menu->insertItem("Clear Mob Stats", this, SLOT(clearMob()));

	m_menu_bar->insertItem("&Clear", m_clear_menu);

	updateOffense();
	updateDefense();
	updatePetDefense();
	updateMob();

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
	m_listview_offense->addColumn("Min");
	m_listview_offense->setColumnAlignment(5, Qt::AlignRight);
	m_listview_offense->addColumn("Max");
	m_listview_offense->setColumnAlignment(6, Qt::AlignRight);
	m_listview_offense->addColumn("Total");
	m_listview_offense->setColumnAlignment(7, Qt::AlignRight);

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

	QGrid *mitigationGrid = new QGrid(6, mitigationGBox);

	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_minhit = new QLabel(mitigationGrid);
	m_label_defense_mitigate_minhit->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);
	new QLabel("Avg. Hit:", mitigationGrid);
	m_label_defense_mitigate_avghit = new QLabel(mitigationGrid);
	m_label_defense_mitigate_avghit->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);

	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_maxhit = new QLabel(mitigationGrid);
	m_label_defense_mitigate_maxhit->setAlignment(Qt::AlignRight);
	new QLabel("", mitigationGrid);

	((QGridLayout *)mitigationGrid->layout())->setColStretch(2, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(5, 1);
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

    QGrid *mitigationGrid = new QGrid(6, mitigationGBox);

    new QLabel("Min:", mitigationGrid);
    m_label_pet_defense_mitigate_minhit = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_minhit->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);
    new QLabel("Avg. Hit:", mitigationGrid);
    m_label_pet_defense_mitigate_avghit = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_avghit->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);

    new QLabel("Max:", mitigationGrid);
    m_label_pet_defense_mitigate_maxhit = new QLabel(mitigationGrid);
    m_label_pet_defense_mitigate_maxhit->setAlignment(Qt::AlignRight);
    new QLabel("", mitigationGrid);

    ((QGridLayout *)mitigationGrid->layout())->setColStretch(2, 1);
    ((QGridLayout *)mitigationGrid->layout())->setColStretch(5, 1);
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
	m_listview_mob->setColumnAlignment(2, Qt::AlignRight);
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

void CombatWindow::savePrefs()
{
  // save the SEQWindow's prefs
  SEQWindow::savePrefs();

  // save the SEQListViews' prefs
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


	//	empty the list so we can repopulate
	m_listview_offense->clear();

    if (0 != m_nonmelee_offense_record->getHits() + m_nonmelee_offense_record->getMisses())
    {
        const int iHits = m_nonmelee_offense_record->getHits();
        const int iMinDamage = m_nonmelee_offense_record->getMinDamage();
        const int iMaxDamage = m_nonmelee_offense_record->getMaxDamage();
        const int iDamage = m_nonmelee_offense_record->getTotalDamage();

        const double dAvgDamage = (double)iDamage / (double)iHits;

        QString s_type = "Non-melee (all sources):";
        QString s_hits;
        s_hits.setNum(iHits);
        QString s_misses;
        QString s_accuracy;
        QString s_avgdamage = doubleToQString(dAvgDamage, 0);
        QString s_mindamage = intToQString(iMinDamage);
        QString s_maxdamage = intToQString(iMaxDamage);
        QString s_damage;
        s_damage.setNum(iDamage);

        QListViewItem *pItem = new QListViewItem(m_listview_offense,
                s_type, s_hits, s_misses, s_accuracy,
                s_avgdamage, s_mindamage, s_maxdamage, s_damage);

        m_listview_offense->insertItem(pItem);

        iNonmeleeDamage += iDamage;
        iNonmeleeHits += iHits;
    }

	CombatOffenseRecord *pRecord;

	for(pRecord = m_combat_offense_list.first(); pRecord != 0; pRecord = m_combat_offense_list.next())
	{
		int iType = pRecord->getType();
		int iSpell = pRecord->getSpell();
		int iHits = pRecord->getHits();
		int iMisses = pRecord->getMisses();
		int iMinDamage = pRecord->getMinDamage();
		int iMaxDamage = pRecord->getMaxDamage();
		int iDamage = pRecord->getTotalDamage();

		double dAvgDamage = (double)iDamage / (double)iHits;
		double dAccuracy = (double)iHits / (double)(iMisses+iHits);

		QString s_type;
		const DamageCategory category = damageCategory(iType);
		switch(category)
		{
			case DAMAGE_CATEGORY_MELEE:
			case DAMAGE_CATEGORY_MELEE_SPECIAL:
			{
				// this is a normal skill
				s_type.sprintf("%s(%d)", (const char*)skill_name(iType), iType);
				break;
			}
			case DAMAGE_CATEGORY_NONMELEE:
			{
				s_type.sprintf("Spell: %s(%d)", (const char*)spell_name(iSpell), iSpell);
				break;
			}
			case DAMAGE_CATEGORY_DAMAGE_SHIELD:
			default:
			{
				s_type.sprintf("Damage Shield: (%d)", iType);
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
		QString s_avgdamage = doubleToQString(dAvgDamage, 0);
		QString s_mindamage = intToQString(iMinDamage);
		QString s_maxdamage = intToQString(iMaxDamage);
		QString s_damage;
		s_damage.setNum(iDamage);

		QListViewItem *pItem = new QListViewItem(m_listview_offense,
			s_type, s_hits, s_misses, s_accuracy,
			s_avgdamage, s_mindamage, s_maxdamage, s_damage);

		m_listview_offense->insertItem(pItem);

		switch(category)
		{
			case DAMAGE_CATEGORY_MELEE:
			{
				iMeleeDamage += iDamage;
				iMeleeHits += iHits;
				break;
			}
			case DAMAGE_CATEGORY_MELEE_SPECIAL:
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
			default:
			{
				iDSDamage += iDamage;
				iDSHits += iHits;
				break;
			}
		}
	}

    PetOffenseRecord *petRecord;
    for(petRecord = m_pet_offense_list.first(); petRecord != 0; petRecord = m_pet_offense_list.next())
    {
        const QString iPetName = petRecord->getPetName();
        const int iPetID = petRecord->getPetID();
        const int iType = petRecord->getType();
        const int iSpell = petRecord->getSpell();
        const int iHits = petRecord->getHits();
        const int iMisses = petRecord->getMisses();
        const int iMinDamage = petRecord->getMinDamage();
        const int iMaxDamage = petRecord->getMaxDamage();
        const int iDamage = petRecord->getTotalDamage();

        const double dAvgDamage = (double)iDamage / (double)iHits;
        const double dAccuracy = (double)iHits / (double)(iMisses+iHits);

        QString s_type;
        const DamageCategory category = damageCategory(iType);
        switch(category)
        {
            case DAMAGE_CATEGORY_MELEE:
            case DAMAGE_CATEGORY_MELEE_SPECIAL:
                {
                    s_type.sprintf("Pet: %s(%d): %s",
                            (const char*)iPetName, iPetID, (const char*)skill_name(iType));
                    break;
                }
            case DAMAGE_CATEGORY_NONMELEE:
                {
                    s_type.sprintf("Pet: %s(%d): Spell: %s",
                            (const char*)iPetName, iPetID, (const char*)spell_name(iSpell));
                    break;
                }
            case DAMAGE_CATEGORY_DAMAGE_SHIELD:
            default:
                {
                    s_type.sprintf("Pet: %s(%d): Damage Shield: (%d)", (const char*)iPetName, iPetID, iType);
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
        QString s_avgdamage = doubleToQString(dAvgDamage, 0);
        QString s_mindamage = intToQString(iMinDamage);
        QString s_maxdamage = intToQString(iMaxDamage);
        QString s_damage;
        s_damage.setNum(iDamage);

        QListViewItem *pItem = new QListViewItem(m_listview_offense,
                s_type, s_hits, s_misses, s_accuracy,
                s_avgdamage, s_mindamage, s_maxdamage, s_damage);

        m_listview_offense->insertItem(pItem);

        switch(category)
        {
            case DAMAGE_CATEGORY_MELEE:
                {
                    iPetMeleeDamage += iDamage;
                    iPetMeleeHits += iHits;
                    break;
                }
            case DAMAGE_CATEGORY_MELEE_SPECIAL:
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
            default:
                {
                    iPetDSDamage += iDamage;
                    iPetDSHits += iHits;
                    break;
                }
        }
    }

	DotOffenseRecord *dotRecord;

	for(dotRecord = m_dot_offense_list.first(); dotRecord != 0; dotRecord = m_dot_offense_list.next())
	{
		int iTicks = dotRecord->getTicks();
		int iMinDamage = dotRecord->getMinDamage();
		int iMaxDamage = dotRecord->getMaxDamage();
		int iDamage = dotRecord->getTotalDamage();

		double dAvgDamage = (double)iDamage / (double)iTicks;

		QString s_type;
		s_type.sprintf("DoT: %s", (const char*)dotRecord->getSpellName());
		QString s_hits;
		s_hits.setNum(iTicks);
		QString s_misses;
		QString s_accuracy;
		QString s_avgdamage = doubleToQString(dAvgDamage, 0);
		QString s_mindamage = intToQString(iMinDamage);
		QString s_maxdamage = intToQString(iMaxDamage);
		QString s_damage;
		s_damage.setNum(iDamage);

		QListViewItem *pItem = new QListViewItem(m_listview_offense,
			s_type, s_hits, s_misses, s_accuracy,
			s_avgdamage, s_mindamage, s_maxdamage, s_damage);

		m_listview_offense->insertItem(pItem);

		iDotDamage += iDamage;
		iDotTicks += iTicks;
	}

	// iNonmeleeDamage includes the pet's non-melee damage because
	// NonMeleeOffenseRecord simply accumulates all non-melee hit messages,
	// which are sent to the client for both player's and pet's non-melee hits.
	// We subtract out the pet's non-melee damage from the total, which gives
	// us the player's non-melee damage.
	iNonmeleeDamage -= iPetNonmeleeDamage;

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

	double dAvgHit = (double)iTotalDamage / (double)iHits;
	int iMinDamage = m_combat_defense_record->getMinDamage();
	int iMaxDamage = m_combat_defense_record->getMaxDamage();

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
	m_label_defense_mitigate_avghit->setText(doubleToQString(dAvgHit, 0));
	m_label_defense_mitigate_minhit->setText(intToQString(iMinDamage));
	m_label_defense_mitigate_maxhit->setText(intToQString(iMaxDamage));
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
        m_label_pet_defense_mitigate_avghit->clear();
        m_label_pet_defense_mitigate_minhit->clear();
        m_label_pet_defense_mitigate_maxhit->clear();
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

    const double dAvgHit = (double)iTotalDamage / (double)iHits;
    const int iMinDamage = pRecord->getMinDamage();
    const int iMaxDamage = pRecord->getMaxDamage();

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
    m_label_pet_defense_mitigate_avghit->setText(doubleToQString(dAvgHit, 0));
    m_label_pet_defense_mitigate_minhit->setText(intToQString(iMinDamage));
    m_label_pet_defense_mitigate_maxhit->setText(intToQString(iMaxDamage));
    m_label_pet_defense_summary_mobhits->setText(QString::number(iHits));
    m_label_pet_defense_summary_mobattacks->setText(QString::number(iMobAttacks));
    m_label_pet_defense_summary_percentavoided->setText(doubleToQString(dAvoided, 1));
    m_label_pet_defense_summary_percentprevented->setText(doubleToQString(dPrevented, 1));
    m_label_pet_defense_summary_totaldamage->setText(QString::number(iTotalDamage));
}

void CombatWindow::updateMob()
{

	int iTotalMobs = 0;
	double dAvgDPS = 0.0;
	double dDPSSum = 0.0;
	double dAvgPetDPS = 0.0;
	double dPetDPSSum = 0.0;

	double dAvgMobDPS = 0.0;
	double dMobDPSSum = 0.0;
	double dAvgPetMobDPS = 0.0;
	double dPetMobDPSSum = 0.0;

	//	empty the list so we can repopulate
	m_listview_mob->clear();

	CombatMobRecord *pRecord;

	for(pRecord = m_combat_mob_list.first(); pRecord != 0; pRecord = m_combat_mob_list.next())
	{
		int iID = pRecord->getID();
		int iDuration = pRecord->getDuration() / 1000;
		int iDamageGiven = pRecord->getDamageGiven();
		double dDPS = pRecord->getDPS();
		int iDamageTaken = pRecord->getDamageTaken();
		double dMobDPS = pRecord->getMobDPS();
		const int iPetDamageGiven = pRecord->getPetDamageGiven();
		const double dPetDPS = pRecord->getPetDPS();
		const int iPetDamageTaken = pRecord->getPetDamageTaken();
		const double dPetMobDPS = pRecord->getPetMobDPS();

		char s_time[64];
		time_t timev = pRecord->getTime();
		strftime(s_time, 64, "%m/%d %H:%M:%S", localtime(&timev));
		QString s_name = pRecord->getName();
		QString s_id = QString::number(iID);
		QString s_duration = QString::number(iDuration);
		QString s_damagegiven = intToQString(iDamageGiven);
		QString s_dps = doubleToQString(dDPS, 1);
		QString s_iDamageTaken = intToQString(iDamageTaken);
		QString s_mobdps = doubleToQString(dMobDPS, 1);
		QString s_petdamagegiven = intToQString(iPetDamageGiven);
		QString s_petdps = doubleToQString(dPetDPS, 1);
		QString s_petiDamageTaken = intToQString(iPetDamageTaken);
		QString s_petmobdps = doubleToQString(dPetMobDPS, 1);


		QListViewItem *pItem = new QListViewItem(m_listview_mob,
			s_time, s_name, s_id, s_duration, s_damagegiven,
			s_dps, s_iDamageTaken, s_mobdps);
		pItem->setText(8, s_petdamagegiven);
		pItem->setText(9, s_petdps);
		pItem->setText(10, s_petiDamageTaken);
		pItem->setText(11, s_petmobdps);

		m_listview_mob->insertItem(pItem);

		iTotalMobs++;
		dDPSSum += dDPS;
		dPetDPSSum += dPetDPS;
		dMobDPSSum += dMobDPS;
		dPetMobDPSSum += dPetMobDPS;
	}

	dAvgDPS = dDPSSum / (double)iTotalMobs;
	dAvgMobDPS = dMobDPSSum / (double)iTotalMobs;
	dAvgPetDPS = dPetDPSSum / (double)iTotalMobs;
	dAvgPetMobDPS = dPetMobDPSSum / (double)iTotalMobs;

	m_label_mob_totalmobs->setText(QString::number(iTotalMobs));
	m_label_mob_avgdps->setText(doubleToQString(dAvgDPS, 1));
	m_label_mob_avgmobdps->setText(doubleToQString(dAvgMobDPS, 1));
	m_label_mob_avgpetdps->setText(doubleToQString(dAvgPetDPS, 1));
	m_label_mob_avgpetmobdps->setText(doubleToQString(dAvgPetMobDPS, 1));
}

void CombatWindow::addNonMeleeHit(const QString& iTargetName, const int iDamage)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addNonMeleeHit starting...");
#endif

    if (iTargetName.isEmpty())
        return;

    addNonMeleeOffenseRecord(iTargetName, iDamage);
    updateOffense();
    updateDPS(iDamage);

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addNonMeleeHit finished...");
#endif
}

void CombatWindow::addNonMeleeOffenseRecord(const QString& iTargetName, const int iDamage)
{
    m_nonmelee_offense_record->addHit(iDamage);
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
        m_dot_offense_list.append(pRecord);
    }

    pRecord->addTick(iDamage);

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addDotOffenseRecord finished...");
#endif
}

void CombatWindow::addCombatRecord(
        int iTargetID, const Spawn* target,
        int iSourceID, const Spawn* source,
        int iType, int iSpell, int iDamage)
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addCombatRecord starting...");
	seqDebug("target=%d, source=%d, type=%d, spell=%d, damage=%d",
			iTargetID, iSourceID, iType, iSpell, iDamage);
#endif

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

	if(iTargetID == iPlayerID && iSourceID != iPlayerID)
	{
		// Damage shields show up as negative damage
		if (isDamageShield(iType))
		{
			addDefenseRecord(-iDamage);
			updateDefense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, -iDamage, tName, sName);
			updateMob();
		}
		else if (isNonMeleeDamage(iType, iDamage) || isMelee(iType)) {
			addDefenseRecord(iDamage);
			updateDefense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, iDamage, tName, sName);
			updateMob();
		}

		if(iDamage > 0)
			updateMobDPS(iDamage);
		else if(isDamageShield(iType))
			updateMobDPS(-iDamage);
	}
	else if(iSourceID == iPlayerID && iTargetID != iPlayerID)
	{
		// Damage shields show up as negative damage
		if (isDamageShield(iType))
		{
			addOffenseRecord(iType, -iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, -iDamage, tName, sName);
			updateMob();
		}
		// For the player, non-melee has positive damage on killing blows
		// (OP_Death) but not regular hits (OP_Action) against others, so
		// the player's non-melee damage is handled via addNonMeleeHit, not here
		else if (isMelee(iType)) {
			addOffenseRecord(iType, iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, iDamage, tName, sName);
			updateMob();
		}

		// Check for melee to avoid non-melee killing blows from OP_Death
		if(iDamage > 0 && isMelee(iType))
			updateDPS(iDamage);
		else if(isDamageShield(iType))
			updateDPS(-iDamage);
	}
	else if (iPlayerID == iTargetPetOwnerID)
	{
		// Damage shields show up as negative damage
		if (isDamageShield(iType))
		{
			addPetDefenseRecord(target, -iDamage);
			updatePetDefense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, -iDamage, tName, sName);
			updateMob();
		}
		else if (isNonMeleeDamage(iType, iDamage) || isMelee(iType)) {
			addPetDefenseRecord(target, iDamage);
			updatePetDefense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, iDamage, tName, sName);
			updateMob();
		}

		if(iDamage > 0)
			updatePetMobDPS(iDamage);
		else if(isDamageShield(iType))
			updatePetMobDPS(-iDamage);
	}
	else if (iPlayerID == iSourcePetOwnerID)
	{
		// Damage shields show up as negative damage
		if (isDamageShield(iType))
		{
			addPetOffenseRecord(iSourceID, sName, iType, -iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, -iDamage, tName, sName);
			updateMob();
		}
		else if (isNonMeleeDamage(iType, iDamage) || isMelee(iType)) {
			addPetOffenseRecord(iSourceID, sName, iType, iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iTargetPetOwnerID, iSourceID, iSourcePetOwnerID, iDamage, tName, sName);
			updateMob();
		}

		if(iDamage > 0)
			updatePetDPS(iDamage);
		else if(isDamageShield(iType))
			updatePetDPS(-iDamage);
	}

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addCombatRecord finished...");
#endif
}

void CombatWindow::addOffenseRecord(int iType, int iDamage, int iSpell)
{

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addOffenseRecord starting...");
#endif

	bool bFoundRecord = false;

	CombatOffenseRecord *pRecord;

	for(pRecord = m_combat_offense_list.first(); pRecord != 0; pRecord = m_combat_offense_list.next())
	{
		if(pRecord->getType() == iType
			&& (isMelee(iType) || pRecord->getSpell() == iSpell))
		{
			bFoundRecord = true;
			break;
		}
	}

	if(!bFoundRecord)
	{
		pRecord = new CombatOffenseRecord(iType, m_player, iSpell);
		m_combat_offense_list.append(pRecord);
	}

	if(iDamage > 0)
	{
		pRecord->addHit(iDamage);
	}
	else if (isMelee(iType))
	{
		pRecord->addMiss(iDamage);
	}

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addOffenseRecord finished...");
#endif
}

void CombatWindow::addPetOffenseRecord(int petID, const QString& petName, int iType, int iDamage, int iSpell)
{
#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addPetOffenseRecord starting...");
#endif

	bool bFoundRecord = false;

	PetOffenseRecord *pRecord;

	for(pRecord = m_pet_offense_list.first(); pRecord != 0; pRecord = m_pet_offense_list.next())
	{
		if(pRecord->getType() == iType && pRecord->getPetID() == petID
			&& (isMelee(iType) || pRecord->getSpell() == iSpell))
		{
			bFoundRecord = true;
			break;
		}
	}

	if(!bFoundRecord)
	{
		pRecord = new PetOffenseRecord(petID, petName, iType, m_player, iSpell);
		m_pet_offense_list.append(pRecord);
	}

	if(iDamage > 0)
	{
		pRecord->addHit(iDamage);
	}
	else if (isMelee(iType))
	{
		pRecord->addMiss(iDamage);
	}

#ifdef DEBUGCOMBAT
    seqDebug("CombatWindow::addPetOffenseRecord finished...");
#endif
}

void CombatWindow::addDefenseRecord(int iDamage)
{
	if(iDamage > 0)
		m_combat_defense_record->addHit(iDamage);
	else
		m_combat_defense_record->addMiss(iDamage);

}

void CombatWindow::addPetDefenseRecord(const Spawn* s, int iDamage)
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
        m_combat_pet_defense_list.append(pRecord);
        m_combobox_pet_defense->insertItem(pet_id_string, 0);
    }

    if (m_autoupdate_pet_defense_selection)
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
        pRecord->addHit(iDamage);
    else
        pRecord->addMiss(iDamage);
}

void CombatWindow::addMobRecord(int iTargetID, int iTargetPetOwnerID,
        int iSourceID, int iSourcePetOwnerID,
        int iDamage, QString tName, QString sName)
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
		//invalid record
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
		m_combat_mob_list.append(pRecord);
	}
	pRecord->addHit(iTargetID, iSourceID, iDamage);

#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addMobRecord finished...");
#endif
}


void CombatWindow::updateDPS(int iDamage)
{

	int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iDPSTimeLast + 10000))
	{
		//	reset DPS
		m_dDPSLast = m_dDPS;
		m_dDPS = 0.0;
		m_iDPSStartTime = iTimeNow;
		m_iCurrentDPSTotal = 0;
	}

	m_iDPSTimeLast = mTime();
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

	m_iPetDPSTimeLast = mTime();
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
	const int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iMobDPSTimeLast + 10000))
	{
		m_dMobDPSLast = m_dDPS;
		m_dMobDPS = 0.0;
		m_iMobDPSStartTime = iTimeNow;
		m_iCurrentMobDPSTotal = 0;
	}

	m_iMobDPSTimeLast = mTime();
	m_iCurrentMobDPSTotal += iDamage;

	int iTimeElapsed = (iTimeNow - m_iMobDPSStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dMobDPS = (double)m_iCurrentMobDPSTotal / (double)iTimeElapsed;
	}

	const QString sMobDPS = doubleToQString(m_dMobDPS, 1);
	const QString sMobDPSLast = doubleToQString(m_dMobDPSLast, 1);

	m_label_mob_currentmobdps->setText(sMobDPS);
	m_label_mob_lastmobdps->setText(sMobDPSLast);
}


void CombatWindow::updatePetMobDPS(int iDamage)
{
	const int iTimeNow = mTime();

	//	reset if it's been 10 seconds without an update
	if(iTimeNow > (m_iPetMobDPSTimeLast + 10000))
	{
		m_dPetMobDPSLast = m_dPetDPS;
		m_dPetMobDPS = 0.0;
		m_iPetMobDPSStartTime = iTimeNow;
		m_iPetCurrentMobDPSTotal = 0;
	}

	m_iPetMobDPSTimeLast = mTime();
	m_iPetCurrentMobDPSTotal += iDamage;

	const int iTimeElapsed = (iTimeNow - m_iPetMobDPSStartTime) / 1000;

	if(iTimeElapsed > 0)
	{
		m_dPetMobDPS = (double)m_iPetCurrentMobDPSTotal / (double)iTimeElapsed;
	}

	const QString sPetMobDPS = doubleToQString(m_dPetMobDPS, 1);
	const QString sPetMobDPSLast = doubleToQString(m_dPetMobDPSLast, 1);

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

void CombatWindow::clearMob()
{
    m_combat_mob_list.clear();
    updateMob();
}

void CombatWindow::clearOffense()
{
    m_combat_offense_list.clear();
    m_pet_offense_list.clear();
    m_dot_offense_list.clear();
    m_nonmelee_offense_record->clear();
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
  m_combat_mob_list.clear();
  updateMob();
  m_combat_offense_list.clear();
  m_pet_offense_list.clear();
  m_dot_offense_list.clear();
  m_nonmelee_offense_record->clear();
  updateOffense();
  m_combat_defense_record->clear();
  updateDefense();
  m_combobox_pet_defense->clear();
  m_combat_pet_defense_current_record = NULL;
  m_combat_pet_defense_list.clear();
  updatePetDefense();
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

#include "combatlog.moc"
