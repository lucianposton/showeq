/*
 * logger.cpp
 *
 * packet/data logging class
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <qstring.h>
#include <qlist.h>
#include "everquest.h"
#include "spawn.h"
#include "logger.h"
#include "util.h"

SEQLogger::SEQLogger(FILE *fp, QObject* parent, const char* name)
  : QObject(parent, name)
{
    m_FP = fp;
    m_errOpen = 0;
}

SEQLogger::SEQLogger(const QString& fname, QObject* parent, const char* name)
  : QObject(parent, name)
{
    m_FP = NULL;
    m_filename = fname;
    m_errOpen = 0;
}

int
SEQLogger::logOpen()
{
    if (m_FP != NULL)
        return(0);

    m_FP = fopen((const char*)m_filename,"a");

    if (m_FP == NULL)
    { 
        if (m_errOpen == 0)
        {
            fprintf(stderr,"Error opening %s: %s (will keep trying)\n",
		    (const char*)m_filename, strerror(errno));
            m_errOpen = 1;
        }

        return(-1);
    }
 
    m_errOpen = 0;

    return(0);
}

int
SEQLogger::outputf(const char *fmt, ...)
{
    va_list args;
    int count;

    if (m_FP == NULL)
        return(0);

    va_start(args, fmt);
    count = vfprintf(m_FP, fmt, args);
    va_end(args);
    return(count);
}

int
SEQLogger::output(const void* data, int length)
{
    int i;
    int count = 0;
    unsigned char *ptr = (unsigned char *) data;

    for(i = 0; i < length; i++,ptr++)
        count += outputf("%.2X", *ptr);

    return(count);
}


void
PktLogger::logProcessMaskString(const char *maskstr, unsigned *m1, unsigned *m2, unsigned *m3)
{
   int masklen;
   unsigned int mask1 = 0, mask2 = 0, mask3 = 0;
   char tempmask[10];

   /* Need to break up mask string into 3 seperate ints  */
   /* perhaps later down the road we can implement named */
   /* masks seperated by |. But for starters we'll       */
   /* just do a single hex constant                      */

   if (*maskstr == '#')
       maskstr++;

   if ((maskstr[0] == '0') && (maskstr[1] == 'x'))
       maskstr += 2;

   masklen = strlen(maskstr);

   /* 333333332222222211111111 = 24 bytes max */

   if (masklen <= 24) /* > 24 is too big a number so we bail */
   {
       if (masklen > 16) /* pull off mask3 from front of string */
       {
           strncpy(tempmask,maskstr,masklen-16);
           tempmask[masklen-16] = 0;
           mask3 = strtoul(tempmask,NULL,16);
           maskstr += (masklen - 16);
           masklen -= (masklen -16);
       }
       if (masklen > 8) /* pull off mask2 from front of string */
       {
           strncpy(tempmask,maskstr,masklen-8);
           tempmask[masklen-8] = 0;
           mask2 = strtoul(tempmask,NULL,16);
           maskstr += (masklen - 8);
           masklen -= (masklen -8);
       }
       if (masklen > 0) /* pull off mask 1 from front of string */
       {
           strncpy(tempmask,maskstr,masklen);
           tempmask[masklen] = 0;
           mask1 = strtoul(tempmask,NULL,16);
           maskstr += masklen;
           masklen -= masklen;
       }

       *m1 = mask1;
       *m2 = mask2;
       *m3 = mask3;

       fprintf(stderr,"Opcode Logging Mask: %X %X %X\n",mask3,mask2,mask1);
   }
}


PktLogger::PktLogger(FILE *fp, unsigned m1, unsigned m2, unsigned m3)
  : SEQLogger(fp, NULL, "PktLogger")
{
    mask1 = m1;
    mask2 = m2;
    mask3 = m3;
}

PktLogger::PktLogger(FILE *fp, const QString& maskstr)
  : SEQLogger(fp, NULL, "PktLogger")
{
    logProcessMaskString(maskstr,&mask1,&mask2,&mask3);
}

PktLogger::PktLogger(const QString& fname, 
		     unsigned m1, unsigned m2, unsigned m3)
  : SEQLogger(fname, NULL, "PktLogger")
{
    mask1 = m1;
    mask2 = m2;
    mask3 = m3;
}

PktLogger::PktLogger(const QString& fname, const QString& maskstr)
  : SEQLogger(fname, NULL, "PktLogger")
{
    logProcessMaskString(maskstr,&mask1,&mask2,&mask3);
}

void
PktLogger::logItemHeader(const itemStruct *item)
{
    outputf("[%.35s] ", item->name);
    outputf("[%.60s] ", item->lore);
    outputf("[%.6s] ", item->idfile);
    output(item->unknown0150, 24);
    outputf(" %d %d %d ", item->weight, item->nosave, item->nodrop);
    outputf("%d ", item->size);
    output(item->unknown0178, 2);
    outputf(" %d %d ", item->itemNr, item->iconNr);
    outputf("%d %d ", item->equipSlot, item->equipableSlots);
    outputf("%d ", item->cost);
    output(item->unknown0196, 32);
    outputf(" ");
    return;
}

void
PktLogger::logBookItem(const itemBookStruct *book)
{
    logItemHeader(book);
    output(book->unknown0228, 6);
    outputf(" [%15s] ", book->file);
    output(book->unknown0246, 15);
    return;
}   
    
