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
# include <cmath>
# include <limits>

#include "seqwindow.h"
#include "seqlistview.h"
#include "player.h"
#include "group.h"

//----------------------------------------------------------------------
// forward declarations
class Player;

//----------------------------------------------------------------------
// enum declarations
enum DamageCategory
{
    DAMAGE_CATEGORY_MELEE,
    DAMAGE_CATEGORY_MELEE_RANGED,
    DAMAGE_CATEGORY_MELEE_SPECIAL_BASIC,
    DAMAGE_CATEGORY_MELEE_SPECIAL_BACKSTAB,
    DAMAGE_CATEGORY_MELEE_SPECIAL_MONK,
    DAMAGE_CATEGORY_NONMELEE,
    DAMAGE_CATEGORY_ENVIRONMENT,
    DAMAGE_CATEGORY_DAMAGE_SHIELD,
    DAMAGE_CATEGORY_OTHER
};

class Record
{
    public:
        Record();
        virtual ~Record();

        void addHit(int iDamage, DamageCategory category, int iTarget, int iSource);
        void addMiss(int iMissReason);
        void clear();

        void update(QListView* parent);
    protected:
        void updateViewItem(
                QListView* parent,
                const QString& l0=QString::null,
                const QString& l1=QString::null,
                const QString& l2=QString::null,
                const QString& l3=QString::null,
                const QString& l4=QString::null,
                const QString& l5=QString::null,
                const QString& l6=QString::null,
                const QString& l7=QString::null,
                const QString& l8=QString::null,
                const QString& l9=QString::null,
                const QString& l10=QString::null,
                const QString& l11=QString::null);
    private:
        virtual void addHitImpl(int iDamage, DamageCategory category,
                int iTarget, int iSource) = 0;
        virtual void addMissImpl(int iMissReason) = 0;
        virtual void clearImpl() = 0;
        virtual void updateImpl(QListView* parent) = 0;
        virtual void initializeViewItem(SEQListViewItem<>*) = 0;

        // TODO double check ptr lifetime
        bool m_isDirty;
        SEQListViewItem<>* m_viewItem;
};

////////////////////////////////////////////
//  CombatOffenseRecord definition
//////////////////////////////////////////`//
class CombatOffenseRecord : public Record
{
public:

	CombatOffenseRecord(int iType, const Player* p, int iSpell);

	int		getType() { return m_iType; };
	int		getSpell() { return m_iSpell; };
	int		getHits() { return m_iHits; };
	int		getMisses() { return m_iMisses; };
	int		getMinDamage() { return m_iMinDamage; };
	int		getMaxDamage() { return m_iMaxDamage; };
	int		getTotalDamage() { return m_iTotalDamage; };
	double	getAverage() const { return m_dAverage; };
	double	getStandardDeviation() const
		{ return m_iHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dM2/(m_iHits-1)); };

private:
    virtual void addHitImpl(int iDamage, DamageCategory category,
            int iTarget, int iSource);
    virtual void addMissImpl(int iMissReason);
    virtual void clearImpl();
    virtual void updateImpl(QListView* parent);
    virtual void initializeViewItem(SEQListViewItem<>*);

	int			m_iType;
	int			m_iSpell;
	const Player*	m_player;

	int 		m_iHits;
	int			m_iMisses;
	int			m_iMinDamage;
	int			m_iMaxDamage;
	int			m_iTotalDamage;
	double		m_dAverage;
	double		m_dM2;
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
    virtual void updateImpl(QListView* parent);
    virtual void initializeViewItem(SEQListViewItem<>*);

    const int			m_iPetID;
    const QString	m_iPetName;
};


////////////////////////////////////////////
//  DotOffenseRecord definition
//////////////////////////////////////////`//
class DotOffenseRecord : public CombatOffenseRecord
{
    public:
        DotOffenseRecord(const Player* p, const QString& iSpellName);

        QString getSpellName() { return m_iSpellName; };

    private:
        virtual void updateImpl(QListView* parent);
        virtual void initializeViewItem(SEQListViewItem<>*);

