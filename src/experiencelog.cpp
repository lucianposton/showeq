/*
 *  experiencelog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */


#include "experiencelog.h"
#include "map.h"
#include "util.h"
#include "group.h"
#include "player.h"
#include "datalocationmgr.h"
#include "diagnosticmessages.h"
#include "zonemgr.h"

#include <stdio.h>
#include <time.h>

#include <qgrid.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qmessagebox.h>
#include <qfileinfo.h>

#include <cmath>

#define DEBUGEXP

#undef DEBUGEXP

ExperienceRecord::ExperienceRecord( const QString &mob_name, 
				    int mob_level,
				    long xp_gained, 
				    time_t time, 
				    const QString &zone_name, 
				    long penalty, uint8_t level, float zem,
				    float totalLevels, 
				    float groupBonus) 
  : m_penalty(penalty),
    m_level(level),
    m_zem(zem),
    m_totalLevels(totalLevels),
    m_groupBonus(groupBonus),
    m_zone_name(zone_name), 
    m_mob_name(mob_name),
    m_mob_level(mob_level),
    m_xp_gained(xp_gained),
    m_time(time)
{

}

const QString &ExperienceRecord::getMobName() const 
{
   return m_mob_name;

}

int ExperienceRecord::getMobLevel() const 
{
   return m_mob_level;

}

long ExperienceRecord::getExpGained() const 
{
   return m_xp_gained;

}

long ExperienceRecord::getExpValue() const 
{
   // TODO: This isn't right for all mob levels!
   //     But casey's working on it!
   return m_mob_level*m_mob_level*75;
}

long ExperienceRecord::getExpValueZEM() const 
{
  long ZEM = long(m_zem * 100);
  return ZEM*m_mob_level*m_mob_level;
}
 
long ExperienceRecord::getExpValuep() const 
{
   long baseExp = getExpValueZEM();
   return (long)((float)baseExp*((float)m_penalty/(float)10));
}

long ExperienceRecord::getExpValueg() const 
{
   long pExp = getExpValuep();

   return (long)(((float)pExp)*m_groupBonus*((float)m_level/m_totalLevels));
}

time_t ExperienceRecord::getTime() const 
{
   return m_time;

}

const QString &ExperienceRecord::getZoneName() const 
{
   return m_zone_name;

}

ExperienceWindow::~ExperienceWindow() 
{
  if (m_log)
    ::fclose(m_log);
}

