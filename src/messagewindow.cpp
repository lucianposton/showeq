/*
 * messagewindow.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sf.net/
 */

#include "messagefilterdialog.h"
#include "messagewindow.h"
#include "messagefilter.h"
#include "messages.h"
#include "main.h"

#include <qpopupmenu.h>
#include <qinputdialog.h>
#include <qfontdialog.h>
#include <qcolordialog.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qtextstream.h>

//---------------------------------------------------------------------- 
// MessageBrowser
MessageBrowser::MessageBrowser(QWidget* parent, const char* name)
  : QTextEdit(parent, name)
{
}

bool MessageBrowser::eventFilter(QObject *o, QEvent *e)
{
#if 0 // ZBTEMP
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* k = (QKeyEvent*)e;
    fprintf(stderr, "MessageBrowser::eventFilter(KeyEvent, %x)\n", k->key());
  }
#endif // ZBTEPM
  if (e->type() != QEvent::MouseButtonPress)
    return QTextEdit::eventFilter(o, e);

  QMouseEvent* m = (QMouseEvent*)e;

  if (m->button() == RightButton)
  {
    emit rightClickedMouse(m);

    return true;
  }

  return QTextEdit::eventFilter(o, e);
}

void MessageBrowser::keyPressEvent(QKeyEvent* e)
{
  //fprintf(stderr, "MessageBrowser::keyPressEvent(%x)\n", e->key());
  switch (e->key())
  {
  case Key_R:
    if (e->state() == ControlButton)
    {
      emit refreshRequest();
      return;
    }
  case Key_F:
    if (e->state() == ControlButton)
    {
      emit findRequest();
      return;
    }
  case Key_L:
    if (e->state() == ControlButton)
    {
      emit lockRequest();
      return;
    }
  };
  
  QTextEdit::keyPressEvent(e);
}

//----------------------------------------------------------------------
// MessageFindDialog
MessageFindDialog::MessageFindDialog(MessageBrowser* messageWindow, 
				     const QString& caption,
				     QWidget* parent, const char* name)
  : QDialog(parent, name, false, 0),
    m_messageWindow(messageWindow),
    m_lastParagraph(0),
    m_lastIndex(0)
{
  setCaption(caption);
  
  // setup the GUI
  QGridLayout* grid = new QGridLayout(this, 5, 2);

  // sets margin around the grid
  grid->setMargin(5);

  m_findText = new QLineEdit(this);
  m_findText->setReadOnly(false);
  connect(m_findText, SIGNAL(textChanged(const QString&)),
	  this, SLOT(textChanged(const QString&)));
  grid->addMultiCellWidget(m_findText, 0, 0, 1, 2);
  QLabel* label = new QLabel("Find &Text:", this);
  label->setBuddy(m_findText);
  grid->addWidget(label, 0, 0);
  m_matchCase = new QCheckBox("&Match case", this);
  grid->addWidget(m_matchCase, 1, 1);
  m_wholeWords = new QCheckBox("&Whole Words", this);
  grid->addWidget(m_wholeWords, 2, 1);
  m_findBackwards = new QCheckBox("Find &Backwards", this);
  grid->addWidget(m_findBackwards, 3, 1);

  QHBoxLayout* layout = new QHBoxLayout(grid);
  grid->addMultiCell(layout, 5, 5, 0, 2);
  layout->addStretch();
  m_find = new QPushButton("&Find", this);
  layout->addWidget(m_find);
  m_find->setEnabled(false);
  connect(m_find, SIGNAL(clicked()),
	  this, SLOT(find()));
  layout->addStretch();
  QPushButton* close = new QPushButton("&Close", this);
  layout->addWidget(close);
  connect(close, SIGNAL(clicked()),
	  this, SLOT(close()));
  layout->addStretch();

  // turn off resizing
  setSizeGripEnabled(false);
}

void MessageFindDialog::find()
{
  // perform a find in the message window, starting at the current position
  // using the settings from the checkboxes.
  m_messageWindow->find(m_findText->text(),
			m_matchCase->isChecked(),
			m_wholeWords->isChecked(), 
			!m_findBackwards->isChecked(), 
			0, 0);
}

void MessageFindDialog::close()
{
  // close the dialog
  QDialog::close(false);
}

void MessageFindDialog::textChanged(const QString& newText)
{
  // enable the find button iff there is text to search with
  m_find->setEnabled(!newText.isEmpty());
}

//----------------------------------------------------------------------
// MessageTypeStyle
MessageTypeStyle::MessageTypeStyle()
  : m_useDefaultFont(true)
{
}

MessageTypeStyle::MessageTypeStyle(const MessageTypeStyle& style)
  : m_color(style.color()),
    m_bgColor(style.bgColor()),
    m_font(style.font()),
    m_useDefaultFont(style.useDefaultFont())
{
}

MessageTypeStyle::~MessageTypeStyle()
{
}

MessageTypeStyle& MessageTypeStyle::operator=(const MessageTypeStyle& style)
{
  m_color = style.color();
  m_bgColor = style.bgColor();
  m_font = style.font();
  m_useDefaultFont = style.useDefaultFont();

  return *this;
}

