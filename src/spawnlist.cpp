/*
 * spawnlist.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

/*
 * Orig Author - Maerlyn (MaerlynTheWiz@yahoo.com)
 * Date   - 3/16/00
 */

/*
 * SpawnListItem
 *
 * SpawnListItem is class intended to store information about an EverQuest
 * Spawn.  It inherits from QListViewItem but overrides functionality to allow
 * paint styles such as color changes
 *
 */

#include "spawnlist.h"
#include "category.h"
#include "spawnshell.h"
#include "filtermgr.h"
#include "util.h"
#include "player.h"
#include "diagnosticmessages.h"

#include <stddef.h>
#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <math.h>
#include <regex.h>

// ------------------------------------------------------
SpawnList::SpawnList(Player* player, 
		     SpawnShell* spawnShell,
		     CategoryMgr* categoryMgr,
		     QWidget *parent, const char* name)
  : SEQListView("SpawnList", parent, name),
    m_categoryMgr(categoryMgr),
    m_player(player),
    m_spawnShell(spawnShell),
    m_menu(NULL)
{
   setRootIsDecorated(true);

   addColumn ("Name");
   addColumn ("Lvl", "Level");
   addColumn ("Hp", "HP");
   addColumn ("MaxHP");
   if(showeq_params->retarded_coords) 
   {
     addColumn ("N/S", "Coord1");
     addColumn ("E/W", "Coord2");
   } 
   else 
   {
     addColumn ("X", "Coord1");
     addColumn ("Y", "Coord2");
   }
   addColumn ("Z", "Coord3");
   addColumn ("ID");
   addColumn ("Dist");
   addColumn ("Race");
   addColumn ("Class");
   addColumn ("Info");
   addColumn ("SpawnTime");
   addColumn("Deity");
   addColumn("Body Type", "BodyType");
   addColumn("Guild Tag", "GuildTag");

   // restore the columns settings from preferences
   restoreColumns();

   // connect a QListView signal to ourselves
   connect(this, SIGNAL(selectionChanged(QListViewItem*)),
	   this, SLOT(selChanged(QListViewItem*)));

   connect (this, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
            this, SLOT(mousePressEvent(int, QListViewItem*, const QPoint&, int)));

   connect (this, SIGNAL(doubleClicked(QListViewItem*)),
            this, SLOT(mouseDoubleClickEvent(QListViewItem*)));

   // connect SpawnList slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(addItem(const Item *)),
	   this, SLOT(addItem(const Item *)));
   connect(m_spawnShell, SIGNAL(delItem(const Item *)),
	   this, SLOT(delItem(const Item *)));
   connect(m_spawnShell, SIGNAL(changeItem(const Item *, uint32_t)),
	   this, SLOT(changeItem(const Item *, uint32_t)));
   connect(m_spawnShell, SIGNAL(killSpawn(const Item *, const Item*, uint16_t)),
	   this, SLOT(killSpawn(const Item *)));
   connect(m_spawnShell, SIGNAL(selectSpawn(const Item *)),
	   this, SLOT(selectSpawn(const Item *)));
   connect(m_spawnShell, SIGNAL(clearItems()),
	   this, SLOT(clear()));

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

   // populate the initial spawn list
   rebuildSpawnList();
}

void SpawnList::setPlayer(int16_t x, int16_t y, int16_t z, 
			   int16_t deltaX, int16_t deltaY, int16_t deltaZ, 
			   int32_t degrees)
{
//   seqDebug("SpawnList::setPlayer()");
   char buff[200];  

   SpawnListItem *i = (SpawnListItem*)firstChild();
//   if (i) seqDebug("============= firstChild, name=%s type=%s", i->item()->name().data(), i->type());

   // is this a fast machine?
   if (!showeq_params->fast_machine)
   {
     // no, cheat using integer distance calculation ignoring Z dimension
     while (i != NULL) 
     {   
       if (i->type() != tUnknown) 
       {
	 sprintf(buff, "%5d", i->item()->calcDist2DInt(x, y));
	 i->setText(tSpawnColDist, buff);
       }
       i = (SpawnListItem*)i->nextSibling();
     }
   }
   else
   {
     // fast machine so calculate the correct floating point 3D distance
     while (i != NULL) 
     {   
       if (i->type() != tUnknown) 
       {
	 sprintf(buff, "%5.1f", i->item()->calcDist(x, y, z));
	 i->setText(tSpawnColDist, buff);
       }
       i = (SpawnListItem*)i->nextSibling();
     }
   }
}

