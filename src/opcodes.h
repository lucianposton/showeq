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

#define OP_AckPacket                0x0017 // 02/10/04  Appears to be generic ack at the presentation level

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

#define OP_Consent                  0x0013 // 02/10/04  /consent

#define OP_ItemLinkClick            0x001f // 02/10/04 

#define OP_GMServers                0x0020 // 02/10/04  GM /servers  - ?

#define OP_BeginCast                0x0021 // 02/10/04  - old BeginCastCode

#define OP_CommonMessage            0x0024 // 02/10/04  - old ChannelMessageCode

#define OP_TimeOfDay                0x0026 // 02/10/04  - old TimeOfDayCode

#define OP_ClientUpdate             0x0027 // 02/10/04  - old PlayerPosCode

#define OP_SendAAStats1		    0x002b // 05/26/04 - dont know which is right for AAStats

#define OP_TradeAcceptClick         0x002d // 02/10/04 

#define OP_CancelTrade              0x002e // 02/10/04  

#define OP_FinishTrade		    0x002f // 05/26/04

#define OP_ItemToTrade		    0x0030 // 05/26/04

#define OP_TradeCoins		    0x0036 // 05/26/04

#define OP_TradeRequestAck          0x0037 // 02/10/04  Trade request recipient is acknowledging they are able to trade

#define OP_MobUpdate                0x003e // 02/10/04  - old MobUpdateCode

#define OP_TradeSkillCombine        0x0042 // 02/10/04 
 
#define OP_GMFind                   0x0047 // 02/10/04  GM /find	- ?

#define OP_WhoAllRequest            0x0056 // 02/10/04  - old WhoAllReqCode

#define OP_GuildMemberList          0x0059 // 02/10/04 

#define OP_SkillUpdate              0x0064 // 02/10/04  - old SkillIncCode

#define OP_ShopPlayerBuy            0x0065 // 02/10/04  - old BuyItemCode

#define OP_Petition                 0x0068 // 11/05/03 

#define OP_PetitionUpdate           0x0069 // 02/10/04? ? Updates the Petitions in the Que

#define OP_ShopPlayerSell           0x006a // 02/10/04  - old SellItemCode

#define OP_PlayerProfile            0x006b // 02/10/04  - old CharProfileCode

#define OP_ShopEnd                  0x006c // 02/10/04  - old CloseVendorCode

#define OP_TraderItemUpdate         0x006e // 02/10/04? 

#define OP_GMBecomeNPC              0x0074 // 02/10/04  GM /becomenpc - Become an NPC

#define OP_PetitionCheckout         0x0076 // 11/05/03 ?Petition Checkout

#define OP_LevelUpdate              0x0078 // 02/10/04  - old LevelUpUpdateCode

#define OP_ExpUpdate                0x0079 // 02/10/04  - old ExpUpdateCode

#define OP_Split		    0x007a // 05/26/04

#define OP_Mend			    0x007d // 05/26/04

#define OP_PetitionCheckIn          0x007e // 11/05/03  Petition Checkin

#define OP_PetitionQue 		    0x0082 //  

#define OP_RandomReply              0x0087 // 02/10/04  - old RandomCode

#define OP_ClientReady              0x0086 // 02/10/04 

#define OP_SetRunMode               0x008c // 02/10/04  - old cRunToggleCode

#define OP_PetitionUnCheckout	    0x0090 // 05/26/04

#define OP_PetitionDelete           0x0091 // 11/05/03 ?Client Petition Delete Request

#define OP_GMSearchCorpse           0x0097 // 11/05/03  GM /searchcorpse - Search all zones for named corpse

#define OP_GuildPeace               0x009a // 11/05/03  /guildpeace

#define OP_MendHPUpdate             0x009b // 05/26/04

#define OP_Sneak                    0x009d // 02/10/04  Clicked sneak

#define OP_Hide                     0x009e // 02/10/04 

#define OP_SaveOnZoneReq            0x00a1 // 11/05/03 

#define OP_GMLastName               0x00a3 // 02/10/04 ? GM /lastname - Change user lastname

#define OP_GuildWar		    0x00a4 // 05/26/04

#define OP_GuildLeader              0x00a7 // 02/10/04  /guildleader

