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

#define GuildListCode               0x005c

/***********************/
/* Zone Server OpCodes */


#define ChannelMessageCode          0x001f // 

#define PlayerPosCode               0x0022 // 

#define MobUpdateCode               0x0039 //

#define GuildMemberUpdate           0x0055 //?

#define CharProfileCode             0x0067 //

#define StartCastCode               0x00bb //

#define MemSpellCode                0x00bf // 

#define NewZoneCode                 0x00e4 //

#define EmoteTextCode               0x00eb //

#define DeleteSpawnCode             0x00ec //

#define MakeDropCode                0x00f3 //

#define RemDropCode                 0x00fd //?

#define NewCorpseCode               0x00fe //

#define SpawnUpdateCode             0x013a //

#define ZoneChangeCode              0x0136 // 

#define xBuffDropCode               0x0150 //

#define ConsiderCode                0x0155 //

#define ZoneSpawnsCode              0x0169 //

#define ClientTargetCode            0x016c //

#define DoorSpawnsCode              0x01b8 //

#define FormattedMessageCode        0x01dd //

#define NewSpawnCode                0x0201 //

#define sWhoAllOutputCode           0x0212 //

#define ZoneEntryCode               0x0224 //

#define NpcHpUpdateCode             0x0999

#define CorpseLocCode               0xf014

#define ActionCode                  0xf04d

#define TimeOfDayCode               0xf0fe

#define ManaDecrementCode           0xf175

#define ExpUpdateCode               0xf187

#define AltExpUpdateCode            0xf209


// Obsoleted Item Codes, new code is 0x02c5 with entirely different format
// and handling as of 08/26/03
#define PlayerItemsCode             0x01dc

#define PlayerContainerCode         0x018c

#define PlayerBookCode              0x018d

#define PlayerItemCode              0x018f

/*********************/
/* out of date          */

#define SpawnAppearanceCode         0x0125 //

#define DropCoinsCode               0x0740

#define cStartCampingCode           0x0206

#define RemoveCoinsCode             0x0820

#define OpenVendorCode              0x00cd

#define cItemInShopCode             0x007f

#define SysMsgCode                  0x1420

#define PlayerAACode                0x1522

#define cRunToggleCode              0x1f40

#define cJumpCode                   0x00a5

#define GroupInfoCode               0x0263

#define MoneyThingCode              0x2820

#define cCursorItemCode             0x0149

#define CharInfo                    0x2e40

#define TradeContainerInCode        0x3040

#define TradeItemInCode             0x3140

#define TradeBookInCode             0x3440

#define cConCorpseCode              0x0219

#define xBuyItemCode                0x3540

#define sSpellFizzleRegainCode      0x3542

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

#define SummonedItemCode            0x7841

#define SummonedContainerCode       0x7921

#define NewGuildInZoneCode          0x7b21

#define SPMesgCode                  0x8041

#define BindWoundCode               0x8341

#define cTrackCode                  0x8441

#define cSneakCode                  0x8521

#define cHideCode                   0x8641

#define cSenseHeadingCode           0x006b

#define SkillIncCode                0x8941

#define DoorOpenCode                0x0110

#define IllusionCode                0x9140

#define WearChangeCode              0x9240

#define cForageCode                 0x9440

#define LevelUpUpdateCode           0x9841

#define OpenGMCode                  0x9c20

#define sLootItemCode               0xa040

#define Attack2Code                 0xa140

#define BeginCastCode               0x0016

#define InspectDataCode             0x0261

#define BookTextCode                0xffd2

#define xTradeSpellBookSlotsCode    0xce41

#define BadCastCode                 0xd321

#define TradeItemOutCode            0xdf40

#define RandomCode                  0xe741

#define LFGCode                     0xf041

#define cWhoAllCode                 0xf440

#define cChatFiltersCode            0xff41

#endif // OPCODES_H
