/*
 * point.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sf.net/
 */

// Original Author: Zaphod (dohpaz@users.sourceforge.net)
//   interfaces modeled after QPoint and QPointArray interface
//   but for 3D data of arbitraty type T.

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects. 
//

#ifndef __POINT_H_
#define __POINT_H_

#include <stdint.h>
#include <math.h>
#include <stdarg.h>

#include <qpoint.h>
#include <qarray.h>

template <class _T>
class Point3D 
{
 public:
  typedef _T dimension_type;

  // constructors
  Point3D();
  Point3D(_T xPos, _T yPos, _T zPos);
  Point3D(const Point3D& point);

  // virtual destructor
  virtual ~Point3D();

  // operators
  Point3D& operator=(const Point3D& point);
  Point3D& operator+=(const Point3D& point);
  Point3D& operator-=(const Point3D& point);
  Point3D& operator*=(int coef);
  Point3D& operator*=(double coef);
  Point3D& operator/=(int coef);
  Point3D& operator/=(double coef);
  bool operator==(const Point3D& point) const;

  // get methods
  _T xPos() const { return m_xPos; }
  _T yPos() const { return m_yPos; }
  _T zPos() const { return m_zPos; }

  // alternate get methods
  _T x() const { return m_xPos; }
  _T y() const { return m_yPos; }
  _T z() const { return m_zPos; }

  // test method
  bool isNull() const;

  // set methods
  void setPoint(_T xPos, _T yPos, _T zPos)
    { m_xPos = xPos; m_yPos = yPos; m_zPos = zPos; }
  void setPoint(const Point3D& point)
    { m_xPos = point.m_xPos; m_yPos = point.m_yPos; m_zPos = point.m_zPos; }
  void setXPos(_T xPos) { m_xPos = xPos; }
  void setYPos(_T yPos) { m_yPos = yPos; }
  void setZPos(_T zPos) { m_zPos = zPos; }

  // utility methods
  
  // add values to point
  void addPoint(_T xPos, _T yPos, _T zPos);

  // retrieve a QPoint of this point
  QPoint qpoint() const { return QPoint(xPos(), yPos()); }
  
  // retrieve offset point
  QPoint& offsetPoint(const QPoint& centerPoint, double ratio);
  QPoint& inverseOffsetPoint(const QPoint& centerPoint, double ratio);

  // Calculate distance in 2 space ignoring Z dimension
  uint32_t calcDist2DInt(_T xPos, _T yPos) const;
  uint32_t calcDist2DInt(const Point3D& point) const
    { return calcDist2DInt(point.xPos(), point.yPos()); }
  uint32_t calcDist2DInt(const QPoint& point) const
    { return calcDist2DInt(point.x(), point.y()); }
  double calcDist2D(_T xPos, _T yPos) const;
  double calcDist2D(const Point3D& point) const
    { return calcDist2D(point.xPos(), point.yPos()); }
  double calcDist2D(const QPoint& point) const
    { return calcDist2D(point.x(), point.y()); }

  // Calculate distance in 3 space
  uint32_t calcDistInt(_T xPos, _T yPos, _T zPos = 0) const;
  uint32_t calcDistInt(const Point3D& point) const
    { return calcDistInt(point.xPos(), point.yPos(), point.zPos()); }
  double calcDist(_T xPos, _T yPos, _T zPos = 0) const;
  double calcDist(const Point3D& point) const
    { return calcDist(point.xPos(), point.yPos(), point.zPos()); }

 protected:
  // position information
  _T m_xPos;
  _T m_yPos;
  _T m_zPos;
};

// default constructor
template <class _T> inline
Point3D<_T>::Point3D<_T>()
{
  setPoint(0, 0, 0);
}

// copy constructor
template <class _T> inline
Point3D<_T>::Point3D<_T>(const Point3D<_T>& point)
{
  setPoint(point.xPos(), point.yPos(), point.zPos());
}

// convenience constructor
template <class _T> inline
Point3D<_T>::Point3D<_T>(_T xPos, _T yPos, _T zPos)
{
  setPoint(xPos, yPos, zPos);
}

// obligatory virtual destructor
template <class _T> inline
Point3D<_T>::~Point3D<_T>()
{
}

// assignment operator
template <class _T> inline
Point3D<_T>& Point3D<_T>::operator=(const Point3D<_T>& point)
{
  setPoint(point.xPos(), point.yPos(), point.zPos());
  return *this;
}

// math operators
template <class _T> inline
Point3D<_T>& Point3D<_T>::operator+=(const Point3D<_T>& point)
{
  m_xPos += point.xPos();
  m_yPos += point.yPos();
  m_zPos += point.zPos();
  return *this;
}

template <class _T> inline
Point3D<_T>& Point3D<_T>::operator-=(const Point3D<_T>& point)
{
  m_xPos -= point.xPos();
  m_yPos -= point.yPos();
  m_zPos -= point.zPos();
  return *this;
}

