/*
 * logger.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#ifndef SEQLOGGER_H
#define SEQLOGGER_H

#include <qobject.h>

#include "spawn.h"
#include "util.h"

class SEQLogger : public QObject
{
   Q_OBJECT

public:
    SEQLogger(const QString& fname, QObject* parent=0, const char* name="SEQLogger");
    SEQLogger(FILE *fp, QObject* parent=0, const char* name="SEQLogger");
protected:
    int logOpen(void);
    int outputf(const char *fmt, ...);
    int output(const void *data, int length);
    void flush() { fflush(m_FP); }
    FILE* m_FP;
    QString m_filename;
    int m_errOpen;
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
#define SIUpdateMask        0x00400000
#define InspectDataMask     0x00800000
#define BookTextMask        0x01000000
#define BadCastMask         0x02000000
#define TradeItemOutMask    0x04000000
#define RandomMask          0x08000000
#define TimeOfDayMask       0x10000000
#define PlayerPosMask       0x20000000
#define SpawnAppearanceMask 0x40000000
#define PlayerItemsMask    0x80000000 /* mask 2 */

#define CDoorSpawnsMask     0x00000001 /* mask 3 */
#define ZoneSpawnsMask      0x00000002
#define CharProfileMask     0x00000004
#define NewSpawnMask        0x00000008
#define UnknownOpcodeMask   0x00000010

class PktLogger: public SEQLogger 
{
   Q_OBJECT

public:
public:
    int  isLoggingZoneServerInfo()  { return( mask1 & ZoneServerInfoMask );  }
    int  isLoggingPlayerItems()    { return( mask2 & PlayerItemsMask );    }
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
    int  isLoggingSIUpdate()        { return( mask2 & SIUpdateMask );        }
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
    PktLogger(const QString& fname,unsigned mask1,unsigned mask2,unsigned mask3);
    PktLogger(FILE *fp, const QString& maskstr);
    PktLogger(const QString& fname, const QString& maskstr);

