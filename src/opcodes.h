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

/* 
 * NOTE: opcodes with #defines beginning with OP_ are curtesy of the
 *       EQEmu staff
 */

/************************/
/* World Server OpCodes */

#define OP_AckPacket                0x000d // 09/09/03 Appears to be generic ack at the presentation level

#define GuildListCode               0x0059 // 09/09/03

#define OP_ExpansionInfo            0x00da // 09/09/03 Which expansions user has

#define OP_DeleteCharacter          0x00e3 // 09/09/03 Delete character @ char select

#define OP_SendCharInfo             0x00fb // 09/09/03 Send all chars visible @ char select

#define OP_CharacterCreate          0x00fd // 09/09/03 Create character @ char select

#define OP_ApproveName              0x011e // 09/09/03 Approving new character name @ char creation

#define MOTDCode                    0x01b0 // 09/09/03

#define OP_World_Client_CRC1	    0x0153 // 09/09/03 Contains a snippet of spell data
#define OP_World_Client_CRC2        0x0157 // 09/09/03 Second client verification packet

#define OP_LogServer                0x017f

#define OP_ApproveWorld             0x0195

#define OP_SendLoginInfo            0x023a

#define OP_EnterWorld               0x024a // 09/09/03 Server approval for client to enter world

#define ZoneServerInfo              0x024d // 09/09/03

#define OP_SetChatServer            0x0252 // 09/09/03 Chatserver? IP,Port,servername.Charname,password(?)

/***********************/
/* Zone Server OpCodes */

#define OP_Consent                  0x0009 // 09/09/03 /consent

#define ItemQueryInfoCode           0x0015 // 09/09/03

#define OP_GMServers                0x0016 // 09/09/03 GM /servers  - ?

#define BeginCastCode               0x0017 // 09/09/03

#define ChannelMessageCode          0x001f // 09/09/03

#define TimeOfDayCode               0x0021 // 09/09/03

#define PlayerPosCode               0x0022 // 09/09/03

#define OP_TradeAcceptClick         0x0028 // 09/09/03

#define OP_TradeRequestAck          0x0032 // 09/09/03 Trade request recipient is acknowledging they are able to trade

#define MobUpdateCode               0x0039 // 09/09/03

#define OP_TradeSkillCombine        0x003d // 09/09/03
 
#define OP_GMFind                   0x0044 // 09/09/03 GM /find	- ?

#define WhoAllReqCode               0x0052 // 09/09/03

#define GuildMemberListCode         0x0055 // 09/09/03

#define SkillIncCode                0x0060 // 09/09/03

#define BuyItemCode                 0x0061 // 09/09/03

#define OP_Petition                 0x0064 // 09/09/03

#define SellItemCode                0x0066 // 09/09/03

#define CharProfileCode             0x0067 // 09/09/03

#define CloseVendorCode             0x0068 // 09/09/03

#define OP_GMBecomeNPC              0x0070 // GM /becomenpc - Become an NPC

#define LevelUpUpdateCode           0x0074 // 09/09/03

#define ExpUpdateCode               0x0075 // 09/09/03

#define RandomCode                  0x0083 // 09/09/03

#define OP_PetitionRefresh          0x0082 // 09/09/03

#define cRunToggleCode              0x0088 // 09/09/03

#define OP_GMSearchCorpse           0x0093 // 09/09/03 GM /searchcorpse - Search all zones for named corpse

#define OP_GuildPeace               0x0096 // 09/09/03 /guildpeace

#define OP_Hide                     0x009a // 09/09/03

#define SaveZoningPlayerCode        0x009d // 09/09/03

#define OP_GMLastName               0x009f // 09/09/03 GM /lastname - Change user lastname

#define OP_GuildLeader              0x00a3 // 09/09/03 /guildleader

#define OP_BoardBoat                0x00b8 // 09/09/03

#define OP_LeaveBoat                0x00b9 // 09/09/03

#define StartCastCode               0x00bb // 09/09/03

#define SpellFadeCode               0x00bd // 09/09/03

#define MemSpellCode                0x00bf // 09/09/03

#define cSenseHeadingCode           0x00c0 // 09/09/03

#define cJumpCode                   0x00d0 // 09/09/03

#define CorpseLocCode               0x00d1 // 09/09/03

