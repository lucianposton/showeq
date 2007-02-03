/*
 * filternotifications.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 * 
 * Portions Copyright 2003-2007 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

#include "filternotifications.h"
#include "filtermgr.h"
#include "spawn.h"
#include "main.h"

#include <stdio.h>

#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>

FilterNotifications::FilterNotifications(QObject* parent, const char* name)
  : QObject(parent, name),
    m_useSystemBeep(false),
    m_useCommands(false)
{
  m_useSystemBeep = 
    pSEQPrefs->getPrefBool("SystemBeep", "Filters", m_useSystemBeep);
  m_useCommands = 
    pSEQPrefs->getPrefBool("EnableCommands", "Filters", 
			   m_useCommands);
}

FilterNotifications::~FilterNotifications()
{
}

void FilterNotifications::setUseSystemBeep(bool val)
{
  m_useSystemBeep = val;

  pSEQPrefs->setPrefBool("SystemBeep", "Filters", m_useSystemBeep);
}

void FilterNotifications::setUseCommands(bool val)
{
  m_useCommands = val;

  pSEQPrefs->setPrefBool("EnableCommands", "Filters", m_useCommands);
}

void FilterNotifications::addItem(const Item* item)
{
  uint32_t filterFlags = item->filterFlags();

  // first handle alert
  if (filterFlags & FILTER_FLAG_ALERT)
    handleAlert(item, "SpawnAudioCommand", "Spawned");

  // then the rest of the filters
  if (filterFlags & FILTER_FLAG_LOCATE)
    handleAlert(item, "LocateSpawnAudioCommand", "Locate Spawned");
  if (filterFlags & FILTER_FLAG_CAUTION)
    handleAlert(item, "CautionSpawnAudioCommand", "Caution Spawned");
  if (filterFlags & FILTER_FLAG_HUNT)
    handleAlert(item, "HuntSpawnAudioCommand", "Hunt Spawned");
  if (filterFlags & FILTER_FLAG_DANGER)
    handleAlert(item, "DangerSpawnAudioCommand", "Danger Spawned");
}

void FilterNotifications::delItem(const Item* item)
{
  // first handle alert
  if (item->filterFlags() & FILTER_FLAG_ALERT)
    handleAlert(item, "DeSpawnAudioCommand", "Despawned");
}

void FilterNotifications::killSpawn(const Item* item)
{
  // first handle alert
  if (item->filterFlags() & FILTER_FLAG_ALERT)
    handleAlert(item, "DeathAudioCommand", "Died");
}

void FilterNotifications::changeItem(const Item* item, uint32_t changeType)
{
  // if all has changed, it is effectively if not literally a new item
  if (changeType == tSpawnChangedALL)
    addItem(item);
}

void FilterNotifications::handleAlert(const Item* item, 
				      const QString& commandPref, 
				      const QString& cue)
{
  if (m_useSystemBeep)
    beep();

  if (m_useCommands)
    executeCommand(item, pSEQPrefs->getPrefString(commandPref, "Filters"), cue);
}

void FilterNotifications::beep(void)
{
  QApplication::beep();
}

void FilterNotifications::executeCommand(const Item* item, 
					 const QString& rawCommand,
					 const QString& audioCue)
{
  if (rawCommand.isEmpty())
    return;

  // time to cook the command line
  QString command = rawCommand;

  QRegExp nameExp("%n");
  QRegExp cueExp("%c");
  
  // get the name, and replace all occurrances of %n with the name
  QString name = item->transformedName();
  command.replace(nameExp, name);
  
  // now, replace all occurrances of %c with the audio cue
  command.replace(cueExp, audioCue);
  
  // fire off the command
  system ((const char*)command);
}

#ifndef QMAKEBUILD
#include "filternotifications.moc"
#endif





