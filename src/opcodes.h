/*
** opcodes.h
**
** ShowEQ Distributed under GPL
** http://sourceforge.net/projects/seq/
**/

#ifndef OPCODES_H
#define OPCODES_H

#define opCodeVersion               3


/*
** Please be kind and remember to correctly re-order
** the values in here whenever you add a new item,
** thanks.  - Andon
*/

/* World Server OpCodes */

#define ZoneServerInfo              0x0480

/* Zone Server OpCodes */
#define dropCoinsVer                3
#define dropCoinsCode               0x0720

#define ChannelMessageVer           3
#define ChannelMessageCode          0x0721

#define removeCoinsVer              3
#define removeCoinsCode             0x0820

#define openVendorVer               3
#define openVendorCode              0x0b20

#define ItemInShopVer               3
#define ItemInShopCode              0x0c20

#define SysMsgVer                   3
#define SysMsgCode                  0x1420

#define emoteTextVer                3
#define emoteTextCode               0x1520

#define corpseLocVer                3
#define corpseLocCode               0x2120

#define AltExpUpdateVer             3
#define AltExpUpdateCode            0x2322 

#define groupinfoVer                3
#define groupinfoCode               0x2640

#define moneyThingVer               3
#define moneyThingCode              0x2820

#define ZoneEntryVer                3
#define ZoneEntryCode               0x2920

#define DeleteSpawnVer              3
#define DeleteSpawnCode             0x2a20

#define RemDropVer                  3
#define RemDropCode                 0x2c20

#define MakeDropVer                 3
#define MakeDropCode                0x2d20

#define ItemTradeVer                3
#define ItemTradeCode               0x3120

#define CharProfileVer              3
#define CharProfileCode             0x3620

#define considerVer                 3
#define considerCode                0x3721

#define moneyUpdateVer              3
#define moneyUpdateCode             0x3d21

#define closeGMVer                  3
#define closeGMCode                 0x4321

#define closeVendorVer              3
#define closeVendorCode             0x4521

#define castOnVer                   3
#define castOnCode                  0x4620

#define NewSpawnVer                 3
#define NewSpawnCode                0x4921

#define CorpseVer                   3
#define CorpseCode                  0x4a20

#define MoneyOnCorpseVer            3
#define MoneyOnCorpseCode           0x5020

#define ItemOnCorpseVer             3
#define ItemOnCorpseCode            0x5220

#define staminaVer                  3
#define staminaCode                 0x5721

#define ActionVer                   3
#define ActionCode                  0x5820

#define NewZoneVer                  3
#define NewZoneCode                 0x5b20

#define ZoneSpawnsVer               3
#define ZoneSpawnsCode              0x6121

#define clientTargetVer             3
#define clientTargetCode            0x6221

#define PlayerItemVer               3
#define PlayerItemCode              0x6421

#define PlayerBookVer               3
#define PlayerBookCode              0x6521

#define PlayerContainerVer          3
#define PlayerContainerCode         0x6621

#define summonedItemVer             3
#define summonedItemCode            0x7821

#define newGuildInZoneVer           3
#define newGuildInZoneCode          0x7b21

#define StartCastVer                3
#define StartCastCode               0x7e21

#define manaDecrementVer            3
#define manaDecrementCode           0x7f21

#define SPMesgVer                   3
#define SPMesgCode                  0x8021

#define MemSpellVer                 3
#define MemSpellCode                0x8221

#define bindWoundVer                3
#define bindWoundCode               0x8321

#define SkillIncVer                 3
#define SkillIncCode                0x8921

#define DoorOpenVer                 3
#define DoorOpenCode                0x8e20

#define IllusionVer                 3
#define IllusionCode                0x9120

#define WearChangeVer               3
#define WearChangeCode              0x9220

#define DoorSpawnVer                3
#define DoorSpawnCode               0x9520

#define LevelUpUpdateVer            3
#define LevelUpUpdateCode           0x9821

#define ExpUpdateVer                3
#define ExpUpdateCode               0x9921

#define openGMVer                   3
#define openGMCode                  0x9c20

#define MobUpdateVer                3
#define MobUpdateCode               0x9f20

#define attack2Ver                  3
#define attack2Code                 0xa120

#define ZoneChangeVer               3
#define ZoneChangeCode              0xa320

#define BeginCastVer                3
#define BeginCastCode               0xa920

#define HPUpdateVer                 3
#define HPUpdateCode                0xb220

#define InspectDataVer              3
#define InspectDataCode             0xb620

#define BookTextVer                 3
#define BookTextCode                0xce20

#define BadCastVer                  3
#define BadCastCode                 0xd321

#define tradeItemVer                3
#define tradeItemCode               0xdf20

#define RandomVer                   3
#define RandomCode                  0xe721

#define TimeOfDayVer                3
#define TimeOfDayCode               0xf220

#define PlayerPosVer                3
#define PlayerPosCode               0xf320

#define spawnAppearanceVer          3
#define spawnAppearanceCode         0xf520

#define CompressedPlayerItemVer     3
#define CompressedPlayerItemCode    0xf621

#define CompressedDoorSpawnVer      3
#define CompressedDoorSpawnCode     0xf721

#endif // OPCODES_H