void MessageTypeStyle::load(const QString& preferenceName, 
			    const QString& typeName)
{
  // load the preferences
  m_color = pSEQPrefs->getPrefColor(typeName + "Color", preferenceName,
				    m_color);
  m_bgColor = pSEQPrefs->getPrefColor(typeName + "BGColor", preferenceName,
				      m_bgColor);
  m_useDefaultFont = pSEQPrefs->getPrefBool(typeName + "UseDefaultFont",
					    preferenceName,
					    true);
  m_font = pSEQPrefs->getPrefFont(typeName + "Font", preferenceName,
				  QFont());
}

void MessageTypeStyle::save(const QString& preferenceName, 
			    const QString& typeName) const
{
  // save the preferences
  pSEQPrefs->setPrefColor(typeName + "Color", preferenceName, m_color);
  pSEQPrefs->setPrefColor(typeName + "BGColor", preferenceName, m_bgColor);
  pSEQPrefs->setPrefBool(typeName + "UseDefaultFont", preferenceName,
			 m_useDefaultFont);
  pSEQPrefs->setPrefFont(typeName + "Font", preferenceName, m_font);
}

//----------------------------------------------------------------------
// MessageTypeStyleDialog
MessageTypeStyleDialog::MessageTypeStyleDialog(MessageTypeStyle& style, 
					       const QColor& color, 
					       const QColor& bgColor,
					       const QString& caption, 
					       QWidget* parent, 
					       const char* name)
  :QDialog(parent, name, true, 0),
   m_style(style),
   m_defaultColor(color),
   m_defaultBGColor(bgColor)
{
  setCaption(caption);
  
  // setup the GUI
  QGridLayout* grid = new QGridLayout(this, 6, 2);

  // sets margin around the grid
  grid->setMargin(10);

  m_color = new QPushButton("...", this, "color");
  if (m_style.color().isValid())
    m_color->setPaletteBackgroundColor(m_style.color());
  else
    m_color->setPaletteBackgroundColor(m_defaultColor);
  connect(m_color, SIGNAL(clicked()),
	  this, SLOT(selectColor()));

  grid->addWidget(m_color, 0, 1);
  QLabel* label = new QLabel("&Color", this);
  label->setBuddy(m_color);
  grid->addWidget(label, 0, 0);

  m_bgColor = new QPushButton("...", this, "backgroundcolor");
  if (m_style.bgColor().isValid())
    m_bgColor->setPaletteBackgroundColor(m_style.bgColor());
  else
    m_bgColor->setPaletteBackgroundColor(m_defaultBGColor);
  connect(m_bgColor, SIGNAL(clicked()),
	  this, SLOT(selectBGColor()));

  grid->addWidget(m_bgColor, 1,1);
  label = new QLabel("&Background Color", this);
  label->setBuddy(m_bgColor);
  grid->addWidget(label, 1, 0);

  m_useDefaultFont = new QCheckBox("Use &Default Font", 
				   this, "usedefaultfont");
  m_useDefaultFont->setChecked(m_style.useDefaultFont());
  connect(m_useDefaultFont, SIGNAL(toggled(bool)),
	  this, SLOT(useDefaultFontToggled(bool)));
  grid->addWidget(m_useDefaultFont, 2, 0);
  
  m_font = new QPushButton("&Font", this, "font");
  m_font->setEnabled(!m_style.useDefaultFont());
  grid->addWidget(m_font, 2, 1);
  connect(m_font, SIGNAL(clicked()),
	  this, SLOT(selectFont()));

  grid->addRowSpacing(3, 10);

  QGroupBox* exampleBox = new QGroupBox(1, Horizontal, "Example", 
					this, "examplebox");
  grid->addMultiCellWidget(exampleBox, 4, 4, 0, 2);

  m_example = new QLabel(caption, exampleBox, "example");
  m_example->setFrameShape(QFrame::Box);
  m_example->setFrameShadow(QFrame::Sunken);
  if (m_style.color().isValid())
    m_example->setPaletteForegroundColor(m_style.color());
  else
    m_example->setPaletteForegroundColor(m_defaultColor);
  if (m_style.bgColor().isValid())
    m_example->setPaletteBackgroundColor(m_style.bgColor());
  else 
    m_example->setPaletteBackgroundColor(m_defaultBGColor);
  if (m_style.useDefaultFont())
    m_example->setFont(parent->font());
  else
    m_example->setFont(m_style.font());

  grid->addRowSpacing(5, 0);

  QHBoxLayout* layout = new QHBoxLayout(grid);
  grid->addMultiCell(layout, 6, 6, 0, 2);
  layout->addStretch();
  QPushButton* ok = new QPushButton("OK", this);
  connect(ok, SIGNAL(clicked()),
	  this, SLOT(accept()));
  layout->addWidget(ok);

  layout->addStretch();
  QPushButton* cancel = new QPushButton("Cancel", this);
  connect(cancel, SIGNAL(clicked()),
	  this, SLOT(reject()));
  cancel->setDefault(true);
  layout->addWidget(cancel);
  layout->addStretch();
}

