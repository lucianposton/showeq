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

#include <qfontdialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qfont.h>
#include <qpainter.h>

#include "seqwindow.h"
#include "seqlistview.h"
#include "spawnlistcommon.h"
#include "category.h"
#include "filtermgr.h"
#include "spawnshell.h"
#include "main.h"
#include "player.h"

SpawnListItem::SpawnListItem(QListViewItem *parent) : QListViewItem(parent)
{
  m_textColor = Qt::black;
  m_item = NULL;
  m_npc = 0;
}

SpawnListItem::SpawnListItem(QListView *parent) : QListViewItem(parent)
{
  m_textColor = Qt::black; 
  m_item = NULL;
  m_npc = 0;
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

  uint32_t filterFlags = 0;

  if (m_item != NULL)
    filterFlags = m_item->filterFlags();

  if (!(filterFlags & (FILTER_FLAG_FILTERED |
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
    if (filterFlags & FILTER_FLAG_FILTERED)
      newCg.setColor( QColorGroup::Text, Qt::gray);
    
    if (filterFlags & FILTER_FLAG_ALERT)
      font.setBold(true);
    else
      font.setBold(false);
    
    if (filterFlags & FILTER_FLAG_LOCATE)
      font.setItalic(true);
    else
      font.setItalic(false);
    
    if ((filterFlags & FILTER_FLAG_CAUTION) || 
	(filterFlags & FILTER_FLAG_DANGER))
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
    
  if ((column < tSpawnColLevel) || (column > tSpawnColDist))
    return text(column);

  double num = text(column).toDouble();
  QString textNum;
  textNum.sprintf("%08.2f", num);
  return textNum;
}

void SpawnListItem::update(Player* player, uint32_t changeType)
{
//   printf ("SpawnListItem::update()\n");
   QString buff;
   const Spawn* spawn = NULL;

   if ((item()->type() == tSpawn) || (item()->type() == tPlayer))
     spawn = (const Spawn*)item();

   if (changeType & tSpawnChangedName)
   {
     // Name
     if (!showeq_params->showRealName)
       buff = item()->transformedName();
     else
       buff = item()->name();

     if ((spawn != NULL) && !spawn->lastName().isEmpty())
       buff.sprintf("%s (%s)", 
		    (const char*)buff, (const char*)spawn->lastName());

     setText(tSpawnColName, buff);
   }

   // only spawns contain level info
   if (spawn != NULL)
   {
     if (changeType & tSpawnChangedLevel)
     {
       // Level
       buff.sprintf("%2d", spawn->level());
       setText(tSpawnColLevel, buff);
     }
     
     if (changeType & tSpawnChangedHP)
     {
       // Hitpoints
       buff.sprintf("%5d", spawn->HP());
       setText(tSpawnColHP, buff);
       
       // Maximum Hitpoints
       buff.sprintf("%5d", spawn->maxHP());
       setText(tSpawnColMaxHP, buff);
     }

     if (changeType == tSpawnChangedALL)
     {
       setText(tSpawnColDeity, spawn->deityName());
       setText(tSpawnColBodyType, spawn->typeString());
     }
   }
   else if (changeType == tSpawnChangedALL)
   {
     buff = "0";
     setText(tSpawnColLevel, buff);
     setText(tSpawnColHP, buff);
     setText(tSpawnColMaxHP, buff);
   }

   if (changeType & tSpawnChangedPosition)
   {
     // X position
     buff.sprintf("%5d", showeq_params->retarded_coords ? 
		  (int)item()->y() : (int)item()->x());
     setText(tSpawnColXPos, buff);
     
     // Y position
     buff.sprintf("%5d", showeq_params->retarded_coords ? 
		  (int)item()->x() : (int)item()->y());
     setText(tSpawnColYPos, buff);
     
     // Z position
     buff.sprintf("%5.1f", item()->displayZPos());
     setText(tSpawnColZPos, buff);

     // Distance
     if (!showeq_params->fast_machine)
       buff.sprintf("%5d", player->calcDist2DInt(*item()));
     else
       buff.sprintf("%5.1f", player->calcDist(*item()));
     setText(tSpawnColDist, buff);
   }

   if (changeType == tSpawnChangedALL)
   {
     // Id
     buff.sprintf("%5d", item()->id());
     setText(tSpawnColID, buff);
     
     // Race
     setText(tSpawnColRace, item()->raceString());
     
     // Class
     setText(tSpawnColClass, item()->classString());
     
     // Spawntime
     setText(tSpawnColSpawnTime, m_item->spawnTimeStr());

     // CJD TODO - Deity, PVP teams
   }

   if (changeType & tSpawnChangedWearing)
   {
     // Info
     setText(tSpawnColInfo, item()->info());
   }

   m_npc = item()->NPC();
}

void SpawnListItem::updateTitle(const QString& name)
{
  // update childcount in header
  QString temp;
  temp.sprintf("%s (%d)",
	       (const char*)name, childCount());
  setText(tSpawnColName, temp);
} // end if spawn should be in this category

void SpawnListItem::setShellItem(const Item *item)
{
   m_item = item;
   if (item)
      m_npc = item->NPC();
}

//----------------------------------------------------------------------
//
// pickTextColor
// 
// insert color schemes here
//
void SpawnListItem::pickTextColor(const Item* item, 
				  Player* player, 
				  QColor def)
{
  if (item == NULL)
  {
    m_textColor = def;
    return;
  }
  
  const Spawn* spawn = NULL;
  if ((item->type() == tSpawn) || (item->type() == tPlayer))
    spawn = (const Spawn*)item;

  if (spawn == NULL)
  {
    m_textColor = def;
    return;
  }
  
  switch (spawn->typeflag())
  {
  case 65:
    m_textColor = Qt::magenta;
    return;
  case 66:
  case 67:
    m_textColor = Qt::darkMagenta;
    return;
  }

  // color by pvp team
  if (showeq_params->pvp) // if pvp
  {
    switch(spawn->raceTeam()) 
    {
    case RTEAM_HUMAN:
      m_textColor = Qt::blue;
      return;
    case RTEAM_ELF:
      m_textColor = QColor(196,206,12);
      return;
    case RTEAM_DARK:
      m_textColor = QColor(206,151,33);
      return;
    case RTEAM_SHORT:
      m_textColor = Qt::magenta;
      return;
    }
  } 
  else if (showeq_params->deitypvp) // if deitypvp
  {
    switch(spawn->deityTeam()) 
    {
    case DTEAM_GOOD:
      m_textColor = Qt::blue;
      return;
    case DTEAM_NEUTRAL:
      m_textColor = QColor(196,206,12);
      return;
    case DTEAM_EVIL:
      m_textColor = Qt::magenta;
      return;
    }
  }

  // color by consider difficulty
  m_textColor = player->pickConColor(spawn->level());
  if (m_textColor == Qt::white)
    m_textColor = Qt::black;
  if (m_textColor == Qt::yellow)
    m_textColor = QColor(206,151,33);
} // end pickTextColor

SpawnListMenu::SpawnListMenu(SEQListView* spawnlist,
			     SEQWindow* spawnlistWindow,
			     FilterMgr* filterMgr,
			     CategoryMgr* categoryMgr,
		             QWidget* parent, const char* name)
  : m_spawnlist(spawnlist),
    m_spawnlistWindow(spawnlistWindow),
    m_filterMgr(filterMgr),
    m_categoryMgr(categoryMgr)
{
  // Show Columns
  QPopupMenu* spawnListColMenu = new QPopupMenu;
  insertItem( "Show &Column", spawnListColMenu);
  spawnListColMenu->setCheckable(true);
  m_id_spawnList_Cols[tSpawnColName] = 
    spawnListColMenu->insertItem("&Name");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColName], 
				     tSpawnColName);
  m_id_spawnList_Cols[tSpawnColLevel] = spawnListColMenu->insertItem("&Level");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColLevel], 
				     tSpawnColLevel);
  m_id_spawnList_Cols[tSpawnColHP] = spawnListColMenu->insertItem("&HP");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColHP], 

				     tSpawnColHP);
  m_id_spawnList_Cols[tSpawnColMaxHP] = 
    spawnListColMenu->insertItem("&Max HP");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColMaxHP], 
				     tSpawnColMaxHP);
  m_id_spawnList_Cols[tSpawnColXPos] = 
    spawnListColMenu->insertItem("Coord &1");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColXPos], 
				     tSpawnColXPos);
  m_id_spawnList_Cols[tSpawnColYPos] = 
    spawnListColMenu->insertItem("Coord &2");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColYPos], 
				     tSpawnColYPos);
  m_id_spawnList_Cols[tSpawnColZPos] = 
    spawnListColMenu->insertItem("Coord &3");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColZPos], 
				     tSpawnColZPos);
  m_id_spawnList_Cols[tSpawnColID] = 
    spawnListColMenu->insertItem("I&D");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColID], 
				     tSpawnColID);
  m_id_spawnList_Cols[tSpawnColDist] = spawnListColMenu->insertItem("&Dist");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColDist], 
				     tSpawnColDist);
  m_id_spawnList_Cols[tSpawnColRace] = spawnListColMenu->insertItem("&Race");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColRace], 
				     tSpawnColRace);
  m_id_spawnList_Cols[tSpawnColClass] = spawnListColMenu->insertItem("&Class");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColClass], 
				     tSpawnColClass);
  m_id_spawnList_Cols[tSpawnColInfo] = spawnListColMenu->insertItem("&Info");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColInfo], 
				     tSpawnColInfo);
  m_id_spawnList_Cols[tSpawnColSpawnTime] = 
    spawnListColMenu->insertItem("Spawn &Time");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColSpawnTime], 
				     tSpawnColSpawnTime);
  m_id_spawnList_Cols[tSpawnColDeity] = spawnListColMenu->insertItem("&Deity");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColDeity], 
				     tSpawnColDeity);
  m_id_spawnList_Cols[tSpawnColBodyType] = spawnListColMenu->insertItem("&Body Type");
  spawnListColMenu->setItemParameter(m_id_spawnList_Cols[tSpawnColBodyType], 
				     tSpawnColBodyType);
  
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
  for (int i = 0; i < tSpawnColMaxCols; i++)
    setItemChecked(m_id_spawnList_Cols[i], 
		   	m_spawnlist->columnVisible(i));
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
			  QLineEdit::Normal,
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
    m_categoryMgr->remCategory(m_currentCategory);
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
  // get a new font
  newFont = QFontDialog::getFont(&ok, m_spawnlistWindow->font(),
				 this, "ShowEQ Spawn List Font");
    
    
    // if the user entered a font and clicked ok, set the windows font
    if (ok)
      m_spawnlistWindow->setWindowFont(newFont);
}

void SpawnListMenu::set_caption(int id)
{
  bool ok = false;

  QString caption = 
    QInputDialog::getText("ShowEQ Spawn List Window Caption",
			  "Enter caption for the Spawn List Window:",
			  QLineEdit::Normal, m_spawnlistWindow->caption(),
			  &ok, this);
  
  // if the user entered a caption and clicked ok, set the windows caption
  if (ok)
    m_spawnlistWindow->setCaption(caption);
}
