/*
 * filteredspawnlog.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _FILTEREDSPAWNLOG_H_
#define _FILTEREDSPAWNLOG_H_

#include <qobject.h>
#include "logger.h"

//----------------------------------------------------------------------
// forward declarations
class QDateTime;
class DateTimeMgr;
class FilterMgr;
class Item;

//----------------------------------------------------------------------
// FilteredSpawnLog
class FilteredSpawnLog: public SEQLogger 
{
   Q_OBJECT

public:
   FilteredSpawnLog(DateTimeMgr* dateTimeMgr, FilterMgr* filterMgr,
		    const QString& filename);
   ~FilteredSpawnLog();
   uint32_t filters() { return m_logFilters; }

public slots:
   void setFilters(uint32_t flags);

   void addItem(const Item* item);
   void delItem(const Item* item);
   void killSpawn(const Item* item);

 protected:
   void logSpawn(const Item* item, const char* action, uint32_t flag);
   DateTimeMgr* m_dateTimeMgr;
   FilterMgr* m_filterMgr;
   uint32_t m_logFilters;
};

#endif // _FILTEREDSPAWNLOG_H_

