/*
 * skilllist.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <qheader.h>

#include "player.h"
#include "statlist.h"
#include "util.h"
#include "main.h" // for pSEQPrefs & showeq_params

static const char* statNames[] =
{ 
  "HP", "Mana", "Stam", "Exp", "Food", "Watr", "STR", "STA", "CHA", "DEX", 
  "INT", "AGI", "WIS", "MR", "FR", "CR", "DR", "PR", "AC", "ExpAA", "RSV",
};

EQStatList::EQStatList(EQPlayer* player,
                       QWidget* parent, 
                       const char* name)
                      : QListView(parent, name),
                        m_pPlayer(player)
{
   int i;

   m_guessMaxMana = 0;

   for (i = 0; i < LIST_MAXLIST; i++)
     m_statList[i] = NULL;

   QString section = "StatList";
#if QT_VERSION >= 210
   setShowSortIndicator(TRUE);
#endif
   setRootIsDecorated(false);
   QListView::setCaption(pSEQPrefs->getPrefString("Caption", section, 
						  "ShowEQ - Stats"));

   // set font and add columns
   setFont(QFont("Helvetica", showeq_params->fontsize));
   addColumn("Stat");
   addColumn("Val");
   addColumn("Max");
   addColumn("%");
   setAllColumnsShowFocus(true);

   QString statPrefName;
   for (int nloop = 0; nloop < LIST_MAXLIST; nloop++)
   {
     statPrefName.sprintf("show%s", statNames[nloop]);
     m_showStat[nloop] = pSEQPrefs->getPrefBool(statPrefName, section, false);
     updateStat(nloop);
   }

   // Restore column order
   QString tStr = pSEQPrefs->getPrefString("ColumnOrder", section, "N/A");
   if (tStr != "N/A") {
      int i = 0;
      while (!tStr.isEmpty()) {
         int toIndex;
         if (tStr.find(':') != -1) {
            toIndex = tStr.left(tStr.find(':')).toInt();
            tStr = tStr.right(tStr.length() - tStr.find(':') - 1);
         } else {
            toIndex = tStr.toInt();
            tStr = "";
         }
         header()->moveSection(toIndex, i++);
      }
   }
   // StatList column sizes
   if (pSEQPrefs->isPreference("StatWidth", section))
   {
     i = pSEQPrefs->getPrefInt("StatWidth", section, 
			       columnWidth(STATCOL_NAME));
     setColumnWidthMode(STATCOL_NAME, QListView::Manual);
     setColumnWidth(STATCOL_NAME, i);
   }
   if (pSEQPrefs->isPreference("ValueWidth", section))
   {
     i = pSEQPrefs->getPrefInt("ValueWidth", section, 
			       columnWidth(STATCOL_VALUE));
     setColumnWidthMode(STATCOL_VALUE, QListView::Manual);
     setColumnWidth(STATCOL_VALUE, i);
   }
   if (pSEQPrefs->isPreference("MaxWidth", section))
   {
     i = pSEQPrefs->getPrefInt("MaxWidth", section, 
			       columnWidth(STATCOL_MAXVALUE));
     setColumnWidthMode(STATCOL_MAXVALUE, QListView::Manual);
     setColumnWidth(STATCOL_MAXVALUE, i);
   }
   if (pSEQPrefs->isPreference("PercentWidth", section))
   {
     i = pSEQPrefs->getPrefInt("PercentWidth", section, 
			       columnWidth(STATCOL_PERCENT));
     setColumnWidthMode(STATCOL_PERCENT, QListView::Manual);
     setColumnWidth(STATCOL_PERCENT, i);
   }

   connect (m_pPlayer, SIGNAL(statChanged(int,int,int)), 
	    this, SLOT(statChanged (int,int,int)));
   connect (m_pPlayer, SIGNAL(expChangedInt(int,int,int)),
	    this, SLOT(expChanged(int,int,int)));
   connect (m_pPlayer, SIGNAL(expAltChangedInt(int,int,int)),
	    this, SLOT(expAltChanged(int,int,int)));
   connect (m_pPlayer, SIGNAL(stamChanged(int,int,int,int,int,int)),
	    this, SLOT(stamChanged(int,int,int,int,int,int)));
   connect (m_pPlayer, SIGNAL(manaChanged(uint32_t,uint32_t)),
	    this, SLOT(manaChanged(uint32_t,uint32_t)));
   connect (m_pPlayer, SIGNAL(hpChanged(uint16_t, uint16_t)), 
	    this, SLOT(hpChanged(uint16_t, uint16_t)));
}

EQStatList::~EQStatList()
{
}

void EQStatList::savePrefs(void)
{
  QString section = "StatList";
  // only save the preferences if visible
  if (isVisible())
  {
    if (pSEQPrefs->getPrefBool("SaveWidth", section, true))
    {
      pSEQPrefs->setPrefInt("StatWidth", section, 
			      columnWidth(STATCOL_NAME));
      pSEQPrefs->setPrefInt("ValueWidth", section, 
			      columnWidth(STATCOL_VALUE));
      pSEQPrefs->setPrefInt("MaxWidth", section, 
			      columnWidth(STATCOL_MAXVALUE));
      pSEQPrefs->setPrefInt("PercentWidth", section, 
			      columnWidth(STATCOL_PERCENT));
    }
    // Save column order
    char tempStr[256], tempStr2[256];
    if (header()->count() > 0)
      sprintf(tempStr, "%d", header()->mapToSection(0));
    for(int i=1; i<header()->count(); i++) {
      sprintf(tempStr2, ":%d", header()->mapToSection(i));
      strcat(tempStr, tempStr2);
    }
    pSEQPrefs->setPrefString("ColumnOrder", section, tempStr);
  }
}

void EQStatList::setCaption(const QString& text)
{
  // set the caption
  QListView::setCaption(text);

  // set the preference
  pSEQPrefs->setPrefString("Caption", "StatList", caption());
}

void EQStatList::expChanged  (int val, int min, int max) 
{
    if (!m_showStat[LIST_EXP])
      return;

    QString buf;

    m_statList[LIST_EXP]->setText (1, Commanate((uint32_t) (val - min)));
    m_statList[LIST_EXP]->setText (2, Commanate((uint32_t) (max - min)));

    buf = Commanate((uint32_t) ((val - min) / ((max - min)/100))) + " %";

    m_statList[LIST_EXP]->setText (3, buf);
}

void EQStatList::expAltChanged  (int val, int min, int max) 
{
    if (!m_showStat[LIST_ALTEXP])
      return;

    QString buf;

    m_statList[LIST_ALTEXP]->setText (1, Commanate((uint32_t) (val - min)));
    m_statList[LIST_ALTEXP]->setText (2, Commanate((uint32_t) (max - min)));

    buf = Commanate((uint32_t) ((val - min) / ((max - min)/100))) + " %";

    m_statList[LIST_ALTEXP]->setText (3, buf);
}

void EQStatList::statChanged (int stat, int val, int max)
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

void EQStatList::hpChanged(uint16_t val, uint16_t max)
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

void EQStatList::manaChanged (uint32_t val, uint32_t max)
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

void EQStatList::stamChanged (int Sval, int Smax, 
			       int Fval, int Fmax,
			       int Wval, int Wmax) 
{
  char buf[64];

  if (m_showStat[LIST_STAM])
   {
  	sprintf(buf,"%d",Sval);
  	m_statList[LIST_STAM]->setText (1, buf);
  	sprintf(buf,"%d",Smax);
  	m_statList[LIST_STAM]->setText (2, buf);
  	sprintf(buf,"%d %%",Sval*100/Smax);
  	m_statList[LIST_STAM]->setText (3, buf);
   }

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

void EQStatList::resetMaxMana(void)
{
  if (!m_showStat[LIST_MANA])
    return;

   char buf[20];
   m_guessMaxMana = 0;
   sprintf(buf,"%d",m_guessMaxMana);
   m_statList[LIST_MANA]->setText (2, buf);
}

void EQStatList::enableStat(uint8_t stat, bool enabled)
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

void EQStatList::updateStat(uint8_t stat)
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
      bool valid = m_pPlayer->getStatValue(stat, value, max);

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
	    uint32_t minExp = calc_exp(m_pPlayer->getPlayerLevel() - 1,
				       m_pPlayer->getPlayerRace(),
				       m_pPlayer->getPlayerClass());

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
      m_statList[stat] = new QListViewItem(this,
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
