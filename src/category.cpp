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

#include<qcolordialog.h>

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
  
  int cFlags = REG_EXTENDED | REG_ICASE;

  // allocate the filter item
  m_filterItem = new FilterItem(filter, cFlags);
  m_filteredFilter = (filter.find(":Filtered:", 0, false) != -1);

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
   QFont labelFont;
   labelFont.setBold(true);

   QBoxLayout* topLayout = new QVBoxLayout(this);
   QBoxLayout* row4Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row3Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row2Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row1Layout = new QHBoxLayout(topLayout);
   QBoxLayout* row0Layout = new QHBoxLayout(topLayout);

   QLabel *colorLabel = new QLabel ("Color", this);
   colorLabel->setFont(labelFont);
   colorLabel->setAlignment(QLabel::AlignRight|QLabel::AlignVCenter);
   row1Layout->addWidget(colorLabel, 0, AlignLeft);

   m_Color = new QPushButton(this, "color");
   m_Color->setText("...");
   m_Color->setFont(labelFont);
   connect(m_Color, SIGNAL(clicked()),
	   this, SLOT(select_color()));
   row1Layout->addWidget(m_Color);

   QLabel *nameLabel = new QLabel ("Name", this);
   nameLabel->setFont(labelFont);
   nameLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row4Layout->addWidget(nameLabel);

   m_Name = new QLineEdit(this, "Name");
   m_Name->setFont(labelFont);
   row4Layout->addWidget(m_Name);

   QLabel *filterLabel = new QLabel ("Filter", this);
   filterLabel->setFont(labelFont);
   filterLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row3Layout->addWidget(filterLabel);

   m_Filter  = new QLineEdit(this, "Filter");
   m_Filter->setFont(labelFont);
   row3Layout->addWidget(m_Filter);

   QLabel *filteroutLabel = new QLabel ("FilterOut", this);
   filteroutLabel->setFont(labelFont);
   filteroutLabel->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
   row2Layout->addWidget(filteroutLabel);

   m_FilterOut  = new QLineEdit(this, "FilterOut");
   m_FilterOut->setFont(labelFont);
   row2Layout->addWidget(m_FilterOut);

   QPushButton *ok = new QPushButton("OK", this);
   row0Layout->addWidget(ok, 0, AlignLeft);

   QPushButton *cancel = new QPushButton("Cancel", this);
   row0Layout->addWidget(cancel, 0, AlignRight);

   // Hook on pressing the buttons
   connect(ok, SIGNAL(clicked()), SLOT(accept()));
   connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void CFilterDlg::select_color(void)
{
  QColor newColor = 
    QColorDialog::getColor(m_Color->backgroundColor(), this, "Category Color");

  if (newColor.isValid())
    m_Color->setPalette(QPalette(QColor(newColor)));
}

// ------------------------------------------------------
// CategoryMgr
CategoryMgr::CategoryMgr(QObject* parent, const char* name)
  : QObject(parent, name)
{
  reloadCategories();
}

CategoryMgr::~CategoryMgr()
{
}

const CategoryDict CategoryMgr::findCategories(const QString& filterString, 
					       int level) const
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
  m_changed = true;
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
  m_changed = true;

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
  m_changed = true;
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
    dlg->m_Color->setPalette(QPalette(QColor(cat->color())));
  }
  else
  {
    dlg->m_Name->setText("");
    dlg->m_Filter->setText(".");
    dlg->m_FilterOut->setText("");
    dlg->m_Color->setPalette(QPalette(QColor("black")));
  }

  // execute the dialog
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
		dlg->m_Color->backgroundColor());
  
  delete dlg;
}

void CategoryMgr::reloadCategories(void)
{
  clearCategories();
  m_changed = false;
  
  QString section = "CategoryMgr";
  int i = 0;
  QString prefBaseName;
  QString tempStr;
  for(i = 1; i <= tMaxNumCategories; i++)
  {
    prefBaseName.sprintf("Category%d_", i);
    
    // attempt to pull a button title from the preferences
    tempStr = prefBaseName + "Name";
    if (pSEQPrefs->isPreference(tempStr, section))
    {
      QString name = pSEQPrefs->getPrefString(tempStr, section);
      QString filter =
	pSEQPrefs->getPrefString(prefBaseName + "Filter", section);
      QColor color = pSEQPrefs->getPrefColor(prefBaseName + "Color", 
					     section, QColor("black"));
      tempStr = prefBaseName + "FilterOut";
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

void CategoryMgr::savePrefs(void)
{
  if (!m_changed)
    return;

  int count = 1;
  QString section = "CategoryMgr";
  QString prefBaseName;

  CategoryDictIterator it(m_categories);
  for (Category* curCategory = it.toFirst(); 
       curCategory != NULL;
       curCategory = ++it)
  {
    prefBaseName.sprintf("Category%d_", count++);
    pSEQPrefs->setPrefString(prefBaseName + "Name", section, 
			     curCategory->name());
    pSEQPrefs->setPrefString(prefBaseName + "Filter", section,
			     curCategory->filter());
    pSEQPrefs->setPrefString(prefBaseName + "FilterOut", section,
			     curCategory->filterout());
    pSEQPrefs->setPrefColor(prefBaseName + "Color", section,
			    curCategory->color());
  }

  QColor black("black");
  while (count <= tMaxNumCategories)
  {
    prefBaseName.sprintf("Category%d_", count++);
    pSEQPrefs->setPrefString(prefBaseName + "Name", section, "");
    pSEQPrefs->setPrefString(prefBaseName + "Filter", section, "");
    pSEQPrefs->setPrefString(prefBaseName + "FilterOut", section, "");
    pSEQPrefs->setPrefColor(prefBaseName + "Color", section, black);
  }
}