void SpawnList::changeItem(const Item* item, uint32_t changeItem)
{
  if (item == NULL)
    return;

  QListViewItemIterator it(this);
  SpawnListItem *i = Find(it, item);
  while (i) 
  {
    // reinsert only if level, NPC or filterFlags changes
    if (!(changeItem & (tSpawnChangedLevel |
			tSpawnChangedNPC | 
			tSpawnChangedFilter | 
			tSpawnChangedRuntimeFilter)))
      i->update(m_player, changeItem);
    else 
    {
      bool select = false;
      
      // check if this is the selected item.
      if (Selected() == i)
	select = true;
      
      // delete ALL SpawnListItems that relate to item
      delItem(item);
      
      // reinsert ALL the SpawnListItems that relate to item
      addItem(item);
      
      // reset the selected item, if it was this item.
      if (select)
	selectSpawn(item);
      
      // Delete item deleted everything, addItem re-inserted everything
      // nothing more to be done.
      break;  
    }
    
    // keep searching/updating...
    i = Find(it, item);
  } // while i
}

void SpawnList::killSpawn(const Item* item)
{
  if (item == NULL)
    return;

   QListViewItemIterator it(this);
   const SpawnListItem *i = Find(it, item);
   // was this spawn in the list
   if (i) 
   {
     // yes, remove and re-add it.
     bool select = false;
       
     // check if this is the selected item.
     if (Selected() == i)
       select = true;
     
     // delete ALL SpawnListItems that relate to item
     delItem(item);
     
     // reinsert ALL the SpawnListItems that relate to item
     addItem(item);
     
     // reset the selected item, if it was this item.
     if (select)
       selectSpawn(item);
     
     // Delete item deleted everything, addItem re-inserted everything
     // nothing more to be done.
   }
   else // no, killed something not in list, just add it.
     addItem(item);
}

SpawnListItem* SpawnList::Find(QListViewItemIterator& it, 
				const Item* item, 
				bool first)
{
  if (first) 
    it = QListViewItemIterator(this); // reset iterator to the beginning
  else
    it++; // increment past the current item

  SpawnListItem *i;
  // while there are still items, increment forward
  while(it.current())
  {
    // get the current item
    i = (SpawnListItem*)it.current();

    // is it the one we're looking for?
    if (i->item() == item)
      return i; // yes, return it

    // keep iterating
    ++it;
  }

  // not found, return NULL
  return NULL;
}

