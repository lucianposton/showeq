/*
 * msgdlg.cpp
 *
 * Filterable Message Box module
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

/*
 * Orig Author - Maerlyn (MaerlynTheWiz@yahoo.com)
 * Date   - 3/19/00
 */

//
// Notes: This REALLY needs to be cleaned up
//  + Possibly use filter classes to do the filtering as oppossed to 
//    constantly re-compiling the regex
//  + Add ability for user to specify a filter like in the Map code
//  + General cleanup

/*
 * Todo: MyButton should be put inside of the MsgDialog class
 *       to avoid namespace collsions.  How to do this with Qt moc?
 *
 *       Add style / color capabilities to the filters
 *       Fix height resizing of dialog when adding a button dynamically
 *       
 *       Clean up general functionality... scrolllock should be automatic
 */

/*
 * MsgDialog
 *
 * Class provides a topLevel dialog for displaying messages.  Features include:
 *
 *  - List of checkboxes which can be edited for filters
 *  - Configuration of additive or subtractive filtering
 *        (a match on a filter will show the message if additive, will
 *         filter the message if subtractive)
 *  - WordWrapping with indentation
 *
 * public Slots:
 *
 *    newMessage(const QString &)  -  to send a new message to the dialog
 *    setAdditive(bool bAdd)       -  change the filter scheme:
 *                                 TRUE is a positive filter, FALSE is negative
 *    setMargin(int nIndent)     { m_nIndent = nIndent; refresh(); }
 *
 * If you wish to construct and have a list of strings already that you wish
 * to send to the dialog, you can use the constructor to send a stringlist
 */

#include <stdio.h>
#include <stdlib.h>

#include <qlayout.h>
#include <qlist.h>
#include <qmultilineedit.h>
#include <qstring.h>
#include <qvbox.h>
#include <qlcdnumber.h>
#include <qframe.h>
#include <qstringlist.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlineedit.h>

#include "msgdlg.h"
#include "main.h"

//#define DEBUGMSG

#undef DEBUGMSG


