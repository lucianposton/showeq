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

#define GuildListCode               0x9241

/***********************/
/* Zone Server OpCodes */
#define NewSpawnCode                0x023f

#define MobUpdateCode               0x0002

#define ConsiderCode                0x0136

#define ChannelMessageCode          0x00fc

#define PlayerPosCode               0x00ff

#define ZoneChangeCode              0x00a9

#define ZoneEntryCode               0x02ae

#define NewZoneCode                 0x0056

#define CPlayerItemsCode            0x01e9

#define ClientTargetCode            0x014b

#define ZoneSpawnsCode              0x0148

#define CharProfileCode             0x002e

#define DeleteSpawnCode             0x001e

#define NewCorpseCode               0x0042

#define ActionCode                  0x004d

#define CorpseLocCode               0x0014

#define CDoorSpawnsCode             0x01ea

#define ManaDecrementCode           0x0175

#define HPUpdateCode                0x0101

/*********************/
/* outdated          */

#define DropCoinsCode               0x0740

#define cStartCampingCode           0x0742

#define RemoveCoinsCode             0x0820

#define OpenVendorCode              0x0b40

#define sWhoAllOutputCode           0x0b20

#define cItemInShopCode             0x0c40

#define SysMsgCode                  0x1420

#define EmoteTextCode               0x1540

#define PlayerAACode                0x1522

#define cRunToggleCode              0x1f40

#define cJumpCode                   0x2040

#define AltExpUpdateCode            0x2342

#define GroupInfoCode               0x2640

#define MoneyThingCode              0x2820

#define RemDropCode                 0x0025

#define cCursorItemCode             0x2c41

#define MakeDropCode                0x0021

#define CharInfo                    0x2e40

#define TradeContainerInCode        0x3040

#define TradeItemInCode             0x3140

#define xBuffDropCode               0x3241

#define TradeBookInCode             0x3440

#define cConCorpseCode              0x3442

#define xBuyItemCode                0x3540

#define sSpellFizzleRegainCode      0x3542

#define FormattedMessageCode        0x3642

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

#define CompressedWrapCode          0x7642

#define PacketWrapCode              0x7742

#define SummonedItemCode            0x7841

#define SummonedContainerCode       0x7921

#define NewGuildInZoneCode          0x7b21

#define StartCastCode               0x0174

#define SPMesgCode                  0x8041

#define MemSpellCode                0x0178

#define BindWoundCode               0x8341

#define cTrackCode                  0x8441

#define cSneakCode                  0x8521

#define cHideCode                   0x8641

#define cSenseHeadingCode           0x8741

#define SkillIncCode                0x8941

#define DoorOpenCode                0x8e40

#define IllusionCode                0x9140

#define WearChangeCode              0x9240

#define cForageCode                 0x9440

#define DoorSpawnCode               0x9520

#define LevelUpUpdateCode           0x9841

#define ExpUpdateCode               0x9941

#define OpenGMCode                  0x9c20

#define sLootItemCode               0xa040

#define Attack2Code                 0xa140

#define BeginCastCode               0xa940

#define InspectDataCode             0xb640

#define BookTextCode                0xce40

#define xTradeSpellBookSlotsCode    0xce41

#define BadCastCode                 0xd321

#define TradeItemOutCode            0xdf40

#define RandomCode                  0xe741

#define LFGCode                     0xf041

#define TimeOfDayCode               0x00fe

#define cWhoAllCode                 0xf440

#define SpawnAppearanceCode         0xf540

#define cChatFiltersCode            0xff41

#endif // OPCODES_H
