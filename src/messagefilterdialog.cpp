/*
 * messagefilterdialog.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sf.net/
 *
 */

#include "messagefilterdialog.h"
#include "messagefilter.h"
#include "message.h"

#include <stdint.h>
#include <stdio.h>

#include <qstring.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>

//----------------------------------------------------------------------
// MessageFilterListBoxText
class MessageFilterListBoxText : public QListBoxText
{
public:
  MessageFilterListBoxText(QListBox * listbox, 
			   const QString & text = QString::null, 
			   uint32_t data = 0);
  MessageFilterListBoxText(QListBox * listbox, QListBoxItem* after,
			   const QString & text = QString::null, 
			   uint32_t data = 0);
  virtual ~MessageFilterListBoxText();

  uint32_t data() { return m_data; }
  void setData(uint32_t data) { m_data = data; }

protected:
  uint32_t m_data;
};

MessageFilterListBoxText::MessageFilterListBoxText(QListBox * listbox, 
						   const QString & text, 
						   uint32_t data)
  : QListBoxText(listbox, text),
    m_data(data)
{
}

MessageFilterListBoxText::MessageFilterListBoxText(QListBox* listbox, 
						   QListBoxItem* after,
						   const QString& text, 
						   uint32_t data)
  : QListBoxText(listbox, text, after),
    m_data(data)
{
}

MessageFilterListBoxText::~MessageFilterListBoxText()
{
}

//----------------------------------------------------------------------
// MessageFilterDialog
MessageFilterDialog::MessageFilterDialog(MessageFilters* filters, 
					 const QString& caption,
					 QWidget* parent,
					 const char* name)
  : QDialog(parent, name, false, WType_Dialog),
    m_filters(filters), 
    m_currentFilterNum(0xFF),
    m_currentFilter(0)
{
  // set the caption
  setCaption(caption);

  // don't support resizing the dialog
  setSizeGripEnabled(false);

  // connect to the MessageFilter signals
  connect(m_filters, SIGNAL(removed(uint32_t, uint8_t)),
	  this, SLOT(removedFilter(uint32_t, uint8_t)));
  connect(m_filters, SIGNAL(added(uint32_t, uint8_t, 
				  const MessageFilter&)),
	  this, SLOT(addedFilter(uint32_t, uint8_t, const MessageFilter&)));

  // setup the dialog
  QVBoxLayout* outerLayout = new QVBoxLayout(this, 5, -1, "outerlayout");
  QHBoxLayout* columnLayout = new QHBoxLayout(outerLayout, -1, "columns");
  QVBoxLayout* column1Layout = new QVBoxLayout(5, "column1");
  columnLayout->addLayout(column1Layout, 1);

  // layout 1st column
  QLabel* label = new QLabel("&Existing Filters", this);
  column1Layout->addWidget(label, 1, AlignCenter);

  m_existingFilters = new QListBox(this, "existingfilters");
  column1Layout->addWidget(m_existingFilters, 10);
  label->setBuddy(m_existingFilters);
  m_existingFilters->setSelectionMode(QListBox::Single);
  connect(m_existingFilters, SIGNAL(selectionChanged(QListBoxItem*)),
	  this, SLOT(existingFilterSelectionChanged(QListBoxItem*))); 

  m_new = new QPushButton("Ne&w", this);
  column1Layout->addWidget(m_new, 1, AlignCenter);
  connect(m_new, SIGNAL(clicked()),
	  this, SLOT(newFilter()));

  m_filterGroup = new QGroupBox(1, Vertical, 
				"New &Filter", this, "filtergroup");
  columnLayout->addWidget(m_filterGroup, 5);

  QFrame* dummy = new QFrame(m_filterGroup, "dummy");

  QGridLayout* filterLayout = new QGridLayout(dummy, 8, 3, 5, -1, "filterlayout");
  
  label = new QLabel("&Name", dummy);
  filterLayout->addWidget(label, 0, 0, AlignLeft | AlignVCenter);
  m_name = new QLineEdit(dummy, "name");
  filterLayout->addMultiCellWidget(m_name, 0, 0, 1, 2);
  label->setBuddy(m_name);
  connect(m_name, SIGNAL(textChanged(const QString&)),
	  this, SLOT(anyTextChanged(const QString&)));

  label = new QLabel("&Pattern", dummy);
  filterLayout->addWidget(label, 1, 0, AlignLeft | AlignVCenter);
  m_pattern = new QLineEdit(dummy, "pattern");
  filterLayout->addMultiCellWidget(m_pattern, 1, 1, 1, 2);
  label->setBuddy(m_pattern);
  connect(m_pattern, SIGNAL(textChanged(const QString&)),
	  this, SLOT(anyTextChanged(const QString&)));

  label = new QLabel("&Message Types", dummy);
  filterLayout->addWidget(label, 2, 0, AlignLeft | AlignVCenter);
  m_messageTypes = new QListBox(dummy, "messagetypes");
  filterLayout->addMultiCellWidget(m_messageTypes, 2, 6, 1, 2);
  label->setBuddy(m_messageTypes);
  m_messageTypes->setSelectionMode(QListBox::Multi);
  connect(m_messageTypes, SIGNAL(selectionChanged()),
	  this, SLOT(messageTypeSelectionChanged()));

  m_delete = new QPushButton("&Delete", dummy);
  filterLayout->addWidget(m_delete, 7, 0, AlignCenter);
  m_delete->setEnabled(false);
  connect(m_delete, SIGNAL(clicked()),
	  this, SLOT(deleteFilter()));

  m_update = new QPushButton("&Update", dummy);
  filterLayout->addWidget(m_update, 7, 1, AlignCenter);
  m_update->setEnabled(false);
  connect(m_update, SIGNAL(clicked()),
	  this, SLOT(updateFilter()));

  m_add = new QPushButton("&Add", dummy);
  filterLayout->addWidget(m_add, 7, 2, AlignCenter);
  m_add->setEnabled(false);
  connect(m_add, SIGNAL(clicked()),
	  this, SLOT(addFilter()));

  QPushButton* close = new QPushButton("&Close", this);
  outerLayout->addWidget(close, 1, AlignCenter);
  connect(close, SIGNAL(clicked()),
	  this, SLOT(accept()));

  // fill in message types
  QString typeName;
  for (int i = MT_Guild; i <= MT_Max; i++)
  {
    typeName = MessageEntry::messageTypeString((MessageType)i);
    if (!typeName.isEmpty())
      (void)new MessageFilterListBoxText(m_messageTypes, typeName, i);
  }

  // fill in existing messages
  const MessageFilter* filter;
  for (int i = 0; i < maxMessageFilters; i++)
  {
    filter = m_filters->filter(i);
    if (filter)
      (void)new MessageFilterListBoxText(m_existingFilters, filter->name(), i);
  }
}

