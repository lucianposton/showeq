/*
 * filtermgr.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#include <stdio.h>

#include <qregexp.h>
#include <qstring.h>
#include <qfileinfo.h>

#include "filtermgr.h"
#include "filter.h"

//
// ZBTEMP: dependency on ShowEQ main.h and it's showeq_params will be 
// migrated out, these are only here temporarily
//
//
// ZBTEMP: predefined filters and filter flags will be migrated out
// so that ShowEQ code can register the file based filters and there flags
// at runtime ala the runtime Filter stuff
//

#include "main.h"

//----------------------------------------------------------------------
// FilterMgr
FilterMgr::FilterMgr()
  : QObject(NULL, "filtermgr")
{
  // Initialize spawn filters
  m_cFlags = REG_EXTENDED;
  if (!showeq_params->spawnfilter_case)
    m_cFlags |= REG_ICASE;

  // get the current filter file name
  m_filterFile = showeq_params->filterfile;

  printf("Loading filters from '%s'\n", (const char*)m_filterFile);
  
  m_filters[HUNT_FILTER] = new Filter(m_filterFile, 
				      "Hunt", m_cFlags);
  m_filters[CAUTION_FILTER] = new Filter(m_filterFile, 
					 "Caution", m_cFlags);
  m_filters[DANGER_FILTER] = new Filter(m_filterFile, 
					"Danger", m_cFlags);
  m_filters[LOCATE_FILTER] = new Filter(m_filterFile, 
					"Locate", m_cFlags);
  m_filters[ALERT_FILTER] = new Filter(m_filterFile, 
				       "Alert", m_cFlags);
  m_filters[FILTERED_FILTER] = new Filter(m_filterFile, 
					  "Filtered", m_cFlags);
  m_filters[TRACER_FILTER] = new Filter(m_filterFile, 
					"Tracer", m_cFlags);

  // setup runtime filterings
  m_runtimeFiltersAllocated = 0;
  for (int i = 0; i < MAXSIZEOF_RUNTIMEFILTERS; i++)
    m_runtimeFilters[i] = NULL;
}

FilterMgr::~FilterMgr()
{
  // delete the filters
  for (int i = 0; i < SIZEOF_FILTERS; i++)
    delete m_filters[i];
}

uint32_t FilterMgr::filterFlags(const QString& filterString, int level) const
{
  uint32_t flags = 0;

  // determine the filter flags, iterate over all the filters
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
  {
    // if there's a match, note it in the flags
    if(m_filters[index]->isFiltered(filterString, level))
      flags |= (1 << index);
  }
  
  return flags;
}

QString FilterMgr::filterString(uint32_t filterFlags) const
{
   QString text = "";

   if (filterFlags & FILTER_FLAG_HUNT)
     text += "Hunt:";

   if (filterFlags & FILTER_FLAG_CAUTION)
     text += "Caution:";

   if (filterFlags & FILTER_FLAG_DANGER)
     text += "Danger:";

   if (filterFlags & FILTER_FLAG_LOCATE)
     text += "Locate:";

   if (filterFlags & FILTER_FLAG_ALERT)
     text += "Alert:";

   if (filterFlags & FILTER_FLAG_FILTERED)
     text += "Filtered:";

   if (filterFlags & FILTER_FLAG_TRACER)
     text += "Tracer:";

   return text;
}

void FilterMgr::loadFilters(void)
{
  // load the filters using the existing filter file
  loadFilters(m_filterFile);
}

void FilterMgr::loadFilters(const QString& filterFileName)
{
  m_filterFile = filterFileName;
  printf("Loading filters from '%s'\n", (const char*)m_filterFile);
  
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_filters[index]->changeFilterFile(m_filterFile);

  emit filtersChanged();
}

void FilterMgr::saveFilters(void)
{
  printf("Saving filters to %s\n", (const char*)m_filterFile);

  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_filters[index]->saveFilters();
}

void FilterMgr::listFilters(void)
{
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
  {
    printf("Filters %d from '%s' section of file '%s':\n", index,
	   (const char*)m_filters[index]->filterType(), 
	   (const char*)m_filters[index]->filterFile());
    m_filters[index]->listFilters();
  }
}

void FilterMgr::loadZone(const QString& shortZoneName)
{
  QString zoneFilterFileName;

  zoneFilterFileName.sprintf(LOGDIR "/filters_%s.conf", 
			     (const char*)shortZoneName.lower());

  QFileInfo fileInfo(zoneFilterFileName);

  if (fileInfo.exists())
  {
    m_filterFile = zoneFilterFileName;
    printf("Loading Filter File: %s\n", (const char*)m_filterFile);
  }
  else
  {
    m_filterFile = showeq_params->filterfile;
    printf("No Zone Specific filter file '%s'.\n"
	   "Loading default '%s'.\n",
	   (const char*)zoneFilterFileName, (const char*)m_filterFile);
  }

  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_filters[index]->changeFilterFile(m_filterFile);

  emit filtersChanged();
}

bool FilterMgr::registerRuntimeFilter(const QString& name, 
				      uint8_t& flag,
				      uint32_t& flagMask)
{
  int tmpFlagMask;
  for(int index = 0 ; index < MAXSIZEOF_RUNTIMEFILTERS ; index++)
  {
    tmpFlagMask = (1 << index);
    if (!(m_runtimeFiltersAllocated & tmpFlagMask))
    {
      m_runtimeFiltersAllocated |= tmpFlagMask;
      m_runtimeFilterNames[index] = name;
      m_runtimeFilters[index] = new Filter(m_cFlags);
      flag = index;
      flagMask = tmpFlagMask;
      return true;
    }
  }

  return false;
}

void FilterMgr::unregisterRuntimeFilter(uint8_t flag)
{
  uint32_t flagMask = (1 << flag);

  // make sure it's actually an allocated flag
  if (m_runtimeFiltersAllocated & flagMask)
  {
    m_runtimeFiltersAllocated &= ~flagMask;
    m_runtimeFilterNames[flag] = "";
    delete m_runtimeFilters[flag];
    m_runtimeFilters[flag] = NULL;
  }
}

uint32_t FilterMgr::runtimeFilterFlags(const QString& filterString,
				       int level) const
{
  uint32_t flags = 0;
  uint32_t flagMask;

  // determine the filter flags, iterate over all the filters
  for(int index = 0 ; index < MAXSIZEOF_RUNTIMEFILTERS ; index++)
  {
    flagMask = (1 << index);

    if (m_runtimeFiltersAllocated & flagMask)
    {
      // if there's a match, note it in the flags
      if(m_runtimeFilters[index]->isFiltered(filterString, level))
	flags |= flagMask;
    }
  }

  return flags;
}

QString FilterMgr::runtimeFilterString(uint32_t filterFlags) const
{
   QString text = "";

   uint32_t flagMask;
   for (int index = 0; index < MAXSIZEOF_RUNTIMEFILTERS; index++)
   {
     flagMask = (1 << index);
     if ((filterFlags & flagMask) && (m_runtimeFiltersAllocated & flagMask))
       text += m_runtimeFilterNames[index] + ":";
   }

   return text;
}

bool FilterMgr::runtimeFilterAddFilter(uint8_t flag, const QString& filter)
{
  uint32_t flagMask = (1 << flag);
  if ((m_runtimeFiltersAllocated & flagMask) &&
      (m_runtimeFilters[flag] != NULL))
    return m_runtimeFilters[flag]->addFilter(filter);

  return false;
}

void FilterMgr::runtimeFilterRemFilter(uint8_t flag, const QString& filter)
{
  uint32_t flagMask = (1 << flag);
  if ((m_runtimeFiltersAllocated & flagMask) &&
      (m_runtimeFilters[flag] != NULL))
    m_runtimeFilters[flag]->remFilter(filter);
}

void FilterMgr::runtimeFilterCommit(uint8_t flag)
{
  // notify that the runtime filters have changed
  emit runtimeFiltersChanged(flag);
}

