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

#define GuildListCode               0x0059 // 08/26/03

#define MOTDCode                    0x01b0 // 08/26/03

/***********************/
/* Zone Server OpCodes */

#define ItemQueryInfoCode           0x0015 // 08/26/03

#define ChannelMessageCode          0x001f // 08/26/03

#define TimeOfDayCode               0x0021 // 08/26/03

#define PlayerPosCode               0x0022 // 08/26/03

#define MobUpdateCode               0x0039 // 08/26/03

#define cWhoAllCode                 0x0052 // 08/26/03

#define GuildMemberUpdate           0x0055 // 08/26/03

#define SkillIncCode                0x0060 // 08/26/03

#define BuyItemCode                 0x0061 // 08/26/03

#define SellItemCode                0x0066 // 08/26/03

#define CharProfileCode             0x0067 // 08/26/03

#define CloseVendorCode             0x0068 // 08/26/03

#define LevelUpUpdateCode           0x0074 // 08/26/03

#define ExpUpdateCode               0x0075 // 08/26/03

#define RandomCode                  0x0083 // 08/26/03

#define cRunToggleCode              0x0088 // 08/26/03

#define StartCastCode               0x00bb // 08/26/03

#define MemSpellCode                0x00bf // 08/26/03

#define cSenseHeadingCode           0x00c0 // 08/26/03

#define cJumpCode                   0x00d0 // 08/26/03

#define CorpseLocCode               0x00d1 // 08/26/03

#define ActionCode                  0x00db // 08/26/03

#define NewZoneCode                 0x00e4 // 08/26/03

#define EmoteTextCode               0x00eb // 08/26/03

#define DeleteSpawnCode             0x00ec // 08/26/03

#define OpenVendorCode              0x00f0 // 08/26/03

#define RemDropCode                 0x00f2 // 08/26/03

#define MakeDropCode                0x00f3 // 08/26/03

#define NewCorpseCode               0x00fe // 08/26/03

#define cLootCorpseCode             0x0112 // 08/26/03

#define cDoneLootingCode            0x0113 // 08/26/03

#define MoneyOnCorpseCode           0x0114 // 08/26/03

#define DoorOpenCode                0x0121 // 08/26/03

#define SpawnUpdateCode             0x0125 // 08/26/03

#define OpenGMCode                  0x012f // 08/26/03

#define ZoneChangeCode              0x0136 // 08/26/03

#define SpawnAppearanceCode         0x013a // 08/26/03

#define xBuffDropCode               0x0150 // 08/26/03

#define WeatherCode                 0x0154 // 08/26/03

#define ConsiderCode                0x0155 // 08/26/03

#define ConsumeCode                 0x0160 // 08/26/03

#define StaminaCode                 0x0161 // 08/26/03

#define ZoneSpawnsCode              0x0169 // 08/26/03

#define LogoutCode                  0x0163 // 08/26/03

#define ClientTargetCode            0x016c // 08/26/03

#define SkillTrainCode              0x016e // 08/26/03

#define CloseGMCode                 0x0171 // 08/26/03

#define sDoneLootingCode            0x0172 // 08/26/03

#define RandomReqCode               0x0197 // 08/26/03

#define LFGCode                     0x01af // 08/26/03

#define DoorSpawnsCode              0x01b8 // 08/26/03

#define cStartCampingCode           0x01c7 // 08/26/03

#define cConCorpseCode              0x01db // 08/26/03

#define SimpleMessageCode           0x01dc // 08/26/03

#define FormattedMessageCode        0x01dd // 08/26/03

#define BazaarSearchCode            0x01ec // 08/26/03

#define OpenTraderCode              0x01f0 // 08/26/03

#define ItemInfoCode                0x01fa // 08/26/03

#define NewSpawnCode                0x0201 // 08/26/03

#define sWhoAllOutputCode           0x0212 // 08/26/03

#define ZoneEntryCode               0x0224 // 08/26/03

#define InspectRequestCode          0x0231 // 08/26/03

#define InspectDataCode             0x0232 // 08/26/03

#define LFGReqCode                  0x0259 // 08/26/03

#define ItemCode                    0x02c5 // 08/26/03

#define NpcHpUpdateCode             0x022d // 08/26/03

#define BookTextCode                0x027e // 08/26/03


/*********************/
/* out of date          */

#define ManaDecrementCode           0xf175

#define AltExpUpdateCode            0xf209

#define BeginCastCode               0x0016

#define cItemInShopCode             0x007f

#define cCursorItemCode             0x0149

#define GroupInfoCode               0x0263

#define SysMsgCode                  0x1420

#define PlayerAACode                0x1522

#define MoneyThingCode              0x2820

#define CharInfo                    0x2e40

#define TradeContainerInCode        0x3040

#define TradeItemInCode             0x3140

#define TradeBookInCode             0x3440

#define sSpellFizzleRegainCode      0x3542

#define MoneyUpdateCode             0x3d41

#define GroupAcceptCode             0x3d20

#define GroupInviteCode             0x3e20

#define GroupDeclineCode            0x4140

#define sSpellInterruptedCode       0x4142

#define GroupDeleteCode             0x4420

#define CastOnCode                  0x4640

#define ItemOnCorpseCode            0x5220

#define CharUpdateCode              0x5521

#define cOpenSpellBookCode          0x5821

#define SummonedItemCode            0x7841

#define SummonedContainerCode       0x7921

#define NewGuildInZoneCode          0x7b21

#define SPMesgCode                  0x8041

#define BindWoundCode               0x8341

#define cTrackCode                  0x8441

#define cSneakCode                  0x8521

#define cHideCode                   0x8641

#define IllusionCode                0x9140

#define WearChangeCode              0x9240

#define cForageCode                 0x9440

#define sLootItemCode               0xa040

#define Attack2Code                 0xa140

#define xTradeSpellBookSlotsCode    0xce41

#define BadCastCode                 0xd321

#define TradeItemOutCode            0xdf40

#define cChatFiltersCode            0xff41

// Obsoleted Item Codes, new code is 0x02c5 with entirely different format
// and handling as of 08/26/03
#define PlayerItemsCode             0x01dc

#define PlayerContainerCode         0x018c

#define PlayerBookCode              0x018d

#define PlayerItemCode              0x018f

#define DropCoinsCode               0x0740 // Long since dead

#define RemoveCoinsCode             0x0820 // Long since dead

#endif // OPCODES_H
