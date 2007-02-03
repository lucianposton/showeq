/*
 * seqwindow.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef SEQWINDOW_H
#define SEQWINDOW_H

#include <qwidget.h>
#include <qstring.h>
#include <q3dockwindow.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <QMouseEvent>

class Q3PopupMenu;

class SEQWindow : public Q3DockWindow
{
   Q_OBJECT

 public:
   SEQWindow(const QString prefName, const QString caption,
	    QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0);
  ~SEQWindow();

  virtual Q3PopupMenu* menu();

  const QString& preferenceName() const { return m_preferenceName; }

 public slots:
   virtual void setCaption(const QString&);
   virtual void setWindowFont(const QFont&);
   virtual void restoreSize();
   virtual void restorePosition();
   virtual void restoreFont();
   virtual void savePrefs(void);
  
   virtual void mousePressEvent(QMouseEvent* e);

 private:
  QString m_preferenceName;
};

#endif // SEQWINDOW_H

