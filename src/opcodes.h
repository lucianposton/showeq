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
#define DropCoinsVer                3
#define DropCoinsCode               0x0720

#define ChannelMessageVer           3
#define ChannelMessageCode          0x0721

#define RemoveCoinsVer              3
#define RemoveCoinsCode             0x0820

#define OpenVendorVer               3
#define OpenVendorCode              0x0b20

#define ItemInShopVer               3
#define ItemInShopCode              0x0c20

#define SysMsgVer                   3
#define SysMsgCode                  0x1420

#define EmoteTextVer                3
#define EmoteTextCode               0x1520

#define CorpseLocVer                3
#define CorpseLocCode               0x2120

#define AltExpUpdateVer             3
#define AltExpUpdateCode            0x2322 

#define GroupInfoVer                3
#define GroupInfoCode               0x2640

#define MoneyThingVer               3
#define MoneyThingCode              0x2820

#define ZoneEntryVer                3
#define ZoneEntryCode               0x2920

#define DeleteSpawnVer              3
#define DeleteSpawnCode             0x2a20

#define RemDropVer                  3
#define RemDropCode                 0x2c20

#define MakeDropVer                 3
#define MakeDropCode                0x2d20

#define TradeItemInVer              3
#define TradeItemInCode             0x3120

#define CharProfileVer              3
#define CharProfileCode             0x3620

#define ConsiderVer                 3
#define ConsiderCode                0x3721

#define MoneyUpdateVer              3
#define MoneyUpdateCode             0x3d21

#define CloseGMVer                  3
#define CloseGMCode                 0x4321

#define CloseVendorVer              3
#define CloseVendorCode             0x4521

#define CastOnVer                   3
#define CastOnCode                  0x4620

#define NewSpawnVer                 3
#define NewSpawnCode                0x4921

#define NewCorpseVer                3
#define NewCorpseCode               0x4a20

#define MoneyOnCorpseVer            3
#define MoneyOnCorpseCode           0x5020

#define ItemOnCorpseVer             3
#define ItemOnCorpseCode            0x5220

#define StaminaVer                  3
#define StaminaCode                 0x5721

#define ActionVer                   3
#define ActionCode                  0x5820

#define NewZoneVer                  3
#define NewZoneCode                 0x5b20

#define ZoneSpawnsVer               3
#define ZoneSpawnsCode              0x6121

#define ClientTargetVer             3
#define ClientTargetCode            0x6221

#define PlayerItemVer               3
#define PlayerItemCode              0x6421

#define PlayerBookVer               3
#define PlayerBookCode              0x6521

#define PlayerContainerVer          3
#define PlayerContainerCode         0x6621

#define SummonedItemVer             3
#define SummonedItemCode            0x7821

#define NewGuildInZoneVer           3
#define NewGuildInZoneCode          0x7b21

#define StartCastVer                3
#define StartCastCode               0x7e21

#define ManaDecrementVer            3
#define ManaDecrementCode           0x7f21

#define SPMesgVer                   3
#define SPMesgCode                  0x8021

#define MemSpellVer                 3
#define MemSpellCode                0x8221

#define BindWoundVer                3
#define BindWoundCode               0x8321

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

#define OpenGMVer                   3
#define OpenGMCode                  0x9c20

#define MobUpdateVer                3
#define MobUpdateCode               0x9f20

#define Attack2Ver                  3
#define Attack2Code                 0xa120

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

#define TradeItemOutVer             3
#define TradeItemOutCode            0xdf20

#define RandomVer                   3
#define RandomCode                  0xe721

#define TimeOfDayVer                3
#define TimeOfDayCode               0xf220

#define PlayerPosVer                3
#define PlayerPosCode               0xf320

#define SpawnAppearanceVer          3
#define SpawnAppearanceCode         0xf520

#define CPlayerItemsVer             3
#define CPlayerItemsCode            0xf621

#define CDoorSpawnsVer              3
#define CDoorSpawnsCode             0xf721

#endif // OPCODES_H
