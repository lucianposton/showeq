/*
 * spawnlist.h
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

/*
 * Orig Author - Maerlyn (MaerlynTheWiz@yahoo.com)
 * Date   - 3/16/00
 */

/* 
 * SpawnListItem 
 *
 * SpawnListItem is a class intended to store information about an EverQuest
 * Spawn.  It inherits from QListViewItem but overrides functionality to allow
 * paint styles such as color changes
 *
 * currently it just provides a widget and maintains a QColor for the text
 * display of that widget
 */
 
#ifndef SPAWNLIST_H
#define SPAWNLIST_H

#include <time.h>
#include <stdio.h>
#include <sys/time.h>

#include <q3valuelist.h>
#include <q3listview.h>
#include <q3ptrdict.h>
#include <q3textstream.h>
//Added by qt3to4:
#include <Q3PopupMenu>

// these are all used for the CFilterDlg
#include <regex.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qpushbutton.h>

#include "seqwindow.h"
#include "seqlistview.h"
#include "spawnlistcommon.h"
#include "spawn.h"

//--------------------------------------------------
// forward declarations
class Category;
class CategoryMgr;
class Item;
class Player;
class SpawnShell;
class FilterMgr;

class SpawnList;
class SpawnListItem;
class SpawnListMenu;

//--------------------------------------------------
// SpawnList
class SpawnList : public SEQListView
{
   Q_OBJECT
public:
   SpawnList(Player* player, 
	     SpawnShell* spawnShell, 
	     CategoryMgr* categoryMgr,
	     QWidget *parent = 0, const char * name = 0);

   SpawnListItem* Selected();
   SpawnListItem* Find(Q3ListViewItemIterator& it, 
		       const Item* item, 
		       bool first = false);

   const Category* getCategory(SpawnListItem *);

   SpawnListMenu* menu();

signals:
   void listUpdated();   // flags in spawns have changed
   void listChanged();   // categories have changed
   void spawnSelected(const Item* item);
   void keepUpdated(bool on);

public slots: 
   void setPlayer(int16_t x, int16_t y, int16_t z, 
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ, 
		  int32_t degrees); 
   void selectNext(void);
   void selectPrev(void);
   // SpawnShell signals
   void addItem(const Item *);
   void delItem(const Item *);
   void changeItem(const Item *, uint32_t changeType);
   void killSpawn(const Item *);
   void selectSpawn(const Item *);
   void clear();
   void addCategory(const Category* cat);
   void delCategory(const Category* cat);
   void clearedCategories(void);
   void loadedCategories(void);
   
   void rebuildSpawnList();
   void playerLevelChanged(uint8_t);
   
private slots:
   void selChanged(Q3ListViewItem*);

   void mousePressEvent (int button, Q3ListViewItem *litem, const QPoint &point, int col);
   void mouseDoubleClickEvent(Q3ListViewItem *litem);

private:
   void setSelectedQuiet(Q3ListViewItem* item, bool selected);
   void populateSpawns(void);
   void populateCategory(const Category* cat);
   QString filterString(const Item *item, int flags = 0);

   void selectAndOpen(SpawnListItem *);
   CategoryMgr* m_categoryMgr;
   Player *m_player;
   SpawnShell* m_spawnShell;

   // category pointer used as keys to look up the associated SpawnListItem
   Q3PtrDict<SpawnListItem> m_categoryListItems;

   SpawnListMenu* m_menu;

};

class SpawnListWindow : public SEQWindow
{
  Q_OBJECT

 public:
  SpawnListWindow(Player* player, 
		  SpawnShell* spawnShell, 
		  CategoryMgr* categoryMgr,
		  QWidget* parent = 0, const char* name = 0);
  ~SpawnListWindow();
  virtual Q3PopupMenu* menu();
  SpawnList* spawnList() { return m_spawnList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  SpawnList* m_spawnList;
};

#endif // SPAWNLIST_H

