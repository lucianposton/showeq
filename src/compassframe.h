/*
 * compassframe.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 */

#ifndef _COMPASSFRAME_H_
#define _COMPASSFRAME_H_

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>

#include "player.h"
#include "spawnshell.h"
#include "compass.h"

class CompassFrame : public QVBox
{
  Q_OBJECT

 public:
  CompassFrame(EQPlayer* player, QWidget* parent = 0, const char* name = 0);
  virtual ~CompassFrame();

  Compass* compass() { return m_compass; }

 public slots:
  void selectSpawn(const Item* item);
  void posChanged(int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,int32_t);
  virtual void setCaption(const QString&);
  
 private:
  Compass* m_compass;
  QLabel* m_xPos;
  QLabel* m_yPos;
  QLabel* m_zPos;
};

#endif // _COMPASSFRAME_H_
