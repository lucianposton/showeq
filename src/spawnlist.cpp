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

#include "spawnlist.h"
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
     setText(SPAWNCOL_RACE, item()->raceName());
     
     // Class
     setText(SPAWNCOL_CLASS, item()->className());
     
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
  : QListView(parent, name), m_menu(NULL)
{
   bDebug = FALSE;
   m_player = player;
   m_spawnShell = spawnShell;
   m_categoryMgr = categoryMgr;

   QString section = "SpawnList";

   setRootIsDecorated(true);
   setShowSortIndicator(true);
   QListView::setCaption(pSEQPrefs->getPrefString("Caption", section,
						  "ShowEQ - Spawn List"));

   restoreFont();

   addColumn ("Name");
   addColumn ("Lvl");
   addColumn ("Hp");
   addColumn ("MaxHP");
   if(showeq_params->retarded_coords) {
     addColumn ("N/S");
     addColumn ("E/W");
   } else {
     addColumn ("X");
     addColumn ("Y");
   }
   addColumn ("Z");
   addColumn ("ID");
   addColumn ("Dist");
   addColumn ("Race");
   addColumn ("Class");
   addColumn ("Info");
   addColumn ("SpawnTime");
   setAllColumnsShowFocus(TRUE);

   // Restore column order
   QString tStr = pSEQPrefs->getPrefString("ColumnOrder", section, "N/A");
   if (tStr != "N/A") {
      int i = 0;
      while (!tStr.isEmpty()) {
         int toIndex;
         if (tStr.find(':') != -1) {
            toIndex = tStr.left(tStr.find(':')).toInt();
            tStr = tStr.right(tStr.length() - tStr.find(':') - 1);
         } else {
            toIndex = tStr.toInt();
            tStr = "";
         }
         header()->moveSection(toIndex, i++);
      }
   }

   // SpawnList column sizes
   int x;
   if (pSEQPrefs->isPreference("NameWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("NameWidth", section, columnWidth(SPAWNCOL_NAME));
      setColumnWidthMode(SPAWNCOL_NAME, QListView::Manual);
      setColumnWidth(SPAWNCOL_NAME, x);
   }
   if (pSEQPrefs->isPreference("LevelWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("LevelWidth", section, columnWidth(SPAWNCOL_LEVEL));
      setColumnWidth(SPAWNCOL_LEVEL, x);
      setColumnWidthMode(SPAWNCOL_LEVEL, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("HPWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("HPWidth", section, columnWidth(SPAWNCOL_HP));
      setColumnWidth(SPAWNCOL_HP, x);
      setColumnWidthMode(SPAWNCOL_HP, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("MaxHPWidth", section)) {
      x = pSEQPrefs->getPrefInt("MaxHPWidth", section, columnWidth(SPAWNCOL_MAXHP));
      setColumnWidth(SPAWNCOL_MAXHP, x);
      setColumnWidthMode(SPAWNCOL_MAXHP, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("Coord1Width", section)) 
   {
      x = pSEQPrefs->getPrefInt("Coord1Width", section, columnWidth(SPAWNCOL_XPOS));
      setColumnWidth(SPAWNCOL_XPOS, x);
      setColumnWidthMode(SPAWNCOL_XPOS, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("Coord2Width", section)) 
   {
      x = pSEQPrefs->getPrefInt("Coord2Width", section, columnWidth(SPAWNCOL_YPOS));
      setColumnWidth(SPAWNCOL_YPOS, x);
      setColumnWidthMode(SPAWNCOL_YPOS, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("Coord3Width", section)) 
   {
      x = pSEQPrefs->getPrefInt("Coord3Width", section, columnWidth(SPAWNCOL_ZPOS));
      setColumnWidth(SPAWNCOL_ZPOS, x);
      setColumnWidthMode(SPAWNCOL_ZPOS, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("IDWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("IDWidth", section, columnWidth(SPAWNCOL_ID));
      setColumnWidth(SPAWNCOL_ID, x);
      setColumnWidthMode(SPAWNCOL_ID, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("DistWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("DistWidth", section, columnWidth(SPAWNCOL_DIST));
      setColumnWidth(SPAWNCOL_DIST, x);
      setColumnWidthMode(SPAWNCOL_DIST, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("RaceWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("RaceWidth", section, columnWidth(SPAWNCOL_RACE));
      setColumnWidth(SPAWNCOL_RACE, x);
      setColumnWidthMode(SPAWNCOL_RACE, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("ClassWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("ClassWidth", section, columnWidth(SPAWNCOL_CLASS));
      setColumnWidth(SPAWNCOL_CLASS, x);
      setColumnWidthMode(SPAWNCOL_CLASS, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("InfoWidth", section)) {
      x = pSEQPrefs->getPrefInt("InfoWidth", section, columnWidth(SPAWNCOL_INFO));
      setColumnWidth(SPAWNCOL_INFO, x);
      setColumnWidthMode(SPAWNCOL_INFO, QListView::Manual);
   }
   if (pSEQPrefs->isPreference("SpawnTimeWidth", section)) 
   {
      x = pSEQPrefs->getPrefInt("SpawnTimeWidth", section, columnWidth(SPAWNCOL_SPAWNTIME));
      setColumnWidth(SPAWNCOL_SPAWNTIME, x);
      setColumnWidthMode(SPAWNCOL_SPAWNTIME, QListView::Manual);
   }

   // connect a QListView signal to ourselves
   connect(this, SIGNAL(selectionChanged(QListViewItem*)),
	   this, SLOT(selChanged(QListViewItem*)));

   connect (this, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
            this, SLOT(myMousePressEvent(int, QListViewItem*, const QPoint&, int)));

   connect (this, SIGNAL(doubleClicked(QListViewItem*)),
            this, SLOT(myMouseDoubleClickEvent(QListViewItem*)));

#if 0 //FEETMP it goes bye bye
   connect (this, SIGNAL(rightButtonPressed( QListViewItem *,
					     const QPoint&, int )),
	    this, SLOT(rightBtnPressed(QListViewItem*,
				       const QPoint&, int)) );
   connect (this, SIGNAL(rightButtonClicked( QListViewItem *,
					     const QPoint&, int )),
	    this, SLOT(rightBtnReleased(QListViewItem*,
					const QPoint&, int)) );
#endif

   // connect CSpawnList slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(addItem(const Item *)),
	   this, SLOT(addItem(const Item *)));
   connect(m_spawnShell, SIGNAL(delItem(const Item *)),
	   this, SLOT(delItem(const Item *)));
   connect(m_spawnShell, SIGNAL(changeItem(const Item *, uint32_t)),
	   this, SLOT(changeItem(const Item *, uint32_t)));
   connect(m_spawnShell, SIGNAL(killSpawn(const Item *)),
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

void CSpawnList::savePrefs()
{
  if (!isVisible())
    return;

  QString section = "SpawnList";
  if (pSEQPrefs->getPrefBool("SaveWidth", section, 1)) 
  {
    pSEQPrefs->setPrefInt("NameWidth", section,
      columnWidth(SPAWNCOL_NAME));
    pSEQPrefs->setPrefInt("LevelWidth", section,
      columnWidth(SPAWNCOL_LEVEL));
    pSEQPrefs->setPrefInt("HPWidth", section,
      columnWidth(SPAWNCOL_HP));
    pSEQPrefs->setPrefInt("MaxHPWidth", section,
      columnWidth(SPAWNCOL_MAXHP));
    pSEQPrefs->setPrefInt("Coord1Width", section,
      columnWidth(SPAWNCOL_XPOS));
    pSEQPrefs->setPrefInt("Coord2Width", section,
      columnWidth(SPAWNCOL_YPOS));
    pSEQPrefs->setPrefInt("Coord3Width", section,
      columnWidth(SPAWNCOL_ZPOS));
    pSEQPrefs->setPrefInt("IDWidth", section,
      columnWidth(SPAWNCOL_ID));
    pSEQPrefs->setPrefInt("DistWidth", section,
      columnWidth(SPAWNCOL_DIST));
    pSEQPrefs->setPrefInt("RaceWidth", section,
      columnWidth(SPAWNCOL_RACE));
    pSEQPrefs->setPrefInt("ClassWidth", section,
      columnWidth(SPAWNCOL_CLASS));
    pSEQPrefs->setPrefInt("InfoWidth", section,
      columnWidth(SPAWNCOL_INFO));
    pSEQPrefs->setPrefInt("SpawnTimeWidth", section,
      columnWidth(SPAWNCOL_SPAWNTIME));
    char tempStr[256], tempStr2[256];
    if (header()->count() > 0)
      sprintf(tempStr, "%d", header()->mapToSection(0));
    for(int i=1; i<header()->count(); i++) {
      sprintf(tempStr2, ":%d", header()->mapToSection(i));
      strcat(tempStr, tempStr2);
    }
    pSEQPrefs->setPrefString("ColumnOrder", section, tempStr);
  }
}

void CSpawnList::setCaption(const QString& text)
{
  // set the caption
  QListView::setCaption(text);

  // set the preference
  pSEQPrefs->setPrefString("Caption", "SpawnList", caption());
}

void CSpawnList::setWindowFont(const QFont& font)
{
  // set the font preference
  pSEQPrefs->setPrefFont("Font", "SpawnList", font);

  // restore the font to the preference
  restoreFont();
}

void CSpawnList::restoreFont()
{
  QString section = "SpawnList";
  // set the applications default font
  if (pSEQPrefs->isPreference("Font", section))
  {
    // use the font specified in the preferences
    QFont font = pSEQPrefs->getPrefFont("Font", section);
    setFont( font);
  }
}

void CSpawnList::setColumnVisible(int id, bool visible)
{
  if (visible)
  {
    int width;

    QString section = "SpawnList";
    switch(id)
    {
    case SPAWNCOL_NAME:
      width = pSEQPrefs->getPrefInt("NameWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_LEVEL:
      width = pSEQPrefs->getPrefInt("LevelWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_HP:
      width = pSEQPrefs->getPrefInt("HPWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_MAXHP:
      width = pSEQPrefs->getPrefInt("MaxHPWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_XPOS:
      width = pSEQPrefs->getPrefInt("Coord1Width", section, columnWidth(id));
      break;
    case SPAWNCOL_YPOS:
      width = pSEQPrefs->getPrefInt("Coord2Width", section, columnWidth(id));
      break;
    case SPAWNCOL_ZPOS:
      width = pSEQPrefs->getPrefInt("Coord3Width", section, columnWidth(id));
      break;
    case SPAWNCOL_ID:
      width = pSEQPrefs->getPrefInt("IDWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_DIST:
      width = pSEQPrefs->getPrefInt("DistWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_RACE:
      width = pSEQPrefs->getPrefInt("RaceWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_CLASS:
      width = pSEQPrefs->getPrefInt("ClassWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_INFO:
      width = pSEQPrefs->getPrefInt("InfoWidth", section, columnWidth(id));
      break;
    case SPAWNCOL_SPAWNTIME:
      width = pSEQPrefs->getPrefInt("SpawnTimeWidth", section, columnWidth(id));
      break;
    default:
      width = 0;
    }

    // if now real width found, just guess at a minimum width
    if (width == 0)
      width = 35;

    // set the column width
    setColumnWidth(id, width);
  }
  else
  {
    // set the column width to 0
    setColumnWidth(id, 0);
  }

  // set the column width mode
  setColumnWidthMode(id, QListView::Manual);

  // trigger an update, otherwise things look messy
  triggerUpdate();
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
	j->update();

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
	k->update();
	
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
#if 0
      printf("Checking ='%s'= in category '%s' \n `-- filter='%s' filterout='%s' flags=%d\n", 
	     (const char*)filterString(item,flags), (const char*)cat->name(),
	     (const char*)cat->filter(), (const char*)cat->filterout());
#endif
      // skip filtered spawns, if this is a filtered filter category
      if ((item->filterFlags() & FILTER_FLAG_FILTERED) &&
	  cat->isFilteredFilter())
      {
#if 0
	printf("Skipping\n");
#endif
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
	j->update();

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
     j->update();
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
    setSelectedQueit(j, true);
    
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
  setSelectedQueit(i, true);

  // if configured to do so, make sure it's visible
  if (showeq_params->keep_selected_visible)
    ensureItemVisible(i);
}

void CSpawnList::setSelectedQueit(QListViewItem* item, bool selected)
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
	cat->isFilteredFilter())
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
      litem->update();
      
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
	  cat->isFilteredFilter())
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
	litem->update();

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
	    cat->isFilteredFilter())
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
	  litem->update();
	  
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
	      cat->isFilteredFilter())
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
	    litem->update();
	    
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
	litem->update();
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

void CSpawnList::myMousePressEvent(int button, QListViewItem* litem,
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
     //press right button to display popup menu or edit category
     const Category* cat = getCategory((SpawnListItem*)litem);

     if (cat)
     {
       // edit the category
       m_categoryMgr->editCategories(cat, this);
       return;
     }
     //else
//	  menu()->popup((point));
  }
}

void CSpawnList::myMouseDoubleClickEvent(QListViewItem* litem)
{
   //print spawn info to console
  if (litem == NULL)
    return;

  const Item* item = ((SpawnListItem*)litem)->item();
  if (item != NULL)
    printf("%s\n",(const char*)item->filterString());
}

  
#if 0 //FEETMP  on its way to the door 
//
// rightButtonPressed
//
// if you right click on a spawn in the list dump its contents
// and mark the column you selected as the current column for filtering
//
void CSpawnList::rightBtnPressed(QListViewItem* litem, 
				 const QPoint &point, 
				 int col)
{
  if (litem == NULL)
    return;

  const Item* item = ((SpawnListItem*)litem)->item();
  if (item != NULL)
  {
    printf("%s\n",(const char*)item->filterString());

    emit keepUpdated(true);
  }
  else
  {
    const Category* cat = getCategory((SpawnListItem*)litem);

    if (!cat)
      return;

    // edit the category
    m_categoryMgr->editCategories(cat, this);
  }
}

void CSpawnList::rightBtnReleased(QListViewItem *item,
				  const QPoint &point, 
				  int col)
{
  emit keepUpdated(false);
}
#endif

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
  printf("CSpawnList::menu()\n");
  if (m_menu != NULL)
      return m_menu;

      m_menu = new SpawnListMenu(this, this, "spawnlist menu");
  printf("CSpawnList::menu() - m_menu returned\n");

      return m_menu;
}

SpawnListMenu::SpawnListMenu(CSpawnList* spawnlist,
		             QWidget* parent, const char* name)
                             : m_spawnlist(spawnlist)
{


}

SpawnListMenu::~SpawnListMenu()
{
}
