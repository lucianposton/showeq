/*
 * compass.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
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

#include <qwidget.h>
#include <qsize.h>
#include <qpoint.h>

class Compass : public QWidget
{
   Q_OBJECT

 public:
   Compass ( QWidget *parent=0, const char *name=0);
   QSize sizeHint() const; // preferred size
   QSizePolicy sizePolicy() const; // size policy

 public slots:
   void setHeading(int32_t degrees);
   void setPos(int16_t x, int16_t y);
   void setTargetPos(int x, int y);
   void clearTarget(void);

 signals:
   void angleChanged (int);

 protected:
   void paintEvent (QPaintEvent *);

 private:
   void paintCompass ( QPainter * );
   void calcTargetHeading();
   QRect compassRect() const;
   int ang;
   double m_dSpawnAngle;
   QPoint m_cPlayer;
   QPoint m_cTarget;
};

#endif // EQCOMPASS_H