#define OP_GMInquire                0x00d2 // 09/09/03 GM /inquire - Search soulmark data

#define OP_GMSoulmark               0x00d4 // 09/09/03 GM /praise /warn - Add soulmark comment to user file

#define OP_GMHideMe                 0x00d6 // 09/09/03 GM /hideme - Remove self from spawn lists and make invis

#define ActionCode                  0x00db // 09/09/03

#define NewZoneCode                 0x00e4 // 09/09/03

#define OP_ReqNewZone               0x00e5 // 09/09/03 Client requesting NewZone_Struct

#define EmoteTextCode               0x00eb // 09/09/03

#define OP_EnvDamage                0x00e1 // 09/09/03

#define OP_SafePoint                0x00e8 // 09/09/03

#define DeleteSpawnCode             0x00ec // 09/09/03

#define OpenVendorCode              0x00f0 // 09/09/03

#define RemDropCode                 0x00f2 // 09/09/03

#define MakeDropCode                0x00f3 // 09/09/03

#define OP_Save                     0x00f4 // 09/09/03 Client asking server to save user state

#define OP_ReqClientSpawn           0x00f6 // 09/09/03 Client requesting spawn data

#define OP_Action                   0x00fa // 09/09/03

#define NewCorpseCode               0x00fe // 09/09/03

#define OP_GMKill                   0x0102 // 09/09/03 GM /kill - Insta kill mob/pc

#define OP_GMKick                   0x0103 // GM /kick - Boot player

#define OP_GMGoto                   0x0104 // GM /goto - Transport to another loc

#define cLootCorpseCode             0x0112 // 09/09/03

#define cDoneLootingCode            0x0113 // 09/09/03

#define MoneyOnCorpseCode           0x0114 // 09/09/03

#define DoorClickCode               0x0120 // 09/09/03

#define DoorOpenCode                0x0121 // 09/09/03

#define IllusionCode                0x0124 // 09/09/03

#define SpawnUpdateCode             0x0125 // 09/09/03

#define ForageCode                  0x0127 // 09/09/03

#define OP_Adventure                0x02d7 // 09/09/03 /adventure

#define OpenGMCode                  0x012f // 09/09/03

#define OP_GMEndTraining            0x0130 // 09/09/03

#define OP_LootItem                 0x0133 // 09/09/03

#define AttackAnimationCode         0x0134 // 09/09/03

#define ZoneChangeCode              0x0136 // 09/09/03

#define SpawnAppearanceCode         0x013a // 09/09/03

#define OP_GuildRemove              0x013d // 09/09/03 /guildremove

#define OP_PlaceItem                0x014a // 09/09/03 Client moving an item from one slot to another

#define OP_MoveCash                 0x014b // 09/09/03

#define BuffDropCode                0x0150 // 09/09/03

#define WeatherCode                 0x0154 // 09/09/03

#define ConsiderCode                0x0155 // 09/09/03

#define OP_Feedback                 0x015a // 09/09/03 /feedback

#define ConsumeCode                 0x0160 // 09/09/03

#define StaminaCode                 0x0161 // 09/09/03

#define ZoneSpawnsCode              0x0169 // 09/09/03

#define LogoutCode                  0x0163 // 09/09/03

#define OP_AutoAttack               0x016b // 09/09/03

#define ClientTargetCode            0x016c // 09/09/03 - Targeting a person

#define SkillTrainCode              0x016e // 09/09/03

#define CloseGMCode                 0x0171 // 09/09/03

#define sDoneLootingCode            0x0172 // 09/09/03

#define OP_GMZoneRequest            0x017c // 09/09/03 /zone

#define OP_AutoAttack2              0x017e // 09/09/03

#define OP_Surname                  0x0180 // 09/09/03

#define TradeSpellBookSlotsCode     0x018f // 09/09/03

#define OP_YellForHelp              0x0192 // 09/09/03 

#define RandomReqCode               0x0197 // 09/09/03

#define OP_SetDataRate              0x0198 // 09/09/03 - Client sending datarate.txt value

#define OP_GMDelCorpse              0x0199 // 09/09/03 /delcorpse

#define OP_PetCommands              0x01aa // 09/09/03

#define OP_GMApproval               0x01ae // 09/09/03 GM /approval - Name approval duty?

