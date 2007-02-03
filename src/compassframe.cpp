/*
 * compassframe.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 *
 *  Copyright 2003-2007 by the respective ShowEQ Developers
 */

#include <qfont.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QLabel>

#include "main.h"
#include "compassframe.h"

CompassFrame::CompassFrame(Player* player, QWidget* parent, const char* name)
  : SEQWindow("Compass", "ShowEQ - Compass", parent, name)
{
  Q3VBoxLayout* layout = new Q3VBoxLayout(boxLayout());
  m_compass = new Compass (this, "compass");
  layout->addWidget(m_compass);
  Q3HBox* coordsbox = new Q3HBox(this);
  layout->addWidget(coordsbox);
  m_compass->setFixedWidth(120);
  m_compass->setFixedHeight(120);

  for(int a=0;a<2;a++) 
  {
    if((a+showeq_params->retarded_coords)%2 == 0) 
    {
      // Create the x: label
      QLabel *labelx = new QLabel(showeq_params->retarded_coords?"E/W:":"X:",
				  coordsbox);
      labelx->setFixedHeight(labelx->sizeHint().height());
      labelx->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
      
      // Create the xpos label
      m_x = new QLabel("----",coordsbox);
      m_x->setFixedHeight(m_x->sizeHint().height());
      m_x->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    } 
    else 
    {
      // Create the y: label
      QLabel *labely = new QLabel(showeq_params->retarded_coords?"N/S:":"Y:",
				  coordsbox);
      labely->setFixedHeight(labely->sizeHint().height());
      labely->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
      
      // Create the ypos label
      m_y = new QLabel("----",coordsbox);
      m_y->setFixedHeight(m_y->sizeHint().height());
      m_y->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    }
   }
  
  // Create the z: label
  QLabel *labelz = new QLabel("Z:",coordsbox);
  labelz->setFixedHeight(labelz->sizeHint().height());
  labelz->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  
  // Create the zpos label
  m_z = new QLabel("----",coordsbox);
  m_z->setFixedHeight(m_z->sizeHint().height());      
  m_z->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

  // connect
  connect(player, SIGNAL(posChanged(int16_t,int16_t,int16_t,
				    int16_t,int16_t,int16_t,int32_t)), 
	  this, SLOT(posChanged(int16_t,int16_t,int16_t,
				int16_t,int16_t,int16_t,int32_t)));

  // initialize compass
  m_compass->setPos(player->x(), player->y(), player->z());
  m_compass->setHeading(player->headingDegrees());
}

CompassFrame::~CompassFrame()
{
}

void CompassFrame::selectSpawn(const Item* item)
{
   if (item)
     m_compass->setTargetPos(item->x(), item->y(), item->z());
   else
     m_compass->clearTarget();
}

void CompassFrame::posChanged(int16_t x, int16_t y, int16_t z,
			      int16_t /*deltaX*/, int16_t /*deltaY*/, int16_t /*deltaZ*/,
			      int32_t heading)
{
  // set compass info
  m_compass->setPos(x, y, z);
  m_compass->setHeading(heading);

  // set position labels
  m_x->setText(QString::number(x));
  m_y->setText(QString::number(y));
  m_z->setText(QString::number(z));
}

#ifndef QMAKEBUILD
#include "compassframe.moc"
#endif

