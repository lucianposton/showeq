/*
 *  experiencelog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#include "experiencelog.h"
#include "interface.h"
#include "util.h"

#include <qgrid.h>
#include <qtimer.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qmessagebox.h>
#include <stdio.h>

#define DEBUGEXP

#undef DEBUGEXP

ExperienceRecord::ExperienceRecord( const QString &mob_name, 
				    int mob_level,
				    long xp_gained, 
				    time_t time, 
				    const QString &zone_name, 
				    EQPlayer* player,
				    GroupMgr* groupMgr) 
: m_player(player), 
  m_group(groupMgr),
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
   EQInterface *parent = (EQInterface*)m_player->parent();
   int ZEM = parent->mapMgr()->getZEM();
   return m_mob_level*m_mob_level*ZEM;
}
 
long ExperienceRecord::getExpValuep() const 
{
   int p_penalty; 
   // WAR and ROG are at 10 since thier EXP is not scaled to compensate
   // for thier bonus
   switch (m_player->getPlayerClass())
   {
      case 1 : p_penalty = 10; break; // WAR
      case 2 : p_penalty = 10; break; // CLR
      case 3 : p_penalty = 14; break; // PAL
      case 4 : p_penalty = 14; break; // RNG
      case 5 : p_penalty = 14; break; // SHD
      case 6 : p_penalty = 10; break; // DRU
      case 7 : p_penalty = 12; break; // MNK
      case 8 : p_penalty = 14; break; // BRD
      case 9 : p_penalty = 10; break; // ROG
      case 10: p_penalty = 10; break; // SHM
      case 11: p_penalty = 11; break; // NEC
      case 12: p_penalty = 11; break; // WIZ
      case 13: p_penalty = 11; break; // MAG
      case 14: p_penalty = 11; break; // ENC
      default: /* why are we here? */
         p_penalty = 10; break; 
   }
   int baseExp = getExpValueZEM();
   return (int)((float)baseExp*((float)p_penalty/(float)10));
}

long ExperienceRecord::getExpValueg() const 
{
   int pExp = getExpValuep();
   int gbonus;
   int myLevel = m_player->getPlayerLevel();
   int group_ag;

   gbonus = m_group->groupPercentBonus();

   /* calculate group aggregate level */
   printf("GROUPSIZE: %d\n", m_group->groupSize());
   group_ag = m_group->totalLevels();
   printf("MY Level: %d\n GroupTot: %d\n BONUS   :%d\n", 
	  myLevel, group_ag, gbonus);

   return (int) (((float)pExp)*((float)gbonus/(float)100)*((float)myLevel/(float)group_ag));
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

   if (logfd > -1)
      close(logfd);
}

ExperienceWindow::ExperienceWindow( EQPlayer* player, 
				    GroupMgr* groupMgr) 
  : m_player(player),
    m_group(groupMgr)
{
  /* Hopefully this is only called once to set up the window,
     so this is a good place to initialize some things which
     otherwise won't be. */

   m_ratio = 1;
   m_timeframe = 0;
   m_calcZEM=0;
   m_ZEMviewtype = 0;

   QString section = "Experience";
   QWidget::setCaption(pSEQPrefs->getPrefString("Caption", section, 
						"ShowEQ - Experience"));

   restoreFont();

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

   m_menu_bar = new QMenuBar( this );
   m_menu_bar->insertItem( "&View", m_view_menu );

   m_layout = new QVBoxLayout( this );
   m_layout->addSpacing( m_menu_bar->height() + 5 );

   QGroupBox *listGBox = new QVGroupBox( "Experience Log", this );
   m_layout->addWidget( listGBox );

   m_exp_listview = new QListView(listGBox);
   m_exp_listview->addColumn("Time");
   m_exp_listview->addColumn("Mob");
   m_exp_listview->addColumn("Level");
   m_exp_listview->addColumn("Base Exp");
   m_exp_listview->addColumn("ZEM total");
   m_exp_listview->addColumn("Class total");
   m_exp_listview->addColumn("Group total");
   m_exp_listview->addColumn("Experience Gained");
   
   m_exp_listview->setMinimumSize( m_exp_listview->sizeHint().width(),
      200 );

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

//   setMinimumSize( sizeHint() );

   logfd = open(LOGDIR "/exp.log", O_RDWR | O_CREAT | O_APPEND);
   if (logfd < 0)
   {
      m_log_exp = 0;
      printf("Error opening exp.log, no exp will be logged this session\n");
   }
   else
   {
      m_log_exp = 1;
      logstr = fdopen(logfd, "a");
   }

}


void ExperienceWindow::setCaption(const QString& text)
{
  // set the caption
  QWidget::setCaption(text);

  // set the preference
  pSEQPrefs->setPrefString("Caption", "Experience", caption());
}

void ExperienceWindow::setWindowFont(const QFont& font)
{
  // set the font preference
  pSEQPrefs->setPrefFont("Font", "Experience", font);

  // restore the font to the preference
  restoreFont();
}

void ExperienceWindow::restoreFont()
{
  QString section = "Experience";
  // set the applications default font
  if (pSEQPrefs->isPreference("Font", section))
  {
    // use the font specified in the preferences
    QFont font = pSEQPrefs->getPrefFont("Font", section);
    setFont( font);
  }
}