        const QString m_iSpellName;
};


////////////////////////////////////////////
//  NonmeleeOffenseRecord definition
//////////////////////////////////////////`//
class NonmeleeOffenseRecord : public CombatOffenseRecord
{
    public:
        NonmeleeOffenseRecord();

    private:
        virtual void updateImpl(QListView* parent);
        virtual void initializeViewItem(SEQListViewItem<>*);
};


////////////////////////////////////////////
//  CombatDefenseRecord definition
////////////////////////////////////////////
class CombatDefenseRecord : public Record
{
public:

	CombatDefenseRecord(const Spawn* s);

	QString		displayString() const { return m_displayString; };
	static QString createRecordIDString(const QString& name, int id,
			const QString& classname, int level);

	int		getHits() const { return m_iHits; };
	int		getMisses() const { return m_iMisses; };
	int		getBlocks() const { return m_iBlocks; };
	int		getParries() const { return m_iParries; };
	int		getRipostes() const { return m_iRipostes; };
	int		getDodges() const { return m_iDodges; };
	int		getInvulnerables() const { return m_iInvulnerables; };
	int		getShieldAbsorbs() const { return m_iShieldAbsorbs; };
	int		getTotalDamage() const { return m_iTotalDamage; };
	int		getTotalAttacks() const { return m_iTotalAttacks; };

	int		getMeleeHits() const { return m_iMeleeHits; };
	int		getMeleeTotalDamage() const { return m_iMeleeTotalDamage; };
	int		getMeleeMinDamage() const { return m_iMeleeMinDamage; };
	int		getMeleeMaxDamage() const { return m_iMeleeMaxDamage; };
	double	getMeleeAverage() const { return m_dMeleeAverage; };
	double	getMeleeStandardDeviation() const
		{ return m_iMeleeHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dMeleeM2/(m_iMeleeHits-1)); };

	int		getRangedHits() const { return m_iRangedHits; };
	int		getRangedTotalDamage() const { return m_iRangedTotalDamage; };
	int		getRangedMinDamage() const { return m_iRangedMinDamage; };
	int		getRangedMaxDamage() const { return m_iRangedMaxDamage; };
	double	getRangedAverage() const { return m_dRangedAverage; };
	double	getRangedStandardDeviation() const
		{ return m_iRangedHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dRangedM2/(m_iRangedHits-1)); };

	int		getSpecialHits() const { return m_iSpecialHits; };
	int		getSpecialTotalDamage() const { return m_iSpecialTotalDamage; };
	int		getSpecialMinDamage() const { return m_iSpecialMinDamage; };
	int		getSpecialMaxDamage() const { return m_iSpecialMaxDamage; };
	double	getSpecialAverage() const { return m_dSpecialAverage; };
	double	getSpecialStandardDeviation() const
		{ return m_iSpecialHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dSpecialM2/(m_iSpecialHits-1)); };

	int		getBackstabHits() const { return m_iBackstabHits; };
	int		getBackstabTotalDamage() const { return m_iBackstabTotalDamage; };
	int		getBackstabMinDamage() const { return m_iBackstabMinDamage; };
	int		getBackstabMaxDamage() const { return m_iBackstabMaxDamage; };
	double	getBackstabAverage() const { return m_dBackstabAverage; };
	double	getBackstabStandardDeviation() const
		{ return m_iBackstabHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dBackstabM2/(m_iBackstabHits-1)); };

	int		getMonkHits() const { return m_iMonkHits; };
	int		getMonkTotalDamage() const { return m_iMonkTotalDamage; };
	int		getMonkMinDamage() const { return m_iMonkMinDamage; };
	int		getMonkMaxDamage() const { return m_iMonkMaxDamage; };
	double	getMonkAverage() const { return m_dMonkAverage; };
	double	getMonkStandardDeviation() const
		{ return m_iMonkHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dMonkM2/(m_iMonkHits-1)); };

	int		getNonmeleeHits() const { return m_iNonmeleeHits; };
	int		getNonmeleeTotalDamage() const { return m_iNonmeleeTotalDamage; };
	int		getNonmeleeMinDamage() const { return m_iNonmeleeMinDamage; };
	int		getNonmeleeMaxDamage() const { return m_iNonmeleeMaxDamage; };
	double	getNonmeleeAverage() const { return m_dNonmeleeAverage; };
	double	getNonmeleeStandardDeviation() const
		{ return m_iNonmeleeHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dNonmeleeM2/(m_iNonmeleeHits-1)); };

	int		getDamageShieldHits() const { return m_iDamageShieldHits; };
	int		getDamageShieldTotalDamage() const { return m_iDamageShieldTotalDamage; };
	int		getDamageShieldMinDamage() const { return m_iDamageShieldMinDamage; };
	int		getDamageShieldMaxDamage() const { return m_iDamageShieldMaxDamage; };
	double	getDamageShieldAverage() const { return m_dDamageShieldAverage; };
	double	getDamageShieldStandardDeviation() const
		{ return m_iDamageShieldHits < 2 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(m_dDamageShieldM2/(m_iDamageShieldHits-1)); };

