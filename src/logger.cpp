#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "everquest.h"
#include "logger.h"

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

       fprintf(stderr,"mask1=%X, mask2=%X, mask3=%X\n",mask1,mask2,mask3);
   }
}

PktLogger::PktLogger(FILE *fp, unsigned m1, unsigned m2, unsigned m3)
{
    FP = fp;
    filename = NULL;
    mask1 = m1;
    mask2 = m2;
    mask3 = m3;
}

PktLogger::PktLogger(FILE *fp, const char *maskstr)
{
    FP = fp;
    filename = NULL;
    logProcessMaskString(maskstr,&mask1,&mask2,&mask3);
}

PktLogger::PktLogger(const char *fname, unsigned m1, unsigned m2, unsigned m3)
{
    filename = strdup(fname);
    FP = NULL;
    mask1 = m1;
    mask2 = m2;
    mask3 = m3;
}

PktLogger::PktLogger(const char *fname, const char *maskstr)
{
    filename = strdup(fname);
    FP = NULL;
    logProcessMaskString(maskstr,&mask1,&mask2,&mask3);
}


/*
    int pktLogOpen()

    Attempt to open the filename specified in the configuration
    file for PktLoggerFilename.  If we are unable to open the
    file we print an error once. Every log attempt will try to
    open the file if isn't open, so there will be a system slowdown
    if logging is enable and the file can't be created. This is
    intentional, but subject to change.
*/

int
PktLogger::logOpen()
{
    if (FP != NULL)
        return(0);

    FP = fopen(filename,"a");

    if ((FP == NULL) && (pktLogErr != 0))
    {
        fprintf(stderr,"Error opening %s: %s (will keep trying)\n",
           filename, strerror(errno));
        pktLogErr = 1;
        return(-1);
    }
 
    pktLogErr = 0;

    return(0);
}

int
PktLogger::outputf(const char *fmt, ...)
{
    va_list args;
    int count;

    va_start(args, fmt);
    count = vfprintf(FP, fmt, args);
    va_end(args);
    return(count);
}

int
PktLogger::output(const void *data, int length)
{
    int i;
    int count = 0;
    unsigned char *ptr = (unsigned char *) data;

    for(i = 0; i < length; i++,ptr++)
        count += outputf("%.2X", *ptr);

    return(count);
}

void
PktLogger::logItemHeader(const itemStruct *item)
{
    outputf("[%.35s] ", item->name);
    outputf("[%.60s] ", item->lore);
    outputf("[%.6s] ", item->idfile);
    outputf("%d ", item->flag);
    output(item->unknown0103, 22);
    outputf(" %d %d %d ", item->weight, item->nosave, item->nodrop);
    outputf("%d ", item->size);
    output(&item->unknown0129, 1);
    outputf("%d %d ", item->itemNr, item->iconNr);
    outputf("%d %d ", item->equipSlot, item->equipableSlots);
    outputf("%d ", item->cost);
    output(item->unknown0144, 28);
    outputf(" ");
    return;
}

void
PktLogger::logBookItem(const itemStruct *item)
{
    logItemHeader(item);
    output(item->book.unknown0172, 3);
    outputf(" [%15s] ", item->book.file);
    output(item->book.unknown0190, 102);
    return;
}   
    
void
PktLogger::logItemCommons(const itemStruct *item)
{
    outputf(" %d %d %d ", 
        item->common.STR, item->common.STA, item->common.CHA);
    outputf("%d %d %d ", 
        item->common.DEX, item->common.INT, item->common.AGI);
    outputf("%d %d %d ", 
        item->common.WIS, item->common.MR, item->common.FR);
    outputf("%d %d %d ", 
        item->common.CR, item->common.DR, item->common.PR);
    outputf("%d %d %d ", 
        item->common.HP, item->common.MANA, item->common.AC);
    output(item->common.unknown0187, 2);
    outputf(" %d %d %d ", 
        item->common.light, item->common.delay, item->common.damage);
    output(&item->common.unknown0192, 1);
    outputf(" %d %d ",
        item->common.range,item->common.skill);
    outputf("%d %d %d ", 
        item->common.light, item->common.delay, item->common.damage);
    outputf("%d %d %d ", 
        item->common.magic, item->common.level0, item->common.material);
    output(item->common.unknown0198, 2);
    outputf(" %d  ", item->common.color);
    output(item->common.unknown0204, 2);
    outputf(" %d %d ", 
        item->common.spellId0, item->common.classes);
    output(item->common.unknown0210, 2);
    outputf(" ");
    return;
}

