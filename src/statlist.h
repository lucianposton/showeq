/*
 * statlist.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQSTATLIST_H
#define EQSTATLIST_H

#include <qwidget.h>
#include <q3listview.h>

#include "seqwindow.h"
#include "seqlistview.h"
#include "player.h"

#define STATCOL_NAME 0
#define STATCOL_VALUE 1
#define STATCOL_MAXVALUE 2
#define STATCOL_PERCENT 3

class StatList : public SEQListView
{
   Q_OBJECT

 public:
   // constructor
   StatList (Player* player,
	       QWidget*  parent = 0, 
	       const char* name = 0); 
   
   // destructor
   ~StatList();

   bool statShown(int stat) { return m_showStat[stat]; }
   
 public slots:

   void expChanged(int val, int min, int max);
   void expAltChanged(int val, int min, int max);
   void hpChanged(int16_t val, int16_t max);
   void manaChanged(uint32_t val, uint32_t max);
   void stamChanged(int Fval, int Fmax, int Wval, int Wmax);
   void statChanged (int statNum, int val, int max);
   void resetMaxMana(void);
   void enableStat(uint8_t stat, bool enable);
   void updateStat(uint8_t stat);

 private:
   // the player this skill list is monitoring
   Player* m_player;

   Q3ListViewItem* m_statList[LIST_MAXLIST]; 

   uint32_t  m_guessMaxMana;
   bool m_showStat[LIST_MAXLIST];
};

class StatListWindow : public SEQWindow
{
  Q_OBJECT

 public:
  StatListWindow(Player* player, QWidget* parent = 0, const char* name = 0);
  ~StatListWindow();
  StatList* statList() { return m_statList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  StatList* m_statList;
};

#endif // EQSTATLIST_H