void
PktLogger::logItemCommons(const itemItemStruct *item)
{
    outputf(" %d %d %d ", 
        item->STR, item->STA, item->CHA);
    outputf("%d %d %d ", 
        item->DEX, item->INT, item->AGI);
    outputf("%d %d %d ", 
        item->WIS, item->MR, item->FR);
    outputf("%d %d %d ", 
        item->CR, item->DR, item->PR);
    outputf("%d %d %d ", 
        item->HP, item->MANA, item->AC);
    output(item->unknown0246, 2);
    outputf(" %d %d %d ", 
        item->light, item->delay, item->damage);
    output(&item->unknown0251, 1);
    outputf(" %d %d ",
        item->range,item->skill);
    outputf("%d %d %d ", 
        item->magic, item->level0, item->material);
    output(item->unknown0258, 3);
    outputf(" %d ", item->color);
    output(item->unknown0264, 2);
    outputf(" %d %d ", 
        item->spellId0, item->classes);
    output(item->unknown0270, 2);
    outputf(" ");
    return;
}

void
PktLogger::logContainerItem(const itemContainerStruct *container)
{
    logItemHeader(container);
    output(&container->unknown0228,41);
    outputf(" %d ", container->numSlots);
    output(&container->unknown0271,1);
    outputf(" %d ", container->sizeCapacity);
    outputf("%d ", container->weightReduction);
    output(&container->unknown0273, 3);
}   
    
void
PktLogger::logNormalItem(const itemItemStruct *item)
{
    logItemHeader(item);
    logItemCommons(item);
    outputf(" %d ", item->races);
    output(item->unknown0274, 2);
    outputf(" %d %d %d %d ", 
	    item->stackable,
	    item->level, item->charges, item->effectType);
    outputf(" %d ", item->spellId);
    output(item->unknown0282, 10);
    outputf(" %d ", item->castTime);
    output(item->unknown0296, 41);
    outputf(" ");
}

void
PktLogger::logZoneServerInfo(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingZoneServerInfo())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        op->code, op->version);

    output(data, len);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logCPlayerItems(const cPlayerItemsStruct *citems, uint32_t len, uint8_t dir)
{
    if (!isLoggingCPlayerItems())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        citems->opCode, citems->version);

    output(citems->compressedData, len);

    outputf("\n");
    flush();

    return;
}

void 
PktLogger::logItemInShop(const itemInShopStruct *sitem, uint32_t len, uint8_t dir)
{
    if (!isLoggingItemInShop())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        sitem->opCode, sitem->version);

    outputf(" %d ", sitem->playerid);
    outputf(" %d ", sitem->itemType);

    switch(sitem->itemType)
    {
    case 0:
      logNormalItem(&sitem->item);
      break;
    case 1:
      logContainerItem(&sitem->container);
      break;
    case 2:
      logBookItem(&sitem->book);
    }

    outputf(" ");
    output(sitem->unknown0297, 6);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logMoneyOnCorpse(const moneyOnCorpseStruct *money, uint32_t len, uint8_t dir)
{
    if (!isLoggingMoneyOnCorpse())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        money->opCode, money->version);

    output(money->unknown0002, 4);
    outputf(" %u %u %u %u\n", money->platinum, money->gold, 
        money->silver, money->copper);

    flush();
    return;
}