// Slot coming from SpawnShell::addItem.  Called when any spawn is created
void SpawnList::addItem(const Item* item)
{
  if (item == NULL)
    return;

  // ZB: Need to figure out how to derive flags
  int flags = 0;

  QListViewItemIterator it(this);
  const Item* i;
  SpawnListItem* j = NULL;

  // if none found in list, add it
  const Spawn* spawn = NULL;
  if ((item->type() == tSpawn) || (item->type() == tPlayer))
    spawn = (const Spawn*)item;

  uint8_t level = 0;

  if (spawn != NULL)
    level = spawn->level();

  // check if the ID is already in the list
  j = Find(it, item);
  
  if (j) 
  {
    // yes, check if it's a major modification, or can get by with just
    // an update
    int l = j->text(tSpawnColLevel).toInt();

    // reinsert only if name, level, NPC, or filterFlags changes
    if ((l == level) &&
	(j->m_npc == item->NPC()) &&
	(j->text(tSpawnColName) == item->name()))
    {
      // it matches, just update all of it's instances

      // loop through all instances relating to item
      while (j != NULL)
      {
	// update the SpawnListItem
	j->update(m_player, tSpawnChangedALL);

	// find the next one
	j = Find(it, item);
      }
      
      // return the first one so the caller has the option of selecting it
      return;
    } 
    else 
    {
      // major change, delete all instances relating to item
      delItem(item);
    }
  }

  // if this is a pet, make it the child of the owner
  if ((spawn != NULL) && (spawn->petOwnerID()))
  {
    // loop through all matches on owner and add as child
    i = m_spawnShell->findID(tSpawn, spawn->petOwnerID());

    // can only do this if the pet owner's already been seen.
    if (i)
    {
      // start at the beginning
      j = Find(it, i, true);

      // loop until we run out of pets
      while (j) 
      {
	// create a new SpawnListItem 
	SpawnListItem *k = new SpawnListItem(j);
	
	// set the item
	k->setShellItem(item);
	k->pickTextColor(item, m_player);
	k->update(m_player, tSpawnChangedALL);
	
	// find the next item
	j = Find(it, i);
      }
    }
  } // if petOwnerId

  // get the filter string for use in filtering by category
  QString filterStr = filterString(item, flags);

   // Next, add the spawn to each appropriate category
  if (m_categoryMgr->count())
  {
    CategoryListIterator cit(m_categoryMgr->getCategories());
    const Category* cat;
    SpawnListItem* catlitem;

    // iterate over all the categories
    for(cat = cit.toFirst(); cat != NULL; cat = ++cit)
    { 
      // skip filtered spawns, if this isn't a filtered filter category
      if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	  !cat->isFilteredFilter())
      {
	continue;
      }

      if (cat->isFiltered(filterStr, level))
      {
	// retrieve the list item associated with the category
	catlitem = m_categoryListItems.find((void*)cat);

	// We have a good category, add spawn as it's child
	j = new SpawnListItem(catlitem);
#if 0
	seqDebug("`-- Adding to %s (%d)", 
	       (const char*)cat->name(), catlitem->childCount());
#endif
	j->setShellItem(item);
	j->update(m_player, tSpawnChangedALL);

	// color spawn
	j->pickTextColor(item, m_player, cat->color());
	
	// update childcount in header
	catlitem->updateTitle(cat->name());
      } // end if spawn should be in this category
    }
   } // end if categories
   else 
   {
     // just create a new SpawnListItem
     j = new SpawnListItem(this);
     j->setShellItem(item);
     
     // color spawn
     j->pickTextColor(item, m_player);
     j->update(m_player, tSpawnChangedALL);
   } // else
   
   return;
} // end addItem

void SpawnList::delItem(const Item* item)
{
//   seqDebug("SpawnList::delItem() id=%d", id);
  if (item == NULL)
    return;

   SpawnListItem *j = NULL;

   // create a list of items to be deleted
   QList<QListViewItem>* delList = new QList<QListViewItem>();

   // set the list to automatically delete the items placed in it when it is
   // cleared/deleted...
   delList->setAutoDelete(true);

   // create a list of categories to be updated
   QList<const Category> catUpdateList;

   // make sure it doesn't attempt to delete the category
   catUpdateList.setAutoDelete(false);

   const Category* cat;

   // start at the top of the list
   QListViewItemIterator it(this);
   
   do 
   {
     // find the next item in the list
     j = Find(it, item);

     // if there was an item, delete it and all it's children
     if (j) 
     {
       //       seqDebug("  Deleting...");
       // delete children
       QListViewItem* child = j->firstChild();
       QListViewItem* next;
       while(child) 
       {
	 // get the next child
	 next = (SpawnListItem *) child->nextSibling();

	 // add to the list of items to delete
	 delList->append(child);

	 // the next child is now the current child
	 child = next;
       }

       // get the category that the item SpawnListItem belongs to
       cat = getCategory(j);

       // add to the list of items to delete
       delList->append(j);

       // if there's a category, add it to the list to be updated
       if (cat != NULL)
	 catUpdateList.append(cat);
     } // if j

     // not done until out of items
   } while (j);

   // delete the list of items to be deleted, which auto-deletes the items
   delete delList;

   // now iterate over the updated categories and update them
   for (cat = catUpdateList.first(); cat != 0; cat = catUpdateList.next())
   {
     // retrieve the category list item
     SpawnListItem* catlitem = m_categoryListItems.find((void*)cat);
     
     // update the list items title
     catlitem->updateTitle(cat->name());
   }
}