ExperienceWindow::ExperienceWindow(const DataLocationMgr* dataLocMgr, 
				   Player* player, GroupMgr* groupMgr,
				   ZoneMgr* zoneMgr,
				   QWidget* parent, const char* name) 
  : SEQWindow("Experience", "ShowEQ - Experience", parent, name),
    m_dataLocMgr(dataLocMgr),
    m_player(player),
    m_group(groupMgr),
    m_zoneMgr(zoneMgr)
{
  /* Hopefully this is only called once to set up the window,
     so this is a good place to initialize some things which
     otherwise won't be. */

   m_ratio = 1;
   m_timeframe = 0;
   m_calcZEM=0;
   m_ZEMviewtype = 0;

   m_view_menu = new QPopupMenu( this );
   m_view_menu->insertItem( "&All Mobs", this, SLOT(viewAll()) );
   m_view_menu->insertItem( "Previous &15 Minutes", this, SLOT(view15Minutes()) );
   m_view_menu->insertItem( "Previous &30 Minutes", this, SLOT(view30Minutes()) );
   m_view_menu->insertItem( "Previous &60 Minutes", this, SLOT(view60Minutes()) );
   m_view_menu->setItemChecked( m_view_menu->idAt(0), true );

   m_view_menu->insertSeparator();
   m_exp_rate_menu = new QPopupMenu( this );
   m_exp_rate_menu->insertItem( "per &minute", this, SLOT(viewRatePerMinute()) );
   m_exp_rate_menu->insertItem( "per &hour", this, SLOT(viewRatePerHour()) );
   m_exp_rate_menu->setItemChecked( m_exp_rate_menu->idAt(0), true );
   m_view_menu->insertItem( "Experience &Rate", m_exp_rate_menu );

   m_view_menu->insertSeparator();
   m_view_menu->insertItem( "Clear Kills", this, SLOT(viewClear()) );

   m_view_menu->insertSeparator();
   m_ZEM_menu = new QPopupMenu( this );
   m_ZEM_menu->insertItem( "Calculated Value", this, SLOT(viewZEMcalculated()) );
   m_ZEM_menu->insertItem( "Raw Value", this, SLOT(viewZEMraw()) );
   m_ZEM_menu->insertItem( "Percent Bonus", this, SLOT(viewZEMpercent()) );
   m_ZEM_menu->setItemChecked( m_ZEM_menu->idAt(0), true );
   m_view_menu->insertItem( "ZEM View Options", m_ZEM_menu );
   m_view_menu->insertItem( "Calculate ZEM on next kill", this, SLOT(calcZEMNextKill()) );

   m_options_menu = new QPopupMenu( this );
   m_options_menu->insertItem( "&Enable Class EXP Penalty", this, SLOT(toggleExpPenalty()) );
   m_options_menu->setItemChecked(m_options_menu->idAt(0), m_player->isClassExpPenaltyActive());

   m_menu_bar = new QMenuBar( this );
   m_menu_bar->insertItem( "&View", m_view_menu );
   m_menu_bar->insertItem( "&Options", m_options_menu );

   m_layout = new QVBoxLayout(boxLayout());

   //m_layout->addSpacing( m_menu_bar->height() + 5 );
   m_layout->addWidget(m_menu_bar);

   QGroupBox *listGBox = new QVGroupBox( "Experience Log", this );
   m_layout->addWidget( listGBox );

   m_exp_listview = new SEQListView(preferenceName(), listGBox);
   m_exp_listview->addColumn("Time");
   m_exp_listview->setColumnAlignment(0, Qt::AlignLeft);
   m_exp_listview->addColumn("Mob");
   m_exp_listview->setColumnAlignment(1, Qt::AlignLeft);
   m_exp_listview->addColumn("Level");
   m_exp_listview->setColumnAlignment(2, Qt::AlignRight);
   m_exp_listview->addColumn("Base Exp");
   m_exp_listview->setColumnAlignment(3, Qt::AlignRight);
   m_exp_listview->addColumn("ZEM Raw");
   m_exp_listview->setColumnAlignment(4, Qt::AlignRight);
   m_exp_listview->addColumn("ZEM %");
   m_exp_listview->setColumnAlignment(5, Qt::AlignRight);
   m_exp_listview->addColumn("ZEM Exp");
   m_exp_listview->setColumnAlignment(6, Qt::AlignRight);
   m_exp_listview->addColumn("Class Exp");
   m_exp_listview->setColumnAlignment(7, Qt::AlignRight);
   m_exp_listview->addColumn("Group Exp");
   m_exp_listview->setColumnAlignment(8, Qt::AlignRight);
   m_exp_listview->addColumn("Exp Gained");
   m_exp_listview->setColumnAlignment(9, Qt::AlignRight);
   
   m_exp_listview->restoreColumns();

   //m_exp_listview->setMinimumSize( m_exp_listview->sizeHint().width(),
   //   200 );

   QGroupBox *statsGBox = new QVGroupBox( "Statistics", this );
   m_layout->addWidget( statsGBox );
   
   QGrid *statsGrid = new QGrid( 4, statsGBox );

   new QLabel( "Total Experience Received:",
      statsGrid );
   m_total_received = new QLabel( statsGrid );

   new QLabel( "Play Time:", statsGrid );
   m_play_time = new QLabel( statsGrid );

   new QLabel( "Total Mobs Killed:", statsGrid );
   m_mob_count = new QLabel( statsGrid );

   m_experience_rate_label = new QLabel( "Experience Rate (per minute):", statsGrid );
   m_experience_rate = new QLabel( statsGrid );

   new QLabel( "Average Experience per Mob:",
      statsGrid );
   m_average_per_mob = new QLabel( statsGrid );

   new QLabel( "Estimated Kills To Level:",
      statsGrid );
   m_kills_to_level = new QLabel( statsGrid );

   new QLabel( "Experience Remaining:",
      statsGrid );
   m_experience_remaining = new QLabel( statsGrid );

   new QLabel( "Estimated Time To Level:",
      statsGrid );
   m_time_to_level = new QLabel( statsGrid );

   // ewww, why can't we just get it from QGrid? :(
   ((QGridLayout *)statsGrid->layout())->setColStretch( 1, 1 );
   ((QGridLayout *)statsGrid->layout())->setColStretch( 3, 1 );
   statsGrid->layout()->setSpacing( 5 );

   updateAverage( );

   // timer to update the average xp
   QTimer *timer = new QTimer( this );
   connect( timer, SIGNAL(timeout()), SLOT(updateAverage()));
   timer->start(15*1000); // calculate every 15 seconds

   QFileInfo fileInfo = m_dataLocMgr->findWriteFile("logs", "exp.log");

   m_log = fopen(fileInfo.absFilePath(), "a");
   if (m_log == 0)
   {
      m_log_exp = 0;
      seqWarn("Error opening exp.log, no exp will be logged this session");
   }
   else
   {
     m_log_exp = 1;
   }

   fileInfo = m_dataLocMgr->findWriteFile("logs", "newexp.log");

   m_newExpLogFile = fileInfo.absFilePath();

   // Clear the exp list on removes and deletes.
   m_exp_list.setAutoDelete(true);  
}