#define OP_ApplyPoison		    0x00b6 // 05/26/04

#define OP_BoardBoat                0x00bb // 02/10/04 ?

#define OP_LeaveBoat                0x00bc // 02/10/04 ?

#define OP_CastSpell                0x00be // 02/10/04  - old StartCastCode

#define OP_ManaChange		    0x00bf // 05/26/04

#define OP_BuffFadeMsg              0x00c0 // 02/10/04  - old SpellFadeCode

#define OP_MemorizeSpell            0x00c2 // 02/10/04  - old MemSpellCode

#define OP_SenseHeading             0x00c1 // 02/10/04  - old cSenseHeadingCode

#define OP_GroupInvite2		    0x00d5 // 05/26/04

#define OP_CancelInvite		    0x00d7 // 05/26/04

#define OP_Jump                     0x00d8 // 02/10/04  - old cJumpCode?

#define OP_CorpseLocResponse        0x00d0 // 11/05/03  - old CorpseLocCode:

#define OP_GMInquire                0x00da // 02/10/04  GM /inquire - Search soulmark data

#define OP_GMSoulmark               0x00dc // 02/10/04  GM /praise /warn - Add soulmark comment to user file

#define OP_GMHideMe                 0x00de // 02/10/04  GM /hideme - Remove self from spawn lists and make invis

#define OP_Action2		    0x00e2 // 05/26/04

#define OP_CastBuff                 0x00db //  - old ActionCode

#define OP_NewZone                  0x00eb // 02/10/04  - old NewZoneCode

#define OP_ReqNewZone               0x00ec // 02/10/04  Client requesting NewZone_Struct

#define OP_EnvDamage                0x00e8 // 02/10/04 

#define OP_SafePoint                0x00ef // 02/10/04 

#define OP_Emote                    0x00f2 // 02/10/04  - old EmoteTextCode

#define OP_DeleteSpawn              0x00f3 // 02/10/04  - old DeleteSpawnCode

#define OP_ShopRequest              0x00f7 // 02/10/04  - old OpenVendorCode

#define OP_ClickObject              0x00f9 // 02/10/04  - old RemDropCode

#define OP_GroundSpawn              0x00fa // 02/10/04  - old MakeDropCode

#define OP_Save                     0x00fb // 02/10/04  Client asking server to save user state

#define OP_ReqClientSpawn           0x00fd // 02/10/04  Client requesting spawn data

#define OP_GroupDisband		    0x00ff // 05/26/04

#define OP_Unknown52604_2	    0x0100 // 05/26/04

#define OP_Action                   0x0101 // 02/10/04 

#define OP_Death                    0x0105 // 02/10/04  - old NewCorpseCode

#define OP_GMKill                   0x0109 // 02/10/04  GM /kill - Insta kill mob/pc

#define OP_GMKick                   0x010a // 02/10/04 ? GM /kick - Boot player

#define OP_GMGoto                   0x010b // 02/10/04 ? GM /goto - Transport to another loc

#define OP_LootRequest              0x0119 // 02/10/04  - old cLootCorpseCode

#define OP_EndLootRequest           0x011a // 02/10/04  - old cDoneLootingCode

#define OP_MoneyOnCorpse            0x011b // 02/10/04  - old MoneyOnCorpseCode

#define OP_Door                     0x012a // 02/10/04  - old DoorClickCode

#define OP_ClickDoor		    0x0127 // 05/26/04 

#define OP_MoveDoor                 0x0128 // 02/10/04  - old DoorOpenCode

#define OP_Illusion                 0x012b // 11/05/03  - old IllusionCode

#define OP_WearChange               0x012c // 02/10/04  - old SpawnUpdateCode

#define OP_BindWound		    0x012d // 05/26/04

#define OP_Forage                   0x012e // 02/10/04  - old ForageCode

#define OP_Adventure                0x02ef // 11/05/03  /adventure

#define OP_GMTraining               0x013b // 11/05/03  - old OpenGMCode

#define OP_GMEndTraining            0x013c // 02/10/04 

#define OP_Animation                0x0140 // 02/10/04 

#define OP_ZoneChange               0x0142 // 02/10/04  - old ZoneChangeCode

#define OP_Begging		    0x014c // 05/26/04

