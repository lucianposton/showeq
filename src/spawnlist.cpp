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

#include <stddef.h>
#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <math.h>
#include <regex.h>

#include <qpainter.h>
#include <qfont.h>
#include <qheader.h>
#include <qinputdialog.h>
#include <qfontdialog.h>
#include <qmessagebox.h>

#include "spawnlist.h"
#include "category.h"
#include "spawnshell.h"
#include "filtermgr.h"
#include "util.h"
#include "itemdb.h"

//----------------------------------------------------------------------
// Added by Wally59
//----------------------------------------------------------------------
SpawnListItem::SpawnListItem(QListViewItem *parent) : QListViewItem(parent)
{
  m_textColor = Qt::black;
  m_item = NULL;
  m_npc = 0;
  m_filterFlags = 0;
  m_runtimeFilterFlags = 0;
}

SpawnListItem::SpawnListItem(QListView *parent) : QListViewItem(parent)
{
  m_textColor = Qt::black; 
  m_item = NULL;
  m_npc = 0;
  m_filterFlags = 0;
  m_runtimeFilterFlags = 0;
}

SpawnListItem::~SpawnListItem()
{
}

//----------------------------------------------------------------------

//
// paintCell 
//
// overridden from base class in order to change color and style attributes
//
void SpawnListItem::paintCell( QPainter *p, const QColorGroup &cg,
                               int column, int width, int alignment )
{
  QColorGroup newCg( cg );
  
  newCg.setColor( QColorGroup::Text, m_textColor);
  
  QFont font = this->listView()->font();
  
  if (!(m_filterFlags & (FILTER_FLAG_FILTERED |
			 FILTER_FLAG_ALERT |
			 FILTER_FLAG_LOCATE | 
			 FILTER_FLAG_CAUTION |
			 FILTER_FLAG_DANGER)))
  {
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
  }
  else 
  {
    // color filtered spawns grey
    if (m_filterFlags & FILTER_FLAG_FILTERED)
	newCg.setColor( QColorGroup::Text, Qt::gray);
    
    if (m_filterFlags & FILTER_FLAG_ALERT)
      font.setBold(true);
    else
      font.setBold(false);
    
    if (m_filterFlags & FILTER_FLAG_LOCATE)
      font.setItalic(true);
    else
      font.setItalic(false);
    
    if ((m_filterFlags & FILTER_FLAG_CAUTION) || 
	(m_filterFlags & FILTER_FLAG_DANGER))
      font.setUnderline(true);
    else
      font.setUnderline(false);
  }
  
  p->setFont(font);
  
  QListViewItem::paintCell( p, newCg, column, width, alignment );
}

itemType SpawnListItem::type()
{
   return item() ? item()->type() : tUnknown;
}

QString SpawnListItem::key(int column, bool ascending) const
{
  if (m_item == NULL)
    return text(0);
    
  if ((column < SPAWNCOL_LEVEL) || (column > SPAWNCOL_DIST))
    return text(column);

  double num = text(column).toDouble();
  QString textNum;
  textNum.sprintf("%08.2f", num);
  return textNum;
}

