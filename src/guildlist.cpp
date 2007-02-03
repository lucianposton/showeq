/*
 * guildlist.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2004-2007 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#include "guildlist.h"
#include "guildshell.h"
#include "player.h"
#include "zonemgr.h"
#include "main.h"

#include <qfont.h>
#include <qpainter.h>
#include <qfontdialog.h>
#include <qinputdialog.h>
#include <q3popupmenu.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

//----------------------------------------------------------------------
// GuildListItem
GuildListItem::GuildListItem(Q3ListView* parent,
			     const GuildMember* member, 
			     const GuildShell* guildShell)
  : Q3ListViewItem(parent),
    m_member(member)
{
  update(guildShell);
}

GuildListItem::~GuildListItem()
{
}

void GuildListItem::paintCell( QPainter *p, const QColorGroup &cg,
			       int column, int width, int alignment )
{
  QFont font = this->listView()->font();

  // members that are actually logged in have their names in bold
  if (m_member->zoneId())
    font.setBold(true);
  else
    font.setBold(false);

  p->setFont(font);

  Q3ListViewItem::paintCell( p, cg, column, width, alignment );
}

int GuildListItem::compare(Q3ListViewItem *o, int col, bool ascending) const
{
  if (!m_member)
    return -1;

  GuildListItem* other = (GuildListItem*)o;
  const GuildMember* otherMember = other->guildMember();

  switch (col)
  {
  case 1: // Level
    if (m_member->level() == otherMember->level())
      return 0;
    else 
      return m_member->level() > otherMember->level() ? 1 : -1;
  case 3: // Guild Rank
    if (m_member->guildRank() == otherMember->guildRank())
      return 0;
    else
      return m_member->guildRank() > otherMember->guildRank() ? 1 : -1;
  case 4: // Banker Rank
    if (m_member->bankRank() == otherMember->bankRank())
      return 0;
    else
      return m_member->bankRank() > otherMember->bankRank() ? 1 : -1;

  case 5: // Alt rank
    if (m_member->altRank() == otherMember->altRank())
      return 0;
    else
      return m_member->altRank() > otherMember->altRank() ? 1 : -1;
  case 6: // Last On
    if (m_member->lastOn() == otherMember->lastOn())
      return 0;
    else
      return m_member->lastOn() > otherMember->lastOn() ? 1 : -1;
  default:
    return Q3ListViewItem::compare(o, col, ascending);
  }
}

static const QString dateFormat("ddd MMM dd hh:mm:ss yyyy");

void GuildListItem::update(const GuildShell* guildShell)
{
  if (!m_member)
    return;

  setText(tGuildListColName, m_member->name());
  setText(tGuildListColLevel, QString::number(m_member->level()));
  setText(tGuildListColClass, m_member->classString());
  setText(tGuildListColRank, m_member->guildRankString());
  setText(tGuildListColBank, m_member->bankRankString());
  setText(tGuildListColAlt, m_member->altRankString());
  QDateTime dt;
  dt.setTime_t(m_member->lastOn());
  setText(tGuildListColLastOn, dt.toString(dateFormat));
  QString zoneString = guildShell->zoneString(m_member->zoneId());
  if (m_member->zoneInstance())
    zoneString += ":" + QString::number(m_member->zoneInstance());
  setText(tGuildListColZone, zoneString);
  setText(tGuildListColPublicNote, m_member->publicNote());
}

void GuildListItem::setGuildMember(const GuildMember* member)
{
  m_member = member;
}

int GuildListItem::rtti() const
{
  return 2004;
}

//----------------------------------------------------------------------
// GuildListWindow
GuildListWindow::GuildListWindow(Player* player,
				 GuildShell* guildShell,
				 QWidget* parent, const char* name)
  : SEQWindow("GuildList", "ShowEQ - Guild Member List", parent, name),
    m_player(player),
    m_guildShell(guildShell), 
    m_guildListItemDict(709),
    m_menu(0),
    m_membersOn(0)
{
  m_guildListItemDict.setAutoDelete(false);

  // get whether to show offline guildmates or not
  m_showOffline = pSEQPrefs->getPrefBool("ShowOffline", preferenceName(),
					 false);

  // get whether to keep the list sorted or not
  m_keepSorted = pSEQPrefs->getPrefBool("KeepSorted", preferenceName(), false);

  m_showAlts = pSEQPrefs->getPrefBool("ShowAlts", preferenceName(), true);

  Q3BoxLayout* vLayout = new Q3VBoxLayout(boxLayout());
  Q3HBoxLayout* hLayout= new Q3HBoxLayout(vLayout);

  // Guild Name
  m_guildName = new QLabel("Guild", this);
  m_guildName->setAlignment(Qt::AlignLeft|Qt::AlignVCenter|SingleLine);
  m_guildName->setFrameShape(LineEditPanel);
  m_guildName->setFrameShadow(Sunken);
  m_guildName->setMinimumWidth(50);
  m_guildName->setMaximumWidth(300);
  hLayout->addWidget(m_guildName, 1, Qt::AlignLeft);
  guildChanged();

  // Guild Totals
  m_guildTotals = new QLabel("", this);
  m_guildTotals->setAlignment(Qt::AlignRight|Qt::AlignVCenter|SingleLine);
  m_guildTotals->setFrameShape(LineEditPanel);
  m_guildTotals->setFrameShadow(Sunken);
  m_guildTotals->setMinimumWidth(30);
  m_guildTotals->setMaximumWidth(120);
  hLayout->addWidget(m_guildTotals, 0, Qt::AlignRight);

  // create the spawn listview
  m_guildList = new SEQListView(preferenceName(), 
				this, "guildlistview");
  vLayout->addWidget(m_guildList);

  // setup the columns
  m_guildList->addColumn("Name");
  m_guildList->addColumn("Level");
  m_guildList->addColumn("Class");
  m_guildList->addColumn("Rank");
  m_guildList->addColumn("Banker");
  m_guildList->addColumn("Alt");
  m_guildList->addColumn("Last On", "LastOn");
  m_guildList->addColumn("Zone");
  m_guildList->addColumn("Public Note", "PublicNote");

  // restore the columns settings from preferences
  m_guildList->restoreColumns();

  connect(m_guildShell, SIGNAL(cleared()),
	  this, SLOT(cleared()));
  connect(m_guildShell, SIGNAL(loaded()),
	  this, SLOT(loaded()));
  connect(m_guildShell, SIGNAL(updated(const GuildMember*)),
	  this, SLOT(updated(const GuildMember*)));

  connect(m_player, SIGNAL(guildChanged()),
	  this, SLOT(guildChanged()));

  // populate the window
  populate();
}

GuildListWindow::~GuildListWindow()
{
}

Q3PopupMenu* GuildListWindow::menu()
{
  if (m_menu)
    return m_menu;

  m_menu = new Q3PopupMenu;
  Q3PopupMenu* guildListColMenu = new Q3PopupMenu;
  m_menu->insertItem( "Show &Column", guildListColMenu);
  guildListColMenu->setCheckable(true);
  m_id_guildList_Cols[tGuildListColName] = 
    guildListColMenu->insertItem("&Name");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColName], 
				     tGuildListColName);
  m_id_guildList_Cols[tGuildListColLevel] = 
    guildListColMenu->insertItem("&Level");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColLevel], 
				     tGuildListColLevel);
  m_id_guildList_Cols[tGuildListColClass] = 
    guildListColMenu->insertItem("&Class");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColClass], 
				     tGuildListColClass);
  m_id_guildList_Cols[tGuildListColRank] = 
    guildListColMenu->insertItem("&Rank");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColRank], 
				     tGuildListColRank);
  m_id_guildList_Cols[tGuildListColBank] = 
    guildListColMenu->insertItem("&Banker");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColBank], 
				     tGuildListColBank);
  m_id_guildList_Cols[tGuildListColAlt] = 
    guildListColMenu->insertItem("&Alt");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColAlt], 
				     tGuildListColAlt);
  m_id_guildList_Cols[tGuildListColLastOn] = 
    guildListColMenu->insertItem("Last &On");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColLastOn], 
				     tGuildListColLastOn);
  m_id_guildList_Cols[tGuildListColZone] = 
    guildListColMenu->insertItem("&Zone");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColZone], 
				     tGuildListColZone);
  m_id_guildList_Cols[tGuildListColPublicNote] = 
    guildListColMenu->insertItem("&Public Note");
  guildListColMenu->setItemParameter(m_id_guildList_Cols[tGuildListColPublicNote], 
				     tGuildListColPublicNote);

  connect (guildListColMenu, SIGNAL(activated(int)), 
	   this, SLOT(toggle_guildListCol(int)));

  m_menu->insertSeparator(-1);
  int x;
  x = m_menu->insertItem("Show Offline", 
			 this, SLOT(toggle_showOffline(int)));
  m_menu->setItemChecked(x, m_showOffline);
  x = m_menu->insertItem("Show Alts", 
			 this, SLOT(toggle_showAlts(int)));
  m_menu->setItemChecked(x, m_showAlts);
  x = m_menu->insertItem("Keep Sorted", 
			 this, SLOT(toggle_keepSorted(int)));
  m_menu->setItemChecked(x, m_keepSorted);

  m_menu->insertSeparator(-1);
  m_menu->insertItem("&Font...", this, SLOT(set_font(int)));
  m_menu->insertItem("&Caption...", this, SLOT(set_caption(int)));

  connect(m_menu, SIGNAL(aboutToShow()),
	  this, SLOT(init_Menu()));

  return m_menu;
}

void GuildListWindow::cleared()
{
  clear();
}

void GuildListWindow::loaded()
{
  populate();
}
 
void GuildListWindow::updated(const GuildMember* member)
{
  GuildListItem* memberItem = m_guildListItemDict.find((void*)member);

  if (memberItem)
  {
    // We have them in our list already. Need to update.
    bool bRemove = false;

    if (! m_showAlts && member->altRank())
    {
        // This is an alt and we're not showing alts
        bRemove = true;
    }

    // if not-showing offline users and this user has become offline,
    // then remove it
    if (! m_showOffline && ! member->zoneId())
    {
        // This dude is offline and we're not showing offline.
        bRemove = true;
    }

    // If we got an update for someone we had, but now they are offline,
    // make them offline
    if (! member->zoneId())
    {
        m_membersOn--;
    }

    if (bRemove)
    {
        // remove the item from the item dictionary
        m_guildListItemDict.remove((void*)member);
	
        // delete the item
        delete memberItem;
    }
    else
    {
        memberItem->update(m_guildShell);
    }
  }
  else 
  {
    // Not in list yet.
    if (member->zoneId())
    {
        // Online.
        m_membersOn++;
    }

    // Assume we should add them.
    bool bAdd = true;

    // Don't add ignored offliners.
    if (! m_showOffline && ! member->zoneId())
    {
        bAdd = false;
    }

    // Don't add ignored alts.
    if (! m_showAlts && member->altRank())
    {
        bAdd = false;
    }

    if (bAdd)
    {
        // add the new guild member item
        memberItem = new GuildListItem(m_guildList, member, m_guildShell);
      
        // insert it into the dictionary
        m_guildListItemDict.insert((void*)member, memberItem);
    }
  }

  // make sure the guild list is sorted
  if (m_keepSorted)
    m_guildList->sort();

  updateCount();
}

void GuildListWindow::guildChanged()
{
  QString guild(" Guild: %1 ");

  // set the guild name
  m_guildName->setText(guild.arg(m_player->guildTag()));
}

void GuildListWindow::init_Menu(void)
{
  // make sure the menu bar settings are correct
  for (int i = 0; i < tGuildListColMaxCols; i++)
    m_menu->setItemChecked(m_id_guildList_Cols[i], 
			   m_guildList->columnVisible(i));
}

void GuildListWindow::toggle_showOffline(int id)
{
  // toggle immediate update value
  m_showOffline = !m_showOffline;
  m_menu->setItemChecked(id, m_showOffline);
  pSEQPrefs->setPrefBool("ShowOffline", preferenceName(), 
			 m_showOffline);

  // re-populate the window
  populate();
}

void GuildListWindow::toggle_showAlts(int id)
{
  // toggle immediate update value
  m_showAlts = !m_showAlts;
  m_menu->setItemChecked(id, m_showAlts);
  pSEQPrefs->setPrefBool("ShowAlts", preferenceName(), 
			 m_showAlts);

  // re-populate the window
  populate();
}

void GuildListWindow::toggle_keepSorted(int id)
{
  // toggle immediate update value
  m_keepSorted = !m_keepSorted;
  m_menu->setItemChecked(id, m_keepSorted);
  pSEQPrefs->setPrefBool("KeepSorted", preferenceName(), 
			 m_keepSorted);
  if (m_keepSorted)
    m_guildList->sort();
}

void GuildListWindow::toggle_guildListCol( int id )
{
  int colnum;

  colnum = m_menu->itemParameter(id);
  
  if (menu()->isItemChecked(id))
    m_guildList->setColumnVisible(colnum, false);
  else
    m_guildList->setColumnVisible(colnum, true);
}

void GuildListWindow::set_font(int /*id*/)
{
  QFont newFont;
  bool ok = false;
  // get a new font
  newFont = QFontDialog::getFont(&ok, font(),
				 this, caption() + " Font");
    
    
    // if the user entered a font and clicked ok, set the windows font
    if (ok)
      setWindowFont(newFont);
}

