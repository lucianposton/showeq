/*
 * datetimemgr.h
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
 */

#ifndef _DATETIMEMGR_H_
#define _DATETIMEMGR_H_

#include <qobject.h>
#include <qdatetime.h>

struct timeOfDayStruct;

class DateTimeMgr : public QObject
{
 Q_OBJECT

 public:
  DateTimeMgr(QObject* parent = 0, const char* name = 0);
  const QDateTime& eqDateTime() const;
  const QDateTime& updatedDateTime();
  int updateFrequency();
  void setUpdateFrequency(int seconds); // 3 seconds = 1 EQ minute

 public slots:
  void timeOfDay(const timeOfDayStruct *tday);
  void update();

 signals:
  void syncDateTime(const QDateTime& dt);
  void updatedDateTime(const QDateTime& dt);

 protected:

 private:
  int m_updateFrequency;
  QTimer* m_timer;
  QDateTime m_eqDateTime;
  QDateTime m_refDateTime;
};

inline const QDateTime& DateTimeMgr::eqDateTime() const 
{ 
  return m_eqDateTime; 
}

inline const QDateTime& DateTimeMgr::updatedDateTime() 
{
  update(); 
  return m_eqDateTime; 
}

inline int DateTimeMgr::updateFrequency() 
{ 
  return m_updateFrequency / 1000; 
}

#endif // _DATETIMEMGR_H_
