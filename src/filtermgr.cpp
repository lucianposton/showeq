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
#include <errno.h>

#include <qregexp.h>
#include <qstring.h>
#include <qfileinfo.h>

#include "filtermgr.h"
#include "filter.h"

//
// ZBTEMP: predefined filters and filter flags will be migrated out
// so that ShowEQ code can register the file based filters and there flags
// at runtime ala the runtime Filter stuff
//

//----------------------------------------------------------------------
// FilterMgr
FilterMgr::FilterMgr(const QString filterFile, bool spawnfilter_case)
  : QObject(NULL, "filtermgr"),
    m_isCaseSensitive(spawnfilter_case)
{
  // Initialize spawn filters
  m_cFlags = REG_EXTENDED;
  if (!m_isCaseSensitive)
    m_cFlags |= REG_ICASE;

  // get the current filter file name
  m_filterFile = filterFile;

  printf("Loading filters from '%s'\n", (const char*)m_filterFile);
  
  m_filters[HUNT_FILTER] = new Filter(m_filterFile, "Hunt", m_cFlags);
  m_filters[CAUTION_FILTER] = new Filter(m_filterFile, "Caution", m_cFlags);
  m_filters[DANGER_FILTER] = new Filter(m_filterFile, "Danger", m_cFlags);
  m_filters[LOCATE_FILTER] = new Filter(m_filterFile, "Locate", m_cFlags);
  m_filters[ALERT_FILTER] = new Filter(m_filterFile, "Alert", m_cFlags);
  m_filters[FILTERED_FILTER] = new Filter(m_filterFile, "Filtered", m_cFlags);
  m_filters[TRACER_FILTER] = new Filter(m_filterFile, "Tracer", m_cFlags);

  m_zoneFilterFile = LOGDIR "/filters_unknown.conf";
  m_zoneFilters[HUNT_FILTER] = new Filter(m_zoneFilterFile, "Hunt", m_cFlags);
  m_zoneFilters[CAUTION_FILTER] = new Filter(m_zoneFilterFile, "Caution", 
					     m_cFlags);
  m_zoneFilters[DANGER_FILTER] = new Filter(m_zoneFilterFile, "Danger", 
					    m_cFlags);
  m_zoneFilters[LOCATE_FILTER] = new Filter(m_zoneFilterFile, "Locate", 
					    m_cFlags);
  m_zoneFilters[ALERT_FILTER] = new Filter(m_zoneFilterFile, "Alert", 
					   m_cFlags);
  m_zoneFilters[FILTERED_FILTER] = new Filter(m_zoneFilterFile, "Filtered", 
					      m_cFlags);
  m_zoneFilters[TRACER_FILTER] = new Filter(m_zoneFilterFile, "Tracer", 
					    m_cFlags);

  // setup runtime filterings
  m_runtimeFiltersAllocated = 0;
  for (int i = 0; i < MAXSIZEOF_RUNTIMEFILTERS; i++)
    m_runtimeFilters[i] = NULL;
}

FilterMgr::~FilterMgr()
{
  int i;
  // delete the filters
  for (i = 0; i < SIZEOF_FILTERS; i++)
    delete m_filters[i];
  for (i = 0; i < SIZEOF_FILTERS; i++)
    delete m_zoneFilters[i];
  for (i = 0; i < MAXSIZEOF_RUNTIMEFILTERS; i++)
    if (m_runtimeFilters[i])
      delete m_runtimeFilters[i];
}