void GuildListWindow::set_caption(int /*id*/)
{
  bool ok = false;

  QString captionText = 
    QInputDialog::getText("ShowEQ Guild List Window Caption",
			  "Enter caption for the Guild List Window:",
			  QLineEdit::Normal, caption(),
			  &ok, this);
  
  // if the user entered a caption and clicked ok, set the windows caption
  if (ok)
    setCaption(captionText);
}

void GuildListWindow::clear(void)
{
  // clear count
  m_membersOn = 0;

  // clear out the guild list item dictionary
  m_guildListItemDict.clear();
  
  // clear the guild list contents
  m_guildList->clear();

  updateCount();
}

void GuildListWindow::populate(void)
{
  GuildMember* member;
  GuildListItem* memberItem;

  // make sure everything is out of the list view first...
  clear();

  // disable updates
  setUpdatesEnabled(false);

  // iterate over the members
  GuildMemberDictIterator it(m_guildShell->members());

    // iterate over all the members
    while ((member = it.current()))
    {
        // increment members on count for each member on
        if (member->zoneId())
        {
	        m_membersOn++;
        }

        bool bAdd = true;

        if (member->altRank() && ! m_showAlts)
        {
            bAdd = false;
        }

        if (! member->zoneId() && ! m_showOffline)
        {
            bAdd = false;
        }

        if (bAdd)
        {
            memberItem = new GuildListItem(m_guildList, member, m_guildShell);
      
            // insert it into the dictionary
            m_guildListItemDict.insert((void*)member, memberItem);
        }

        ++it;
    }  

  // make sure the guild list is sorted
  m_guildList->sort();

  // update the counts
  updateCount();

  // re-enable updates and force a repaint
  setUpdatesEnabled(true);
  repaint();
}

void GuildListWindow::updateCount(void)
{
  QString text(" %1 on/%2 total ");
  m_guildTotals->setText(text.arg(m_membersOn).arg(m_guildShell->members().count()));
}

#ifndef QMAKEBUILD
#include "guildlist.moc"
#endif