void
PktLogger::logItemOnCorpse(const itemOnCorpseStruct *item, uint32_t len, uint8_t dir)
{
    if (!isLoggingItemOnCorpse())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        item->opCode, item->version);

    logNormalItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logTradeItemIn(const tradeItemInStruct *item, uint32_t len, uint8_t dir)
{
    if (!isLoggingTradeItemIn())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    logNormalItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logTradeItemOut(const tradeItemOutStruct *item, uint32_t len, uint8_t dir)
{
    if (isLoggingTradeItemOut())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    output(item->unknown0002,6);
    outputf(" %d ", item->itemType);

    switch(item->itemType)
    {
    case 0:
      logNormalItem(&item->item);
      break;
    case 1:
      logContainerItem(&item->container);
      break;
    case 2:
      logBookItem(&item->book);
    }

    outputf(" ");
    output(item->unknown0253, 5);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logPlayerItem(const playerItemStruct *item, uint32_t len, uint8_t dir)
{
    if (!isLoggingPlayerItem())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    logNormalItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logSummonedItem(const summonedItemStruct *item, uint32_t len, uint8_t dir)
{
    if (!isLoggingSummonedItem())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    logNormalItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logECharProfile(const charProfileStruct *data, uint32_t len, uint8_t dir)
{
    if (!isLoggingECharProfile())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        data->opCode, 0xFF /* data->version */);

    output(data,len);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logCharProfile(const charProfileStruct *profile, uint32_t len, uint8_t dir)
{
    if (!isLoggingCharProfile())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    const struct spellBuff *buff;
    int i;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ",timestamp, len, dir, 
        profile->opCode, profile->version );

//    output(profile->unknown0002,4);
 //   outputf(" [%.30s] [%.20s] %u %u ", profile->name, profile->lastName,
//        profile->face, profile->race);
//    output(&profile->unknown0059,1); 
 //   outputf(" %u %u %u ", profile->class_, profile->gender,
  //      profile->level);
//    output(profile->unknown0063,3);
//    outputf(" %u ",profile->exp);
//    output(profile->unknown0070,2);
//    outputf(" %u ",profile->MANA);
//    output(profile->unknown0074,48);
//    outputf(" %u ",profile->curHp);
//    output(&profile->unknown0124,1);
//    outputf(" %u %u %u ", profile->STR, profile->STA, profile->CHA);
//    outputf("%u %u %u %u ", profile->DEX, profile->INT, profile->AGI,
//        profile->WIS);

    for(i = 0; i < 25; i++)
        outputf("%u ", profile->languages[i]);

    for(i = 0; i < 25; i++)
    {
        buff = &profile->buffs[i];

        output(&buff->unknown0000,1);

        outputf(" %d %d %d ", buff->level, buff->spell, 
            buff->duration);

        output(&buff->unknown0002,2);
        outputf(" ");
    }

    //output(profile->unknown0800, 1080);
    outputf(" ");
    
    for(i = 0; i < 256; i++)
        outputf("%d ", profile->sSpellBook[i]);

    for(i = 0; i < 8; i++)
        outputf("%d ", profile->sMemSpells[i]);
    
    //output(profile->unknown2408, 54);
    outputf(" %u %u %u %u %u %u %u %u ", profile->platinum,
        profile->gold, profile->silver, profile->copper, profile->platinumBank,
        profile->goldBank, profile->silverBank, profile->copperBank);

    //output(profile->unknown2494, 16); 
    outputf(" ");

    for(i = 0; i < 74; i++)
        outputf("%u ", profile->skills[i]);

    //output(profile->unknown2584, 118); 

    //outputf(" [%.144s] [%.20s] [%.20s] [%.20s] [%.20s] [%20s] ",
    //    profile->GUILD, profile->bindpoint, profile->miscnames[0],
    //    profile->miscnames[1], profile->miscnames[2], profile->miscnames[3]);

    //output(profile->unknown2946, 1212); 
    outputf(" %u ",profile->deity);
    //output(profile->unknown4160, 62); 
    outputf(" ");

    for(i = 0; i < 5; i++)
        outputf("[%.48s] ", &profile->GroupMembers[i][0]);

    //output(profile->unknown4462, 72); 
    outputf(" %u ", profile->altexp);
    //output(profile->unknown4538, 358); 
    outputf(" %u ", profile->aapoints);
    //output(profile->unknown4897, 3209);
    outputf("\n");
    flush();
    return;
}

void
PktLogger::logNewCorpse(const newCorpseStruct *s, uint32_t len, uint8_t dir)
{
    if (!isLoggingNewCorpse())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u %u ", timestamp, len, dir,              s->opCode, s->version, s->spawnId, s->killerId);

    output(s->unknown0006, 4);
    outputf(" %u %d ", s->spellId, s->type);
    output(&s->unknown0013, 1);
    outputf(" %u ", s->damage);
    output(s->unknown0016, 2);
    outputf(" ");
    output(s->unknown0018, 4);
    outputf("\n");
    flush();
    return;
}

void
PktLogger::logDeleteSpawn(const deleteSpawnStruct *s, uint32_t len, uint8_t dir)
{
    if (!isLoggingDeleteSpawn())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir, 
        s->opCode, s->version, s->spawnId);
    outputf("\n");
    flush();
}

void
PktLogger::logChannelMessage(const channelMessageStruct *msg, uint32_t len, uint8_t dir)
{
    if (!isLoggingChannelMessage())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    zoneTimestamp = timestamp; /* save for future Decrypt log */

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        msg->opCode, msg->version);

    outputf(" [%.64s] ", msg->target);
    outputf(" [%.64s] ", msg->sender);
    outputf(" %u ", msg->language);
    output(&msg->unknown0131,1);
    outputf(" %u ", msg->chanNum);
    output(&msg->unknown0133,5);
    outputf(" [%s]\n", msg->message);
    flush();
    return;
}

void
PktLogger::logENewSpawn(const newSpawnStruct *spawn, uint32_t len, uint8_t dir)
{
    if (!isLoggingENewSpawn())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        spawn->opCode, 0xFF /* spawn->version */ );

    output(spawn, len);
    outputf("\n");
    flush();
    return;
}

void
PktLogger::logSpawnStruct(const spawnStruct *spawn)
{
    output(spawn->unknown0000, 49);
    outputf(" %d %d %d %d %d %d %u %d %u %d ", spawn->heading,
        spawn->deltaHeading, spawn->y, spawn->x, spawn->z,
        spawn->deltaY, spawn->spacer1, spawn->deltaZ, spawn->spacer2,
        spawn->deltaZ);

    //output(spawn->unknown0061, 1);
    outputf(" %u %d ", spawn->spawnId, spawn->typeflag);
    //output(spawn->unknown0065, 1);

    outputf(" %u %d %d %u %u %u %u %u ", spawn->petOwnerId,
        spawn->maxHp, spawn->curHp, spawn->race, spawn->NPC, spawn->class_,
        spawn->gender, spawn->level);

    //output(spawn->unknown0077,4);
    //outputf(" %u ", spawn->light);
    //output(spawn->unknown0082, 10);
    //outputf(" %u %u %u %u %u %u %u %u %u [%.30s] [%.20s] ", 
     //   spawn->equipment[0], spawn->equipment[1], spawn->equipment[2], 
      //  spawn->equipment[3], spawn->equipment[4], spawn->equipment[5], 
       // spawn->equipment[6], spawn->equipment[7], spawn->equipment[8], 
       // spawn->name, spawn->lastname);

//    output(spawn->unknown0160, 2);
 //   outputf(" %u ", spawn->deity);
  //  output(spawn->unknown0164, 8);
    outputf(" ");
}

