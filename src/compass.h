/*
 * compass.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 *
 *  Copyright 2002-2007 by the respective ShowEQ Developers
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#ifndef EQCOMPASS_H
#define EQCOMPASS_H

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif

#include "point.h"

#include <qwidget.h>
#include <qsize.h>
//Added by qt3to4:
#include <QPaintEvent>

///////////////////////////////////////////
// type definitions
typedef Point3D<int16_t> CompassPoint;

class Compass : public QWidget
{
   Q_OBJECT

 public:
   Compass ( QWidget *parent=0, const char *name=0);
   QSize sizeHint() const; // preferred size
   QSizePolicy sizePolicy() const; // size policy

 public slots:
   void setHeading(int32_t degrees);
   void setPos(int16_t x, int16_t y, int16_t z);
   void setTargetPos(int x, int y, int z);
   void clearTarget(void);

 signals:
   void angleChanged (int);

 protected:
   void paintEvent (QPaintEvent *);

 private:
   void paintCompass ( QPainter * );
   void calcTargetHeading();
   QRect compassRect() const;
   int m_ang;
   double m_dSpawnAngle;
   CompassPoint m_cPlayer;
   CompassPoint m_cTarget;
};

#endif // EQCOMPASS_H
