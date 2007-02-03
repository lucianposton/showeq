/*
 * spawnlog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2001-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2007 Zaphod (dohpaz@users.sourceforge.net). 
 */

#include "filteredspawnlog.h"
#include "filtermgr.h"
#include "spawn.h"
#include "datetimemgr.h"

FilteredSpawnLog::FilteredSpawnLog(DateTimeMgr* dateTimeMgr, 
				   FilterMgr* filterMgr,
				   const QString& filename)
  : SEQLogger(filename, NULL, "FilteredSpawnLog"),
    m_dateTimeMgr(dateTimeMgr),
    m_filterMgr(filterMgr),
    m_logFilters(0)
{
}

FilteredSpawnLog::~FilteredSpawnLog()
{
}

void FilteredSpawnLog::setFilters(uint32_t flags)
{
  m_logFilters = flags;
}

void FilteredSpawnLog::addItem(const Item* item)
{
  uint32_t filterFlags = item->filterFlags();

  if (!(filterFlags & m_logFilters))
    return;

  if (filterFlags & (FILTER_FLAG_ALERT | FILTER_FLAG_DANGER | 
		     FILTER_FLAG_CAUTION | FILTER_FLAG_HUNT | 
		     FILTER_FLAG_LOCATE))
    logSpawn(item, "spawned", filterFlags);
}

void FilteredSpawnLog::delItem(const Item* item)
{
  uint32_t filterFlags = item->filterFlags();

  if (!(filterFlags & m_logFilters))
    return;

  if (filterFlags & FILTER_FLAG_ALERT)
    logSpawn(item, "despawned", filterFlags);
}

void FilteredSpawnLog::killSpawn(const Item* item)
{
  uint32_t filterFlags = item->filterFlags();

  if (!(filterFlags & m_logFilters))
    return;

  if (filterFlags & FILTER_FLAG_ALERT)
    logSpawn(item, "killed", filterFlags);
}

void FilteredSpawnLog::logSpawn(const Item* item, const char* action, 
				uint32_t flag)
{
  // make sure the log file is open
  if (!open())
    return;

  // get the current EQ Date&Time
  const QDateTime& eqDate = m_dateTimeMgr->updatedDateTime();

  // log the information
  outputf("%s %s %s LOC %dy, %dx, %dz at %s (%s)\n", 
	  (const char*)m_filterMgr->filterString(flag),
	  action,
	  (const char*)item->name(), 
	  item->y(), item->x(), item->z(),
	  eqDate.isValid() ? (const char*)eqDate.toString() : "",
	  (const char*)item->spawnTimeStr());

  flush();
}

#ifndef QMAKEBUILD
#include "filteredspawnlog.moc"
#endif

