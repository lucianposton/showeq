
#include "spawnlist2.h"
#include "category.h"
#include "spawnshell.h"
#include "filtermgr.h"
#include "player.h"
#include "diagnosticmessages.h"
#include "main.h"

#include <qcombobox.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qlayout.h>

SpawnListWindow2::SpawnListWindow2(Player* player, 
				   SpawnShell* spawnShell,
				   CategoryMgr* categoryMgr,
				   QWidget* parent, const char* name)
  : SEQWindow("SpawnList2", "ShowEQ - Spawns", parent, name),
    m_player(player),
    m_categoryMgr(categoryMgr),
    m_spawnShell(spawnShell),
    m_currentCategory(NULL),
    m_selectedItem(NULL),
    m_menu(NULL),
    m_spawnListItemDict(709),
    m_immediateUpdate(true)
{
  m_spawnListItemDict.setAutoDelete(false);

  // get whether to keep the list sorted or not
  m_keepSorted = pSEQPrefs->getPrefBool("KeepSorted", preferenceName(), false);

  // get whether to make sure the selected item is visible
  m_keepSelectedVisible = 
    pSEQPrefs->getPrefBool("KeepSelectedVisible", preferenceName(), true);

  // get the immediate update setting
  m_immediateUpdate = pSEQPrefs->getPrefBool("ImmediateUpdate", 
					     preferenceName(), false);

  // get framerate 
  int fpm = pSEQPrefs->getPrefInt("FPM", preferenceName(), 10);
  m_delay = 60000L / fpm;

  QBoxLayout* vLayout = new QVBoxLayout(boxLayout());
  QHBoxLayout* hLayout= new QHBoxLayout(vLayout);

  // create the spawn list combo box
  m_categoryCombo = new QComboBox(false, this, "spawnlistcombo");
  m_categoryCombo->setDuplicatesEnabled(false);
  hLayout->addWidget(m_categoryCombo, 0, AlignLeft);
  connect(m_categoryCombo, SIGNAL(activated(int)),
	  this, SLOT(categorySelected(int)));

  // Create the Spawn Counter
  m_totalSpawns = new QLineEdit(this);
  m_totalSpawns->setReadOnly(TRUE);
  m_totalSpawns->setAlignment(AlignCenter);
  m_totalSpawns->setMinimumWidth(5);
  m_totalSpawns->setMaximumWidth(50);
  hLayout->addWidget(m_totalSpawns, 0, AlignCenter);  

  // setup spinbox to control frame rate (FPM)
  m_fpmSpinBox = new QSpinBox(5, 60, 1, this, "fpmSpinBox");
  m_fpmSpinBox->setValue(fpm);
  m_fpmSpinBox->setSuffix("FPM");
  hLayout->addWidget(m_fpmSpinBox, 0, AlignRight);
  connect(m_fpmSpinBox, SIGNAL(valueChanged(int)),
	  this, SLOT(setFPM(int)));

  // enable/disable the spin box
  m_fpmSpinBox->setEnabled(!m_immediateUpdate);

  // create the spawn listview
  m_spawnList = new SEQListView(preferenceName(), 
				this, "spawnlistview");
  vLayout->addWidget(m_spawnList);

  m_spawnList->addColumn ("Name");
  m_spawnList->addColumn ("Lvl", "Level");
  m_spawnList->addColumn ("Hp", "HP");
  m_spawnList->addColumn ("MaxHP");
  if(showeq_params->retarded_coords) 
  {
    m_spawnList->addColumn ("N/S", "Coord1");
    m_spawnList->addColumn ("E/W", "Coord2");
  } 
  else 
  {
    m_spawnList->addColumn ("X", "Coord1");
    m_spawnList->addColumn ("Y", "Coord2");
  }
  m_spawnList->addColumn ("Z", "Coord3");
  m_spawnList->addColumn ("ID");
  m_spawnList->addColumn ("Dist");
  m_spawnList->addColumn ("Race");
  m_spawnList->addColumn ("Class");
  m_spawnList->addColumn ("Info");
  m_spawnList->addColumn ("SpawnTime");
  m_spawnList->addColumn("Deity");
  m_spawnList->addColumn("Body Type", "BodyType");
  m_spawnList->addColumn("Guild Tag", "GuildTag");
  
  // restore the columns settings from preferences
  m_spawnList->restoreColumns();

  // setup timer for refreshing the spawn list
  m_timer = new QTimer(this, "spawnlist2timer");

  // connect a QListView signal to ourselves
  connect(m_spawnList, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(selChanged(QListViewItem*)));
  connect (m_spawnList, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
	   this, SLOT(mousePressEvent(int, QListViewItem*, const QPoint&, int)));
  connect (m_spawnList, SIGNAL(doubleClicked(QListViewItem*)),
	   this, SLOT(mouseDoubleClickEvent(QListViewItem*)));

  // connect SpawnList slots to SpawnShell signals
  connect(m_spawnShell, SIGNAL(addItem(const Item *)),
	  this, SLOT(addItem(const Item *)));
  connect(m_spawnShell, SIGNAL(delItem(const Item *)),
	  this, SLOT(delItem(const Item *)));
  connect(m_spawnShell, SIGNAL(killSpawn(const Item *, const Item*, uint16_t)),
	  this, SLOT(killSpawn(const Item *)));
  connect(m_spawnShell, SIGNAL(selectSpawn(const Item *)),
	  this, SLOT(selectSpawn(const Item *)));
  connect(m_spawnShell, SIGNAL(clearItems()),
	  this, SLOT(clear()));
  if (m_immediateUpdate)
    connect(m_spawnShell, SIGNAL(changeItem(const Item *, uint32_t)),
	    this, SLOT(changeItem(const Item *, uint32_t)));
  
  // connect SpawnList slots to Player signals
  connect(m_player, SIGNAL(posChanged(int16_t,int16_t,int16_t,
				      int16_t,int16_t,int16_t,int32_t)), 
	  this, SLOT(setPlayer(int16_t,int16_t,int16_t,
			       int16_t,int16_t,int16_t,int32_t)));
  connect(m_player, SIGNAL(levelChanged(uint8_t)),
	  this, SLOT(playerLevelChanged(uint8_t)));
  
  // connect SpawnList slots to CategoryMgr signals
  connect(m_categoryMgr, SIGNAL(addCategory(const Category*)),
	  this, SLOT(addCategory(const Category*)));
  connect(m_categoryMgr, SIGNAL(delCategory(const Category*)),
	  this, SLOT(delCategory(const Category*)));
  connect(m_categoryMgr, SIGNAL(clearedCategories()),
	  this, SLOT(clearedCategories()));
  connect(m_categoryMgr, SIGNAL(loadedCategories()),
	  this, SLOT(loadedCategories()));

  // connect SpawnList slots to QTimer signals
  connect(m_timer, SIGNAL(timeout()),
	  this, SLOT(refresh()));

  // populate the categories list which will in turn populate the spawn list
  loadedCategories();

  // enable/disable the spin box
  m_fpmSpinBox->setEnabled(!m_immediateUpdate);

  if (!m_immediateUpdate)
    m_timer->start(m_delay);
}  

