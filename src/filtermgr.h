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

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif

#include <map>

#include <qobject.h>

#include "everquest.h"

//----------------------------------------------------------------------
// forward declarations
class Filter;
class Filters;
class FilterTypes;
class DataLocationMgr;

//
// ZBTEMP: predefined filters and filter mask will be migrated out
// so that ShowEQ code can register the file based filters and there mask
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

//----------------------------------------------------------------------
// FilterMgr
class FilterMgr : public QObject
{
  Q_OBJECT

 public:
  FilterMgr(const DataLocationMgr* dataLocMgr, 
	    const QString filterFile, bool spawnfilter_case);
  ~FilterMgr();
  
  const QString& filterFile(void) { return m_filterFile; }
  const QString& zoneFilterFile(void) { return m_zoneFilterFile; }
  bool caseSensitive(void) { return m_caseSensitive; }
  void setCaseSensitive(bool caseSensitive);

  uint32_t filterMask(const QString& filterString, uint8_t level) const;
  QString filterString(uint32_t mask) const;
  QString filterName(uint8_t filter) const;
  bool addFilter(uint8_t filter, const QString& filterString);
  void remFilter(uint8_t filter, const QString& filterString);
  bool addZoneFilter(uint8_t filter, const QString& filterString);
  void remZoneFilter(uint8_t filter, const QString& filterString);

  bool registerRuntimeFilter(const QString& name, 
			     uint8_t& flag,
			     uint32_t& flagMask);
  void unregisterRuntimeFilter(uint8_t flag);
  uint32_t runtimeFilterMask(const QString& filterString, uint8_t level) const;
  QString runtimeFilterString(uint32_t filterMask) const;
  bool runtimeFilterAddFilter(uint8_t flag, const QString& filter);
  void runtimeFilterRemFilter(uint8_t flag, const QString& filter);
  void runtimeFilterCommit(uint8_t flag);

 public slots:
  void loadFilters(void);
  void loadFilters(const QString& filterFile);
  void saveFilters(void);
  void listFilters(void);
  void loadZone(const QString& zoneShortName);
  void loadZoneFilters(void);
  void listZoneFilters(void);
  void saveZoneFilters(void);

 signals:
  void filtersChanged();
  void runtimeFiltersChanged(uint8_t flag);


 private:
  const DataLocationMgr* m_dataLocMgr;
  FilterTypes* m_types;
  QString m_filterFile;
  Filters* m_filters;
  QString m_zoneFilterFile;
  Filters* m_zoneFilters;

  FilterTypes* m_runtimeTypes;
  Filters* m_runtimeFilters;

  bool m_caseSensitive;
};

#endif // FILTERMGR_H