//
// Constructor
//                                                                  
MsgDialog::MsgDialog(QWidget *parent, const char *name, 
		     const QString& prefName, QStringList &list)
  : SEQWindow(prefName, name, parent, name)
{
#ifdef DEBUGMSG
  qDebug("MsgDialog() '%s' List passed by ref with %d elements", 
       name, list.count());
#endif

   m_bScrollLock = FALSE;
   m_nButtons = 0;
   m_nIndent = 5;   // num of spaces to indent wrapped lines
   m_nLockIndex = 0;
   m_nIndex = 0;
   m_bUseIndexing = FALSE;
   m_bShowType = TRUE;
   m_nShown = 0;
   m_nEditItem = -1;
   m_nDeleteItem = -1;
   m_bAdditiveFilter = FALSE;
   m_pButtonOver = NULL;
   m_buttonList.setAutoDelete(false);

   // use the shared list given to us
   m_pStringList = &list;

   // install event filter to catch right clicks to add buttons 
   installEventFilter(this);
   
   // top-level layout; a vertical box to contain all widgets and sublayouts
   QBoxLayout *topLayout = new QVBoxLayout(this);
  
   // Make an hbox that will hold the textbox and the row of filterbuttons
   QBoxLayout *middleLayout = new QHBoxLayout(topLayout);
  
   // add the edit
   m_pEdit = new MyEdit(this, "edit"); 
   m_pEdit->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   m_pEdit->setReadOnly(TRUE);
   m_pEdit->setWordWrap(QMultiLineEdit::WidgetWidth);
   m_pEdit->setWrapPolicy(QMultiLineEdit::AtWhiteSpace);
   middleLayout->addWidget(m_pEdit);
  
   // add a vertical box to hold the button layout and the stretch 
   QBoxLayout *rightLayout = new QVBoxLayout(middleLayout);
  
   // add a vertical box to hold the filter buttons
   m_pButtonsPanel = new QWidget(this, "buttonPanel");
   rightLayout->addWidget(m_pButtonsPanel);
   m_pButtonsLayout = new QVBoxLayout(m_pButtonsPanel);
  
   // Make an hbox that will hold the tools 
   QBoxLayout *tools = new QHBoxLayout(m_pButtonsLayout);

   // Make an bbox that will hold the right tools 
   QBoxLayout *righttools = new QVBoxLayout(tools);

   // Add an 'additive' vs 'subtractive checkbox
   m_pAdditiveCheckBox = new QCheckBox("Additive", m_pButtonsPanel);
   m_pAdditiveCheckBox->setChecked(isAdditive());
   connect(m_pAdditiveCheckBox, SIGNAL (toggled(bool)), 
                this, SLOT (setAdditive(bool)));
   righttools->addWidget(m_pAdditiveCheckBox);
  
   // Add a 'scroll-lock' checkbox
   QCheckBox *pScrollLockCheckBox= new QCheckBox("Lock", m_pButtonsPanel);
   pScrollLockCheckBox->setChecked(FALSE);
   connect(pScrollLockCheckBox, SIGNAL (toggled(bool)),
                this, SLOT (scrollLock(bool)));
   righttools->addWidget(pScrollLockCheckBox);

   // Add a 'msg type' checkbox
   m_pMsgTypeCheckBox = new QCheckBox("Msg Type", m_pButtonsPanel);
   m_pMsgTypeCheckBox->setChecked(m_bShowType);
   connect(m_pMsgTypeCheckBox, SIGNAL (toggled(bool)),
                this, SLOT (showMsgType(bool)));
   righttools->addWidget(m_pMsgTypeCheckBox);

   // Add a decrorative frame seperator
   QFrame *frame = new QFrame(m_pButtonsPanel, "seperator");
   frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   frame->setFixedHeight(2);
   m_pButtonsLayout->addWidget(frame);
   
   // 
   // Status Bar
   //
   // create a label to look like a status bar
   QBoxLayout *statusLayout = new QHBoxLayout(topLayout);
   m_pStatusBarFilter = new QLabel(this);
   m_pStatusBarFilter->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   statusLayout->addWidget(m_pStatusBarFilter, 4);
   m_pStatusBarMsgcount = new QLabel(this);
   m_pStatusBarMsgcount->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   statusLayout->addWidget(m_pStatusBarMsgcount, 1);
   m_pStatusBarTotMsgcount = new QLabel(this);
   m_pStatusBarTotMsgcount->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   statusLayout->addWidget(m_pStatusBarTotMsgcount, 1);
   m_pStatusBarLock = new QLabel(this);
   m_pStatusBarLock->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   statusLayout->addWidget(m_pStatusBarLock, 1);
  
   // Add an empty widget to fill the space and stretch when resized
   rightLayout->addStretch(10);
  
   // Add popup menu
   m_pMenu = new QPopupMenu(this, "popup");
   m_pMenu->insertItem("&Add Button", this, SLOT(addButton()));
   m_pMenu->insertSeparator();
   m_pMenu->insertItem("&Toggle Controls", this, SLOT(toggleControls()));
   connect(m_pMenu, SIGNAL (aboutToShow(void)), 
              this, SLOT (menuAboutToShow(void)));

   // load the preferences
   load();

   // refresh the messages 
   refresh();
  
} // end constructor


//
// refresh()
//
// Apply active filters to messages in editline
//
void
MsgDialog::refresh(void)
{
#ifdef DEBUGMSG
  qDebug("refresh()");
#endif
  if (!m_pEdit) return;

  m_pEdit->clear();
  m_nShown = 0;

  // add the strings from the current array
  if (m_pStringList)
  {
    QStringList::Iterator strIt;
    m_nIndex = 0;
    for (strIt = m_pStringList->begin(); strIt != m_pStringList->end();++strIt )
    {
      m_nIndex++;
      addMessage(*strIt);
    }
  } // end if m_pStringList

   // update the status bar
   QString temp("");
   temp.sprintf("%d", m_nShown);
   m_pStatusBarMsgcount->setText(temp);
   if (isAdditive())
      temp = " + ";
   else
      temp = " - ";
   QStringList::Iterator filterIt;
   for(filterIt = m_filterList.begin(); filterIt != m_filterList.end();
         filterIt++)
   {
      temp += ", ";
      temp += *filterIt;
   } 
   m_pStatusBarFilter->setText(temp);

} // end refresh()