void
PktLogger::logContainerItem(const itemStruct *item)
{
    logItemHeader(item);
    logItemCommons(item);
    output(&item->common.container.unknown0212,1);
    outputf(" %d ", item->common.container.numSlots);
    output(&item->common.container.unknown0214,1);
    outputf(" %d ", item->common.container.sizeCapacity);
    outputf("%d ", item->common.container.weightReduction);
    outputf("%d %d ", item->common.level, item->common.charges);
    output(&item->common.unknown0219, 1);
    outputf(" %d ", item->common.spellId);
    output(item->common.unknown0222, 70);
}   
    
void
PktLogger::logNormalItem(const itemStruct *item)
{
    logItemHeader(item);
    logItemCommons(item);
    outputf(" %d ", item->common.normal.races);
    output(item->common.normal.unknown0214, 3);
    outputf(" %d %d", item->common.level, item->common.charges);
    output(&item->common.unknown0219, 1);
    outputf(" %d ", item->common.spellId);
    output(item->common.unknown0222, 70);
}

void
PktLogger::logItem(const itemStruct *item)
{
    if (item->flag == 0x7669) /* book */
        logBookItem(item);
    else if (item->flag == 0x5400) /* container */   
        logContainerItem(item);
    else if (item->flag == 0x5450) /* container */   
        logContainerItem(item);
    else /* normal item */
        logNormalItem(item);
}

