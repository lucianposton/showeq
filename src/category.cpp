/*
 * category.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sf.net/
 */

// Author: Zaphod (dohpaz@users.sourceforge.net)
//   Many parts derived from existing ShowEQ/SINS map code

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#include <stdio.h>

// ZBTEMP: Temporarily use pSEQPrefs for data
#include "main.h"

#include "category.h"

// ------------------------------------------------------
// Category
Category::Category(const QString& name, 
		   const QString& filter, 
		   const QString& filterout, 
		   QColor color)
{
  m_name = name;
  m_filter = filter;
  if (!filterout.isEmpty())
    m_filterout = filterout;
  m_color = color;
  m_flags = 0;
  
  int cFlags = REG_EXTENDED | REG_ICASE;

  // allocate the filter item
  if (filter != "Filtered")
    m_filterItem = new FilterItem(filter, cFlags);
  else
    m_filterItem = NULL;

  // allocate the filter out item
  if (m_filterout.isEmpty())
    m_filterOutItem = NULL;
  else
    m_filterOutItem = new FilterItem(filterout, cFlags);
}

Category::~Category()
{
  delete m_filterItem;
  delete m_filterOutItem;
}

bool Category::isFilteredFilter() const
{
  return (m_filterItem == NULL);
}

bool Category::isFiltered(const QString& filterString, int level) const
{
  if ((m_filterItem != NULL) &&
      m_filterItem->isFiltered(filterString, level))
  {
    if ((m_filterOutItem != NULL) && 
	m_filterOutItem->isFiltered(filterString, level))
      return false;
    else
      return true;
  }

  return false;
}