private:
    virtual void addHitImpl(int iDamage, DamageCategory category,
            int iTarget, int iSource);
    virtual void addMissImpl(int iMissReason);
    virtual void clearImpl();
    virtual void updateImpl(QListView* parent);
    virtual void initializeViewItem(SEQListViewItem<>*);

	const QString		m_displayString;

	int 		m_iHits;
	int			m_iMisses;
	int			m_iBlocks;
	int			m_iParries;
	int			m_iRipostes;
	int			m_iDodges;
	int			m_iInvulnerables;
	int			m_iShieldAbsorbs;
	int			m_iTotalDamage;
	int			m_iTotalAttacks;

	// M2 is the sum of squares of differences from mean
	double		m_dMeleeM2;
	double		m_dMeleeAverage;
	int 		m_iMeleeHits;
	int			m_iMeleeTotalDamage;
	int			m_iMeleeMinDamage;
	int			m_iMeleeMaxDamage;

	double		m_dRangedM2;
	double		m_dRangedAverage;
	int 		m_iRangedHits;
	int			m_iRangedTotalDamage;
	int			m_iRangedMinDamage;
	int			m_iRangedMaxDamage;

	double		m_dSpecialM2;
	double		m_dSpecialAverage;
	int 		m_iSpecialHits;
	int			m_iSpecialTotalDamage;
	int			m_iSpecialMinDamage;
	int			m_iSpecialMaxDamage;

	double		m_dBackstabM2;
	double		m_dBackstabAverage;
	int 		m_iBackstabHits;
	int			m_iBackstabTotalDamage;
	int			m_iBackstabMinDamage;
	int			m_iBackstabMaxDamage;

	double		m_dMonkM2;
	double		m_dMonkAverage;
	int 		m_iMonkHits;
	int			m_iMonkTotalDamage;
	int			m_iMonkMinDamage;
	int			m_iMonkMaxDamage;

	double		m_dNonmeleeM2;
	double		m_dNonmeleeAverage;
	int 		m_iNonmeleeHits;
	int			m_iNonmeleeTotalDamage;
	int			m_iNonmeleeMinDamage;
	int			m_iNonmeleeMaxDamage;

	double		m_dDamageShieldM2;
	double		m_dDamageShieldAverage;
	int 		m_iDamageShieldHits;
	int			m_iDamageShieldTotalDamage;
	int			m_iDamageShieldMinDamage;
	int			m_iDamageShieldMaxDamage;
};


////////////////////////////////////////////
//  CombatMobRecord definition
////////////////////////////////////////////
class CombatMobRecord : public Record
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

	double	getDPS() const { return m_dDPS; };
	double	getMobDPS() const { return m_dMobDPS; };
	double	getPetDPS() const { return m_dPetDPS; };
	double	getPetMobDPS() const { return m_dPetMobDPS; };

