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
    m_updateFrequency(60 * 1000),
    m_timer(0)
{
}

void DateTimeMgr::setUpdateFrequency(int seconds)
{
  // set the new frequency (in ms)
  m_updateFrequency = seconds * 1000;

  if (m_timer)
  {
    // update the current time
    update();

    // set the timer to the new interval
    m_timer->changeInterval(m_updateFrequency);
  }
}

void DateTimeMgr::timeOfDay(const timeOfDayStruct* tday)
{
#if (QT_VERSION > 0x030100)
  m_refDateTime = QDateTime::currentDateTime(Qt::UTC);
#else
  m_refDateTime = QDateTime::currentDateTime();
#endif
  m_eqDateTime.setDate(QDate(tday->year, tday->month, tday->day));
  m_eqDateTime.setTime(QTime(tday->hour - 1, tday->minute, 0));
  if (!m_timer)
  {
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(update()));
    m_timer->start(m_updateFrequency, false);
  }

  emit syncDateTime(m_eqDateTime);
}

void DateTimeMgr::update()
{
#if (QT_VERSION > 0x030100)
  const QDateTime& current = QDateTime::currentDateTime(Qt::UTC);
#else
  const QDateTime& current = QDateTime::currentDateTime();
#endif

  int secs = m_refDateTime.secsTo(current);
  if (secs)
  {
    m_eqDateTime = m_eqDateTime.addSecs(secs * 20);
    m_refDateTime = current;
    emit updatedDateTime(m_eqDateTime);
  }
}