// scaling operators
template <class _T> inline
Point3D<_T>& Point3D<_T>::operator*=(int coef)
{
  m_xPos *= coef;
  m_yPos *= coef;
  m_zPos *= coef;
  return *this;
}

template <class _T> inline
Point3D<_T>& Point3D<_T>::operator*=(double coef)
{
  m_xPos *= coef;
  m_yPos *= coef;
  m_zPos *= coef;
  return *this;
}

template <class _T> inline
Point3D<_T>& Point3D<_T>::operator/=(int coef)
{
  m_xPos /= coef;
  m_yPos /= coef;
  m_zPos /= coef;
  return *this;
}

template <class _T> inline
Point3D<_T>& Point3D<_T>::operator/=(double coef)
{
  m_xPos /= coef;
  m_yPos /= coef;
  m_zPos /= coef;
  return *this;
}

// equivalency operator
template <class _T> inline
bool Point3D<_T>::operator==(const Point3D<_T>& point) const
{
  return ((xPos() == point.xPos()) &&
	  (yPos() == point.yPos()) &&
	  (zPos() == point.zPos()));
}

// returns true if all 3 dimensions are 0
template <class _T> inline
bool Point3D<_T>::isNull() const
{ 
  return ((xPos() == 0) && (yPos() == 0) && (zPos() == 0));
}

// utility methods

template <class _T> inline
void Point3D<_T>::addPoint(_T xPos, _T yPos, _T zPos)
{
  m_xPos += xPos;
  m_yPos += yPos;
  m_zPos += zPos;
}

template <class _T> inline
QPoint& Point3D<_T>::offsetPoint(const QPoint& centerPoint, double ratio)
{ 
  return QPoint((centerPoint.x() - (int)(xPos() / ratio)),
		(centerPoint.y() - (int)(yPos() / ratio)));
}

template <class _T> inline
QPoint& Point3D<_T>::inverseOffsetPoint(const QPoint& centerPoint, double ratio)
{
  return QPoint(int(rint((centerPoint.x() - xPos()) * ratio)),
		int(rint((centerPoint.y() - yPos()) * ratio)));
}

// Calculate distance using/returning int in 2 space ignoring any Z dimension
template <class _T> inline
uint32_t Point3D<_T>::calcDist2DInt(_T x, _T y) const
{
  int32_t xDiff = xPos() - x;
  int32_t yDiff = yPos() - y;
  
  return uint32_t(sqrt(double((yDiff * yDiff) + (xDiff * xDiff))));
}

// Calculate distance using/returning double in 2 space ignoring any Z dimension
template <class _T> inline
double Point3D<_T>::calcDist2D(_T x, _T y) const
{
  double xDiff = double(xPos()) - double(x);
  double yDiff = double(yPos()) - double(y);
  return sqrt((xDiff * xDiff) + (yDiff * yDiff));
}

// Calculate distance using/returning int
template <class _T> inline
uint32_t Point3D<_T>::calcDistInt(_T x, _T y, _T z) const
{
  int32_t xDiff = xPos() - x;
  int32_t yDiff = yPos() - y;
  int32_t zDiff = zPos() - z;

  return uint32_t(sqrt(double((yDiff * yDiff) + 
			      (xDiff * xDiff) + 
			      (zDiff * zDiff))));
}

// Calculate distance using/returning double
template <class _T> inline
double Point3D<_T>::calcDist(_T x, _T y, _T z) const
{
  double xDiff = double(xPos()) - double(x);
  double yDiff = double(yPos()) - double(y);
  double zDiff = double(zPos()) - double(z);

  return sqrt((xDiff * xDiff) + (yDiff * yDiff) + (zDiff * zDiff));
}

// Point3DArray
template <class _T>
class Point3DArray : public QArray<Point3D<_T> >
{
 public:
  Point3DArray() {};
  Point3DArray(int size) : QArray<Point3D<_T> > (size) {}
  Point3DArray(const Point3DArray<_T>& array) : QArray<Point3D<_T> > (array) {}
  Point3DArray(uint32_t nPoints, const _T* points);
  ~Point3DArray() {};

  Point3DArray<_T>& operator=(const Point3DArray<_T>& array)
    { return (Point3DArray<_T>&)assign(array); }
  Point3DArray<_T> copy() const
    { Point3DArray<_T> tmp; return *((Point3DArray<_T>*)&tmp.duplicate(*this)); }

  QRect boundingRect() const;