void
PktLogger::logZoneServerInfo(const void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logCPlayerItems(const cPlayerItemsStruct *citems, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logItemInShop(const itemInShopStruct *sitem, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        sitem->opCode, sitem->version);

    output(sitem->unknown0002, 4);
    outputf(" %d ", sitem->itemType);

    logItem(&sitem->item);

    outputf(" ");
    output(sitem->unknown0251, 4);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logMoneyOnCorpse(const moneyOnCorpseStruct *money, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logItemOnCorpse(const itemOnCorpseStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir,
        item->opCode, item->version);

    logItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logTradeItemIn(const tradeItemInStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    logItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logTradeItemOut(const tradeItemOutStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    output(item->unknown0002,6);
    outputf(" %d ", item->itemtype);

    logItem(&item->item);

    outputf(" ");
    output(item->unknown0253, 5);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logPlayerItem(const playerItemStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    logItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void
PktLogger::logSummonedItem(const summonedItemStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        item->opCode, item->version);

    logItem(&item->item);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logECharProfile(const charProfileStruct *data, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logCharProfile(const charProfileStruct *profile, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    const struct spellBuff *buff;
    int i;

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ",timestamp, len, dir, 
        profile->opCode, profile->version );

    output(profile->unknown0002,4);
    outputf(" [%.30s] [%.20s] %u %u ", profile->name, profile->lastName,
        profile->face, profile->race);
    output(&profile->unknown0059,1); 
    outputf(" %u %u %u ", profile->class_, profile->gender,
        profile->level);
    output(profile->unknown0063,3);
    outputf(" %u ",profile->exp);
    output(profile->unknown0070,2);
    outputf(" %u ",profile->MANA);
    output(profile->unknown0074,48);
    outputf(" %u ",profile->curHp);
    output(&profile->unknown0124,1);
    outputf(" %u %u %u ", profile->STR, profile->STA, profile->CHA);
    outputf("%u %u %u %u ", profile->DEX, profile->INT, profile->AGI,
        profile->WIS);

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

    output(profile->unknown0800, 1080);
    outputf(" ");
    
    for(i = 0; i < 256; i++)
        outputf("%d ", profile->sSpellBook[i]);

    for(i = 0; i < 8; i++)
        outputf("%d ", profile->sMemSpells[i]);
    
    output(profile->unknown2408, 54);
    outputf(" %u %u %u %u %u %u %u %u ", profile->platinum,
        profile->gold, profile->silver, profile->copper, profile->platinumBank,
        profile->goldBank, profile->silverBank, profile->copperBank);

    output(profile->unknown2494, 16); 
    outputf(" ");

    for(i = 0; i < 74; i++)
        outputf("%u ", profile->skills[i]);

    output(profile->unknown2584, 118); 

    outputf(" [%.144s] [%.20s] [%.20s] [%.20s] [%.20s] [%20s] ",
        profile->GUILD, profile->bindpoint, profile->miscnames[0],
        profile->miscnames[1], profile->miscnames[2], profile->miscnames[3]);

    output(profile->unknown2946, 1212); 
    outputf(" %u ",profile->deity);
    output(profile->unknown4160, 62); 
    outputf(" ");

    for(i = 0; i < 5; i++)
        outputf("[%.48s] ", &profile->GroupMembers[i][0]);

    output(profile->unknown4462, 72); 
    outputf(" %u ", profile->altexp);
    output(profile->unknown4538, 358); 
    outputf(" %u ", profile->aapoints);
    output(profile->unknown4897, 3209);
    outputf("\n");
    flush();
    return;
}

void
PktLogger::logNewCorpse(const newCorpseStruct *s, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u %u ", timestamp, len, dir,              s->opCode, s->version, s->spawnId, s->killerId);

    output(s->unknown0006, 4);
    outputf(" %u %d ", s->spellId, s->type);
    output(&s->unknown0013, 1);
    outputf(" %u ", s->damage);
    output(s->unknown0016, 2);
    outputf("\n");
    flush();
    return;
}

void
PktLogger::logDeleteSpawn(const deleteSpawnStruct *s, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir, 
        s->opCode, s->version, s->spawnId);
    output(s->unknown0004, 2);
    outputf("\n");
    flush();
}

void
PktLogger::logChannelMessage(const channelMessageStruct *msg, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    zoneTimestamp = timestamp; /* save for future Decrypt log */

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X ", timestamp, len, dir, 
        msg->opCode, msg->version);

    output(msg->unknown0002,32);
    outputf(" [%.23s] ", msg->sender);
    output(msg->unknown0057,9);
    outputf(" %u ", msg->language);
    output(&msg->unknown0064,1);
    outputf(" %u ", msg->chanNum);
    output(msg->unknown0069,5);
    outputf(" [%s]\n", msg->message);
    flush();
    return;
}

void
PktLogger::logENewSpawn(const newSpawnStruct *spawn, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
        spawn->deltaHeading, spawn->yPos, spawn->xPos, spawn->zPos,
        spawn->deltaY, spawn->spacer1, spawn->deltaZ, spawn->spacer2,
        spawn->deltaZ);

    output(spawn->unknown0061, 1);
    outputf(" %u %d ", spawn->spawnId, spawn->typeflag);
    output(spawn->unknown0065, 1);

    outputf(" %u %d %d %u %u %u %u %u ", spawn->petOwnerId,
        spawn->maxHp, spawn->curHp, spawn->race, spawn->NPC, spawn->class_,
        spawn->Gender, spawn->level);

    output(spawn->unknown0077,4);
    outputf(" %u ", spawn->light);
    output(spawn->unknown0082, 10);
    outputf(" %u %u %u %u %u %u %u %u %u [%.30s] [%.20s] ", 
        spawn->equipment[0], spawn->equipment[1], spawn->equipment[2], 
        spawn->equipment[3], spawn->equipment[4], spawn->equipment[5], 
        spawn->equipment[6], spawn->equipment[7], spawn->equipment[8], 
        spawn->name, spawn->lastname);

    output(spawn->unknown0160, 2);
    outputf(" %u ", spawn->deity);
    output(spawn->unknown0164, 8);
    outputf(" ");
}

void
PktLogger::logNewSpawn(const newSpawnStruct *s, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logZoneSpawns(const zoneSpawnsStruct* zspawns, int len, int dir)
{
    int spawndatasize = (len - 2) / sizeof(spawnStruct);

    if (FP == NULL)
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
PktLogger::logEZoneSpawns(const zoneSpawnsStruct *zone, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    zoneTimestamp = timestamp; /* save for future Decrypt log */

    if (FP == NULL)
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
PktLogger::logTimeOfDay(const timeOfDayStruct *tday, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logBookText(const bookTextStruct *book, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    char *text;
    int tlen;

    if (FP == NULL)
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
PktLogger::logRandom(const randomStruct *ran, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    
    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u %u\n", timestamp, len, dir,
        ran->opCode, ran->version, ran->bottom, ran->top);

    flush();
}

void
PktLogger::logEmoteText(const emoteTextStruct *emote, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    
    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
        emote->opCode, emote->version);

    output(emote->unknown0002,2); 
    outputf(" [%s]\n", emote->text);
    flush();
}

void
PktLogger::logCorpseLoc(const corpseLocStruct *corpse, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
       corpse->opCode, corpse->version, corpse->spawnId);

    output(&corpse->unknown0004,2);
    outputf(" %f %f %f\n", corpse->xPos, corpse->yPos, corpse->zPos);
    flush();
    return;
}

void
PktLogger::logPlayerBook(const playerBookStruct *book, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       book->opCode, book->version);

    logItem(&book->item);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logPlayerContainer(const playerContainerStruct *container, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       container->opCode, container->version);

    logItem(&container->item);

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logInspectData(const inspectDataStruct *data, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    int i;

    if (FP == NULL)
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
PktLogger::logHPUpdate(const hpUpdateStruct *hp, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logSPMesg(const spMesgStruct *msg, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %d [%s]\n", timestamp, len, dir,
       msg->opCode, msg->version, msg->msgType, msg->message);

    flush();
}

void 
PktLogger::logMemSpell(const memSpellStruct *spell, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logBeginCast(const beginCastStruct *spell, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u %u %d %d\n", timestamp, len,
       dir, spell->opCode, spell->version, spell->spawnId, spell->spellId, 
       spell->param1, spell->param2);

    flush();
}

void 
PktLogger::logStartCast(const startCastStruct *spell, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
       spell->opCode, spell->version);

    output(&spell->unknown0002, 2);
    outputf(" %u ", spell->spellId);
    output(&spell->unknown0006, 4);
    outputf(" ");
    output(&spell->unknown0008, 4);
    outputf(" %u ", spell->targetId);
    output(spell->unknown0014, 4);
    outputf("\n");

    flush();
}

void 
PktLogger::logMobUpdate(const mobUpdateStruct *update, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    const spawnPositionUpdate *pos;
    int i;

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %d ", timestamp, len, dir,
       update->opCode, update->version, update->numUpdates);

    for(i = 0; i < update->numUpdates; i++)
    {
        pos = &update->spawnUpdate[i];
        outputf("%u %d %d %d %d %d %d %d %u %d %u %d ",
            pos->spawnId, pos->animation, pos->heading,
            pos->deltaHeading, pos->yPos, pos->xPos,
            pos->zPos, pos->deltaY, pos->spacer1,
            pos->deltaX, pos->spacer2, pos->deltaZ);
    }
    outputf("\n");
    flush();
}

void 
PktLogger::logExpUpdate(const expUpdateStruct *exp, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logAltExpUpdate(const altExpUpdateStruct *alt, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logLevelUpUpdate(const levelUpUpdateStruct *level, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u %u %u\n", timestamp, len, dir,
        level->opCode, level->version, level->level, level->levelOld,
        level->exp);

    flush();
    return;
}

void 
PktLogger::logSkillInc(const skillIncStruct *skill, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logDoorOpen(void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logIllusion(void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logBadCast(const badCastStruct *spell, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X %u [%s]\n", timestamp, len, dir,
        spell->opCode, spell->version, spell->spawnId, spell->message);

    flush();
    return;
}

void 
PktLogger::logSysMsg(const sysMsgStruct *msg, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%2.X [%s]\n", timestamp, len, dir,
        msg->opCode, msg->version, msg->message);

    flush();
    return;
}

void
PktLogger::logZoneEntry(const void *data, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    if (dir == 2)
    {
        const ServerZoneEntryStruct *zone = (ServerZoneEntryStruct *) data;

        outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
            zone->opCode,zone->version);

        output(zone->unknown0002, 5);
        outputf(" [%s] [%s] ", zone->name, zone->zoneShortName);
        output(zone->unknown0052, 107);
        outputf(" %u %u ", zone->class_, zone->race);
        output(zone->unknown0161, 2);
        outputf(" %u ", zone->level);
        output(zone->unknown0164, 148);
        outputf(" %u ", zone->deity);
        output(&zone->unknown0314, 8);
        outputf("\n");
    }
    else
    {
        const ClientZoneEntryStruct *zone = (ClientZoneEntryStruct *) data;

        outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
            zone->opCode,zone->version);

        output(&zone->unknown0002,4);
        outputf(" [%.30s] ", zone->name);
        output(&zone->unknown0036,2);
        outputf("\n");
    }

    flush();
    return;
}

void
PktLogger::logNewZone(const newZoneStruct *zone, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X [%s] [%s] ", timestamp, len, dir,
        zone->opCode,zone->version, zone->charName, zone->shortName);

    output(zone->unknown0047, 5);
    outputf(" [%s] ", zone->longName);
    output(zone->unknown0232, 170);
    outputf("\n");

    flush();

    return;
}

void 
PktLogger::logZoneChange(const zoneChangeStruct *zone, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X [%.32s] [%.16s] ", timestamp, len,
        dir, zone->opCode, zone->version, zone->charName, zone->zoneName);

    output(zone->unknown0050, 20);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logPlayerPos(const playerPosStruct *pos, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;

    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
        pos->opCode, pos->version, pos->spawnId);

    output(pos->unknown0004,1);
    outputf(" %d %d %d %d %d %d %u %d %u %d\n", pos->heading, 
        pos->deltaHeading, pos->yPos, pos->xPos, pos->zPos, pos->deltaY,
        pos->spacer1, pos->deltaZ, pos->spacer2, pos->deltaX);

    flush();
    return;
}

void 
PktLogger::logWearChange(const wearChangeStruct *wear, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
        wear->opCode,wear->version, wear->spawnId);

    output(wear->unknown0004,2);
    outputf("%d %u ", wear->wearSlotId, wear->newItemId);
    output(wear->unknown0008,10);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logAction(const actionStruct *action, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %u ", timestamp, len, dir,
        action->opCode, action->version, action->target);

    output(action->unknown0004,2);
    outputf(" %u ",action->source);
    output(action->unknown0008,2);
    outputf(" %d ",action->type);
    output(&action->unknown0011,1);
    outputf(" %d %d ",action->spell, action->damage);
    output(action->unknown0018,12);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logCastOn(const castOnStruct *spell, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logManaDecrement(const manaDecrementStruct *mana, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %d %d\n", timestamp, len, dir,
        mana->opCode, mana->version, mana->newMana, mana->spellID);

    flush();
    return;
}

void 
PktLogger::logStamina(const staminaStruct *stamina, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X %d %d %d\n", timestamp, len, dir,
        stamina->opCode, stamina->version, stamina->food, stamina->water, 
        stamina->fatigue);

    flush();
    return;
}

void
PktLogger::logMakeDrop(const makeDropStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir, 
        item->opCode, item->version);

    output(item->unknown0002,8);
    outputf(" %u ", item->itemNr);
    output(item->unknown0012,2);
    outputf(" %u ", item->dropId);
    output(item->unknown0016,26);
    outputf(" %f %f %f ", item->yPos, item->xPos, item->zPos);
    output(item->unknown0054,4);
    outputf(" [%.16s] ", item->idFile);
    output(item->unknown0074,168);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logRemDrop(const remDropStruct *item, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logDropCoins(const dropCoinsStruct *coins, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%.2X ", timestamp, len, dir,
        coins->opCode, coins->version);

    output(coins->unknown0002,24);
    outputf(" %u ",coins->dropId);
    output(coins->unknown0028,22);
    outputf(" %u ", coins->amount);
    output(coins->unknown0054,4);
    outputf(" %f %f %f ", coins->yPos, coins->xPos, coins->zPos);
    output(coins->unknown0070,12);
    outputf(" [%.15s] ", coins->type);

    output(coins->unknown0097,17);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logRemoveCoins(const removeCoinsStruct *coins, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logOpenVendor(const void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logCloseVendor(const void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logOpenGM(const void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logCloseGM(const void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logSpawnAppearance(const spawnAppearanceStruct *spawn, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logAttack2(const attack2Struct *attack, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logConsider(const considerStruct *consider, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir, 
        consider->opCode, consider->version, consider->playerid);

    output(consider->unknown0004,2);
    outputf(" %u ", consider->targetid);
    output(consider->unknown0008,2);
    outputf(" %d %d %d %d ", consider->faction, consider->level,
        consider->curHp, consider->maxHp);
    output(&consider->unknown0026,4);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logNewGuildInZone(const newGuildInZoneStruct *guild, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logMoneyUpdate(const moneyUpdateStruct *money, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logMoneyThing(const moneyThingStruct *thing, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logClientTarget(const clientTargetStruct *target, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        target->opCode, target->version, target->newTarget);

    output(&target->unknown0004,2);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logBindWound(const bindWoundStruct *bind, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logCDoorSpawns(const cDoorSpawnsStruct *doors, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
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
PktLogger::logDoorSpawns(const doorSpawnsStruct *doors, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);
    int i;
    
    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X %u ", timestamp, len, dir,
        doors->opCode, doors->version, doors->count);

    for(i = 0; i < doors->count; i++)
    {
        outputf("[%.8s] ", doors->doors[i].name);
        output(doors->doors[i].unknown0008,8);
        outputf(" %f %f %f ",doors->doors[i].yPos,
            doors->doors[i].xPos, doors->doors[i].zPos);
        output(doors->doors[i].unknown0032,12);
        outputf(" %d %d ", doors->doors[i].doorId,
            doors->doors[i].size);
        output(doors->doors[i].unknown0042,6);
        outputf(" ");
    }

    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logGroupInfo(const groupInfoStruct *group, int len, int dir)
{
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("R %u %04d %d %.2X%2.X [%.32s] [%.32s] ", timestamp, len,
        dir, group->opCode, group->version, group->yourname, group->membername);

    output(group->unknown0066, 35);
    outputf(" %u ", group->bgARC);
    output(group->unknown0102, 83);
    outputf(" %d %d ", group->oper, group->ARC2);
    output(group->unknown0187, 43);
    outputf("\n");
    flush();
    return;
}

void 
PktLogger::logUnknownOpcode(void *data, int len, int dir)
{
    struct opCode *op = (struct opCode *) data;
    unsigned int timestamp = (unsigned int) time(NULL);

    if (FP == NULL)
        if (logOpen() != 0)
            return;
 
    outputf("U %u %04d %d %.2X%.2X ", timestamp, len, dir,
        op->code, op->version);

    output(data,len);
    outputf("\n");
    flush();
    return;
}