void ExperienceWindow::savePrefs()
{
  // save the SEQWindow's prefs
  SEQWindow::savePrefs();

  // save the listview's prefs
  m_exp_listview->savePrefs();
}

void ExperienceWindow::addExpRecord(const QString &mob_name,
   int mob_level, long xp_gained, QString zone_name ) 
{

   ExperienceRecord *xp = 
     new ExperienceRecord(mob_name, mob_level, xp_gained, time(0), zone_name, 
			  m_player->getClassExpPenalty(), m_player->level(), 
			  m_zoneMgr->zoneExpMultiplier(), 
			  m_group->totalLevels(),
			  m_group->groupBonus());

#ifdef DEBUGEXP
   resize( sizeHint() );
  qDebug("ExperienceWindow::addExpRecord()  '%s', lvl %d, exp %d",
      mob_name.ascii(), mob_level, xp_gained);
#endif

   if (m_log_exp)
      logexp(xp_gained, mob_level);

   m_exp_list.append( xp );

   // convert everything to string representations for the list view
   QString s_mob_name = mob_name;
   QString s_mob_level;
   s_mob_level.setNum(mob_level);
   QString s_xp_gained;
   s_xp_gained.setNum(xp_gained);
   QString s_xp_value;

   if (m_calcZEM && mob_level > 0)
   {
      calculateZEM(xp_gained, mob_level);
      m_calcZEM = 0;
      m_view_menu->setItemChecked(m_view_menu->idAt(10), false);
   }   
   s_xp_value.setNum(xp->getExpValue());
   const QString s_zem_value = doubleToQString(m_zoneMgr->zoneExpMultiplier(), 2, true);
   QString s_zem_bonus;
   s_zem_bonus.setNum((int)(zemToExpBonus(m_zoneMgr->zoneExpMultiplier())*100));
   QString s_xp_valueZEM;
   s_xp_valueZEM.setNum(xp->getExpValueZEM());

   QString s_xp_valuep;
   s_xp_valuep.setNum(xp->getExpValuep());
   QString s_xp_valueg;
   s_xp_valueg.setNum(xp->getExpValueg());

   char s_time[64];
   time_t timev = xp->getTime();
   strftime(s_time, 64, "%m/%d %H:%M:%S", localtime( &timev ));

   SEQListViewItem<>* new_exp_entry = new SEQListViewItem<>(
           m_exp_listview,
           s_time,
           s_mob_name,
           s_mob_level,
           s_xp_value,
           s_zem_value,
           s_zem_bonus,
           s_xp_valueZEM,
           s_xp_valuep);
   new_exp_entry->setText(8, s_xp_valueg);
   new_exp_entry->setText(9, s_xp_gained);
   new_exp_entry->setColComparator(2, SEQListViewItemCompareInt);
   new_exp_entry->setColComparator(3, SEQListViewItemCompareInt);
   new_exp_entry->setColComparator(4, SEQListViewItemCompareInt);
   new_exp_entry->setColComparator(5, SEQListViewItemCompareDouble);
   new_exp_entry->setColComparator(6, SEQListViewItemCompareInt);
   new_exp_entry->setColComparator(7, SEQListViewItemCompareInt);
   new_exp_entry->setColComparator(8, SEQListViewItemCompareInt);
   new_exp_entry->setColComparator(9, SEQListViewItemCompareInt);

   m_exp_listview->insertItem( new_exp_entry );
   m_exp_listview->setSelected( new_exp_entry, TRUE );
   m_exp_listview->ensureItemVisible( new_exp_entry );

   // Initial work on new logging mechanism with more data
   FILE* newlogfp = NULL;

   // open the file for append
   newlogfp = fopen(m_newExpLogFile, "a");

   if (newlogfp != NULL)
   {   
      // append a new record entry

      fprintf(newlogfp, 
              "0\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%lu",
              s_time,
              (const char*)s_mob_name,
              mob_level,
              (const char*)s_xp_value,
              (const char*)s_xp_valueZEM,
              (const char*)s_xp_valuep,
              (const char*)s_xp_valueg,
              (const char*)s_xp_gained,
              (const char*)m_player->name(),
              (const char*)m_player->lastName(),
              m_player->level(),
              m_player->classVal(),
              m_group->groupSize());

      uint8_t memberLevel;
      // continue with info for group members
      for (int i=0; i < MAX_GROUP_MEMBERS; i++)
      {
          memberLevel = m_group->memberLevelBySlot(i);
          if (memberLevel)
              fprintf(newlogfp, "\t%d", memberLevel);
      }

      // finish the record with a line
      fprintf(newlogfp, "\n"); 

      // close the file (so multiple ShowEQ instances can be up at once)
      fclose(newlogfp);
   }

   // and update the average display
   updateAverage();

}

