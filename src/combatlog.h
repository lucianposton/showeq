/*
 * experiencelog.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net
 */

#ifndef COMBATLOG_H
# define COMBATLOG_H

# include <qobject.h>
# include <qwidget.h>
# include <qtabwidget.h>
# include <qlist.h>
# include <qlistview.h>
# include <qcombobox.h>
# include <qlabel.h>
# include <qlayout.h>
# include <qmenubar.h>

# include <sys/time.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdio.h>

#include "seqwindow.h"
#include "seqlistview.h"
#include "player.h"
#include "group.h"

//----------------------------------------------------------------------
// forward declarations
class Player;

////////////////////////////////////////////
//  CombatOffenseRecord definition
//////////////////////////////////////////`//
class CombatOffenseRecord
{
public:

	CombatOffenseRecord(int iType, Player* p);

	int		getType() { return m_iType; };
	int		getHits() { return m_iHits; };
	int		getMisses() { return m_iMisses; };
	int		getMinDamage() { return m_iMinDamage; };
	int		getMaxDamage() { return m_iMaxDamage; };
	int		getTotalDamage() { return m_iTotalDamage; };

	void	addMiss(int iMissReason) { m_iMisses++; };
	void	addHit(int iDamage);

private:
	int			m_iType;
	Player*	m_player;

	int 		m_iHits;
	int			m_iMisses;
	int			m_iMinDamage;
	int			m_iMaxDamage;
	int			m_iTotalDamage;

};


////////////////////////////////////////////
//  CombatDefenseRecord definition
////////////////////////////////////////////
class CombatDefenseRecord
{
public:

	CombatDefenseRecord(Player* p);

	int		getHits() { return m_iHits; };
	int		getMisses() { return m_iMisses; };
	int		getBlocks() { return m_iBlocks; };
	int		getParries() { return m_iParries; };
	int		getRipostes() { return m_iRipostes; };
	int		getDodges() { return m_iDodges; };
	int		getMinDamage() { return m_iMinDamage; };
	int		getMaxDamage() { return m_iMaxDamage; };
	int		getTotalDamage() { return m_iTotalDamage; };
	int		getTotalAttacks() { return m_iTotalAttacks; };

	void	addMiss(int iMissReason);
	void	addHit(int iDamage);

private:
	Player*	m_player;

	int 		m_iHits;
	int			m_iMisses;
	int			m_iBlocks;
	int			m_iParries;
	int			m_iRipostes;
	int			m_iDodges;
	int			m_iMinDamage;
	int			m_iMaxDamage;
	int			m_iTotalDamage;
	int			m_iTotalAttacks;
};


////////////////////////////////////////////
//  CombatMobRecord definition
////////////////////////////////////////////
class CombatMobRecord
{
public:

	CombatMobRecord(int iID, int iStartTime, Player* p);

	int		getID() { return m_iID; };
	int		getDuration() { return (m_iLastTime - m_iStartTime); };
	int		getDamageGiven() { return m_iDamageGiven; };
	int		getDamageTaken() { return m_iDamageTaken; };

	QString		getName() { return m_iName; };
	void		setName(QString iName) { m_iName = iName; };

	time_t		getTime() { return m_time; };
	void		setTime(time_t iTime) { m_time = iTime; };

	double	getDPS();
	double	getMobDPS();

	void	addHit(int iTarget, int iSource, int iDamage);
	
private:
	int			m_iID;
	Player*	m_player;

	int			m_iStartTime;
	int			m_iLastTime;
	int			m_iDamageGiven;
	double		m_dDPS;
	int			m_iDamageTaken;
	double		m_dMobDPS;

	QString			m_iName;
	time_t			m_time;
};

////////////////////////////////////////////
//  CombatWindow definition
////////////////////////////////////////////
class CombatWindow : public SEQWindow
{
	Q_OBJECT

public:

	CombatWindow(Player* player,
		     QWidget* parent = 0, const char* name = 0);
	~CombatWindow();

public slots:

	virtual void savePrefs(void);
	void addCombatRecord(int iTargetID, int iSourceID, int iType, int iSpell, int iDamage, QString tName, QString sName);
	void resetDPS();
	void clearMob();
	void clearOffense();

private:

	void initUI();
	QWidget* initOffenseWidget();
	QWidget* initDefenseWidget();
	QWidget* initMobWidget();

	void addOffenseRecord(int iType, int iDamage);
	void addDefenseRecord(int iDamage);
	void addMobRecord(int iTargetID, int iSourceID, int iDamage, QString tName, QString sName);

	void updateOffense();
	void updateDefense();
	void updateMob();
	void updateDPS(int iDamage);


private:
	Player*	m_player;

	QWidget* 	m_widget_offense;
	QWidget* 	m_widget_defense;
	QWidget*	m_widget_mob;

	QTabWidget*     m_tab;
	QVBoxLayout*	m_layout_offense;
	QVBoxLayout*	m_layout_defense;
	QVBoxLayout*	m_layout_mob;

	SEQListView* 	m_listview_offense;
	SEQListView* 	m_listview_mob;

	QLabel* 	m_label_offense_totaldamage;
	QLabel*		m_label_offense_percentspecial;
	QLabel*		m_label_offense_percentnonmelee;
	QLabel*		m_label_offense_avgmelee;
	QLabel*		m_label_offense_avgspecial;
	QLabel*		m_label_offense_avgnonmelee;

	QLabel*		m_label_defense_avoid_misses;
	QLabel*		m_label_defense_avoid_block;
	QLabel*		m_label_defense_avoid_parry;
	QLabel*		m_label_defense_avoid_riposte;
	QLabel*		m_label_defense_avoid_dodge;
	QLabel*		m_label_defense_avoid_total;
	QLabel*		m_label_defense_mitigate_avghit;
	QLabel*		m_label_defense_mitigate_minhit;
	QLabel*		m_label_defense_mitigate_maxhit;
	QLabel*		m_label_defense_summary_mobattacks;
	QLabel*		m_label_defense_summary_percentavoided;
	//QLabel*		m_label_defense_summary_ratio;
	QLabel*		m_label_defense_summary_totaldamage;

	QLabel*		m_label_mob_totalmobs;
	QLabel*		m_label_mob_avgdps;
	QLabel*		m_label_mob_currentdps;
	QLabel*		m_label_mob_lastdps;

	QList<CombatOffenseRecord> m_combat_offense_list;
	CombatDefenseRecord *m_combat_defense_record;
	QList<CombatMobRecord> m_combat_mob_list;

	QMenuBar	*m_menu_bar;
	QPopupMenu	*m_clear_menu;

	int		m_iCurrentDPSTotal;
	int		m_iDPSStartTime;
	int		m_iDPSTimeLast;
	double		m_dDPS;
	double		m_dDPSLast;

};

#endif // COMBATLOG_H
