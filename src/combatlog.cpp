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

#include <qgrid.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <stdio.h>
#include <time.h>

#define DEBUGCOMBAT

#undef DEBUGCOMBAT

namespace {

enum DamageCategory
{
    DAMAGE_CATEGORY_MELEE,
    DAMAGE_CATEGORY_MELEE_SPECIAL,
    DAMAGE_CATEGORY_NONMELEE,
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
    DamageCategory c = damageCategory(iType);
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
	m_iMinDamage(65536),
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

	if(iDamage > 0 && iDamage < m_iMinDamage)
		m_iMinDamage = iDamage;

	if(iDamage > m_iMaxDamage)
		m_iMaxDamage = iDamage;

}


////////////////////////////////////////////
//  DotOffenseRecord implementation
////////////////////////////////////////////
DotOffenseRecord::DotOffenseRecord(const Player* p, const QString& iSpellName) :
    m_iSpellName(iSpellName),
    m_player(p),
    m_iTicks(0),
    m_iMinDamage(65536),
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

    if(iDamage > 0 && iDamage < m_iMinDamage)
        m_iMinDamage = iDamage;

    if(iDamage > m_iMaxDamage)
        m_iMaxDamage = iDamage;
}


////////////////////////////////////////////
//  CombatDefenseRecord implementation
////////////////////////////////////////////
CombatDefenseRecord::CombatDefenseRecord(Player* p) :
	m_player(p)
{
  clear();
}

void CombatDefenseRecord::clear(void)
{
  m_iHits = 0;
  m_iMisses = 0;
  m_iBlocks = 0;
  m_iParries = 0;
  m_iRipostes = 0;
  m_iDodges = 0;
  m_iMinDamage = 65536;
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

	if(iDamage > 0 && iDamage < m_iMinDamage)
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
		default:
		{
#ifdef DEBUGCOMBAT
		  seqDebug("CombatDefenseRecord::addMiss:WARNING: invalid miss reason");
#endif
			break;
		}
	}

}


////////////////////////////////////////////
//	CombatMobRecord implementation
////////////////////////////////////////////
CombatMobRecord::CombatMobRecord(int iID, int iStartTime, Player* p) :
m_iID(iID),
m_player(p),
m_iStartTime(iStartTime),
m_iLastTime(iStartTime),
m_iDamageGiven(0),
m_dDPS(0.0),
m_iDamageTaken(0),
m_dMobDPS(0.0)
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

