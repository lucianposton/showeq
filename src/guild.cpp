/*
 * group.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Fee (fee@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by fee (fee@users.sourceforge.net)
 * for use under the terms of the GNU General Public License,
 * incorporated herein by reference.
 *
 */

#include "guild.h"
#include "packet.h"
#include "diagnosticmessages.h"

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
  const worldGuildListStruct* gls = (const worldGuildListStruct*)data;

  writeGuildList(gls, len);
  readGuildList();
}

void GuildMgr::writeGuildList(const worldGuildListStruct* gls, size_t len)
{
  QFile guildsfile(guildsFileName);

  if (guildsfile.exists()) {
     if (!guildsfile.remove()) {
       seqWarn("GuildMgr: Could not remove old %s, unable to replace with server data!"
,
                guildsFileName.latin1());
        return;
     }
  }

  if(!guildsfile.open(IO_WriteOnly))
    seqWarn("GuildMgr: Could not open %s for writing, unable to replace with server data!",
             guildsFileName.latin1());

  QDataStream guildDataStream(&guildsfile);

  guildDataStream.writeRawBytes((char *)gls->guilds, sizeof(gls->guilds));

  guildsfile.close();
  seqInfo("GuildMgr: New guildsfile written");
}

void GuildMgr::readGuildList()
{
  QFile guildsfile(guildsFileName);

  m_guildMap.clear();
  if (guildsfile.open(IO_ReadOnly))
  {
    worldGuildListStruct tmp;
     if (guildsfile.size() != sizeof(tmp.guilds))
     {
	seqWarn("GuildMgr: Guildsfile not loaded, expected size %d got %ld",
                sizeof(worldGuildListStruct), guildsfile.size()); 
	return;
     }

     struct guildListStruct gl;
     
     while (!guildsfile.atEnd())
     {
         guildsfile.readBlock(reinterpret_cast<char*>(&gl), sizeof(gl));
        // Commented out until verified that this needs to actually be 
		// removed. -- Ratt 
		// if (strlen(gl.guildName) > 0)
            m_guildMap.push_back(QString::fromUtf8(gl.guildName));
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

#include "guild.moc"
