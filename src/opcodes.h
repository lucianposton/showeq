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

#define OP_AckPacket                0x0018 // 02/10/04  Appears to be generic ack at the presentation level

#define OP_GuildList                0x005e // 02/10/04  - old GuildListCode

#define OP_ExpansionInfo            0x00e3 // 02/10/04  Which expansions user has

#define OP_DeleteCharacter          0x00ec // 02/10/04 ?Delete character @ char select

#define OP_SendCharInfo             0x0104 // 02/10/04  Send all chars visible @ char select

#define OP_CharacterCreate          0x0106 // 02/10/04  Create character @ char select

#define OP_ApproveName              0x0127 // 02/10/04  Approving new character name @ char creation

#define OP_World_Client_CRC1	    0x015c // 02/10/04  Contains a snippet of spell data

#define OP_World_Client_CRC2        0x0160 // 02/10/04  Second client verification packet

#define OP_LogServer                0x0188 // 02/10/04  

#define OP_ApproveWorld             0x0196 // 02/10/04 

#define OP_MOTD                     0x01b3 // 02/10/04  - old MOTDCode Server message of the day

#define OP_SendLoginInfo            0x0241 // 02/10/04 

#define OP_EnterWorld               0x0264 // 02/10/04  Server approval for client to enter world

#define OP_ZoneServerInfo           0x0267 // 02/10/04  - old ZoneServerInfo

#define OP_SetChatServer            0x026c // 02/10/04  Chatserver? IP,Port,servername.Charname,password(?)

#define OP_GetName                  0x02ae // 02/10/04


/***********************/
/* Zone Server OpCodes */

#define OP_Consent                  0x0014 // 02/10/04  /consent

#define OP_ItemLinkClick            0x0020 // 02/10/04 

#define OP_GMServers                0x0021 // 02/10/04  GM /servers  - ?

#define OP_BeginCast                0x0022 // 02/10/04  - old BeginCastCode

#define OP_CommonMessage            0x0025 // 02/10/04  - old ChannelMessageCode

#define OP_TimeOfDay                0x0027 // 02/10/04  - old TimeOfDayCode

#define OP_ClientUpdate             0x0028 // 02/10/04  - old PlayerPosCode

#define OP_TradeAcceptClick         0x002e // 02/10/04 

#define OP_CancelTrade              0x002f // 02/10/04  

#define OP_TradeRequestAck          0x0038 // 02/10/04  Trade request recipient is acknowledging they are able to trade

#define OP_MobUpdate                0x003f // 02/10/04  - old MobUpdateCode

#define OP_TradeSkillCombine        0x0043 // 02/10/04 
 
#define OP_GMFind                   0x0048 // 02/10/04  GM /find	- ?

#define OP_WhoAllRequest            0x0057 // 02/10/04  - old WhoAllReqCode

#define OP_GuildMemberList          0x005a // 02/10/04 

#define OP_SkillUpdate              0x0065 // 02/10/04  - old SkillIncCode

#define OP_ShopPlayerBuy            0x0066 // 02/10/04  - old BuyItemCode

#define OP_Petition                 0x0065 // 11/05/03 

#define OP_PetitionUpdate           0x006a // 02/10/04? ? Updates the Petitions in the Que

#define OP_ShopPlayerSell           0x006b // 02/10/04  - old SellItemCode

#define OP_PlayerProfile            0x006c // 02/10/04  - old CharProfileCode

#define OP_TraderItemUpdate         0x006f // 02/10/04? 

#define OP_ShopEnd                  0x006d // 02/10/04  - old CloseVendorCode

#define OP_PetitionCheckIn          0x907b // 11/05/03  Petition Checkin

#define OP_GMBecomeNPC              0x0075 // 02/10/04  GM /becomenpc - Become an NPC

#define OP_PetitionCheckout         0x0073 // 11/05/03 ?Petition Checkout

#define OP_LevelUpdate              0x0079 // 02/10/04  - old LevelUpUpdateCode