void ExperienceWindow::updateAverage( ) 
{
   static bool force = true;

   // don't worry about updating the window if it's not even visible,
   // unless 
   if ( !force && !isVisible() )
      return;

   force = false;

   // calculate the earliest time to accept
   time_t time_cutoff = 0;
   double total_exp = 0;

   if (m_timeframe)
      time_cutoff = time(0) - (m_timeframe*60);

   // start at the end, add up the xp & mob count until we hit the
   // beginning of list
   // or the time cutoff
   QListIterator<ExperienceRecord> it(m_exp_list);

   int mob_count = 0;
   time_t first_kill_time = 0;

   it.toLast();
   while ( it.current() && it.current()->getTime() >= time_cutoff ) 
   {

      total_exp+=it.current()->getExpGained();
      mob_count++;

      if ( it.current()->getTime() < first_kill_time || !first_kill_time )
         first_kill_time = it.current()->getTime();

      --it;
   }

   // calculate the number of minutes that have passed
   double minutes = ( time(0) - first_kill_time ) / 60;
      
   if (!first_kill_time || minutes < 1)
      minutes = 0;

   // calculate and display the averages
   QString s_avg_exp;
   QString s_play_time;
   double avg_exp;

   if (!m_timeframe) {
      avg_exp = ( minutes ? total_exp/minutes : 0 );
      s_avg_exp=Commanate( (uint32_t)(avg_exp * m_ratio) );
      s_play_time.setNum( minutes );
      s_play_time+=" min";
   } 
   else 
   {
      avg_exp = total_exp/m_timeframe;
      s_avg_exp=Commanate( (uint32_t)(avg_exp * m_ratio) );
      s_play_time.setNum( m_timeframe );
      s_play_time+=" min";
   }
      
   if (m_ratio == 1)
      m_experience_rate_label->setText( "Experience Rate (per minute):" );
   else if (m_ratio == 60)
      m_experience_rate_label->setText( "Experience Rate (per hour):" );

   QString s_total_exp;
   s_total_exp=Commanate( (uint32_t)total_exp );

   QString s_mob_count;
   s_mob_count.setNum( (uint32_t)mob_count );
   
   QString s_mob_avg_exp;
   if (mob_count)
      s_mob_avg_exp=Commanate( (uint32_t)(total_exp/mob_count) );
   else
      s_mob_avg_exp="0";

   int exp_remaining;
   
   if ( (m_player->getMaxExp() > m_player->getCurrentExp()) &&
	(m_player->getCurrentExp() > 0))
   {
     /* since currentExp is calculated before maxExp when the decoder
	is broken, sometimes maxExp ends up set to zero or undefined.
	This can result in strange exp_remaining values, so some sanity
	checks have been added.  cpphack */
     exp_remaining = m_player->getMaxExp() - m_player->getCurrentExp();
   } 
   else
     exp_remaining = 0;

   QString s_exp_remaining;

   /* This now checks if the exp value is sensible, and if not
      it is displayed as "unknown". cpphack */
   if (exp_remaining > 0)
     s_exp_remaining=Commanate( exp_remaining );
   else
     s_exp_remaining="unknown";

   QString s_kills_to_level;
   if (mob_count)
      s_kills_to_level.setNum( std::ceil(exp_remaining / (total_exp/mob_count)) );
   else
      s_kills_to_level="unknown";

   int time_to_level;
   (!avg_exp) ? time_to_level = 0 : time_to_level = (int)(exp_remaining / avg_exp);
   QString s_time_to_level;

   if ( avg_exp ) {
      if (time_to_level > 120)
         s_time_to_level = QString("%1 hours, %2 minutes")
            .arg( (int)(time_to_level/60)).arg( time_to_level % 60 );
      else if (time_to_level > 60)
         s_time_to_level = QString("1 hour, %2 minutes")
            .arg( time_to_level % 60 );
      else
         s_time_to_level = QString("%1 minutes").arg( (int)time_to_level );
   } else
      s_time_to_level="unknown";

   m_total_received->setText( s_total_exp );
   m_mob_count->setText( s_mob_count );
   m_average_per_mob->setText( s_mob_avg_exp );
   m_experience_remaining->setText( s_exp_remaining );
   
   m_play_time->setText( s_play_time );
   m_experience_rate->setText( s_avg_exp );
   m_kills_to_level->setText( s_kills_to_level );
   m_time_to_level->setText( s_time_to_level );

}