MessageTypeStyleDialog::~MessageTypeStyleDialog()
{
}

void MessageTypeStyleDialog::useDefaultFontToggled(bool on)
{
  m_style.setUseDefaultFont(on);

  m_font->setEnabled(!on);

  if (on)
    m_example->setFont(parentWidget()->font());
  else
    m_example->setFont(m_style.font());
}

void MessageTypeStyleDialog::selectColor()
{
  QColor color = QColorDialog::getColor(m_style.color(), this, 
					caption() + " Color");
    
  if (color.isValid())
  {
    m_style.setColor(color);
    
    m_color->setPaletteBackgroundColor(m_style.color());
    
    m_example->setPaletteForegroundColor(m_style.color());
  }
}

void MessageTypeStyleDialog::selectBGColor()
{
  QColor color = QColorDialog::getColor(m_style.bgColor(), this, 
					caption() + " Background Color");
    
  if (color.isValid())
  {
    m_style.setBGColor(color);
    
    m_bgColor->setPaletteBackgroundColor(m_style.bgColor());

    m_example->setPaletteBackgroundColor(m_style.bgColor());
  }
}

void MessageTypeStyleDialog::selectFont()
{
  QFont newFont;
  bool ok = false;
  // get a new font
  newFont = QFontDialog::getFont(&ok, m_style.font(),
				 this, caption() + " Font");
  
  
  // if the user entered a font and clicked ok, set the windows font
  if (ok)
  {
    // set the styles font
    m_style.setFont(newFont);
    
    // set the example
    m_example->setFont(m_style.font());
  }
}

