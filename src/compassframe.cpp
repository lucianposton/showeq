/*
 * compassframe.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sf.net/
 */

#include <qfont.h>

#include "main.h"
#include "compassframe.h"

CompassFrame::CompassFrame(EQPlayer* player, QWidget* parent, const char* name)
  : QVBox(parent, name)
{
  QString section = "Compass";

  QVBox::setCaption(pSEQPrefs->getPrefString("Caption", section,
					     "ShowEQ - Compass"));
  
  m_compass = new Compass (this, "compass");
  QHBox* coordsbox = new QHBox(this);
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
      labelx->setFont(QFont("Helvetica", 10, QFont::Bold));
      labelx->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
      
      // Create the xpos label
      m_xPos = new QLabel("----",coordsbox);
      m_xPos->setFixedHeight(m_xPos->sizeHint().height());
      m_xPos->setFont(QFont("Helvetica", 10));
      m_xPos->setAlignment(QLabel::AlignRight|QLabel::AlignVCenter);
    } 
    else 
    {
      // Create the y: label
      QLabel *labely = new QLabel(showeq_params->retarded_coords?"N/S:":"Y:",
				  coordsbox);
      labely->setFixedHeight(labely->sizeHint().height());
      labely->setFont(QFont("Helvetica", 10, QFont::Bold));
      labely->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
      
      // Create the ypos label
      m_yPos = new QLabel("----",coordsbox);
      m_yPos->setFixedHeight(m_yPos->sizeHint().height());
      m_yPos->setFont(QFont("Helvetica", 10));
      m_yPos->setAlignment(QLabel::AlignRight|QLabel::AlignVCenter);
    }
   }
  
  // Create the z: label
  QLabel *labelz = new QLabel("Z:",coordsbox);
  labelz->setFixedHeight(labelz->sizeHint().height());
  labelz->setFont(QFont("Helvetica", 10, QFont::Bold));
  labelz->setAlignment(QLabel::AlignLeft|QLabel::AlignVCenter);
  
  // Create the zpos label
  m_zPos = new QLabel("----",coordsbox);
  m_zPos->setFixedHeight(m_zPos->sizeHint().height());      
  m_zPos->setFont(QFont("Helvetica", 10));
  m_zPos->setAlignment(QLabel::AlignRight|QLabel::AlignVCenter);

  // connect
  connect(player, SIGNAL(posChanged(int16_t,int16_t,int16_t,
				    int16_t,int16_t,int16_t,int32_t)), 
	  this, SLOT(posChanged(int16_t,int16_t,int16_t,
				int16_t,int16_t,int16_t,int32_t)));

  // initialize compass
  m_compass->setPos(player->getPlayerX(), player->getPlayerY());
  m_compass->setHeading(player->getPlayerHeading());
}

CompassFrame::~CompassFrame()
{
}

void CompassFrame::setCaption(const QString& text)
{
  // set the caption
  QVBox::setCaption(text);

  // set the preference
  pSEQPrefs->setPrefString("Caption", "Compass", caption());
}

void CompassFrame::selectSpawn(const Item* item)
{
   if (item)
     m_compass->setTargetPos(item->xPos(), item->yPos());
   else
     m_compass->clearTarget();
}

void CompassFrame::posChanged(int16_t xPos, int16_t yPos, int16_t zPos,
			      int16_t deltaX, int16_t deltaY, int16_t deltaZ,
			      int32_t heading)
{
  // set compass info
  m_compass->setPos(xPos, yPos);
  m_compass->setHeading(heading);

  // set position labels
  m_xPos->setText(QString::number(xPos));
  m_yPos->setText(QString::number(yPos));
  m_zPos->setText(QString::number(zPos));
}
