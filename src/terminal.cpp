/*
 * terminal.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#include "terminal.h"
#include "main.h"

//----------------------------------------------------------------------
// Constants

// ISO/ANSI/ECMA graphics codes, refer to ECMA-048 Section 8.3.117 SGR
//   available here http://www.ecma.ch/ecma1/STAND/ECMA-048.HTM
const char* defaultStyle = "\e[0m";
const char* typeStyles[] = 
  {
    "\e[1;32m",   // 0 - Guild
    NULL,         // 1
    "\e[0;36m",   // 2 - Group
    "\e[1;31m",   // 3 - Shout
    "\e[1;42m",   // 4 - Auction
    "\e[0;32m",   // 5 - OOC
    NULL,         // 6
    "\e[0;35m",   // 7 - Tell
    "\e[1;37m",   // 8 - Say
    NULL,         // 9
    NULL,         // 10
    "\e[5;31m",   // 11 - GM Say
    NULL,         // 12
    NULL,         // 13
    "\e[5;31m",   // 14 - GM Tell 
    "\e[1;36m",   // 15 - Raid
    NULL,         // 16 - Debug
    NULL,         // 17 - Info
    NULL,         // 18 - General
    NULL,         // 19 - Motd
    NULL,         // 20 - System
    NULL,         // 21 - Money
    NULL,         // 22 - Random
    NULL,         // 23 - Emote
    NULL,         // 24 - Time
    NULL,         // 25 - Spell
    NULL,         // 26 - Zone
    NULL,         // 27 - Inspect
    NULL,         // 28 - Player
    NULL,         // 29 - Consider
    "\e[1;37;45m",   // 30 - Alert
    "\e[1;37;41m",   // 31 - Danger
    "\e[1;37;43m",   // 32 - Caution
    "\e[1;37;46m",   // 33 - Hunt
    "\e[1;37;44m",   // 34 - Locate
  };

//----------------------------------------------------------------------
// Terminal
Terminal::Terminal(Messages* messages,
		   QObject* parent, const char* name)
  : QObject(parent, name),
    m_messages(messages),
    m_enabledTypes(0xFFFFFFFFFFFFFFFFULL),
    m_itemPattern("\022(\\d{5,7})\\w*-\\d+-\\d+-\\d+-\\d+-.{13}([^\022]+)\022"),    m_dateTimeFormat("hh:mm"),
    m_eqDateTimeFormat("ddd M/d/yyyy h:mm"),
    m_displayType(true),
    m_displayDateTime(false),
    m_displayEQDateTime(false),
    m_useColor(true)
{
  const QString preferenceName = "Terminal";
  m_enabledTypes = pSEQPrefs->getPrefUInt64("EnabledTypes", preferenceName, 
					    m_enabledTypes);
  m_dateTimeFormat = pSEQPrefs->getPrefString("DateTimeFormat",
					      preferenceName, 
					      m_dateTimeFormat);
  m_eqDateTimeFormat = pSEQPrefs->getPrefString("EQDateTimeFormat",
						preferenceName,
						m_eqDateTimeFormat);
  m_displayType = pSEQPrefs->getPrefBool("DisplayType", preferenceName,
					 m_displayType);
  m_displayDateTime = pSEQPrefs->getPrefBool("DisplayDateTime",
					     preferenceName, 
					     m_displayDateTime);
  m_displayEQDateTime = pSEQPrefs->getPrefBool("DisplayEQDateTime",
					       preferenceName,
					       m_displayEQDateTime);
  m_useColor = pSEQPrefs->getPrefBool("UseColor", preferenceName,
				      m_useColor);

  // connect to the Messages signal(s)
  connect(m_messages, SIGNAL(newMessage(const MessageEntry&)),
	  this, SLOT(newMessage(const MessageEntry&)));
}

Terminal::~Terminal()
{
}

void Terminal::setEnabledTypes(uint64_t types) 
{ 
  m_enabledTypes = types; 
  pSEQPrefs->setPrefUInt64("EnabledTypes", "Terminal", m_enabledTypes);
}

void Terminal::setEnabledShowUserFilters(uint32_t filters)
{
  m_enabledShowUserFilters = filters;
  // save the new setting
  pSEQPrefs->setPrefUInt("EnabledShowUserFilters", "Terminal", 
			 m_enabledShowUserFilters);
}

void Terminal::setEnabledHideUserFilters(uint32_t filters)
{
  m_enabledHideUserFilters = filters;
  // save the new setting
  pSEQPrefs->setPrefUInt("EnabledHideUserFilters", "Terminal", 
			 m_enabledHideUserFilters);
}

void Terminal::setDateTimeForamt(const QString& dateTime)
{
  m_dateTimeFormat = dateTime;
  pSEQPrefs->setPrefString("DateTimeFormat", "Terminal", m_dateTimeFormat);
}

void Terminal::setEQDateTimeFormat(const QString& dateTime)
{
  m_eqDateTimeFormat = dateTime;
  pSEQPrefs->setPrefString("EQDateTimeFormat", "Terminal", m_eqDateTimeFormat);
}

void Terminal::setDisplayType(bool enable) 
{
  m_displayType = enable; 
  pSEQPrefs->setPrefBool("DisplayType", "Terminal", m_displayType);
}

void Terminal::setDisplayDateTime(bool enable) 
{
  m_displayDateTime = enable; 
  pSEQPrefs->setPrefBool("DisplayDateTime", "Terminal", m_displayDateTime);
}

void Terminal::setDisplayEQDateTime(bool enable) 
{
  m_displayEQDateTime = enable; 
  pSEQPrefs->setPrefBool("DisplayEQDateTime", "Terminal", m_displayEQDateTime);
}

void Terminal::setUseColor(bool enable)
{ 
  m_useColor = enable; 
  pSEQPrefs->setPrefBool("UseColor", "Terminal", m_useColor);
}

void Terminal::newMessage(const MessageEntry& message)
{
  MessageType type = message.type();

  // if the message type isn't enabled, nothing to do
  if ((((m_enabledTypes & ( uint64_t(1) << type)) == 0) &&
       ((m_enabledShowUserFilters & message.filterFlags()) == 0)) ||
      ((m_enabledHideUserFilters & message.filterFlags()) != 0))
    return;

  const char* setColor = 0;

  // if using color, and the type is within range, try to use color
  if ((m_useColor) && 
      (size_t(type) < (sizeof(typeStyles)/sizeof(const char*))))
  {
    // if a color is specified for the type, use it
    if (typeStyles[type])
    {
      setColor = typeStyles[type];
      fputs(setColor, stdout);
    }
  }
 
  QString text;

  // if displaying the type, add it
  if (m_displayType)
    text = MessageEntry::messageTypeString(type) + ": ";
    
  // if displaying the message date/time append it
  if (m_displayDateTime)
    text += message.dateTime().toString(m_dateTimeFormat) + " - ";

  // if displaying the messages eq date/time, append it
  if (m_displayEQDateTime && (message.eqDateTime().isValid()))
    text += message.eqDateTime().toString(m_eqDateTimeFormat) + " - ";

  // append the actual message text
  text += message.text();

  if (m_useColor)
    text.replace(m_itemPattern, 
		 QString("\e[1;4;34m\\2 (#\\1)\e[0;24m") + setColor);
  else
    text.replace(m_itemPattern, "\\2 (#\\1)");

  // now append the message text to the buffer
  fputs((const char*)text.latin1(), stdout);

  if (setColor)
    fputs(defaultStyle, stdout);
  
  fputc('\n', stdout);
}

#include "terminal.moc"
