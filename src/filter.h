/*
 * filter.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//
#ifndef FILTER_H
#define FILTER_H

#include <sys/types.h>
#include <regex.h>

#include <qstring.h>
#include <qlist.h>

//--------------------------------------------------
// forward declarations
class FilterItem;
class Filter;

//--------------------------------------------------
// typedefs
typedef QList<FilterItem> FilterList;
typedef QListIterator<FilterItem> FilterListIterator;

//--------------------------------------------------
// FilterItem
class FilterItem
{
public:
  FilterItem(const QString& filterPattern, int cflags = 0);
  ~FilterItem(void);

  bool isFiltered(const QString& filterString, int level);

  const QString& name() { return m_name; }
  const QString& filterPattern() { return m_name; }
  int minLevel() { return m_minLevel; }
  int maxLevel() { return m_maxLevel; }
  bool valid() { return m_valid; }

 protected:
  int m_minLevel;
  int m_maxLevel;
  regex_t m_regexp;
  QString m_name;
  bool m_valid;
};


//--------------------------------------------------
// Filter
class Filter
{
public:
   Filter(int cFlags = 0);
   Filter(const QString& file = 0, int cFlags = 0);
   Filter(const QString& file = 0, const char *type = 0, int cFlags = 0);
   ~Filter();

   const QString& filterFile() { return m_file; }
   const QString& filterType() { return m_type; }

   bool loadFilters(void);
   bool saveFilters(void);
   bool saveAsFilters(const QString& filterFileName);
   bool isFiltered(const QString& filterString, int level);
   bool addFilter(const QString& filterPattern);
   void remFilter(const QString& filterPattern); 
   void listFilters(void);
   void changeFilterFile(const QString&);
   void setCFlags(int cFlags);

private:
   FilterItem* findFilter(const QString& filterPattern);

   QString m_file;
   FilterList m_filterItems;
   char  m_cFlags;        // flags passed to regcomp
   QString m_type;
};

#endif // FILTER_H
