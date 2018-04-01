/*
 * seqlistview.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef SEQLISTVIEW_H
#define SEQLISTVIEW_H

#include <qlistview.h>
#include <qstring.h>
#include <qstringlist.h>

class SEQListView : public QListView
{
   Q_OBJECT

 public:
  SEQListView(const QString prefName, 
	      QWidget* parent = 0, const char* name = 0, WFlags f = 0);
  ~SEQListView();

  const QString& preferenceName() const { return m_preferenceName; }
  const QString& columnPreferenceName(int column) const;

  QSizePolicy sizePolicy() const;

  virtual int addColumn(const QString& label,
			int width = -1);
  virtual int addColumn(const QString& label, 
			const QString& preference, 
			int width = -1);
  virtual void removeColumn(int column);
  bool columnVisible(int column) { return (columnWidth(column) != 0); }
  virtual void setSorting(int column, bool increasing = true);
  int sortColumn() const { return m_sortColumn; }
  bool sortIncreasing() const { return m_sortIncreasing; }

 public slots:
   virtual void restoreColumns(void);
   virtual void savePrefs(void);
   void setColumnVisible(int column, bool visible);

 private:
   QString m_preferenceName;
   QStringList m_columns;
   int m_sortColumn;
   bool m_sortIncreasing;
};

typedef int(*SEQListViewItemComparator)(const QString&, const QString&);

int SEQListViewItemCompareInt(const QString&, const QString&);
int SEQListViewItemCompareDouble(const QString&, const QString&);
int SEQListViewItemCompareRemainingTime(const QString&, const QString&);

template<int S=12>
class SEQListViewItem : public QListViewItem
{
public:
    SEQListViewItem(
            QListView* parent,
            const QString& l1=QString::null,
            const QString& l2=QString::null,
            const QString& l3=QString::null,
            const QString& l4=QString::null,
            const QString& l5=QString::null,
            const QString& l6=QString::null,
            const QString& l7=QString::null,
            const QString& l8=QString::null);
    virtual ~SEQListViewItem();

    virtual int compare(QListViewItem*, int, bool) const;
    void setColComparator(int, SEQListViewItemComparator);

protected:
    // No std::array in c++98...
    // Using std::pair instead of std::optional, since using c++98
    std::pair<bool,SEQListViewItemComparator> m_comparators[S];
};

#endif // SEQLISTVIEW_H