void ExperienceWindow::resizeEvent ( QResizeEvent *e ) 
{
  //   int mh = m_menu_bar->height() + 4;

   //m_layout->setGeometry( 0, mh, e->size().width(),  e->size().height()-mh );

}

void ExperienceWindow::toggleExpPenalty()
{
    const bool val = !m_options_menu->isItemChecked(m_options_menu->idAt(0));
    m_options_menu->setItemChecked(m_options_menu->idAt(0), val);
    m_player->setUseClassExpPenalty(val);
}

void ExperienceWindow::viewAll() 
{
   m_timeframe = 0;
   updateAverage();
   m_view_menu->setItemChecked(m_view_menu->idAt(0), true);
   m_view_menu->setItemChecked(m_view_menu->idAt(1), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(2), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(3), false);

}

void ExperienceWindow::view15Minutes() 
{
   m_timeframe = 15;
   updateAverage();
   m_view_menu->setItemChecked(m_view_menu->idAt(0), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(1), true);
   m_view_menu->setItemChecked(m_view_menu->idAt(2), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(3), false);

}

void ExperienceWindow::view30Minutes() 
{
   m_timeframe = 30;
   updateAverage();
   m_view_menu->setItemChecked(m_view_menu->idAt(0), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(1), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(2), true);
   m_view_menu->setItemChecked(m_view_menu->idAt(3), false);

}

