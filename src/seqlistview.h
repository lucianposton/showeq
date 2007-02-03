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

#include <q3listview.h>
#include <qstring.h>
#include <qstringlist.h>

class SEQListView : public Q3ListView
{
   Q_OBJECT

 public:
  SEQListView(const QString prefName, 
	      QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0);
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

#endif // SEQLISTVIEW_H