void ExperienceWindow::addExpRecord(const QString &mob_name,
   int mob_level, long xp_gained, QString zone_name ) 
{

   ExperienceRecord *xp = new ExperienceRecord( mob_name, mob_level,
      xp_gained, time(0), zone_name, m_player, m_group);

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

   if (m_calcZEM)
   {
      calculateZEM(xp_gained, mob_level);
      m_calcZEM = 0;
      m_view_menu->setItemChecked(m_view_menu->idAt(9), false);
   }   
   s_xp_value.setNum(xp->getExpValue());
   QString s_xp_valueZEM;
   EQInterface *parent = (EQInterface*)m_player->parent();
   switch (m_ZEMviewtype) {
      case 1 : s_xp_valueZEM.setNum(parent->mapMgr()->getZEM()); break;
      case 2 : s_xp_valueZEM.setNum(((float)(parent->mapMgr()->getZEM()-75)/(float)75)*100);
         break;
      default: s_xp_valueZEM.setNum(xp->getExpValueZEM()); break;
   }
   QString s_xp_valuep;
   s_xp_valuep.setNum(xp->getExpValuep());
   QString s_xp_valueg;
   s_xp_valueg.setNum(xp->getExpValueg());

   char s_time[64];
   time_t timev = xp->getTime();
   strftime(s_time, 64, "%m/%d %H:%M:%S", localtime( &timev ));

   /* Update suggested by Shag */
   QListViewItem *new_exp_entry = 
     new QListViewItem( m_exp_listview, s_time, s_mob_name, 
			s_mob_level, s_xp_value, s_xp_valueZEM,
			s_xp_valuep, s_xp_valueg, s_xp_gained );

   m_exp_listview->insertItem( new_exp_entry );
   m_exp_listview->setSelected( new_exp_entry, TRUE );
   m_exp_listview->ensureItemVisible( new_exp_entry );

   // Initial work on new logging mechanism with more data
   FILE* newlogfp = NULL;

   // open the file for append
   newlogfp = fopen(LOGDIR "/newexp.log", "a");

   if (newlogfp != NULL)
   {   
      // append a new record entry

      fprintf(newlogfp, 
              "0\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%d", 
              s_time, (const char*)s_mob_name, mob_level, 
              (const char*)s_xp_value, (const char*)s_xp_valueZEM,
              (const char*)s_xp_valuep, (const char*)s_xp_valueg,
              (const char*)s_xp_gained, 
              (const char*)m_player->getPlayerName(),
              (const char*)m_player->getPlayerLastName(),
              m_player->getPlayerLevel(), 
              m_player->getPlayerClass(), m_group->groupSize());

      // continue with info for group members
      for (int i=0; i < m_group->groupSize(); i++)
        fprintf(newlogfp, "\t%d", m_group->memberBySlot(i)->level());

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
      s_kills_to_level.setNum( exp_remaining / (total_exp/mob_count) );
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
   m_view_menu->setItemChecked(m_view_menu->idAt(9), true);
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
   switch( QMessageBox::information( this, "ShowEQ",
      "This function will clear all data listed in the experience "
      "log.  Do you want to continue?",
      "&OK", "&Cancel", QString::null, 1, 1 ) ) {

      case 0:
        m_exp_list.clear();
        m_exp_listview->clear();
        break;

      default:
        break;
    }
}

void ExperienceWindow::logexp(long xp_gained, int mob_level) 
{
   if (!m_log_exp || logstr == NULL) /* sanity */
      return;

   fprintf(logstr, "1 %d %d %ld %d %d %d", 
	   (int)time(NULL), mob_level, 
      xp_gained, m_player->getPlayerLevel(), 
      m_player->getPlayerClass(), m_group->groupSize());

   for (int i=0; i < m_group->groupSize(); i++)
      fprintf(logstr, " %d", m_group->memberBySlot(i)->level());

   fprintf(logstr, "\n"); 
   fflush(logstr); /* some people like to tail -f and see live data :) */
}

void ExperienceWindow::calculateZEM(long xp_gained, int mob_level) 
{
   int gbonus=100;
   int penalty; 
   int myLevel = m_player->getPlayerLevel();
   int group_ag;
   EQInterface *parent = (EQInterface*)m_player->parent();
   gbonus = m_group->groupPercentBonus();
   group_ag = m_group->totalLevels();
   if (m_group->groupSize())
   {
      printf("MY Level: %d\n GroupTot: %d\n BONUS   :%d\n", 
	     myLevel, group_ag, gbonus);
   }
   // WAR and ROG are at 10 since thier EXP is not scaled to compensate
   // for thier bonus
   switch (m_player->getPlayerClass())
   {
      case 1 : penalty = 10; break; // WAR
      case 2 : penalty = 10; break; // CLR
      case 3 : penalty = 14; break; // PAL
      case 4 : penalty = 14; break; // RNG
      case 5 : penalty = 14; break; // SHD
      case 6 : penalty = 10; break; // DRU
      case 7 : penalty = 12; break; // MNK
      case 8 : penalty = 14; break; // BRD
      case 9 : penalty = 10; break; // ROG
      case 10: penalty = 10; break; // SHM
      case 11: penalty = 11; break; // NEC
      case 12: penalty = 11; break; // MAG
      case 13: penalty = 11; break; // ENC
      default: /* why are we here? */
         penalty = 10; break; 
   }
   unsigned char ZEM = (unsigned char) ((float)xp_gained*((float)((float)group_ag/(float)myLevel)*(float)((float)100/(float)gbonus))*((float)1/(float)(mob_level*mob_level))*((float)10/(float)penalty));
   printf("xpgained: %ld\ngroup_ag: %d\nmyLevel: %d\ngbonus: %d\nmob_level: %d\npenalty: %d\n", xp_gained, group_ag, myLevel, gbonus, mob_level, penalty);
   printf("ZEM - ZEM - ZEM ===== %d\n", ZEM);
   parent->mapMgr()->setZEM(ZEM);
}
