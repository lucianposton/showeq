/*
** opcodes.h
**
** ShowEQ Distributed under GPL
** http://sourceforge.net/projects/seq/
**/

#ifndef OPCODES_H
#define OPCODES_H

/*
** Please be kind and remember to correctly re-order
** the values in here whenever you add a new item,
** thanks.  - Andon
*/

/************************/
/* World Server OpCodes */

#define ZoneServerInfo              0x0480

#define ClientHashCode              0x3941

#define GuildListCode               0x0f00

/***********************/
/* Zone Server OpCodes */


//#define ZoneEntryCode               0x0010
#define ZoneEntryCode               0x0254

//#define CharProfileCode             0x0084
#define CharProfileCode             0x006b

//#define ChannelMessageCode          0x0133
#define ChannelMessageCode          0x001f

//#define PlayerPosCode               0x0136
#define PlayerPosCode               0x0022

//#define ConsiderCode                0x0165
#define ConsiderCode                0x0175

//#define ZoneSpawnsCode              0x0179
#define ZoneSpawnsCode              0x0189

//#define ClientTargetCode            0x017c
#define ClientTargetCode            0x018c

//#define CPlayerItemsCode            0x01f6
#define CPlayerItemsCode            0x01dd

//#define NewSpawnCode                0x023a
#define NewSpawnCode                0x0229

//#define MobUpdateCode               0x0242
#define MobUpdateCode               0x0039

//#define sWhoAllOutputCode           0x025e
#define sWhoAllOutputCode           0x023e

//#define NewZoneCode                 0x00ba
#define NewZoneCode                 0x00f0

//#define DeleteSpawnCode             0x00ca
#define DeleteSpawnCode             0x00f8

//#define EmoteTextCode               0x00c1
#define EmoteTextCode               0x00f7

//#define MakeDropCode                0x00d1
#define MakeDropCode                0x00ff

//#define DoorSpawnsCode              0x01f7
#define DoorSpawnsCode              0x01de

#define GuildMemberUpdate           0x02a0

#define NpcHpUpdateCode             0x0263

//#define SpawnUpdateCode             0x014a
#define SpawnUpdateCode             0x015a

//#define NewCorpseCode               0x00e5
#define NewCorpseCode               0x0115

/*********************/
/* outdated          */

#define ZoneChangeCode              0xff01

#define CorpseLocCode               0xf014

#define RemDropCode 				0x00fe

#define ActionCode                  0xf04d

#define TimeOfDayCode               0xf0fe

#define SpawnAppearanceCode         0xf101

#define ManaDecrementCode           0xf175

#define StartCastCode 				0x00c6

#define MemSpellCode                0xf178

#define ExpUpdateCode               0xf187

#define AltExpUpdateCode            0xf209

#define DropCoinsCode               0x0740

#define cStartCampingCode           0x0206

#define RemoveCoinsCode             0x0820

#define OpenVendorCode              0x00ce

#define cItemInShopCode             0x0080

#define SysMsgCode                  0x1420

#define PlayerAACode                0x1522

#define cRunToggleCode              0x1f40

#define cJumpCode                   0x00a6

#define GroupInfoCode               0x2640

#define MoneyThingCode              0x2820

#define cCursorItemCode             0x014a

#define CharInfo                    0x2e40

#define TradeContainerInCode        0x3040

#define TradeItemInCode             0x3140

#define xBuffDropCode               0x0160

#define TradeBookInCode             0x3440

#define cConCorpseCode              0x021a

#define xBuyItemCode                0x3540

#define sSpellFizzleRegainCode      0x3542

#define FormattedMessageCode        0x021c

#define MoneyUpdateCode             0x3d41

#define GroupAcceptCode             0x3d20

#define GroupInviteCode             0x3e20

#define GroupDeclineCode            0x4140

#define sSpellInterruptedCode       0x4142

#define CloseGMCode                 0x4321

#define GroupDeleteCode             0x4420

#define sDoneLootingCode            0x4421

#define CloseVendorCode             0x4541

#define CastOnCode                  0x4640

#define cLootCorpseCode             0x4e20

#define cDoneLootingCode            0x4f20

#define MoneyOnCorpseCode           0x5020

#define ItemOnCorpseCode            0x5220

#define CharUpdateCode              0x5521

#define StaminaCode                 0x5741

#define cOpenSpellBookCode          0x5821

#define PlayerItemCode              0x6441

#define PlayerBookCode              0x6541

#define PlayerContainerCode         0x6641

#define SummonedItemCode            0x7841

#define SummonedContainerCode       0x7921

#define NewGuildInZoneCode          0x7b21

#define SPMesgCode                  0x8041

#define BindWoundCode               0x8341

#define cTrackCode                  0x8441

#define cSneakCode                  0x8521

#define cHideCode                   0x8641

#define cSenseHeadingCode           0x006a

#define SkillIncCode                0x8941

#define DoorOpenCode                0x0111

#define IllusionCode                0x9140

#define WearChangeCode              0x9240

#define cForageCode                 0x9440

#define LevelUpUpdateCode           0x9841

#define OpenGMCode                  0x9c20

#define sLootItemCode               0xa040

#define Attack2Code                 0xa140

#define BeginCastCode               0xa940

#define InspectDataCode             0xb640

#define BookTextCode                0xffd2

#define xTradeSpellBookSlotsCode    0xce41

#define BadCastCode                 0xd321

#define TradeItemOutCode            0xdf40

#define RandomCode                  0xe741

#define LFGCode                     0xf041

#define cWhoAllCode                 0xf440

#define cChatFiltersCode            0xff41

#endif // OPCODES_H
