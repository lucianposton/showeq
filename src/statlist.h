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

#include "player.h"

class EQStatList : public QListView
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
   void savePrefs(void);

   void expChanged(int val, int min, int max);
   void hpChanged(uint16_t val, uint16_t max);
   void manaChanged(uint32_t val, uint32_t max);
   void stamChanged(int Sval, int Smax, int Fval, int Fmax, int Wval, int Wmax);
   void statChanged (int statNum, int val, int max);
   void resetMaxMana(void);
   void updateStat(uint8_t stat, bool enable);
 private:
   // the player this skill list is monitoring
   EQPlayer* m_pPlayer;

   QListViewItem* m_statList[LIST_MAXLIST]; 

   uint32_t  m_guessMaxMana;
   bool m_showStat[LIST_MAXLIST];
};

#endif // EQSTATLIST_H
