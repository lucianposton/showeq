/*
 *  combatlog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net
 */

#include "combatlog.h"
#include "interface.h"
#include "util.h"

#include <qgrid.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qmessagebox.h>
#include <stdio.h>

#define DEBUGCOMBAT

#undef DEBUGCOMBAT


////////////////////////////////////////////
//  CombatOffenseRecord implementation
////////////////////////////////////////////
CombatOffenseRecord::CombatOffenseRecord( int iType, EQPacket* p) :
	m_iType(iType),
	m_packet(p),
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
//  CombatDefenseRecord implementation
////////////////////////////////////////////
CombatDefenseRecord::CombatDefenseRecord(EQPacket* p) :
	m_packet(p),
	m_iHits(0),
	m_iMisses(0),
	m_iBlocks(0),
	m_iParries(0),
	m_iRipostes(0),
	m_iDodges(0),
	m_iMinDamage(65536),
	m_iMaxDamage(0),
	m_iTotalDamage(0),
	m_iTotalAttacks(0)
{

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
			printf("CombatDefenseRecord::addMiss:WARNING: invalid miss reason\n");
#endif
			break;
		}
	}

}


////////////////////////////////////////////
//	CombatMobRecord implementation
////////////////////////////////////////////
CombatMobRecord::CombatMobRecord(int iID, int iStartTime, EQPacket* p) :
m_iID(iID),
m_packet(p),
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

	int iPlayerID = m_packet->getplayer()->getPlayerID();

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

CombatWindow::CombatWindow(EQPacket* p)
  : m_packet(p),
  m_iCurrentDPSTotal(0),
  m_iDPSStartTime(0),
  m_iDPSTimeLast(0),
  m_dDPS(0.0),
  m_dDPSLast(0.0)
{
   QString section = "Combat";
   setCaption(pSEQPrefs->getPrefString("Title", section, "ShowEQ Combat"));

  /* Hopefully this is only called once to set up the window,
     so this is a good place to initialize some things which
     otherwise won't be. */

	m_combat_offense_list.setAutoDelete(true);
	m_combat_defense_record = new CombatDefenseRecord(p);
	m_combat_mob_list.setAutoDelete(true);

	initUI();
}

void CombatWindow::initUI()
{
#ifdef DEBUGCOMBAT
	printf("CombatWindow::initUI: starting...\n");
#endif

	m_widget_offense = initOffenseWidget();
	this->addTab(m_widget_offense, "&Offense");

	m_widget_defense = initDefenseWidget();
	this->addTab(m_widget_defense, "&Defense");

	m_widget_mob = initMobWidget();
	this->addTab(m_widget_mob, "&Mobs");

	updateOffense();
	updateDefense();
	updateMob();

#ifdef DEBUGCOMBAT
	printf("CombatWindow::initUI: finished...\n");
#endif
}

QWidget* CombatWindow::initOffenseWidget()
{
	QWidget *pWidget = new QWidget(this);

	m_layout_offense = new QVBoxLayout(pWidget);

	QGroupBox *listGBox = new QVGroupBox(pWidget);
	m_layout_offense->addWidget(listGBox);

	m_listview_offense = new QListView(listGBox);
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

	m_listview_offense->setMinimumSize(m_listview_offense->sizeHint().width(), 200);

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

	((QGridLayout *)summaryGrid->layout())->setColStretch(1, 1);
	((QGridLayout *)summaryGrid->layout())->setColStretch(3, 1);
	summaryGrid->layout()->setSpacing(5);

	return pWidget;
}