//----------------------------------------------------------------------
// MessageWindow
MessageWindow::MessageWindow(Messages* messages, MessageFilters* filters,
			     const QString& prefName,
			     const QString& caption,
			     QWidget* parent, const char* name)
  : SEQWindow(prefName, caption, parent, name),
    m_messages(messages),
    m_messageFilters(filters),
    m_messageWindow(0),
    m_menu(0),
    m_typeFilterMenu(0),
    m_findDialog(0),
    m_filterDialog(0),
    m_enabledTypes(0xFFFFFFFFFFFFFFFFULL),
    m_enabledShowUserFilters(0),
    m_enabledHideUserFilters(0),
    m_defaultColor(black),
    m_defaultBGColor(white),
    m_dateTimeFormat("hh:mm"),
    m_eqDateTimeFormat("ddd M/d/yyyy h:mm"),
    m_typeStyles(0),
    m_itemPattern("\022(\\d{5,7})\\w*-\\d+-\\d+-\\d+-\\d+-.{13}([^\022]+)\022"),
    m_lockedText(false),
    m_displayType(true),
    m_displayDateTime(false),
    m_displayEQDateTime(false),
    m_useTypeStyles(true),
    m_wrapText(true)
{
  m_enabledTypes = pSEQPrefs->getPrefUInt64("EnabledTypes", preferenceName(), 
					    m_enabledTypes);
  m_enabledShowUserFilters = pSEQPrefs->getPrefUInt("EnabledShowUserFilters",
						preferenceName(), 
						m_enabledShowUserFilters);
  m_enabledHideUserFilters = pSEQPrefs->getPrefUInt("EnabledHideUserFilters",
						preferenceName(), 
						m_enabledHideUserFilters);
  m_defaultColor = pSEQPrefs->getPrefColor("DefaultColor", preferenceName(),
					   m_defaultColor);
  m_defaultBGColor = pSEQPrefs->getPrefColor("DefaultBGColor", 
					     preferenceName(), 
					     m_defaultBGColor);
  m_dateTimeFormat = pSEQPrefs->getPrefString("DateTimeFormat",
					      preferenceName(), 
					      m_dateTimeFormat);
  m_eqDateTimeFormat = pSEQPrefs->getPrefString("EQDateTimeForamt",
						preferenceName(),
						m_eqDateTimeFormat);
  m_displayType = pSEQPrefs->getPrefBool("DisplayType", preferenceName(),
					 m_displayType);
  m_displayDateTime = pSEQPrefs->getPrefBool("DisplayDateTime",
					     preferenceName(), 
					     m_displayDateTime);
  m_displayEQDateTime = pSEQPrefs->getPrefBool("DisplayEQDateTime",
					       preferenceName(),
					       m_displayEQDateTime);
  m_useTypeStyles = pSEQPrefs->getPrefBool("UseTypeStyles", preferenceName(),
				      m_useTypeStyles);
  m_wrapText = pSEQPrefs->getPrefBool("WrapText", preferenceName(),
				      m_wrapText);

  // allocate the array of type styles
  m_typeStyles = new MessageTypeStyle[MT_Max+1];

  // create the window for text display
  m_messageWindow = new MessageBrowser(this, "messageText");

  // make the message window the main widget of the SEQWindow
  setWidget(m_messageWindow);
  
  // set the message window frame style
  m_messageWindow->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  // set the current font
  m_messageWindow->setCurrentFont(font());

  // set the colors
  m_messageWindow->setColor(m_defaultColor);
  m_messageWindow->setPaper(m_defaultBGColor);

  // make sure history isn't kept
  m_messageWindow->setUndoDepth(0);
  m_messageWindow->setUndoRedoEnabled(false);

  m_messageWindow->setTextFormat(PlainText);

  // set it to read only
  m_messageWindow->setReadOnly(true);

  // set the word wrap 
  m_messageWindow->setWordWrap(m_wrapText ? 
			       QTextEdit::WidgetWidth : QTextEdit::NoWrap);

  // set the wrap policy to break at space
  m_messageWindow->setWrapPolicy(QTextEdit::AtWhiteSpace);

  // connect to the Messages signal(s)
  connect(m_messages, SIGNAL(newMessage(const MessageEntry&)),
	  this, SLOT(newMessage(const MessageEntry&)));
  connect(m_messages, SIGNAL(cleared(void)),
	  this, SLOT(refreshMessages(void)));

  // connect to the message filters signals
  connect(m_messageFilters, SIGNAL(removed(uint32_t, uint8_t)),
	  this, SLOT(removedFilter(uint32_t, uint8_t)));
  connect(m_messageFilters, SIGNAL(added(uint32_t, uint8_t, 
					 const MessageFilter&)),
	  this, SLOT(addedFilter(uint32_t, uint8_t, const MessageFilter&)));

  // connect up our own signals
  connect(m_messageWindow, SIGNAL(rightClickedMouse(QMouseEvent*)),
	  this, SLOT(mousePressEvent(QMouseEvent*)));
  connect(m_messageWindow, SIGNAL(refreshRequest()),
	  this, SLOT(refreshMessages()));
  connect(m_messageWindow, SIGNAL(findRequest()),
	  this, SLOT(findDialog()));
  connect(m_messageWindow, SIGNAL(lockRequest()),
	  this, SLOT(toggleLockedText()));

  m_menu = new QPopupMenu;
  QPopupMenu* typeStyleMenu = new QPopupMenu;

  m_typeFilterMenu = new QPopupMenu;
  m_menu->insertItem("Message &Type Filter - Show", m_typeFilterMenu);

  m_typeFilterMenu->insertItem("&Enable All", 
			       this, SLOT(enableAllTypeFilters()), 0, 64);
  m_typeFilterMenu->insertItem("&Disable All", 
			       this, SLOT(disableAllTypeFilters()), 0, 65);
  
  m_typeFilterMenu->insertSeparator(-1);

  QString typeName;
  // iterate over the message types, filling in various menus and getting 
  // font color preferences
  for (int i = MT_Guild; i <= MT_Max; i++)
  {
    typeName = MessageEntry::messageTypeString((MessageType)i);
    if (!typeName.isEmpty())
    {
      m_typeFilterMenu->insertItem(typeName, i);
      m_typeFilterMenu->setItemChecked(i, (((uint64_t(1) << i) & m_enabledTypes) != 0));
      typeStyleMenu->insertItem(typeName + "...", i);

      m_typeStyles[i].load(preferenceName(), typeName);
    }
  }
  
  connect(m_typeFilterMenu, SIGNAL(activated(int)),
	  this, SLOT(toggleTypeFilter(int)));
  connect(typeStyleMenu, SIGNAL(activated(int)),
	  this, SLOT(setTypeStyle(int)));

  m_showUserFilterMenu = new QPopupMenu;
  m_menu->insertItem("User Message Filter - &Show", m_showUserFilterMenu);

  m_showUserFilterMenu->insertItem("&Enable All", 
			       this, SLOT(enableAllShowUserFilters()), 0, 66);
  m_showUserFilterMenu->insertItem("&Disable All", 
			       this, SLOT(disableAllShowUserFilters()), 0, 67);
  m_showUserFilterMenu->insertSeparator(-1);

  m_hideUserFilterMenu = new QPopupMenu;
  m_menu->insertItem("User Message Filter - &Hide", m_hideUserFilterMenu);

  m_hideUserFilterMenu->insertItem("&Enable All", 
			       this, SLOT(enableAllHideUserFilters()), 0, 66);
  m_hideUserFilterMenu->insertItem("&Disable All", 
			       this, SLOT(disableAllHideUserFilters()), 0, 67);
  m_hideUserFilterMenu->insertSeparator(-1);

  const MessageFilter* filter;
  for(int i = 0; i < maxMessageFilters; i++)
  {
    filter = m_messageFilters->filter(i);
    if (filter)
    {
      m_showUserFilterMenu->insertItem(filter->name(), i);
      m_showUserFilterMenu->setItemChecked(i, (1 << i) & m_enabledShowUserFilters);

      m_hideUserFilterMenu->insertItem(filter->name(), i);
      m_hideUserFilterMenu->setItemChecked(i, (1 << i) & m_enabledHideUserFilters);
    }
  }

  connect(m_showUserFilterMenu, SIGNAL(activated(int)),
	  this, SLOT(toggleShowUserFilter(int)));
  connect(m_hideUserFilterMenu, SIGNAL(activated(int)),
	  this, SLOT(toggleHideUserFilter(int)));
  m_menu->insertItem("Edit User &Message Filters...", 
		     this, SLOT(messageFilterDialog()));

  m_menu->insertSeparator(-1);
  m_menu->insertItem("&Find...", this, SLOT(findDialog()), 
		     CTRL+Key_F);
  m_menu->insertSeparator(-1);
  m_id_lockText = m_menu->insertItem("&Lock Text", this,
				     SLOT(toggleLockedText()), CTRL+Key_L);
  int x;
  x = m_menu->insertItem("Refresh Messages...", this, SLOT(refreshMessages()),
			 CTRL+Key_R);
  m_menu->insertItem("Save Message Text...", this, SLOT(saveText()),
		     CTRL+Key_S);
  m_menu->insertSeparator(-1);
  m_menu->setItemChecked(x, m_lockedText);
  m_menu->insertSeparator(-1);
  x = m_menu->insertItem("&Display Type", this, SLOT(toggleDisplayType(int)));
  m_menu->setItemChecked(x, m_displayType);
  x = m_menu->insertItem("Display T&ime/Date", this,
			 SLOT(toggleDisplayTime(int)));
  m_menu->setItemChecked(x, m_displayDateTime);
  x = m_menu->insertItem("Display &EQ Date/Time", this,
			 SLOT(toggleEQDisplayTime(int)));
  m_menu->setItemChecked(x, m_displayEQDateTime);
  x = m_menu->insertItem("&Use Type Styles", this,
			 SLOT(toggleUseTypeStyles(int)));
  m_menu->setItemChecked(x, m_useTypeStyles);
  x = m_menu->insertItem("&Wrap Text", this, SLOT(toggleWrapText(int)));
  m_menu->setItemChecked(x, m_wrapText);
  m_menu->insertItem("Fo&nt...", this, SLOT(setFont()));
  m_menu->insertItem("&Caption...", this, SLOT(setCaption()));
  m_menu->insertItem("Text Colo&r...", this, SLOT(setColor()));
  m_menu->insertItem("Text Back&ground Color...", this, SLOT(setBGColor()));
  m_menu->insertItem("Type St&yles", typeStyleMenu);

  refreshMessages();
}