SpawnListWindow2::~SpawnListWindow2()
{
}

SpawnListItem* SpawnListWindow2::selected()
{
   return ((SpawnListItem*) m_spawnList->selectedItem());
}

SpawnListItem* SpawnListWindow2::find(const Item* item)
{
  return m_spawnListItemDict.find((void*)item);
}

QString SpawnListWindow2::filterString(const Item* item)
{
   if (item == NULL)
     return "";
   
   QString text = ":";

   // get the filter flags
   text += m_spawnShell->filterMgr()->filterString(item->filterFlags());
 
   // get runtime filter flags
   text += m_spawnShell->filterMgr()->runtimeFilterString(item->runtimeFilterFlags());

   // append the filter string
   text += item->filterString();

   // and return thenew and improved filter string.
   return text;
}

QPopupMenu* SpawnListWindow2::menu()
{
  if (m_menu != NULL)
  {
    // make sure the menu is setup
    m_menu->setCurrentItem(0);
    m_menu->setCurrentCategory(m_currentCategory);
    return m_menu;
  }

  m_menu = new SpawnListMenu(m_spawnList, this, m_spawnShell->filterMgr(),
			     m_categoryMgr, this, "spawnlist menu");
  m_menu->insertSeparator(-1);
  int x;
  x = m_menu->insertItem("Immediate Update", 
			 this, SLOT(toggle_immediateUpdate(int)));
  m_menu->setItemChecked(x, m_immediateUpdate);
  x = m_menu->insertItem("Keep Sorted", 
			 this, SLOT(toggle_keepSorted(int)));
  m_menu->setItemChecked(x, m_keepSorted);
  x = m_menu->insertItem("Keep Selected Visible", 
			 this, SLOT(toggle_keepSelectedVisible(int)));
  m_menu->setItemChecked(x, m_keepSelectedVisible);

  m_menu->setCurrentCategory(m_currentCategory);

  return m_menu;
}

