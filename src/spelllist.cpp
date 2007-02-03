/*
 * spelllist.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 *  Copyright 2001-2007 by the respective ShowEQ Developers
 */
 
/*
 * Orig Author - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date - 9/7/2001
 */

#include <qpainter.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PopupMenu>

#include "spelllist.h"
#include "main.h"

SpellListItem::SpellListItem(Q3ListViewItem *parent) : Q3ListViewItem(parent)
{
   m_textColor = Qt::black;
   m_item = NULL;
}

SpellListItem::SpellListItem(Q3ListView *parent) : Q3ListViewItem(parent)
{
   m_textColor = Qt::black;
   m_item = NULL;
}


//Added in by Worried to make color change by time remaining work
// paintCell 
//
// overridden from base class in order to change color and style attributes
//
void SpellListItem::paintCell( QPainter *p, const QColorGroup &cg,
                               int column, int width, int alignment )
{
  QColorGroup newCg( cg );
  
  newCg.setColor( QColorGroup::Text, m_textColor);
  
  QFont font = this->listView()->font();
  p->setFont(font);
  
  Q3ListViewItem::paintCell( p, newCg, column, width, alignment );
}

const QColor SpellListItem::textColor()
{
   return m_textColor;
}

void SpellListItem::setTextColor(const QColor& color)
{
   m_textColor = color;
}

void SpellListItem::update()
{
   //color change by Worried
   //change spell colors according to time remaining

  if (m_item->duration() > 120)
    this->setTextColor(Qt::black);
  else if (m_item->duration() <= 120 and m_item->duration() > 60)
    this->setTextColor(QColor(128,54,193));
  else if (m_item->duration() <= 60 and m_item->duration() > 30)
    this->setTextColor(Qt::blue);
  else if (m_item->duration() <= 30 and m_item->duration() > 12)
    this->setTextColor(Qt::magenta);
  else if (m_item->duration() <= 12)
    this->setTextColor(Qt::red);

   setText(SPELLCOL_SPELLID, QString("%1").arg(m_item->spellId()));
   setText(SPELLCOL_SPELLNAME, m_item->spellName());
   setText(SPELLCOL_CASTERID, QString("%1").arg(m_item->casterId()));
   setText(SPELLCOL_CASTERNAME, m_item->casterName());
   setText(SPELLCOL_TARGETID, QString("%1").arg(m_item->targetId()));
   setText(SPELLCOL_TARGETNAME, m_item->targetName());
   setText(SPELLCOL_CASTTIME, m_item->castTimeStr());
   setText(SPELLCOL_DURATION, m_item->durationStr());
}

void SpellListItem::setSpellItem(const SpellItem *item)
{
   m_item = item;
}

const SpellItem* SpellListItem::item() const
{
   return m_item;
}

const QString& SpellListItem::category() const
{
   return m_category;
}

void SpellListItem::setCategory(QString& cat)
{
   m_category = cat;
   //CJD TODO - fill in
}

// -------------------------------------------------------------------

SpellList::SpellList(SpellShell* sshell, QWidget *parent, const char *name)
  : SEQListView("SpellList", parent, name),
    m_spellShell(sshell),
    m_menu(0)
{
   //addColumn... spell icon
   addColumn("Spell", "SpellName");
   addColumn("Spell ID", "SpellID");
   addColumn("Caster ID", "CasterID");
   addColumn("Caster", "CasterName");
   addColumn("Target ID", "TargetID");
   addColumn("Target", "TargetName");
   addColumn("Casted", "CastTime");
   addColumn("Remain", "RemainTime");
   setSorting(SPELLCOL_DURATION);

   restoreColumns();

   connect (this, SIGNAL(doubleClicked(Q3ListViewItem*)),
	    this, SLOT(mouseDoubleClicked(Q3ListViewItem*)));
   connect(this, SIGNAL(rightButtonClicked(Q3ListViewItem*, const QPoint&, int)),
	   this, SLOT(rightButtonClicked(Q3ListViewItem*, const QPoint&, int)));
}

