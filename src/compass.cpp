/*
 * compass.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 *
 *  Copyright 2003-2007 by the respective ShowEQ Developers
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//
#include "compass.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <qfont.h>
//Added by qt3to4:
#include <QPaintEvent>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

Compass::Compass (QWidget *parent, const char *name)
  : QWidget ( parent, name )
{
   m_ang = 0;
   m_dSpawnAngle = -1;
}

const QRect compass_rect(-3, 0, 40, 6);

QSize Compass::sizeHint() const // preferred size
{
   return QSize(110, 110);
}

QSizePolicy Compass::sizePolicy() const // size policy
{
   return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void Compass::calcTargetHeading()
{
   int deltaX, deltaY;
   double quadRads, quadDegs;

   deltaX = m_cPlayer.x() - m_cTarget.x();
   deltaY = (m_cPlayer.y() - m_cTarget.y()) * -1;

   quadRads = atan2(deltaY,deltaX);

   // Convert radians to degrees
   quadDegs = (quadRads/(2 * M_PI)) * 360;

   // Account for stuff in the lower half of the circle
   if (quadDegs < 0)
   {
      quadDegs += 360.0;
   }

   m_dSpawnAngle = quadDegs;

   repaint ( compassRect(), FALSE);
}

QRect Compass::compassRect() const
{
   QRect r(0,0,width(),height());
   r.moveBottomLeft (rect().bottomLeft() );
   return r;
}

void Compass::setHeading(int32_t degrees)
{
   if (m_ang == ((360-degrees)+90))
     return;
   m_ang = (360-degrees)+90;
   repaint ( compassRect(), FALSE);
   emit angleChanged(m_ang);
}

void Compass::setPos(int16_t x, int16_t y, int16_t z)
{
   m_cPlayer.setPoint(x, y, z);
   
   if (m_dSpawnAngle > 0)
   {
      calcTargetHeading();
   }
}

void Compass::setTargetPos(int x,int y, int z)
{
   // Recalc the mob heading
   m_cTarget.setPoint(x,y,z);

   calcTargetHeading();
}

void Compass::clearTarget(void)
{
  m_dSpawnAngle = -1;
}

void Compass::paintCompass ( QPainter *p )
{
   QRect cr = compassRect();
   QPixmap pix (cr.size());
   QPainter tmp;
   pix.fill (this, cr.topLeft() );
   int pwd2 = pix.width() >> 1;
   int pwd4 = pix.width() >> 2;
   int phd2 = pix.height() >> 1;
   int phd4 = pix.height() >> 2;

   tmp.begin (&pix);

   tmp.setBrush(QColor(64, 64, 64));
   tmp.setPen(darkGray);
   tmp.drawEllipse(0,0,pix.width(), pix.height());
   tmp.setPen(gray);
   tmp.drawLine(0,phd2, pix.width(), phd2);
   tmp.drawLine(pwd2,0, pwd2, pix.height());
   tmp.drawLine(pwd4, phd4, 
		pix.width()-pwd4, pix.height()-phd4);
   tmp.drawLine(pix.width()-pwd4, phd4, 
		pwd4, pix.height()-(phd4));
   tmp.translate(pwd2, phd2);

   tmp.rotate (-m_ang);
   tmp.setBrush(blue);
   tmp.setPen(blue);
   tmp.drawLine(0-pwd4, 0, pwd2, 0);
   tmp.drawLine(0,0-phd4, 0, phd4);
   tmp.setBrush(red);
   tmp.setPen(red);
   tmp.drawLine(0, 1, pwd2, 1);
   tmp.drawLine(0, -1, pwd4, -1);
   tmp.setBrush(blue);
   tmp.setPen(red);
   tmp.drawEllipse (0-5, 0-5, 10, 10);
   tmp.setBrush(red);
   tmp.setPen(red);
   tmp.drawEllipse ((pwd2) -2, -2, 4, 4);

   if (m_dSpawnAngle > 0)
   {
      tmp.resetXForm();
      tmp.translate(pwd2, phd2);
      tmp.rotate(-m_dSpawnAngle);
      tmp.setPen(green);
      tmp.setBrush(green);
      tmp.drawEllipse(pwd2 -2, -2, 4, 4);
   }

   tmp.end();
   p->drawPixmap (cr.topLeft(), pix);
}

void Compass::paintEvent ( QPaintEvent *e )
{
   QRect updateR = e->rect();
   QPainter p;
   p.begin(this);
   if (updateR.intersects ( compassRect() ) )
     paintCompass (&p);
   p.end();
}

#ifndef QMAKEBUILD
#include "compass.moc"
#endif