void SpawnListWindow2::updateCount()
{
  m_totalSpawns->setText(QString::number(m_spawnList->childCount()));
}

void SpawnListWindow2::addItem(const Item* item)
{
  // just call change item (it will update/remove/add as appropriate)
  changeItem(item, tSpawnChangedALL);
}

void SpawnListWindow2::delItem(const Item* item)
{
  if (!item)
    return;

  // find the list item
  SpawnListItem* litem = find(item);

  // delete the list item
  if (litem != NULL)
  {
    m_spawnListItemDict.remove((void*)item);

    delete litem;

    updateCount();
  }

  if (item == m_selectedItem)
    m_selectedItem = NULL;
}

void SpawnListWindow2::changeItem(const Item* item, uint32_t changeItem)
{
  if (!item)
    return;

  SpawnListItem* litem = NULL;

  // see if the item is already in the list
  litem = find(item);

  // if nothing significant changed, just update it
  if (!(changeItem & (tSpawnChangedName | 
		      tSpawnChangedLevel | 
		      tSpawnChangedNPC | 
		      tSpawnChangedFilter | 
		      tSpawnChangedRuntimeFilter)))
  {
    if (litem != NULL)
      litem->update(m_player, changeItem);

    return;
  }

  // make sure there is a current category, otherwise, nothing gets done
  // This quick check is in case someone deleted all their categories or 
  // are an idiot using an outdated seqdef.xml
  if (m_currentCategory == NULL)
    return;

  uint8_t level = 0;
  
  // if item is a spawn get its level
  if ((item->type() == tSpawn) || (item->type() == tPlayer))
    level = ((Spawn*)item)->level();
    
  // if this is a filtered spawn and the current category isn't a filtered 
  // filter category, then don't add it
  if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
      !m_currentCategory->isFilteredFilter())
  {
    // delete the item (if it already existed)
    if (litem != NULL)
    {
      m_spawnListItemDict.remove((void*)item);
      
      delete litem;

      // update the displayed count
      updateCount();
    }
    
    // nothing more to do
    return;
  }
  
  // if this item doesn't fit the filter
  if (!m_currentCategory->isFiltered(filterString(item), level))
  {
    // delete the item (if it already existed)
    if (litem != NULL)
    {
      m_spawnListItemDict.remove((void*)item);
      
      delete litem;

      // update the displayed count
      updateCount();
    }
    
    // nothing more to do
    return;
  }

  // if their is an item already, just update it
  if (litem != NULL)
  {
    // just update the item
    litem->update(m_player, changeItem);
    
    // make sure it's sorted into the proper place
    if (m_keepSorted)
      m_spawnList->sort();

    // nothing more to do
    return;
  }

  // add the new spawn list item
  litem = new SpawnListItem(m_spawnList);

  // insert it into the dictionary
  m_spawnListItemDict.insert((void*)item, litem);

  // setup the spawn list item
  litem->setShellItem(item);
  litem->update(m_player, tSpawnChangedALL);
  litem->pickTextColor(item, m_player, m_currentCategory->color());

  // update the displayed count
  updateCount();
}

