/*
 * logger.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#ifndef SEQLOGGER_H
#define SEQLOGGER_H

#include "spawn.h"
#include "util.h"

class SEQLogger
{
public:
    SEQLogger(const char *fname);
    SEQLogger(FILE *fp);
protected:
    int logOpen(void);
    int outputf(const char *fmt, ...);
    int output(const void *data, int length);
    void flush() { fflush(FP); }
    FILE *FP;
    char *filename;
    int errOpen;
};

#define ZoneServerInfoMask 0x00000001 /* mask 1 */
#define DropCoinsMask      0x00000002
#define ChannelMessageMask 0x00000004
#define RemoveCoinsMask    0x00000008
#define OpenVendorMask     0x00000010
#define ItemInShopMask     0x00000020
#define SysMsgMask         0x00000040
#define EmoteTextMask      0x00000080
#define CorpseLocMask      0x00000100
#define AltExpUpdateMask   0x00000200
#define GroupInfoMask      0x00000400
#define MoneyThingMask     0x00000800
#define ZoneEntryMask      0x00001000
#define DeleteSpawnMask    0x00002000
#define RemDropMask        0x00004000
#define MakeDropMask       0x00008000
#define TradeItemInMask    0x00010000
#define ECharProfileMask   0x00020000
#define ConsiderMask       0x00040000
#define MoneyUpdateMask    0x00080000
#define CloseGMMask        0x00100000
#define CloseVendorMask    0x00200000
#define CastOnMask         0x00400000
#define ENewSpawnMask      0x00800000
#define NewCorpseMask      0x01000000
#define MoneyOnCorpseMask  0x02000000
#define ItemOnCorpseMask   0x04000000
#define StaminaMask        0x08000000
#define ActionMask         0x10000000
#define NewZoneMask        0x20000000
#define EZoneSpawnsMask    0x40000000
#define ClientTargetMask   0x80000000 /* mask 1 */

#define PlayerItemMask      0x00000001 /* mask 2 */
#define PlayerBookMask      0x00000002
#define PlayerContainerMask 0x00000004
#define SummonedItemMask    0x00000008
#define NewGuildInZoneMask  0x00000010
#define StartCastMask       0x00000020
#define ManaDecrementMask   0x00000040
#define SPMesgMask          0x00000080
#define MemSpellMask        0x00000100
#define BindWoundMask       0x00000200
#define SkillIncMask        0x00000400
#define DoorOpenMask        0x00000800
#define IllusionMask        0x00001000
#define WearChangeMask      0x00002000
#define DoorSpawnsMask      0x00004000
#define LevelUpUpdateMask   0x00008000
#define ExpUpdateMask       0x00010000
#define OpenGMMask          0x00020000
#define MobUpdateMask       0x00040000
#define Attack2Mask         0x00080000
#define ZoneChangeMask      0x00100000
#define BeginCastMask       0x00200000
#define HPUpdateMask        0x00400000
#define InspectDataMask     0x00800000
#define BookTextMask        0x01000000
#define BadCastMask         0x02000000
#define TradeItemOutMask    0x04000000
#define RandomMask          0x08000000
#define TimeOfDayMask       0x10000000
#define PlayerPosMask       0x20000000
#define SpawnAppearanceMask 0x40000000
#define CPlayerItemsMask    0x80000000 /* mask 2 */

#define CDoorSpawnsMask     0x00000001 /* mask 3 */
#define ZoneSpawnsMask      0x00000002
#define CharProfileMask     0x00000004
#define NewSpawnMask        0x00000008
#define UnknownOpcodeMask   0x00000010