private:
    virtual void addHitImpl(int iDamage, DamageCategory category,
            int iTarget, int iSource);
    virtual void addMissImpl(int iMissReason);
    virtual void clearImpl();
    virtual void updateImpl(QListView* parent);
    virtual void initializeViewItem(SEQListViewItem<>*);

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
//  CombatOtherRecord definition
////////////////////////////////////////////
class CombatOtherRecord : public Record
{
public:
    CombatOtherRecord(
            int iTargetID, int iSourceID,
            const QString& tName, const QString& sName);

    int getSourceID() const { return m_iSourceID; };
    int getTargetID() const { return m_iTargetID; };
    QString getSourceName() const { return m_iSourceName; };
    QString getTargetName() const { return m_iTargetName; };

    int getDuration() const { return m_iLastTime - m_iStartTime; };
    int getDamageTotal() const { return m_iDamageTotal; };
    double getDPS() const { return m_dDPS; };

    time_t getTime() const { return m_time; };

private:
    virtual void addHitImpl(int iDamage, DamageCategory category,
            int iTarget, int iSource);
    virtual void addMissImpl(int iMissReason);
    virtual void clearImpl();
    virtual void updateImpl(QListView* parent);
    virtual void initializeViewItem(SEQListViewItem<>*);

    const int m_iTargetID;
    const int m_iSourceID;
    const QString m_iTargetName;
    const QString m_iSourceName;

    int m_iStartTime;
    int m_iLastTime;
    double m_dDPS;
    int m_iDamageTotal;

    time_t m_time;
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
	void addCombatRecord(int iTargetID, const Spawn* target, int iSourceID, const Spawn* source, int iType, int iSpell, int iDamage, bool isKillingBlow);
	void resetDPS();
	void clearOther();
	void clearMob();
	void clearOffense();
	void clearDefense();
	void clearPetDefense();
	void clear(void);
	void considerSpawn();

	void charmUpdate(const uint8_t* data);
	void newSpawn(const uint8_t* data);
	void petDefenseComboboxSelectionChanged(const QString& selected);

private:

	void initUI();
	QWidget* initOffenseWidget();
	QWidget* initDefenseWidget();
	QWidget* initPetDefenseWidget();
	QWidget* initMobWidget();
	QWidget* initOtherWidget();

	void addNonMeleeOffenseRecord(const QString& iTargetName, const int iDamage);
	void addDotOffenseRecord(const QString& iSpellName, int iDamage);
	void addOffenseRecord(int iType, DamageCategory category, int iDamage, int iSpell);
	void addPetOffenseRecord(int petID, const QString& petName, int iType, DamageCategory category, int iDamage, int iSpell);
	void addDefenseRecord(int iDamage, DamageCategory category);
	void addPetDefenseRecord(const Spawn* s, int iDamage, DamageCategory category);
	void addMobRecord(int iTargetID, int iTargetPetOwnerID, int iSourceID, int iSourcePetOwnerID, DamageCategory category, int iDamage, const QString& tName, const QString& sName);
	void addOtherRecord(DamageCategory c, int iTargetID, int iSourceID, int iDamage,
			const QString& tName, const QString& sName, bool isKillingBlow);

	void updateOffense();
	void updateDefense();
	void updatePetDefense();
	void updateMob();
	void updateOther();
	void updateDPS(int iDamage);
	void updatePetDPS(int iDamage);
	void updateMobDPS(int iDamage);
	void updatePetMobDPS(int iDamage);

private:
	Player*	m_player;
	int		m_lastConsider;
	bool	m_autoupdate_pet_defense_selection;

	QWidget* 	m_widget_offense;
	QWidget* 	m_widget_defense;
	QWidget* 	m_widget_pet_defense;
	QWidget*	m_widget_mob;
	QWidget*	m_widget_other;

	QTabWidget*     m_tab;
	QVBoxLayout*	m_layout_offense;
	QVBoxLayout*	m_layout_defense;
	QVBoxLayout*	m_layout_mob;
	QVBoxLayout*	m_layout_other;
	QVBoxLayout*	m_layout_pet_defense;