void SpawnListWindow2::killSpawn(const Item* item)
{
  // just call change item (it will update/remove/add as appropriate)
  changeItem(item, tSpawnChangedALL);
}

void SpawnListWindow2::selectSpawn(const Item *item)
{
  if (!item)
    return;

  // cache the selected item
  m_selectedItem = item;

  SpawnListItem* litem = NULL;

  // see if the item is in the list
  litem = find(item);

  if (litem)
  {
    // select the item
    setSelectedQuiet(litem, true);

    // make sure item is visible if configured to do so
    if (m_keepSelectedVisible)
      m_spawnList->ensureItemVisible(litem);
  }  
}

void SpawnListWindow2::clear(void)
{
  // clear out the spawn list item dictionary
  m_spawnListItemDict.clear();

  // clear the spawn list contents
  m_spawnList->clear();
}

void SpawnListWindow2::addCategory(const Category* cat)
{
  // add the new category to the combo box
  m_categoryCombo->insertItem(cat->name());

  // set it to be the current item
  m_categoryCombo->setCurrentItem(m_categoryCombo->count() - 1);
}

void SpawnListWindow2::delCategory(const Category* cat)
{
  int count = m_categoryCombo->count();
  int i;
  for (i = 0; i < count; i++)
  {
    if (m_categoryCombo->text(i) == cat->name())
      break;
  }

  if (i < count)
  {
    // remove the item from the combo box
    m_categoryCombo->removeItem(i);

    // if the category being removed was the current category, 
    // set the selected category to the new selected category (if any).
    if (cat == m_currentCategory)
      categorySelected(m_categoryCombo->currentItem());
  }
}

void SpawnListWindow2::clearedCategories(void)
{
  // clear the category combo box
  m_categoryCombo->clear();

  // clear out the list
  clear();
}

void SpawnListWindow2::loadedCategories(void)
{
  // stop widget updates
  setUpdatesEnabled(false);

  // clear the category combo box
  m_categoryCombo->clear();

  // fill in the category combo box
  CategoryListIterator it(m_categoryMgr->getCategories());
  const Category* cat;
  for (cat = it.toFirst(); cat != NULL; cat = ++it)
    m_categoryCombo->insertItem(cat->name());

  int n = pSEQPrefs->getPrefInt("CurrentCategory", preferenceName(), 0);
  m_categoryCombo->setCurrentItem(n);
  categorySelected(m_categoryCombo->currentItem());

  // clear the spawn list
  clear();

  // populate the spawn list
  populateSpawns();

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();
}

void SpawnListWindow2::playerLevelChanged(uint8_t)
{
  if (m_currentCategory == NULL)
    return;

  QListViewItemIterator it(m_spawnList);
  SpawnListItem* slitem = NULL;
  const Item* item;

  // iterate until we are out of items
  while (it.current())
  {
    // get the current SpawnListItem
    slitem = (SpawnListItem*)it.current();

    // get the item associated with the list item
    item = slitem->item();
    
    // set the color
    slitem->pickTextColor(item, m_player, m_currentCategory->color());

    ++it;
  }
}

