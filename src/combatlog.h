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

	CombatOffenseRecord(int iType, Player* p, int iSpell);

	int		getType() { return m_iType; };
	int		getSpell() { return m_iSpell; };
	int		getHits() { return m_iHits; };
	int		getMisses() { return m_iMisses; };
	int		getMinDamage() { return m_iMinDamage; };
	int		getMaxDamage() { return m_iMaxDamage; };
	int		getTotalDamage() { return m_iTotalDamage; };

	void	addMiss(int iMissReason) { m_iMisses++; };
	void	addHit(int iDamage);

protected:
	int			m_iType;
	int			m_iSpell;
	Player*	m_player;

	int 		m_iHits;
	int			m_iMisses;
	int			m_iMinDamage;
	int			m_iMaxDamage;
	int			m_iTotalDamage;

};


////////////////////////////////////////////
//  NonMeleeOffenseRecord definition
//////////////////////////////////////////`//
class NonMeleeOffenseRecord : public CombatOffenseRecord
{
public:
    NonMeleeOffenseRecord();

    void clear();
};


////////////////////////////////////////////
//  PetOffenseRecord definition
//////////////////////////////////////////`//
class PetOffenseRecord : public CombatOffenseRecord
{
public:
    PetOffenseRecord(int iPetID, const QString& iPetName, int iType,
            Player* p, int iSpell);

    int		getPetID() { return m_iPetID; };
    QString		getPetName() { return m_iPetName; };

private:
    const int			m_iPetID;
    const QString	m_iPetName;
};


////////////////////////////////////////////
//  DotOffenseRecord definition
//////////////////////////////////////////`//
class DotOffenseRecord
{
    public:
        DotOffenseRecord(const Player* p, const QString& iSpellName);

        int getTicks() { return m_iTicks; };
        int getMinDamage() { return m_iMinDamage; };
        int getMaxDamage() { return m_iMaxDamage; };
        int getTotalDamage() { return m_iTotalDamage; };
        QString getSpellName() { return m_iSpellName; };

        void addTick(int iDamage);

    private:
        const QString m_iSpellName;
        const Player* m_player;

        int m_iTicks;
        int m_iMinDamage;
        int m_iMaxDamage;
        int m_iTotalDamage;
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
	int		getInvulnerables() { return m_iInvulnerables; };
	int		getShieldAbsorbs() { return m_iShieldAbsorbs; };
	int		getMinDamage() { return m_iMinDamage; };
	int		getMaxDamage() { return m_iMaxDamage; };
	int		getTotalDamage() { return m_iTotalDamage; };
	int		getTotalAttacks() { return m_iTotalAttacks; };

	void    clear(void);
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
	int			m_iInvulnerables;
	int			m_iShieldAbsorbs;
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

	CombatMobRecord(int iID, const QString& iName, Player* p);

	int		getID() { return m_iID; };
	int		getDuration();
	int		getDamageGiven() { return m_iDamageGiven; };
	int		getDamageTaken() { return m_iDamageTaken; };
	int		getPetDamageGiven() { return m_iPetDamageGiven; };
	int		getPetDamageTaken() { return m_iPetDamageTaken; };

	QString		getName() { return m_iName; };

	time_t		getTime() { return m_time; };

	double	getDPS();
	double	getMobDPS();
	double	getPetDPS();
	double	getPetMobDPS();

	void	addHit(int iTarget, int iSource, int iDamage);
	
private:
	int			m_iID;
	QString			m_iName;
	Player*	m_player;

	int			m_iStartTime;
	int			m_iLastTime;
	int			m_iDamageGiven;
	double		m_dDPS;
	int			m_iDamageTaken;
	double		m_dMobDPS;

	int			m_iPetStartTime;
	int			m_iPetLastTime;
	int			m_iPetDamageGiven;
	double		m_dPetDPS;
	int			m_iPetDamageTaken;
	double		m_dPetMobDPS;

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
	void addNonMeleeHit(const QString& iTargetName, int iDamage);
	void addDotTick(const QString& iTargetName,
			const QString& iSpellName, int iDamage);
	void addCombatRecord(int iTargetID, const Spawn* target, int iSourceID, const Spawn* source, int iType, int iSpell, int iDamage);
	void resetDPS();
	void clearMob();
	void clearOffense();
	void clearDefense();
	void clear(void);

private:

	void initUI();
	QWidget* initOffenseWidget();
	QWidget* initDefenseWidget();
	QWidget* initMobWidget();

	void addNonMeleeOffenseRecord(const QString& iTargetName, const int iDamage);
	void addDotOffenseRecord(const QString& iSpellName, int iDamage);
	void addOffenseRecord(int iType, int iDamage, int iSpell);
	void addPetOffenseRecord(int petID, const QString& petName, int iType, int iDamage, int iSpell);
	void addDefenseRecord(int iDamage);
	void addMobRecord(int iTargetID, int iTargetPetOwnerID, int iSourceID, int iSourcePetOwnerID, int iDamage, QString tName, QString sName);

