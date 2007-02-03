/*
 * message.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002-2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdint.h>

#include <qstring.h>
#include <qdatetime.h>
#include <q3valuevector.h>

//----------------------------------------------------------------------
// constants
const uint32_t ME_InvalidColor = 0x000000FF;

//----------------------------------------------------------------------
// enumerated types
enum MessageType
{
  MT_Guild = 0, 
  MT_Group = 2,
  MT_Shout = 3,
  MT_Auction = 4,
  MT_OOC = 5,
  MT_Tell = 7,
  MT_Say = 8,
  MT_GMSay = 11,
  MT_GMTell = 14,
  MT_Raid = 15,
  MT_Debug,
  MT_Info,
  MT_Warning,
  MT_General,
  MT_Motd,
  MT_System,
  MT_Money,
  MT_Random,
  MT_Emote,
  MT_Time,
  MT_Spell,
  MT_Zone,
  MT_Inspect,
  MT_Player,
  MT_Consider,
  MT_Alert,
  MT_Danger,
  MT_Caution,
  MT_Hunt,
  MT_Locate,
  MT_Max = MT_Locate,
};

//----------------------------------------------------------------------
// MessageEntry
class MessageEntry
{
 public:
  MessageEntry(MessageType type, 
	       const QDateTime& dateTime,
	       const QDateTime& eqDateTime,
	       const QString& text,
	       uint32_t color = ME_InvalidColor,
	       uint32_t filterFlags = 0);
  MessageEntry();
  ~MessageEntry();

  MessageType type() const { return m_type; }
  const QDateTime& dateTime() const { return m_dateTime; }
  const QDateTime& eqDateTime() const { return m_eqDateTime; }
  const QString& text() const { return m_text; }
  const uint32_t color() const { return m_color; }

  uint32_t filterFlags() const { return m_filterFlags; }
  void setFilterFlags(uint32_t filterFlags) { m_filterFlags = filterFlags; }

  static const QString& messageTypeString(MessageType type);

 protected:
  MessageType m_type;
  QDateTime m_dateTime;
  QDateTime m_eqDateTime;
  QString m_text;
  uint32_t m_color;
  uint32_t m_filterFlags;

  static QString s_messageTypeStrings[MT_Max+1];
};

inline MessageEntry::MessageEntry(MessageType type, 
				  const QDateTime& dateTime,
				  const QDateTime& eqDateTime,
				  const QString& text,
				  uint32_t color,
				  uint32_t filters)
  : m_type(type),
    m_dateTime(dateTime),
    m_eqDateTime(eqDateTime),
    m_text(text),
    m_color(color),
    m_filterFlags(filters) 
{
}

inline MessageEntry::MessageEntry()
  : m_type(MT_Debug),
    m_color(0x000000FF),
    m_filterFlags(0)
{
}

inline MessageEntry::~MessageEntry()
{
}

inline const QString& MessageEntry::messageTypeString(MessageType type)
{
  static QString dummy;

  if (type <= MT_Max)
    return s_messageTypeStrings[type];

  return dummy;
}

#endif // _MESSAGE_H_