MessageWindow::~MessageWindow()
{
  delete [] m_typeStyles;
}

QPopupMenu* MessageWindow::menu()
{
  return m_menu;
}

void MessageWindow::addMessage(const MessageEntry& message)
{
  MessageType type = message.type();

  // if the message type isn't enabled, nothing to do
  if ((((m_enabledTypes & ( uint64_t(1) << type)) == 0) &&
       ((m_enabledShowUserFilters & message.filterFlags()) == 0)) ||
      ((m_enabledHideUserFilters & message.filterFlags()) != 0))
    return;
  
  QString text;

  // if displaying the type, add it
  if (m_displayType)
    text = MessageEntry::messageTypeString(message.type()) + ": ";
    
  // if displaying the message date/time append it
  if (m_displayDateTime)
    text += message.dateTime().toString(m_dateTimeFormat) + " - ";

  // if displaying the messages eq date/time, append it
  if (m_displayEQDateTime && (message.eqDateTime().isValid()))
    text += message.eqDateTime().toString(m_eqDateTimeFormat) + " - ";

  // append the actual message text
  text += message.text();

  text.replace(m_itemPattern, "\\2 (#\\1)");

  // now append the message text to the buffer
  m_messageWindow->append(text);
}

void MessageWindow::addColorMessage(const MessageEntry& message)
{
  MessageType type = message.type();

  // if the message type isn't enabled, nothing to do
  if ((((m_enabledTypes & ( uint64_t(1) << type)) == 0) &&
       ((m_enabledShowUserFilters & message.filterFlags()) == 0)) ||
      ((m_enabledHideUserFilters & message.filterFlags()) != 0))
    return;

  // if the message has a specific color, then use it
  if (message.color() != ME_InvalidColor)
    m_messageWindow->setColor(QColor(message.color()));
  else if (m_typeStyles[type].color().isValid()) // or use the types color
    m_messageWindow->setColor(m_typeStyles[type].color());
  else // otherwise use the default color
    m_messageWindow->setColor(m_defaultColor);

  if (m_typeStyles[type].useDefaultFont())
    m_messageWindow->setCurrentFont(font());
  else
    m_messageWindow->setCurrentFont(m_typeStyles[type].font());

  QString text;

  // if displaying the type, add it
  if (m_displayType)
    text = MessageEntry::messageTypeString(type) + ": ";
    
  // if displaying the message date/time append it
  if (m_displayDateTime)
    text += message.dateTime().toString(m_dateTimeFormat) + " - ";

  // if displaying the messages eq date/time, append it
  if (m_displayEQDateTime && (message.eqDateTime().isValid()))
    text += message.eqDateTime().toString(m_eqDateTimeFormat) + " - ";

  // append the actual message text
  text += message.text();

  text.replace(m_itemPattern, "\\2 (#\\1)");

  // now append the message text to the buffer
  m_messageWindow->append(text);

  int para = m_messageWindow->paragraphs() - 1;
  if (m_typeStyles[type].bgColor().isValid())
    m_messageWindow->setParagraphBackgroundColor(para, 
						 m_typeStyles[type].bgColor());
  else
    m_messageWindow->setParagraphBackgroundColor(para, m_defaultBGColor);
}