	void updateOffense();
	void updateDefense();
	void updateMob();
	void updateDPS(int iDamage);
	void updatePetDPS(int iDamage);
	void updateMobDPS(int iDamage);
	void updatePetMobDPS(int iDamage);

private:
	Player*	m_player;

	QWidget* 	m_widget_offense;
	QWidget* 	m_widget_defense;
	QWidget*	m_widget_mob;

	QTabWidget*     m_tab;
	QVBoxLayout*	m_layout_offense;
	QVBoxLayout*	m_layout_defense;
	QHBoxLayout*	m_layout_defense_top_third;
	QVBoxLayout*	m_layout_mob;

	SEQListView* 	m_listview_offense;
	SEQListView* 	m_listview_mob;

	QLabel* 	m_label_offense_totaldamage;
	QLabel* 	m_label_offense_percentpettotaldamage;

	QLabel* 	m_label_offense_currentdps;
	QLabel* 	m_label_offense_currentpetdps;

	QLabel*		m_label_offense_percentmelee;
	QLabel*		m_label_offense_percentspecial;
	QLabel*		m_label_offense_percentnonmelee;
	QLabel*		m_label_offense_percentdot;
	QLabel*		m_label_offense_percentds;
	QLabel*		m_label_offense_avgmelee;
	QLabel*		m_label_offense_avgspecial;
	QLabel*		m_label_offense_avgnonmelee;
	QLabel*		m_label_offense_avgdottick;
	QLabel*		m_label_offense_avgds;

	QLabel*		m_label_offense_percentpetmelee;
	QLabel*		m_label_offense_percentpetspecial;
	QLabel*		m_label_offense_percentpetnonmelee;
	QLabel*		m_label_offense_percentpetds;
	QLabel*		m_label_offense_avgpetmelee;
	QLabel*		m_label_offense_avgpetspecial;
	QLabel*		m_label_offense_avgpetnonmelee;
	QLabel*		m_label_offense_avgpetds;

	QLabel*		m_label_defense_avoid_misses;
	QLabel*		m_label_defense_avoid_block;
	QLabel*		m_label_defense_avoid_parry;
	QLabel*		m_label_defense_avoid_riposte;
	QLabel*		m_label_defense_avoid_dodge;
	QLabel*		m_label_defense_avoid_total;
	QLabel*		m_label_defense_prevented_invulnerables;
	QLabel*		m_label_defense_prevented_shield_absorb;
	QLabel*		m_label_defense_prevented_total;
	QLabel*		m_label_defense_mitigate_avghit;
	QLabel*		m_label_defense_mitigate_minhit;
	QLabel*		m_label_defense_mitigate_maxhit;
	QLabel*		m_label_defense_summary_mobhits;
	QLabel*		m_label_defense_summary_mobattacks;
	QLabel*		m_label_defense_summary_percentavoided;
	QLabel*		m_label_defense_summary_percentprevented;
	//QLabel*		m_label_defense_summary_ratio;
	QLabel*		m_label_defense_summary_totaldamage;

	QLabel*		m_label_mob_totalmobs;

	QLabel*		m_label_mob_avgdps;
	QLabel*		m_label_mob_avgpetdps;
	QLabel*		m_label_mob_currentdps;
	QLabel*		m_label_mob_currentpetdps;
	QLabel*		m_label_mob_lastdps;
	QLabel*		m_label_mob_lastpetdps;

	QLabel*		m_label_mob_avgmobdps;
	QLabel*		m_label_mob_avgpetmobdps;
	QLabel*		m_label_mob_currentmobdps;
	QLabel*		m_label_mob_currentpetmobdps;
	QLabel*		m_label_mob_lastmobdps;
	QLabel*		m_label_mob_lastpetmobdps;

	QList<CombatOffenseRecord> m_combat_offense_list;
	QList<DotOffenseRecord> m_dot_offense_list;
	QList<PetOffenseRecord> m_pet_offense_list;
	NonMeleeOffenseRecord *m_nonmelee_offense_record;
	CombatDefenseRecord *m_combat_defense_record;
	QList<CombatMobRecord> m_combat_mob_list;

	QMenuBar	*m_menu_bar;
	QPopupMenu	*m_clear_menu;

	int		m_iCurrentDPSTotal;
	int		m_iDPSStartTime;
	int		m_iDPSTimeLast;
	double		m_dDPS;
	double		m_dDPSLast;

	int		m_iPetCurrentDPSTotal;
	int		m_iPetDPSStartTime;
	int		m_iPetDPSTimeLast;
	double		m_dPetDPS;
	double		m_dPetDPSLast;

	int		m_iCurrentMobDPSTotal;
	int		m_iMobDPSStartTime;
	int		m_iMobDPSTimeLast;
	double		m_dMobDPS;
	double		m_dMobDPSLast;

	int		m_iPetCurrentMobDPSTotal;
	int		m_iPetMobDPSStartTime;
	int		m_iPetMobDPSTimeLast;
	double		m_dPetMobDPS;
	double		m_dPetMobDPSLast;
};

#endif // COMBATLOG_H