#define OP_ExpUpdate                0x007a // 02/10/04  - old ExpUpdateCode

#define OP_PetitionRefresh          0x0082 //  

#define OP_RandomReply              0x0088 // 02/10/04  - old RandomCode

#define OP_ClientReady              0x0087 // 02/10/04 

#define OP_SetRunMode               0x008d // 02/10/04  - old cRunToggleCode

#define OP_PetitionDelete           0x008e // 11/05/03 ?Client Petition Delete Request

#define OP_GMSearchCorpse           0x0094 // 11/05/03  GM /searchcorpse - Search all zones for named corpse

#define OP_GuildPeace               0x0098 // 11/05/03  /guildpeace

#define OP_Sneak                    0x009e // 02/10/04  Clicked sneak

#define OP_Hide                     0x009f // 02/10/04 

#define OP_SaveOnZoneReq            0x009e // 11/05/03 

#define OP_GMLastName               0x00a4 // 02/10/04 ? GM /lastname - Change user lastname

#define OP_GuildLeader              0x00a5 // 02/10/04  /guildleader

#define OP_BoardBoat                0x00bc // 02/10/04 ?

#define OP_LeaveBoat                0x00bd // 02/10/04 ?

#define OP_CastSpell                0x00bf // 02/10/04  - old StartCastCode

#define OP_BuffFadeMsg              0x00c1 // 02/10/04  - old SpellFadeCode

#define OP_MemorizeSpell            0x00c3 // 02/10/04  - old MemSpellCode

#define OP_SenseHeading             0x00c2 // 02/10/04  - old cSenseHeadingCode

#define OP_SenseTraps               0x0187 // 11/05/03  Clicked sense traps

#define OP_Jump                     0x00d9 // 02/10/04  - old cJumpCode?

#define OP_CorpseLocResponse        0x00d1 // 11/05/03  - old CorpseLocCode:

#define OP_GMInquire                0x00db // 02/10/04  GM /inquire - Search soulmark data

#define OP_GMSoulmark               0x00dd // 02/10/04  GM /praise /warn - Add soulmark comment to user file

#define OP_GMHideMe                 0x00df // 02/10/04  GM /hideme - Remove self from spawn lists and make invis

#define OP_CastBuff                 0x00db //  - old ActionCode

#define OP_NewZone                  0x00ed // 02/10/04  - old NewZoneCode

#define OP_ReqNewZone               0x00ee // 02/10/04  Client requesting NewZone_Struct

#define OP_EnvDamage                0x00ea // 02/10/04 

#define OP_SafePoint                0x00f1 // 02/10/04 

#define OP_Emote                    0x00f4 // 02/10/04  - old EmoteTextCode

#define OP_DeleteSpawn              0x00f5 // 02/10/04  - old DeleteSpawnCode

#define OP_ShopRequest              0x00f9 // 02/10/04  - old OpenVendorCode

#define OP_ClickObject              0x00fb // 02/10/04  - old RemDropCode

#define OP_GroundSpawn              0x00fc // 02/10/04  - old MakeDropCode

#define OP_Save                     0x00fd // 02/10/04  Client asking server to save user state

#define OP_ReqClientSpawn           0x00ff // 02/10/04  Client requesting spawn data

#define OP_Action                   0x0103 // 02/10/04 

#define OP_Death                    0x0107 // 02/10/04  - old NewCorpseCode

#define OP_GMKill                   0x010b // 02/10/04  GM /kill - Insta kill mob/pc

#define OP_GMKick                   0x010c // 02/10/04 ? GM /kick - Boot player

#define OP_GMGoto                   0x010d // 02/10/04 ? GM /goto - Transport to another loc

#define OP_LootRequest              0x011b // 02/10/04  - old cLootCorpseCode

#define OP_EndLootRequest           0x011c // 02/10/04  - old cDoneLootingCode

#define OP_MoneyOnCorpse            0x011d // 02/10/04  - old MoneyOnCorpseCode

#define OP_Door                     0x012a // 02/10/04  - old DoorClickCode