void MessageWindow::newMessage(const MessageEntry& message)
{
  // if text is locked, nothing to do
  if (m_lockedText)
    return;

  if (m_useTypeStyles)
    addColorMessage(message);
  else
    addMessage(message);
}

void MessageWindow::refreshMessages(void)
{
  // get the list of messages
  const MessageList& messages = m_messages->messageList();
 
  // set the IBeam Cursor for easier text selection
  setCursor(Qt::WaitCursor);
  m_messageWindow->setCursor(Qt::WaitCursor);

  // clear the document
  m_messageWindow->clear();

  // turn off updates while doing this mass update
  m_messageWindow->setUpdatesEnabled(false);
  setUpdatesEnabled(false);

  m_messageWindow->append(" ");

  // set the cursor to the beginning of the document
  m_messageWindow->setCursorPosition(0, 0);

  // move the cursor to the end of the document
  m_messageWindow->moveCursor(QTextEdit::MoveEnd, false);

  // iterate over the message list and add the messages
  MessageList::const_iterator it;
  int i;
  if (m_useTypeStyles)
    for (i = 0, it = messages.begin(); it != messages.end(); ++it, ++i)
      addColorMessage(*it); // append the message with color
  else 
    for (i = 0, it = messages.begin(); it != messages.end(); ++it, ++i)
      addMessage(*it); // append the message plain
    
  // turn updates back on 
  m_messageWindow->setUpdatesEnabled(true);
  setUpdatesEnabled(true);

  // repain window now that updates have been re-enabled
  repaint();

  // set the IBeam Cursor for easier text selection
  unsetCursor();
  m_messageWindow->unsetCursor();

  // move the cursor to the end of the document
  m_messageWindow->moveCursor(QTextEdit::MoveEnd, false);

  // move the cursor to the end of the document
  m_messageWindow->ensureCursorVisible();
}

void MessageWindow::findDialog(void)
{
  // create the find dialog, if necessary
  if (!m_findDialog)
    m_findDialog = new MessageFindDialog(m_messageWindow, caption() + " Find",
					 this, "messagefinddialog");

  // show the find dialog
  m_findDialog->show();
}

void MessageWindow::messageFilterDialog(void)
{
  // create the filter dialog, if necessary
  if (!m_filterDialog)
    m_filterDialog = new MessageFilterDialog(m_messageFilters, 
					     caption() + " Message Filters",
					     this, "messagefilterdialog");

  // show the message filter dialog
  m_filterDialog->show();
}

void MessageWindow::saveText(void)
{
  QString fileName = 
    QFileDialog::getSaveFileName("", "*.txt", this,
				 "ShowEQ - Message Text File");

  if (fileName.isEmpty())
    return;

  QFile file( fileName ); // Write the text to a file
  if ( file.open( IO_WriteOnly ) ) 
  {
    QTextStream stream( &file );

    // save all the paragraphs
    //   ZBNOTE: unfortunately just using ->text() doesn't work.
    for (int i = 0; i < m_messageWindow->paragraphs(); i++)
      stream << m_messageWindow->text(i) << endl;
  }
}

void MessageWindow::toggleTypeFilter(int id)
{
  // toggle whether the message type is shown or not
  if (((uint64_t(1) << id) & m_enabledTypes) != 0)
    m_enabledTypes &= ~(uint64_t(1) << id);
  else
    m_enabledTypes |= (uint64_t(1) << id);

  // save the new setting
  pSEQPrefs->setPrefUInt64("EnabledTypes", preferenceName(), m_enabledTypes);

  // (un)check the appropriate menu item
  m_typeFilterMenu->setItemChecked(id, ((m_enabledTypes & (uint64_t(1) << id)) != 0));
}

void MessageWindow::disableAllTypeFilters()
{
  // set and save all message types disabled
  m_enabledTypes = 0;
  pSEQPrefs->setPrefUInt64("EnabledTypes", preferenceName(), m_enabledTypes);

  // make sure the All menu items are unchecked
  m_typeFilterMenu->setItemChecked(64, false);
  m_typeFilterMenu->setItemChecked(65, false);

  // uncheck all the menu items
  QString typeName;
  for (int i = MT_Guild; i <= MT_Max; i++)
  {
    typeName = MessageEntry::messageTypeString((MessageType)i);
    if (!typeName.isEmpty())
      m_typeFilterMenu->setItemChecked(i, false);
  }
}

