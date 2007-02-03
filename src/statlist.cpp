/*
 * skilllist.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 *
 *  Copyright 2000-2007 by the respective ShowEQ Developers
 */

#include <qlayout.h>

#include "player.h"
#include "statlist.h"
#include "util.h"
#include "main.h" // for pSEQPrefs & showeq_params

static const char* statNames[] =
{ 
  "HP", "Mana", "Stam", "Exp", "Food", "Watr", "STR", "STA", "CHA", "DEX", 
  "INT", "AGI", "WIS", "MR", "FR", "CR", "DR", "PR", "AC", "ExpAA", "RSV",
};

StatList::StatList(Player* player,
                       QWidget* parent, 
                       const char* name)
  : SEQListView("StatList", parent, name),
    m_player(player)
{
   int i;

   m_guessMaxMana = 0;

   for (i = 0; i < LIST_MAXLIST; i++)
     m_statList[i] = NULL;

   // add columns
   addColumn("Stat");
   addColumn("Val", "Value");
   addColumn("Max");
   addColumn("%", "Percent");

   restoreColumns();

   QString statPrefName;
   for (int nloop = 0; nloop < LIST_MAXLIST; nloop++)
   {
     statPrefName.sprintf("show%s", statNames[nloop]);
     m_showStat[nloop] = pSEQPrefs->getPrefBool(statPrefName, 
						preferenceName(), false);
     updateStat(nloop);
   }

   connect (m_player, SIGNAL(statChanged(int,int,int)), 
	    this, SLOT(statChanged (int,int,int)));
   connect (m_player, SIGNAL(expChangedInt(int,int,int)),
	    this, SLOT(expChanged(int,int,int)));
   connect (m_player, SIGNAL(expAltChangedInt(int,int,int)),
	    this, SLOT(expAltChanged(int,int,int)));
   connect (m_player, SIGNAL(stamChanged(int,int,int,int)),
	    this, SLOT(stamChanged(int,int,int,int)));
   connect (m_player, SIGNAL(manaChanged(uint32_t,uint32_t)),
	    this, SLOT(manaChanged(uint32_t,uint32_t)));
   connect (m_player, SIGNAL(hpChanged(int16_t, int16_t)), 
	    this, SLOT(hpChanged(int16_t, int16_t)));

   // restore the columns
   restoreColumns();
}

StatList::~StatList()
{
}

void StatList::expChanged  (int val, int min, int max) 
{
    if (!m_showStat[LIST_EXP])
      return;

    QString buf;

    m_statList[LIST_EXP]->setText (1, Commanate((uint32_t) (val - min)));
    m_statList[LIST_EXP]->setText (2, Commanate((uint32_t) (max - min)));

    buf = Commanate((uint32_t) ((val - min) / ((max - min)/100))) + " %";

    m_statList[LIST_EXP]->setText (3, buf);
}

void StatList::expAltChanged  (int val, int min, int max) 
{
    if (!m_showStat[LIST_ALTEXP])
      return;

    QString buf;

    m_statList[LIST_ALTEXP]->setText (1, Commanate((uint32_t) (val - min)));
    m_statList[LIST_ALTEXP]->setText (2, Commanate((uint32_t) (max - min)));

    buf = Commanate((uint32_t) ((val - min) / ((max - min)/100))) + " %";

    m_statList[LIST_ALTEXP]->setText (3, buf);
}

void StatList::statChanged (int stat, int val, int max)
{
    if (stat < 0 || stat >= LIST_MAXLIST) 
        return;

    if (!m_showStat[stat])
      return;

  char buf[64];
  sprintf(buf,"%d",val);
  m_statList[stat]->setText (1, buf);
  sprintf(buf,"%d",max);
  m_statList[stat]->setText (2, buf);
  if (max == 0)
     sprintf(buf, "?? %%");
  else
     sprintf(buf,"%d %%",val * 100 / max);

  m_statList[stat]->setText (3, buf);
}

void StatList::hpChanged(int16_t val, int16_t max)
{
  static int old = 0;
    if (!m_showStat[LIST_HP])
      return;

  char buf[64];
  sprintf(buf,"%d",val);
  m_statList[LIST_HP]->setText (1, buf);
  sprintf(buf,"%d",max);
  m_statList[LIST_HP]->setText (2, buf);

  if (max == 0)
     sprintf(buf, "?? %%");
  else
     sprintf(buf,"%d %%",val*100/max);

  m_statList[LIST_HP]->setText (3, buf);

  if(val != old) {
    printf("HP Changed: %+4d %4d\n", val-old,val);
    old = val;
  }
}

void StatList::manaChanged (uint32_t val, uint32_t max)
{
  static uint32_t oldmana = 0;
  if (!m_showStat[LIST_MANA])
    return;

  char buf[64];
  if (val >= 0)
    sprintf(buf, "%d", val);
  else
    sprintf(buf, "N/A");

  m_statList[LIST_MANA]->setText (1, buf);

   if (val > m_guessMaxMana) {
      m_guessMaxMana = val;
   }
   if (max > m_guessMaxMana) {
      m_guessMaxMana = max;
   }

   if (m_guessMaxMana < 0)
     m_guessMaxMana = 0;

   if (m_guessMaxMana == 0)
     return;

   sprintf(buf,"%d",m_guessMaxMana);
   m_statList[LIST_MANA]->setText (2, buf);
   sprintf(buf,"%d %%",(m_guessMaxMana != 0) ? val*100/m_guessMaxMana : 0);
   m_statList[LIST_MANA]->setText (3, buf);
   if(val != oldmana) {
     printf("\e[0;36mMana changed: %+4d %4d\e[0;0m\n", val-oldmana,val);
     oldmana = val;
   }
}