void SpawnList::selectSpawn(const Item *item)
{
//    seqDebug("SpawnList::selectSpawn(name=%s)", item->name().latin1());
  if (item == NULL)
    return;

  // start iterator at the beginning of this QListView
  QListViewItemIterator it(this);

  SpawnListItem *j = NULL;

  // attempt to find a match on an item that is not collapsed (open)
  do 
  {
    // attempt to find the item
    j = Find(it, item);

    // if it's found, see if it's parent is open, and if so, select it
    if (j) 
    {
      // get the parent
      QListViewItem* litem = (SpawnListItem*) j->parent();
      bool bOpen = true;

      // make sure the parent and all it's parents are open
      while (litem) 
      {
	// is it open
	if (!litem->isOpen()) 
	{
	  // nope, stop looking at the parents, next item
	  bOpen = false;
	  break;
	} 
	
	// get this parents parent
	litem = (SpawnListItem*) litem->parent();
      }

      // yes, this one should be opened, finished
      if (bOpen)
	break;
    }

    // continue until out of items
  } while (j);
  
  // if an item was found, select it
  if (j) 
  {
    // select the item
    setSelectedQuiet(j, true);
    
    // if configured to do so, make sure it's visible
    if (showeq_params->keep_selected_visible)
      ensureItemVisible(j);
  }
  else // try again forcing open
  {
    // find the first item in the QListView
    j = Find(it, item, true);

    // if it was found, open it
    if (j)
      selectAndOpen(j);
  }
} // end selectSpawn

SpawnListItem* SpawnList::Selected()
{
   return ((SpawnListItem*) selectedItem());
}


void SpawnList::selectAndOpen(SpawnListItem *i)
{
  // get the item
  QListViewItem* item = i;
  
  // loop over it's parents, opening all of them
  while (item) 
  {
    item->setOpen(true);
    item = (SpawnListItem*) item->parent();
  }

  // make sure the item is selected
  setSelectedQuiet(i, true);

  // if configured to do so, make sure it's visible
  if (showeq_params->keep_selected_visible)
    ensureItemVisible(i);
}

void SpawnList::setSelectedQuiet(QListViewItem* item, bool selected)
{
  if (!item || (item->isSelected() == selected) ||
      !item->isSelectable())
    return;

  // get the old selection
  QListViewItem *oldItem = selectedItem();

  // unselect the old selected item if any
  if ((oldItem != item) && (oldItem != NULL) && (oldItem->isSelected()))
    oldItem->setSelected(false);

  // mark the new selected item
  item->setSelected(selected);

  // set the selected item to be the current item (should signal selection 
  // notifications since the selection state is already changed).
  setCurrentItem(item);

  // repaint the old item
  if (oldItem != NULL)
    repaintItem(oldItem);

  // repaint the selected item
  repaintItem(item);
}

// Select next item of the same type and id as currently selected item
void SpawnList::selectNext(void)
{
//   seqDebug("SpawnList::selectNext()");
  SpawnListItem *i;
  const Item* item;

  // retrieve the currently selected item
  i = (SpawnListItem *) selectedItem();

  // nothing selected, nothing to do
  if (!i)
    return;

  // start the iterator at the current item
  QListViewItemIterator it(i);

  // get the Item from the SpawnListItem
  item = i->item();

  //seqDebug("SelectNext(): Current selection '%s'", i->text(0).latin1());

  // attempt to find another one
  i = Find(it, item);

  // there are no more with item, wrap around to beginning
  if (!i)  
    i = Find(it, item, true);

  // if it's found, select it, and make sure it's parents are open
  if (i) 
  {
    //seqDebug("SelectNext(): Next selection '%s'", i->text(0).latin1());
    selectAndOpen(i);
  }
} // end selectNext