Q3PopupMenu* SpellList::menu()
{
  // if the menu already exists, return it
  if (m_menu)
    return m_menu;
  
  m_menu = new Q3PopupMenu(this);
  m_menu->setCheckable(true);
  
  mid_spellName = m_menu->insertItem("Spell Name");
  mid_spellId = m_menu->insertItem("Spell ID");
  mid_casterId = m_menu->insertItem("Caster ID");
  mid_casterName = m_menu->insertItem("Caster Name");
  mid_targetId = m_menu->insertItem("Target ID");
  mid_targetName = m_menu->insertItem("Target Name");
  mid_casttime = m_menu->insertItem("Cast Time");
  mid_duration = m_menu->insertItem("Remaining Time");

  connect(m_menu, SIGNAL(activated(int)), this, SLOT(activated(int)));
  connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(init_menu()));
  
  return m_menu;
}

void SpellList::init_menu(void)
{ 
  m_menu->setItemChecked(mid_spellName, columnWidth(SPELLCOL_SPELLNAME) != 0);
  m_menu->setItemChecked(mid_spellId, columnWidth(SPELLCOL_SPELLID) != 0);
  m_menu->setItemChecked(mid_casterId, columnWidth(SPELLCOL_CASTERID) != 0);
  m_menu->setItemChecked(mid_casterName, columnWidth(SPELLCOL_CASTERNAME) != 0);
  m_menu->setItemChecked(mid_targetId, columnWidth(SPELLCOL_TARGETID) != 0);
  m_menu->setItemChecked(mid_targetName, columnWidth(SPELLCOL_TARGETNAME) != 0);
  m_menu->setItemChecked(mid_casttime, columnWidth(SPELLCOL_CASTTIME) != 0);
  m_menu->setItemChecked(mid_duration, columnWidth(SPELLCOL_DURATION) != 0);
}

void SpellList::selectSpell(const SpellItem *item)
{
   if (item) {
      SpellListItem *i = Find(item);
      selectAndOpen(i);
   }
}

SpellListItem* SpellList::InsertSpell(const SpellItem *item)
{
   if (!item)
      return NULL;

   Q3ValueList<SpellListItem *>::Iterator it;
   for(it = m_spellList.begin(); it != m_spellList.end(); it++) {
      if ((*it)->item() == item)
         break;
   }
   if (it != m_spellList.end()) {
      int sid = (*it)->text(SPELLCOL_SPELLID).toInt();
      int cid = (*it)->text(SPELLCOL_CASTERID).toInt();
      int tid = (*it)->text(SPELLCOL_TARGETID).toInt();
      if ((sid == (*it)->item()->spellId()) &&
          (cid == (*it)->item()->casterId()) &&
          (tid == (*it)->item()->targetId())) {
         (*it)->update();
         return (*it);
      } else {
         DeleteItem((*it)->item());
      }
   }

   // now insert
   // CJD TODO - checks for putting in appropriate category
   SpellListItem *j = new SpellListItem(this);
   m_spellList.append(j);
   j->setSpellItem(item);

   //j->setTextColor(pickColorSpawn(item));
   j->update();

   return j;
}

void SpellList::DeleteItem(const SpellItem *item)
{
   if (item) {
      SpellListItem *i = Find(item);
      if (i) {
         m_spellList.remove(i);
         delete i;
      }
   }
}

//SpellItem* SpellList::AddCategory(QString& name, QColor color)
//{
//}

//void SpellList::RemCategory(SpellListItem *)
//{
//}

//void SpellList::clearCategory()
//{
//}

QColor SpellList::pickSpellColor(const SpellItem *item, QColor def) const
{
   return Qt::black;
}

SpellListItem* SpellList::Find(const SpellItem *item)
{
   if (item) {
      Q3ValueList<SpellListItem*>::Iterator it;
      for(it = m_spellList.begin(); it != m_spellList.end(); ++it) {
         if ((*it)->item() == item)
            return (*it);
      }
   }
   return NULL;
}

