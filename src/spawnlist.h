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
                             { m_textColor = QColor(color); m_btextSet = TRUE; }
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
   bool m_btextSet;
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

public:
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
   
private slots:
   void selChanged(QListViewItem*);
   void rightBtnPressed(QListViewItem* litem, 
			const QPoint &point, 
			int col);
   void rightBtnReleased(QListViewItem *item,
			 const QPoint &point, 
			 int col);

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
};

#endif // SPAWNLIST_H