void SpawnList::selectPrev(void)
{
//   seqDebug("SpawnList::SelectPrev()");
  SpawnListItem *i, *last, *cur;
  const Item* item;

  // start at the current item
  i = cur = (SpawnListItem *) selectedItem();

  // nothing selected, nothing to do
  if (!i)
    return;

  // start the iterator at the current item
  QListViewItemIterator it(i);

  // get the SpawnShellitem from the SpawnListItem
  item = i->item();

  // no last item found
  last = NULL;

//seqDebug("SelectPrev(): Current selection '%s'", i->text(0).ascii());

  // search backwards, wrapping around, until we hit the current item
  do 
  {
    // the current item becomes the last item
    last = i;

    // attempt to find the next item
    i = Find(it, item);

    // if no more found, then wrap to the beginning
    if (!i) 
    { 
      // Start searching again from the beginning
      i = Find(it, item, true);
    }

    // continue until it's back to the current item
  } while (i != cur);

  // if there is a last item, select and open it.
  if (last) 
  {
    //seqDebug("SelectPrev(): Prev selection '%s'", i->text(0).ascii());
    selectAndOpen(last);
  }
} // end SelectPrev

void SpawnList::clear(void)
{
//seqDebug("SpawnList::clear()");
  QListView::clear();
  m_categoryListItems.clear();

  // rebuild headers
  CategoryListIterator it(m_categoryMgr->getCategories());
  SpawnListItem* litem;
  const Category* cat;
  for (cat = it.toFirst(); cat != NULL; cat = ++it)
  {
    // create the spawn list item
    litem = new SpawnListItem(this);
    
    // insert the category and it's respective list item
    m_categoryListItems.insert((void*)cat, litem);

    // set color
    litem->setTextColor(cat->color());

    // update count
    litem->updateTitle(cat->name());
  }
} // end clear

void SpawnList::addCategory(const Category* cat)
{
  // create a top level spawn list item for the category
  SpawnListItem* litem = new SpawnListItem(this);

  // associate the new spawn list item with the category
  m_categoryListItems.insert((void*)cat, litem);

  // set color
  litem->setTextColor(cat->color());
  
  // update count
  litem->updateTitle(cat->name());

  // populate the category
  populateCategory(cat);
}

void SpawnList::delCategory(const Category* cat)
{
  // retrieve the list item associated with the category
  SpawnListItem* litem = m_categoryListItems.find((void*)cat);

  // if there's a list item associated with this category, clean it out
  if (litem != NULL)
  {
    SpawnListItem *next;
    SpawnListItem *child;

    // remove all children from list
    // start with the first child
    child = (SpawnListItem *) litem->firstChild();
    
    // iterate until the category is out of children
    while (child) 
    {
      // get the next child
      next = (SpawnListItem *) child->nextSibling();
      
      // delete the current child
      delete child;
      
      // the next child is now the current child
      child = next;
    }

    // remove the item from the category list
    m_categoryListItems.remove((void*)cat);

    // delete the list item
    delete litem;
  }
}

void SpawnList::clearedCategories(void)
{
  // clear out the list of category list items
  m_categoryListItems.clear();

  // clear out the list
  QListView::clear();
}

void SpawnList::loadedCategories(void)
{
  // stop widget updates
  setUpdatesEnabled(false);

  // clear the existing stuff
  clear();

  // populate the spawn list
  populateSpawns();

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();
}

void SpawnList::rebuildSpawnList()
{
  // stop widget updates
  setUpdatesEnabled(false);

  // clear the existing stuff
  clear();

  // repopulate the spawn list
  populateSpawns();

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();
}

