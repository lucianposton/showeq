/*
 * group.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Fee (fee@users.sourceforge.net). All Rights Reserved.
 *
 * Portions Copyright 2001-2007 by the respective ShowEQ Developers
 *
 * Contributed to ShowEQ by fee (fee@users.sourceforge.net)
 * for use under the terms of the GNU General Public License,
 * incorporated herein by reference.
 *
 */

#include "guild.h"
#include "packet.h"
#include "diagnosticmessages.h"
#include "netstream.h"

#include <qfile.h>
#include <qdatastream.h>
#include <qtextstream.h>

GuildMgr::GuildMgr(QString fn, QObject* parent, const char* name)
  : QObject(parent, name)
{
  guildsFileName = fn;

  readGuildList();
}

GuildMgr::~GuildMgr()
{
}

QString GuildMgr::guildIdToName(uint16_t guildID)
{
  if (guildID >= m_guildMap.size())
    return "";
  return m_guildMap[guildID];
}

void GuildMgr::worldGuildList(const uint8_t* data, size_t len)
{
  writeGuildList(data, len);
  readGuildList();
}

void GuildMgr::writeGuildList(const uint8_t* data, size_t len)
{
  QFile guildsfile(guildsFileName);

  if (guildsfile.exists()) {
     if (!guildsfile.remove()) {
       seqWarn("GuildMgr: Could not remove old %s, unable to replace with server data!",
                guildsFileName.latin1());
        return;
     }
  }

  if(!guildsfile.open(IO_WriteOnly))
    seqWarn("GuildMgr: Could not open %s for writing, unable to replace with server data!",
             guildsFileName.latin1());

  QDataStream guildDataStream(&guildsfile);

  NetStream netStream(data,len);
  QString guildName;
  uint32_t size = 0; // to keep track of how much we're reading from the packet
  uint32_t guildId = 0;

  /*
   0x48 in the packet starts the serialized list.  See guildListStruct
   and worldGuildListStruct in everquest.h
  */

  // skip to the first guild in the list
  netStream.skipBytes(0x44);
  size += 0x44;

  while(!netStream.end())
  {
     guildId = netStream.readUInt32NC();
     guildName = netStream.readText();
     size += 4; // four bytes for the guild ID

     if(guildName.length())
     {
        m_guildList[guildId] = guildName;

        // add guild name length, plus one for the null character
        size += guildName.length() + 1;
     }

     if(size + 1 == len)
        break; // the end
  }

  std::map<uint32_t, QString>::iterator it;

  for(it = m_guildList.begin(); it != m_guildList.end(); it++)
  {
     char szGuildName[64] = {0};

     strcpy(szGuildName, it->second.latin1());
     //seqDebug("GuildMgr::writeGuildList - add guild '%s' (%d)", szGuildName, it->first);
     guildDataStream.writeRawBytes(szGuildName, sizeof(szGuildName));
  }

  guildsfile.close();
  seqInfo("GuildMgr: New guildsfile written");
}

void GuildMgr::readGuildList()
{
  QFile guildsfile(guildsFileName);

  m_guildMap.clear();
  if (guildsfile.open(IO_ReadOnly))
  {
     while (!guildsfile.atEnd())
     {
        char szGuildName[64] = {0};

        guildsfile.readBlock(szGuildName, sizeof(szGuildName));
        //seqDebug("GuildMgr::readGuildList - read guild '%s'", szGuildName);
        m_guildMap.push_back(QString::fromUtf8(szGuildName));
     }

    guildsfile.close();
    seqInfo("GuildMgr: Guildsfile loaded");
  }
  else
    seqWarn("GuildMgr: Could not load guildsfile, %s", (const char*)guildsFileName);
}

void GuildMgr::guildList2text(QString fn)
{
  QFile guildsfile(fn);
  QTextStream guildtext(&guildsfile);

    if (guildsfile.exists()) {
         if (!guildsfile.remove()) {
             seqWarn("GuildMgr: Could not remove old %s, unable to process request!",
                   fn.latin1());
           return;
        }
   }

   if (!guildsfile.open(IO_WriteOnly)) {
     seqWarn("GuildMgr: Could not open %s for writing, unable to process request!",
              fn.latin1());
      return;
   }

   for (unsigned int i =0 ; i < m_guildMap.size(); i++) 
   {
       if (m_guildMap[i])
          guildtext << i << "\t" << m_guildMap[i] << endl;
   }

   guildsfile.close();

   return;
}


void GuildMgr::listGuildInfo()
{
   for (unsigned int i = 0; i < m_guildMap.size(); i++) 
   {
     if (m_guildMap[i])
       seqInfo("%d\t%s", i, (const char*)m_guildMap[i]);
   }
}

#ifndef QMAKEBUILD
#include "guild.moc"
#endif