    QComboBox* m_combobox_pet_defense;

	SEQListView* 	m_listview_offense;
	SEQListView* 	m_listview_mob;
	SEQListView* 	m_listview_other;

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

	QLabel*		m_label_defense_mitigate_melee_percent;
	QLabel*		m_label_defense_mitigate_melee_avg;
	QLabel*		m_label_defense_mitigate_melee_sd;
	QLabel*		m_label_defense_mitigate_melee_min;
	QLabel*		m_label_defense_mitigate_melee_max;
	QLabel*		m_label_defense_mitigate_melee_count;

	QLabel*		m_label_defense_mitigate_ranged_percent;
	QLabel*		m_label_defense_mitigate_ranged_avg;
	QLabel*		m_label_defense_mitigate_ranged_sd;
	QLabel*		m_label_defense_mitigate_ranged_min;
	QLabel*		m_label_defense_mitigate_ranged_max;
	QLabel*		m_label_defense_mitigate_ranged_count;

	QLabel*		m_label_defense_mitigate_special_percent;
	QLabel*		m_label_defense_mitigate_special_avg;
	QLabel*		m_label_defense_mitigate_special_sd;
	QLabel*		m_label_defense_mitigate_special_min;
	QLabel*		m_label_defense_mitigate_special_max;
	QLabel*		m_label_defense_mitigate_special_count;

	QLabel*		m_label_defense_mitigate_backstab_percent;
	QLabel*		m_label_defense_mitigate_backstab_avg;
	QLabel*		m_label_defense_mitigate_backstab_sd;
	QLabel*		m_label_defense_mitigate_backstab_min;
	QLabel*		m_label_defense_mitigate_backstab_max;
	QLabel*		m_label_defense_mitigate_backstab_count;

	QLabel*		m_label_defense_mitigate_monk_percent;
	QLabel*		m_label_defense_mitigate_monk_avg;
	QLabel*		m_label_defense_mitigate_monk_sd;
	QLabel*		m_label_defense_mitigate_monk_min;
	QLabel*		m_label_defense_mitigate_monk_max;
	QLabel*		m_label_defense_mitigate_monk_count;

	QLabel*		m_label_defense_mitigate_nonmelee_percent;
	QLabel*		m_label_defense_mitigate_nonmelee_avg;
	QLabel*		m_label_defense_mitigate_nonmelee_sd;
	QLabel*		m_label_defense_mitigate_nonmelee_min;
	QLabel*		m_label_defense_mitigate_nonmelee_max;
	QLabel*		m_label_defense_mitigate_nonmelee_count;

	QLabel*		m_label_defense_mitigate_damageshield_percent;
	QLabel*		m_label_defense_mitigate_damageshield_avg;
	QLabel*		m_label_defense_mitigate_damageshield_sd;
	QLabel*		m_label_defense_mitigate_damageshield_min;
	QLabel*		m_label_defense_mitigate_damageshield_max;
	QLabel*		m_label_defense_mitigate_damageshield_count;

	QLabel*		m_label_defense_summary_mobhits;
	QLabel*		m_label_defense_summary_mobattacks;
	QLabel*		m_label_defense_summary_percentavoided;
	QLabel*		m_label_defense_summary_percentprevented;
	//QLabel*		m_label_defense_summary_ratio;
	QLabel*		m_label_defense_summary_totaldamage;

	QLabel*		m_label_pet_defense_avoid_misses;
	QLabel*		m_label_pet_defense_avoid_block;
	QLabel*		m_label_pet_defense_avoid_parry;
	QLabel*		m_label_pet_defense_avoid_riposte;
	QLabel*		m_label_pet_defense_avoid_dodge;
	QLabel*		m_label_pet_defense_avoid_total;

	QLabel*		m_label_pet_defense_prevented_invulnerables;
	QLabel*		m_label_pet_defense_prevented_shield_absorb;
	QLabel*		m_label_pet_defense_prevented_total;