void StatList::stamChanged(int Fval, int Fmax, int Wval, int Wmax) 
{
  char buf[64];

  if (m_showStat[LIST_FOOD])
   {
  	sprintf(buf,"%d",Fval);
  	m_statList[LIST_FOOD]->setText (1, buf);
  	sprintf(buf,"%d",Fmax);
  	m_statList[LIST_FOOD]->setText (2, buf);
  	sprintf(buf,"%d %%",Fval*100/Fmax);
  	m_statList[LIST_FOOD]->setText (3, buf);
   }

  if (m_showStat[LIST_WATR])
   {
  	sprintf(buf,"%d",Wval);
  	m_statList[LIST_WATR]->setText (1, buf);
  	sprintf(buf,"%d",Wmax);
  	m_statList[LIST_WATR]->setText (2, buf);
  	sprintf(buf,"%d %%",Wval*100/Wmax);
  	m_statList[LIST_WATR]->setText (3, buf);
  }
}

void StatList::resetMaxMana(void)
{
  if (!m_showStat[LIST_MANA])
    return;

   char buf[20];
   m_guessMaxMana = 0;
   sprintf(buf,"%d",m_guessMaxMana);
   m_statList[LIST_MANA]->setText (2, buf);
}

void StatList::enableStat(uint8_t stat, bool enabled)
{
  // validate argument
  if (stat >= LIST_MAXLIST)
    return;

  // set the enabled status of the stat
  m_showStat[stat] = enabled;

  QString statPrefName;
  statPrefName.sprintf("show%s", statNames[stat]);

  pSEQPrefs->setPrefBool(statPrefName, "StatList", m_showStat[stat]);

  updateStat(stat);
}

void StatList::updateStat(uint8_t stat)
{
  // validate argument
  if (stat >= LIST_MAXLIST)
    return;
 
  // should this status be displayed?
  if (m_showStat[stat])
  {
    // yes, create the stat item if necessary
    if (m_statList[stat] == NULL)
    {
      QString valStr = "??";
      QString maxStr = "??";
      QString percentStr = "??";

      uint32_t value, max;

      // attempt to get the current attribute values...
      bool valid = m_player->getStatValue(stat, value, max);

      // if the values are valid, use them...
      if (valid)
      {
	switch(stat)
	{
	case LIST_MANA:
	  {
	    valStr = QString::number(value);

	    // attempt to calculate the maximum amount of mana
	    if (value > m_guessMaxMana)
	      m_guessMaxMana = value;
	    if (max > m_guessMaxMana)
	      m_guessMaxMana = max;

	    if (m_guessMaxMana < 0)
	      m_guessMaxMana = 0;

	    // if it's non-zero, we'll use the guess for display
	    if (m_guessMaxMana != 0)
	    {
	      maxStr = QString::number(m_guessMaxMana);
	      percentStr = QString::number(value * 100 / m_guessMaxMana);
	    }
	  }
	  break;
	case LIST_EXP: 
	  {
	    // get the experiance needed for the previous level
	    uint32_t minExp = calc_exp(m_player->level() - 1,
				       m_player->race(),
				       m_player->classVal());

	    valStr = Commanate(value - minExp);
	    maxStr = Commanate(value - max);
	    percentStr = Commanate((value - minExp) / ((max - minExp)/100));
	  }
	  break;
	default: // most can get away with just this...
	  valStr = QString::number(value);
	  maxStr = QString::number(max);
	  
	  if (max != 0)
	    percentStr = QString::number( (value * 100) / max);
	  break;
	}
      }

      // append the % character
      percentStr += " %";

      // create the statistic item
      m_statList[stat] = new Q3ListViewItem(this,
					   statNames[stat] , 
					   valStr, maxStr, percentStr);
      

    }
  }
  else
  {
    // no, delete the status item if necessary
    if (m_statList[stat] != NULL)
    {
      delete m_statList[stat];
      m_statList[stat] = NULL;
    }
  }
}


StatListWindow::StatListWindow(Player* player, 
				 QWidget* parent, const char* name)
  : SEQWindow("StatList", "ShowEQ - Stats", parent, name)
{
  //QVBoxLayout* layout = new QVBoxLayout(this);
  //layout->setAutoAdd(true);
  
  m_statList = new StatList(player, this, name);
  setWidget(m_statList);
}

StatListWindow::~StatListWindow()
{
  delete m_statList;
}

void StatListWindow::savePrefs(void)
{
  // save SEQWindow prefs
  SEQWindow::savePrefs();

  // make the listview save it's prefs
  m_statList->savePrefs();
}

#ifndef QMAKEBUILD
#include "statlist.moc"
#endif