QWidget* CombatWindow::initDefenseWidget()
{
	QWidget *pWidget = new QWidget(this);
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
	QWidget *pWidget = new QWidget(this);

	m_layout_mob = new QVBoxLayout(pWidget);

	QGroupBox *listGBox = new QVGroupBox(pWidget);
	m_layout_mob->addWidget(listGBox);

	m_listview_mob = new QListView(listGBox);
	m_listview_mob->addColumn("Name");
	m_listview_mob->setColumnAlignment(0, Qt::AlignRight);
	m_listview_mob->addColumn("ID");
	m_listview_mob->setColumnAlignment(1, Qt::AlignRight);
	m_listview_mob->addColumn("Duration");
	m_listview_mob->setColumnAlignment(2, Qt::AlignRight);
	m_listview_mob->addColumn("Damage Given");
	m_listview_mob->setColumnAlignment(3, Qt::AlignRight);
	m_listview_mob->addColumn("DPS");
	m_listview_mob->setColumnAlignment(4, Qt::AlignRight);
	m_listview_mob->addColumn("Damage Taken");
	m_listview_mob->setColumnAlignment(5, Qt::AlignRight);
	m_listview_mob->addColumn("MOB DPS");
	m_listview_mob->setColumnAlignment(6, Qt::AlignRight);

	m_listview_mob->setMinimumSize(m_listview_mob->sizeHint().width(), 200);

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


void CombatWindow::updateOffense()
{
#ifdef DEBUGCOMBAT
	printf("CombatWindow::updateOffense starting...\n");
#endif


	QString s_totaldamage;
	QString s_percentspecial;
	QString s_percentnonmelee;
	QString s_avgmelee;
	QString s_avgspecial;
	QString s_avgnonmelee;

	int iTotalDamage = 0;
	int iTotalHits = 0;
	double dPercentSpecial = 0.0;
	double dPercentNonmelee = 0.0;
	double dAvgMelee = 0.0;
	double dAvgSpecial = 0.0;
	double dAvgNonmelee = 0.0;

	int iMeleeDamage = 0;
	int iMeleeHits = 0;
	int iSpecialDamage = 0;
	int iSpecialHits = 0;
	int iNonmeleeDamage = 0;
	int iNonmeleeHits = 0;


	//	empty the list so we can repopulate
	m_listview_offense->clear();

	CombatOffenseRecord *pRecord;

	for(pRecord = m_combat_offense_list.first(); pRecord != 0; pRecord = m_combat_offense_list.next())
	{
		int iType = pRecord->getType();
		int iHits = pRecord->getHits();
		int iMisses = pRecord->getMisses();
		int iMinDamage = pRecord->getMinDamage();
		int iMaxDamage = pRecord->getMaxDamage();
		int iDamage = pRecord->getTotalDamage();

		double dAvgDamage = (double)iDamage / (double)iHits;
		double dRatio = (double)iHits / (double)iMisses;

		QString s_type;
		s_type.sprintf("%s(%d)", (const char*)skill_name(iType), iType);
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

		switch(iType)
		{
			case 0:		// 1H Blunt
			case 1:		// 1H Slashing
			case 2:		// 2H Blunt
			case 3:		// 2H Slashing
			case 28:	// Hand To Hand
			case 36:	// Piercing
			{
				iMeleeDamage += iDamage;
				iMeleeHits += iHits;
				break;
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
				iSpecialDamage += iDamage;
				iSpecialHits += iHits;
				break;
			}
			default:
			{
				iNonmeleeDamage += iDamage;
				iNonmeleeHits += iHits;
				break;
			}
		}
	}

	iTotalDamage = iMeleeDamage + iSpecialDamage + iNonmeleeDamage;
	iTotalHits = iMeleeHits + iSpecialHits + iNonmeleeHits;

	dPercentSpecial = ((double)iSpecialDamage / (double)iTotalDamage) * 100.0;
	dPercentNonmelee = ((double)iNonmeleeDamage / (double)iTotalDamage) * 100.0;

	dAvgMelee = (double)iMeleeDamage / (double)iMeleeHits;
	dAvgSpecial = (double)iSpecialDamage / (double)iSpecialHits;
	dAvgNonmelee = (double)iNonmeleeDamage / (double)iNonmeleeHits;

	s_totaldamage.setNum(iTotalDamage);
	s_percentspecial.setNum(dPercentSpecial);
	s_percentnonmelee.setNum(dPercentNonmelee);
	s_avgmelee.setNum(dAvgMelee);
	s_avgspecial.setNum(dAvgSpecial);
	s_avgnonmelee.setNum(dAvgNonmelee);

	m_label_offense_totaldamage->setText(s_totaldamage);
	m_label_offense_percentspecial->setText(s_percentspecial);
	m_label_offense_percentnonmelee->setText(s_percentnonmelee);
	m_label_offense_avgmelee->setText(s_avgmelee);
	m_label_offense_avgspecial->setText(s_avgspecial);
	m_label_offense_avgnonmelee->setText(s_avgnonmelee);


#ifdef DEBUGCOMBAT
	printf("CombatWindow::updateOffense finished...\n");
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

		QString s_name = QString::number(iID);
		QString s_id = QString::number(iID);
		QString s_duration = QString::number(iDuration);
		QString s_damagegiven = QString::number(iDamageGiven);
		QString s_dps = QString::number(dDPS);
		QString s_iDamageTaken = QString::number(iDamageTaken);
		QString s_mobdps = QString::number(dMobDPS);


		QListViewItem *pItem = new QListViewItem(m_listview_mob,
			s_name, s_id, s_duration, s_damagegiven,
			s_dps, s_iDamageTaken, s_mobdps);

		m_listview_mob->insertItem(pItem);

		iTotalMobs++;
		dDPSSum += dDPS;
	}

	dAvgDPS = dDPSSum / (double)iTotalMobs;

	m_label_mob_totalmobs->setText(QString::number(iTotalMobs));
	m_label_mob_avgdps->setText(QString::number(dAvgDPS));
	m_label_mob_currentdps->setText(QString::number(m_dDPS));
	m_label_mob_lastdps->setText(QString::number(m_dDPSLast));

}

