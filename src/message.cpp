/*
 * message.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#include "message.h"

QString MessageEntry::s_messageTypeStrings[MT_Max+1] = 
{
  "Guild",
  "",
  "Group",
  "Shout",
  "Auction",
  "OOC",
  "",
  "Tell",
  "Say",
  "",
  "",
  "GMSay",
  "",
  "",
  "GMTell",
  "Raid",
  "Debug",
  "Info",
  "Warning",
  "General",
  "MOTD",
  "System",
  "Money",
  "Random",
  "Emote",
  "Time",
  "Spell",
  "Zone",
  "Inspect",
  "Player",
  "Consider",
  "Alert",
  "Danger",
  "Caution",
  "Hunt",
  "Locate",
};