#define LFGCode                     0x01af // 09/09/03

#define OP_GMToggle                 0x01b1 // 09/09/03 GM /toggle - Toggle ability to receive tells from other PC's

#define OP_GMEmoteZone              0x01b5 // 09/09/03 GM /emotezone - Send zonewide emote

#define DoorSpawnsCode              0x01b8 // 09/09/03

#define OP_TargetCommand            0x01be // 09/09/03 Target user

#define OP_ReqZoneObjects           0x01bf // 09/09/03 Client requesting zone objects

#define OP_GuildMOTD                0x01c3 // 09/09/03 - GuildMOTD

#define cStartCampingCode           0x01c7 // 09/09/03

#define OP_FaceChange               0x01cf // /face

#define cConCorpseCode              0x01db // 09/09/03

#define SimpleMessageCode           0x01dc // 09/09/03

#define FormattedMessageCode        0x01dd // 09/09/03

#define OP_Report                   0x01e3 // 09/09/03

#define BazaarSearchCode            0x01ec // 09/09/03

#define OpenTraderCode              0x01f0 // 09/09/03

#define OP_PViewPetition            0x01f1 // 09/09/03

#define OP_PDeletePetition          0x01f2 // 09/09/03

#define ItemInfoCode                0x01fa // 09/09/03

#define NewSpawnCode                0x0201 // 09/09/03

#define OP_SpecialMesg              0x0205 // 09/09/03 Communicate textual info to client

#define sWhoAllOutputCode           0x0212 // 09/09/03

#define OP_GMZoneRequest2           0x0222 // 09/09/03 /zone 2

#define ZoneEntryCode               0x0224 // 09/09/03

#define OP_SendZonePoints           0x0230 // 09/09/03 Coords in a zone that will port you to another zone

#define InspectRequestCode          0x0231 // 09/09/03

#define InspectDataCode             0x0232 // 09/09/03

#define GuildMemberUpdateCode       0x0257 // 09/09/03

#define LFGReqCode                  0x0259 // 09/09/03

#define LFGReqGetMatchesCode        0x025a // 09/09/03

#define LFPReqCode                  0x025b // 09/09/03

#define LFPGetMatchesReqCode        0x025c // 09/09/03

#define LFGGetMatchesCode           0x025d // 09/09/03

#define LFPGetMatchesCode           0x025e // 09/09/03

#define ItemCode                    0x02c5 // 09/09/03

#define NpcHpUpdateCode             0x022d // 09/09/03

#define OP_Bug                      0x022f // 09/09/03 /bug

#define OP_GMSummon                 0x0279 // 09/09/03 GM /summon - Summon PC to self

#define BookTextCode                0x027e // 09/09/03

#define OP_TradeRequest             0x0281 // 09/09/03 Client request trade session

#define OP_OpenObject               0x0286 // 09/09/03

#define OP_SummonCorpse             0x029c // 09/09/03 /summoncorpse

/*********************/
/* Out Of Date          */

#define ManaDecrementCode           0xf175

#define AltExpUpdateCode            0xf209

#define cItemInShopCode             0x007f

#define GroupInfoCode               0x0263

#define SysMsgCode                  0x1420

#define PlayerAACode                0x1522

#define MoneyThingCode              0x2820

#define CharInfo                    0x2e40

#define sSpellFizzleRegainCode      0x3542

#define MoneyUpdateCode             0x3d41

#define GroupAcceptCode             0x3d20

#define GroupInviteCode             0x3e20

#define GroupDeclineCode            0x4140

#define sSpellInterruptedCode       0x4142

#define GroupDeleteCode             0x4420

#define CastOnCode                  0x4640

#define CharUpdateCode              0x5521

#define cOpenSpellBookCode          0x5821

#define NewGuildInZoneCode          0x7b21

#define SPMesgCode                  0x8041

#define BindWoundCode               0x8341

#define cTrackCode                  0x8441

#define cSneakCode                  0x8521

#define cHideCode                   0x8641

#define WearChangeCode              0x9240

#define Attack2Code                 0xa140

#define BadCastCode                 0xd321

#define cChatFiltersCode            0xff41

// Obsoleted Item Codes, new code is 0x02c5 with entirely different format

#endif // OPCODES_H