MessageFilterDialog::~MessageFilterDialog()
{
}

void MessageFilterDialog::newFilter()
{
  // clear any selection
  m_existingFilters->clearSelection();

  // clear the filter display
  clearFilter();

  // check the current state and set UI up accordingly
  checkState();
}

void MessageFilterDialog::addFilter()
{
  uint32_t type;
  uint64_t types = 0;

  // iterate over the message types
  for (QListBoxItem* currentLBT = m_messageTypes->firstItem();
       currentLBT;
       currentLBT = currentLBT->next())
  {
    // if the item isn't selected, add in its type flag, and enable updates
    if (currentLBT->isSelected())
    {
      // get the message type of the selected item
      type = ((MessageFilterListBoxText*)currentLBT)->data();

      // add its flag to the types 
      types |= (uint64_t(1) << type);
    }
  } 

  // create a message filter object
  MessageFilter newFilter(m_name->text(), types, m_pattern->text());

  // if this isn't a valid filter, don't create it
  if (!newFilter.valid())
    return;

  // add the new filter
  m_currentFilterNum = m_filters->addFilter(newFilter);
  
  // if it is a valid filter, make the new filter the current selection
  if (m_currentFilterNum != 0xFF)
  {
    // retrieve the current item
    m_currentFilter = m_filters->filter(m_currentFilterNum);

    // iterate over the existing filters
    for (QListBoxItem* currentLBT = m_existingFilters->firstItem();
	 currentLBT;
	 currentLBT = currentLBT->next())
    {
      // find the current filter
      if (((MessageFilterListBoxText*)currentLBT)->data() == m_currentFilterNum)
      {
	// make the current filter the selected filter
	m_existingFilters->setSelected(currentLBT, true);
	break;
      }
    }
  }
  else // clear the current filter
  {
    // clear the current filter
    m_currentFilter = 0;
    clearFilter();
  }
  
  // setup the current dialog state
  checkState();
}

void MessageFilterDialog::updateFilter()
{
  // delete the old filter
  if (m_currentFilter)
    m_filters->remFilter(*m_currentFilter);

  // add in a new filter
  addFilter();
}

void MessageFilterDialog::deleteFilter()
{
  // remove the current filter (if any are selected)
  if (m_currentFilter)
    m_filters->remFilter(*m_currentFilter);

  // clear any selection
  m_existingFilters->clearSelection();

  // clear the filter display
  clearFilter();

  // check the current state and set UI up accordingly
  checkState();
}

void MessageFilterDialog::anyTextChanged(const QString& newText)
{
  // check the state whenever any text changes
  checkState();
}

void MessageFilterDialog::messageTypeSelectionChanged()
{
  // check the state whenever the message type selection changed
  checkState();
}

