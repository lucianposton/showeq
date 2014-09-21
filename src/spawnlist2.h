/*
 * spawnlist2.h
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#ifndef SPAWNLIST2_H
#define SPAWNLIST2_H

#include <q3ptrdict.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include "seqwindow.h"
#include "seqlistview.h"
#include "spawnlistcommon.h"

//--------------------------------------------------
// forward declarations
class Category;
class CategoryMgr;
class Player;
class SpawnShell;
class FilterMgr;

class QComboBox;
class QTimer;
class QSpinBox;

class SpawnListWindow2 : public SEQWindow
{
  Q_OBJECT

 public:
  SpawnListWindow2(Player* player, 
		   SpawnShell* spawnShell, 
		   CategoryMgr* categoryMgr,
		   QWidget* parent = 0, const char* name = 0);
  ~SpawnListWindow2();

   virtual Q3PopupMenu* menu();


   SpawnListItem* selected();
   SpawnListItem* find(const Item* item);

   QString filterString(const Item* item);

signals:
   void listUpdated();   // flags in spawns have changed
   void listChanged();   // categories have changed
   void spawnSelected(const Item* item);
   void keepUpdated(bool on);

public slots: 
   // SpawnShell signals
   void addItem(const Item *);
   void delItem(const Item *);
   void changeItem(const Item *, uint32_t changeType);
   void killSpawn(const Item *);
   void selectSpawn(const Item *);
   void clear(void);

   // CategoryMgr signals
   void addCategory(const Category* cat);
   void delCategory(const Category* cat);
   void clearedCategories(void);
   void loadedCategories(void);

   // Player signals
   void playerLevelChanged(uint8_t);
   void setPlayer(int16_t x, int16_t y, int16_t z, 
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ, 
		  int32_t degrees); 

   void rebuildSpawnList(void);
   void refresh(void);
   virtual void savePrefs(void);
 private slots:
   // category combo box signals
   void categorySelected(int index);
   
    // listview signals
   void selChanged(Q3ListViewItem*);

   void mousePressEvent (int button, Q3ListViewItem *litem, const QPoint &point, int col);
   void mouseDoubleClickEvent(Q3ListViewItem *litem);

   // fpm spinbox signals
   void setFPM(int rate);

   // additional menu items
   void toggle_immediateUpdate(int id);
   void toggle_keepSorted(int id);
   void toggle_keepSelectedVisible(int id);
 private:
   void setSelectedQuiet(Q3ListViewItem* item, bool selected);
   void populateSpawns(void);
   void populateCategory(const Category* cat);
   void updateCount(void);
   // data sources
   Player *m_player;
   CategoryMgr* m_categoryMgr;
   SpawnShell* m_spawnShell;

   // category currently being viewed
   Category* m_currentCategory;

   // currently selected item
   const Item* m_selectedItem;

   // GUI Items
   QComboBox* m_categoryCombo;
   QSpinBox* m_fpmSpinBox;
   SEQListView* m_spawnList;
   SpawnListMenu* m_menu;
   QLineEdit* m_totalSpawns;

   // index dictionary for retrieving SpawnListItems by Item
   Q3PtrDict<SpawnListItem> m_spawnListItemDict;
   
   // timer used
   QTimer* m_timer;

   // frames per minute (how many times spawnlist is updated per minute)
   int m_delay;
   time_t m_lastUpdate;

   // whether to immediately update the spawn list as changes occur, or
   // use timer.
   bool m_immediateUpdate;
   bool m_keepSorted;
   bool m_keepSelectedVisible;
};

#endif // SPAWNLIST2_H