	QLabel*		m_label_pet_defense_mitigate_melee_percent;
	QLabel*		m_label_pet_defense_mitigate_melee_avg;
	QLabel*		m_label_pet_defense_mitigate_melee_sd;
	QLabel*		m_label_pet_defense_mitigate_melee_min;
	QLabel*		m_label_pet_defense_mitigate_melee_max;
	QLabel*		m_label_pet_defense_mitigate_melee_count;

	QLabel*		m_label_pet_defense_mitigate_ranged_percent;
	QLabel*		m_label_pet_defense_mitigate_ranged_avg;
	QLabel*		m_label_pet_defense_mitigate_ranged_sd;
	QLabel*		m_label_pet_defense_mitigate_ranged_min;
	QLabel*		m_label_pet_defense_mitigate_ranged_max;
	QLabel*		m_label_pet_defense_mitigate_ranged_count;

	QLabel*		m_label_pet_defense_mitigate_special_percent;
	QLabel*		m_label_pet_defense_mitigate_special_avg;
	QLabel*		m_label_pet_defense_mitigate_special_sd;
	QLabel*		m_label_pet_defense_mitigate_special_min;
	QLabel*		m_label_pet_defense_mitigate_special_max;
	QLabel*		m_label_pet_defense_mitigate_special_count;

	QLabel*		m_label_pet_defense_mitigate_backstab_percent;
	QLabel*		m_label_pet_defense_mitigate_backstab_avg;
	QLabel*		m_label_pet_defense_mitigate_backstab_sd;
	QLabel*		m_label_pet_defense_mitigate_backstab_min;
	QLabel*		m_label_pet_defense_mitigate_backstab_max;
	QLabel*		m_label_pet_defense_mitigate_backstab_count;

	QLabel*		m_label_pet_defense_mitigate_monk_percent;
	QLabel*		m_label_pet_defense_mitigate_monk_avg;
	QLabel*		m_label_pet_defense_mitigate_monk_sd;
	QLabel*		m_label_pet_defense_mitigate_monk_min;
	QLabel*		m_label_pet_defense_mitigate_monk_max;
	QLabel*		m_label_pet_defense_mitigate_monk_count;

	QLabel*		m_label_pet_defense_mitigate_nonmelee_percent;
	QLabel*		m_label_pet_defense_mitigate_nonmelee_avg;
	QLabel*		m_label_pet_defense_mitigate_nonmelee_sd;
	QLabel*		m_label_pet_defense_mitigate_nonmelee_min;
	QLabel*		m_label_pet_defense_mitigate_nonmelee_max;
	QLabel*		m_label_pet_defense_mitigate_nonmelee_count;

	QLabel*		m_label_pet_defense_mitigate_damageshield_percent;
	QLabel*		m_label_pet_defense_mitigate_damageshield_avg;
	QLabel*		m_label_pet_defense_mitigate_damageshield_sd;
	QLabel*		m_label_pet_defense_mitigate_damageshield_min;
	QLabel*		m_label_pet_defense_mitigate_damageshield_max;
	QLabel*		m_label_pet_defense_mitigate_damageshield_count;

	QLabel*		m_label_pet_defense_summary_mobhits;
	QLabel*		m_label_pet_defense_summary_mobattacks;
	QLabel*		m_label_pet_defense_summary_percentavoided;
	QLabel*		m_label_pet_defense_summary_percentprevented;
	QLabel*		m_label_pet_defense_summary_totaldamage;

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

	QLabel*		m_label_other_totalmobs;

	QLabel*		m_label_other_avgdps;

	QList<CombatOffenseRecord> m_combat_offense_list;
	QList<DotOffenseRecord> m_dot_offense_list;
	QList<PetOffenseRecord> m_pet_offense_list;
	NonmeleeOffenseRecord *m_nonmelee_offense_record;
	CombatDefenseRecord *m_combat_defense_record;
	QList<CombatDefenseRecord> m_combat_pet_defense_list;
	const CombatDefenseRecord *m_combat_pet_defense_current_record;
	QList<CombatMobRecord> m_combat_mob_list;
	QList<CombatOtherRecord> m_combat_other_list;

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