// ------------------------------------------------------
// CFilterDlg
CFilterDlg::CFilterDlg(QWidget *parent, QString name)
 : QDialog(parent, name, TRUE)
{
#ifdef DEBUG
   debug ("CFilterDlg()");
#endif /* DEBUG */

   QBoxLayout* topLayout = new QVBoxLayout(this);
   QBoxLayout* row4Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row3Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row2Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row1Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row0Layout = new QHBoxLayout(topLayout);

   QLabel *colorLabel = new QLabel ("Color", this);
   colorLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
   colorLabel->setAlignment(QLabel::AlignRight|QLabel::AlignVCenter);
   row1Layout->addWidget(colorLabel);

   m_Color = new QComboBox(FALSE, this, "Color");
   m_Color->insertItem("black");
   m_Color->insertItem("gray");
   m_Color->insertItem("DarkBlue");
   m_Color->insertItem("DarkGreen");
   m_Color->insertItem("DarkCyan");
   m_Color->insertItem("DarkRed");
   m_Color->insertItem("DarkMagenta");
   m_Color->insertItem("yellow4");
   m_Color->insertItem("DarkGray");
   m_Color->insertItem("white");
   m_Color->insertItem("blue");
   m_Color->insertItem("green");
   m_Color->insertItem("cyan");
   m_Color->insertItem("red");
   m_Color->insertItem("magenta");
   m_Color->insertItem("yellow");
   m_Color->insertItem("white");

   m_Color->setFont(QFont("Helvetica", 12));
   row1Layout->addWidget(m_Color, 0, AlignLeft);

   QLabel *nameLabel = new QLabel ("Name", this);
   nameLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
   nameLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row4Layout->addWidget(nameLabel);

   m_Name = new QLineEdit(this, "Name");
   m_Name->setFont(QFont("Helvetica", 12, QFont::Bold));
   row4Layout->addWidget(m_Name);

   QLabel *filterLabel = new QLabel ("Filter", this);
   filterLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
   filterLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row3Layout->addWidget(filterLabel);

   m_Filter  = new QLineEdit(this, "Filter");
   m_Filter->setFont(QFont("Helvetica", 12, QFont::Bold));
   row3Layout->addWidget(m_Filter);

   QLabel *filteroutLabel = new QLabel ("FilterOut", this);
   filteroutLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
   filteroutLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row2Layout->addWidget(filteroutLabel);

   m_FilterOut  = new QLineEdit(this, "FilterOut");
   m_FilterOut->setFont(QFont("Helvetica", 12, QFont::Bold));
   row2Layout->addWidget(m_FilterOut);

   QPushButton *ok = new QPushButton("OK", this);
   row0Layout->addWidget(ok, 0, AlignLeft);

   QPushButton *cancel = new QPushButton("Cancel", this);
   row0Layout->addWidget(cancel, 0, AlignRight);

   // Hook on pressing the buttons
   connect(ok, SIGNAL(clicked()), SLOT(accept()));
   connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

// ------------------------------------------------------
// CategoryMgr
CategoryMgr::CategoryMgr(QObject* parent, const char* name)
  : QObject(parent, name)
{
}

CategoryMgr::~CategoryMgr()
{
}

const CategoryDict CategoryMgr::findCategories(const QString& filterString, int level) const
{
  CategoryDict tmpDict;
  
  // iterate over all the categories looking for a match
  CategoryDictIterator it(m_categories);
  for (Category* curCategory = it.toFirst(); 
       curCategory != NULL;
       curCategory = ++it)
  {
    // if it matches the category add it to the dictionary
    if (curCategory->isFiltered(filterString, level))
      tmpDict.insert(curCategory->name(), curCategory);
  }

  return tmpDict;
}

const Category* CategoryMgr::AddCategory(const QString& name, 
					 const QString& filter, 
					 const QString& filterout, 
					 QColor color)
{
//printf("AddCategory() '%s' - '%s':'%s'\n", name, filter, filterout?filterout:"null");

  // ZBTEMP: TODO, need to add check for duplicate category name

   if (!name.isEmpty() && !filter.isEmpty()) 
   {
     Category* newcat = new Category(name, filter, filterout, color);

     m_categories.insert(newcat->name(), newcat);

     emit addCategory(newcat);

//printf("Added '%s'-'%s' '%s' %d\n", newcat->name, newcat->filter, newcat->listitem->text(0).ascii(), newcat->listitem);
     return newcat;
   }

   return NULL;
}

void CategoryMgr::RemCategory(const Category* cat)
{
//printf("RemCategory()\n");

  if (cat != NULL) 
  {
    // signal that the category is being deleted
    emit delCategory(cat);

    // remove the category from the list
    m_categories.remove(cat->name());

    // delete the category
    delete cat;
  }
}

void CategoryMgr::clearCategories(void)
{
//printf("clearCategories()\n");
  emit clearedCategories();

  m_categories.clear();
}

void CategoryMgr::addCategory(QWidget* parent)
{
  // not editing an existing, adding a new
  editCategories(NULL, parent);
}


void CategoryMgr::editCategories(const Category* cat, QWidget* parent)
{
  // Create the filter dialog
  CFilterDlg* dlg = new CFilterDlg(parent, "FilterDlg");

  // editing an existing category?
  if (cat != NULL)
  {
    // yes, use it's info for the defaults
    dlg->m_Name->setText(cat->name());
    dlg->m_Filter->setText(cat->filter());
    dlg->m_FilterOut->setText(cat->filterout());
  }
  else
  {
    dlg->m_Filter->setText("Filter");
    dlg->m_FilterOut->setText("");
    dlg->m_Color->setCurrentItem(0);
  }

  int res = dlg->exec();

  // if the dialog wasn't accepted, don't add/change a category
  if (res != QDialog::Accepted)
    return;

  // remove the old category
  if (cat != NULL)
    RemCategory(cat);

  // Add Category
  QString name = dlg->m_Name->text();
  QString filter = dlg->m_Filter->text();

//printf("Got name: '%s', filter '%s', filterout '%s', color '%s'\n",
//  name?name:"", color?color:"", filter?filter:"", filterout?filterout:""); 

  if (!name.isEmpty() && !filter.isEmpty())
    AddCategory(name, 
		filter, 
		dlg->m_FilterOut->text(), 
		QColor(dlg->m_Color->currentText()));
  
  delete dlg;
}

void CategoryMgr::reloadCategories(void)
{
   clearCategories();

   QString section = "CategoryMgr";
   int i = 0;
   char tempStr[256];
   for(i = 1; i < 25; i++)
   {
      // attempt to pull a button title from the preferences
      sprintf(tempStr, "Category%d_Name", i);
      if (pSEQPrefs->isPreference(tempStr, section))
      {
        QString name = pSEQPrefs->getPrefString(tempStr, section);
        sprintf(tempStr, "Category%d_Filter", i);
        QString filter = pSEQPrefs->getPrefString(tempStr, section);
        sprintf(tempStr, "Category%d_Color", i);
        QColor color = pSEQPrefs->getPrefColor(tempStr, section, QColor("black"));
        sprintf(tempStr, "Category%d_FilterOut", i);
        QString filterout;
        if (pSEQPrefs->isPreference(tempStr, section))
            filterout = pSEQPrefs->getPrefString(tempStr, section);

//printf("%d: Got '%s' '%s' '%s'\n", i, name, filter, color);
        if (!name.isEmpty() && !filter.isEmpty())
        {
	  Category* newcat = new Category(name, filter, filterout, color);

	  m_categories.insert(newcat->name(), newcat);
        }
      }
   }

   // signal that the categories have been loaded
   emit loadedCategories();

   printf("Categories Reloaded\n");
}
