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

//#define DEBUGMSG

#undef DEBUGMSG


//
// Constructor
//                                                                  
MsgDialog::MsgDialog(QWidget *parent, const char *name, QStringList &list)
// : QDialog(parent, name)
 : QWidget(parent, name)
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
   m_bAdditiveFilter = FALSE;

#if 0
   m_pMsgTypeCheckBox = 0;
   m_pButtonsPanel = 0;
   m_pEdit = 0;
   m_pStatusBar = 0;
   m_pStatusBarLock = 0;
   m_pStatusBarMsgcount = 0;
   m_pStatusBarTotMsgcount = 0;
   m_pStatusBarFilter = 0;
   m_pButtonsLayout = 0;
   m_pMenu = 0;
   m_pStringList = 0;
#endif

//  Anyone want to explain to me why ShowEQ segfaults upon exit when I
//  uncomment out the following line.  This baffles me.... it acts like 
//  it causes something to get destroyed when it's not supposed to be
//     - Maerlyn
//   m_pButtonOver = 0;

   // use the shared list given to us
   m_pStringList = &list;

   // set Title
   setCaption(QString(name));

   // install event filter to catch right clicks to add buttons 
   installEventFilter(this);
   
   // top-level layout; a vertical box to contain all widgets and sublayouts
   QBoxLayout *topLayout = new QVBoxLayout(this);
  
   // Make an hbox that will hold the textbox and the row of filterbuttons
   QBoxLayout *middleLayout = new QHBoxLayout(topLayout);
  
   // add the edit
//   m_pEdit = new QMultiLineEdit(this, "edit"); 
   m_pEdit = new MyEdit(this, "edit"); 
   m_pEdit->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   m_pEdit->setReadOnly(TRUE);
   m_pEdit->setFont(QFont("Helvetica", 10));
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
  

#if 0
   // Add some default filter buttons
   MyButton *but;
 
   for (int i = 0; i < 5; i++)
   {
      char temp[15];
      sprintf(temp, "Empty%d", i);
      QString name(temp);
      QString filter(temp);
      QString color("Black");
      newButton(name, filter, color, FALSE);
   } 
#endif

   // Add an empty widget to fill the space and stretch when resized
   rightLayout->addStretch(10);
  
   // Add popup menu
   m_pMenu = new QPopupMenu(this, "popup");
   m_pMenu->insertItem("&Add Button", this, SLOT(addButton()));
   m_pMenu->insertSeparator();
   m_pMenu->insertItem("&Toggle Controls", this, SLOT(toggleControls()));
   connect(m_pMenu, SIGNAL (aboutToShow(void)), 
              this, SLOT (menuAboutToShow(void)));
  
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

  // keep at bottom
  if (m_pEdit && !m_bScrollLock)
    m_pEdit->pageDown(FALSE);

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
           appendWithWrap(temp);
//           appendWithWrap(QString(string.right(string.length() - index)));
        }
     }
     else 
       appendWithWrap(string);
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
// appendWithWrap(QString &message)  - append a msg to the current text
//
// Word wrap is performed to wrap messages and indent them according to
// the indentation configured
//
void
MsgDialog::appendWithWrap(QString &s)
{
  QFontMetrics fm( font() );
  int i = 0;
  int a = 0;
  int lastSpace = 0;
  int linew = 0;
  int lastw = 0;
  bool doBreak = FALSE;
  bool bWrap = FALSE;
  int indentw = 0;
  QString indentStr("                                         ");

  if (!m_pEdit) return;

#ifdef DEBUGMSG
//  qDebug("appendWithWrap() '%s'", s.ascii() );
#endif
  while( i<int(s.length()) ) 
  {
    doBreak = FALSE;
    if ( s[i] != '\n' )
    {
      linew += fm.width( s[i] );
      if (lastSpace > a)
      {
        if ( (linew + indentw + 10) >= m_pEdit->contentsRect().width())
        {
          doBreak = TRUE;
          if (lastSpace > a)
          {
            i = lastSpace;
            linew = lastw;
          }
          else
            i = QMAX(a, i-1);
        }
      }
    }
//printf("doBreak %d, a %d, i %d\n", doBreak, a, i); 
    if (doBreak)
    {
       QString newstring = s.mid(a, i - a);

       linew = 0;
       lastSpace = a;
       if (bWrap)
         newstring.insert(0,indentStr.left(m_nIndent));
//printf("Adding '%s'\n", newstring.ascii());
       if (m_bUseIndexing)
       {
          QString index("");
          index.sprintf("%05d: ", m_nIndex);
          newstring.insert(0, index);
       }
       m_pEdit->append(newstring);
       bWrap = TRUE;
       indentw = m_nIndent * fm.width(s[32]); 
       a=i+1;
    }

    if (s[i].isSpace())
    {
       lastSpace = i;
       lastw = linew;
    }
    if (lastSpace <= a)
      lastw = linew;

    i++;
  }

  // add remainder of line
  QString newstring = s.mid(a, i - a);
  if (bWrap)
     newstring.insert(0,indentStr.left(m_nIndent));
  if (m_bUseIndexing)
  {
     QString index("");
     index.sprintf("%05d: ", m_nIndex);
     newstring.insert(0, index);
  }
  m_pEdit->append(newstring);
//printf("Adding '%s'\n", newstring.ascii());

} // end appendWithWrap 


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

   return FALSE;

} // end eventFilter