void SpawnListItem::update(uint32_t changeType)
{
//   printf ("SpawnListItem::update()\n");
   QString buff;

   if (changeType & tSpawnChangedName)
   {
     // Name
     if (!showeq_params->showRealName)
       setText(SPAWNCOL_NAME, item()->transformedName());
     else
       setText(SPAWNCOL_NAME, item()->name());
   }

   // only spawns contain level info
   if (item()->type() == tSpawn)
   {
     const Spawn* spawn = (const Spawn*)item();

     if (changeType & tSpawnChangedLevel)
     {
       // Level
       buff.sprintf("%2d", spawn->level());
       setText(SPAWNCOL_LEVEL, buff);
     }
     
     if (changeType & tSpawnChangedHP)
     {
       // Hitpoints
       buff.sprintf("%5d", spawn->HP());
       setText(SPAWNCOL_HP, buff);
       
       // Maximum Hitpoints
       buff.sprintf("%5d", spawn->maxHP());
       setText(SPAWNCOL_MAXHP, buff);
     }
   }
   else if (changeType == tSpawnChangedALL)
   {
     buff = "0";
     setText(SPAWNCOL_LEVEL, buff);
     setText(SPAWNCOL_HP, buff);
     setText(SPAWNCOL_MAXHP, buff);
   }

   if (changeType & tSpawnChangedPosition)
   {
     // X position
     buff.sprintf("%5d", showeq_params->retarded_coords ? 
		  (int)item()->yPos() : (int)item()->xPos());
     setText(SPAWNCOL_XPOS, buff);
     
     // Y position
     buff.sprintf("%5d", showeq_params->retarded_coords ? 
		  (int)item()->xPos() : (int)item()->yPos());
     setText(SPAWNCOL_YPOS, buff);
     
     // Z position
     buff.sprintf("%5.1f", item()->displayZPos());
     setText(SPAWNCOL_ZPOS, buff);

     // Distance
     if (!showeq_params->fast_machine)
       buff.sprintf("%5d", ((CSpawnList*)listView())->calcDistInt(item()));
     else
       buff.sprintf("%5.1f", ((CSpawnList*)listView())->calcDist(item()));
     setText(SPAWNCOL_DIST, buff);
   }

   if (changeType == tSpawnChangedALL)
   {
     // Id
     buff.sprintf("%5d", item()->id());
     setText(SPAWNCOL_ID, buff);
     
     // Race
     setText(SPAWNCOL_RACE, item()->raceString());
     
     // Class
     setText(SPAWNCOL_CLASS, item()->classString());
     
     // Spawntime
     setText(SPAWNCOL_SPAWNTIME, m_item->spawnTimeStr());

     // CJD TODO - Deity, PVP teams
   }

   if (changeType & tSpawnChangedWearing)
   {
     // Info
     setText(SPAWNCOL_INFO, item()->info());
   }

   m_npc = item()->NPC();
}

void SpawnListItem::updateTitle(const QString& name)
{
  // update childcount in header
  QString temp;
  temp.sprintf("%s (%d)",
	       (const char*)name, childCount());
  setText(SPAWNCOL_NAME, temp);
} // end if spawn should be in this category

void SpawnListItem::setShellItem(const Item *item)
{
   m_item = item;
   if (item)
      m_npc = item->NPC();
}

