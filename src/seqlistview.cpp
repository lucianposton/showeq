/*
 * seqlistview.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001,2007 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#include <q3header.h>
#include <qsizepolicy.h>

#include "seqlistview.h"
#include "main.h"

SEQListView::SEQListView(const QString prefName, 
			 QWidget* parent, const char* name, Qt::WFlags f)
  : Q3ListView(parent, name, f),
    m_preferenceName(prefName),
    m_sortColumn(0),
    m_sortIncreasing(true)
{
  // setup common listview defaults
  setShowSortIndicator(true);
  setRootIsDecorated(false);
  setSelectionMode(Single);
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
}

SEQListView::~SEQListView()
{
}

const QString& SEQListView::columnPreferenceName(int column) const
{
  // return the base name of the preference for the requested column
  return m_columns[column];
}

QSizePolicy SEQListView::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

int SEQListView::addColumn(const QString& label,
			   int width)
{
  return addColumn(label, label, width);
}

int SEQListView::addColumn(const QString& label, 
			   const QString& preference, 
			   int width)
{
  // add the column to the listview
  int column = Q3ListView::addColumn(label, width);

  // add the column to the list of preferences
  m_columns.append(preference);

  // return the column index
  return column;
}

void SEQListView::removeColumn(int column)
{
  // remove the column from the listview
  Q3ListView::removeColumn(column);
  
  // remove the preference from the list
  m_columns.remove(m_columns.at(column));
}

void SEQListView::setSorting(int column, bool increasing)
{
  // save the sort information
  m_sortColumn = column;
  m_sortIncreasing = increasing;

  // set the sort order in the underlying listview
  Q3ListView::setSorting(column, increasing);
}

void SEQListView::savePrefs()
{
  // only save the preferences if visible
  if (isVisible())
  {
    int i;
    int width;
    QString columnName;
    QString show = "Show";

    // save the column width's/visibility
    for (i = 0; i < columns(); i++)
    {
      columnName = columnPreferenceName(i);
      width = columnWidth(i);
      if (width != 0)
      {
	pSEQPrefs->setPrefInt(columnName + "Width", preferenceName(), width);
	pSEQPrefs->setPrefBool(show + columnName, preferenceName(), true);
      }
      else
	pSEQPrefs->setPrefBool(show + columnName, preferenceName(), false);
    }
    
    // save the column order
    QString tempStr, tempStr2;
    if (header()->count() > 0)
      tempStr.sprintf("%d", header()->mapToSection(0));
    for(i=1; i < header()->count(); i++) 
    {
      tempStr2.sprintf(":%d", header()->mapToSection(i));
      tempStr += tempStr2;
    }
    pSEQPrefs->setPrefString("ColumnOrder", preferenceName(), tempStr);

    // save the current sorting state
    pSEQPrefs->setPrefInt("SortColumn", preferenceName(), m_sortColumn);
    pSEQPrefs->setPrefBool("SortIncreasing", preferenceName(), 
			   m_sortIncreasing);
  }
}

void SEQListView::restoreColumns()
{
  int i;
  int width;
  QString columnName;
  QString show = "Show";
 
  // restore the column width's/visibility
  for (i = 0; i < columns(); i++)
  {
    columnName = columnPreferenceName(i);

    // check if the column is visible
    if (pSEQPrefs->getPrefBool(show + columnName, preferenceName(), true))
    {
      // check if the column has a width specified
      if (pSEQPrefs->isPreference(columnName + "Width", preferenceName()))
      {
	// use the specified column width
	width = pSEQPrefs->getPrefInt(columnName + "Width", preferenceName());
	setColumnWidthMode(i, Q3ListView::Manual);
	setColumnWidth(i, width);
      }
    }
    else
    {
      // column is not visible, hide it.
      setColumnWidthMode(i, Q3ListView::Manual);
      setColumnWidth(i, 0);
    }
  }
  
  // restore the column order
  QString tStr = pSEQPrefs->getPrefString("ColumnOrder", preferenceName(), 
					  "N/A");
  if (tStr != "N/A") 
  {
    int i = 0;
    while (!tStr.isEmpty()) 
    {
      int toIndex;
      if (tStr.find(':') != -1) 
      {
        toIndex = tStr.left(tStr.find(':')).toInt();
        tStr = tStr.right(tStr.length() - tStr.find(':') - 1);
      } 
      else 
      {
        toIndex = tStr.toInt();
        tStr = "";
      }
      header()->moveSection(toIndex, i++);
    }
  }

  // restore sorting state
  setSorting(pSEQPrefs->getPrefInt("SortColumn", preferenceName(), 
				   m_sortColumn),
	     pSEQPrefs->getPrefBool("SortIncreasing", preferenceName(),
				    m_sortIncreasing));
}

void SEQListView::setColumnVisible(int column, bool visible)
{
  QString columnName = columnPreferenceName(column);

  // default width is 0
  int width = 0;

  // if column is to become visible, get it's width
  if (visible)
  {
    // get the column width
    width = pSEQPrefs->getPrefInt(columnName + "Width", preferenceName(), 
				  columnWidth(column));

    // if it's zero, use default width of 40
    if (width == 0)
      width = 40;
  }

  // set the column width mode
  setColumnWidthMode(column, Q3ListView::Manual);

  // set the column width
  setColumnWidth(column, width);

  // set the the preferences as to if the column is shown
  pSEQPrefs->setPrefBool(QString("Show") + columnName, preferenceName(),
			 (width != 0));

  // trigger an update, otherwise things may look messy
  triggerUpdate();
}

#ifndef QMAKEBUILD
#include "seqlistview.moc"
#endif