void CombatWindow::addCombatRecord(int iTargetID, int iSourceID, int iType, int iSpell, int iDamage)
{
#ifdef DEBUGCOMBAT
	printf("CombatWindow::addCombatRecord starting...\n");
	printf("target=%d, source=%d, type=%d, spell=%d, damage=%d\n",
			iTargetID, iSourceID, iType, iSpell, iDamage);
#endif

	int iPlayerID = m_packet->getplayer()->getPlayerID();

	//	The one case we won't handle (for now) is where the Target
	//	and Source are the same.

	if(iTargetID == iPlayerID && iSourceID != iPlayerID)
	{
		addDefenseRecord(iDamage);
		updateDefense();
		addMobRecord(iTargetID, iSourceID, iDamage);
		updateMob();
	}
	else if(iSourceID == iPlayerID && iTargetID != iPlayerID)
	{
		addOffenseRecord(iType, iDamage);
		updateOffense();
		addMobRecord(iTargetID, iSourceID, iDamage);
		updateMob();

		if(iDamage > 0)
			updateDPS(iDamage);
	}

#ifdef DEBUGCOMBAT
	printf("CombatWindow::addCombatRecord finished...\n");
#endif
}

void CombatWindow::addOffenseRecord(int iType, int iDamage)
{

#ifdef DEBUGCOMBAT
	printf("CombatWindow::addOffenseRecord starting...\n");
#endif

	bool bFoundRecord = false;

	CombatOffenseRecord *pRecord;

	for(pRecord = m_combat_offense_list.first(); pRecord != 0; pRecord = m_combat_offense_list.next())
	{
		if(pRecord->getType() == iType)
		{
			bFoundRecord = true;
			break;
		}
	}

	if(!bFoundRecord)
	{
		pRecord = new CombatOffenseRecord(iType, m_packet);
		m_combat_offense_list.append(pRecord);
	}

	if(iDamage > 0)
	{
		pRecord->addHit(iDamage);
	}
	else
	{
		pRecord->addMiss(iDamage);
	}

#ifdef DEBUGCOMBAT
	printf("CombatWindow::addOffenseRecord finished...\n");
#endif
}


void CombatWindow::addDefenseRecord(int iDamage)
{
	if(iDamage > 0)
		m_combat_defense_record->addHit(iDamage);
	else
		m_combat_defense_record->addMiss(iDamage);

}

void CombatWindow::addMobRecord(int iTargetID, int iSourceID, int iDamage)
{
#ifdef DEBUGCOMBAT
	printf("CombatWindow::addMobRecord starting...\n");
#endif

	int iTimeNow = mTime();
	int iPlayerID = m_packet->getplayer()->getPlayerID();
	int iMobID;

	if(iPlayerID == iTargetID)
	{
		iMobID = iSourceID;
	}
	else if(iPlayerID == iSourceID)
	{
		iMobID = iTargetID;
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
		pRecord = new CombatMobRecord(iMobID, iTimeNow, m_packet);
		m_combat_mob_list.append(pRecord);
	}

	pRecord->addHit(iTargetID, iSourceID, iDamage);


#ifdef DEBUGCOMBAT
	printf("CombatWindow::addMobRecord finished...\n");
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
	//	we'll let updateDPS do all the work
	//	by simply setting m_iDPSTimeLast to 0

	m_iDPSTimeLast = 0;

	updateDPS(0);
}