//
// newMessage(int index)
//
// add a new message to the message list and apply it to active filers
// and refresh
//
void
MsgDialog::newMessage(int index)
{
  if (!m_pStringList) return;

#ifdef DEBUGMSG
  qDebug("newMessage() adding index %d: '%s'", index,
           ((*m_pStringList)[index]).ascii() );
#endif

  // update status bar
  QString temp("");
  temp.sprintf("%d", m_pStringList->count());
  m_pStatusBarTotMsgcount->setText(temp);

  // append it to the editbox
    addMessage((*m_pStringList)[index]);

  // set index
  m_nIndex = index;

#if QT_VERSION < 300 // only necessary for older Qt pre-3.0
  // keep at bottom
  if (m_pEdit && !m_bScrollLock)
    m_pEdit->pageDown(FALSE);
#endif

} // end newMessage
 

//
// addMessage
//
// Add a message to the dialog using filters in the filterlist
// Note:  Filters are inclusive.  If no match on any filter, then the message
//        will not be added
//        Filters can be additive or subtractive.  If there is a match on an
//        additive filter the msg is added, match on subtractive it is not 
//        added.  Filters are processed in order as added therefore any filter
//        can override a previous filters affect on a message.
//
void
MsgDialog::addMessage(QString &string)
{
#ifdef DEBUGMSG
   qDebug("addMessage() '%s'", string.ascii() );
#endif

   bool bAdd = !isAdditive();

   // iterate through filters
   QStringList::Iterator filterIt;
   for(filterIt = m_filterList.begin(); filterIt != m_filterList.end();
         filterIt++)
   {
     QRegExp re(*filterIt);

     // if additive and we ahve a match add it
     if ( -1 != re.match(string) )
        bAdd = isAdditive();

   } // end for all filters

   if (bAdd & !m_bScrollLock)
   {
     m_nShown++;

     // delete the filter string if we are not showing it
     if (!m_bShowType)
     {
        int index = string.find(':');
        if (index)
        {
           QString temp(string.right(string.length() - (index + 1)));
	   m_pEdit->append(temp);
        }
     }
     else 
       m_pEdit->append(string);
   }

} // end addMessage()


//
// addFilter(const QString &filter)     - add a filter to the active filters
//
// calls refresh() as this is usually an emit from a MyButton
//
void
MsgDialog::addFilter(const QString &filter)
{
#ifdef DEBUGMSG
   qDebug("addFilter() - '%s' %d", filter.ascii(), m_filterList.count() + 1);
#endif
   m_filterList.append(filter);

   refresh();

} // addFilter


void
MsgDialog::addFilter(MyButton *but)
{
   addFilter(but->filter());
}

//
// delFilter(const QString &filter)   - remove a filter from the active filters
//
// calls refresh() as this is usually an emit from a MyButton
//
void
MsgDialog::delFilter(const QString &filter)
{
#ifdef DEBUGMSG
   qDebug("delFilter() - '%s'", filter.ascii());
#endif
   m_filterList.remove(filter);

   refresh();

} // delFilter

//
// eventFilter - filter events looking for a rightclick
//
// On a right mouseclick we call the addButton method since I can't get
// the popup menu working
//
bool
MsgDialog::eventFilter(QObject *o, QEvent *e)
{
  switch (e->type())
  {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *m = (QMouseEvent *) e;

        if (m->button() == 1)  // left click
        {
        }
        if (m->button() == 2)  // right click
        {
          m_pMenu->popup(m->globalPos());
          return TRUE;  // don't process event any further 
        }
    }
      break;

    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    default:
        break;
   }

   return QWidget::eventFilter(o, e);

} // end eventFilter


void
MsgDialog::setAdditive(bool bAdd)
{
#ifdef DEBUGMSG
   qDebug("setAdditive %d", bAdd);
#endif
   m_bAdditiveFilter = bAdd;
   m_pAdditiveCheckBox->setChecked(m_bAdditiveFilter);

   pSEQPrefs->setPrefBool("Additive", preferenceName(), m_bAdditiveFilter);
   refresh();
}


