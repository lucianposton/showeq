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
#include <qdockwindow.h>

class QPopupMenu;

class SEQWindow : public QDockWindow
{
   Q_OBJECT

 public:
   SEQWindow(const QString prefName, const QString caption,
	    QWidget* parent = 0, const char* name = 0, WFlags f = 0);
  ~SEQWindow();

  virtual QPopupMenu* menu();

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