void MessageWindow::enableAllTypeFilters()
{
  // set and save all message types enabled
  m_enabledTypes = 0xFFFFFFFFFFFFFFFFULL;
  pSEQPrefs->setPrefUInt64("EnabledTypes", preferenceName(), m_enabledTypes);

  // make sure the All menu items are unchecked
  m_typeFilterMenu->setItemChecked(64, false);
  m_typeFilterMenu->setItemChecked(65, false);

  // check all the menu items
  QString typeName;
  for (int i = MT_Guild; i <= MT_Max; i++)
  {
    typeName = MessageEntry::messageTypeString((MessageType)i);
    if (!typeName.isEmpty())
      m_typeFilterMenu->setItemChecked(i, true);
  }
}

void MessageWindow::toggleShowUserFilter(int id)
{
  // toggle whether the filter is enabled/disabled
  if (((1 << id) & m_enabledShowUserFilters) != 0)
    m_enabledShowUserFilters &= ~(1 << id);
  else
    m_enabledShowUserFilters |= (1 << id);

  // save the new setting
  pSEQPrefs->setPrefUInt("EnabledShowUserFilters", preferenceName(), 
			 m_enabledShowUserFilters);
 
  // (un)check the appropriate menu item
  m_showUserFilterMenu->setItemChecked(id, 
				   ((m_enabledShowUserFilters & (1 << id)) != 0));
}

void MessageWindow::disableAllShowUserFilters()
{
  // set and save all filters disabled setting
  m_enabledShowUserFilters = 0;
  pSEQPrefs->setPrefUInt("EnabledShowUserFilters", preferenceName(), 
			 m_enabledShowUserFilters);
  
  // make sure the All menu items are unchecked
  m_showUserFilterMenu->setItemChecked(66, false);
  m_showUserFilterMenu->setItemChecked(67, false);

  // uncheck all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_showUserFilterMenu->setItemChecked(i, false);
  }
}

void MessageWindow::enableAllShowUserFilters()
{
  // set and save all filters enabled flag
  m_enabledShowUserFilters = 0xFFFFFFFF;
  pSEQPrefs->setPrefUInt("EnabledShowUserFilters", preferenceName(), 
			 m_enabledShowUserFilters);

  // make sure the All menu items are unchecked
  m_showUserFilterMenu->setItemChecked(66, false);
  m_showUserFilterMenu->setItemChecked(67, false);

  // check all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_showUserFilterMenu->setItemChecked(i, true);
  }
}

void MessageWindow::toggleHideUserFilter(int id)
{
  // toggle whether the filter is enabled/disabled
  if (((1 << id) & m_enabledHideUserFilters) != 0)
    m_enabledHideUserFilters &= ~(1 << id);
  else
    m_enabledHideUserFilters |= (1 << id);

  // save the new setting
  pSEQPrefs->setPrefUInt("EnabledHideUserFilters", preferenceName(), 
			 m_enabledHideUserFilters);
 
  // (un)check the appropriate menu item
  m_hideUserFilterMenu->setItemChecked(id, 
				   ((m_enabledHideUserFilters & (1 << id)) != 0));
}

void MessageWindow::disableAllHideUserFilters()
{
  // set and save all filters disabled setting
  m_enabledHideUserFilters = 0;
  pSEQPrefs->setPrefUInt("EnabledHideUserFilters", preferenceName(), 
			 m_enabledHideUserFilters);
  
  // make sure the All menu items are unchecked
  m_hideUserFilterMenu->setItemChecked(66, false);
  m_hideUserFilterMenu->setItemChecked(67, false);

  // uncheck all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_hideUserFilterMenu->setItemChecked(i, false);
  }
}

void MessageWindow::enableAllHideUserFilters()
{
  // set and save all filters enabled flag
  m_enabledHideUserFilters = 0xFFFFFFFF;
  pSEQPrefs->setPrefUInt("EnabledHideUserFilters", preferenceName(), 
			 m_enabledHideUserFilters);

  // make sure the All menu items are unchecked
  m_hideUserFilterMenu->setItemChecked(66, false);
  m_hideUserFilterMenu->setItemChecked(67, false);

  // check all the menu items
  QString typeName;
  for (int i = 0; i <= maxMessageFilters; i++)
  {
    if (m_messageFilters->filter(i))
      m_hideUserFilterMenu->setItemChecked(i, true);
  }
}

void MessageWindow::toggleLockedText()
{
  m_lockedText = !m_lockedText;
  m_menu->setItemChecked(m_id_lockText, m_lockedText);

  // if the text had been locked, refresh the messages
  if (!m_lockedText)
    refreshMessages();
}

void MessageWindow::toggleDisplayType(int id)
{
  m_displayType = !m_displayType;
  m_menu->setItemChecked(id, m_displayType);

  pSEQPrefs->setPrefBool("DisplayType", preferenceName(), m_displayType);
}

void MessageWindow::toggleDisplayTime(int id)
{
  m_displayDateTime = !m_displayDateTime;
  m_menu->setItemChecked(id, m_displayDateTime);
  pSEQPrefs->setPrefBool("DisplayDateTime", preferenceName(),
			 m_displayDateTime);
}