void SpawnList::playerLevelChanged(uint8_t)
{
  QListViewItemIterator it(this);
  SpawnListItem* slitem = NULL;
  const Category* cat = NULL;
  const Item* item = NULL;
  
  // iterate until we are out of items
  while (it.current())
  {
    // get the current SpawnListItem
    slitem = (SpawnListItem*)it.current();

    // if this is a top level item, see if it's a category item, and if so
    // get the category.
    if (slitem->parent() == NULL)
    {
      cat = NULL;
      QPtrDictIterator<SpawnListItem> it(m_categoryListItems);
      
      for (it.toFirst(); it.current() != NULL; ++it)
      {
	if (slitem == it.current())
	{
	  cat = (const Category*)it.currentKey();
	  break;
	}
      }
    }

    // get the item associated with the list item
    item = slitem->item();
    
    // set the color
    if (cat != NULL)
      slitem->pickTextColor(item, m_player, cat->color());
    else
      slitem->pickTextColor(item, m_player);

    ++it;
  }
}

void SpawnList::populateCategory(const Category* cat)
{
  if (cat == NULL)
    return;

  // disable updates
  setUpdatesEnabled(false);

  // types of items to populate category with
  spawnItemType types[] = { tSpawn, tDrop, tDoors, tPlayer};

  int flags = 0;
  const ItemMap& itemMap = m_spawnShell->spawns();
  ItemConstIterator it(itemMap);
  const Item* item;
  SpawnListItem* litem;
  SpawnListItem* catlitem = m_categoryListItems.find((void*)cat);
  
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
	  !cat->isFilteredFilter())
	continue;

      // if item is a spawn, get its level
      if ((item->type() == tSpawn) || (item->type() == tPlayer))
	level = ((Spawn*)item)->level();

      // does this spawn match the category
      if (cat->isFiltered(filterString(item, flags), level))
      {
	// yes, add it
	litem = new SpawnListItem(catlitem);

	// set up the list item
	litem->setShellItem(item);
	litem->update(m_player, tSpawnChangedALL);

	// color the spawn
	litem->pickTextColor(item, m_player, cat->color());
      }
    }
  }

  // update child count in header
  catlitem->updateTitle(cat->name());

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();
}

void SpawnList::populateSpawns(void)
{
  // types of items to populate category with
  spawnItemType types[] = { tSpawn, tDrop, tDoors, tPlayer };

  int flags = 0;
  const Item* item;
  SpawnListItem* litem;
  SpawnListItem* catlitem;

  // only deal with categories if there are some to deal with
  if (m_categoryMgr->count() != 0)
  {
    const ItemMap& itemMap = m_spawnShell->spawns();
    ItemConstIterator it(itemMap);
    const Category* cat;
    QString filterStr;
    CategoryListIterator cit(m_categoryMgr->getCategories());
   
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

	// retrieve the filter string
	filterStr = filterString(item, flags);

	// iterate over all the categories
	for(cat = cit.toFirst(); cat != NULL; cat = ++cit)
	{ 
	  // skip filtered spawns
	  if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	      !cat->isFilteredFilter())
	    continue;

	  // if item is a spawn, get its level
	  if ((item->type() == tSpawn) || (item->type() == tPlayer))
	    level = ((Spawn*)item)->level();

	  // does this spawn match the category
	  if (cat->isFiltered(filterStr, level))
	  {
	    // retrieve the category list item
	    catlitem = m_categoryListItems.find((void*)cat);

	    // yes, add it
	    litem = new SpawnListItem(catlitem);

	    // set up the list item
	    litem->setShellItem(item);
	    litem->update(m_player, tSpawnChangedALL);
	    
	    // color the spawn
	    litem->pickTextColor(item, m_player, cat->color());
	  }
	}
      }
    }

    // done adding items, now iterate over all the categories and 
    // update the counts
    for(cat = cit.toFirst(); cat != NULL; cat = ++cit)
    {
      catlitem =  m_categoryListItems.find((void*)cat);
      catlitem->updateTitle(cat->name());
    }
  }
  else
  {
    // no categories, just add all the spawns

    // iterate over all spawn types
    for (uint8_t i = 0; i < (sizeof(types) / sizeof(spawnItemType)); i++)
    {
      const ItemMap& itemMap = m_spawnShell->getConstMap(types[i]);
      ItemConstIterator it(itemMap);
      
      // iterate over all spawns in of the current type
      for (; it.current(); ++it)
      {
	// get the item from the list
	item = it.current();

	// just create a new SpawnListItem
	litem = new SpawnListItem(this);
	litem->setShellItem(item);
	
	// color spawn
	litem->pickTextColor(item, m_player);
	litem->update(m_player, tSpawnChangedALL);
      }
    }
  }
}

