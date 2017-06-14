/*
 * datetimemgr.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 *
 */
#include <datetimemgr.h>
#include "everquest.h"

#include <qdatetime.h>
#include <qtimer.h>

DateTimeMgr::DateTimeMgr(QObject* parent, const char* name)
  : QObject(parent, name),
    m_timer(0)
{
}

DateTimeMgr::~DateTimeMgr()
{
    if (m_timer)
    {
        delete m_timer;
    }
}

void DateTimeMgr::timeOfDay(const uint8_t* data)
{
  const timeOfDayStruct* tday = (const timeOfDayStruct*)data;

  m_eqDateTime.setDate(QDate(tday->year, tday->month, tday->day));
  m_eqDateTime.setTime(QTime(tday->hour - 1, tday->minute, 0));
  if (!m_timer)
  {
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(update()));
    // 3 seconds = 1 EQ minute
    m_timer->start(3*1000, false);
  }

  emit syncDateTime(m_eqDateTime);
}

void DateTimeMgr::update()
{
  if (!m_eqDateTime.isValid())
    return;

  m_eqDateTime = m_eqDateTime.addSecs(60);
  emit updatedDateTime(m_eqDateTime);
}

#include "datetimemgr.moc"

