/*
 * filternotifications.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 * 
 * Portions Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

#ifndef _FILTERNOTIFICATIONS_H_
#define _FILTERNOTIFICATIONS_H_

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif

#include <qobject.h>

class QString;

class Item;

class FilterNotifications : public QObject
{
  Q_OBJECT
 public:
  FilterNotifications(QObject* parent = 0, const char* name = 0);
  ~FilterNotifications();
  
  bool useSystemBeep() const { return m_useSystemBeep; }
  bool useCommands() const { return m_useCommands; }

 public slots:
   void setUseSystemBeep(bool val);
   void setUseCommands(bool val);
   
   void listAfkCheck(const uint8_t* data, size_t, uint8_t dir);
   void addItem(const Item* item);
   void delItem(const Item* item);
   void killSpawn(const Item* item);
   void changeItem(const Item* item, uint32_t changeType);
   void initialZoneSpawn(bool);

 protected:
   void handleAlert(const QString& name, 
		    const QString& commandPref, const QString& cue);
   void beep(void);
   void executeCommand(const QString& name, 
		       const QString& rawCommand,
		       const QString& audioCue);

 protected:
  bool m_useSystemBeep;
  bool m_useCommands;

 private:
  bool m_isInitialZoneSpawn;
};

#endif // _FILTERNOTIFICATIONS_H_

