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

#include <qfile.h>
#include <qdatastream.h>
#include <qtextstream.h>

#include "guild.h"
#include "packet.h"

GuildMgr::GuildMgr(QString fn, EQPacket *packet, QObject* parent, const char* name)
                   : QObject(parent, name)
{
  guildsFileName = fn;

   connect(packet, SIGNAL(worldGuildList(const char*, uint32_t)),
           this, SLOT(worldGuildList(const char*, uint32_t)));
   connect(parent, SIGNAL(guildList2text(QString)),
           this, SLOT(guildList2text(QString)));

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

void GuildMgr::worldGuildList(const char* guildData, uint32_t len)
{
  writeGuildList(guildData, len);
  readGuildList();
}

void GuildMgr::writeGuildList(const char* data, uint32_t len)
{
  QFile guildsfile(guildsFileName);

  if (guildsfile.exists()) {
     if (!guildsfile.remove()) {
        fprintf(stderr, "WARNING: could not remove old %s, unable to replace with server data!\n"
,
                guildsFileName.latin1());
        return;
     }
  }

  if(!guildsfile.open(IO_WriteOnly))
     fprintf(stderr, "WARNING: could not open %s for writing, unable to replace with server data!\n",
             guildsFileName.latin1());

  QDataStream guildDataStream(&guildsfile);

  worldGuildListStruct *gls = (worldGuildListStruct *)data;

  guildDataStream.writeRawBytes((char *)gls->guilds, sizeof(gls->guilds));

  guildsfile.close();
  printf("GuildMgr: new guildsfile written\n");
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
	fprintf(stderr, "WARNING: guildsfile not loaded, expected size %d got %ld\n",
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
    printf("GuildMgr: guildsfile loaded\n");
  }
  else
    printf("GuildMgr: WARNING - could not load guildsfile, %s\n", (const char*)guildsFileName);
}

void GuildMgr::guildList2text(QString fn)
{
  QFile guildsfile(fn);
  QTextStream guildtext(&guildsfile);

    if (guildsfile.exists()) {
         if (!guildsfile.remove()) {
             fprintf(stderr, "WARNING: could not remove old %s, unable to process request!\n",
                   fn.latin1());
           return;
        }
   }

   if (!guildsfile.open(IO_WriteOnly)) {
      fprintf(stderr, "WARNING: could not open %s for writing, unable to process request!\n",
              fn.latin1());
      return;
   }

   for (unsigned int i =0 ; i < m_guildMap.size(); i++) 
   {
       if (m_guildMap[i])
          guildtext << i << "\t" << m_guildMap[i] << "\n";
   }

   guildsfile.close();

   return;
}


void GuildMgr::listGuildInfo()
{
   for (unsigned int i = 0; i < m_guildMap.size(); i++) 
   {
       if (m_guildMap[i])
           printf("%d	%s\n", i, (const char*)m_guildMap[i]);
   }
}