void
PktLogger::logNewSpawn(const newSpawnStruct *s, uint32_t len, uint8_t dir)
{
    if (!isLoggingNewSpawn())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir, 
        s->opCode, s->version );

    output(&s->unknown0002, 4);
    outputf(" ");
    logSpawnStruct(&s->spawn);
    outputf("\n");
    flush();

    return;
}

/*
    void PktLogger::logZoneSpawns(zoneSpawnsStruct* zspawns, int len)
 
    In this case we generate multiple lines per packet. One for 
    each spawn decrypted. We repeat the opcode header for each
    header. 
*/

void
PktLogger::logZoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len, uint8_t dir)
{
  logZoneSpawnsTimestamp();

    if (!isLoggingZoneSpawns())
      return;

    int spawndatasize = (len - 2) / sizeof(spawnStruct);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    for (int i = 0; i < spawndatasize; i++)
    {
        const spawnZoneStruct *zspawn = &zspawns->spawn[i];
        const spawnStruct *spawn = &zspawns->spawn[i].spawn;

        outputf("R %u %04d %d %.2X%.2X ", zoneTimestamp, 
            sizeof(spawnStruct)+2, dir, zspawns->opCode,  zspawns->version );

        output(zspawn->unknown0000, 4);
        outputf(" ");
        logSpawnStruct(spawn);
        outputf("\n");
        flush();
    }

    return;
}

void
PktLogger::logEZoneSpawns(const zoneSpawnsStruct *zone, uint32_t len, uint8_t dir)
{
    if (!isLoggingEZoneSpawns())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    zoneTimestamp = timestamp; /* save for future Decrypt log */

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        zone->opCode, 0xFF /* zone->version */ );

    output(zone, len);
    outputf("\n");
    flush();
    return;
}

void
PktLogger::logTimeOfDay(const timeOfDayStruct *tday, uint32_t len, uint8_t dir)
{
    if (!isLoggingTimeOfDay())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %d %d %d %d %u\n", timestamp, len,
        dir, tday->opCode, tday->version, tday->hour, tday->minute, 
        tday->day, tday->month, tday->year);

    flush();

    return;
}

/* 
   void PktLogger::logZoneSpawnsTimestamp()

   Save the timestamp when last encrypted ZoneSpawns came in so we
   can include it with the decrypted ZoneSpawns log.
*/

void
PktLogger::logZoneSpawnsTimestamp()
{
    zoneTimestamp = (unsigned int) time(NULL);
    return;
}

void
PktLogger::logBookText(const bookTextStruct *book, uint32_t len, uint8_t dir)
{
    if (!isLoggingBookText())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    char *text;
    int tlen;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    text = strdup(book->text);
 
    if (text == NULL)
        text = (char *) &book->text;
    else
    {
        tlen = strlen(text);

        if (tlen > 0)
            if (text[tlen-1] == '\n')
                text[tlen-1] = 0;
    }

    outputf("R %u %04d %d %.2X%.2X [%s]\n", timestamp, len, dir,
        book->opCode, book->version,text);

    if (text != book->text)
        free(text);

    flush();
}

void
PktLogger::logRandom(const randomStruct *ran, uint32_t len, uint8_t dir)
{
    if (!isLoggingRandom())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    
    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u %u\n", timestamp, len, dir,
        ran->opCode, ran->version, ran->bottom, ran->top);

    flush();
}

void
PktLogger::logEmoteText(const emoteTextStruct *emote, uint32_t len, uint8_t dir)
{
    if (!isLoggingEmoteText())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    
    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
        emote->opCode, emote->version);

    output(emote->unknown0002,2); 
    outputf(" [%s]\n", emote->text);
    flush();
}

void
PktLogger::logCorpseLoc(const corpseLocStruct *corpse, uint32_t len, uint8_t dir)
{
    if (!isLoggingCorpseLoc())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
       corpse->opCode, corpse->version, corpse->spawnId);

    output(&corpse->unknown0004,2);
    outputf(" %f %f %f\n", corpse->x, corpse->y, corpse->z);
    flush();
    return;
}

void
PktLogger::logPlayerBook(const playerBookStruct *book, uint32_t len, uint8_t dir)
{
    if (!isLoggingPlayerBook())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       book->opCode, book->version);

    logBookItem(&book->book);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logPlayerContainer(const playerContainerStruct *container, uint32_t len, uint8_t dir)
{
    if (!isLoggingPlayerContainer())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       container->opCode, container->version);

    logContainerItem(&container->container);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logInspectData(const inspectDataStruct *data, uint32_t len, uint8_t dir)
{
    if (!isLoggingInspectData())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    int i;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       data->opCode, data->version);

    output(data->unknown0002,40);
    outputf(" ");

    for(i = 0; i < 21; i++)
        outputf("[%.32s] ", (char *)&data->itemNames[i]);

    for(i = 0; i < 21; i++)
        outputf("%d ", data->icons[i]);

    output(data->unknown0756,2);
    outputf(" [%.200s] ", data->mytext);
    output(data->unknown0958,88);
    outputf("\n");
    flush();
}

