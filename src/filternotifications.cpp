/*
 * filternotifications.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 * 
 * Portions Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

#include "filternotifications.h"
#include "filtermgr.h"
#include "spawn.h"
#include "main.h"
#include "diagnosticmessages.h"
#include "packetcommon.h"

#include <stdio.h>

#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>

bool isCommandActive(const QString& name)
{
    return pSEQPrefs->getPrefBool(name + "Enabled", "Filters", true);
}

FilterNotifications::FilterNotifications(QObject* parent, const char* name)
  : QObject(parent, name),
    m_useSystemBeep(false),
    m_useCommands(false),
    m_isInitialZoneSpawn(false)
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

void FilterNotifications::initialZoneSpawn(bool isInitialZoneSpawn)
{
    m_isInitialZoneSpawn = isInitialZoneSpawn;
}

void FilterNotifications::listAfkCheck(const uint8_t* data, size_t, uint8_t dir)
{
    if (dir == DIR_Client)
        return;

    const List_Struct* list = (const List_Struct*)data;
    if (list->op != 2)
        return;

    handleAlert("none", "ListAfkCheckCommand", "none");
}

void FilterNotifications::addItem(const Item* item)
{
    if (m_isInitialZoneSpawn)
        return;

  uint32_t filterFlags = item->filterFlags();

  // first handle alert
  if (filterFlags & FILTER_FLAG_ALERT)
    handleAlert(item->transformedName(), "SpawnAudioCommand", "Spawned");

  // then PvP
  if (filterFlags & FILTER_FLAG_PVP)
    handleAlert(item->transformedName(), "PvPSpawnAudioCommand", "PvP Spawned");

  // then the rest of the filters
  if (filterFlags & FILTER_FLAG_LOCATE)
    handleAlert(item->transformedName(), "LocateSpawnAudioCommand", "Locate Spawned");
  if (filterFlags & FILTER_FLAG_CAUTION)
    handleAlert(item->transformedName(), "CautionSpawnAudioCommand", "Caution Spawned");
  if (filterFlags & FILTER_FLAG_HUNT)
    handleAlert(item->transformedName(), "HuntSpawnAudioCommand", "Hunt Spawned");
  if (filterFlags & FILTER_FLAG_DANGER)
    handleAlert(item->transformedName(), "DangerSpawnAudioCommand", "Danger Spawned");
}

void FilterNotifications::delItem(const Item* item)
{
  // first handle alert
  if (item->filterFlags() & FILTER_FLAG_ALERT)
    handleAlert(item->transformedName(), "DeSpawnAudioCommand", "Despawned");
}

void FilterNotifications::killSpawn(const Item* item)
{
  // first handle alert
  if (item->filterFlags() & FILTER_FLAG_ALERT)
    handleAlert(item->transformedName(), "DeathAudioCommand", "Died");
}

void FilterNotifications::changeItem(const Item* item, uint32_t changeType)
{
  // if all has changed, it is effectively if not literally a new item
  if (changeType == tSpawnChangedALL)
    addItem(item);
}

void FilterNotifications::handleAlert(const QString& name, 
				      const QString& commandPref, 
				      const QString& cue)
{
  if (m_useSystemBeep)
    beep();

  if (m_useCommands && isCommandActive(commandPref))
    executeCommand(name, pSEQPrefs->getPrefString(commandPref, "Filters"), cue);
}

void FilterNotifications::beep(void)
{
  QApplication::beep();
}

void FilterNotifications::executeCommand(const QString& name, 
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
  command.replace(nameExp, name);
  
  // now, replace all occurrances of %c with the audio cue
  command.replace(cueExp, audioCue);
  
  // fire off the command
  if (-1 == system((const char*)command))
  {
      seqWarn("FilterNotifications::executeCommand(): Failed to create child process");
  }
}

#include "filternotifications.moc"




