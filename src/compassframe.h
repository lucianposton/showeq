/*
 * compassframe.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 */

#ifndef _COMPASSFRAME_H_
#define _COMPASSFRAME_H_

#include <q3hbox.h>
#include <q3vbox.h>
#include <qlabel.h>

#include "player.h"
#include "spawnshell.h"
#include "compass.h"
#include "seqwindow.h"

class CompassFrame : public SEQWindow
{
  Q_OBJECT

 public:
  CompassFrame(Player* player, QWidget* parent = 0, const char* name = 0);
  virtual ~CompassFrame();

  Compass* compass() { return m_compass; }

 public slots:
  void selectSpawn(const Item* item);
  void posChanged(int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,int32_t);
  
 private:
  Compass* m_compass;
  QLabel* m_x;
  QLabel* m_y;
  QLabel* m_z;
};

#endif // _COMPASSFRAME_H_
