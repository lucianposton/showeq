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

#define OP_AckPacket                0x000d // 09/09/03 Appears to be generic ack at the presentation level

#define OP_GuildList                0x0058 // 10/09/03 - old GuildListCode

#define OP_ExpansionInfo            0x00d8 // 10/09/03 Which expansions user has

#define OP_DeleteCharacter          0x00e3 // 09/09/03 Delete character @ char select

#define OP_SendCharInfo             0x00f9 // 10/09/03 Send all chars visible @ char select

#define OP_CharacterCreate          0x00fb // 10/09/03 Create character @ char select

#define OP_ApproveName              0x011c // 10/09/03 Approving new character name @ char creation

#define OP_World_Client_CRC1	    0x0151 // 10/09/03 Contains a snippet of spell data

#define OP_World_Client_CRC2        0x0155 // 10/09/03 Second client verification packet

#define OP_LogServer                0x017d // 10/09/03 

#define OP_ApproveWorld             0x0193 // 10/09/03

#define OP_MOTD                     0x01ae // 10/09/03 - old MOTDCode

#define OP_SendLoginInfo            0x0238 // 10/09/03

#define OP_EnterWorld               0x0248 // 10/09/03 Server approval for client to enter world

#define OP_ZoneServerInfo           0x024b // 10/09/03 - old ZoneServerInfo

#define OP_SetChatServer            0x0250 // 10/09/03 Chatserver? IP,Port,servername.Charname,password(?)

/***********************/
/* Zone Server OpCodes */

#define OP_Consent                  0x0009 // 10/09/03 /consent

#define OP_ItemLinkClick            0x0015 // 10/09/03

#define OP_GMServers                0x0016 // 09/09/03 GM /servers  - ?

#define OP_BeginCast                0x0017 // 10/09/03 - old BeginCastCode

#define OP_CommonMessage            0x001f // 10/09/03 - old ChannelMessageCode

#define OP_TimeOfDay                0x0021 // 10/09/03 - old TimeOfDayCode

#define OP_ClientUpdate             0x0022 // 10/09/03 - old PlayerPosCode

#define OP_TradeAcceptClick         0x0028 // 10/09/03

#define OP_TradeCancelClick         0x0029 // 10/09/03 

#define OP_TradeRequestAck          0x0032 // 10/09/03 Trade request recipient is acknowledging they are able to trade

#define OP_MobUpdate                0x0039 // 10/09/03 - old MobUpdateCode

#define OP_TradeSkillCombine        0x003d // 10/09/03
 
#define OP_GMFind                   0x0044 // 09/09/03 GM /find	- ?

#define OP_WhoAllRequest            0x0051 // 10/09/03 - old WhoAllReqCode

#define OP_GuildMemberList          0x0054 // 10/09/03

#define OP_SkillUpdate              0x005f // 10/09/03 - old SkillIncCode

#define OP_ShopTakeMoney            0x0060 // 10/09/03 - old BuyItemCode

#define OP_Petition                 0x0062 // 10/09/03?

#define OP_PetitionUpdate           0x0064 // 10/09/03? Updates the Petitions in the Que

#define OP_ShopPlayerSell           0x0065 // 10/09/03 - old SellItemCode

#define OP_PlayerProfile            0x0066 // 10/09/03 - old CharProfileCode

#define OP_TraderItemUpdate         0x0069 // 09/09/03

#define OP_ShopEnd                  0x0067 // 10/09/03 - old CloseVendorCode

#define OP_GMBecomeNPC              0x0071 // 10/09/03? GM /becomenpc - Become an NPC

#define OP_PetitionCheckout         0x0073 // 09/09/03 Petition Checkout

#define OP_LevelUpdate              0x0072 // 10/09/03 - old LevelUpUpdateCode

#define OP_ExpUpdate                0x0075 // 10/09/03 - old ExpUpdateCode

#define OP_PetitionCheckIn          0x007a // 09/09/03 Petition Checkin

#define OP_RandomReply              0x0082 // 10/09/03 - old RandomCode

#define OP_ClientReady              0x0080 // 10/09/03

#define OP_SetRunMode               0x0087 // 10/09/03 - old cRunToggleCode

#define OP_PetitionDelete           0x008d // 09/09/03 Client Petition Delete Request

#define OP_GMSearchCorpse           0x0094 // 10/09/03? GM /searchcorpse - Search all zones for named corpse

#define OP_GuildPeace               0x0096 // 10/09/03 /guildpeace

#define OP_Hide                     0x009a // 09/09/03

#define OP_SaveOnZoneReq            0x009c // 10/09/03

#define OP_GMLastName               0x009f // 10/09/03? GM /lastname - Change user lastname

#define OP_GuildLeader              0x00a3 // 09/09/03 /guildleader

#define OP_BoardBoat                0x00b8 // 09/09/03

#define OP_LeaveBoat                0x00b9 // 09/09/03

#define OP_CastSpell                0x00b9 // 10/09/03 - old StartCastCode

#define OP_BuffFadeMsg              0x00bb // 10/09/03 - old SpellFadeCode

