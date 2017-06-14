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

#include <sys/types.h>
#include <stdint.h>

#include <qobject.h>
#include <qdatetime.h>

struct timeOfDayStruct;

class DateTimeMgr : public QObject
{
 Q_OBJECT

 public:
  DateTimeMgr(QObject* parent = 0, const char* name = 0);
  virtual ~DateTimeMgr();
  const QDateTime& eqDateTime() const;
  const QDateTime& updatedDateTime();

 public slots:
  void timeOfDay(const uint8_t* tday);
  void update();

 signals:
  void syncDateTime(const QDateTime& dt);
  void updatedDateTime(const QDateTime& dt);

 protected:

 private:
  QTimer* m_timer;
  QDateTime m_eqDateTime;
};

inline const QDateTime& DateTimeMgr::eqDateTime() const 
{ 
  return m_eqDateTime; 
}

inline const QDateTime& DateTimeMgr::updatedDateTime() 
{
  return m_eqDateTime; 
}

#endif // _DATETIMEMGR_H_
