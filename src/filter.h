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

#include <stdint.h>
#include <sys/types.h>

#include <qstring.h>
#include <qlist.h>
#include <qregexp.h>
#include <qxml.h>
#include <q3textstream.h>

#include <map>

//--------------------------------------------------
// forward declarations
class FilterItem;
class Filter;
class Filters;
class FilterTypes;

//--------------------------------------------------
// typedefs
typedef QList<FilterItem> FilterList;
typedef QListIterator<FilterItem> FilterListIterator;
typedef std::map<uint32_t, QString> FilterTypeMap;
typedef std::map<uint32_t, Filter*> FilterMap;

//--------------------------------------------------
// FilterItem
class FilterItem
{
public:
  FilterItem(const QString& filterPattern, bool caseSensitive);
  FilterItem(const QString& filterPattern, bool caseSensitive, 
	     uint8_t minLevel, uint8_t maxLevel);
  ~FilterItem(void);
   bool save(QString& indent, Q3TextStream& out);

  bool isFiltered(const QString& filterString, uint8_t level) const;

  QString name() const { return m_regexp.pattern(); }
  QString filterPattern() const { return m_regexp.pattern(); }
  uint8_t minLevel() const { return m_minLevel; }
  uint8_t maxLevel() const { return m_maxLevel; }
  bool valid() { return m_regexp.isValid(); }

 protected:
  void init(const QString& filterPattern, bool caseSensitive, uint8_t minLevel,
         uint8_t maxLevel);
  
  QRegExp m_regexp;
  QString m_regexpOriginalPattern;
  uint8_t m_minLevel;
  uint8_t m_maxLevel;
};


//--------------------------------------------------
// Filter
class Filter
{
public:
   Filter(bool caseSensitive = 0);
   ~Filter();

   bool save(QString& indent, Q3TextStream& out);
   bool isFiltered(const QString& filterString, uint8_t level);
   bool addFilter(const QString& filterPattern);
   bool addFilter(const QString& filterPattern, uint8_t minLevel, uint8_t maxLevel);
   void remFilter(const QString& filterPattern); 
   void listFilters(void);
   void setCaseSensitive(bool caseSensitive);

private:
   FilterItem* findFilter(const QString& filterPattern);

   FilterList m_filterItems;
   bool m_caseSensitive;
};

//--------------------------------------------------
// Filters
class Filters
{
 public:
  Filters(const FilterTypes& types);
  ~Filters();
  
  bool clear(void);
  bool clearType(uint8_t type);
  bool load(const QString& filename);
  bool save(const QString& filename) const;
  bool save(void) const;
  void list(void) const;

  bool caseSensitive(void) const { return m_caseSensitive; }
  void setCaseSensitive(bool caseSensitive);
  uint32_t filterMask(const QString& filterString, uint8_t level) const;
  bool addFilter(uint8_t type, const QString& filterString, 
		 uint8_t minLevel = 0, uint8_t maxLevel = 0);
  void remFilter(uint8_t type, const QString& filterString);

 protected:

 protected:
  QString m_file;
  FilterMap m_filters;
  const FilterTypes& m_types;
  bool m_caseSensitive;
};

//--------------------------------------------------
// FilterTypes
class FilterTypes
{
 public:
  FilterTypes();
  ~FilterTypes();

  bool registerType(const QString& name, uint8_t& type, uint32_t& mask);
  void unregisterType(uint8_t type);
  uint8_t maxType(void) const;
  uint32_t mask(uint8_t type) const;
  uint32_t mask(const QString& name) const;
  uint8_t type(const QString& name) const;
  QString name(uint8_t type) const;
  QString name(uint32_t mask) const;
  QString names(uint32_t mask) const;
  bool validMask(uint32_t mask) const;

 protected:
  uint32_t m_allocated;
  FilterTypeMap m_filters;
  uint8_t m_maxType;
};

inline uint8_t FilterTypes::maxType(void) const
{
  return m_maxType;
}

inline uint32_t FilterTypes::mask(uint8_t type) const
{
  return (1 << type);
}

inline QString FilterTypes::name(uint8_t type) const
{
  return name(mask(type));
}

inline QString FilterTypes::name(uint32_t mask) const
{
  FilterTypeMap::const_iterator it = m_filters.find(mask);

  if (it != m_filters.end())
    return it->second;

  return QString("Unknown:") + QString::number(mask);
}

inline bool FilterTypes::validMask(uint32_t mask) const 
{
  return ((m_allocated & mask) != 0);
}

#endif // FILTER_H