#define OP_MoveDoor                 0x0129 // 02/10/04  - old DoorOpenCode

#define OP_Illusion                 0x0124 // 11/05/03  - old IllusionCode

#define OP_WearChange               0x012e // 02/10/04  - old SpawnUpdateCode

#define OP_Forage                   0x0130 // 02/10/04  - old ForageCode

#define OP_Adventure                0x02d7 // 11/05/03  /adventure

#define OP_GMTraining               0x013d // 11/05/03  - old OpenGMCode

#define OP_GMEndTraining            0x013e // 02/10/04 

#define OP_Animation                0x0142 // 02/10/04 

#define OP_ZoneChange               0x0144 // 02/10/04  - old ZoneChangeCode

#define OP_SpawnAppearance          0x0131 // 02/10/04  - old SpawnAppearanceCode

#define OP_LootItem                 0x0141 // 02/10/04 

#define OP_GuildRemove              0x0197 // 02/10/04  /guildremove

#define OP_MoveItem                 0x0153 // 02/10/04  Client moving an item from one slot to another

#define OP_MoveCash                 0x0154 // 02/10/04 

#define OP_Buff                     0x0159 // 02/10/04  - old BuffDropCode

#define OP_Weather                  0x015d // 02/10/04  - old WeatherCode

#define OP_Consider                 0x015e // 02/10/04  - old ConsiderCode

#define OP_Feedback                 0x015a // 11/05/03? ?/feedback

#define OP_Consume                  0x016a // 02/10/04  - old ConsumeCode

#define OP_Stamina                  0x0161 // 11/05/03  - old StaminaCode

#define OP_Logout                   0x016c // 02/10/04 

#define OP_ZoneSpawns               0x0172 // 02/10/04  - old ZoneSpawnsCode

#define OP_AutoAttack               0x0174 // 02/10/04 

#define OP_TargetMouse              0x0175 // 02/10/04  - Targeting a person - old ClientTargetCode

#define OP_GMTrainSkill             0x0177 // 02/10/04  - old SkillTrainCode

#define OP_ConfirmDelete            0x016f // 11/05/03? ?Client sends this to server to confirm op_deletespawn

#define OP_GMEndTrainingResponse    0x017a // 02/10/04  - old CloseGMCode

#define OP_LootComplete             0x017b // 02/10/04  - old sDoneLootingCode

#define OP_TraderDelItem            0x0174 // ?

#define OP_GMZoneRequest            0x0185 // 02/10/04  /zone

#define OP_AutoAttack2              0x0187 // 02/10/04 

#define OP_Surname                  0x0180 // 11/05/03 

#define OP_DisarmTraps              0x0186 // 11/05/03  Clicked disarm traps

#define OP_SwapSpell                0x0190 // 02/10/04  - old TradeSpellBookSlotsCode

#define OP_YellForHelp              0x0193 // 02/10/04

#define OP_RandomReq                0x0198 // 02/10/04  - old RandomReqCode

#define OP_SetDataRate              0x0199 // 02/10/04  - Client sending datarate.txt value

#define OP_GMDelCorpse              0x019a // 02/10/04  /delcorpse

#define OP_PetCommands              0x01aa // 02/10/04 

#define OP_GMApproval               0x01b1 // 02/10/04  GM /approval - Name approval duty?

#define OP_LFGResponse              0x01b2 // 02/10/04 

#define OP_ItemPlayerPacket         0x0281 // 02/10/04  - old ItemCode

#define OP_GMToggle                 0x01b4 // 02/10/04  GM /toggle - Toggle ability to receive tells from other PC's

#define OP_GMEmoteZone              0x027f // 02/10/04  GM /emotezone - Send zonewide emote

#define OP_SpawnDoor                0x0282 // 02/10/04  - old DoorSpawnsCode

#define OP_MoneyUpdate              0x01be // 11/05/03 

#define OP_TargetCommand            0x01c0 // 11/05/03  Target user

