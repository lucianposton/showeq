/*
 * seqwindow.cpp
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

#include "seqwindow.h"
#include "main.h"

SEQWindow::SEQWindow(const QString prefName, const QString caption,
		     QWidget* parent, const char* name, WFlags f)
  : QWidget(parent, name, f),
    m_preferenceName(prefName)
{
  // set the windows caption
  QWidget::setCaption(pSEQPrefs->getPrefString("Caption", preferenceName(),
					       caption));
  
  // restore the font
  restoreFont();
}

SEQWindow::~SEQWindow()
{
}


void SEQWindow::setCaption(const QString& text)
{
  // set the caption
  QWidget::setCaption(text);

  // set the preference
  pSEQPrefs->setPrefString("Caption", preferenceName(), caption());
}

void SEQWindow::setWindowFont(const QFont& font)
{
  // set the font preference
  pSEQPrefs->setPrefFont("Font", preferenceName(), font);

  // restore the font to the preference
  restoreFont();
}

void SEQWindow::restoreFont()
{
  // set the applications default font
  if (pSEQPrefs->isPreference("Font", preferenceName()))
  {
    // use the font specified in the preferences
    QFont font = pSEQPrefs->getPrefFont("Font", preferenceName());
    setFont( font);
  }
}

void SEQWindow::restoreSize()
{
  // retrieve the saved size information
  QSize s = pSEQPrefs->getPrefSize("WindowSize", preferenceName(), size());

  resize(s);
}

void SEQWindow::restorePosition()
{
  // retrieve the saved position information
  QPoint p = pSEQPrefs->getPrefPoint("WindowPos", preferenceName(), pos());

  // Move window to new position
  move(p);
}

void SEQWindow::savePrefs(void)
{
  if (pSEQPrefs->getPrefBool("SavePosition", "Interface", true))
  {
    // save the windows size and position information
    pSEQPrefs->setPrefSize("WindowSize", preferenceName(), size());
    pSEQPrefs->setPrefPoint("WindowPos", preferenceName(), pos());
  }
}