void
MsgDialog::setMargin(int nIndent)
{
#ifdef DEBUGMSG
   qDebug("setMargin %d", nIndent);
#endif
   m_nIndent = nIndent;
   refresh();
}

void
MsgDialog::setIndexing(bool bIndex)
{
#ifdef DEBUGMSG
   qDebug("setIndexing %d", bIndex);
#endif
   m_bUseIndexing = bIndex;
   refresh();
}

//
// editButton(MyButton *button) - edit the button 'button'
//
// Pops up a modal CButDlg widget.  After the button has been edited
// it is removed and readded from the active filters if it is selected
//
void
MsgDialog::editButton(MyButton *but)
{
#ifdef DEBUGMSG
  qDebug("editButton(MyButton *) '%s'", but->text().ascii());
#endif

  QString oldfilter(but->filter());  // hold copy of old filter

  CButDlg butdlg(parentWidget(), "ButtonDlg", but);

  butdlg.exec();

  // re-apply the filter if we are active
  if (but->isChecked())
  {
    delFilter(oldfilter);
    addFilter(but->filter());
  }
}  // end edit Button



//
// editButton - edit a button
//
// TODO:  Get the popup menu working
//
// This should edit the button the mouse is over when called but I can't
// get the popup menu to work so I haven't implemented it
//
void
MsgDialog::editButton(void)
{
#ifdef DEBUGMSG
  qDebug("editButton()");
#endif
  editButton(m_pButtonOver);
}

void
MsgDialog::deleteButton(void)
{
  if (!m_pButtonOver)
    return;

#ifdef DEBUGMSG
  qDebug("deleteButton() '%s'", m_pButtonOver->text().ascii());
#endif

  // remove the filter if active
  if (m_pButtonOver->isChecked())
    delFilter(m_pButtonOver->filter());

  // remove the button from the button list
  m_buttonList.remove(m_pButtonOver);

  // delete the button
  delete m_pButtonOver;

  // reset the button over to NULL
  m_pButtonOver = NULL;
  
  // delete the associated menu items
  if (-1 != m_nDeleteItem)
  {
    m_pMenu->removeItem(m_nDeleteItem);
    m_nDeleteItem = -1;
  }
  if (-1 != m_nEditItem)
  {
    m_pMenu->removeItem(m_nEditItem);
    m_nEditItem = -1;
  }
}

void
MsgDialog::setButton(MyButton* but, bool active)
{
  if (active)
  {
    if (-1 != m_nDeleteItem)
      m_pMenu->removeItem(m_nDeleteItem);
    if (-1 != m_nEditItem)
      m_pMenu->removeItem(m_nEditItem);

    QString tempstr;
    tempstr.sprintf("&Edit '%s'", but->name().ascii());
    m_nEditItem = m_pMenu->insertItem(tempstr, this, SLOT(editButton()));
    tempstr.sprintf("&Delete '%s'", but->name().ascii());
    m_nDeleteItem = m_pMenu->insertItem(tempstr, this, SLOT(deleteButton()));
    m_pButtonOver = but;
  }
}


//
// newButton - add a button
//
// Creates a new button given all the paramerters for it
// This does the majority of the work for the other addButton slots
//
MyButton *
MsgDialog::newButton(const QString &name, const QString &filter,
           const QColor &color, bool bAct)
{
#ifdef DEBUGMSG
  qDebug("newButton() '%s', '%s', '%s' %s", name.ascii(), filter.ascii(),
       color.name().ascii(), bAct?"Active":"InActive");
#endif

  // Create a new button
  MyButton *but = new MyButton(name, m_pButtonsPanel, this);
  but->setFilter(filter);
  but->setChecked(bAct);

  // setup signals        
  connect(but, SIGNAL (editButton(MyButton *)),
	  this, SLOT (editButton(MyButton *)));
  connect(but, SIGNAL (addFilter(const QString &)),
	  this, SLOT (addFilter(const QString &)));
  connect(but, SIGNAL (delFilter(const QString &)),
	  this, SLOT (delFilter(const QString &)));
  connect(but, SIGNAL (toggled(bool)),
	  but, SLOT (toggled(bool)));
  connect(but, SIGNAL (setButton(MyButton*, bool)),
	  this, SLOT (setButton(MyButton*, bool)));

  // setup Color
  but->setColor(color);

  // add to the button layout
  m_nButtons++;
  m_pButtonsLayout->addWidget(but);
  but->show();

  // add the button to the button list
  m_buttonList.append(but);

  // re-apply the filter if we are active
  if (but->isChecked())
    addFilter(but->filter());

  return but;

} // end newButton()         


