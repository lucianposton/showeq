/*
 * zonemgr.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef ZONEMGR_H
#define ZONEMGR_H

#include <qobject.h>
#include <qstring.h>

#include "point.h"

//----------------------------------------------------------------------
// forward declarations
struct ClientZoneEntryStruct;
struct ServerZoneEntryStruct;
struct zoneChangeStruct;
struct newZoneStruct;
struct zonePointsStruct;
struct zonePointStruct;

class ZoneMgr : public QObject
{
  Q_OBJECT

 public:
  ZoneMgr(QObject* parent = 0, const char* name =0);

  QString zoneNameFromID(uint16_t zoneId);
  QString zoneLongNameFromID(uint16_t zoneId);
  bool isZoning() const { return m_zoning; }
  const QString& shortZoneName() const { return m_shortZoneName; }
  const QString& longZoneName() const { return m_longZoneName; }
  const Point3D<int16_t>& safePoint() const { return m_safePoint; }
  float zoneExpMultiplier() { return m_zone_exp_multiplier; }
  const zonePointStruct* zonePoint(uint32_t zoneTrigger);

 public slots:
  void saveZoneState(void);
  void restoreZoneState(void);

 protected slots:
  void zoneEntryClient(const uint8_t* zsentry, size_t, uint8_t);
  void zonePlayer(const uint8_t* zsentry);
  void zoneEntryServer(const uint8_t* zsentry, size_t, uint8_t);
  void zoneChange(const uint8_t* zoneChange, size_t, uint8_t);
  void zoneNew(const uint8_t* zoneNew, size_t, uint8_t);
  void zonePoints(const uint8_t* zp, size_t, uint8_t);

 signals:
  void zoneBegin();
  void zoneBegin(const QString& shortZoneName);
  void zoneBegin(const ClientZoneEntryStruct* zsentry, size_t len, uint8_t dir);
  void zoneBegin(const ServerZoneEntryStruct* zsentry, size_t len, uint8_t dir);
  void zoneChanged(const QString& shortZoneName);
  void zoneChanged(const zoneChangeStruct*, size_t, uint8_t);
  void zoneEnd(const QString& shortZoneName, const QString& longZoneName);
  
 private:
  QString m_longZoneName;
  QString m_shortZoneName;
  bool m_zoning;
  Point3D<int16_t>  m_safePoint;
  float m_zone_exp_multiplier;
  size_t m_zonePointCount;
  zonePointStruct* m_zonePoints;
};

#endif // ZONEMGR