void 
PktLogger::logHPUpdate(const hpUpdateStruct *hp, uint32_t len, uint8_t dir)
{
    if (!isLoggingHPUpdate())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
       hp->opCode, hp->version, hp->spawnId);

    output(&hp->unknown0004,2);
    outputf(" %d ", hp->curHp);
    output(&hp->unknown0008,2);
    outputf(" %d ", hp->maxHp);
    output(&hp->unknown0012,2);
    outputf("\n");
    flush();
}

void 
PktLogger::logSPMesg(const spMesgStruct *msg, uint32_t len, uint8_t dir)
{
    if (!isLoggingSPMesg())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %d [%s]\n", timestamp, len, dir,
       msg->opCode, msg->version, msg->msgType, msg->message);

    flush();
}

void 
PktLogger::logMemSpell(const memSpellStruct *spell, uint32_t len, uint8_t dir)
{
    if (!isLoggingMemSpell())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
       spell->opCode, spell->version, spell->spawnId);

    output(&spell->unknown0004, 2);
    outputf(" %d %d %d %d\n", 
       spell->spellId, spell->param1, spell->param2, spell->param3);
    flush();
}

void 
PktLogger::logBeginCast(const beginCastStruct *spell, uint32_t len, uint8_t dir)
{
    if (!isLoggingBeginCast())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u %u %d %d\n", timestamp, len,
       dir, spell->opCode, spell->version, spell->spawnId, spell->spellId, 
       spell->param1, spell->param2);

    flush();
}

void 
PktLogger::logStartCast(const startCastStruct *spell, uint32_t len, uint8_t dir)
{
    if (!isLoggingStartCast())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       spell->opCode, spell->version);

    output(&spell->unknown0002, 2);
    outputf(" %u ", spell->spellId);
    output(&spell->unknown0006, 4);
    outputf(" ");
    outputf(" %u ", spell->targetId);
    outputf("\n");

    flush();
}

void 
PktLogger::logMobUpdate(const mobUpdateStruct *update, uint32_t len, uint8_t dir)
{
    if (!isLoggingMobUpdate())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    const spawnPositionUpdate *pos;
    int i;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %d ", timestamp, len, dir,
       update->opCode, update->version, update->numUpdates);

    for(i = 0; i < update->numUpdates; i++)
    {
        pos = &update->spawnUpdate[i];
        outputf("%u %d %d %d %d %d %d %d %u %d %u %d ",
            pos->spawnId, pos->animation, pos->heading,
            pos->deltaHeading, pos->y, pos->x,
            pos->z, pos->deltaY, pos->spacer1,
            pos->deltaX, pos->spacer2, pos->deltaZ);
    }
    outputf("\n");
    flush();
}

void 
PktLogger::logExpUpdate(const expUpdateStruct *exp, uint32_t len, uint8_t dir)
{
    if (!isLoggingExpUpdate())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        exp->opCode, exp->version, exp->exp);
    output(&exp->unknown0004,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logAltExpUpdate(const altExpUpdateStruct *alt, uint32_t len, uint8_t dir)
{
    if (!isLoggingAltExpUpdate())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u ",timestamp,len,dir,
        alt->opCode, alt->version, alt->altexp);

    output(&alt->unknown0004,2);
    outputf(" %u %u ", alt->aapoints, alt->percent);
    output(&alt->unknown0009,1);
    outputf(" ");
    flush();
    return;
}

void 
PktLogger::logLevelUpUpdate(const levelUpUpdateStruct *level, uint32_t len, uint8_t dir)
{
    if (!isLoggingLevelUpUpdate())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u %u %u\n", timestamp, len, dir,
        level->opCode, level->version, level->level, level->levelOld,
        level->exp);

    flush();
    return;
}

void 
PktLogger::logSkillInc(const skillIncStruct *skill, uint32_t len, uint8_t dir)
{
    if (!isLoggingSkillInc())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        skill->opCode, skill->version, skill->skillId);

    output(skill->unknown0004,2);
    outputf(" %d ", skill->value);
    output(skill->unknown0008,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logDoorOpen(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingDoorOpen())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        op->code, op->version);

    output(data, len);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logIllusion(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingIllusion())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        op->code, op->version);

    output(data, len);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logBadCast(const badCastStruct *spell, uint32_t len, uint8_t dir)
{
    if (!isLoggingBadCast())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u [%s]\n", timestamp, len, dir,
        spell->opCode, spell->version, spell->spawnId, spell->message);

    flush();
    return;
}

void 
PktLogger::logSysMsg(const sysMsgStruct *msg, uint32_t len, uint8_t dir)
{
    if (!isLoggingSysMsg())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    char *text;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    // This one can have embedded newlines, so we need to encode
    // them. I chose ASCII 201 which is a different symbol depending
    // on your current character set. I will always embed it
    // by value rather than symbol.

    text = strdup(msg->message);

    if (text != NULL)
    {
        char *i;
 
        for(i=text;*i;i++)
            if (*i == '\n') *i = 201;
    }

    outputf("R %u %04d %d %.2X%2.X [%s]\n", timestamp, len, dir,
        msg->opCode, msg->version, (text) ? text : msg->message);

    flush();

    if (text != NULL)
        free(text);

    return;
}