class PktLogger: public SEQLogger 
{
public:
    int  isLoggingZoneServerInfo()  { return( mask1 & ZoneServerInfoMask );  }
    int  isLoggingCPlayerItems()    { return( mask2 & CPlayerItemsMask );    }
    int  isLoggingItemInShop()      { return( mask1 & ItemInShopMask );      }
    int  isLoggingMoneyOnCorpse()   { return( mask1 & MoneyOnCorpseMask );   }
    int  isLoggingItemOnCorpse()    { return( mask1 & ItemOnCorpseMask );    }
    int  isLoggingTradeItemOut()    { return( mask2 & TradeItemOutMask );    }
    int  isLoggingTradeItemIn()     { return( mask1 & TradeItemInMask );     }
    int  isLoggingPlayerItem()      { return( mask2 & PlayerItemMask );      }
    int  isLoggingSummonedItem()    { return( mask2 & SummonedItemMask );    }
    int  isLoggingZoneEntry()       { return( mask1 & ZoneEntryMask );       }
    int  isLoggingECharProfile()    { return( mask1 & ECharProfileMask );    }
    int  isLoggingCharProfile()     { return( mask3 & CharProfileMask );     }
    int  isLoggingNewCorpse()       { return( mask1 & NewCorpseMask );       }
    int  isLoggingDeleteSpawn()     { return( mask1 & DeleteSpawnMask );     }
    int  isLoggingChannelMessage()  { return( mask1 & ChannelMessageMask );  }
    int  isLoggingENewSpawn()       { return( mask1 & ENewSpawnMask );       }
    int  isLoggingNewSpawn()        { return( mask3 & NewSpawnMask );        }
    int  isLoggingEZoneSpawns()     { return( mask1 & EZoneSpawnsMask );     }
    int  isLoggingZoneSpawns()      { return( mask3 & ZoneSpawnsMask );      }
    int  isLoggingTimeOfDay()       { return( mask1 & TimeOfDayMask );       }
    int  isLoggingBookText()        { return( mask2 & BookTextMask );        }
    int  isLoggingRandom()          { return( mask2 & RandomMask );          }
    int  isLoggingEmoteText()       { return( mask1 & EmoteTextMask );       }
    int  isLoggingCorpseLoc()       { return( mask1 & CorpseLocMask );       }
    int  isLoggingPlayerBook()      { return( mask2 & PlayerBookMask );      }
    int  isLoggingPlayerContainer() { return( mask2 & PlayerContainerMask ); }
    int  isLoggingInspectData()     { return( mask2 & InspectDataMask );     }
    int  isLoggingHPUpdate()        { return( mask2 & HPUpdateMask );        }
    int  isLoggingSPMesg()          { return( mask2 & SPMesgMask );          }
    int  isLoggingMemSpell()        { return( mask2 & MemSpellMask );        }
    int  isLoggingBeginCast()       { return( mask2 & BeginCastMask );       }
    int  isLoggingStartCast()       { return( mask2 & StartCastMask );       }
    int  isLoggingMobUpdate()       { return( mask2 & MobUpdateMask );       }
    int  isLoggingExpUpdate()       { return( mask2 & ExpUpdateMask );       }
    int  isLoggingAltExpUpdate()    { return( mask2 & AltExpUpdateMask );    }
    int  isLoggingLevelUpUpdate()   { return( mask2 & LevelUpUpdateMask );   }
    int  isLoggingSkillInc()        { return( mask2 & SkillIncMask );        }
    int  isLoggingDoorOpen()        { return( mask2 & DoorOpenMask );        }
    int  isLoggingIllusion()        { return( mask2 & IllusionMask );        }
    int  isLoggingBadCast()         { return( mask2 & BadCastMask );         }
    int  isLoggingSysMsg()          { return( mask1 & SysMsgMask );          }
    int  isLoggingZoneChange()      { return( mask2 & ZoneChangeMask );      }
    int  isLoggingNewZone()         { return( mask1 & NewZoneMask );         }
    int  isLoggingPlayerPos()       { return( mask2 & PlayerPosMask );       }
    int  isLoggingWearChange()      { return( mask2 & WearChangeMask );      }
    int  isLoggingAction()          { return( mask1 & ActionMask );          }
    int  isLoggingCastOn()          { return( mask1 & CastOnMask );          }
    int  isLoggingManaDecrement()   { return( mask2 & ManaDecrementMask );   }
    int  isLoggingStamina()         { return( mask1 & StaminaMask );         }
    int  isLoggingMakeDrop()        { return( mask1 & MakeDropMask );        }
    int  isLoggingRemDrop()         { return( mask1 & RemDropMask );         }
    int  isLoggingDropCoins()       { return( mask1 & DropCoinsMask );       }
    int  isLoggingRemoveCoins()     { return( mask1 & RemoveCoinsMask );     }
    int  isLoggingOpenVendor()      { return( mask1 & OpenVendorMask );      }
    int  isLoggingCloseVendor()     { return( mask1 & CloseVendorMask );     }
    int  isLoggingOpenGM()          { return( mask2 & OpenGMMask );          }
    int  isLoggingCloseGM()         { return( mask1 & CloseGMMask );         }
    int  isLoggingSpawnAppearance() { return( mask2 & SpawnAppearanceMask ); }
    int  isLoggingAttack2()         { return( mask2 & Attack2Mask );         }
    int  isLoggingConsider()        { return( mask1 & ConsiderMask );        }
    int  isLoggingNewGuildInZone()  { return( mask2 & NewGuildInZoneMask );  }
    int  isLoggingMoneyUpdate()     { return( mask1 & MoneyUpdateMask );     }
    int  isLoggingMoneyThing()      { return( mask1 & MoneyThingMask );      }
    int  isLoggingClientTarget()    { return( mask1 & ClientTargetMask );    }
    int  isLoggingBindWound()       { return( mask2 & BindWoundMask );       }
    int  isLoggingCDoorSpawns()     { return( mask2 & CDoorSpawnsMask );     }
    int  isLoggingGroupInfo()       { return( mask1 & GroupInfoMask );       }
    int  isLoggingUnknownOpcode()   { return( mask3 & UnknownOpcodeMask );   }
    int  isLoggingDoorSpawns()      { return( mask3 & DoorSpawnsMask );      }

