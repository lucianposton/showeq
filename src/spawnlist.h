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

#include <qvaluelist.h>
#include <qlistview.h>
#include <qptrdict.h>
#include <qtextstream.h>

// these are all used for the CFilterDlg
#include <regex.h>

#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>

#include "everquest.h"
#include "player.h"
#include "spawnshell.h"
#include "category.h"

//--------------------------------------------------
// defines
#define SPAWNCOL_NAME   0
#define SPAWNCOL_LEVEL  1
#define SPAWNCOL_HP     2
#define SPAWNCOL_MAXHP  3
#define SPAWNCOL_XPOS   4
#define SPAWNCOL_YPOS   5
#define SPAWNCOL_ZPOS   6
#define SPAWNCOL_ID     7
#define SPAWNCOL_DIST   8
#define SPAWNCOL_RACE   9
#define SPAWNCOL_CLASS 10
#define SPAWNCOL_INFO  11
#define SPAWNCOL_SPAWNTIME 12
#define SPAWNCOL_MAXCOLS 13

//--------------------------------------------------
// forward declarations
class CSpawnList;
class SpawnListItem;

//--------------------------------------------------
// SpawnListMenu
class SpawnListMenu : public QPopupMenu
{
   Q_OBJECT

 public:
  SpawnListMenu(CSpawnList* spawnlist, 
		FilterMgr* filterMgr, 
		CategoryMgr* categoryMgr,
		QWidget* parent = 0, const char* name = 0);
  virtual ~SpawnListMenu();
  void setCurrentCategory(const Category* cat);
  void setCurrentItem(const Item* item);

 protected slots:
   void init_Menu(void);
   void toggle_spawnListCol( int id );
   void add_filter(int id);
   void add_category(int id);
   void edit_category(int id);
   void delete_category(int id);
   void reload_categories(int id);
   void rebuild_spawnlist(int id);

 protected:
  CSpawnList* m_spawnlist;
  FilterMgr* m_filterMgr;
  CategoryMgr* m_categoryMgr;
  const Category* m_currentCategory;
  const Item* m_currentItem;
  int m_id_filterMenu;
  int m_id_spawnList_Cols[SPAWNCOL_MAXCOLS];
  int m_id_edit_category;
  int m_id_delete_category;
};


//--------------------------------------------------
// SpawnListItem
class SpawnListItem : public QListViewItem
{
public:
   SpawnListItem(QListViewItem *parent);
   SpawnListItem(QListView *parent);
   virtual ~SpawnListItem();

   virtual void paintCell( QPainter *p, const QColorGroup &cg,
                           int column, int width, int alignment );

   const QColor textColor()  { return m_textColor; }
   void setTextColor(const QColor &color)  
                             { m_textColor = QColor(color); }
   const Item*    item() { return m_item; }
   uint32_t filterFlags() { return m_filterFlags; }
   uint32_t runtimeFilterFlags() { return m_runtimeFilterFlags; }
   void setFilterFlags(uint32_t flags) { m_filterFlags = flags; }
   void setRuntimeFilterFlags(uint32_t flags) { m_runtimeFilterFlags = flags; }

   void update(uint32_t changeType = tSpawnChangedALL);
   void updateTitle(const QString& name);
   void setShellItem(const Item *);
   itemType type();
   //--------------------------------------------------
   int m_npc;
private:
   QColor m_textColor;
   uint32_t m_filterFlags;
   uint32_t m_runtimeFilterFlags;    

   const Item *m_item;
};

//--------------------------------------------------
// CSpawnList
class CSpawnList : public QListView
{
   Q_OBJECT
public:
   CSpawnList(EQPlayer* player, 
	      SpawnShell* spawnShell, 
	      CategoryMgr* categoryMgr,
	      QWidget *parent = 0, const char * name = 0);

   const QString& preferenceName() { return m_preferenceName; }

   SpawnListItem* Selected();
   void DeleteItem(const Item* item);
   SpawnListItem* Find(QListViewItemIterator& it, 
		       const Item* item, 
		       bool first = false);

   double calcDist(const Item* item)
   { 
     return item->calcDist(m_player->getPlayerX(), 
			   m_player->getPlayerY(),
			   m_player->getPlayerZ());
   }

   int calcDistInt(const Item* item)
   { 
     return item->calcDist2DInt(m_player->getPlayerX(), 
				m_player->getPlayerY());
   }

   QColor pickSpawnColor(const Item *item, QColor def = Qt::black);
   const Category* getCategory(SpawnListItem *);
   bool debug() { return bDebug; };

   SpawnListMenu* menu();

signals:
   void listUpdated();   // flags in spawns have changed
   void listChanged();   // categories have changed
   void spawnSelected(const Item* item);
   void keepUpdated(bool on);

public slots: 
   void savePrefs();
   void setPlayer(int16_t xPos, int16_t yPos, int16_t zPos, 
		  int16_t deltaX, int16_t deltaY, int16_t deltaZ, 
		  int32_t degrees); 
   void setDebug(bool bset)       { bDebug = bset; }
   void selectNext(void);
   void selectPrev(void);
   void setColumnVisible(int id, bool visible);
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
   virtual void setCaption(const QString&);
   void setWindowFont(const QFont&);
   void restoreFont();
   
private slots:
   void selChanged(QListViewItem*);

//#if 0
//   void rightBtnPressed(QListViewItem* litem, 
//			const QPoint &point, 
//			int col);
//   void rightBtnReleased(QListViewItem *item,
//			 const QPoint &point, 
//			 int col);
//#endif
   void myMousePressEvent (int button, QListViewItem *litem, const QPoint &point, int col);
   void myMouseDoubleClickEvent(QListViewItem *litem);

private:
   void setSelectedQueit(QListViewItem* item, bool selected);
   void populateSpawns(void);
   void populateCategory(const Category* cat);
   QString filterString(const Item *item, int flags = 0);

   void selectAndOpen(SpawnListItem *);
   CategoryMgr* m_categoryMgr;
   EQPlayer *m_player;
   SpawnShell* m_spawnShell;

   // category pointer used as keys to look up the associated SpawnListItem
   QPtrDict<SpawnListItem> m_categoryListItems;
   bool     bDebug;

   QString m_preferenceName;
   SpawnListMenu* m_menu;
};

#endif // SPAWNLIST_H