//
// toggleControls
//
void
MsgDialog::toggleControls(void)
{
   if (m_pButtonsPanel->isVisible())
      m_pButtonsPanel->hide();
   else
      m_pButtonsPanel->show();

  pSEQPrefs->setPrefBool("HideControls", preferenceName(), 
			 !m_pButtonsPanel->isVisible());
}


void
MsgDialog::showControls(bool bShow)
{
  if (bShow)
    m_pButtonsPanel->show();
  else
    m_pButtonsPanel->hide();
  
  pSEQPrefs->setPrefBool("HideControls", preferenceName(), !bShow);
}


//
// addButton - add a button
//
// Creates a new button and Pops up a modal CButDlg widget to edit it.
// After the button has been edited it added to the active filters
// if it is selected
// (this is for a slot)
//
void
MsgDialog::addButton(void)
{
#ifdef DEBUGMSG
  qDebug("addButton()");
#endif

  MyButton* but = newButton("Name", "Filter", QColor("black"), FALSE);

  QString oldfilter(but->filter());  // hold copy of old filter
  CButDlg butdlg(parentWidget(), "ButtonDlg", but);
  butdlg.exec();

  // apply the filter if we are active
  if (but->isChecked())
    addFilter(but->filter());
} // end addButton()



//
// scrollLock
//
// keep widgetf rom scrolling
void
MsgDialog::scrollLock(bool bLock)
{
  // what a cheap way to do this... can't figure out how to lock the scrolling
  // bottom line is that the QMultiLineEdit box sucks donkey doodoo
  if (bLock)
  {
    // we'll keep from posting messages and catch up later
    m_nLockIndex = m_pStringList->count();
//printf("Locked at line %d\n", m_nLockIndex);
    m_bScrollLock = TRUE;
    m_pStatusBarLock->setText("Lock");
  }
  else
  {
//printf("Unlocked at line %d, catching up to %d\n", m_nLockIndex,
//      m_pStringList->count());
    m_bScrollLock = FALSE;
    m_pStatusBarLock->setText("");
    refresh();
  }
} // end scrollLock


//
// showMsgType - toggle displaying message type 
//
void
MsgDialog::showMsgType(bool bshow)
{
  if (bshow)
     m_bShowType = TRUE;
  else
     m_bShowType = FALSE;
     
  if (m_pMsgTypeCheckBox)
    m_pMsgTypeCheckBox->setChecked(m_bShowType);
  
  pSEQPrefs->setPrefBool("ShowMsgType", preferenceName(), m_bShowType);
  refresh();
}

void 
MsgDialog::load()
{
  QString tempStr;
  
  // set Additive mode
  m_bAdditiveFilter = pSEQPrefs->getPrefBool("Additive", preferenceName());
  m_pAdditiveCheckBox->setChecked(m_bAdditiveFilter);
  
  // set control mode
  if (pSEQPrefs->getPrefBool("HideControls", preferenceName()))
    m_pButtonsPanel->hide();
  else
    m_pButtonsPanel->show();
  
  // set Msg Type mode
  m_bShowType = pSEQPrefs->getPrefBool("ShowMsgType", preferenceName());
  if (m_pMsgTypeCheckBox)
    m_pMsgTypeCheckBox->setChecked(m_bShowType);
  
  int j = 0;
  // Configure buttons
  for(j = 1; j < 15; j++)
  {
    // attempt to pull button description from the preferences
    tempStr.sprintf("Button%dName", j);
    QString buttonname(pSEQPrefs->getPrefString(tempStr, preferenceName()));
    tempStr.sprintf("Button%dFilter", j);
    QString buttonfilter(pSEQPrefs->getPrefString(tempStr, preferenceName()));
    tempStr.sprintf("Button%dColor", j);
    QColor buttoncolor(pSEQPrefs->getPrefColor(tempStr, preferenceName(), 
					       QColor("black")));
    tempStr.sprintf("Button%dActive", j);
    
    // if we have a name and filter string
    if (!buttonname.isEmpty() && !buttonfilter.isEmpty())
    {
      // Add the button
      addButton(buttonname, buttonfilter,
		buttoncolor, 
		pSEQPrefs->getPrefBool(tempStr, preferenceName()));
    }
    else
    {
      if (!buttonname.isEmpty() || !buttonfilter.isEmpty())
	fprintf(stderr, "Error: Incomplete definition of Button '%s'\n",
		       (const char*)caption());
    }
  } // end for buttons
}

