/*
 * filter.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net
 */


//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#ifndef FILTERMGR_H
#define FILTERMGR_H

#include <stdint.h>

#include <map>

#include <qobject.h>

#include "filter.h"
#include "everquest.h"

//
// ZBTEMP: predefined filters and filter flags will be migrated out
// so that ShowEQ code can register the file based filters and there flags
// at runtime ala the runtime Filter stuff
//

//----------------------------------------------------------------------
// Macro defintions
//Filter Flags
#define HUNT_FILTER 0
#define CAUTION_FILTER 1
#define DANGER_FILTER 2
#define LOCATE_FILTER 3
#define ALERT_FILTER 4
#define FILTERED_FILTER 5
#define TRACER_FILTER 6
#define SIZEOF_FILTERS 7

// max of 32 flags

#define FILTER_FLAG_HUNT		(1 << HUNT_FILTER)
#define FILTER_FLAG_CAUTION		(1 << CAUTION_FILTER)
#define FILTER_FLAG_DANGER		(1 << DANGER_FILTER)
#define FILTER_FLAG_LOCATE		(1 << LOCATE_FILTER)
#define FILTER_FLAG_ALERT		(1 << ALERT_FILTER)
#define FILTER_FLAG_FILTERED		(1 << FILTERED_FILTER)
#define FILTER_FLAG_TRACER		(1 << TRACER_FILTER)
#define FILTER_FLAG_CONSIDERED	        (1 << (SIZEOF_FILTERS + 0))

#define MAXSIZEOF_FILEFILTERS 16
#define MAXSIZEOF_RUNTIMEFILTERS 16

//----------------------------------------------------------------------
// FilterMgr
class FilterMgr : public QObject
{
  Q_OBJECT

 public:
  FilterMgr();
  ~FilterMgr();
  
  uint32_t filterFlags(const QString& filterString, int level) const;
  QString filterString(uint32_t filterFlags) const;
  const QString& filterFile(void) { return m_filterFile; }
  
  bool registerRuntimeFilter(const QString& name, 
			     uint8_t& flag,
			     uint32_t& flagMask);
  void unregisterRuntimeFilter(uint8_t flag);
  uint32_t runtimeFilterFlags(const QString& filterString, int level) const;
  QString runtimeFilterString(uint32_t filterFlags) const;
  bool runtimeFilterAddFilter(uint8_t flag, const QString& filter);
  void runtimeFilterRemFilter(uint8_t flag, const QString& filter);
  void runtimeFilterCommit(uint8_t flag);

 public slots:
  void loadFilters(void);
  void loadFilters(const QString& filterFile);
  void saveFilters(void);
  void listFilters(void);
  void zoneNew(const newZoneStruct* zoneNew, bool client);
  
 signals:
  void filtersChanged();
  void runtimeFiltersChanged(uint8_t flag);

 private:
  QString m_filterFile;
  Filter* m_filters[SIZEOF_FILTERS];
  int m_cFlags;

  uint16_t m_runtimeFiltersAllocated;
  Filter* m_runtimeFilters[MAXSIZEOF_RUNTIMEFILTERS];
  QString m_runtimeFilterNames[MAXSIZEOF_RUNTIMEFILTERS];
};

#endif // FILTERMGR_H