 public slots:
    void logZoneSpawnsTimestamp(void);
    void logZoneServerInfo(const uint8_t* data, uint32_t len, uint8_t dir);
    void logPlayerItems(const playerItemsStruct* items, uint32_t len, uint8_t dir);
    void logItemInShop(const itemInShopStruct* item, uint32_t len, uint8_t dir);
    void logMoneyOnCorpse(const moneyOnCorpseStruct* money, uint32_t len, uint8_t dir);
    void logItemOnCorpse(const itemOnCorpseStruct* item, uint32_t len, uint8_t dir);
    void logTradeItemOut(const tradeItemOutStruct* item, uint32_t len, uint8_t dir);
    void logTradeItemIn(const tradeItemInStruct* item, uint32_t len, uint8_t dir);
    void logPlayerItem(const playerItemStruct* item, uint32_t len, uint8_t dir);
    void logSummonedItem(const summonedItemStruct* item, uint32_t len, uint8_t dir);
    void logZoneEntry(const ServerZoneEntryStruct* data, uint32_t len, uint8_t dir);
    void logZoneEntry(const ClientZoneEntryStruct* data, uint32_t len, uint8_t dir);
    void logECharProfile(const charProfileStruct* data, uint32_t len, uint8_t dir);
    void logCharProfile(const charProfileStruct* profile, uint32_t len, uint8_t dir);
    void logNewCorpse(const newCorpseStruct* corpse, uint32_t len, uint8_t dir);
    void logDeleteSpawn(const deleteSpawnStruct* spawn, uint32_t len, uint8_t dir);
    void logChannelMessage(const channelMessageStruct* msg, uint32_t len, uint8_t dir);
    void logENewSpawn(const newSpawnStruct* spawn, uint32_t len, uint8_t dir);
    void logNewSpawn(const newSpawnStruct* spawn, uint32_t len, uint8_t dir);
    void logEZoneSpawns(const zoneSpawnsStruct* spawns, uint32_t len, uint8_t dir);
    void logZoneSpawns(const zoneSpawnsStruct* spawns, uint32_t len, uint8_t dir);
    void logTimeOfDay(const timeOfDayStruct* tday, uint32_t len, uint8_t dir);
    void logBookText(const bookTextStruct* book, uint32_t len, uint8_t dir);
    void logRandom(const randomReqStruct* ran, uint32_t len, uint8_t dir);
    void logRandom(const randomStruct* ran, uint32_t len, uint8_t dir);
    void logEmoteText(const emoteTextStruct* emote, uint32_t len, uint8_t dir);
    void logCorpseLoc(const corpseLocStruct* corpse, uint32_t len, uint8_t dir);
    void logPlayerBook(const playerBookStruct* item, uint32_t len, uint8_t dir);
    void logPlayerContainer(const playerContainerStruct* item, uint32_t len, uint8_t dir);
    void logInspectData(const inspectDataStruct* data, uint32_t len, uint8_t dir);
    void logSpawnInfoUpdate(const SpawnUpdateStruct* spawnupdate, uint32_t len, uint8_t dir);
    void logSPMesg(const spMesgStruct* msg, uint32_t len, uint8_t dir);
    void logMemSpell(const memSpellStruct* spell, uint32_t len, uint8_t dir);
    void logBeginCast(const beginCastStruct* spell, uint32_t len, uint8_t dir);
    void logStartCast(const startCastStruct* spell, uint32_t len, uint8_t dir);
    void logMobUpdate(const spawnPositionUpdate* update, uint32_t len, uint8_t dir);
    void logExpUpdate(const expUpdateStruct* exp, uint32_t len, uint8_t dir);
    void logAltExpUpdate(const altExpUpdateStruct* alt, uint32_t len, uint8_t dir);
    void logLevelUpUpdate(const levelUpUpdateStruct* level, uint32_t len, uint8_t dir);
    void logSkillInc(const skillIncStruct* skill, uint32_t len, uint8_t dir);
    void logDoorOpen(const uint8_t* data, uint32_t len, uint8_t dir);
    void logIllusion(const uint8_t* data, uint32_t len, uint8_t dir);
    void logBadCast(const badCastStruct* spell, uint32_t len, uint8_t dir);
    void logSysMsg(const sysMsgStruct* msg, uint32_t len, uint8_t dir);
    void logZoneChange(const zoneChangeStruct* zone, uint32_t len, uint8_t dir);
    void logNewZone(const newZoneStruct* zone, uint32_t len, uint8_t dir);
    void logPlayerPos(const playerSpawnPosStruct* pos, uint32_t len, uint8_t dir);
    void logPlayerPos(const playerSelfPosStruct* pos, uint32_t len, uint8_t dir);
    void logWearChange(const wearChangeStruct* wear, uint32_t len, uint8_t dir);
    void logAction(const action2Struct* action, uint32_t len, uint8_t dir);
    void logCastOn(const castOnStruct* spell, uint32_t len, uint8_t dir);
    void logManaDecrement(const manaDecrementStruct* mana, uint32_t len, uint8_t dir);
    void logStamina(const staminaStruct* stamina, uint32_t len, uint8_t dir);
    void logMakeDrop(const makeDropStruct* item, uint32_t len, uint8_t dir);
    void logRemDrop(const remDropStruct* item, uint32_t len, uint8_t dir);
    void logDropCoins(const dropCoinsStruct* coins, uint32_t len, uint8_t dir);
    void logRemoveCoins(const removeCoinsStruct* coins, uint32_t len, uint8_t dir);
    void logOpenVendor(const uint8_t* data, uint32_t len, uint8_t dir);
    void logCloseVendor(const uint8_t* data, uint32_t len, uint8_t dir);
    void logOpenGM(const uint8_t* data, uint32_t len, uint8_t dir);
    void logCloseGM(const uint8_t* data, uint32_t len, uint8_t dir);
    void logSpawnAppearance(const spawnAppearanceStruct* spawn, uint32_t len, uint8_t dir);
    void logAttack2(const attack2Struct* attack, uint32_t len, uint8_t dir);
    void logConsider(const considerStruct* consider, uint32_t len, uint8_t dir);
    void logNewGuildInZone(const newGuildInZoneStruct* guild, uint32_t len, uint8_t dir);
    void logMoneyUpdate(const moneyUpdateStruct* money, uint32_t len, uint8_t dir);
    void logMoneyThing(const moneyThingStruct* thing, uint32_t len, uint8_t dir);
    void logClientTarget(const clientTargetStruct* target, uint32_t len, uint8_t dir);
    void logBindWound(const bindWoundStruct* bind, uint32_t len, uint8_t dir);
    void logDoorSpawns(const doorSpawnsStruct* doors, uint32_t len, uint8_t dir);
    void logGroupInfo(const groupMemberStruct* guild, uint32_t len, uint8_t dir);
    void logGroupInvite(const groupInviteStruct* guild, uint32_t len, uint8_t dir);
    void logGroupDecline(const groupDeclineStruct* guild, uint32_t len, uint8_t dir);
    void logGroupAccept(const groupAcceptStruct* guild, uint32_t len, uint8_t dir);
    void logGroupDelete(const groupDeleteStruct* guild, uint32_t len, uint8_t dir);
    void logUnknownOpcode(const uint8_t* data, uint32_t len, uint8_t dir);

private:
    void logItemHeader(const itemStruct *item);
    void logBookItem(const itemBookStruct *book);
    void logNormalItem(const itemItemStruct *item);
    void logContainerItem(const itemContainerStruct *item);
    void logItemCommons(const itemItemStruct *item);
    void logProcessMaskString(const char *maskstr, unsigned *m1, unsigned *m2, unsigned *m3);
    void logSpawnStruct(const spawnStruct *spawn);
    unsigned int mask1;
    unsigned int mask2;
    unsigned int mask3;
    unsigned zoneTimestamp;
};

class SpawnLogger: public SEQLogger 
{
   Q_OBJECT

public:
    SpawnLogger(const QString& filename);
    SpawnLogger(FILE *fp);

public slots:
    void logTimeSync(const timeOfDayStruct *tday);
    void logNewZone(const QString& zone);
    void logZoneSpawn(const newSpawnStruct* nspawn);
    void logZoneSpawn(const spawnStruct *spawn);
    void logZoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t len);
    void logNewSpawn(const newSpawnStruct* spawn);
    void logKilledSpawn(const Item* item, const Item* kitem, uint16_t kid);
    void logDeleteSpawn(const Item *spawn);

private:
    void logSpawnInfo(const char *type, const char *name, int id, int level, 
                      int x, int y, int z, time_t timeCurrent, 
                      const char *killer, int kid, int guildid);
    int version;
    QString zoneShortName;
    EQTime *l_time;
};

#endif
