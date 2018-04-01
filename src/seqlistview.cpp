/*
 * seqlistview.cpp
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

#include <qheader.h>
#include <qsizepolicy.h>

#include "seqlistview.h"
#include "diagnosticmessages.h"
#include "main.h"

SEQListView::SEQListView(const QString prefName, 
			 QWidget* parent, const char* name, WFlags f)
  : QListView(parent, name, f),
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
  int column = QListView::addColumn(label, width);

  // add the column to the list of preferences
  m_columns.append(preference);

  // return the column index
  return column;
}

void SEQListView::removeColumn(int column)
{
  // remove the column from the listview
  QListView::removeColumn(column);
  
  // remove the preference from the list
  m_columns.remove(m_columns.at(column));
}

void SEQListView::setSorting(int column, bool increasing)
{
  // save the sort information
  m_sortColumn = column;
  m_sortIncreasing = increasing;

  // set the sort order in the underlying listview
  QListView::setSorting(column, increasing);
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
	setColumnWidthMode(i, QListView::Manual);
	setColumnWidth(i, width);
      }
    }
    else
    {
      // column is not visible, hide it.
      setColumnWidthMode(i, QListView::Manual);
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
  setColumnWidthMode(column, QListView::Manual);

  // set the column width
  setColumnWidth(column, width);

  // set the the preferences as to if the column is shown
  pSEQPrefs->setPrefBool(QString("Show") + columnName, preferenceName(),
			 (width != 0));

  // trigger an update, otherwise things may look messy
  triggerUpdate();
}

template<int S>
SEQListViewItem<S>::SEQListViewItem(
        QListView* parent,
        const QString& l1,
        const QString& l2,
        const QString& l3,
        const QString& l4,
        const QString& l5,
        const QString& l6,
        const QString& l7,
        const QString& l8)
    : QListViewItem(parent, l1, l2, l3, l4, l5, l6, l7, l8)
{
    if (S < 1)
        seqWarn("SEQListViewItem<S=%d> template parameter should be at least 1.", S);
}

template<int S>
SEQListViewItem<S>::~SEQListViewItem()
{
}

template<int S>
int SEQListViewItem<S>::compare(QListViewItem* rhs, int col, bool ascending) const
{
    if (col < 0 || col >= S || !m_comparators[col].first)
    {
        return QListViewItem::compare(rhs, col, ascending);
    }

    return m_comparators[col].second(key(col, ascending), rhs->key(col, ascending));
}

template<int S>
void SEQListViewItem<S>::setColComparator(int col, SEQListViewItemComparator comparator)
{
    if (col < 0)
    {
        seqWarn("Failed to set comparator. col=%d is less than 0", col);
        return;
    }
    const int num_comparators = sizeof(m_comparators)/sizeof(m_comparators[0]);
    if (col > num_comparators)
    {
        seqWarn("Failed to set comparator. col=%d is greater than size=%d",
                col, num_comparators);
        return;
    }

    m_comparators[col] = std::make_pair(true, comparator);
}

int SEQListViewItemCompareInt(const QString& lhs, const QString& rhs)
{
    const bool lhs_is_empty = lhs.isEmpty();
    const bool rhs_is_empty = rhs.isEmpty();
    if (lhs_is_empty && rhs_is_empty)
        return 0;
    else if (lhs_is_empty)
        return -1;
    else if (rhs_is_empty)
        return 1;

    bool ok;
    const int lhs_int = lhs.toInt(&ok);
    if (!ok)
        seqWarn("SEQListViewItemCompareInt(%s,%s): lhs failed",
                (const char*)lhs, (const char*)rhs);
    const int rhs_int = rhs.toInt(&ok);
    if (!ok)
        seqWarn("SEQListViewItemCompareInt(%s,%s): rhs failed",
                (const char*)lhs, (const char*)rhs);
    return lhs_int - rhs_int;
}

int SEQListViewItemCompareDouble(const QString& lhs, const QString& rhs)
{
    const bool lhs_is_empty = lhs.isEmpty();
    const bool rhs_is_empty = rhs.isEmpty();
    if (lhs_is_empty && rhs_is_empty)
        return 0;
    else if (lhs_is_empty)
        return -1;
    else if (rhs_is_empty)
        return 1;

    bool ok;
    const double lhs_double = lhs.toDouble(&ok);
    if (!ok)
        seqWarn("SEQListViewItemCompareDouble(%s,%s): lhs failed",
                (const char*)lhs, (const char*)rhs);
    const double rhs_double = rhs.toDouble(&ok);
    if (!ok)
        seqWarn("SEQListViewItemCompareDouble(%s,%s): rhs failed",
                (const char*)lhs, (const char*)rhs);
    if (lhs_double == rhs_double)
        return 0;
    else if (lhs_double < rhs_double)
        return -1;
    else if (lhs_double > rhs_double)
        return 1;
    else // if (lhs_double != rhs_double) // e.g. comparing nan to nan
        // Return 1 to sort nan's after everything else
        return 1;
}

int SEQListViewItemCompareRemainingTime(const QString& lhs, const QString& rhs)
{
    const bool lhs_is_empty = lhs.isEmpty();
    const bool rhs_is_empty = rhs.isEmpty();
    if (lhs_is_empty && rhs_is_empty)
        return 0;
    else if (lhs_is_empty)
        return -1;
    else if (rhs_is_empty)
        return 1;

    // Sorts "\d+:\d{2}"
    const QString seconds_str = lhs.section(':',1,1);
    const QString iseconds_str = rhs.section(':',1,1);
    if (!seconds_str.isEmpty() && !iseconds_str.isEmpty())
    {
        bool ok;
        const int mins = lhs.section(':',0,0).toInt(&ok);
        if (!ok)
            seqWarn("SEQListViewItemCompareRemainingTime(%s,%s): mins failed",
                    (const char*)lhs, (const char*)rhs);
        const int imins = rhs.section(':',0,0).toInt(&ok);
        if (!ok)
            seqWarn("SEQListViewItemCompareRemainingTime(%s,%s): imins failed",
                    (const char*)lhs, (const char*)rhs);
        if (mins != imins)
            return mins - imins;

        const int seconds = seconds_str.toInt(&ok);
        if (!ok)
            seqWarn("SEQListViewItemCompareRemainingTime(%s,%s): seconds failed",
                    (const char*)lhs, (const char*)rhs);
        const int iseconds = iseconds_str.toInt(&ok);
        if (!ok)
            seqWarn("SEQListViewItemCompareRemainingTime(%s,%s): iseconds failed",
                    (const char*)lhs, (const char*)rhs);
        return seconds - iseconds;
    }

    // plain unicode comparison keeps "   now" sorted before remaining time
    return lhs.compare(rhs);
}

// Forward declare required sizes
template class SEQListViewItem<12>;

#include "seqlistview.moc"