#define OP_MemorizeSpell            0x00bd // 10/09/03 - old MemSpellCode

#define OP_SenseHeading             0x00be // 10/09/03 - old cSenseHeadingCode

#define OP_Jump                     0x00ce // 10/09/03 - old cJumpCode?

#define OP_CorpseLocResponse        0x00d3 // 10/09/03 - old CorpseLocCode:

#define OP_GMInquire                0x00d8 // 09/09/03 GM /inquire - Search soulmark data

#define OP_GMSoulmark               0x00d4 // 09/09/03 GM /praise /warn - Add soulmark comment to user file

#define OP_GMHideMe                 0x00d6 // 09/09/03 GM /hideme - Remove self from spawn lists and make invis

#define OP_CastBuff                 0x00d9 // 10/09/03 - old ActionCode

#define OP_NewZone                  0x00e2 // 10/09/03 - old NewZoneCode

#define OP_ReqNewZone               0x00e3 // 10/09/03 Client requesting NewZone_Struct

#define OP_EnvDamage                0x00e1 // 09/09/03

#define OP_SafePoint                0x00e8 // 09/09/03

#define OP_Emote                    0x00e9 // 10/09/03 - old EmoteTextCode

#define OP_DeleteSpawn              0x00ea // 10/09/03 - old DeleteSpawnCode

#define OP_ShopRequest              0x00ee // 10/09/03 - old OpenVendorCode

#define OP_ClickObject              0x00f0 // 10/09/03 - old RemDropCode

#define OP_GroundSpawn              0x00f1 // 10/09/03 - old MakeDropCode

#define OP_Save                     0x00f2 // 10/09/03 Client asking server to save user state

#define OP_ReqClientSpawn           0x00f4 // 10/09/03 Client requesting spawn data

#define OP_Action                   0x00f8 // 10/09/03

#define OP_Death                    0x00fc // 10/09/03 - old NewCorpseCode

#define OP_GMKill                   0x0102 // 09/09/03 GM /kill - Insta kill mob/pc

#define OP_GMKick                   0x010c // 10/09/03? GM /kick - Boot player

#define OP_GMGoto                   0x010d // 10/09/03? GM /goto - Transport to another loc

#define OP_LootRequest              0x0110 // 10/09/03 - old cLootCorpseCode

#define OP_EndLootRequest           0x0111 // 10/09/03 - old cDoneLootingCode

#define OP_MoneyOnCorpse            0x0112 // 10/09/03 - old MoneyOnCorpseCode

#define OP_ClickDoor                0x011e // 10/09/03 - old DoorClickCode

#define OP_MoveDoor                 0x011f // 10/09/03 - old DoorOpenCode

#define OP_WearChange               0x0123 // 10/09/03 - old SpawnUpdateCode

#define OP_Illusion                 0x0126 // 10/09/03 - old IllusionCode

#define OP_Forage                   0x0127 // 09/09/03 - old ForageCode

#define OP_Adventure                0x02d7 // 09/09/03 /adventure

#define OP_GMTraining               0x012d // 10/09/03 - old OpenGMCode

#define OP_GMEndTraining            0x012e // 10/09/03

#define OP_Animation                0x0132 // 10/09/03

#define OP_ZoneChange               0x0134 // 10/09/03 - old ZoneChangeCode

#define OP_SpawnAppearance          0x0138 // 10/09/03 - old SpawnAppearanceCode

#define OP_LootItem                 0x0139 // 10/09/03

#define OP_GuildRemove              0x013d // 09/09/03 /guildremove

#define OP_MoveItem                 0x0148 // 10/09/03 Client moving an item from one slot to another

#define OP_MoveCash                 0x0149 // 10/09/03

#define OP_Buff                     0x014e // 10/09/03 - old BuffDropCode

#define OP_Weather                  0x0152 // 10/09/03 - old WeatherCode

#define OP_Consider                 0x0153 // 10/09/03 - old ConsiderCode

#define OP_Feedback                 0x015a // 09/09/03 /feedback

#define OP_Consume                  0x015e // 10/09/03 - old ConsumeCode

#define OP_Stamina                  0x015f // 10/09/03 - old StaminaCode

#define OP_Logout                   0x0161 // 10/09/03

#define OP_ZoneSpawns               0x0167 // 10/09/03 - old ZoneSpawnsCode

#define OP_AutoAttack               0x0169 // 10/09/03

#define OP_TargetMouse              0x016a // 10/09/03 - Targeting a person - old ClientTargetCode

#define OP_GMTrainSkill             0x016c // 10/09/03 - old SkillTrainCode

#define OP_ConfirmDelete            0x016f // 09/09/03 Client sends this to server to confirm op_deletespawn

#define OP_GMEndTrainingResponse    0x016f // 10/09/03 - old CloseGMCode

#define OP_LootComplete             0x0170 // 09/09/03 - old sDoneLootingCode

#define OP_TraderDelItem            0x0172 // 10/09/03?

#define OP_GMZoneRequest            0x017a // 10/09/03 /zone