// ------------------------------------------------------
CSpawnList::CSpawnList(EQPlayer* player, SpawnShell* spawnShell,
		       CategoryMgr* categoryMgr,
		       QWidget *parent, const char* name)
  : SEQListView("SpawnList", parent, name), 
  m_menu(NULL)
{
   bDebug = FALSE;
   m_player = player;
   m_spawnShell = spawnShell;
   m_categoryMgr = categoryMgr;

   setRootIsDecorated(true);

   addColumn ("Name");
   addColumn ("Lvl", "Level");
   addColumn ("Hp", "HP");
   addColumn ("MaxHP");
   if(showeq_params->retarded_coords) {
     addColumn ("N/S", "Coord1");
     addColumn ("E/W", "Coord2");
   } else {
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

   // restore the columns settings from preferences
   restoreColumns();

   // connect a QListView signal to ourselves
   connect(this, SIGNAL(selectionChanged(QListViewItem*)),
	   this, SLOT(selChanged(QListViewItem*)));

   connect (this, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
            this, SLOT(mousePressEvent(int, QListViewItem*, const QPoint&, int)));

   connect (this, SIGNAL(doubleClicked(QListViewItem*)),
            this, SLOT(mouseDoubleClickEvent(QListViewItem*)));

   // connect CSpawnList slots to SpawnShell signals
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

   // connect CSpawnList slots to EQPlayer signals
   connect(m_player, SIGNAL(posChanged(int16_t,int16_t,int16_t,
				       int16_t,int16_t,int16_t,int32_t)), 
	   this, SLOT(setPlayer(int16_t,int16_t,int16_t,
				int16_t,int16_t,int16_t,int32_t)));
   connect(m_player, SIGNAL(levelChanged(uint8_t)),
	   this, SLOT(playerLevelChanged(uint8_t)));
   
   // connect CSpawnList slots to CategoryMgr signals
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

void CSpawnList::setPlayer(int16_t xPos, int16_t yPos, int16_t zPos, 
			   int16_t deltaX, int16_t deltaY, int16_t deltaZ, 
			   int32_t degrees)
{
//   printf ("CSpawnList::setPlayer()\n");
   char buff[200];  

   SpawnListItem *i = (SpawnListItem*)firstChild();
//   if (i) printf("============= firstChild, name=%s type=%s\n", i->item()->name().data(), i->type());

   // is this a fast machine?
   if (!showeq_params->fast_machine)
   {
     // no, cheat using integer distance calculation ignoring Z dimension
     while (i != NULL) 
     {   
       if (i->type() != tUnknown) 
       {
	 sprintf(buff, "%5d", i->item()->calcDist2DInt(xPos, yPos));
	 i->setText(SPAWNCOL_DIST, buff);
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
	 sprintf(buff, "%5.1f", i->item()->calcDist(xPos, yPos, zPos));
	 i->setText(SPAWNCOL_DIST, buff);
       }
       i = (SpawnListItem*)i->nextSibling();
     }
   }
}

void CSpawnList::changeItem(const Item* item, uint32_t changeItem)
{
   if (item)
   {
//      printf("CSL::changeItem - id=%d name='%s'\n", item->id(), item->name().latin1());
     QListViewItemIterator it(this);
     SpawnListItem *i = Find(it, item);
     while (i) 
     {
//     printf("`-- n = %d\n", n);
       // reinsert only if level, NPC or filterFlags changes
       if (!(changeItem & (tSpawnChangedLevel |
			    tSpawnChangedNPC | 
			    tSpawnChangedFilter | 
			    tSpawnChangedRuntimeFilter)))
	 i->update(changeItem);
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
   } // if item
}

void CSpawnList::killSpawn(const Item* item)
{
   if (item == NULL)
     return;

// printf("CSL::changeItem - id=%d name='%s'\n", item->id(), item->name().latin1());
   QListViewItemIterator it(this);
   const SpawnListItem *i = Find(it, item);
   // was this spawn in the list
   if (i) 
   {
     // yes, remove and re-add it.
//   printf("`-- n = %d\n", n);
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

SpawnListItem* CSpawnList::Find(QListViewItemIterator& it, 
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
void CSpawnList::addItem(const Item* item)
{
  if (!item)
    return;

  // ZB: Need to figure out how to derive flags
  int flags = 0;

//   printf("CSL::addItem - id=%d  name='%s'\n", item->id(), item->name().latin1());
  QListViewItemIterator it(this);
  const Item* i;
  SpawnListItem* j = NULL;

  // if none found in list, add it
  const Spawn* spawn = spawnType(item);

  int level = 0;

  if (spawn != NULL)
    level = spawn->level();

  // check if the ID is already in the list
  j = Find(it, item);
  
  if (j) 
  {
    // yes, check if it's a major modification, or can get by with just
    // an update

    int l = j->text(SPAWNCOL_LEVEL).toInt();

    //printf("-- level=%d l=%d / npc=%d m_npc=%d\n",
    //      (*it)->item()->level(), l, (*it)->item()->NPC(), (*it)->m_npc);

    // reinsert only if name, level, NPC, or filterFlags changes
    if ((l == level) &&
	(j->m_npc == item->NPC()) &&
	(j->text(SPAWNCOL_NAME) == item->name()) && 
	(j->filterFlags() == item->filterFlags()))
    {
      // it matches, just update all of it's instances

      // loop through all instances relating to item
      while (j != NULL)
      {
	// update the SpawnListItem
	j->update(tSpawnChangedALL);

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
	k->setTextColor(pickSpawnColor(item));
	k->setFilterFlags(item->filterFlags());
	k->setRuntimeFilterFlags(item->runtimeFilterFlags());
	k->update(tSpawnChangedALL);
	
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
    CategoryDictIterator cit(m_categoryMgr->getCategories());
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
	printf("`-- Adding to %s (%d)\n", 
	       (const char*)cat->name(), catlitem->childCount());
#endif
	j->setShellItem(item);
	j->setFilterFlags(item->filterFlags());
	j->setRuntimeFilterFlags(item->runtimeFilterFlags());
	j->update(tSpawnChangedALL);

	// color spawn
	j->setTextColor(pickSpawnColor(item, cat->color()));
	
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
     j->setFilterFlags(item->filterFlags());
     j->setRuntimeFilterFlags(item->runtimeFilterFlags());
     
     // color spawn
     j->setTextColor(pickSpawnColor(item));
     j->update(tSpawnChangedALL);
   } // else
   
   return;
} // end addItem

void CSpawnList::delItem(const Item* item)
{
//   printf ("CSpawnList::delItem() id=%d\n", id);
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
       //       printf("  Deleting...\n");
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

void CSpawnList::selectSpawn(const Item *item)
{
//    printf("CSpawnList::selectSpawn(name=%s)\n", item->name().latin1());
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

SpawnListItem* CSpawnList::Selected()
{
   return ((SpawnListItem*) selectedItem());
}


void CSpawnList::selectAndOpen(SpawnListItem *i)
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

void CSpawnList::setSelectedQuiet(QListViewItem* item, bool selected)
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
void CSpawnList::selectNext(void)
{
//   printf ("CSpawnList::selectNext()\n");
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

  //printf("SelectNext(): Current selection '%s'\n", i->text(0).latin1());

  // attempt to find another one
  i = Find(it, item);

  // there are no more with item, wrap around to beginning
  if (!i)  
    i = Find(it, item, true);

  // if it's found, select it, and make sure it's parents are open
  if (i) 
  {
    //printf("SelectNext(): Next selection '%s'\n", i->text(0).latin1());
    selectAndOpen(i);
  }
} // end selectNext


void CSpawnList::selectPrev(void)
{
//   printf ("CSpawnList::SelectPrev()\n");
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

//printf("SelectPrev(): Current selection '%s'\n", i->text(0).ascii());

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
    //printf("SelectPrev(): Prev selection '%s'\n", i->text(0).ascii());
    selectAndOpen(last);
  }
} // end SelectPrev

void CSpawnList::clear(void)
{
//printf("CSpawnList::clear()\n");
  QListView::clear();
  m_categoryListItems.clear();

  // rebuild headers
  CategoryDictIterator it(m_categoryMgr->getCategories());
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

void CSpawnList::addCategory(const Category* cat)
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

void CSpawnList::delCategory(const Category* cat)
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

void CSpawnList::clearedCategories(void)
{
  // clear out the list of category list items
  m_categoryListItems.clear();

  // clear out the list
  QListView::clear();
}

void CSpawnList::loadedCategories(void)
{
  // clear the existing stuff
  clear();

  // populate the spawn list
  populateSpawns();
}

void CSpawnList::rebuildSpawnList()
{
  // clear the existing stuff
  clear();

  // repopulate the spawn list
  populateSpawns();
}

void CSpawnList::playerLevelChanged(uint8_t)
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
      slitem->setTextColor(pickSpawnColor(item, cat->color()));
    else
      slitem->setTextColor(pickSpawnColor(item));

    ++it;
  }
}

void CSpawnList::populateCategory(const Category* cat)
{
  if (cat == NULL)
    return;

  // types of items to populate category with
  itemType types[4] = { tDrop, tCoins, tDoors, };

  int flags = 0;
  const ItemMap& itemMap = m_spawnShell->spawns();
  ItemConstIterator it;
  const Item* item;
  const Spawn* spawn;
  SpawnListItem* litem;
  SpawnListItem* catlitem = m_categoryListItems.find((void*)cat);
  
  // iterate over all spawns in of the current type
  for (it = itemMap.begin();
       it != itemMap.end(); 
       ++it)
  {
    // get the item from the list
    item = it->second;

    // skip filtered spawns
    if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	!cat->isFilteredFilter())
      continue;

    spawn = (const Spawn*)item;

    // does this spawn match the category
    if (cat->isFiltered(filterString(item, flags), spawn->level()))
    {
      // yes, add it
      litem = new SpawnListItem(catlitem);
      
      // set up the list item
      litem->setShellItem(item);
      litem->setFilterFlags(item->filterFlags());
      litem->setRuntimeFilterFlags(item->runtimeFilterFlags());
      litem->update(tSpawnChangedALL);
      
      // color the spawn
      litem->setTextColor(pickSpawnColor(item, cat->color()));
    }
  }

  // iterate over all spawn types
  for (int i = 0; i < 2; i++)
  {
    const ItemMap& itemMap = m_spawnShell->getConstMap(types[i]);

    // iterate over all spawns in of the current type
    for (it = itemMap.begin();
	 it != itemMap.end(); 
	 ++it)
    {
      // get the item from the list
      item = it->second;

      // skip filtered spawns
      if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	  !cat->isFilteredFilter())
	continue;

      // does this spawn match the category
      if (cat->isFiltered(filterString(item, flags), 0))
      {
	// yes, add it
	litem = new SpawnListItem(catlitem);

	// set up the list item
	litem->setShellItem(item);
	litem->setFilterFlags(item->filterFlags());
	litem->setRuntimeFilterFlags(item->runtimeFilterFlags());
	litem->update(tSpawnChangedALL);

	// color the spawn
	litem->setTextColor(pickSpawnColor(item, cat->color()));
      }
    }
  }

  // update child count in header
  catlitem->updateTitle(cat->name());
}

void CSpawnList::populateSpawns(void)
{
  // types of items to populate category with
  itemType types[2] = { tDrop, tCoins, };

  int flags = 0;
  const Item* item;
  const Spawn* spawn;
  SpawnListItem* litem;
  SpawnListItem* catlitem;

  // only deal with categories if there are some to deal with
  if (m_categoryMgr->count() != 0)
  {
    const ItemMap& itemMap = m_spawnShell->spawns();
    ItemConstIterator it;
    const Category* cat;
    QString filterStr;
    CategoryDictIterator cit(m_categoryMgr->getCategories());
  
    // iterate over all spawns in of the current type
    for (it = itemMap.begin();
	 it != itemMap.end(); 
	 ++it)
    {
      // get the item from the list
      item = it->second;

      // retrieve the filter string
      filterStr = filterString(item, flags);
      
      // iterate over all the categories
      for(cat = cit.toFirst(); cat != NULL; cat = ++cit)
      { 
	// skip filtered spawns
	if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	    !cat->isFilteredFilter())
	  continue;
	
	spawn = (const Spawn*)item;

	// does this spawn match the category
	if (cat->isFiltered(filterStr, spawn->level()))
	{
	  // retrieve the category list item
	  catlitem = m_categoryListItems.find((void*)cat);
	  
	  // yes, add it
	  litem = new SpawnListItem(catlitem);
	  
	  // set up the list item
	  litem->setShellItem(item);
	  litem->setFilterFlags(item->filterFlags());
	  litem->setRuntimeFilterFlags(item->runtimeFilterFlags());
	  litem->update(tSpawnChangedALL);
	  
	  // color the spawn
	  litem->setTextColor(pickSpawnColor(item, cat->color()));
	}
      }
    }
   
    // iterate over all spawn types
    for (int i = 0; i < 2; i++)
    {
      const ItemMap& itemMap = m_spawnShell->getConstMap(types[i]);
      
      // iterate over all spawns in of the current type
      for (it = itemMap.begin();
	   it != itemMap.end(); 
	   ++it)
      {
	// get the item from the list
	item = it->second;

	// retrieve the filter string
	filterStr = filterString(item, flags);

	// iterate over all the categories
	for(cat = cit.toFirst(); cat != NULL; cat = ++cit)
	{ 
	  // skip filtered spawns
	  if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	      !cat->isFilteredFilter())
	    continue;

	  // does this spawn match the category
	  if (cat->isFiltered(filterStr, 0))
	  {
	    // retrieve the category list item
	    catlitem = m_categoryListItems.find((void*)cat);

	    // yes, add it
	    litem = new SpawnListItem(catlitem);

	    // set up the list item
	    litem->setShellItem(item);
	    litem->setFilterFlags(item->filterFlags());
	    litem->setRuntimeFilterFlags(item->runtimeFilterFlags());
	    litem->update(tSpawnChangedALL);
	    
	    // color the spawn
	    litem->setTextColor(pickSpawnColor(item, cat->color()));
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
    ItemConstIterator it;

    // iterate over all spawn types
    for (int i = 0; i < 3; i++)
    {
      const ItemMap& itemMap = m_spawnShell->getConstMap(types[i]);
      
      // iterate over all spawns in of the current type
      for (it = itemMap.begin();
	   it != itemMap.end(); 
	   ++it)
      {
	// get the item from the list
	item = it->second;

	// just create a new SpawnListItem
	litem = new SpawnListItem(this);
	litem->setShellItem(item);
	litem->setFilterFlags(item->filterFlags());
	litem->setRuntimeFilterFlags(item->runtimeFilterFlags());
	
	// color spawn
	litem->setTextColor(pickSpawnColor(item));
	litem->update(tSpawnChangedALL);
      }
    }
  }
}


//----------------------------------------------------------------------
//
// pickSpawnColor
// 
// insert color schemes here
//
QColor CSpawnList::pickSpawnColor(const Item* item, QColor def)
{
  if (item == NULL)
    return def;

  const Spawn* spawn = spawnType(item);
  if (spawn == NULL)
    return def;

   // color by pvp team
   if (showeq_params->pvp) 
   {
     switch(spawn->raceTeam()) 
     {
     case RTEAM_HUMAN:
       return (Qt::blue);
       break;

     case RTEAM_ELF:
       return (QColor(196,206,12));
       break;
   
     case RTEAM_DARK:
       return (QColor(206,151,33));
       break;
   
     case RTEAM_SHORT:
       return (Qt::magenta);
       break;
      }
   } // if pvp
   else if (showeq_params->deitypvp) 
   {
     switch(spawn->deityTeam()) 
     {
     case DTEAM_GOOD:
       return (Qt::blue);
       break;
     case DTEAM_NEUTRAL:
       return (QColor(196,206,12));
       break;
     case DTEAM_EVIL:
       return (Qt::magenta);
       break;
     }
   } // if deitypvp
   else 
   { // color by consider difficulty
     QColor color = m_player->pickConColor(spawn->level());
     if (color == Qt::white)
       color = Qt::black;
     if (color == Qt::yellow)
         color = (QColor(206,151,33));
     return color;
   }
   
   return def;
} // end pickSpawnColor

void CSpawnList::selChanged(QListViewItem* litem)
{
  if (litem == NULL)
    return;
  
  const Item* item = ((SpawnListItem*)litem)->item();

  // it might have been a category title selected, only select if it's an item
  if (item != NULL)
    emit spawnSelected(item);
}

void CSpawnList::mousePressEvent(int button, QListViewItem* litem,
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

void CSpawnList::mouseDoubleClickEvent(QListViewItem* litem)
{
   //print spawn info to console
  if (litem == NULL)
    return;

  const Item* item = ((SpawnListItem*)litem)->item();
  if (item != NULL)
  {
    printf("%s\n",(const char*)item->filterString());
  }
}

QString CSpawnList::filterString(const Item* item, int flags)
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


const Category* CSpawnList::getCategory(SpawnListItem *item)
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

SpawnListMenu* CSpawnList::menu()
{
  if (m_menu != NULL)
    return m_menu;
  
  m_menu = new SpawnListMenu(this, m_spawnShell->filterMgr(),
				 m_categoryMgr, this, "spawnlist menu");

  return m_menu;
}

SpawnListMenu::SpawnListMenu(CSpawnList* spawnlist,
			     FilterMgr* filterMgr,
			     CategoryMgr* categoryMgr,
		             QWidget* parent, const char* name)
                             : m_spawnlist(spawnlist)
{
  m_spawnlist = spawnlist;
  m_filterMgr = filterMgr;
  m_categoryMgr = categoryMgr;

  // Show Columns
  QPopupMenu* spawnListColMenu = new QPopupMenu;
  insertItem( "Show &Column", spawnListColMenu);
  spawnListColMenu->setCheckable(true);
  m_id_spawnList_Cols[SPAWNCOL_NAME] = 
    spawnListColMenu->insertItem("&Name");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_NAME], 
				     SPAWNCOL_NAME);
  m_id_spawnList_Cols[SPAWNCOL_LEVEL] = spawnListColMenu->insertItem("&Level");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_LEVEL], 
				     SPAWNCOL_LEVEL);
  m_id_spawnList_Cols[SPAWNCOL_HP] = spawnListColMenu->insertItem("&HP");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_HP], 

				     SPAWNCOL_HP);
  m_id_spawnList_Cols[SPAWNCOL_MAXHP] = 
    spawnListColMenu->insertItem("&Max HP");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_MAXHP], 
				     SPAWNCOL_MAXHP);
  m_id_spawnList_Cols[SPAWNCOL_XPOS] = 
    spawnListColMenu->insertItem("Coord &1");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_XPOS], 
				     SPAWNCOL_XPOS);
  m_id_spawnList_Cols[SPAWNCOL_YPOS] = 
    spawnListColMenu->insertItem("Coord &2");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_YPOS], 
				     SPAWNCOL_YPOS);
  m_id_spawnList_Cols[SPAWNCOL_ZPOS] = 
    spawnListColMenu->insertItem("Coord &3");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_ZPOS], 
				     SPAWNCOL_ZPOS);
  m_id_spawnList_Cols[SPAWNCOL_ID] = 
    spawnListColMenu->insertItem("I&D");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_ID], 
				     SPAWNCOL_ID);
  m_id_spawnList_Cols[SPAWNCOL_DIST] = spawnListColMenu->insertItem("&Dist");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_DIST], 
				     SPAWNCOL_DIST);
  m_id_spawnList_Cols[SPAWNCOL_RACE] = spawnListColMenu->insertItem("&Race");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_RACE], 
				     SPAWNCOL_RACE);
  m_id_spawnList_Cols[SPAWNCOL_CLASS] = spawnListColMenu->insertItem("&Class");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_CLASS], 
				     SPAWNCOL_CLASS);
  m_id_spawnList_Cols[SPAWNCOL_INFO] = spawnListColMenu->insertItem("&Info");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_INFO], 
				     SPAWNCOL_INFO);
  m_id_spawnList_Cols[SPAWNCOL_SPAWNTIME] = 
    spawnListColMenu->insertItem("Spawn &Time");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[SPAWNCOL_SPAWNTIME], 
				     SPAWNCOL_SPAWNTIME);
  
  connect (spawnListColMenu, SIGNAL(activated(int)), 
	   this, SLOT(toggle_spawnListCol(int)));

  int x;
  QPopupMenu* filterMenu = new QPopupMenu;
  m_id_filterMenu = insertItem("Add &Filter", filterMenu);
  setItemEnabled(m_id_filterMenu, false);
  x = filterMenu->insertItem("&Hunt...");
  filterMenu->setItemParameter(x, HUNT_FILTER);
  x = filterMenu->insertItem("&Caution...");
  filterMenu->setItemParameter(x, CAUTION_FILTER);
  x = filterMenu->insertItem("&Danger...");
  filterMenu->setItemParameter(x, DANGER_FILTER);
  x = filterMenu->insertItem("&Locate...");
  filterMenu->setItemParameter(x, LOCATE_FILTER);
  x = filterMenu->insertItem("&Alert...");
  filterMenu->setItemParameter(x, ALERT_FILTER);
  x = filterMenu->insertItem("&Filtered...");
  filterMenu->setItemParameter(x, FILTERED_FILTER);
  x = filterMenu->insertItem("&Tracer...");
  filterMenu->setItemParameter(x, TRACER_FILTER);
  connect (filterMenu, SIGNAL(activated(int)), 
	   this, SLOT(add_filter(int)));

  insertSeparator(-1);

  x = insertItem("&Add Category...", this, SLOT(add_category(int)));
  m_id_edit_category = 
    insertItem("&Edit Category...", this, SLOT(edit_category(int)));
  m_id_delete_category = 
    insertItem("&Delete Category...", this, SLOT(delete_category(int)));
  insertItem("&Reload Categories", this, SLOT(reload_categories(int)));
  insertItem("Re&build Spawnlist", this, SLOT(rebuild_spawnlist(int)));
  insertSeparator(-1);
  insertItem("&Font...", this, SLOT(set_font(int)));
  insertItem("&Caption...", this, SLOT(set_caption(int)));

  connect(this, SIGNAL(aboutToShow()),
	  this, SLOT(init_Menu()));
}