void CombatMobRecord::addHit(int iTarget, int iSource, int iDamage)
{

	int iPlayerID = m_player->id();

	if(iSource == iPlayerID && iTarget == m_iID)
	{
		//	update m_iLastTime
		m_iLastTime = mTime();

		if(iDamage > 0)
		{
			m_iDamageGiven += iDamage;
		}
	}
	else if(iSource == m_iID && iTarget == iPlayerID)
	{
		//	update m_iLastTime
		m_iLastTime = mTime();

		if(iDamage > 0)
		{
			m_iDamageTaken += iDamage;
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
}

CombatWindow::CombatWindow(Player* player,
			   QWidget* parent, const char* name)
  : SEQWindow("Combat", "ShowEQ - Combat", parent, name),
    m_player(player),
    m_iCurrentDPSTotal(0),
    m_iDPSStartTime(0),
    m_iDPSTimeLast(0),
    m_dDPS(0.0),
    m_dDPSLast(0.0)
{
  /* Hopefully this is only called once to set up the window,
     so this is a good place to initialize some things which
     otherwise won't be. */

	m_combat_offense_list.setAutoDelete(true);
	m_dot_offense_list.setAutoDelete(true);
	m_combat_defense_record = new CombatDefenseRecord(player);
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

	m_widget_mob = initMobWidget();
	m_tab->addTab(m_widget_mob, "&Mobs");

	m_clear_menu = new QPopupMenu(this);
	m_clear_menu->insertItem("Clear Offense Stats", this, SLOT(clearOffense()));
	m_clear_menu->insertItem("Clear Mob Stats", this, SLOT(clearMob()));

	m_menu_bar->insertItem("&Clear", m_clear_menu);

	updateOffense();
	updateDefense();
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

	m_listview_offense = new SEQListView(preferenceName(), listGBox);
	m_listview_offense->addColumn("Type");
	m_listview_offense->setColumnAlignment(0, Qt::AlignRight);
	m_listview_offense->addColumn("Hit");
	m_listview_offense->setColumnAlignment(1, Qt::AlignRight);
	m_listview_offense->addColumn("Miss");
	m_listview_offense->setColumnAlignment(2, Qt::AlignRight);
	m_listview_offense->addColumn("Ratio");
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

	QGrid *summaryGrid = new QGrid(4, summaryGBox);

	new QLabel("Total Damage:", summaryGrid);
	m_label_offense_totaldamage = new QLabel(summaryGrid);

	new QLabel("Avg Melee:", summaryGrid);
	m_label_offense_avgmelee = new QLabel(summaryGrid);

	new QLabel("% from Special:", summaryGrid);
	m_label_offense_percentspecial = new QLabel(summaryGrid);

	new QLabel("Avg Special:", summaryGrid);
	m_label_offense_avgspecial = new QLabel(summaryGrid);

	new QLabel("% from NonMelee:", summaryGrid);
	m_label_offense_percentnonmelee = new QLabel(summaryGrid);

	new QLabel("Avg NonMelee:", summaryGrid);
	m_label_offense_avgnonmelee = new QLabel(summaryGrid);

	new QLabel("% from DoT:", summaryGrid);
	m_label_offense_percentdot = new QLabel(summaryGrid);

	new QLabel("Avg DoT Tick:", summaryGrid);
	m_label_offense_avgdottick = new QLabel(summaryGrid);

	new QLabel("% from DS:", summaryGrid);
	m_label_offense_percentds = new QLabel(summaryGrid);

	new QLabel("Avg DS:", summaryGrid);
	m_label_offense_avgds = new QLabel(summaryGrid);

	((QGridLayout *)summaryGrid->layout())->setColStretch(1, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(3, 1);
	summaryGrid->layout()->setSpacing(5);

	return pWidget;
}

QWidget* CombatWindow::initDefenseWidget()
{
	QWidget *pWidget = new QWidget(m_tab);
	m_layout_defense = new QVBoxLayout(pWidget);

	QGroupBox *avoidanceGBox = new QVGroupBox("Avoidance", pWidget);
	m_layout_defense->addWidget(avoidanceGBox);

	QGrid *avoidanceGrid = new QGrid(6, avoidanceGBox);

	new QLabel("Misses:", avoidanceGrid);
	m_label_defense_avoid_misses = new QLabel(avoidanceGrid);

	new QLabel("Blocks:", avoidanceGrid);
	m_label_defense_avoid_block = new QLabel(avoidanceGrid);

	new QLabel("Parries:", avoidanceGrid);
	m_label_defense_avoid_parry = new QLabel(avoidanceGrid);

	new QLabel("Ripostes:", avoidanceGrid);
	m_label_defense_avoid_riposte = new QLabel(avoidanceGrid);

	new QLabel("Dodges", avoidanceGrid);
	m_label_defense_avoid_dodge = new QLabel(avoidanceGrid);

	new QLabel("Total:", avoidanceGrid);
	m_label_defense_avoid_total = new QLabel(avoidanceGrid);

	((QGridLayout *)avoidanceGrid->layout())->setColStretch(1, 1);
	((QGridLayout *)avoidanceGrid->layout())->setColStretch(3, 1);
	((QGridLayout *)avoidanceGrid->layout())->setColStretch(5, 1);
	avoidanceGrid->layout()->setSpacing(5);

	QGroupBox *mitigationGBox = new QVGroupBox("Mitigation", pWidget);
	m_layout_defense->addWidget(mitigationGBox);

	QGrid *mitigationGrid = new QGrid(6, mitigationGBox);

	new QLabel("Avg. Hit:", mitigationGrid);
	m_label_defense_mitigate_avghit = new QLabel(mitigationGrid);

	new QLabel("Min:", mitigationGrid);
	m_label_defense_mitigate_minhit = new QLabel(mitigationGrid);

	new QLabel("Max:", mitigationGrid);
	m_label_defense_mitigate_maxhit = new QLabel(mitigationGrid);

	((QGridLayout *)mitigationGrid->layout())->setColStretch(1, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(3, 1);
	((QGridLayout *)mitigationGrid->layout())->setColStretch(5, 1);
	mitigationGrid->layout()->setSpacing(5);

	QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
	m_layout_defense->addWidget(summaryGBox);

	QGrid *summaryGrid = new QGrid(6, summaryGBox);

	new QLabel("Mob Attacks:", summaryGrid);
	m_label_defense_summary_mobattacks = new QLabel(summaryGrid);

	new QLabel("% Avoided:", summaryGrid);
	m_label_defense_summary_percentavoided = new QLabel(summaryGrid);

	new QLabel("Total Damage:", summaryGrid);
	m_label_defense_summary_totaldamage = new QLabel(summaryGrid);

	((QGridLayout *)summaryGrid->layout())->setColStretch(1, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(3, 1);
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

	m_listview_mob = new SEQListView(preferenceName(), listGBox);
	m_listview_mob->addColumn("Time");
	m_listview_mob->setColumnAlignment(0, Qt::AlignRight);
	m_listview_mob->addColumn("Name");
	m_listview_mob->setColumnAlignment(1, Qt::AlignRight);
	m_listview_mob->addColumn("ID");
	m_listview_mob->setColumnAlignment(2, Qt::AlignRight);
	m_listview_mob->addColumn("Duration");
	m_listview_mob->setColumnAlignment(3, Qt::AlignRight);
	m_listview_mob->addColumn("Damage Given");
	m_listview_mob->setColumnAlignment(4, Qt::AlignRight);
	m_listview_mob->addColumn("DPS");
	m_listview_mob->setColumnAlignment(5, Qt::AlignRight);
	m_listview_mob->addColumn("Damage Taken");
	m_listview_mob->setColumnAlignment(6, Qt::AlignRight);
	m_listview_mob->addColumn("MOB DPS");
	m_listview_mob->setColumnAlignment(7, Qt::AlignRight);

	m_listview_mob->restoreColumns();

	//m_listview_mob->setMinimumSize(m_listview_mob->sizeHint().width(), 200);

	QGroupBox *summaryGBox = new QVGroupBox("Summary", pWidget);
	m_layout_mob->addWidget(summaryGBox);

	QGrid *summaryGrid = new QGrid(4, summaryGBox);

	new QLabel("Total Mobs", summaryGrid);
	m_label_mob_totalmobs = new QLabel(summaryGrid);

	new QLabel("Avg DPS:", summaryGrid);
	m_label_mob_avgdps = new QLabel(summaryGrid);

	new QLabel("Current DPS:", summaryGrid);
	m_label_mob_currentdps = new QLabel(summaryGrid);

	new QLabel("Last DPS:", summaryGrid);
	m_label_mob_lastdps = new QLabel(summaryGrid);

	((QGridLayout *)summaryGrid->layout())->setColStretch(1, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(3, 1);
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
	QString s_percentspecial;
	QString s_percentnonmelee;
	QString s_percentdot;
	QString s_percentds;
	QString s_avgmelee;
	QString s_avgspecial;
	QString s_avgnonmelee;
	QString s_avgdottick;
	QString s_avgds;

	int iTotalDamage = 0;
	//int iTotalHits = 0;
	double dPercentSpecial = 0.0;
	double dPercentNonmelee = 0.0;
	double dPercentDot = 0.0;
	double dPercentDS = 0.0;
	double dAvgMelee = 0.0;
	double dAvgSpecial = 0.0;
	double dAvgNonmelee = 0.0;
	double dAvgDotTick = 0.0;
	double dAvgDS = 0.0;

	int iMeleeDamage = 0;
	int iMeleeHits = 0;
	int iSpecialDamage = 0;
	int iSpecialHits = 0;
	int iNonmeleeDamage = 0;
	int iNonmeleeHits = 0;
	int iDotDamage = 0;
	int iDotTicks = 0;
	int iDSDamage = 0;
	int iDSHits = 0;


	//	empty the list so we can repopulate
	m_listview_offense->clear();

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
		double dRatio = (double)iHits / (double)iMisses;

		QString s_type;
		switch(damageCategory(iType))
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
		s_misses.setNum(iMisses);
		QString s_ratio;
		s_ratio = QString("%1 to 1").arg(dRatio);
		QString s_avgdamage;
		s_avgdamage.setNum(dAvgDamage);
		QString s_mindamage;
		s_mindamage.setNum(iMinDamage);
		QString s_maxdamage;
		s_maxdamage.setNum(iMaxDamage);
		QString s_damage;
		s_damage.setNum(iDamage);

		QListViewItem *pItem = new QListViewItem(m_listview_offense,
			s_type, s_hits, s_misses, s_ratio,
			s_avgdamage, s_mindamage, s_maxdamage, s_damage);

		m_listview_offense->insertItem(pItem);

		switch(damageCategory(iType))
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
		QString s_misses = "";
		QString s_ratio = "";
		QString s_avgdamage;
		s_avgdamage.setNum(dAvgDamage);
		QString s_mindamage;
		s_mindamage.setNum(iMinDamage);
		QString s_maxdamage;
		s_maxdamage.setNum(iMaxDamage);
		QString s_damage;
		s_damage.setNum(iDamage);

		QListViewItem *pItem = new QListViewItem(m_listview_offense,
			s_type, s_hits, s_misses, s_ratio,
			s_avgdamage, s_mindamage, s_maxdamage, s_damage);

		m_listview_offense->insertItem(pItem);

		iDotDamage += iDamage;
		iDotTicks += iTicks;
	}

	iTotalDamage = iMeleeDamage + iSpecialDamage + iNonmeleeDamage + iDotDamage + iDSDamage;
	//iTotalHits = iMeleeHits + iSpecialHits + iNonmeleeHits;

	dPercentSpecial = ((double)iSpecialDamage / (double)iTotalDamage) * 100.0;
	dPercentNonmelee = ((double)iNonmeleeDamage / (double)iTotalDamage) * 100.0;
	dPercentDot = ((double)iDotDamage / (double)iTotalDamage) * 100.0;
	dPercentDS = ((double)iDSDamage / (double)iTotalDamage) * 100.0;

	dAvgMelee = (double)iMeleeDamage / (double)iMeleeHits;
	dAvgSpecial = (double)iSpecialDamage / (double)iSpecialHits;
	dAvgNonmelee = (double)iNonmeleeDamage / (double)iNonmeleeHits;
	dAvgDotTick = (double)iDotDamage / (double)iDotTicks;
	dAvgDS = (double)iDSDamage / (double)iDSHits;

	s_totaldamage.setNum(iTotalDamage);
	s_percentspecial.setNum(dPercentSpecial);
	s_percentnonmelee.setNum(dPercentNonmelee);
	s_percentdot.setNum(dPercentDot);
	s_percentds.setNum(dPercentDS);
	s_avgmelee.setNum(dAvgMelee);
	s_avgspecial.setNum(dAvgSpecial);
	s_avgnonmelee.setNum(dAvgNonmelee);
	s_avgdottick.setNum(dAvgDotTick);
	s_avgds.setNum(dAvgDS);

	m_label_offense_totaldamage->setText(s_totaldamage);
	m_label_offense_percentspecial->setText(s_percentspecial);
	m_label_offense_percentnonmelee->setText(s_percentnonmelee);
	m_label_offense_percentdot->setText(s_percentdot);
	m_label_offense_percentds->setText(s_percentds);
	m_label_offense_avgmelee->setText(s_avgmelee);
	m_label_offense_avgspecial->setText(s_avgspecial);
	m_label_offense_avgnonmelee->setText(s_avgnonmelee);
	m_label_offense_avgdottick->setText(s_avgdottick);
	m_label_offense_avgds->setText(s_avgds);


#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::updateOffense finished...");
#endif

}

void CombatWindow::updateDefense()
{
	int iMisses = m_combat_defense_record->getMisses();
	int iBlocks = m_combat_defense_record->getBlocks();
	int iParries = m_combat_defense_record->getParries();
	int iRipostes = m_combat_defense_record->getRipostes();
	int iDodges = m_combat_defense_record->getDodges();
	int iTotalAvoid = iMisses+iBlocks+iParries+iRipostes+iDodges;

	double dAvgHit = (double)m_combat_defense_record->getTotalDamage() / (double)m_combat_defense_record->getHits();
	int iMinHit = m_combat_defense_record->getMinDamage();
	int iMaxHit = m_combat_defense_record->getMaxDamage();

	int iMobAttacks = m_combat_defense_record->getTotalAttacks();
	double dAvoided = ((double)iTotalAvoid / (double)iMobAttacks) * 100.0;
	int iTotalDamage = m_combat_defense_record->getTotalDamage();


	m_label_defense_avoid_misses->setText(QString::number(iMisses));
	m_label_defense_avoid_block->setText(QString::number(iBlocks));
	m_label_defense_avoid_parry->setText(QString::number(iParries));
	m_label_defense_avoid_riposte->setText(QString::number(iRipostes));
	m_label_defense_avoid_dodge->setText(QString::number(iDodges));
	m_label_defense_avoid_total->setText(QString::number(iTotalAvoid));
	m_label_defense_mitigate_avghit->setText(QString::number(dAvgHit));
	m_label_defense_mitigate_minhit->setText(QString::number(iMinHit));
	m_label_defense_mitigate_maxhit->setText(QString::number(iMaxHit));
	m_label_defense_summary_mobattacks->setText(QString::number(iMobAttacks));
	m_label_defense_summary_percentavoided->setText(QString::number(dAvoided));
	m_label_defense_summary_totaldamage->setText(QString::number(iTotalDamage));

}

void CombatWindow::updateMob()
{

	int iTotalMobs = 0;
	double dAvgDPS = 0.0;
	double dDPSSum = 0.0;

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

		char s_time[64];
		time_t timev = pRecord->getTime();
		strftime(s_time, 64, "%m/%d %H:%M:%S", localtime(&timev));
		QString s_name = pRecord->getName();
		QString s_id = QString::number(iID);
		QString s_duration = QString::number(iDuration);
		QString s_damagegiven = QString::number(iDamageGiven);
		QString s_dps = QString::number(dDPS);
		QString s_iDamageTaken = QString::number(iDamageTaken);
		QString s_mobdps = QString::number(dMobDPS);


		QListViewItem *pItem = new QListViewItem(m_listview_mob,
			s_time, s_name, s_id, s_duration, s_damagegiven,
			s_dps, s_iDamageTaken, s_mobdps);

		m_listview_mob->insertItem(pItem);

		iTotalMobs++;
		dDPSSum += dDPS;
	}

	if (iTotalMobs)
	  dAvgDPS = dDPSSum / (double)iTotalMobs;
	else
	  dAvgDPS = 0;

	m_label_mob_totalmobs->setText(QString::number(iTotalMobs));
	m_label_mob_avgdps->setText(QString::number(dAvgDPS));
	m_label_mob_currentdps->setText(QString::number(m_dDPS));
	m_label_mob_lastdps->setText(QString::number(m_dDPSLast));

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

void CombatWindow::addCombatRecord(int iTargetID, int iSourceID, int iType, int iSpell, int iDamage, QString tName, QString sName)
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addCombatRecord starting...");
	seqDebug("target=%d, source=%d, type=%d, spell=%d, damage=%d",
			iTargetID, iSourceID, iType, iSpell, iDamage);
#endif

	int iPlayerID = m_player->id();

	//	The one case we won't handle (for now) is where the Target
	//	and Source are the same.

	if(iTargetID == iPlayerID && iSourceID != iPlayerID)
	{
		addDefenseRecord(iDamage);
		updateDefense();
		addMobRecord(iTargetID, iSourceID, iDamage, tName, sName);
		updateMob();
	}
	else if(iSourceID == iPlayerID && iTargetID != iPlayerID)
	{
		// Damage shields show up as negative damage
		if (isDamageShield(iType))
		{
			addOffenseRecord(iType, -iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iSourceID, -iDamage, tName, sName);
			updateMob();
		}
		else if (isNonMeleeDamage(iType, iDamage) || isMelee(iType)) {
			addOffenseRecord(iType, iDamage, iSpell);
			updateOffense();
			addMobRecord(iTargetID, iSourceID, iDamage, tName, sName);
			updateMob();
		}

		if(iDamage > 0)
			updateDPS(iDamage);
		else if(isDamageShield(iType))
			updateDPS(-iDamage);
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


void CombatWindow::addDefenseRecord(int iDamage)
{
	if(iDamage > 0)
		m_combat_defense_record->addHit(iDamage);
	else
		m_combat_defense_record->addMiss(iDamage);

}

void CombatWindow::addMobRecord(int iTargetID, int iSourceID, int iDamage, QString tName, QString sName)
{
#ifdef DEBUGCOMBAT
	seqDebug("CombatWindow::addMobRecord starting...");
#endif

	int iTimeNow = mTime();
	int iPlayerID = m_player->id();
	int iMobID;
	QString mobName;

	if(iPlayerID == iTargetID)
	{
		iMobID = iSourceID;
		mobName = sName;
	}
	else if(iPlayerID == iSourceID)
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
		pRecord = new CombatMobRecord(iMobID, iTimeNow, m_player);
		pRecord->setName(mobName);
		m_combat_mob_list.append(pRecord);
	}
	pRecord->setTime(time(0));
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
		m_dDPS = 0;
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

	m_label_mob_currentdps->setText(QString::number(m_dDPS));
	m_label_mob_lastdps->setText(QString::number(m_dDPSLast));


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
}

void CombatWindow::clearMob()
{
    m_combat_mob_list.clear();
    updateMob();
}

void CombatWindow::clearOffense()
{
    m_combat_offense_list.clear();
    m_dot_offense_list.clear();
    updateOffense();
}

void CombatWindow::clear(void)
{
  m_combat_mob_list.clear();
  updateMob();
  m_combat_offense_list.clear();
  m_dot_offense_list.clear();
  updateOffense();
  m_combat_defense_record->clear();
  updateDefense();
  resetDPS();
}

#include "combatlog.moc"
