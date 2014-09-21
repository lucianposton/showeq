/*
 * experiencelog.h
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#ifndef EXPERIENCELOG_H
# define EXPERIENCELOG_H

#include "seqwindow.h"
#include "seqlistview.h"

# include <qobject.h>
# include <qwidget.h>
# include <qlist.h>
# include <q3listview.h>
# include <qcombobox.h>
# include <qlabel.h>
# include <qlayout.h>
# include <qmenubar.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3VBoxLayout>
#include <Q3PopupMenu>
#include <Q3PtrList>

#include <stdint.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdio.h>

//----------------------------------------------------------------------
// forward declarations
class GroupMgr;
class Player;
class DataLocationMgr;
class ZoneMgr;
	    
//----------------------------------------------------------------------
// ExperienceRecord
class ExperienceRecord 
{
public:

   ExperienceRecord(const QString &mob_name, int mob_level, long xp_gained,
		    time_t time, const QString &zone_name, 
		    uint8_t classVal, uint8_t level, float zem, 
		    float totalLevels, float groupBonus);

   const QString &getMobName() const;
   int getMobLevel() const;
   long getExpGained() const;
   long getExpValue() const; // BASE calculated from spawn level
   long getExpValueZEM() const; // calculated from spawn level and ZEM
   long getExpValuep() const; // calculated from spawn level and ZEM + penalty
   long getExpValueg() const; // calculated from spawn level and all + group
   time_t getTime()  const;
   const QString &getZoneName() const;

private:
   uint8_t m_class;
   uint8_t m_level;
   float m_zem;
   float m_totalLevels;
   float m_groupBonus;
   GroupMgr* m_group;
   QString m_zone_name;
   QString m_mob_name;
   int m_mob_level;
   long m_xp_gained;
   time_t m_time;

};

//----------------------------------------------------------------------
// ExperienceWindow
class ExperienceWindow : public SEQWindow
{
   Q_OBJECT

public:
   ExperienceWindow(const DataLocationMgr* dataLocMgr, 
		    Player* player, GroupMgr* g, ZoneMgr* zoneMgr,
		    QWidget* parent = 0, const char* name = 0 );
   ~ExperienceWindow();

public slots:

   virtual void savePrefs(void);
   void updateAverage( );
   void addExpRecord( const QString &mob_name, int mob_level, long xp_gained,
      QString zone_name );
   void viewRatePerHour();
   void viewRatePerMinute();
   void viewAll();
   void view15Minutes();
   void view30Minutes();
   void view60Minutes();
   void viewClear();
   void calcZEMNextKill();
   void viewZEMraw();
   void viewZEMpercent();
   void viewZEMcalculated();
   void clear(void);

private:

   void resizeEvent( QResizeEvent * );
   void calculateZEM(long xp_gained, int mob_level);
   void logexp(long xp_gained, int mob_level);

   // Need to grab xp totals from here
   const DataLocationMgr* m_dataLocMgr;
   QString m_newExpLogFile;
   Player* m_player;
   GroupMgr* m_group;
   ZoneMgr* m_zoneMgr;

   Q3VBoxLayout *m_layout;

   SEQListView *m_exp_listview;

   QLabel *m_experience_rate_label;
   QLabel *m_total_received, *m_mob_count, *m_average_per_mob,
      *m_experience_remaining, *m_play_time,
      *m_experience_rate, *m_kills_to_level, *m_time_to_level;

   Q3PtrList<ExperienceRecord> m_exp_list;

   QMenuBar *m_menu_bar;
   Q3PopupMenu *m_view_menu, *m_exp_rate_menu, *m_ZEM_menu;

   int m_timeframe;
   int m_ratio;
   int m_calcZEM;
   int m_ZEMviewtype;
   int m_log_exp;
   FILE* m_log;

};

#endif // EQINT_H