SpawnListMenu::~SpawnListMenu()
{
}

void SpawnListMenu::init_Menu(void)
{
  // make sure the menu bar settings are correct
  for (int i = 0; i < SPAWNCOL_MAXCOLS; i++)
    setItemChecked(m_id_spawnList_Cols[i], 
		   m_spawnlist->columnWidth(i) != 0);
}

void SpawnListMenu::setCurrentCategory(const Category* cat)
{
  // set the current category
  m_currentCategory = cat;

  // update the menu item names
  if (cat != NULL)
  {
    changeItem(m_id_edit_category, 
	       "&Edit '" + cat->name() + "' Category...");
    setItemEnabled(m_id_edit_category, true);
    changeItem(m_id_delete_category, 
	       "&Delete '" + cat->name() + "' Category...");
    setItemEnabled(m_id_delete_category, true);
  }
  else
  {
    changeItem(m_id_edit_category, "&Edit Category...");
    setItemEnabled(m_id_edit_category, false);
    changeItem(m_id_delete_category, "&Delete Category...");
    setItemEnabled(m_id_delete_category, false);
  }
}

void SpawnListMenu::setCurrentItem(const Item* item)
{
  // set the current item
  m_currentItem = item;

  // enable/disable item depending on if there is one
  setItemEnabled(m_id_filterMenu, (item != NULL));

  if (item != NULL)
    changeItem(m_id_filterMenu,
	       "Add '" + item->name() + "' &Filter");
  else
    changeItem(m_id_filterMenu,
	       "Add &Filter");
}