void ExperienceWindow::view60Minutes() 
{
   m_timeframe = 60;
   updateAverage();
   m_view_menu->setItemChecked(m_view_menu->idAt(0), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(1), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(2), false);
   m_view_menu->setItemChecked(m_view_menu->idAt(3), true);

}

void ExperienceWindow::viewRatePerHour() 
{
   m_ratio = 60;
   updateAverage();
   m_exp_rate_menu->setItemChecked(m_exp_rate_menu->idAt(0), false);
   m_exp_rate_menu->setItemChecked(m_exp_rate_menu->idAt(1), true);

}

void ExperienceWindow::viewRatePerMinute() 
{
   m_ratio = 1;
   updateAverage();
   m_exp_rate_menu->setItemChecked(m_exp_rate_menu->idAt(0), true);
   m_exp_rate_menu->setItemChecked(m_exp_rate_menu->idAt(1), false);

}

void ExperienceWindow::calcZEMNextKill() 
{
   m_calcZEM = 1;
   m_view_menu->setItemChecked(m_view_menu->idAt(10), true);
}

void ExperienceWindow::viewZEMcalculated() 
{
   m_ZEMviewtype = 0;
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(0), true);
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(1), false);
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(2), false);

}

void ExperienceWindow::viewZEMraw() 
{
   m_ZEMviewtype = 1;
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(0), false);
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(1), true);
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(2), false);

}

void ExperienceWindow::viewZEMpercent() 
{
   m_ZEMviewtype = 2;
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(0), false);
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(1), false);
   m_ZEM_menu->setItemChecked(m_ZEM_menu->idAt(2), true);

}

void ExperienceWindow::viewClear() 
{
   if (QMessageBox::information( this, "ShowEQ",
      "This function will clear all data listed in the experience "
      "log.  Do you want to continue?",
      "&OK", "&Cancel", QString::null, 1, 1) == 0) 
   {
     clear();
   }
}

void ExperienceWindow::clear(void)
{
  m_exp_list.clear();
  m_exp_listview->clear();
}

void ExperienceWindow::logexp(long xp_gained, int mob_level) 
{
   if (!m_log_exp || (!m_log)) /* sanity */
      return;

   fprintf(m_log, "1 %d %d %ld %d %d %lu", 
	   (int)time(NULL), mob_level, 
      xp_gained, m_player->level(), 
      m_player->classVal(), m_group->groupSize());

   for (int i=0; i < MAX_GROUP_MEMBERS; i++)
   {
     uint8_t memberLevel = m_group->memberLevelBySlot(i);
     if (memberLevel)
       fprintf(m_log, " %d", memberLevel);
   }

   fprintf(m_log, "\n"); 
   fflush(m_log); /* some people like to tail -f and see live data :) */
}

void ExperienceWindow::calculateZEM(long xp_gained, int mob_level) 
{
   const float gbonus = m_group->groupBonus();
   const int penalty = m_player->getClassExpPenalty();
   const int myLevel = m_player->level();
   const int group_ag = m_group->totalLevels();

   if (0 != m_group->groupSize())
   {
       seqInfo("MY Level: %d GroupTot: %d BONUS   :%.2f", 
               myLevel, group_ag, gbonus);
   }

   float ZEM = (float)xp_gained
       * ((float)group_ag/(float)myLevel)
       / gbonus
       / (float)(mob_level*mob_level)
       * (10.0/(float)penalty);
   seqInfo("xpgained: %ld group_ag: %d myLevel: %d "
           "gbonus: %.2f mob_level: %d penalty: %d ",
           xp_gained, group_ag, myLevel,
           gbonus, mob_level, penalty);
   seqInfo("ZEM - ZEM - ZEM ===== %.2f ", ZEM);
}

#include "experiencelog.moc"