void MsgDialog::savePrefs()
{
  SEQWindow::savePrefs();

  QString tempStr;
  MyButton* but;
  int j = 1;
  for (but = m_buttonList.first(); but != NULL; but = m_buttonList.next())
  {
    tempStr.sprintf("Button%dName", j);
    pSEQPrefs->setPrefString(tempStr, preferenceName(), but->name());
    tempStr.sprintf("Button%dFilter", j);
    pSEQPrefs->setPrefString(tempStr, preferenceName(), but->filter());
    tempStr.sprintf("Button%dColor", j);
    pSEQPrefs->setPrefColor(tempStr, preferenceName(), but->color());
    tempStr.sprintf("Button%dActive", j);
    pSEQPrefs->setPrefBool(tempStr, preferenceName(), but->isChecked());
    j++;
  }

  while (j < 15)
  {
    tempStr.sprintf("Button%dName", j);
    pSEQPrefs->setPrefString(tempStr, preferenceName(), "");
    tempStr.sprintf("Button%dFilter", j);
    pSEQPrefs->setPrefString(tempStr, preferenceName(), "");
    tempStr.sprintf("Button%dColor", j);
    pSEQPrefs->setPrefColor(tempStr, preferenceName(), "");
    tempStr.sprintf("Button%dActive", j);
    pSEQPrefs->setPrefBool(tempStr, preferenceName(), "");
    j++;
  }
}

//
// addButton - add a button
//
// Creates a new button given all the paramerters for it
// (this is for a public slot)
//
void
MsgDialog::addButton(const QString &name, const QString &filter,
		     const QColor &color, bool bAct)
{
#ifdef DEBUGMSG
  qDebug("addButton() '%s', '%s', '%s' %s", name.ascii(), filter.ascii(),
       color.name().ascii(), bAct?"Active":"InActive");
#endif

  newButton(name, filter, color, bAct);

} // end addButton()         


//
// menuAboutToShow()  - called right before popup menu shows
//
// TODO:  Once I get popup menu working this should change the menu contents
//        based on where the mouse is
//
void
MsgDialog::menuAboutToShow(void)
{
}

void MsgDialog::closeEvent( QCloseEvent *e)
{
    e->accept();
    emit toggle_view_ChannelMsgs();
}

//
//
// MyButton Class
//
// A Button with a built in filter for use with the MsgDialog 
// provides a checkbox with right click detection
//


//
// constructor
//
MyButton::MyButton(const char* name, QWidget* parent, QWidget* owner)
  : QCheckBox(name, parent, name)
{
  setChecked(FALSE);
  m_filter = QString::null;
  m_name = QString(name);
  installEventFilter(this);
  m_color = Qt::black;
  m_pOwner = owner;
}

//
// eventFilter - filter events looking for a rightclick
//
// on a right mouseclick we edit the button
//
bool
MyButton::eventFilter(QObject *o, QEvent *e)
{
  switch (e->type())
  {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *m = (QMouseEvent *) e;

        if (m->button() == 2)  // right click
        {
           // pass on to the dialog owning us 
           return m_pOwner->eventFilter(o, e);
        }
    }
    case QEvent::Enter:
      emit setButton(this, TRUE);
      break;
    case QEvent::Leave:
      emit setButton(this, FALSE);
      break;
  default:
    break;
  }

  return QCheckBox::eventFilter(o, e);
} // end eventFilter