void SpawnListMenu::toggle_spawnListCol(int id)
{
  int colnum;

  colnum = itemParameter(id);
  
  if (isItemChecked(id))
    m_spawnlist->setColumnVisible(colnum, false);
  else
    m_spawnlist->setColumnVisible(colnum, true);
}

void SpawnListMenu::add_filter(int id)
{
  if (m_currentItem == NULL)
    return;

  int filter = itemParameter(id);
  QString filterName = m_filterMgr->filterName(filter);
  QString filterString = m_currentItem->filterString();

  // get the user edited filter string, based on the items filterString
  bool ok = false;
  filterString = 
    QInputDialog::getText(filterName + " Filter",
			  "Enter the filter string:",
			  filterString, &ok, m_spawnlist);


  // if the user clicked ok, add the filter
  if (ok)
    m_filterMgr->addFilter(filter, filterString);
}

void SpawnListMenu::add_category(int id)
{
  // add a category to the category manager
  m_categoryMgr->addCategory(m_spawnlist);
}

void SpawnListMenu::edit_category(int id)
{
  // edit the current category
  m_categoryMgr->editCategories(m_currentCategory, m_spawnlist);
}

void SpawnListMenu::delete_category(int id)
{
  // confirm that the user wants to delete the category
  QMessageBox mb("Are you sure?",
		 "Are you sure you wish to delete category "
		 + m_currentCategory->name() + "?",
		 QMessageBox::NoIcon,
		 QMessageBox::Yes, 
		 QMessageBox::No | QMessageBox::Default | QMessageBox::Escape,
		 QMessageBox::NoButton,
		 m_spawnlist);
  
  // if user chose yes, then delete the category
  if (mb.exec() == QMessageBox::Yes)
    m_categoryMgr->RemCategory(m_currentCategory);
}