    PktLogger(FILE *fp, unsigned mask1, unsigned mask2, unsigned mask3);
    PktLogger(const char *fname,unsigned mask1,unsigned mask2,unsigned mask3);
    PktLogger(FILE *fp, const char *maskstr);
    PktLogger(const char *fname, const char *maskstr);

    void logZoneSpawnsTimestamp(void);
    void logZoneServerInfo(const void *data,int len,int dir);
    void logCPlayerItems(const cPlayerItemsStruct *citems,int len,int dir);
    void logItemInShop(const itemInShopStruct *item,int len,int dir);
    void logMoneyOnCorpse(const moneyOnCorpseStruct *money,int len,int dir);
    void logItemOnCorpse(const itemOnCorpseStruct *item,int len,int dir);
    void logTradeItemOut(const tradeItemOutStruct *item,int len,int dir);
    void logTradeItemIn(const tradeItemInStruct *item,int len,int dir);
    void logPlayerItem(const playerItemStruct *item,int len,int dir);
    void logSummonedItem(const summonedItemStruct *item,int len,int dir);
    void logZoneEntry(const void *data,int len,int dir);
    void logECharProfile(const charProfileStruct *data,int len,int dir);
    void logCharProfile(const charProfileStruct *profile,int len,int dir);
    void logNewCorpse(const newCorpseStruct *corpse,int len,int dir);
    void logDeleteSpawn(const deleteSpawnStruct *spawn,int len,int dir);
    void logChannelMessage(const channelMessageStruct *msg,int len,int dir);
    void logENewSpawn(const newSpawnStruct *spawn,int len,int dir);
    void logNewSpawn(const newSpawnStruct *spawn,int len,int dir);
    void logEZoneSpawns(const zoneSpawnsStruct *spawns,int len,int dir);
    void logZoneSpawns(const zoneSpawnsStruct *spawns,int len,int dir);
    void logTimeOfDay(const timeOfDayStruct *tday,int len,int dir);
    void logBookText(const bookTextStruct *book,int len,int dir);
    void logRandom(const randomStruct *ran,int len,int dir);
    void logEmoteText(const emoteTextStruct *emote,int len,int dir);
    void logCorpseLoc(const corpseLocStruct *corpse,int len,int dir);
    void logPlayerBook(const playerBookStruct *item,int len,int dir);
    void logPlayerContainer(const playerContainerStruct *item,int len,int dir);
    void logInspectData(const inspectDataStruct *data,int len,int dir);
    void logHPUpdate(const hpUpdateStruct *hp,int len,int dir);
    void logSPMesg(const spMesgStruct *msg,int len,int dir);
    void logMemSpell(const memSpellStruct *spell,int len,int dir);
    void logBeginCast(const beginCastStruct *spell,int len,int dir);
    void logStartCast(const startCastStruct *spell,int len,int dir);
    void logMobUpdate(const mobUpdateStruct *update,int len,int dir);
    void logExpUpdate(const expUpdateStruct *exp,int len,int dir);
    void logAltExpUpdate(const altExpUpdateStruct *alt,int len,int dir);
    void logLevelUpUpdate(const levelUpUpdateStruct *level,int len,int dir);
    void logSkillInc(const skillIncStruct *skill,int len,int dir);
    void logDoorOpen(void *data,int len,int dir);
    void logIllusion(void *data,int len,int dir);
    void logBadCast(const badCastStruct *spell,int len,int dir);
    void logSysMsg(const sysMsgStruct *msg,int len,int dir);
    void logZoneChange(const zoneChangeStruct *zone,int len,int dir);
    void logNewZone(const newZoneStruct *zone,int len,int dir);
    void logPlayerPos(const playerPosStruct *pos,int len,int dir);
    void logWearChange(const wearChangeStruct *wear,int len,int dir);
    void logAction(const actionStruct *action,int len,int dir);
    void logCastOn(const castOnStruct *spell,int len,int dir);
    void logManaDecrement(const manaDecrementStruct *mana,int len,int dir);
    void logStamina(const staminaStruct *stamina,int len,int dir);
    void logMakeDrop(const makeDropStruct *item,int len,int dir);
    void logRemDrop(const remDropStruct *item,int len,int dir);
    void logDropCoins(const dropCoinsStruct *coins,int len,int dir);
    void logRemoveCoins(const removeCoinsStruct *coins,int len,int dir);
    void logOpenVendor(const void *data,int len,int dir);
    void logCloseVendor(const void *data,int len,int dir);
    void logOpenGM(const void *data,int len,int dir);
    void logCloseGM(const void *data,int len,int dir);
    void logSpawnAppearance(const spawnAppearanceStruct *spawn,int len,int dir);
    void logAttack2(const attack2Struct *attack,int len,int dir);
    void logConsider(const considerStruct *consider,int len,int dir);
    void logNewGuildInZone(const newGuildInZoneStruct *guild,int len,int dir);
    void logMoneyUpdate(const moneyUpdateStruct *money,int len,int dir);
    void logMoneyThing(const moneyThingStruct *thing,int len,int dir);
    void logClientTarget(const clientTargetStruct *target,int len,int dir);
    void logBindWound(const bindWoundStruct *bind,int len,int dir);
    void logCDoorSpawns(const cDoorSpawnsStruct *doors,int len,int dir);
    void logDoorSpawns(const doorSpawnsStruct *doors,int len,int dir);
    void logGroupInfo(const groupInfoStruct *guild,int len,int dir);
    void logUnknownOpcode(void *data,int len,int dir);

private:
    void logItemHeader(const itemStruct *item);
    void logBookItem(const itemStruct *item);
    void logNormalItem(const itemStruct *item);
    void logContainerItem(const itemStruct *item);
    void logItemCommons(const itemStruct *item);
    void logItem(const itemStruct *item);
    void logProcessMaskString(const char *maskstr, unsigned *m1, unsigned *m2, unsigned *m3);
    void logSpawnStruct(const spawnStruct *spawn);
    unsigned int mask1;
    unsigned int mask2;
    unsigned int mask3;
    unsigned zoneTimestamp;
};

class SpawnLogger: public SEQLogger 
{
public:
    SpawnLogger(const char *filename);
    SpawnLogger(FILE *fp);
    void logTimeSync(const timeOfDayStruct *tday);
    void logZoneSpawn(const spawnStruct *spawn);
    void logNewSpawn(const spawnStruct *spawn);
    void logKilledSpawn(const Spawn *spawn,const char *killer,int kid);
    void logDeleteSpawn(const Spawn *spawn);
    void logNewZone(const char *zone);

private:
    void logSpawnInfo(const char *type, const char *name, int id, int level, 
                      int xPos, int yPos, int zPos, time_t timeCurrent, 
                      const char *killer, int kid);
    int version;
    char zoneShortName[16];
    EQTime *l_time;
};

#endif