  void point(uint32_t i, _T* xPos, _T* yPos, _T* zPos) const;
  const Point3D<_T>& point( uint32_t i) const;
  void setPoint(uint32_t i, _T xPos, _T yPos, _T zPos);
  void setPoint(uint32_t i, const Point3D<_T>& p);
  bool setPoints(uint32_t nPoints, const _T* points);
  bool setPoints(uint32_t nPoints, _T firstx, _T firsty, _T firstz, ...);
  bool putPoints(uint32_t index, uint32_t nPoints, const _T* points);
  bool putPoints(uint32_t index, uint32_t nPoints, _T firstx, _T firsty, _T firstz, ...);
  QPointArray getQPointArray();
};

template <class _T> inline 
Point3DArray<_T>::Point3DArray<_T>(uint32_t nPoints, const _T* points)
{
  setPoints(nPoints, points);
}

template <class _T> inline
QRect Point3DArray<_T>::boundingRect() const
{
  if (isEmpty())
    return QRect(0, 0, 0, 0);

  Point3D<_T>* d = data();
  _T minX, maxX, minY, maxY;

  minX = maxX = d->xPos();
  minY = maxY = d->yPos();
  
  uint32_t i;
  for (++d, i = 1;
       i < size();
       i++, d++)
  {
    if (d->xPos() < minX)
      minX = d->xPos();
    else if (d->xPos() > maxX)
      maxX = d->xPos();
    if (d->yPos() < minY)
      minY = d->yPos();
    else if (d->yPos() > maxY)
      maxY = d->yPos();
  }

  return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

template <class _T> inline
void Point3DArray<_T>::point(uint32_t index, _T* xPos, _T* yPos, _T* zPos) const
{
  Point3D<_T> p = QArray<Point3D<_T> >::at(index);
  *xPos = p.xPos();
  *yPos = p. yPos();
  *zPos = p. zPos();
}

template <class _T> inline 
const Point3D<_T>& Point3DArray<_T>::point(uint32_t index) const
{
  return QArray<Point3D<_T> >::at(index);
}

template <class _T> inline
void Point3DArray<_T>::setPoint(uint32_t index, _T xPos, _T yPos, _T zPos)
{
  QArray<Point3D<_T> >::at(index) = Point3D<_T>(xPos, yPos, zPos);
}

template <class _T> inline 
void Point3DArray<_T>::setPoint(uint32_t index, const Point3D<_T>& p)
{
  QArray<Point3D<_T> >::at(index) = p;
}

template <class _T> inline
bool Point3DArray<_T>::setPoints(uint32_t nPoints, const _T* points)
{
  if (!resize(nPoints))
    return false;

  for (uint32_t i = 0; 
       nPoints;
       nPoints--, i++, points += 3)
    setPoint(i, *points, *(points + 1), *(points + 2));
	 
  return true;
}

template <class _T> inline
bool Point3DArray<_T>::setPoints(uint32_t nPoints, 
				 _T firstx, _T firsty, _T firstz, ...)
{
  if (!resize(nPoints))
    return false;

  setPoint( 0, firstx, firsty, firstz);

  va_list ap;
  va_start(ap, firstz);

  _T xPos, yPos, zPos;

  uint32_t i;
  for (i = 1, --nPoints;
       nPoints;
       nPoints--, i++)
  {
    xPos = va_arg(ap, _T);
    yPos = va_arg(ap, _T);
    zPos = va_arg(ap, _T);
    
    setPoint(i, xPos, yPos, zPos);
  }

  va_end(ap);

  return true;
}

template <class _T> inline
bool Point3DArray<_T>::putPoints(uint32_t index, uint32_t nPoints, const _T* points)
{
  if ((index + nPoints) > size())
    if (!resize(index + nPoints))
      return false;

  for (uint32_t i = index; 
       nPoints;
       nPoints--, i++, points += 3)
    setPoint(i, *points, *(points + 1), *(points + 2));
  
  return true;
}

template <class _T> inline
bool Point3DArray<_T>::putPoints(uint32_t index, uint32_t nPoints, 
			    _T firstx, _T firsty, _T firstz, ...)
{
  if ((index + nPoints) > size())
    if (!resize(index + nPoints))
      return false;

  setPoint( 0, firstx, firsty, firstz);

  va_list ap;
  va_start(ap, firstz);

  _T xPos, yPos, zPos;

  uint32_t i;
  for (i = index + 1, --nPoints;
       nPoints;
       nPoints--, i++)
  {
    xPos = va_arg(ap, _T);
    yPos = va_arg(ap, _T);
    zPos = va_arg(ap, _T);
    
    setPoint(i, xPos, yPos, zPos);
  }

  va_end(ap);

  return true;
}

template <class _T> inline
QPointArray Point3DArray<_T>::getQPointArray()
{
  // create a temporary QPointArray of the same size as this array
  QPointArray tmp(size());

  // copy each Point3D<_T> as a QPoint into the temporary QPointArray
  for (uint32_t i = 0; i < size(); i++)
    tmp.setPoint(i, point(i).qpoint());

  // return the temporary QPointArray
  return tmp;
}

#endif // __POINT_H_