#define OP_ReqZoneObjects           0x01bc // 02/10/04  Client requesting zone objects

#define OP_GuildMOTD                0x01c1 // 02/10/04  - GuildMOTD

#define OP_Camp                     0x01c4 // 02/10/04  - old cStartCampingCode

#define OP_BecomeTrader             0x01c5 // 02/10/04  

#define OP_TraderBuy                0x01cb // 02/10/04  buy from a trader in bazaar

#define OP_FaceChange               0x01cc // 02/10/04  /face

#define OP_ConsiderCorpse           0x01d8 // 02/10/04  - old cConCorpseCode

#define OP_SimpleMessage            0x01d9 // 02/10/04  - old SimpleMessageCode

#define OP_FormattedMessage         0x01da // 02/10/04  - old FormattedMessageCode

#define OP_Report                   0x01e3 // 09/09/03 

#define OP_BazaarSearch		    0x01e9 // 02/10/04 perlmonkey - Bazaar search and reply data

#define OP_Trader                   0x01ea // 02/10/04  /trader

#define OP_AAAction                 0x01eb // 02/10/04 ? Used for changing percent, buying? and activating skills

#define OP_TraderShop               0x01ed // 02/10/04  - old OpenTraderCode right-click on a trader in bazaar

#define OP_ViewPetition             0x01f1 // 11/05/03  Player /viewpetition

#define OP_DeletePetition           0x01f2 // 11/05/03  Player /deletepetition

#define OP_ItemLinkResponse         0x01f7 // 02/10/04  - old ItemInfoCode

#define OP_NewSpawn                 0x021b // 02/10/04  - old NewSpawnCode

#define OP_SpecialMesg              0x021f // 02/10/04 ?Communicate textual info to client

#define OP_WhoAllResponse           0x022c // 02/10/04  - old sWhoAllOutputCode

#define OP_UpdateAA                 0x0225 // 02/10/04 ?

#define OP_RespondAA                0x01ec // 02/10/04 ?

#define OP_MobHealth                0x0231 // 02/10/04  health sent when a player clicks on the mob

#define OP_GMZoneRequest2           0x0222 // 11/05/03? ?/zone 2

#define OP_ZoneEntry                0x023e // 02/10/04  - old ZoneEntryCode

#define OP_HPUpdate                 0x0247 // 02/10/04  - old NpcHpUpdateCode Update HP % of a PC or NPC

#define OP_SendZonePoints           0x024a // 02/10/04  Coords in a zone that will port you to another zone

#define OP_InspectRequest           0x024b // 02/10/04  - old InspectRequestCode

#define OP_InspectAnswer            0x024c // 02/10/04  - old InspectDataCode

#define OP_GuildMemberUpdate        0x0271 // 02/10/04 

#define OP_LFGCommand               0x0273 // 02/10/04  - old LFGReqCode

#define OP_LFGGetMatchesRequest     0x0274 // 02/10/04 

#define OP_LFPCommand               0x0275 // 02/10/04  looking for players

#define OP_LFPGetMatchesRequest     0x0276 // 02/10/04 

#define OP_LFGGetMatchesResponse    0x0277 // 02/10/04 

#define OP_LFPGetMatchesResponse    0x0278 // 02/10/04 

#define OP_Bug                      0x0249 // 09/09/03? ? /bug

#define OP_Track                    0x0289 // 02/10/04  Clicked Track

#define OP_GMSummon                 0x027c // 02/10/04  GM /summon - Summon PC to self

#define OP_ItemTextFile             0x029a // 02/10/04  - old BookTextCode

#define OP_TradeRequest             0x029d // 02/10/04  Client request trade session

#define OP_OpenObject               0x02a2 // 02/10/04 

#define OP_PetitionResolve          0x02b7 // 02/10/04 ?Client Petition Resolve Request

#define OP_SummonCorpse             0x02b8 // 02/10/04 ?/summoncorpse

#define OP_ItemPacket               0x02e3 // 02/10/04  - old ItemCode

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