void FilterMgr::setCaseSensitive(bool caseSensitive)
{
  m_isCaseSensitive = caseSensitive;

  m_cFlags = REG_EXTENDED;
  if (!m_isCaseSensitive)
    m_cFlags |= REG_ICASE;

  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_filters[index]->setCFlags(m_cFlags);
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_zoneFilters[index]->setCFlags(m_cFlags);
  for (int index = 0; index < MAXSIZEOF_RUNTIMEFILTERS; index++)
    if (m_runtimeFilters[index])
      m_runtimeFilters[index]->setCFlags(m_cFlags);
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

  // determine the filter flags, iterate over all the filters
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
  {
    // if there's a match, note it in the flags
    if(m_zoneFilters[index]->isFiltered(filterString, level))
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

QString FilterMgr::filterName(uint8_t filter) const
{
  if (filter == HUNT_FILTER)
    return "Hunt";

   if (filter == CAUTION_FILTER)
     return "Caution";

   if (filter == DANGER_FILTER)
     return "Danger";

   if (filter == LOCATE_FILTER)
     return "Locate";

   if (filter == ALERT_FILTER)
     return "Alert";

   if (filter == FILTERED_FILTER)
     return "Filtered";

   if (filter == TRACER_FILTER)
     return "Tracer";

   return QString("Unknown ") + QString::number(filter);
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


void FilterMgr::saveAsFilters(const QString& shortZoneName)
{
   QString zoneFilterFileName;

   zoneFilterFileName.sprintf(LOGDIR "/filters_%s.conf", 
			     (const char*)shortZoneName.lower());


   // If we are not already in a zone-specific filter, copy the current file to the
   // zone-specific file name

   if (zoneFilterFileName != m_filterFile)
   {
      FILE *in;
      FILE *out;
      char buf[100];
      unsigned i;

      printf("Copying filter file '%s' -> '%s'\n", (const char*)m_filterFile, (const char*)zoneFilterFileName);

      if ((in = fopen((const char*)m_filterFile, "r"))==NULL)
      {
         fprintf (stderr, "Couldn't open filter file for copy. '%s' - %s\n",
                 (const char*)m_filterFile, strerror(errno));
         return;
      }
      if ((out = fopen((const char*)zoneFilterFileName, "w+"))==NULL)
      {
         fclose(in);
         fprintf (stderr, "Couldn't open filter file for copy. '%s' - %s\n",
                 (const char*)zoneFilterFileName, strerror(errno));
         return;
      }

      while ((i=fread(buf,1,100,in)))
         if (fwrite(buf,1,i,out)!=i)
            break;

      if (ferror(in)||ferror(out))
      {
         fprintf (stderr, "Couldn't copy filter file.  - %s\n",
                 strerror(errno));
         fclose(in);
         fclose(out);
         return;
      }

      fclose(in);
      fclose(out);

      m_filterFile = zoneFilterFileName;
   }

  printf("Saving filters to %s\n", (const char*)m_filterFile);

  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_filters[index]->saveAsFilters(zoneFilterFileName);
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

bool FilterMgr::addFilter(uint8_t filter, const QString& filterString)
{
  // make sure it's actually a filter
  if (filter >= SIZEOF_FILTERS)
    return false;

  // add the filter
  bool ok = m_filters[filter]->addFilter(filterString);

  // signal that the filters have changed
  emit filtersChanged();

  return ok;
}

void FilterMgr::remFilter(uint8_t filter, const QString& filterString)
{
  // validate that it's a valid filter
  if (filter >= SIZEOF_FILTERS)
    return;

  // remove a filter
  m_filters[filter]->remFilter(filterString);

  // notify that the filters have changed
  emit filtersChanged();
}

bool FilterMgr::addZoneFilter(uint8_t filter, const QString& filterString)
{
  // make sure it's actually a filter
  if (filter >= SIZEOF_FILTERS)
    return false;

  // add the filter
  bool ok = m_zoneFilters[filter]->addFilter(filterString);

  // signal that the filters have changed
  emit filtersChanged();

  return ok;
}

void FilterMgr::remZoneFilter(uint8_t filter, const QString& filterString)
{
  // validate that it's a valid filter
  if (filter >= SIZEOF_FILTERS)
    return;

  // remove a filter
  m_zoneFilters[filter]->remFilter(filterString);

  // notify that the filters have changed
  emit filtersChanged();
}

void FilterMgr::loadZone(const QString& shortZoneName)
{
  QString zoneFilterFileName;

  zoneFilterFileName.sprintf(LOGDIR "/filters_%s.conf", 
			     (const char*)shortZoneName.lower());

  QFileInfo fileInfo(zoneFilterFileName);

  m_zoneFilterFile = zoneFilterFileName;
  printf("Loading Filter File: %s\n", (const char*)m_zoneFilterFile);

  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_zoneFilters[index]->changeFilterFile(m_zoneFilterFile);

  emit filtersChanged();
}

void FilterMgr::loadZoneFilters(void)
{
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_zoneFilters[index]->changeFilterFile(m_zoneFilterFile);

  emit filtersChanged();
}


void FilterMgr::listZoneFilters(void)
{
  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
  {
    printf("Filters %d from '%s' section of file '%s':\n", index,
	   (const char*)m_filters[index]->filterType(), 
	   (const char*)m_filters[index]->filterFile());
    m_zoneFilters[index]->listFilters();
  }
}


void FilterMgr::saveZoneFilters(void)
{
  printf("Saving filters to %s\n", (const char*)m_zoneFilterFile);

  for(int index = 0 ; index < SIZEOF_FILTERS ; index++)
    m_zoneFilters[index]->saveFilters();
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

