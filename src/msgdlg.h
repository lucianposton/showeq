/*
 * msgdlg.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

/*
 * Orig Author - Maerlyn (MaerlynTheWiz@yahoo.com)
 * Date   - 3/19/00
 */

/*
 * Todo: 
 *
 *   Implement coloring based on filter
 *   Can MyButton and CButDlg be moved into MsgDialog to avoid namespace issues?
 *      I tried and ran into problems with moc... maybe sigs and slots are not
 *      supported for a class within a class
 */ 
#ifndef MSGDLG_H
#define MSGDLG_H

#include <qlist.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>
#include <qdialog.h>
#include <qarray.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qlist.h>

class MyButton;  // button for use in MsgDialog
class CButDlg;   // dialog for editing a button
class MyEdit;

typedef QList<MyButton> MyButtonList;

//////////////////////////////////////////////////////////////////////
//                        MsgDialog                                 //
//////////////////////////////////////////////////////////////////////
 

//
// MsgDialog
//
// Class provides a topLevel dialog for displaying messages.  Features include:
//
//  - List of checkboxes which can be edited for filters
//  - Configuration of additive or subtractive filtering
//        (a match on a filter will show the message if additive, will
//         filter the message if subtractive)
//  - WordWrapping with indentation
//
// public Slots:
//
//    newMessage(int index)    - the index into the array of strings to add 
//    setAdditive(bool bAdd)   -  change the filter scheme: 
//                                 TRUE is a positive filter, FALSE is negative 
//    setMargin(int nIndent)     { m_nIndent = nIndent; refresh(); }
//
// If you wish to construct and have a list of strings already that you wish 
// to send to the dialog, you can use the constructor to send a stringlist
//
//class MsgDialog: public QDialog 
class MsgDialog : public QWidget
{
   Q_OBJECT
public:
   MsgDialog(QWidget *parent, const char *name, 
	     const QString& prefName, QStringList &list);

   bool isAdditive()               { return m_bAdditiveFilter; }
   int  indentMargin()             { return m_nIndent; }
   QRect getRect()                 { return rect(); }

public slots:
   void newMessage(int);
   void setAdditive(bool bAdd);
   void setMargin(int nIndent);
   void addButton(const QString &name, const QString &filter,
           const QColor &color, bool bAct);
   void toggleControls(void);
   void setButton(MyButton* but, bool active);
   void showControls(bool);
   void showMsgType(bool);
   void load();
   void savePrefs();

private slots:
   void editButton(MyButton *);         // popup button edit dialog
   void addFilter(const QString &);     // add a filter (no refresh)
   void delFilter(const QString &);     // delete a filter (no refresh)
   void refresh();                      // refresh msgs based on active filters
   void addFilter(MyButton *);
   void scrollLock(bool);
   void setIndexing(bool);

   // For the popupmenu
   void menuAboutToShow(void);
   void editButton(void);
   void addButton(void);

signals:
   void toggle_view_ChannelMsgs(void);

private:
   void addMessage(QString &);
   bool eventFilter(QObject *, QEvent *);
   MyButton* newButton(const QString &name, const QString &filter,
           const QColor &color, bool bAct);
   void rightButtonPressed(void);
   void applyStyle(QString &);
   void closeEvent( QCloseEvent *e);

   bool               m_bScrollLock;
   int                m_nButtons;        // num of buttons (needed for sizing)
   int                m_nIndent;         // spaces to indent
   int                m_nLockIndex;
   int                m_nIndex;          //
   bool               m_bUseIndexing;    //
   bool               m_bShowType;
   int                m_nShown;          // num of messages shown
   int                m_nEditItem;

   QString            m_preferenceName;
   MyEdit*            m_pEdit;
   QStringList*       m_pStringList;
   QStringList        m_filterList;
   QBoxLayout*        m_pButtonsLayout;
   QCheckBox*         m_pMsgTypeCheckBox;
   QCheckBox*         m_pAdditiveCheckBox;
   QWidget*           m_pButtonsPanel;
   bool               m_bAdditiveFilter;
   QLabel*            m_pStatusBar;
   QLabel*            m_pStatusBarLock;
   QLabel*            m_pStatusBarMsgcount;
   QLabel*            m_pStatusBarTotMsgcount;
   QLabel*            m_pStatusBarFilter;
   QPopupMenu*        m_pMenu;
   MyButton*          m_pButtonOver;
   MyButtonList       m_buttonList;
};


//
// MyButton - implements a Checkbox button with a filter attached.
//            uses eventFilter to catch right click's over itself to 
//            popup a button edit dialog
//
class MyButton: public QCheckBox
{
  Q_OBJECT

public:
  MyButton(const char* name, QWidget* parent, QWidget* owner);

  const QString& name()                    { return m_name; }
  const QString& filter()                  { return m_filter; }
  const QColor&  color()                   { return m_color; }

public slots:
  void setFilter(const QString& string)  { m_filter = string; }
  void setText(const QString& string)    { QButton::setText(string); }
  void toggled(bool on)                  {  if (on) emit addFilter(m_filter);
                                           else emit delFilter(m_filter); }
  void setColor(const QColor& color)     { m_color = color; } 

signals:
  void editButton(MyButton *);
  void addFilter(const QString &);
  void delFilter(const QString &);
  void refresh();
  void setButton(MyButton* but, bool active);

protected:
  bool eventFilter(QObject *o, QEvent *e);
  void paintEvent( QPaintEvent* e);

private:
  QString m_name;
  QString m_filter;
  QColor  m_color;
  QWidget* m_pOwner;
};

//
// CButDlg - implements a configuration editing a MyButton object 
//
class CButDlg : public QDialog
{
   Q_OBJECT
 public:
   CButDlg(QWidget *parent, QString name, MyButton *but);
}; 


//
// MyEdit inherits from QMultiLineEdit and overrides some of the functionality
// to provide more what I need
//
class MyEdit : public QMultiLineEdit
{
  Q_OBJECT
public:
  MyEdit( QWidget *parent = 0, const char *name = 0);

  void pageDown(bool bSelected)  { QMultiLineEdit::pageDown(bSelected); }

protected:
  bool eventFilter(QObject *o, QEvent *e);
};
  

#endif // MSGDLG_H