#define OP_SpawnAppearance          0x012f // 02/10/04  - old SpawnAppearanceCode

#define OP_LootItem                 0x013f // 02/10/04 

#define OP_GuildRemove              0x0132 // 02/10/04  /guildremove

#define OP_MoveItem                 0x0151 // 02/10/04  Client moving an item from one slot to another

#define OP_MoveCash                 0x0152 // 02/10/04 

#define OP_Buff                     0x0157 // 02/10/04  - old BuffDropCode

#define OP_Weather                  0x015b // 02/10/04  - old WeatherCode

#define OP_Consider                 0x015c // 02/10/04  - old ConsiderCode

#define OP_Feedback                 0x0161 // 11/05/03? ?/feedback

#define OP_TradeMoneyUpdate	    0x0162 // 05/26/04 

#define OP_Consume                  0x0168 // 02/10/04  - old ConsumeCode

#define OP_Stamina                  0x015f // 11/05/03  - old StaminaCode

#define OP_Taunt		    0x0160 // 05/26/04

#define OP_Logout                   0x016a // 02/10/04 

#define OP_Stun			    0x016c // 05/26/04

#define OP_ZoneSpawns               0x0170 // 02/10/04  - old ZoneSpawnsCode

#define OP_CombatAbility	    0x0171 // 05/26/04

#define OP_AutoAttack               0x0172 // 02/10/04 

#define OP_TargetMouse              0x0173 // 02/10/04  - Targeting a person - old ClientTargetCode

#define OP_GMTrainSkill             0x0175 // 02/10/04  - old SkillTrainCode

#define OP_ConfirmDelete            0x0176 // 11/05/03? ?Client sends this to server to confirm op_deletespawn

#define OP_GMEndTrainingResponse    0x0178 // 02/10/04  - old CloseGMCode

#define OP_LootComplete             0x0179 // 02/10/04  - old sDoneLootingCode

#define OP_DelItem		    0x0172 // 05/26/04

#define OP_TraderDelItem            0x017b // ?

#define OP_GMZoneRequest            0x0183 // 02/10/04  /zone

#define OP_AutoAttack2              0x0186 // 02/10/04 

#define OP_Surname                  0x0187 // 11/05/03 

#define OP_DisarmTraps              0x0184 // 11/05/03  Clicked disarm traps

#define OP_SenseTraps               0x0187 // 11/05/03  Clicked sense traps

#define OP_SwapSpell                0x018e // 02/10/04  - old TradeSpellBookSlotsCode

#define OP_BecomePK		    0x018f // 05/26/04

#define OP_YellForHelp              0x0191 // 02/10/04

#define OP_RandomReq                0x0196 // 02/10/04  - old RandomReqCode

#define OP_SetDataRate              0x0197 // 02/10/04  - Client sending datarate.txt value

#define OP_GMDelCorpse              0x0198 // 02/10/04  /delcorpse

#define OP_Sacrifice		    0x0199

#define OP_PetCommands              0x01a8 // 02/10/04 

#define OP_GMApproval               0x01af // 02/10/04  GM /approval - Name approval duty?

#define OP_LFGResponse              0x01b0 // 02/10/04 

#define OP_ItemPlayerPacket         0x0290 // 02/10/04  - old ItemCode

#define OP_GMToggle                 0x01b2 // 02/10/04  GM /toggle - Toggle ability to receive tells from other PC's

#define OP_GMEmoteZone              0x028e // 02/10/04  GM /emotezone - Send zonewide emote

#define OP_SpawnDoor                0x0291 // 02/10/04  - old DoorSpawnsCode

#define OP_MoneyUpdate              0x01bc // 11/05/03 

#define OP_TargetCommand            0x01b9 // 11/05/03  Target user

#define OP_ReqZoneObjects           0x01ba // 02/10/04  Client requesting zone objects

#define OP_GuildMOTD                0x01bf // 02/10/04  - GuildMOTD

#define OP_Camp                     0x01c2 // 02/10/04  - old cStartCampingCode

#define OP_BecomeTrader             0x01c3 // 02/10/04  

#define OP_SendAAStats		    0x01c8 // 05/26/04