void SpellList::addSpell(const SpellItem *item)
{
   if (item)
      InsertSpell(item);
}

void SpellList::delSpell(const SpellItem *item)
{
   if (item)
      DeleteItem(item);
}

void SpellList::changeSpell(const SpellItem *item)
{
   if (item) {
      SpellListItem *i = Find(item);
      if (!i)
         return;
      int sid = i->text(SPELLCOL_SPELLID).toInt();
      int cid = i->text(SPELLCOL_CASTERID).toInt();
      int tid = i->text(SPELLCOL_TARGETID).toInt();
      if ((sid == item->spellId()) &&
          (cid == item->casterId()) &&
          (tid == item->targetId()))
         i->update();
      else {
         DeleteItem(item);
         addSpell(item);
      }
   }
}

void SpellList::clear()
{
   Q3ListView::clear();
   m_spellList.clear();
   // rebuild categories...
}

void SpellList::selectAndOpen(SpellListItem *item)
{
   Q3ListViewItem *i = item;
   while(i) {
      item->setOpen(true);
      item = (SpellListItem *)item->parent();
   }
   setSelected(i, 1);
   // CJD TODO - use keep selected setting?
}

//void selfStartSpellCast(struct castStruct *);
//void otherStartSpellCast(struct beginCastStruct *);
//void selfFinishSpellCast(struct beginCastStruct *);
//void interruptSpellCast(struct interruptCastStruct *);
//void spellMessage(QString&);

void SpellList::mouseDoubleClicked(Q3ListViewItem *item) 
{
  if (!item)
    return;

  SpellListItem *i = (SpellListItem *)item;
  const SpellItem *j = i->item(); 
  if (j)
    m_spellShell->deleteSpell(j);
}

void SpellList::rightButtonClicked(Q3ListViewItem *item, const QPoint& pos,
      int col)
{
  Q3PopupMenu* slMenu = menu();

  if (slMenu)
    slMenu->popup(pos);
}

void SpellList::activated(int mid)
{
   int col = 0;
   int id = 0;

   if (mid == mid_spellName) {
      id = mid_spellName;
      col = SPELLCOL_SPELLNAME;
   } else if (mid == mid_spellId) {
      id = mid_spellId;
      col = SPELLCOL_SPELLID;
   } else if (mid == mid_casterId) {
      id = mid_casterId;
      col = SPELLCOL_CASTERID;
   } else if (mid == mid_casterName) {
      id = mid_casterName;
      col = SPELLCOL_CASTERNAME;
   } else if (mid == mid_targetId) {
      id = mid_targetId;
      col = SPELLCOL_TARGETID;
   } else if (mid == mid_targetName) {
      id = mid_targetName;
      col = SPELLCOL_TARGETNAME;
   } else if (mid == mid_casttime) {
      id = mid_casttime;
      col = SPELLCOL_CASTTIME;
   } else if (mid == mid_duration) {
      id = mid_duration;
      col = SPELLCOL_DURATION;
   }

   if (id) 
   {
     setColumnVisible(col, !columnVisible(col));
     m_menu->setItemChecked(id, columnVisible(col));
   }
}

SpellListWindow::SpellListWindow(SpellShell* sshell, 
				 QWidget* parent, const char* name)
  : SEQWindow("SpellList", "ShowEQ - Spell List", parent, name)
{
  //QVBoxLayout* layout = new QVBoxLayout(this);
  //layout->setAutoAdd(true);
  
  m_spellList = new SpellList(sshell, this, name);
  setWidget(m_spellList);
}

SpellListWindow::~SpellListWindow()
{
  delete m_spellList;
}

Q3PopupMenu* SpellListWindow::menu()
{
  return m_spellList->menu();
}

void SpellListWindow::savePrefs(void)
{
  // save SEQWindow prefs
  SEQWindow::savePrefs();

  // make the listview save it's prefs
  m_spellList->savePrefs();
}

#ifndef QMAKEBUILD
#include "spelllist.moc"
#endif