void SpawnListWindow2::setPlayer(int16_t x, int16_t y, int16_t z, 
			   int16_t deltaX, int16_t deltaY, int16_t deltaZ, 
			   int32_t degrees)
{
  QListViewItemIterator it(m_spawnList);
  SpawnListItem* litem;
  QString buff;

  if (!showeq_params->fast_machine)
  {
    // no, cheat using integer distance calculation ignoring Z dimension
    while (it.current())
    {
      // get the current item
      litem = (SpawnListItem*)it.current();
      
       if (litem->type() != tUnknown) 
       {
	 buff.sprintf("%5d", litem->item()->calcDist2DInt(x, y));
	 litem->setText(tSpawnColDist, buff);
       }

      // keep iterating
      ++it;
    }
  }
  else
  {
    // fast machine so calculate the correct floating point 3D distance
    while (it.current())
    {
      // get the current item
      litem = (SpawnListItem*)it.current();
      
       if (litem->type() != tUnknown) 
       {
	 buff.sprintf("%5.1f", litem->item()->calcDist(x, y, z));
	 litem->setText(tSpawnColDist, buff);
       }

      // keep iterating
      ++it;
    }
  }
}

void SpawnListWindow2::rebuildSpawnList(void)
{
  // clear the spawn list contents
  clear();

  // re-populate the spawn list
  populateSpawns();
}

void SpawnListWindow2::refresh(void)
{
#if 0 // ZBTEMP
  QTime test;
  test.start();
  seqDebug("SpawnListWindow2::refresh(void) Category=%08x '%s' %d:%d:%d",
	  m_currentCategory, 
	  (m_currentCategory != NULL) ? 
	  (const char*) m_currentCategory->name() : "NONE",
	  test.hour(), test.minute(), test.second());
  test.start();
#endif

  if (m_currentCategory == NULL)
    return;

  // disable updates
  setUpdatesEnabled(false);

  // types of items to populate category with
  spawnItemType types[] = { tSpawn, tDrop, tDoors, tPlayer };

  const Item* item;
  SpawnListItem* litem;

  // iterate over all spawn types
  for (uint8_t i = 0; i < (sizeof(types) / sizeof(spawnItemType)) ; i++)
  {
    uint8_t level = 0;
    const ItemMap& itemMap = m_spawnShell->getConstMap(types[i]);
    ItemConstIterator it(itemMap);

    // iterate over all spawns in of the current type
    for (; it.current(); ++it)
    {
      // get the item from the list
      item = it.current();

      // if item hasn't changed since last update, then nothing to do, next...
      if (item->lastChanged() <= m_lastUpdate)
	continue;

      // get the underlying listview item (if any)
      litem = find(item);

      // skip filtered spawns
      if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	  !m_currentCategory->isFilteredFilter())
      {
	// delete the item (if it already existed)
	if (litem != NULL)
	{
	  m_spawnListItemDict.remove((void*)item);
	  
	  delete litem;
	}

	// nothing more to do for this item
	continue;
      }

      // if item is a spawn get its level
      if (item->type() == tSpawn)
	level = ((Spawn*)item)->level();


      // if this item doesn't fit the filter
      if (!m_currentCategory->isFiltered(filterString(item), 0))
      {
	// delete the item (if it already existed)
	if (litem != NULL)
	{
	  m_spawnListItemDict.remove((void*)item);
	  
	  delete litem;
	}
    
	// nothing more to do for this item
	continue;
      }

      // if their is an item already, just update it
      if (litem != NULL)
      {
	// just update the item
	litem->update(m_player, tSpawnChangedALL);
	
	// nothing more to do for this item
	continue;
      }

      // add the new spawn list item
      litem = new SpawnListItem(m_spawnList);
      
      // insert it into the dictionary
      m_spawnListItemDict.insert((void*)item, litem);
      
      // setup the spawn list item
      litem->setShellItem(item);
      litem->update(m_player, tSpawnChangedALL);
      litem->pickTextColor(item, m_player, m_currentCategory->color());
    }
  }

  // note the time of the last update
  m_lastUpdate = time(NULL);

#if 0 // ZBTEMP
  seqDebug("* elapsed (pre-sort): %d", test.elapsed());
#endif 

  // make sure the spawnlist is sorted
  if (m_keepSorted)
    m_spawnList->sort();

  // make sure the selected item is selected
  if (m_selectedItem)
    selectSpawn(m_selectedItem);