void MessageFilterDialog::existingFilterSelectionChanged(QListBoxItem * item)
{
  if (item)
  {
    // get the current filter number from the listbox item
    m_currentFilterNum = ((MessageFilterListBoxText*)item)->data();
    
    // get the specified filter
    m_currentFilter = m_filters->filter(m_currentFilterNum);

    // set the GroupBox's label
    m_filterGroup->setTitle(m_currentFilter->name() + " &Filter");

    // setup all the filter values
    m_name->setText(m_currentFilter->name());
    m_pattern->setText(m_currentFilter->regexp().pattern());

    // select all the message types
    uint64_t messageTypes = m_currentFilter->types();
    uint32_t messageType;
    for (QListBoxItem* currentLBT = m_messageTypes->firstItem();
	 currentLBT;
	 currentLBT = currentLBT->next())
    {
      messageType = ((MessageFilterListBoxText*)currentLBT)->data();
      m_messageTypes->setSelected(currentLBT, ((uint64_t(1) << messageType) & messageTypes) != 0);
    }
  }
  else // no item selected, clear all filter setup
    clearFilter();

  // check the current state
  checkState();
}

void MessageFilterDialog::removedFilter(uint32_t mask, uint8_t filter)
{
  // iterate over all the existing filters
  for (QListBoxItem* currentLBT = m_existingFilters->firstItem();
       currentLBT;
       currentLBT = currentLBT->next())
  {
    // check if this is the removed filter
    if (((MessageFilterListBoxText*)currentLBT)->data() == filter)
    {
      // delete the removed filter's list box item
      delete currentLBT;

      // nothing more to do
      break;
    }
  }
}

void MessageFilterDialog::addedFilter(uint32_t mask, uint8_t filterid, 
				      const MessageFilter& filter)
{
  if (m_existingFilters->count() == 0)
  {
      // add the new message filter 
      new MessageFilterListBoxText(m_existingFilters, 0,
				   filter.name(), filterid);

  }

  // iterate over all the existing filters
  for (QListBoxItem* currentLBT = m_existingFilters->firstItem();
       currentLBT;
       currentLBT = currentLBT->next())
  {
    // check if this is the removed filter
    if (((MessageFilterListBoxText*)currentLBT)->data() > filterid)
    {
      // add a new message filter at the appropriate location
      //   NOTE: This maintains list order during an item update
      new MessageFilterListBoxText(m_existingFilters, currentLBT->prev(),
				   filter.name(), filterid);

      break;
    }
  }
}

void MessageFilterDialog::clearFilter()
{
  // set filter information to default state
  m_currentFilterNum = 0xFF;
  m_currentFilter = 0;
  m_filterGroup->setTitle("New &Filter");
  m_name->setText("");
  m_pattern->setText("");
  m_messageTypes->clearSelection();
}

void MessageFilterDialog::checkState()
{
  bool update = false;
  bool add = false;

  // the state check varies depending on if their is a current filter or not
  if (m_currentFilter)
  {
    uint32_t type;
    uint64_t types = 0;
    
    // buttons should only be enabled for valid message filter content
    if (!m_name->text().isEmpty() &&
	!m_pattern->text().isEmpty() &&
	QRegExp(m_pattern->text()).isValid())
    {
      // iterate over all the message types
      for (QListBoxItem* currentLBT = m_messageTypes->firstItem();
	   currentLBT;
	   currentLBT = currentLBT->next())
      {
	// is the current item selected
	if (currentLBT->isSelected())
	{
	  // get the items message type
	  type = ((MessageFilterListBoxText*)currentLBT)->data();

	  // add the message type into the message types
	  types |= (uint64_t(1) << type);

	  // found a selected item, fields are valid for update
	  update = true;
	}
      }

      // only enable add if the filter is different from its predecessor
      if ((m_name->text() != m_currentFilter->name()) || 
	  (m_pattern->text() != m_currentFilter->regexp().pattern()) ||
	  (types != m_currentFilter->types()))
	add = true;

    }
  }
  else
  {
    // buttons should only be enabled for valid message filter content
    if (!m_name->text().isEmpty() &&
	!m_pattern->text().isEmpty())
    {
      // iterate over all the message types
      for (QListBoxItem* currentLBT = m_messageTypes->firstItem();
	   currentLBT;
	   currentLBT = currentLBT->next())
      {
	// if the item isn't selected, try the next item
	if (!currentLBT->isSelected())
	  continue;
	
	// found a selected item, fields are valid for add
	add = true;
	break;
      }
    }
  }

  // set the button states according to the results from above
  m_add->setEnabled(add);
  m_update->setEnabled(update);

  // only enable delete if editing an existing filter
  m_delete->setEnabled(m_currentFilter != 0);
}
