/*
 * spawnpointlist.cpp
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

#include <time.h>

#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qfontdialog.h>
#include <qpainter.h>
#include <qlayout.h>

#include "spawnpointlist.h"
#include "main.h"

SpawnPointListItem::SpawnPointListItem(QListView* parent, const SpawnPoint* sp)
  : QListViewItem( parent )
{
#ifdef DEBUG
//      debug( "SpawnItem::SpawnItem()" );
#endif
  m_textColor = Qt::black;
  m_spawnPoint = sp;

  update();
}

SpawnPointListItem::~SpawnPointListItem()
{
}

void SpawnPointListItem::update()
{
  QString tmpStr;
  // set the coordinate information
  if (showeq_params->retarded_coords)
  {
    setText(tSpawnPointCoord1, QString::number(m_spawnPoint->y()));
    setText(tSpawnPointCoord2, QString::number(m_spawnPoint->x()));
  }
  else
  {
    setText(tSpawnPointCoord1, QString::number(m_spawnPoint->x()));
    setText(tSpawnPointCoord2, QString::number(m_spawnPoint->y()));
  }
  setText(tSpawnPointCoord3, 
	  QString::number(m_spawnPoint->displayZPos(), 'f', 1));

  // construct and set the time remaining string
  if ( m_spawnPoint->m_diffTime == 0 || m_spawnPoint->m_deathTime == 0 )
    tmpStr = "\277 ?";  // upside down questoin mark followed by question mark
  else
  {
    long secs = m_spawnPoint->secsLeft();
    
    if ( secs > 0 )
      tmpStr.sprintf( "%2ld:%02ld", secs / 60, secs % 60  );
    else
      tmpStr = "   now"; // spaces followed by now
  }
  setText(tSpawnPointRemaining, tmpStr);

  // set the name and last spawn info
  setText(tSpawnPointName, m_spawnPoint->name());
  setText(tSpawnPointLast, m_spawnPoint->last());

  // construct and set the spawned string
  QDateTime       dateTime;
  dateTime.setTime_t( m_spawnPoint->m_spawnTime );
  QDate           createDate = dateTime.date();
  tmpStr = "";
  // spawn time
  if ( createDate != QDate::currentDate() )
    tmpStr = createDate.dayName( createDate.dayOfWeek() ) + " ";
  
  tmpStr += dateTime.time().toString();

  // set when it spawned and the count
  setText(tSpawnPointSpawned, tmpStr);
  setText(tSpawnPointCount, QString::number(m_spawnPoint->count()));
}

void SpawnPointListItem::setTextColor( const QColor& color )
{
  m_textColor = color;
  repaint();
}

// overridden from base class in order to change color and style attributes
void SpawnPointListItem::paintCell(QPainter* p, const QColorGroup& cg, 
				   int column, int width, int alignment )
{
#ifdef DEBUG
//      debug( "SpawnItem::paintCell()" );
#endif

  QColorGroup newCg( cg );
  QFont font = this->listView()->font();
  
  if (m_spawnPoint->age() > 220)
  {
    newCg.setColor(QColorGroup::Text, red);
    font.setBold(true);
  }
  else
    font.setBold(false);
  
  p->setFont(font);
  
  QListViewItem::paintCell(p, newCg, column, width, alignment);
}

SpawnPointList::SpawnPointList(SpawnMonitor* spawnMonitor, 
			       QWidget* parent, const char* name):
  SEQListView("SpawnPointList", parent, name),
  m_spawnMonitor(spawnMonitor),
  m_menu(NULL),
  m_aboutToPop(false)
{
  // get whether to keep the list sorted or not
  m_keepSorted = pSEQPrefs->getPrefBool("KeepSorted", preferenceName(), false);

  // add all the columns
  if (showeq_params->retarded_coords)
  {
    addColumn ("N/S", "Coord1");
    addColumn ("E/W", "Coord2");
  }
  else
  {
    addColumn ("X", "Coord1");
    addColumn ("Y", "Coord2");
  }
  addColumn("Z", "Coord3");
  addColumn("Remaining");
  addColumn("Name");
  addColumn("Last");
  addColumn("Spawned");
  addColumn("Count");

  // default sort is on remaining
  setSorting(tSpawnPointRemaining, true);

  // restore the columns settings from preferences
  restoreColumns();

  // put in all the spawn points that might already be present in
  // the spawn monitor
  QAsciiDictIterator<SpawnPoint> it( m_spawnMonitor->spawnPoints() );
  SpawnPoint*             sp;
  while ((sp = it.current()))
  {
    new SpawnPointListItem(this, sp);
    ++it;
  }

  // create the timer
  m_timer = new QTimer( this, "spawn-point-timer" );
  connect( m_timer, SIGNAL( timeout() ), this, SLOT( refresh() ) );


  connect(m_spawnMonitor, SIGNAL(newSpawnPoint(const SpawnPoint*)),
	  this, SLOT(newSpawnPoint(const SpawnPoint*)));
  connect(m_spawnMonitor, SIGNAL(clearSpawnPoints()),
	  this, SLOT(clear()));
  connect(m_spawnMonitor, SIGNAL(selectionChanged(const SpawnPoint*)),
	  this, SLOT(handleSelChanged(const SpawnPoint*)));
  connect(this, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
	  this, SLOT(rightButtonClicked(QListViewItem*, const QPoint&, int)));
  connect(this, SIGNAL( selectionChanged(QListViewItem*)),
	  this, SLOT(handleSelectItem(QListViewItem*)));
  m_timer->start(10000L);
}

void SpawnPointList::setKeepSorted(bool val)
{
  m_keepSorted = val;
  pSEQPrefs->setPrefBool("KeepSorted", preferenceName(), 
			 m_keepSorted);
}

void SpawnPointList::handleSelectItem(QListViewItem* item)
{
  const SpawnPoint* sp = NULL;
  
  if ( item )
  {
    // get the spawn point associated with the list view item
    sp = ((SpawnPointListItem*)item)->spawnPoint();

    // set the selected spawn point
    m_spawnMonitor->setSelected(sp);
  }
}

void SpawnPointList::handleSelChanged(const SpawnPoint* sp)
{
  QListViewItemIterator it(this);

  SpawnPointListItem* splitem;
  
  // keep iterating until we find a match
  while (it.current())
  {
    splitem = (SpawnPointListItem*)it.current();

    // is this the current item
    if (splitem->spawnPoint() == sp)
    {
      // yes, set it as the selected item in the spawn list
      setSelected(splitem, true);
      
      // ensure that the item is visible
      ensureItemVisible(splitem);

      break;
    }
    ++it;
  }
}

void SpawnPointList::rightButtonClicked(QListViewItem* item, 
					const QPoint& point, int)
{
  // popup a context-menu
  SpawnPointListMenu* spawnPointMenu = menu();
  spawnPointMenu->setCurrentItem((const SpawnPointListItem*)item);
  spawnPointMenu->popup(point);
}

void SpawnPointList::renameItem(const SpawnPointListItem* item)
{
  if (item == NULL)
    return;

  // get the underlying spawn point
  const SpawnPoint* sp = ((SpawnPointListItem*)item)->spawnPoint();


  if ( sp )
  {
    // default to the existing name
    QString def = sp->name();

    // if there is no existing name, use the name of the last spawn
    if (def.isEmpty())
      def = sp->last();
    
    // ask the user for the new name
    bool ok = false;
    QString text = QInputDialog::getText(
					 tr("Spawn Point"),
					 tr("New name:"),
					 QLineEdit::Normal,
					 def,
					 &ok,
					 this );

    // if the user clicked ok and they entered a name, set the new name
    if (ok && !text.isEmpty())
    {
      m_spawnMonitor->setName(sp, text);
      ((SpawnPointListItem*)item)->update();
    }
  }
}

void SpawnPointList::deleteItem(const SpawnPointListItem* item)
{
  if (item == NULL)
    return;

  // get the underlying spawn point
  const SpawnPoint* sp = ((SpawnPointListItem*)item)->spawnPoint();


  if (sp == NULL)
    return;

  // default to the existing name
  QString def = sp->name();

  // if there is no existing name, use the name of the last spawn
  if (def.isEmpty())
    def = sp->last();
  
  def.sprintf("%d/%d/%5.1f '%s'", 
	      sp->x(), sp->y(), sp->displayZPos(), (const char*)def);
  
  // confirm that the user wants to delete the category
  QMessageBox mb("Are you sure?",
		 "Are you sure you wish to delete spawn point "
		 + def + "?",
		 QMessageBox::NoIcon,
		 QMessageBox::Yes, 
		 QMessageBox::No | QMessageBox::Default | QMessageBox::Escape,
		 QMessageBox::NoButton,
		 this);
  
  // if user chose eys, then delete the spawn point
  if (mb.exec() == QMessageBox::Yes)
  {
    // remove the item from the spawn point list
    delete item;
    
    // remove the item from the spawn monitor
    m_spawnMonitor->deleteSpawnPoint(sp);
  }
}

void SpawnPointList::refresh()
{
  bool aboutToPop = false;

  // iterate over all the spawn point list items and update them
  QListViewItemIterator lit(this);
  while(lit.current())
  {
    // update the current item
    ((SpawnPointListItem*)lit.current())->update();

    // iterate to the next item
    ++lit;
  }

  // make sure the list view is still sorted.
  if (m_keepSorted)
    sort();

  // iterate over all the spawn points and check how long till they pop
  QAsciiDictIterator<SpawnPoint> it(m_spawnMonitor->spawnPoints());
  SpawnPoint* sp;
  
  while ((sp = it.current()))
  {
    ++it;
    if (sp->secsLeft() < 20)
      aboutToPop = true;
  }

  // set timer to update before something is about to pop
  if (aboutToPop)
    m_timer->start(1000L);
  else
    m_timer->start(10000L);
}

void SpawnPointList::newSpawnPoint( const SpawnPoint* sp )
{
  new SpawnPointListItem(this, sp);
}

void SpawnPointList::clear()
{
  SEQListView::clear();
}

SpawnPointListMenu* SpawnPointList::menu()
{
  // if a menu already exists return it
  if (m_menu != NULL)
    return m_menu;
  
  // create a new menu
  m_menu = new SpawnPointListMenu(this, this, "spawnlist menu");

  return m_menu;
}

SpawnPointListMenu::SpawnPointListMenu(SpawnPointList* spawnPointList,
				       QWidget* parent, const char* name)
  : QPopupMenu(parent, name),
    m_spawnPointList(spawnPointList),
    m_currentItem(NULL)
{
  m_id_rename = insertItem("&Rename Spawn Point...",
			   this, SLOT(rename_item(int)));
  m_id_delete = insertItem("&Delete Spawn Point...",
			   this, SLOT(delete_item(int)));
  
  QPopupMenu* listColMenu = new QPopupMenu;
  insertItem("Show &Column", listColMenu);
  listColMenu->setCheckable(true);

  int x = tSpawnPointCoord1;
  m_id_cols[x] = listColMenu->insertItem("Coord &1");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("Coord &2");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("Coord &3");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("&Remaining");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("&Name");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("&Last");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("&Spawned");
  listColMenu->setItemParameter(m_id_cols[x], x);
  x++;
  m_id_cols[x] = listColMenu->insertItem("&Count");
  listColMenu->setItemParameter(m_id_cols[x], x);
  
  connect (listColMenu, SIGNAL(activated(int)), 
	   this, SLOT(toggle_col(int)));
  
  insertSeparator(-1);
  insertItem("&Font...", this, SLOT(set_font(int)));
  insertItem("&Caption...", this, SLOT(set_caption(int)));

  insertSeparator(-1);
  x = insertItem("Keep Sorted", this, SLOT(toggle_keepSorted(int)));
  setItemChecked(x, m_spawnPointList->keepSorted());

  connect(this, SIGNAL(aboutToShow()),
	  this, SLOT(init_menu()));
}

SpawnPointListMenu::~SpawnPointListMenu()
{
}

void SpawnPointListMenu::setCurrentItem(const SpawnPointListItem* item)
{
  m_currentItem = item;
}

void SpawnPointListMenu::init_menu()
{
  setItemEnabled(m_id_rename, (m_currentItem != NULL));
  setItemEnabled(m_id_delete, (m_currentItem != NULL));

  // make sure the menu bar settings are correct
  for (int i = 0; i < tSpawnPointMaxCols; i++)
    setItemChecked(m_id_cols[i], 
		   m_spawnPointList->columnVisible(i));
}

void SpawnPointListMenu::rename_item(int id)
{
  m_spawnPointList->renameItem(m_currentItem);
}

void SpawnPointListMenu::delete_item(int id)
{
  m_spawnPointList->deleteItem(m_currentItem);
}

void SpawnPointListMenu::toggle_col(int id)
{
  int colnum;

  colnum = itemParameter(id);
  
  if (isItemChecked(id))
    m_spawnPointList->setColumnVisible(colnum, false);
  else
    m_spawnPointList->setColumnVisible(colnum, true);
}

void SpawnPointListMenu::set_font(int id)
{
  QFont newFont;
  bool ok = false;
  SEQWindow* window = (SEQWindow*)m_spawnPointList->parent();

  // get a new font
  newFont = QFontDialog::getFont(&ok, window->font(), 
				 this, "ShowEQ Spawn Point List Font");
    
    
    // if the user entered a font and clicked ok, set the windows font
    if (ok)
      window->setWindowFont(newFont);
}

void SpawnPointListMenu::set_caption(int id)
{
  bool ok = false;
  SEQWindow* window = (SEQWindow*)m_spawnPointList->parent();

  QString caption = 
    QInputDialog::getText("ShowEQ Spawn Point Window Caption",
			  "Enter caption for the Spawn Point Window:",
			  QLineEdit::Normal, window->caption(),
			  &ok, this);
  
  // if the user entered a caption and clicked ok, set the windows caption
  if (ok)
    window->setCaption(caption);
}


void SpawnPointListMenu::toggle_keepSorted(int id)
{
  // toggle immediate update value
  m_spawnPointList->setKeepSorted(!m_spawnPointList->keepSorted());
  setItemChecked(id, m_spawnPointList->keepSorted());
}

SpawnPointWindow::SpawnPointWindow(SpawnMonitor* spawnMonitor, 
				   QWidget* parent, const char* name):
        SEQWindow("SpawnPointList", "ShowEQ - Spawn Points", parent, name)
{
  QVBoxLayout*    layout = new QVBoxLayout( this );
  layout->setAutoAdd( true );
  
  m_spawnPointList = new SpawnPointList(spawnMonitor, this, name );
};

SpawnPointWindow::~SpawnPointWindow()
{
}

void SpawnPointWindow::savePrefs(void)
{
  // make base class save it's preferences
  SEQWindow::savePrefs();

  // make the spawn point listview save it's preferences
  if (m_spawnPointList)
    m_spawnPointList->savePrefs();
}