void MessageWindow::toggleEQDisplayTime(int id)
{
  m_displayEQDateTime = ! m_displayEQDateTime;
  m_menu->setItemChecked(id, m_displayEQDateTime);
  pSEQPrefs->setPrefBool("DisplayEQDateTime", preferenceName(),
			 m_displayEQDateTime);
}

void MessageWindow::toggleUseTypeStyles(int id)
{
  m_useTypeStyles = !m_useTypeStyles;
  m_menu->setItemChecked(id, m_useTypeStyles);
  pSEQPrefs->setPrefBool("UseTypeStyles", preferenceName(), m_useTypeStyles);
}

void MessageWindow::toggleWrapText(int id)
{
  m_wrapText = !m_wrapText;
  m_menu->setItemChecked(id, m_wrapText);

  pSEQPrefs->setPrefBool("WrapText", preferenceName(), m_wrapText);

  // set the wrap policy according to the setting
  m_messageWindow->setWordWrap(m_wrapText ? 
			       QTextEdit::WidgetWidth : QTextEdit::NoWrap);
}

void MessageWindow::setTypeStyle(int id)
{
  // Create the dialog object
  QString typeName = MessageEntry::messageTypeString((MessageType)id);
  QString styleCaption = caption() + " - " + typeName + " Style";
  MessageTypeStyleDialog dialog(m_typeStyles[id],
				m_defaultColor, m_defaultBGColor,
				styleCaption, 
				this, styleCaption);

  // popup the modal dialog
  int result = dialog.exec();

  // if the user accepted the changes, then apply them to the style
  if (result == QDialog::Accepted)
  {
    // apply the style
    m_typeStyles[id] = dialog.style();

    // save the updates
    m_typeStyles[id].save(preferenceName(), typeName);
  }
}

void MessageWindow::setColor()
{
  QString clrCaption = caption() + " Default Text Color";
  
  // get a new color
  QColor color = QColorDialog::getColor(m_defaultColor, this, clrCaption);

  // if the user clicked ok, use/save the preference
  if (color.isValid())
  {
    m_defaultColor = color;

    pSEQPrefs->setPrefColor("DefaultColor", preferenceName(), 
			    m_defaultColor);
  }
}

void MessageWindow::setBGColor()
{
  QString clrCaption = caption() + " Default Text Background Color";

  // get a new background color
  QColor color = QColorDialog::getColor(m_defaultBGColor, this, clrCaption);

  // if the user clicked ok, use/save the preference
  if (color.isValid())
  {
    m_defaultBGColor = color;
    m_messageWindow->setPaper(m_defaultBGColor);

    pSEQPrefs->setPrefColor("DefaultBGColor", preferenceName(), 
			    m_defaultBGColor);
  }
}

void MessageWindow::setFont()
{
  QFont newFont;
  bool ok = false;

  // get a new font
  newFont = QFontDialog::getFont(&ok, m_messageWindow->currentFont(),
				 this, caption() + " Font");
  
  
  // if the user entered a font and clicked ok, set the windows font
  if (ok)
    setWindowFont(newFont);
}

void MessageWindow::setCaption()
{
  bool ok = false;

  QString captionText = 
    QInputDialog::getText("ShowEQ Message Window Caption",
			  "Enter caption for this Message Window:",
			  QLineEdit::Normal, caption(),
			  &ok, this);
  
  // if the user entered a caption and clicked ok, set the windows caption
  if (ok)
    SEQWindow::setCaption(captionText);
}

void MessageWindow::restoreFont()
{
  // restore the font
  SEQWindow::restoreFont();
  
  // set the message windows font to match
  if (m_messageWindow)
    m_messageWindow->setCurrentFont(font());
}

void MessageWindow::removedFilter(uint32_t mask, uint8_t filter)
{
  // remove the user filter menu item
  m_showUserFilterMenu->removeItem(filter);
  m_hideUserFilterMenu->removeItem(filter);
  
  // if all filters are enabled, don't unselect it
  if (m_enabledShowUserFilters != 0xFFFFFFFF)
  {
    // remove the filter from the enabled filters list
    m_enabledShowUserFilters &= ~mask;

    // update the preference
    pSEQPrefs->setPrefUInt("EnabledShowUserFilters", preferenceName(), 
			   m_enabledShowUserFilters);
  }
  
  // if all filters are enabled, don't unselect it
  if (m_enabledHideUserFilters != 0xFFFFFFFF)
  {
    // remove the filter from the enabled filters list
    m_enabledHideUserFilters &= ~mask;

    // update the preference
    pSEQPrefs->setPrefUInt("EnabledHideUserFilters", preferenceName(), 
			   m_enabledHideUserFilters);
  }
}

void MessageWindow::addedFilter(uint32_t mask, uint8_t filterid, 
				const MessageFilter& filter)
{
  // insert a user filter menu item for the new filter
  m_showUserFilterMenu->insertItem(filter.name(), filterid);

  // insert a user filter menu item for the new filter
  m_hideUserFilterMenu->insertItem(filter.name(), filterid);
}