void
PktLogger::logZoneEntry(const ServerZoneEntryStruct* zone, uint32_t len, uint8_t dir)
{
    if (!isLoggingZoneChange())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
            zone->opCode,zone->version);
    
    //output(zone->unknown0002, 5);
    //outputf(" [%s] [%s] ", zone->name, zone->zoneShortName);
//    output(zone->unknown0052, 107);
 //   outputf(" %u %u ", zone->class_, zone->race);
  //  output(zone->unknown0161, 2);
   // outputf(" %u ", zone->level);
    //output(zone->unknown0164, 148);
//    outputf(" %u ", zone->deity);
 //   output(&zone->unknown0310, 8);
  //  output(&zone->unknown0318, 8);
    outputf("\n");

    flush();
    return;
}

void
PktLogger::logZoneEntry(const ClientZoneEntryStruct* zone, uint32_t len, uint8_t dir)
{
    if (!isLoggingZoneChange())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
            zone->opCode,zone->version);
    
    output(&zone->unknown0002,4);
    outputf(" [%.30s] ", zone->name);
    output(&zone->unknown0036,2);
    outputf("\n");

    flush();
    return;
}

void
PktLogger::logNewZone(const newZoneStruct *zone, uint32_t len, uint8_t dir)
{
    if (!isLoggingZoneEntry())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X [%s] [%s] ", timestamp, len, dir,
        zone->opCode,zone->version, zone->charName, zone->shortName);

    //output(zone->unknown0047, 5);
    outputf(" [%s] ", zone->longName);
    //output(zone->unknown0232, 170);
    outputf("\n");

    flush();

    return;
}

void 
PktLogger::logZoneChange(const zoneChangeStruct *zone, uint32_t len, uint8_t dir)
{
    if (!isLoggingNewZone())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X [%.32s] ", timestamp, len,
	    dir, zone->opCode, zone->version, zone->charName);

    output(zone->unknown0050, 20);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logPlayerPos(const playerPosStruct *pos, uint32_t len, uint8_t dir)
{
    if (!isLoggingPlayerPos())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
        pos->opCode, pos->version, pos->spawnId);

    output(pos->unknown0004,1);
    outputf(" %d %d %d %d %d %d %u %d %u %d\n", pos->heading, 
        pos->deltaHeading, pos->y, pos->x, pos->z, pos->deltaY,
        pos->spacer1, pos->deltaZ, pos->spacer2, pos->deltaX);

    flush();
    return;
}

void 
PktLogger::logWearChange(const wearChangeStruct *wear, uint32_t len, uint8_t dir)
{
    if (!isLoggingWearChange())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
        wear->opCode,wear->version, wear->spawnId);

    output(wear->unknown0005,1);
    outputf("%d %u ", wear->wearSlotId, wear->newItemId);
    output(wear->unknown0008,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logAction(const action2Struct *action, uint32_t len, uint8_t dir)
{
    if (!isLoggingAction())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u %u %d ", 
	    timestamp, len, dir,
	    action->opCode, action->version, 
	    action->target, action->source, action->type);

    output(&action->unknown0007,1);
    outputf(" %d %d ",action->spell, action->damage);
    output(action->unknown0014,16);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logCastOn(const castOnStruct *spell, uint32_t len, uint8_t dir)
{
    if (!isLoggingCastOn())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
        spell->opCode, spell->version, spell->targetId);

    output(spell->unknown0004,2);
    outputf(" %d ",spell->sourceId);
    output(spell->unknown0008,2);
    outputf(" ");
    output(spell->unknown0010,24);
    outputf(" %u ",spell->spellId);
    output(spell->unknown0036,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logManaDecrement(const manaDecrementStruct *mana, uint32_t len, uint8_t dir)
{
    if (isLoggingManaDecrement())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %d %d\n", timestamp, len, dir,
        mana->opCode, mana->version, mana->newMana, mana->spellID);

    flush();
    return;
}

void 
PktLogger::logStamina(const staminaStruct *stamina, uint32_t len, uint8_t dir)
{
    if (!isLoggingStamina())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %d %d %d\n", timestamp, len, dir,
        stamina->opCode, stamina->version, stamina->food, stamina->water, 
        stamina->fatigue);

    flush();
    return;
}

