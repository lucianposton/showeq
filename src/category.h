/*
 * category.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

// Author: Zaphod (dohpaz@users.sourceforge.net)
//   Many parts derived from existing ShowEQ/SINS map code

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#ifndef _CATEGORY_H_
#define _CATEGORY_H_

#include <sys/types.h>
#include <regex.h>

#include <qstring.h>
#include <qcolor.h>
#include <qlist.h>

// stuff needed for CategoryDlg
#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qpushbutton.h>

//----------------------------------------------------------------------
// forward declarations
class FilterItem;
class Category;
class CategoryMgr;
class CategoryDlg;

// ------------------------------------------------------
// Category
class Category
{
 public:
  Category(const QString& name, 
	   const QString& filter, 
	   const QString& filterout, 
	   QColor color);
  ~Category();
  const QString& name() const { return m_name; }
  const QString& filter() const { return m_filter; }
  const QString& filterout() const { return m_filterout; }
  const QColor& color() const { return m_color; }

  bool isFilteredFilter() const { return m_filteredFilter; };
  bool isFiltered(const QString& filterString, int level = 0) const;

 private:
  QString m_name;
  QString m_filter;
  QString m_filterout;
  FilterItem* m_filterItem;
  FilterItem* m_filterOutItem;
  QColor m_color;
  bool m_filteredFilter;
};

// ------------------------------------------------------
// CategoryDlg
class CategoryDlg : public QDialog
{
  Q_OBJECT
 public:
  CategoryDlg(QWidget *parent, QString name);
  virtual ~CategoryDlg();

 public slots:
  void select_color(void);

 public:
  QLineEdit* m_Name;
  QLineEdit* m_Filter;
  QLineEdit* m_FilterOut;
  QPushButton* m_Color;
};

typedef QList<Category> CategoryList;
typedef QListIterator<Category> CategoryListIterator;
typedef QListIterator<const Category> CategoryListConstIterator;

// ------------------------------------------------------
// CategoryMgr
class CategoryMgr : public QObject
{
   Q_OBJECT
 public:
   enum { tMaxNumCategories = 32 };

   CategoryMgr(QObject* parent = 0, const char* name = 0);
   virtual ~CategoryMgr();

   const CategoryList findCategories(const QString& filterString, int level) const;
   const CategoryList& getCategories(void) const 
     { return m_categories; }
   uint count() { return m_categories.count(); }

   const Category* addCategory(const QString& name, 
			       const QString& filter, 
			       const QString& filterout, 
			       QColor color = Qt::black);
   void remCategory(const Category* cat);

 public slots:
   void clearCategories(void);
   void addCategory(QWidget* parent = 0);
   void editCategories(const Category* cat, QWidget* parent = 0);
   void reloadCategories(void);
   void savePrefs(void);

 signals:
   void addCategory(const Category* cat);
   void delCategory(const Category* cat);
   void clearedCategories(void);
   void loadedCategories(void);

 private:
   CategoryList m_categories;
   bool m_changed;
};

#endif // _CATEGORY_H_