#if 0 // ZBTEMP
  seqDebug("* elapsed (post-sort): %d", test.elapsed());
#endif 

  // update the displayed count
  updateCount();

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();

#if 0 // ZBTEMP
  seqDebug("* elapsed (post-paint): %d", test.elapsed());
#endif 

  if (!m_immediateUpdate)
    m_timer->start(m_delay, true);
}

void SpawnListWindow2::savePrefs(void)
{
  // save the current category
  pSEQPrefs->setPrefInt("CurrentCategory", preferenceName(), 
			m_categoryCombo->currentItem());

  // save the underlying SEQWindows prefs
  SEQWindow::savePrefs();
  
  // save the SEQListViews prefs
  m_spawnList->savePrefs();
}


void SpawnListWindow2::categorySelected(int index)
{
  CategoryListIterator it(m_categoryMgr->getCategories());
  Category* cat = it.toFirst();
  int i = 0;
  while ((cat != NULL) && (i < index))
  {
    cat = ++it;
    ++i;
  }

  // set the current category
  m_currentCategory = cat;

  // clear the spawn list contents
  clear();

  // re-populate the spawn list
  populateSpawns();
}

void SpawnListWindow2::selChanged(QListViewItem* litem)
{
  if (litem == NULL)
    return;
  
  m_selectedItem = ((SpawnListItem*)litem)->item();

  // it might have been a category title selected, only select if it's an item
  if (m_selectedItem != NULL)
    emit spawnSelected(m_selectedItem);
}

void SpawnListWindow2::mousePressEvent(int button, QListViewItem* litem,
				       const QPoint &point, int col)
{
  // Left Mouse Button Events
  if (button  == LeftButton && litem != NULL)
  {
    m_spawnList->setSelected(litem, true);
  } // Right Mouse Button Events
  else if (button == RightButton)
  {
    const Item* item = NULL;
    if (litem != NULL)
    {
      SpawnListItem* slitem = (SpawnListItem*)litem;
      item = slitem->item();
    }
    SpawnListMenu* spawnMenu = (SpawnListMenu*)menu();
    spawnMenu->setCurrentItem(item);
    spawnMenu->popup(point);
  }
}

void SpawnListWindow2::mouseDoubleClickEvent(QListViewItem* litem)
{
   //print spawn info to console
  if (litem == NULL)
    return;

  const Item* item = ((SpawnListItem*)litem)->item();
  if (item != NULL)
    seqInfo("%s",(const char*)filterString(item));
}


void SpawnListWindow2::setFPM(int rate)
{
  pSEQPrefs->setPrefInt("FPM", preferenceName(), rate);
  m_delay = 60000L / rate;
  if (m_timer->isActive())
    m_timer->start(m_delay, true);
}

void SpawnListWindow2::toggle_immediateUpdate(int id)
{
  // toggle immediate update value
  m_immediateUpdate = !m_immediateUpdate;
  menu()->setItemChecked(id, m_immediateUpdate);
  pSEQPrefs->setPrefBool("ImmediateUpdate", preferenceName(), 
			 m_immediateUpdate);

  // connect/disconnect item change notification and stop/start the timer
  // as appropriate for the new immediate update setting
  if (m_immediateUpdate)
  {
    m_timer->stop();
    connect(m_spawnShell, SIGNAL(changeItem(const Item *, uint32_t)),
	    this, SLOT(changeItem(const Item *, uint32_t)));
  }
  else
  {
    disconnect(m_spawnShell, SIGNAL(changeItem(const Item *, uint32_t)),
	       this, SLOT(changeItem(const Item *, uint32_t)));
    m_timer->start(m_delay);
  }

  // enable/disable the spin box
  m_fpmSpinBox->setEnabled(!m_immediateUpdate);
}