void
PktLogger::logMakeDrop(const makeDropStruct *item, uint32_t len, uint8_t dir)
{
    if (!isLoggingMakeDrop())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir, 
        item->opCode, item->version);

    output(item->unknown0002,8);
    outputf(" %u ", item->itemNr);
    output(item->unknown0012,2);
    outputf(" %u ", item->dropId);
    output(item->unknown0016,26);
    outputf(" %f %f %f ", item->y, item->x, item->z);
    output(item->unknown0054,4);
    outputf(" [%.16s] ", item->idFile);
    output(item->unknown0074,168);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logRemDrop(const remDropStruct *item, uint32_t len, uint8_t dir)
{
    if (!isLoggingRemDrop())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u", timestamp, len, dir, 
        item->opCode, item->version, item->dropId);

    output(item->unknown0004,2);
    outputf(" %u ", item->spawnId);
    output(item->unknown0008,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logDropCoins(const dropCoinsStruct *coins, uint32_t len, uint8_t dir)
{
    if (!isLoggingDropCoins())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
        coins->opCode, coins->version);

    output(coins->unknown0002,24);
    outputf(" %u ",coins->dropId);
    output(coins->unknown0028,22);
    outputf(" %u ", coins->amount);
    output(coins->unknown0054,4);
    outputf(" %f %f %f ", coins->y, coins->x, coins->z);
    output(coins->unknown0070,12);
    outputf(" [%.15s] ", coins->type);

    output(coins->unknown0097,17);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logRemoveCoins(const removeCoinsStruct *coins, uint32_t len, uint8_t dir)
{
    if (!isLoggingRemoveCoins())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u", timestamp, len, dir,
        coins->opCode, coins->version, coins->dropId);

    output(coins->unknown0004,2);
    outputf(" %u ", coins->spawnId);
    output(coins->unknown0008,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logOpenVendor(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingOpenVendor())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        op->code, op->version);

    output(data, len);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logCloseVendor(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingCloseVendor())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        op->code, op->version);

    output(data, len);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logOpenGM(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingOpenGM())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        op->code, op->version);

    output(data, len); 
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logCloseGM(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingCloseGM())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        op->code, op->version);

    output(data, len);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logSpawnAppearance(const spawnAppearanceStruct *spawn, uint32_t len, uint8_t dir)
{
    if (!isLoggingSpawnAppearance())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u", timestamp, len, dir, 
        spawn->opCode, spawn->version, spawn->spawnId);

    output(&spawn->unknown0004,2);
    outputf(" %d ", spawn->type);
    output(&spawn->unknown0008,2);
    outputf(" %d\n", spawn->paramter);
    flush();
    return;
}

void 
PktLogger::logAttack2(const attack2Struct *attack, uint32_t len, uint8_t dir)
{
    if (!isLoggingAttack2())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u %d %d %d %d %d\n",
        timestamp, len, dir, attack->opCode, attack->version, attack->spawnId,
        attack->param1, attack->param2, attack->param3, attack->param4,
        attack->param5);

    flush();
    return;
}

void 
PktLogger::logConsider(const considerStruct *consider, uint32_t len, uint8_t dir)
{
    if (!isLoggingConsider())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir, 
        consider->opCode, consider->version, consider->playerid);

    outputf(" %u ", consider->targetid);
    outputf(" %d %d %d %d ", consider->faction, consider->level,
        consider->curHp, consider->maxHp);
    output(&consider->unknown0026,4);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logNewGuildInZone(const newGuildInZoneStruct *guild, uint32_t len, uint8_t dir)
{
    if (!isLoggingNewGuildInZone())
      return;
   
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        guild->opCode, guild->version);

    output(guild->unknown0002,8);
    outputf(" [%.56s]\n",guild->guildname);
    flush();
    return;
}

void 
PktLogger::logMoneyUpdate(const moneyUpdateStruct *money, uint32_t len, uint8_t dir)
{
    if (!isLoggingMoneyUpdate())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        money->opCode, money->version);

    output(&money->unknown0002,4);
    outputf(" %u ",money->cointype);
    output(money->unknown0007,3);
    outputf(" %u \n",money->amount);
    flush();
    return;
}

void
PktLogger::logMoneyThing(const moneyThingStruct *thing, uint32_t len, uint8_t dir)
{
    if (!isLoggingMoneyThing())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        thing->opCode, thing->version);

    output(&thing->unknown0002,4);
    outputf(" %u ",thing->cointype);
    output(thing->unknown0007,3);
    outputf(" %u \n",thing->amount);
    flush();
    return;
}

void 
PktLogger::logClientTarget(const clientTargetStruct *target, uint32_t len, uint8_t dir)
{
    if (!isLoggingClientTarget())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        target->opCode, target->version, target->newTarget);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logBindWound(const bindWoundStruct *bind, uint32_t len, uint8_t dir)
{
    if (!isLoggingBindWound())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        bind->opCode, bind->version, bind->playerid);

    output(bind->unknown0004,2);
    outputf("%u \n", bind->hpmaybe);
    flush();
    return;
}

void 
PktLogger::logCDoorSpawns(const cDoorSpawnsStruct *doors, uint32_t len, uint8_t dir)
{
    if (!isLoggingCDoorSpawns())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        doors->opCode, doors->version, doors->count);

    output(doors,len);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logDoorSpawns(const doorSpawnsStruct *doors, uint32_t len, uint8_t dir)
{
    if (!isLoggingDoorSpawns())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);
    int i;
    
    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        doors->opCode, doors->version, doors->count);

    for(i = 0; i < doors->count; i++)
    {
        outputf("[%.8s] ", doors->doors[i].name);
        output(doors->doors[i].unknown0008,8);
        outputf(" %f %f %f ",doors->doors[i].y,
            doors->doors[i].x, doors->doors[i].z);
        output(doors->doors[i].unknown0028,10);
        outputf(" %d %d ", doors->doors[i].doorId,
            doors->doors[i].size);
        output(doors->doors[i].unknown0040,4);
        outputf(" ");
    }

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logGroupInfo(const groupMemberStruct *group, uint32_t len, uint8_t dir)
{
    if (!isLoggingGroupInfo())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X [%.64s] [%.64s] ", timestamp, len,
        dir, group->opCode, group->version, group->yourname, group->membername);

    output(group->unknown0130, 324);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logGroupInvite(const groupInviteStruct *group, uint32_t len, uint8_t dir)
{
    if (!isLoggingGroupInfo())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X [%.64s] [%.64s] ", timestamp, len,
        dir, group->opCode, group->version, group->yourname, group->membername);

    output(group->unknown0130, 65);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logGroupDecline(const groupDeclineStruct *group, uint32_t len, uint8_t dir)
{
    if (!isLoggingGroupInfo())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X [%.64s] [%.64s] %.2X", timestamp, len,
        dir, group->opCode, group->version, group->yourname, group->membername, group->reason);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logGroupAccept(const groupAcceptStruct *group, uint32_t len, uint8_t dir)
{
    if (!isLoggingGroupInfo())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X [%.64s] [%.64s] ", timestamp, len,
        dir, group->opCode, group->version, group->yourname, group->membername);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logGroupDelete(const groupDeleteStruct *group, uint32_t len, uint8_t dir)
{
    if (!isLoggingGroupInfo())
      return;

    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X [%.64s] [%.64s] ", timestamp, len,
        dir, group->opCode, group->version, group->yourname, group->membername);
}