void SpawnList::selChanged(QListViewItem* litem)
{
  if (litem == NULL)
    return;
  
  const Item* item = ((SpawnListItem*)litem)->item();

  // it might have been a category title selected, only select if it's an item
  if (item != NULL)
    emit spawnSelected(item);
}

void SpawnList::mousePressEvent(int button, QListViewItem* litem,
		                   const QPoint &point, int col)
{
  // Left Mouse Button Events
  if (button  == LeftButton && litem != NULL)
  {
      setSelected(litem, TRUE);
  }

  // Right Mouse Button Events
  if (button == RightButton)
  {
    SpawnListItem* slitem = (SpawnListItem*)litem;
    const Item* item = NULL;
    if (slitem != NULL)
      item = slitem->item();
    SpawnListMenu* spawnMenu = menu();
    spawnMenu->setCurrentItem(item);
    spawnMenu->setCurrentCategory(getCategory(slitem));
    spawnMenu->popup(point);
  }
}

void SpawnList::mouseDoubleClickEvent(QListViewItem* litem)
{
   //print spawn info to console
  if (litem == NULL)
    return;

  const Item* item = ((SpawnListItem*)litem)->item();
  if (item != NULL)
  {
    seqInfo("%s",(const char*)item->filterString());
  }
}

QString SpawnList::filterString(const Item* item, int flags)
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


const Category* SpawnList::getCategory(SpawnListItem *item)
{
  if (item) 
  {
    // find the topmost parent
    SpawnListItem *j = item;
    while (j) 
    {
      if (j->parent() == NULL)
	break;
      j = (SpawnListItem *)j->parent();
    }
    // find that in m_categoryList
    if (j) 
    {
      QPtrDictIterator<SpawnListItem> it(m_categoryListItems);
      
      for (it.toFirst(); it.current() != NULL; ++it)
      {
	if (j == it.current())
	  return (const Category*)it.currentKey();
      }
    }
  }
  
  return NULL;
}

SpawnListMenu* SpawnList::menu()
{
  if (m_menu != NULL)
    return m_menu;
  
  m_menu = new SpawnListMenu(this, (SEQWindow*)parent(), m_spawnShell->filterMgr(),
				 m_categoryMgr, this, "spawnlist menu");

  return m_menu;
}

SpawnListWindow::SpawnListWindow(Player* player, 
				 SpawnShell* spawnShell,
				 CategoryMgr* categoryMgr,
				 QWidget* parent, const char* name)
  : SEQWindow("SpawnList", "ShowEQ - Spawns", parent, name)
{
  m_spawnList = new SpawnList(player, spawnShell, categoryMgr, 
			      this, name);
  setWidget(m_spawnList);
}

SpawnListWindow::~SpawnListWindow()
{
  delete m_spawnList;
}

QPopupMenu* SpawnListWindow::menu()
{
  // retrieve the menu
  SpawnListMenu* spawnMenu = m_spawnList->menu();

  // set it up with safe values (since the user didn't click on anything
  spawnMenu->setCurrentItem(0);
  spawnMenu->setCurrentCategory(0);

  // return the menu
  return spawnMenu;
}

void SpawnListWindow::savePrefs(void)
{
  // save SEQWindow prefs
  SEQWindow::savePrefs();

  // make the listview save it's prefs
  m_spawnList->savePrefs();
}