void SpawnListWindow2::toggle_keepSorted(int id)
{
  // toggle immediate update value
  m_keepSorted = !m_keepSorted;
  menu()->setItemChecked(id, m_keepSorted);
  pSEQPrefs->setPrefBool("KeepSorted", preferenceName(), 
			 m_keepSorted);
}

void SpawnListWindow2::toggle_keepSelectedVisible(int id)
{
  // toggle immediate update value
  m_keepSelectedVisible = !m_keepSelectedVisible;
  menu()->setItemChecked(id, m_keepSelectedVisible);
  pSEQPrefs->setPrefBool("KeepSelectedVisible", preferenceName(), 
			 m_keepSelectedVisible);
}

void SpawnListWindow2::setSelectedQuiet(QListViewItem* item, bool selected)
{
  if (!item || (item->isSelected() == selected) ||
      !item->isSelectable())
    return;

  // get the old selection
  QListViewItem *oldItem = m_spawnList->selectedItem();

  // unselect the old selected item if any
  if ((oldItem != item) && (oldItem != NULL) && (oldItem->isSelected()))
    oldItem->setSelected(false);

  // mark the new selected item
  item->setSelected(selected);

  // set the selected item to be the current item (should signal selection 
  // notifications since the selection state is already changed).
  m_spawnList->setCurrentItem(item);

  // repaint the old item
  if (oldItem != NULL)
    m_spawnList->repaintItem(oldItem);

  // repaint the selected item
  m_spawnList->repaintItem(item);
}

void SpawnListWindow2::populateSpawns(void)
{
#if 0 // ZBTEMP
  seqDebug("SpawnListWindow2::populateSpawns(void) Category=%08x '%s'",
	  m_currentCategory, 
	  (m_currentCategory != NULL) ? 
	  (const char*) m_currentCategory->name() : "NONE");
  QTime test;
  test.start();
#endif

  if (m_currentCategory == NULL)
    return;

  // disable updates
  setUpdatesEnabled(false);

  // types of items to populate category with
  spawnItemType types[] = { tSpawn, tDrop, tDoors, tPlayer };

  const Item* item;
  SpawnListItem* litem;

  // iterate over all spawn types
  for (uint8_t i = 0; i < (sizeof(types) / sizeof(spawnItemType)); i++)
  {
    const ItemMap& itemMap = m_spawnShell->getConstMap(types[i]);
    ItemConstIterator it(itemMap);
    uint8_t level = 0;

    // iterate over all spawns in of the current type
    for (; it.current(); ++it)
    {
      // get the item from the list
      item = it.current();

      // skip filtered spawns
      if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	  !m_currentCategory->isFilteredFilter())
	continue;

      // if item is a spawn, get its level
      if ((item->type() == tSpawn) || (item->type() == tPlayer))
	level = ((Spawn*)item)->level();

      // does this spawn match the category
      if (m_currentCategory->isFiltered(filterString(item), level))
      {
	// yes, add it
	litem = new SpawnListItem(m_spawnList);

	// insert it into the dictionary
	m_spawnListItemDict.insert((void*)item, litem);

	// set up the list item
	litem->setShellItem(item);
	litem->update(m_player, tSpawnChangedALL);

	// color the spawn
	litem->pickTextColor(item, m_player, m_currentCategory->color());
      }
    }
  }

  // note the time of the last update
  m_lastUpdate = time(NULL);

#if 0 // ZBTEMP
  seqDebug("* elapsed (pre-sort): %d", test.elapsed());
#endif 

  // make sure the spawnlist is sorted
  if (m_keepSorted)
    m_spawnList->sort();

  // update the count display
  updateCount();

  // make sure the selected item is selected
  if (m_selectedItem)
    selectSpawn(m_selectedItem);

#if 0 // ZBTEMP
  seqDebug("* elapsed (post-sort): %d", test.elapsed());
#endif 

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();

#if 0 // ZBTEMP
  seqDebug("* elapsed (post-paint): %d", test.elapsed());
#endif 
}

#include "spawnlist2.moc"
