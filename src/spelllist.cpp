/*
 * spelllist.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */
 
/*
 * Orig Author - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date - 9/7/2001
 */

#include <qpainter.h>
#include <qlayout.h>

#include "spelllist.h"
#include "main.h"

SpellListItem::SpellListItem(QListViewItem *parent) : QListViewItem(parent)
{
   m_textColor = Qt::black;
   m_item = NULL;
}

SpellListItem::SpellListItem(QListView *parent) : QListViewItem(parent)
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
  
  QListViewItem::paintCell( p, newCg, column, width, alignment );
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

void SpellListItem::setSpellItem(SpellItem *item)
{
   m_item = item;
}

SpellItem* SpellListItem::item()
{
   return m_item;
}

QString& SpellListItem::category()
{
   return m_category;
}

void SpellListItem::setCategory(QString& cat)
{
   m_category = cat;
   //CJD TODO - fill in
}

// -------------------------------------------------------------------

SpellList::SpellList(QWidget *parent, const char *name)
   : SEQListView("SpellList", parent, name)
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

   connect(this, SIGNAL(mouseButtonClicked(int, QListViewItem*, const QPoint&, int)),
         this, SLOT(mouseButtonClicked(int, QListViewItem*, const QPoint&, int)));
   connect(this, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
         this, SLOT(rightButtonClicked(QListViewItem*, const QPoint&, int)));
}

void SpellList::selectSpell(SpellItem *item)
{
   if (item) {
      SpellListItem *i = Find(item);
      selectAndOpen(i);
   }
}

SpellListItem* SpellList::InsertSpell(SpellItem *item)
{
   if (!item)
      return NULL;

   QValueList<SpellListItem *>::Iterator it;
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

void SpellList::DeleteItem(SpellItem *item)
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

QColor SpellList::pickSpellColor(SpellItem *item, QColor def)
{
   return Qt::black;
}

SpellListItem* SpellList::Find(SpellItem *item)
{
   if (item) {
      QValueList<SpellListItem*>::Iterator it;
      for(it = m_spellList.begin(); it != m_spellList.end(); ++it) {
         if ((*it)->item() == item)
            return (*it);
      }
   }
   return NULL;
}

void SpellList::addSpell(SpellItem *item)
{
   if (item)
      InsertSpell(item);
}

void SpellList::delSpell(SpellItem *item)
{
   if (item)
      DeleteItem(item);
}

void SpellList::changeSpell(SpellItem *item)
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
   QListView::clear();
   m_spellList.clear();
   // rebuild categories...
}

void SpellList::selectAndOpen(SpellListItem *item)
{
   QListViewItem *i = item;
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

void SpellList::mouseButtonClicked(int button, QListViewItem *item,
      const QPoint& pos, int col)
{
   if ( (item) && (button == MidButton) ) {
      SpellListItem *i = (SpellListItem *)item;
      SpellItem *j = i->item();
      if (j)
         j->setDuration(-20);
   }
}

void SpellList::rightButtonClicked(QListViewItem *item, const QPoint& pos,
      int col)
{
   //printf("rightButtonClicked()\n");
   //if (!m_menu) {
   m_menu = new QPopupMenu(this);
   m_menu->setCheckable(true);

   mid_spellName = m_menu->insertItem("Spell Name");
   if (columnWidth(SPELLCOL_SPELLNAME))
      m_menu->setItemChecked(mid_spellName, true);
   mid_spellId = m_menu->insertItem("Spell ID");
   if (columnWidth(SPELLCOL_SPELLID))
      m_menu->setItemChecked(mid_spellId, true);
   mid_casterId = m_menu->insertItem("Caster ID");
   if (columnWidth(SPELLCOL_CASTERID))
      m_menu->setItemChecked(mid_casterId, true);
   mid_casterName = m_menu->insertItem("Caster Name");
   if (columnWidth(SPELLCOL_CASTERNAME))
      m_menu->setItemChecked(mid_casterName, true);
   mid_targetId = m_menu->insertItem("Target ID");
   if (columnWidth(SPELLCOL_TARGETID))
      m_menu->setItemChecked(mid_targetId, true);
   mid_targetName = m_menu->insertItem("Target Name");
   if (columnWidth(SPELLCOL_TARGETNAME))
      m_menu->setItemChecked(mid_targetName, true);
   mid_casttime = m_menu->insertItem("Cast Time");
   if (columnWidth(SPELLCOL_CASTTIME))
      m_menu->setItemChecked(mid_casttime, true);
   mid_duration = m_menu->insertItem("Remaining Time");
   if (columnWidth(SPELLCOL_DURATION))
      m_menu->setItemChecked(mid_duration, true);
   //}

   connect(m_menu, SIGNAL(activated(int)), this, SLOT(activated(int)));
   m_menu->popup(pos);
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

SpellListWindow::SpellListWindow(QWidget* parent, const char* name)
  : SEQWindow("SpellList", "ShowEQ - Spell List", parent, name)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setAutoAdd(true);
  
  m_spellList = new SpellList(this, name);
}

SpellListWindow::~SpellListWindow()
{
  delete m_spellList;
}

void SpellListWindow::savePrefs(void)
{
  // save SEQWindow prefs
  SEQWindow::savePrefs();

  // make the listview save it's prefs
  m_spellList->savePrefs();
}