void
MsgDialog::setAdditive(bool bAdd)
{
#ifdef DEBUGMSG
   qDebug("setAdditive %d", bAdd);
#endif
   m_bAdditiveFilter = bAdd;
   m_pAdditiveCheckBox->setChecked(m_bAdditiveFilter);
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
MsgDialog::setButton(MyButton* but, bool active)
{
  if (active)
  {
    if (-1 != m_nEditItem)
       m_pMenu->removeItem(m_nEditItem);

    QString tempstr("");
    tempstr.sprintf("&Edit '%s'", but->name().ascii());
    m_nEditItem = m_pMenu->insertItem(tempstr, this, SLOT(editButton()));
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
           const QString &color, bool bAct)
{
#ifdef DEBUGMSG
  qDebug("newButton() '%s', '%s', '%s' %s", name.ascii(), filter.ascii(),
       color.ascii(), bAct?"Active":"InActive");
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
  but->setColor(QColor(color));

  // add to the button layout
  m_nButtons++;
  m_pButtonsLayout->addWidget(but);
  but->show();

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
}


void
MsgDialog::showControls(bool bShow)
{
   if (bShow)
      m_pButtonsPanel->show();
   else
      m_pButtonsPanel->hide();
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

  MyButton* but = newButton("Name", "Filter", "Color", FALSE);

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
  refresh();
}


//
// addButton - add a button
//
// Creates a new button given all the paramerters for it
// (this is for a public slot)
//
void
MsgDialog::addButton(const QString &name, const QString &filter,
           const QString &color, bool bAct)
{
#ifdef DEBUGMSG
  qDebug("addButton() '%s', '%s', '%s' %s", name.ascii(), filter.ascii(),
       color.ascii(), bAct?"Active":"InActive");
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
  }

  return FALSE;

} // end eventFilter


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

   QLabel *nameLabel = new QLabel ("Name", this);
   nameLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
   nameLabel->setFixedHeight(nameLabel->sizeHint().height());
   nameLabel->setFixedWidth(80);
   nameLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row1Layout->addWidget(nameLabel);
  
   QLineEdit *nameEdit = new QLineEdit (this, "Name");
   nameEdit->setText(but->text());
   nameEdit->setFont(QFont("Helvetica", 12, QFont::Bold));
   nameEdit->setFixedHeight(nameEdit->sizeHint().height());
   nameEdit->setFixedWidth(150);
   row1Layout->addWidget(nameEdit);

   QLabel *filterLabel = new QLabel ("Filter", this);
   filterLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
   filterLabel->setFixedHeight(filterLabel->sizeHint().height());
   filterLabel->setFixedWidth(80);
   filterLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row2Layout->addWidget(filterLabel);
  
   QLineEdit *filterEdit = new QLineEdit (this, "Filter");
   filterEdit->setText(but->filter());
   filterEdit->setFont(QFont("Helvetica", 12, QFont::Bold));
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

   return FALSE;

} // end eventFilter