void MyButton::setFilter(const QString& string)  
{ 
  m_filter = string; 
}

void MyButton::setText(const QString& string)    
{ 
  QButton::setText(string); 
  m_name = string;
}

void MyButton::toggled(bool on)       
{  
  if (on) 
    emit addFilter(m_filter);
  else 
    emit delFilter(m_filter); 
}

void MyButton::setColor(const QColor& color)
{ 
  m_color = color; 
} 

//
// paintCell
//
// overridden from base class in order to change color and style attributes
//
void
MyButton::paintEvent( QPaintEvent* e)
{
  // TODO:  Implement setting of color based on row
  // will have to keep track of what color belongs to what row back when 
  // its filtered
  QCheckBox::paintEvent(e);
} // end PaintCell



//
//
// CButDlg Class
//
// Provide a dialog to add/edit a filter button to the MsgDialog 
//
CButDlg::CButDlg(QWidget *parent, QString name, MyButton *but)
  : QDialog(parent, name, TRUE)
{
   QBoxLayout *topLayout = new QVBoxLayout(this);
   QBoxLayout *row1Layout = new QHBoxLayout(topLayout);
   QBoxLayout *row2Layout = new QHBoxLayout(topLayout);

   QFont labelFont(font());
   labelFont.setBold(true);

   QLabel *nameLabel = new QLabel ("Name", this);
   nameLabel->setFont(labelFont);
   nameLabel->setFixedHeight(nameLabel->sizeHint().height());
   nameLabel->setFixedWidth(80);
   nameLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row1Layout->addWidget(nameLabel);
  
   QLineEdit *nameEdit = new QLineEdit (this, "Name");
   nameEdit->setText(but->text());
   nameEdit->setFont(labelFont);
   nameEdit->setFixedHeight(nameEdit->sizeHint().height());
   nameEdit->setFixedWidth(150);
   row1Layout->addWidget(nameEdit);

   QLabel *filterLabel = new QLabel ("Filter", this);
   filterLabel->setFont(labelFont);
   filterLabel->setFixedHeight(filterLabel->sizeHint().height());
   filterLabel->setFixedWidth(80);
   filterLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row2Layout->addWidget(filterLabel);
  
   QLineEdit *filterEdit = new QLineEdit (this, "Filter");
   filterEdit->setText(but->filter());
   filterEdit->setFont(labelFont);
   filterEdit->setFixedHeight(filterEdit->sizeHint().height());
   filterEdit->setFixedWidth(150);
   row2Layout->addWidget(filterEdit);

   QPushButton *ok = new QPushButton("OK", this);
   ok->setFixedWidth(30);
   ok->setFixedHeight(30);
   topLayout->addWidget(ok, 0, AlignCenter);
 
   // connect sigs
   connect(nameEdit, SIGNAL(textChanged(const QString &)),
      but, SLOT(setText(const QString &)));  
   connect(filterEdit, SIGNAL(textChanged(const QString &)),
      but, SLOT(setFilter(const QString &)));  
   connect(ok, SIGNAL(clicked()), SLOT(accept()));

   setMaximumSize(QSize(sizeHint()));
}


//
// MyEdit
//
// overridden QMultiline edit
//
//
MyEdit::MyEdit( QWidget* parent, const char* name)
 : QMultiLineEdit(parent, name)
{
   // install event filter to catch right clicks to add buttons 
  installEventFilter(this);
}
   

//
// eventFilter - filter events looking for a rightclick
//
// On a right mouseclick we call the addButton method since I can't get
// the popup menu working
//
bool
MyEdit::eventFilter(QObject *o, QEvent *e)
{
  switch (e->type())
  {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *m = (QMouseEvent *) e;
        if (m->button() == 1)  // left click
        {
        }
        if (m->button() == 2)  // right click
        {
           // pass on to the dialog owning us 
           return parent()->eventFilter(o, e);
        }
    }
      break;

    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    default:
        break;
   }

  // the parent widget may have it's own event filter, pass the call on
  return QMultiLineEdit::eventFilter(o, e);
} // end eventFilter