void 
PktLogger::logUnknownOpcode(const uint8_t* data, uint32_t len, uint8_t dir)
{
    if (!isLoggingUnknownOpcode())
      return;

    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("U %u %04d %d %.2X%.2X ", timestamp, len, dir,
        op->code, op->version);

    output(data,len);
    outputf("\n");
    flush();
    return;
}


SpawnLogger::SpawnLogger(const QString& fname)
  : SEQLogger(fname, NULL, "SpawnLogger")
{
    version = 3;
    zoneShortName = "unknown";
    l_time = new EQTime();
    return;
}

SpawnLogger::SpawnLogger(FILE *fp)
  : SEQLogger(fp, NULL, "SpawnLogger")
{
    version = 3;
    zoneShortName = "unknown";
    l_time = new EQTime();
    return;
}

void
SpawnLogger::logTimeSync(const timeOfDayStruct *tday)
{
    l_time->setepoch(time(NULL),tday);
}

void
SpawnLogger::logSpawnInfo(const char *type, const char *name, int id, int level,
                          int x, int y, int z, time_t timeCurrent,
                          const char *killedBy, int kid)
{
    struct timeOfDayStruct eqDate;
    struct tm* current;

    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    eqDate  = l_time->eqdate(timeCurrent);
    current = localtime(&timeCurrent);

    outputf("%s:%s(%d):%d:%d,%d,%d:%02d.%02d.%02d:%d:%s:%02d.%02d.%02d.%02d.%04d:%s(%d)\n",
        type,
        name,
        id,
        level,
        x,
        y,
        z,
        current->tm_hour, current->tm_min, current->tm_sec,
        version,
        (const char*)zoneShortName,
        eqDate.hour,
        eqDate.minute,
        eqDate.month,
        eqDate.day,
        eqDate.year,
        killedBy,
        kid
    );

    flush();

    return;
}

void 
SpawnLogger::logZoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len)
{
  int spawndatasize = (len - 2) / sizeof(spawnStruct);

  for (int i = 0; i < spawndatasize; i++)
    logZoneSpawn(&zspawns->spawn[i].spawn);
}

void
SpawnLogger::logZoneSpawn(const spawnStruct *spawn)
{
    logSpawnInfo("z",spawn->name,spawn->spawnId,spawn->level,
                 spawn->x, spawn->y, spawn->z, time(NULL), "", 0);

    return;
}

void
SpawnLogger::logZoneSpawn(const newSpawnStruct *nspawn)
{
  const spawnStruct* spawn = &nspawn->spawn;
  logSpawnInfo("z",spawn->name,spawn->spawnId,spawn->level,
	       spawn->x, spawn->y, spawn->z, time(NULL), "", 0);
  
  return;
}

void
SpawnLogger::logNewSpawn(const newSpawnStruct* nspawn)
{
  const spawnStruct* spawn = &nspawn->spawn;
  logSpawnInfo("+",spawn->name,spawn->spawnId,spawn->level,
	       spawn->x, spawn->y, spawn->z, time(NULL), "", 0);

  return;
}

void
SpawnLogger::logKilledSpawn(const Item *item, const Item* kitem, uint16_t kid)
{
  if (item == NULL)
    return;

  const Spawn* spawn = (const Spawn*)item;
  const Spawn* killer = (const Spawn*)kitem;

  logSpawnInfo("x",(const char *) spawn->name(),spawn->id(), spawn->level(), 
	       spawn->x(), spawn->y(), spawn->z(), time(NULL),
	       killer ? (const char*)killer->name() : "unknown",
	       kid);

  return;
}

void
SpawnLogger::logDeleteSpawn(const Item *item)
{
  if (item->type() != tSpawn)
    return;

  const Spawn* spawn = (const Spawn*)item;

  logSpawnInfo("-",(const char *)spawn->name(),spawn->id(),spawn->level(),
	       spawn->x(), spawn->y(), spawn->z(), time(NULL),"",0);

  return;
}

void
SpawnLogger::logNewZone(const QString& zonename)
{
    if (m_FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("----------\nNEW ZONE: %s\n----------\n", (const char*)zonename);
    outputf(" :name(spawnID):Level:Xpos:Ypos:Zpos:H.m.s:Ver:Zone:eqHour.eqMinute.eqMonth.eqDay.eqYear:killedBy(spawnID)\n");
    flush();
    zoneShortName = zonename;
}