void SpawnListMenu::reload_categories(int id)
{
  // reload the categories
  m_categoryMgr->reloadCategories();
}


void SpawnListMenu::set_font(int id)
{
  QFont newFont;
  bool ok = false;
  SEQWindow* window = (SEQWindow*)m_spawnlist->parent();

  // get a new font
  newFont = QFontDialog::getFont(&ok, window->font(), 
				 this, "ShowEQ Spawn List Font");
    
    
    // if the user entered a font and clicked ok, set the windows font
    if (ok)
      window->setWindowFont(newFont);
}

void SpawnListMenu::set_caption(int id)
{
  bool ok = false;
  SEQWindow* window = (SEQWindow*)m_spawnlist->parent();

  QString caption = 
    QInputDialog::getText("ShowEQ Spawn List Window Caption",
			  "Enter caption for the Spawn List Window:",
			  QLineEdit::Normal, window->caption(),
			  &ok, this);
  
  // if the user entered a caption and clicked ok, set the windows caption
  if (ok)
    window->setCaption(caption);
}

void SpawnListMenu::rebuild_spawnlist(int id)
{
  // rebuild the spawn list
  m_spawnlist->rebuildSpawnList();
}


SpawnListWindow::SpawnListWindow(EQPlayer* player, 
				 SpawnShell* spawnShell,
				 CategoryMgr* categoryMgr,
				 QWidget* parent, const char* name)
  : SEQWindow("SpawnList", "ShowEQ - Spawns", parent, name)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setAutoAdd(true);
  
  m_spawnList = new CSpawnList(player, spawnShell, categoryMgr, this, name);
}

SpawnListWindow::~SpawnListWindow()
{
  delete m_spawnList;
}

void SpawnListWindow::savePrefs(void)
{
  // save SEQWindow prefs
  SEQWindow::savePrefs();

  // make the listview save it's prefs
  m_spawnList->savePrefs();
}