#define OP_AutoAttack2              0x017c // 10/09/03

#define OP_Surname                  0x0185 // 09/09/03

#define OP_SwapSpell                0x018d // 10/09/03 - old TradeSpellBookSlotsCode

#define OP_RandomReq                0x0195 // 10/09/03 - old RandomReqCode

#define OP_SetDataRate              0x0196 // 10/09/03 - Client sending datarate.txt value

#define OP_GMDelCorpse              0x0199 // 09/09/03 /delcorpse

#define OP_YellForHelp              0x019a // 10/09/03 

#define OP_PetCommands              0x01aa // 09/09/03

#define OP_GMApproval               0x01ae // 09/09/03 GM /approval - Name approval duty?

#define OP_LFGResponse              0x01ad // 10/09/03

#define OP_ItemPlayerPacket         0x01b5 // 10/09/03 - old ItemCode

#define OP_GMToggle                 0x01b1 // 09/09/03 GM /toggle - Toggle ability to receive tells from other PC's

#define OP_GMEmoteZone              0x01b5 // 09/09/03 GM /emotezone - Send zonewide emote

#define OP_SpawnDoor                0x01b6 // 10/09/03 - old DoorSpawnsCode

#define OP_MoneyUpdate              0x01bc // 10/09/03

#define OP_TargetCommand            0x01be // 10/09/03 Target user

#define OP_ReqZoneObjects           0x01bf // 09/09/03 Client requesting zone objects

#define OP_GuildMOTD                0x01c1 // 10/09/03 - GuildMOTD

#define OP_Camp                     0x01c5 // 10/09/03 - old cStartCampingCode

#define OP_BecomeTrader             0x01ca // 10/09/03 

#define OP_TraderBuy                0x01ce // 09/09/03 buy from a trader in bazaar

#define OP_FaceChange               0x01cd // 10/09/03 /face

#define OP_ConsiderCorpse           0x01d9 // 10/09/03 - old cConCorpseCode

#define OP_SimpleMessage            0x01da // 10/09/03 - old SimpleMessageCode

#define OP_FormattedMessage         0x01db // 10/09/03 - old FormattedMessageCode

#define OP_Report                   0x01e3 // 09/09/03

#define OP_Bazaar                   0x01ec // 10/09/03 /bazaar - search - old BazaarSearchCode 

#define OP_Trader                   0x01ed // 10/09/03 /trader

#define OP_AAAction                 0x01ee // 09/09/03 Used for changing percent, buying? and activating skills

#define OP_TraderShop               0x01f0 // 09/09/03 - old OpenTraderCode

#define OP_ViewPetition             0x01f1 // 09/09/03 Player /viewpetition

#define OP_DeletePetition           0x01f2 // 09/09/03 Player /deletepetition

#define OP_ItemLinkResponse         0x01f8 // 10/09/03 - old ItemInfoCode

#define OP_NewSpawn                 0x01ff // 10/09/03 - old NewSpawnCode

#define OP_SpecialMesg              0x0205 // 09/09/03 Communicate textual info to client

#define OP_WhoAllResponse           0x0210 // 10/09/03 - old sWhoAllOutputCode

#define OP_MobHealth                0x0215 // 10/09/03 health sent when a player clicks on the mob

#define OP_GMZoneRequest2           0x0222 // 09/09/03 /zone 2

#define OP_ZoneEntry                0x0222 // 10/09/03 - old ZoneEntryCode

#define OP_SendZonePoints           0x022e // 10/09/03 Coords in a zone that will port you to another zone

#define OP_InspectRequest           0x022f // 10/09/03 - old InspectRequestCode

#define OP_InspectAnswer            0x0230 // 10/09/03 - old InspectDataCode

#define OP_GuildMemberUpdate        0x0255 // 10/09/03

#define OP_LFGCommand               0x0257 // 10/09/03 - old LFGReqCode

#define OP_LFGGetMatchesRequest     0x0258 // 10/09/03

#define OP_LFPCommand               0x0259 // 10/09/03 looking for players

#define OP_LFPGetMatchesRequest     0x025a // 10/09/03

#define OP_LFGGetMatchesResponse    0x025b // 10/09/03

#define OP_LFPGetMatchesResponse    0x025c // 10/09/03

#define OP_HPUpdate                 0x022b // 10/09/03 - old NpcHpUpdateCode

#define OP_Bug                      0x022f // 09/09/03 /bug

#define OP_GMSummon                 0x0279 // 10/09/03 GM /summon - Summon PC to self

#define OP_ItemTextFile             0x027d // 10/09/03 - old BookTextCode

#define OP_TradeRequest             0x027f // 10/09/03 Client request trade session

#define OP_OpenObject               0x0284 // 10/09/03

#define OP_PetitionResolve          0x029b // 09/09/03 Client Petition Resolve Request

#define OP_SummonCorpse             0x029c // 09/09/03 /summoncorpse

#define OP_ItemPacket               0x02c3 // 10/09/03 - old ItemCode

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

#endif // OPCODES_H