#define OP_TraderBuy                0x01c9 // 02/10/04  buy from a trader in bazaar

#define OP_FaceChange               0x01ca // 02/10/04  /face

#define OP_Dye			    0x01d5 // 05/26/04

#define OP_ConsiderCorpse           0x01d6 // 02/10/04  - old cConCorpseCode

#define OP_SimpleMessage            0x01d7 // 02/10/04  - old SimpleMessageCode

#define OP_FormattedMessage         0x01d8 // 02/10/04  - old FormattedMessageCode

#define OP_Report                   0x01e1 // 09/09/03 

#define OP_RaidInvite		    0x01e3 // 05/26/04

#define OP_RaidJoin		    0x01e4 // 05/26/04

#define OP_BazaarSearch		    0x01e7 // 02/10/04 perlmonkey - Bazaar search and reply data

#define OP_Trader                   0x01e8 // 02/10/04  /trader

#define OP_AAAction                 0x01e9 // 02/10/04 ? Used for changing percent, buying? and activating skills

#define OP_Bazaar		    0x01ec // 05/26/04

#define OP_TraderShop               0x01eb // 02/10/04  - old OpenTraderCode right-click on a trader in bazaar

#define OP_ViewPetition             0x01ef // 11/05/03  Player /viewpetition

#define OP_DeletePetition           0x01f0 // 11/05/03  Player /deletepetition

#define OP_ItemLinkResponse         0x01f5 // 02/10/04  - old ItemInfoCode

#define OP_NewSpawn                 0x0217 // 02/10/04  - old NewSpawnCode

#define OP_ExpansionSetting	    0x01ff // 05/26/04

#define OP_SpecialMesg              0x021b // 02/10/04 ?Communicate textual info to client

#define OP_GainMoney		    0x0205 // 05/26/04

#define OP_WhoAllResponse           0x0228 // 02/10/04  - old sWhoAllOutputCode

#define OP_UpdateAA                 0x0221 // 02/10/04 ?

#define OP_RespondAA                0x01ea // 02/10/04 ?

#define OP_MobHealth                0x022d // 02/10/04  health sent when a player clicks on the mob

#define OP_SendHPTarget		    0x022e // 05/26/04

#define OP_GMZoneRequest2           0x0225 // 11/05/03? ?/zone 2

#define OP_ZoneEntry                0x023a // 02/10/04  - old ZoneEntryCode

#define OP_HPUpdate                 0x0243 // 02/10/04  - old NpcHpUpdateCode Update HP % of a PC or NPC

#define OP_Unknown52604_1	    0x0244 // 05/26/04

#define OP_SendZonePoints           0x0246 // 02/10/04  Coords in a zone that will port you to another zone

#define OP_InspectRequest           0x0247 // 02/10/04  - old InspectRequestCode

#define OP_InspectAnswer            0x0248 // 02/10/04  - old InspectDataCode

#define OP_GroupUpdate		    0x0249 // 05/26/04

#define OP_GuildMemberUpdate        0x026d // 02/10/04 

#define OP_LFGCommand               0x026f // 02/10/04  - old LFGReqCode

#define OP_LFGGetMatchesRequest     0x0270 // 02/10/04 

#define OP_LFPCommand               0x0271 // 02/10/04  looking for players

#define OP_LFPGetMatchesRequest     0x0272 // 02/10/04 

#define OP_LFGGetMatchesResponse    0x0273 // 02/10/04 

#define OP_LFPGetMatchesResponse    0x0274 // 02/10/04 

#define OP_Bug                      0x0245 // 09/09/03? ? /bug

#define OP_Track                    0x0285 // 02/10/04  Clicked Track

#define OP_GMSummon                 0x028b // 02/10/04  GM /summon - Summon PC to self

#define OP_ItemTextFile             0x0296 // 02/10/04  - old BookTextCode

#define OP_TradeRequest             0x0299 // 02/10/04  Client request trade session

#define OP_OpenObject               0x029e // 02/10/04 

#define OP_PetitionResolve          0x02b3 // 02/10/04 ?Client Petition Resolve Request

#define OP_SummonCorpse             0x02b4 // 02/10/04 ?/summoncorpse

#define OP_ItemPacket               0x02df // 02/10/04  - old ItemCode

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
