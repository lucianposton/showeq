/*
 * statlist.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#ifndef EQSTATLIST_H
#define EQSTATLIST_H

#include <qwidget.h>
#include <qlistview.h>

#include "seqwindow.h"
#include "seqlistview.h"
#include "player.h"

#define STATCOL_NAME 0
#define STATCOL_VALUE 1
#define STATCOL_MAXVALUE 2
#define STATCOL_PERCENT 3

class EQStatList : public SEQListView
{
   Q_OBJECT

 public:
   // constructor
   EQStatList (EQPlayer* player,
	       QWidget*  parent = 0, 
	       const char* name = 0); 
   
   // destructor
   ~EQStatList();

   bool statShown(int stat) { return m_showStat[stat]; }
   
 public slots:

   void expChanged(int val, int min, int max);
   void expAltChanged(int val, int min, int max);
   void hpChanged(uint16_t val, uint16_t max);
   void manaChanged(uint32_t val, uint32_t max);
   void stamChanged(int Sval, int Smax, int Fval, int Fmax, int Wval, int Wmax);
   void statChanged (int statNum, int val, int max);
   void resetMaxMana(void);
   void enableStat(uint8_t stat, bool enable);
   void updateStat(uint8_t stat);

 private:
   // the player this skill list is monitoring
   EQPlayer* m_pPlayer;

   QListViewItem* m_statList[LIST_MAXLIST]; 

   uint32_t  m_guessMaxMana;
   bool m_showStat[LIST_MAXLIST];
};

class StatListWindow : public SEQWindow
{
  Q_OBJECT

 public:
  StatListWindow(EQPlayer* player, QWidget* parent = 0, const char* name = 0);
  ~StatListWindow();
  EQStatList* statList() { return m_statList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  EQStatList* m_statList;
};

#endif // EQSTATLIST_H
