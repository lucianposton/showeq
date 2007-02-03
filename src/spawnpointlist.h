/*
 * spawnpointlist.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Borrowed from:  SINS Distributed under GPL
 * Portions Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). 
 *
 * For use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef SPAWNPOINTLIST_H
#define SPAWNPOINTLIST_H

#include <qtimer.h>
#include <q3popupmenu.h>

#include "seqlistview.h"
#include "seqwindow.h"
#include "spawnmonitor.h"

// constants
const int tSpawnPointCoord1 = 0;
const int tSpawnPointCoord2 = 1;
const int tSpawnPointCoord3 = 2;
const int tSpawnPointRemaining = 3;
const int tSpawnPointName = 4;
const int tSpawnPointLast = 5;
const int tSpawnPointSpawned = 6;
const int tSpawnPointCount = 7;
const int tSpawnPointMaxCols = 8;

// forward declarations
class SpawnPointList;
class SpawnPointListItem;
class SpawnPointListMenu;
class SpawnPointWindow;

class SpawnPointListItem: public Q3ListViewItem
{
public:
  SpawnPointListItem(Q3ListView* parent, const SpawnPoint* spawn);
  virtual ~SpawnPointListItem();

  void update(void);
  virtual void paintCell(QPainter *p, const QColorGroup &cg, 
			 int column, int width, int alignment );
  
  const QColor textColor() const { return m_textColor; }
  void setTextColor(const QColor &color);
  const SpawnPoint* spawnPoint() { return m_spawnPoint; }

 protected:
  QColor m_textColor;
  const SpawnPoint* m_spawnPoint;
};


//--------------------------------------------------
// SpawnListMenu
class SpawnPointListMenu : public Q3PopupMenu
{
   Q_OBJECT

 public:
  SpawnPointListMenu(SpawnPointList* spawnPointList, 
		     QWidget* parent = 0, const char* name = 0);
  virtual ~SpawnPointListMenu();
  void setCurrentItem(const SpawnPointListItem* item);

 protected slots:
   void init_menu(void);
   void rename_item(int id);
   void delete_item(int id);
   void toggle_col( int id );
   void set_font(int id);
   void set_caption(int id);
   void toggle_keepSorted(int id);

 protected:
  SpawnPointList* m_spawnPointList;
  const SpawnPointListItem* m_currentItem;
  int m_id_rename;
  int m_id_delete;
  int m_id_cols[tSpawnPointMaxCols];
};

class SpawnPointList : public SEQListView
{
  Q_OBJECT

 public:
  SpawnPointList(SpawnMonitor* spawnMonitor, 
		 QWidget* parent = 0, const char* name = 0);
  SpawnPointListMenu* menu();

  bool keepSorted() { return m_keepSorted; }
  void setKeepSorted(bool val);

 public slots:
  void rightButtonClicked(Q3ListViewItem*, const QPoint&, int);
  void renameItem(const SpawnPointListItem* item);
  void deleteItem(const SpawnPointListItem* item);
  void clearItems(void);
  void refresh();
  void handleSelectItem(Q3ListViewItem* item);
  void newSpawnPoint(const SpawnPoint* sp);
  void clear();
  void handleSelChanged(const SpawnPoint* sp);

 protected:
  SpawnMonitor* m_spawnMonitor;
  SpawnPointListMenu* m_menu;
  QTimer* m_timer;
  bool m_aboutToPop;
  bool m_keepSorted;
};

class SpawnPointWindow : public SEQWindow
{
  Q_OBJECT

 public:
  SpawnPointWindow(SpawnMonitor* spawnMonitor, 
		   QWidget* parent = 0, const char* name = 0);
  ~SpawnPointWindow();
  virtual Q3PopupMenu* menu();

  SpawnPointList* spawnPointList() { return m_spawnPointList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  SpawnPointList* m_spawnPointList;
};

#endif // SPAWNPOINTLIST_H
