/*
 *  everquest.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */


/*
** Please be kind and remember to correctly re-order
** the values in here whenever you add a new item,
** thanks.  - Andon
*/

/*
** Structures used in the network layer of Everquest
*/

#ifndef EQSTRUCT_H
#define EQSTRUCT_H

#include "config.h"

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif

/*
** ShowEQ specific definitions
*/
// Statistical list defines
#define LIST_HP                         0
#define LIST_MANA                       1
#define LIST_STAM                       2
#define LIST_EXP                        3
#define LIST_FOOD                       4
#define LIST_WATR                       5
#define LIST_STR                        6
#define LIST_STA                        7
#define LIST_CHA                        8
#define LIST_DEX                        9
#define LIST_INT                        10
#define LIST_AGI                        11
#define LIST_WIS                        12
#define LIST_MR                         13
#define LIST_FR                         14
#define LIST_CR                         15
#define LIST_DR                         16
#define LIST_PR                         17
#define LIST_AC                         18
#define LIST_ALTEXP                     19
#define LIST_MAXLIST                    20 

/*
** MOB Spawn Type
*/
#define SPAWN_PLAYER                    0
#define SPAWN_NPC                       1
#define SPAWN_PC_CORPSE                 2
#define SPAWN_NPC_CORPSE                3
#define SPAWN_NPC_UNKNOWN               4
#define SPAWN_DROP                      6
#define SPAWN_DOOR                      7
#define SPAWN_SELF                      10

/* 
** Diety List
*/
#define DEITY_UNKNOWN                   0
#define DEITY_AGNOSTIC			396
#define DEITY_BRELL			202
#define DEITY_CAZIC			203
#define DEITY_EROL			204
#define DEITY_BRISTLE			205
#define DEITY_INNY			206
#define DEITY_KARANA			207
#define DEITY_MITH			208
#define DEITY_PREXUS			209
#define DEITY_QUELLIOUS			210
#define DEITY_RALLOS			211
#define DEITY_SOLUSEK			213
#define DEITY_TRIBUNAL			214
#define DEITY_TUNARE			215
#define DEITY_BERT			201	
#define DEITY_RODCET			212
#define DEITY_VEESHAN			216

//Team numbers for Deity teams
#define DTEAM_GOOD			1
#define DTEAM_NEUTRAL			2
#define DTEAM_EVIL			3
#define DTEAM_OTHER			5

//Team numbers for Race teams
#define RTEAM_HUMAN			1
#define RTEAM_ELF			2
#define RTEAM_DARK			3
#define RTEAM_SHORT			4
#define RTEAM_OTHER			5

//Maximum limits of certain types of data
#define MAX_KNOWN_SKILLS                75
#define MAX_SPELL_SLOTS                 9
#define MAX_KNOWN_LANGS                 25
#define MAX_SPELLBOOK_SLOTS             400
#define MAX_GROUP_MEMBERS               6
#define MAX_BUFFS                       25
#define MAX_GUILDS                      1500
#define MAX_AA                          240
#define MAX_BANDOLIERS                  4
#define MAX_POTIONS_IN_BELT             4
#define MAX_TRIBUTES                    5
#define MAX_DISCIPLINES                 50

//Item Flags
#define ITEM_NORMAL                     0x0000
#define ITEM_NORMAL1                    0x0031
#define ITEM_NORMAL2                    0x0036
#define ITEM_NORMAL3                    0x315f
#define ITEM_NORMAL4                    0x3336
#define ITEM_NORMAL5                    0x0032
#define ITEM_NORMAL6                    0x0033
#define ITEM_NORMAL7                    0x0034
#define ITEM_NORMAL8                    0x0039
#define ITEM_CONTAINER                  0x7900
#define ITEM_CONTAINER_PLAIN            0x7953
#define ITEM_BOOK                       0x7379
#define ITEM_VERSION                    0xFFFF

// Item spellId no spell value
#define ITEM_SPELLID_NOSPELL            0xffff

// Item Field Count
#define ITEM_FIELD_SEPERATOR_COUNT      117
#define ITEM_CMN_FIELD_SEPERATOR_COUNT  102

//Combat Flags
#define COMBAT_MISS						0
#define COMBAT_BLOCK					-1
#define COMBAT_PARRY					-2
#define COMBAT_RIPOSTE					-3
#define COMBAT_DODGE					-4

#define PLAYER_CLASSES     16
#define PLAYER_RACES       15

/*
** Item Packet Type
*/
enum ItemPacketType
{
  ItemPacketViewLink		= 0x00,
  ItemPacketMerchant		= 0x64,
  ItemPacketLoot		= 0x66,
  ItemPacketTrade		= 0x67,
  ItemPacketSummonItem		= 0x6a,
  ItemPacketWorldContainer       = 0x6b
};

/*
** Item types
*/
enum ItemType
{
  ItemTypeCommon		= 0,
  ItemTypeContainer	= 1,
  ItemTypeBook		= 2
};

/*
** Chat Colors
*/
enum ChatColor
{
  CC_Default               = 0,
  CC_DarkGrey              = 1,
  CC_DarkGreen             = 2,
  CC_DarkBlue              = 3,
  CC_Purple                = 5,
  CC_LightGrey             = 6,
  CC_User_Say              = 256,
  CC_User_Tell             = 257,
  CC_User_Group            = 258,
  CC_User_Guild            = 259,
  CC_User_OOC              = 260,
  CC_User_Auction          = 261,
  CC_User_Shout            = 262,
  CC_User_Emote            = 263,
  CC_User_Spells           = 264,
  CC_User_YouHitOther      = 265,
  CC_User_OtherHitYou      = 266,
  CC_User_YouMissOther     = 267,
  CC_User_OtherMissYou     = 268,
  CC_User_Duels            = 269,
  CC_User_Skills           = 270,
  CC_User_Disciplines      = 271,
  CC_User_Default          = 273,
  CC_User_MerchantOffer    = 275,
  CC_User_MerchantExchange = 276,
  CC_User_YourDeath        = 277,
  CC_User_OtherDeath       = 278,
  CC_User_OtherHitOther    = 279,
  CC_User_OtherMissOther   = 280,
  CC_User_Who              = 281,
  CC_User_Yell             = 282,
  CC_User_NonMelee         = 283,
  CC_User_SpellWornOff     = 284,
  CC_User_MoneySplit       = 285,
  CC_User_Loot             = 286,
  CC_User_Random           = 287,
  CC_User_OtherSpells      = 288,
  CC_User_SpellFailure     = 289,
  CC_User_ChatChannel      = 290,
  CC_User_Chat1            = 291,
  CC_User_Chat2            = 292,
  CC_User_Chat3            = 293,
  CC_User_Chat4            = 294,
  CC_User_Chat5            = 295,
  CC_User_Chat6            = 296,
  CC_User_Chat7            = 297,
  CC_User_Chat8            = 298,
  CC_User_Chat9            = 299,
  CC_User_Chat10           = 300,
  CC_User_MeleeCrit        = 301,
  CC_User_SpellCrit        = 302,
  CC_User_TooFarAway       = 303,
  CC_User_NPCRampage       = 304,
  CC_User_NPCFurry         = 305,
  CC_User_NPCEnrage        = 306,
  CC_User_EchoSay          = 307,
  CC_User_EchoTell         = 308,
  CC_User_EchoGroup        = 309,
  CC_User_EchoGuild        = 310,
  CC_User_EchoOOC          = 311,
  CC_User_EchoAuction      = 312,
  CC_User_EchoShout        = 313,
  CC_User_EchoEmote        = 314,
  CC_User_EchoChat1        = 315,
  CC_User_EchoChat2        = 316,
  CC_User_EchoChat3        = 317,
  CC_User_EchoChat4        = 318,
  CC_User_EchoChat5        = 319,
  CC_User_EchoChat6        = 320,
  CC_User_EchoChat7        = 321,
  CC_User_EchoChat8        = 322,
  CC_User_EchoChat9        = 323,
  CC_User_EchoChat10       = 324,
  CC_User_UnusedAtThisTime = 325,
  CC_User_ItemTags         = 326,
  CC_User_RaidSay          = 327,
  CC_User_MyPet            = 328,
  CC_User_DamageShield     = 329,
};

/*
** Group Update actions
*/
enum GroupUpdateAction
{
  GUA_Joined = 0,
  GUA_Left = 1,
  GUA_LastLeft = 6,
  GUA_FullGroupInfo = 7,
  GUA_MakeLeader = 8,
  GUA_Started = 9,
};

/**
 * Leadership AAs enum, used to index into leadershipAAs in charProfileStruct
 */
enum LeadershipAAIndex
{
  groupMarkNPC = 0,
  groupNPCHealth,
  groupDelegateMainAssist,
  groupDelegateMarkNPC,
  groupUnknown4,
  groupUnknown5,
  groupInspectBuffs,
  groupUnknown7,
  groupSpellAwareness,
  groupOffenseEnhancement,
  groupManaEnhancement,
  groupHealthEnhancement,
  groupHealthRegeneration,
  groupFindPathToPC,
  groupHealthOfTargetsTarget,
  groupUnknown15,
  raidMarkNPC,  //0x10
  raidNPCHealth,
  raidDelegateMainAssist,
  raidDelegateMarkNPC,
  raidUnknown4,
  raidUnknown5,
  raidUnknown6,
  raidSpellAwareness,
  raidOffenseEnhancement,
  raidManaEnhancement,
  raidHealthEnhancement,
  raidHealthRegeneration,
  raidFindPathToPC,
  raidHealthOfTargetsTarget,
  raidUnknown14,
  raidUnknown15,
  MAX_LEAD_AA //=32
};


/*
** Compiler override to ensure
** byte aligned structures
*/
#pragma pack(1)

/*
**            Generic Structures used in specific
**                      structures below
*/

// OpCode stuff (all kinda silly, but until we stop including the OpCode everywhere)...
struct opCodeStruct
{
    int16_t opCode;

  // kinda silly -- this is required for us to be able to stuff them in a QValueList
  bool operator== ( const opCodeStruct t ) const
  {
    return ( opCode == t.opCode);
  }
  bool operator== ( uint16_t opCode2 ) const
  {
    return ( opCode == opCode2 );
  }
};

/**
 * Session request on a stream. This is sent by the client to initiate
 * a session with the zone or world server.
 * 
 * Size: 12 Octets
 */
struct SessionRequestStruct
{
/*0000*/ uint32_t unknown0000;
/*0004*/ uint32_t sessionId;
/*0008*/ uint32_t maxLength;
/*0012*/
};

/**
 * Session response on a stream. This is the server replying to a session
 * request with session information.
 *
 * Size: 19 Octets
 */
struct SessionResponseStruct
{
/*0000*/ uint32_t sessionId;
/*0004*/ uint32_t key;
/*0008*/ uint16_t unknown0008;
/*0010*/ uint8_t unknown0010;
/*0011*/ uint32_t maxLength;
/*0015*/ uint32_t unknown0015;
};

/**
 * Session disconnect on a stream. This is the server telling the client to
 * close a stream.
 *
 * Size: 8 Octets
 */
struct SessionDisconnectStruct
{
/*0000*/ uint8_t unknown[8];
};

/* 
 * Used in charProfileStruct
 * Size: 4 bytes
 */
struct Color_Struct
{
  union
  {
    struct
    {
/*0000*/uint8_t blue;
/*0001*/uint8_t red;
/*0002*/uint8_t green;
/*0003*/uint8_t unknown0003;
    } rgb;
/*0000*/uint32_t color;
  };
};

/*
** Buffs
** Length: 20 Octets
** Used in:
**    charProfileStruct(2d20)
*/
struct spellBuff
{
/*0000*/  uint8_t     unknown0000;    //
/*0001*/  int8_t      level;          // Level of person who cast buff
/*0002*/  uint8_t     unknown0002;    //
/*0003*/  uint8_t     unknown0003;    //
/*0004*/  int32_t     spellid;        // Spell
/*0008*/  int32_t     duration;       // Time remaining in ticks
/*0012*/  int32_t     effect;         // holds the dmg absorb amount on runes
/*0016*/  uint32_t    playerId;       // Global id of caster (for wear off)
/*0020*/
};


/* 
 * Used in charProfileStruct
 * Size: 8 octets
 */
struct AA_Array
{
/*000*/ uint32_t AA;
/*004*/ uint32_t value;
/*008*/
};

/**
 * An item inline in the stream, used in Bandolier and Potion Belt.
 * Size: 72 Octets 
 */
struct InlineItem
{
/*000*/ uint32_t itemId;
/*004*/ uint32_t icon;
/*008*/ char itemName[64];
/*072*/
};

/**
 * Used in charProfileStruct. Contents of a Bandolier.
 * Size: 320 Octets 
 */
struct BandolierStruct
{
/*000*/ char bandolierName[32];
/*032*/ InlineItem mainHand;
/*104*/ InlineItem offHand;
/*176*/ InlineItem range;
/*248*/ InlineItem ammo;
/*320*/
};

/**
 * A tribute a player can have loaded.
 * Size: 8 Octets 
 */
struct TributeStruct
{
/*000*/ uint32_t tribute;
/*004*/ uint32_t rank;
/*008*/
};

/**
 * A bind point.
 * Size: 20 Octets
 */
struct BindStruct
{
/*000*/ uint32_t zoneId;
/*004*/ float x;
/*008*/ float y;
/*012*/ float z;
/*016*/ float heading;
/*020*/
};

/*
** Type:   Zone Change Request (before hand)
** Length: 88 Octets
** OpCode: ZoneChangeCode
*/
struct zoneChangeStruct
{
/*0000*/ char     name[64];	     	// Character Name
/*0064*/ uint16_t zoneId;           // zone Id
/*0066*/ uint16_t zoneInstance;     // zone Instance
/*0068*/ uint8_t  unknown0068[8];   // unknown
/*0076*/ uint8_t  unknown0076[12];  // ***Placeholder (6/29/2005)
/*0088*/
};

/*
** Type:  Request Zone Change (server asking the client to change zones)
** Size:  24 Octets
** OpCode: OP_RequestZoneChange
*/
struct requestZoneChangeStruct
{
/*0000*/ uint16_t zoneId;       // Zone to change to
/*0002*/ uint16_t zoneInstance; // Instance to change to
/*0004*/ float x;               // Zone in x coord in next zone
/*0008*/ float y;               // Zone in y coord in next zone
/*0012*/ float z;               // Zone in z coord in next zone
/*0016*/ float heading;               // Zone in heading in next zone
/*0020*/ uint32_t unknown0020;  // *** Placeholder
};

/*
** Client Zone Entry struct
** Length: 68 Octets
** OpCode: ZoneEntryCode (when direction == client)
*/
struct ClientZoneEntryStruct
{
/*0000*/ uint32_t unknown0000;            // ***Placeholder
/*0004*/ char     name[32];               // Player firstname
/*0036*/ uint8_t  unknown0036[28];        // ***Placeholder
/*0064*/ uint32_t unknown0064;            // unknown
};


/*
** New Zone Code
** Length: 700 Octets
** OpCode: NewZoneCode
*/
struct newZoneStruct
{
/*0000*/ char    name[64];                 // Character name
/*0064*/ char    shortName[32];            // Zone Short Name
/*0096*/ char    longName[278];            // Zone Long Name
/*0374*/ uint8_t ztype;                    // Zone type
/*0375*/ uint8_t fog_red[4];               // Zone fog (red)
/*0379*/ uint8_t fog_green[4];             // Zone fog (green)
/*0383*/ uint8_t fog_blue[4];              // Zone fog (blue)
/*0387*/ uint8_t unknown0374[87];          // *** Placeholder
/*0474*/ uint8_t sky;                      // Zone sky
/*0475*/ uint8_t unknown0475[13];          // *** Placeholder
/*0488*/ float   zone_exp_multiplier;      // Experience Multiplier
/*0492*/ float   safe_y;                   // Zone Safe Y
/*0496*/ float   safe_x;                   // Zone Safe X
/*0500*/ float   safe_z;                   // Zone Safe Z
/*0504*/ float   unknown0504;              // *** Placeholder
/*0508*/ float   underworld;               // Underworld
/*0512*/ float   minclip;                  // Minimum view distance
/*0516*/ float   maxclip;                  // Maximum view distance
/*0520*/ uint8_t unknown0520[172];         // *** Placeholder
/*0692*/ uint8_t unknown0692[4];           // *** Placeholder (06/29/2005)
/*0696*/ uint8_t unknown0696[4];           // *** Placeholder (09/13/2005)
}; /*0700*/


/*
** Player Profile
** Length: 19584 Octets
** OpCode: CharProfileCode
*/
struct charProfileStruct
{
/*00000*/ uint32_t  checksum;          //
/*00004*/ uint32_t  gender;            // Player Gender - 0 Male, 1 Female
/*00008*/ uint32_t  race;              // Player race
/*00012*/ uint32_t  class_;            // Player class
/*00016*/ uint32_t  unknown00016;      // ***Placeholder
/*00020*/ uint8_t   level;             // Level of player
/*00021*/ uint8_t   level1;            // Level of player (again?)
/*00022*/ uint8_t   unknown00022[2];   // ***Placeholder
/*00024*/ BindStruct binds[5];         // Bind points (primary is first)
/*00124*/ uint32_t  deity;             // deity
/*00128*/ uint8_t unknown00128[60];
/*00188*/ uint32_t  item_material[9];  // Item texture/material of worn items
/*00224*/ uint8_t unknown00224[48];
/*00272*/ Color_Struct item_tint[9];   // RR GG BB 00
/*00308*/ AA_Array   aa_array[MAX_AA]; // AAs

#if 0
    // aa_array and points overlap... %%%
/*02224*/ uint32_t  points;             // Unspent Practice points
#endif
/*02228*/ uint32_t  MANA;               // Current MANA
/*02232*/ uint32_t  curHp;              // Current HP without +HP equipment
/*02236*/ uint32_t  STR;                // Strength
/*02240*/ uint32_t  STA;                // Stamina
/*02244*/ uint32_t  CHA;                // Charisma
/*02248*/ uint32_t  DEX;                // Dexterity
/*02252*/ uint32_t  INT;                // Intelligence
/*02256*/ uint32_t  AGI;                // Agility
/*02260*/ uint32_t  WIS;                // Wisdom
/*02264*/ uint8_t unknown02264[48];
/*02312*/ int32_t   sSpellBook[400];    // List of the Spells in spellbook
/*03912*/ uint8_t   unknown4184[448];   // all 0xff after last spell    
/*04360*/ int32_t   sMemSpells[MAX_SPELL_SLOTS]; // List of spells memorized
/*04396*/ uint8_t unknown04396[32];
/*04428*/ uint32_t  platinum;           // Platinum Pieces on player
/*04432*/ uint32_t  gold;               // Gold Pieces on player
/*04436*/ uint32_t  silver;             // Silver Pieces on player
/*04440*/ uint32_t  copper;             // Copper Pieces on player
/*04444*/ uint32_t  platinum_cursor;    // Platinum Pieces on cursor
/*04448*/ uint32_t  gold_cursor;        // Gold Pieces on cursor
/*04452*/ uint32_t  silver_cursor;      // Silver Pieces on cursor
/*04456*/ uint32_t  copper_cursor;      // Copper Pieces on cursor
/*04460*/ uint32_t  skills[MAX_KNOWN_SKILLS]; // List of skills 
/*04760*/ uint8_t unknown04760[748];
/*05508*/ spellBuff buffs[MAX_BUFFS];   // Buffs currently on the player
/*06008*/ uint8_t unknown06008[144];
/*06152*/ uint32_t aa_spent;           // Number of spent AA points
/*06156*/ uint32_t aa_unspent;         // Unspent AA points
/*06160*/ uint8_t unknown06160[4];
/*06164*/ BandolierStruct bandoliers[MAX_BANDOLIERS]; // bandolier contents
/*07444*/ uint8_t unknown07444[5120];
/*12564*/ InlineItem potionBelt[MAX_POTIONS_IN_BELT]; // potion belt
/*12852*/ uint8_t unknown12852[88];
/*12940*/ char      name[64];           // Name of player
/*13004*/ char      lastName[32];       // Last name of player
/*13036*/ int32_t   guildID;            // guildID
/*13040*/ uint32_t  birthdayTime;       // character birthday
/*13044*/ uint32_t  lastSaveTime;       // character last save time
/*13048*/ uint32_t  timePlayedMin;      // time character played
/*13052*/ uint8_t unknown13052[32];
/*13084*/ uint8_t   languages[MAX_KNOWN_LANGS]; // List of languages
/*13109*/ uint8_t   unknown13109[7];    // All 0x00 (language buffer?)
/*13116*/ float     x;                  // Players x position
/*13120*/ float     y;                  // Players y position
/*13124*/ float     z;                  // Players z position
/*13128*/ float     heading;            // Players heading   
/*13132*/ uint8_t   unknown13132[4];    // ***Placeholder
/*13136*/ uint32_t  platinum_bank;      // Platinum Pieces in Bank
/*13140*/ uint32_t  gold_bank;          // Gold Pieces in Bank
/*13144*/ uint32_t  silver_bank;        // Silver Pieces in Bank
/*13148*/ uint32_t  copper_bank;        // Copper Pieces in Bank
/*13152*/ uint32_t  platinum_shared;    // Shared platinum pieces
/*13156*/ uint8_t unknown13156[84];
/*13240*/ uint32_t  expansions;         // Bitmask for expansions
/*13244*/ uint8_t unknown13244[32];
/*13276*/ uint16_t  zoneId;             // see zones.h
/*13278*/ uint16_t  zoneInstance;       // Instance id
/*13280*/ char      groupMembers[MAX_GROUP_MEMBERS][64];// all the members in group, including self 
/*13664*/ char      groupLeader[64];    // Leader of the group ?
/*13728*/ uint8_t unknown13728[668];
/*14396*/ uint32_t  ldon_guk_points;    // Earned GUK points
/*14400*/ uint32_t  ldon_mir_points;    // Earned MIR points
/*14404*/ uint32_t  ldon_mmc_points;    // Earned MMC points
/*14408*/ uint32_t  ldon_ruj_points;    // Earned RUJ points
/*14412*/ uint32_t  ldon_tak_points;    // Earned TAK points
/*14416*/ uint32_t  ldon_avail_points;  // Available LDON points
/*14420*/ uint8_t unknown14420[132];
/*14552*/ uint32_t  tributeTime;        // Time remaining on tribute (millisecs)
/*14556*/ uint32_t  careerTribute;      // Total favor points for this char
/*14560*/ uint32_t  unknown7208;        // *** Placeholder
/*14564*/ uint32_t  currentTribute;     // Current tribute points
/*14568*/ uint32_t  unknown7216;        // *** Placeholder
/*14572*/ uint32_t  tributeActive;      // 0 = off, 1=on
/*14576*/ TributeStruct tributes[MAX_TRIBUTES]; // Current tribute loadout
/*14616*/ uint8_t unknown14616[4940];
/*19556*/ uint32_t currentRadCrystals; // Current count of radiant crystals
/*19560*/ uint32_t careerRadCrystals;  // Total count of radiant crystals ever
/*19564*/ uint32_t currentEbonCrystals;// Current count of ebon crystals
/*19568*/ uint32_t careerEbonCrystals; // Total count of ebon crystals ever
/*19572*/ uint8_t  groupAutoconsent;   // 0=off, 1=on
/*19573*/ uint8_t  raidAutoconsent;    // 0=off, 1=on
/*19574*/ uint8_t  guildAutoconsent;   // 0=off, 1=on
/*19575*/ uint8_t  unknown19575[5];    // ***Placeholder (6/29/2005)
/*19580*/ uint32_t unknown19580;       // *** Placeholder
/*19584*/
};

#if 0
/*0240*/ uint8_t   pvp;                // 1=pvp, 0=not pvp
/*0242*/ uint8_t   anon;               // 2=roleplay, 1=anon, 0=not anon     
/*0243*/ uint8_t   gm;                 // 0=no, 1=yes
/*0244*/ int8_t    guildstatus;        // 0=member, 1=officer, 2=guildleader
/*0252*/ uint32_t  intoxication;       // Alcohol level (in ticks till sober?)
/*0256*/ uint32_t  spellSlotRefresh[MAX_SPELL_SLOTS]; // Refresh time (millis)
/*0296*/ uint8_t   haircolor;          // Player hair color
/*0297*/ uint8_t   beardcolor;         // Player beard color
/*0298*/ uint8_t   eyecolor1;          // Player left eye color
/*0299*/ uint8_t   eyecolor2;          // Player right eye color
/*0300*/ uint8_t   hairstyle;          // Player hair style
/*0301*/ uint8_t   beard;              // Player beard type
/*2384*/ char      title[32];          // Current character title
/*2352*/ char      servername[32];     // server the char was created on
/*2416*/ char      suffix[32];         // Current character suffix
/*2452*/ uint32_t  exp;                // Current Experience
/*2504*/ uint8_t   face;               // Player face
/*5408*/ uint32_t  autosplit;          // 0 = off, 1 = on
/*5420*/ uint32_t  saveCounter;        // Number of times your char has saved
/*5456*/ uint32_t  toxicity;           // Potion Toxicity (15=too toxic, each potion adds 3)
/*5476*/ uint32_t  hunger;             // Food (ticks till next eat)
/*5480*/ uint32_t  thirst;             // Drink (ticks till next drink)
/*7264*/ uint32_t  disciplines[MAX_DISCIPLINES]; // Known disciplines
/*7904*/ uint32_t  endurance;          // Current endurance
/*7908*/ uint32_t  expGroupLeadAA;     // Current group lead AA exp (0-1000)
/*7912*/ uint32_t  expRaidLeadAA;      // Current raid lead AA exp (0-2000)
/*7916*/ uint32_t  groupLeadAAUnspent; // Unspent group lead AA points
/*7920*/ uint32_t  raidLeadAAUnspent;  // Unspent raid lead AA points
/*7924*/ uint32_t  leadershipAAs[MAX_LEAD_AA]; // Leader AA ranks
/*8184*/ uint32_t  airRemaining;       // Air supply (seconds)
/*12800*/ uint32_t expAA;              // Exp earned in current AA point
#endif

#if 1
struct playerAAStruct {
/*    0 */  uint8_t unknown0;
  union {
    uint8_t unnamed[17];
    struct _named {  
/*    1 */  uint8_t innate_strength;
/*    2 */  uint8_t innate_stamina;
/*    3 */  uint8_t innate_agility;
/*    4 */  uint8_t innate_dexterity;
/*    5 */  uint8_t innate_intelligence;
/*    6 */  uint8_t innate_wisdom;
/*    7 */  uint8_t innate_charisma;
/*    8 */  uint8_t innate_fire_protection;
/*    9 */  uint8_t innate_cold_protection;
/*   10 */  uint8_t innate_magic_protection;
/*   11 */  uint8_t innate_poison_protection;
/*   12 */  uint8_t innate_disease_protection;
/*   13 */  uint8_t innate_run_speed;
/*   14 */  uint8_t innate_regeneration;
/*   15 */  uint8_t innate_metabolism;
/*   16 */  uint8_t innate_lung_capacity;
/*   17 */  uint8_t first_aid;
    } named;
  } general_skills;
  union {
    uint8_t unnamed[17];
    struct _named {
/*   18 */  uint8_t healing_adept;
/*   19 */  uint8_t healing_gift;
/*   20 */  uint8_t unknown20;
/*   21 */  uint8_t spell_casting_reinforcement;
/*   22 */  uint8_t mental_clarity;
/*   23 */  uint8_t spell_casting_fury;
/*   24 */  uint8_t chanelling_focus;
/*   25 */  uint8_t unknown25;
/*   26 */  uint8_t unknown26;
/*   27 */  uint8_t unknown27;
/*   28 */  uint8_t natural_durability;
/*   29 */  uint8_t natural_healing;
/*   30 */  uint8_t combat_fury;
/*   31 */  uint8_t fear_resistance;
/*   32 */  uint8_t finishing_blow;
/*   33 */  uint8_t combat_stability;
/*   34 */  uint8_t combat_agility;
    } named;
  } archetype_skills;
  union {
    uint8_t unnamed[93];
    struct _name {
/*   35 */  uint8_t mass_group_buff; // All group-buff-casting classes(?)
// ===== Cleric =====
/*   36 */  uint8_t divine_resurrection;
/*   37 */  uint8_t innate_invis_to_undead; // cleric, necromancer
/*   38 */  uint8_t celestial_regeneration;
/*   39 */  uint8_t bestow_divine_aura;
/*   40 */  uint8_t turn_undead;
/*   41 */  uint8_t purify_soul;
// ===== Druid =====
/*   42 */  uint8_t quick_evacuation; // wizard, druid
/*   43 */  uint8_t exodus; // wizard, druid
/*   44 */  uint8_t quick_damage; // wizard, druid
/*   45 */  uint8_t enhanced_root; // druid
/*   46 */  uint8_t dire_charm; // enchanter, druid, necromancer
// ===== Shaman =====
/*   47 */  uint8_t cannibalization;
/*   48 */  uint8_t quick_buff; // shaman, enchanter
/*   49 */  uint8_t alchemy_mastery;
/*   50 */  uint8_t rabid_bear;
// ===== Wizard =====
/*   51 */  uint8_t mana_burn;
/*   52 */  uint8_t improved_familiar;
/*   53 */  uint8_t nexus_gate;
// ===== Enchanter  =====
/*   54 */  uint8_t unknown54;
/*   55 */  uint8_t permanent_illusion;
/*   56 */  uint8_t jewel_craft_mastery;
/*   57 */  uint8_t gather_mana;
// ===== Mage =====
/*   58 */  uint8_t mend_companion; // mage, necromancer
/*   59 */  uint8_t quick_summoning;
/*   60 */  uint8_t frenzied_burnout;
/*   61 */  uint8_t elemental_form_fire;
/*   62 */  uint8_t elemental_form_water;
/*   63 */  uint8_t elemental_form_earth;
/*   64 */  uint8_t elemental_form_air;
/*   65 */  uint8_t improved_reclaim_energy;
/*   66 */  uint8_t turn_summoned;
/*   67 */  uint8_t elemental_pact;
// ===== Necromancer =====
/*   68 */  uint8_t life_burn;
/*   69 */  uint8_t dead_mesmerization;
/*   70 */  uint8_t fearstorm;
/*   71 */  uint8_t flesh_to_bone;
/*   72 */  uint8_t call_to_corpse;
// ===== Paladin =====
/*   73 */  uint8_t divine_stun;
/*   74 */  uint8_t improved_lay_of_hands;
/*   75 */  uint8_t slay_undead;
/*   76 */  uint8_t act_of_valor;
/*   77 */  uint8_t holy_steed;
/*   78 */  uint8_t fearless; // paladin, shadowknight

/*   79 */  uint8_t two_hand_bash; // paladin, shadowknight
// ===== Ranger =====
/*   80 */  uint8_t innate_camouflage; // ranger, druid
/*   81 */  uint8_t ambidexterity; // all "dual-wield" users
/*   82 */  uint8_t archery_mastery; // ranger
/*   83 */  uint8_t unknown83;
/*   84 */  uint8_t endless_quiver; // ranger
// ===== Shadow Knight =====
/*   85 */  uint8_t unholy_steed;
/*   86 */  uint8_t improved_harm_touch;
/*   87 */  uint8_t leech_touch;
/*   88 */  uint8_t unknown88;
/*   89 */  uint8_t soul_abrasion;
// ===== Bard =====
/*   90 */  uint8_t instrument_mastery;
/*   91 */  uint8_t unknown91;
/*   92 */  uint8_t unknown92;
/*   93 */  uint8_t unknown93;
/*   94 */  uint8_t jam_fest;
/*   95 */  uint8_t unknown95;
/*   96 */  uint8_t unknown96;
// ===== Monk =====
/*   97 */  uint8_t critical_mend;
/*   98 */  uint8_t purify_body;
/*   99 */  uint8_t unknown99;
/*  100 */  uint8_t rapid_feign;
/*  101 */  uint8_t return_kick;
// ===== Rogue =====
/*  102 */  uint8_t escape;
/*  103 */  uint8_t poison_mastery;
/*  104 */  uint8_t double_riposte; // all "riposte" users
/*  105 */  uint8_t unknown105;
/*  106 */  uint8_t unknown106;
/*  107 */  uint8_t purge_poison; // rogue
// ===== Warrior =====
/*  108 */  uint8_t flurry;
/*  109 */  uint8_t rampage;
/*  110 */  uint8_t area_taunt;
/*  111 */  uint8_t warcry;
/*  112 */  uint8_t bandage_wound;
// ===== (Other) =====
/*  113 */  uint8_t spell_casting_reinforcement_mastery; // all "pure" casters
/*  114 */  uint8_t unknown114;
/*  115 */  uint8_t extended_notes; // bard
/*  116 */  uint8_t dragon_punch; // monk
/*  117 */  uint8_t strong_root; // wizard
/*  118 */  uint8_t singing_mastery; // bard
/*  119 */  uint8_t body_and_mind_rejuvenation; // paladin, ranger, bard
/*  120 */  uint8_t physical_enhancement; // paladin, ranger, bard
/*  121 */  uint8_t adv_trap_negotiation; // rogue, bard
/*  122 */  uint8_t acrobatics; // all "safe-fall" users
/*  123 */  uint8_t scribble_notes; // bard
/*  124 */  uint8_t chaotic_stab; // rogue
/*  125 */  uint8_t pet_discipline; // all pet classes except enchanter
/*  126 */  uint8_t unknown126;
/*  127 */  uint8_t unknown127;
    } named;
  } class_skills;
};
#endif

/* 
** Generic Spawn Struct 
** Length: 384 Octets 
** Used in: 
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/ 
struct spawnStruct
{
/*0000*/ uint8_t  invis;          // Invis (0=not, 1=invis)
/*0001*/ uint32_t spawnId;        // Spawn Id
/*0005*/ uint8_t  lfg;            // 0=off, 1=lfg on
/*0006*/ uint8_t  beard;          // Beard style (not totally, sure but maybe!)
/*0007*/ uint8_t  eyecolor1;      // Player's left eye color
/*0008*/ uint8_t  is_pet;         // 0=no, 1=yes
/*0009*/ uint32_t petOwnerId;     // If this is a pet, the spawn id of owner
/*0013*/ uint8_t unknown0013[23];
/*0036*/ uint8_t  afk;            // 0=no, 1=afk
/*0037*/ uint8_t  eyecolor2;      // Left eye color
/*0038*/ uint8_t  NPC;            // 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*0039*/ uint8_t  anon;           // 0=normal, 1=anon, 2=roleplay
/*0040*/ uint32_t race;           // Spawn race
/*0044*/ uint8_t unknown0040[5];
/*0049*/ uint8_t  bodytype;       // Bodytype
/*0050*/ uint8_t unknown0050[3];
/*0053*/ uint8_t  max_hp;         // (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
/*0054*/ uint8_t  is_npc;         // 0=no, 1=yes
/*0055*/ uint8_t  class_;         // Player's class
/*0056*/ char     suffix[32];     // Player's suffix (of Veeshan, etc.)
/*0088*/ uint8_t  haircolor;      // Hair color
/*0089*/ int16_t  deity;          // Player's Deity
/*0091*/ uint8_t unknown0091[2];
/*0093*/ uint8_t  face;	          // Face id for players
/*0094*/ uint8_t unknown0094[2];
/*0096*/ uint8_t  light;          // Spawn's lightsource
/*0097*/ uint8_t  curHp;          // Current hp
/*0098*/ char     lastName[32];   // Player's Lastname
/*0130*/ uint8_t  beardcolor;     // Beard color
/*0131*/ uint8_t unknown0131;
/*0132*/ float    size;           // Model size
/*0136*/ int8_t   aa_title;       // 0=none, 1=general, 2=archtype, 3=class
/*0137*/ uint8_t  gender;         // Gender (0=male, 1=female)
/*0138*/ uint8_t unknown0138[7];
/*0145*/ uint32_t guildID;        // Current guild
/*0149*/ uint8_t unknown149[4];
/*0153*/ float    walkspeed;      // Speed when walking
/*0157*/ uint8_t  hairstyle;      // Hair style
/*0158*/ uint8_t unknown0157[6];
/*0164*/ int8_t   guildrank;      // 0=normal, 1=officer, 2=leader
/*0165*/ uint8_t unknown0165[4];
/*0169*/ uint8_t  gm;             // 0=no, 1=gm
/*0170*/ uint8_t unknown0170[5];
union 
{
/*0175*/ int8_t equip_chest2;     // Second place in packet for chest texture (usually 0xFF in live packets)
                                  // Not sure why there are 2 of them, but it effects chest texture!
/*0175*/ int8_t mount_color;      // drogmor: 0=white, 1=black, 2=green, 3=red
                                  // horse: 0=brown, 1=white, 2=black, 3=tan
};
/*0176*/ float    runspeed;       // Speed when running
/*0180*/ union 
         {
             struct 
             {
                 /*0180*/ Color_Struct color_helmet;    // Color of helmet item
                 /*0184*/ Color_Struct color_chest;     // Color of chest item
                 /*0188*/ Color_Struct color_arms;      // Color of arms item
                 /*0192*/ Color_Struct color_bracers;   // Color of bracers item
                 /*0196*/ Color_Struct color_hands;     // Color of hands item
                 /*0200*/ Color_Struct color_legs;      // Color of legs item
                 /*0204*/ Color_Struct color_feet;      // Color of feet item
                 /*0208*/ Color_Struct color_primary;   // Color of primary item
                 /*0212*/ Color_Struct color_secondary; // Color of secondary item
             } equipment_colors;
             /*0180*/ Color_Struct colors[9]; // Array elements correspond to struct equipment_colors above
         };
/*0216*/ uint8_t  findable;       // 0=can't be found, 1=can be found
/*0217*/ union 
         {
             struct 
             {
               /*0217*/ uint32_t equip_helmet;    // Equipment: Helmet Visual
               /*0221*/ uint32_t equip_chest;     // Equipment: Chest Visual
               /*0225*/ uint32_t equip_arms;      // Equipment: Arms Visual
               /*0229*/ uint32_t equip_bracers;   // Equipment: Bracers Visual
               /*0233*/ uint32_t equip_hands;     // Equipment: Hands Visual
               /*0237*/ uint32_t equip_legs;      // Equipment: Legs Visual
               /*0241*/ uint32_t equip_feet;      // Equipment: Feet Visual
               /*0245*/ uint32_t equip_primary;   // Equipment: Primary Visual
               /*0249*/ uint32_t equip_secondary; // Equipment: Secondary Visual
             } equip;
             /*0217*/ uint32_t equipment[9];  // Array elements correspond to struct equipment above
         };
/*0253*/ uint8_t  set_to_0xFF[8]; // ***Placeholder (all ff)
/*0261*/ uint8_t  level;          // Spawn Level
/*0262*/ uint8_t unknown0262;
/*0263*/ char     title[32];      // Title
/*0295*/ char     name[64];       // Player's Name
/*0359*/ uint8_t  unknown0259[4]; // ***Placeholder
/*0363*/ uint8_t  helm;           // Helm texture
/*0364*/ signed   deltaHeading:10;// change in heading
         signed   x:19;           // x coord
         signed   padding0054:3;  // ***Placeholder
/*0368*/ signed   y:19;           // y coord
         signed   animation:10;   // animation
         signed   padding0058:3;  // ***Placeholder
/*0372*/ signed   z:19;           // z coord
         signed   deltaY:13;      // change in y
/*0376*/ signed   deltaX:13;      // change in x
         unsigned heading:12;     // heading
         signed   padding0066:7;  // ***Placeholder
/*0380*/ signed   deltaZ:13;      // change in z
         signed   padding0070:19; // ***Placeholder
}; /*0384*/

#if 0
/*122*/ uint8_t pvp; // 0=Not pvp,1=pvp
#endif

/*
** Server Zone Entry struct
** Length: 383 Octets
** OpCode: ZoneEntryCode (when direction == server)
*
*  This is just a spawnStruct for the player
*/
struct ServerZoneEntryStruct : public spawnStruct
{
};

/*
** Generic Door Struct
** Length: 80 Octets
** Used in: 
**    cDoorSpawnsStruct(f721)
**
*/

struct doorStruct
{
/*0000*/ char     name[16];        // Filename of Door?
/*0016*/ uint8_t  unknown016[16]; // ***Placeholder
/*0032*/ float    y;               // y loc
/*0036*/ float    x;               // x loc
/*0040*/ float    z;               // z loc
/*0044*/ float    heading;         // heading
/*0048*/ uint8_t  unknown0028[7]; // ***Placeholder
/*0055*/ int8_t   auto_return;
/*0056*/ uint8_t  initialState;
/*0057*/ uint8_t  unknown041[3];
/*0060*/ uint8_t  doorId;          // door's id #
/*0061*/ uint8_t  opentype;       
/*0062*/ uint8_t  size;           // size of door
/*0063*/ uint8_t holdstateforever;
/*0064*/ uint32_t zonePoint;
/*0068*/ uint8_t  unknown068[12]; // ***Placeholder
/*0080*/
}; 

/*
** Drop Item On Ground
** Length: 92 Octets
** OpCode: MakeDropCode
*/
struct makeDropStruct
{
/*0000*/ uint32_t prevObject;             // Previous object in the linked list
/*0004*/ uint32_t nextObject;             // Next object in the linked list
/*0008*/ uint32_t unknown0008;            // ***Placeholder
/*0012*/ uint32_t dropId;                 // DropID
/*0016*/ uint16_t zoneId;                 // ZoneID
/*0018*/ uint16_t zoneInstance;           // Zone instance id
/*0014*/ uint8_t  unknown0014[8];         // ***Placeholder
/*0028*/ float    heading;                // Heading
/*0032*/ float    z;                      // Z Position
/*0036*/ float    x;                      // X Position
/*0040*/ float    y;                      // Y Position
/*0044*/ char     idFile[16];             // ACTOR ID
/*0060*/ uint32_t unknown0060[5];         // ***Placeholder
/*0080*/ uint32_t dropType;               // drop type
/*0084*/ uint32_t unknown0084;            // ***Placeholder
/*0088*/ uint32_t userSpawnID;            // spawn id of the person using
/*0092*/
};

/*
** ZonePoint
** Length: 24 Octets
** Sent as part of zonePointsStruct
*/

struct zonePointStruct
{
  /*0000*/ uint32_t zoneTrigger;
  /*0004*/ float    y;
  /*0008*/ float    x;
  /*0012*/ float    z;
  /*0016*/ float    heading;
  /*0020*/ uint16_t zoneId;
  /*0022*/ uint16_t zoneInstance;
  /*0024*/
};

/*
** ZonePointsStruct
** Length: Variable
** OPCode: OP_SendZonePoints
*/
struct zonePointsStruct
{
  /*0000*/ uint32_t        count;
  /*0004*/ zonePointStruct zonePoints[0]; 
  /*0xxx*/ uint8_t         unknown0xxx[24];
  /*0yyy*/
};

/*
** Time of Day
** Length: 8 Octets
** OpCode: TimeOfDayCode
*/
struct timeOfDayStruct
{
/*0000*/ uint8_t  hour;                   // Hour (1-24)
/*0001*/ uint8_t  minute;                 // Minute (0-59)
/*0002*/ uint8_t  day;                    // Day (1-28)
/*0003*/ uint8_t  month;                  // Month (1-12)
/*0004*/ uint16_t year;                   // Year
/*0006*/ uint16_t unknown0016;            // Placeholder
/*0008*/
};

/*
** Item Packet Struct - Works on a variety of item operations
** Packet Types: See ItemPacketType enum
** Length: Variable
** OpCode: ItemCode
*/
struct itemPacketStruct
{
/*000*/	ItemPacketType	packetType;       // See ItemPacketType for more info.
/*004*/	char		serializedItem[0];
/*xx*/
};

/*
** Item Info Request Struct 
** OpCode: ItemInfoCode
*/
struct itemInfoReqStruct
{
/*000*/ uint32_t itemNr;                  // ItemNr 
/*005*/ uint32_t requestSeq;              // Request sequence number
/*008*/ char     name[64];                // Item name
/*072*/
};

/*
** Item Info Response Struct
** Length: Variable
** OpCode: ItemInfoCode
*/
struct itemInfoStruct
{
/*000*/	uint32_t	requestSeq;       // Corresponds to sequence # in req
/*004*/	char		serializedItem[0];
/*xxx*/
};

/*
** Simple Spawn Update
** Length: 14 Octets
** OpCode: MobUpdateCode
*/

struct spawnPositionUpdate 
{
/*0000*/ int16_t  spawnId;
/*0002*/ int64_t  y:19, z:19, x:19, u3:7;
         unsigned heading:12;
         signed unused2:4;
/*0010*/
};

/*
** Rename a spawn
** Length: 200 Octets
** OpCode: SpawnRename
*/
struct spawnRenameStruct
{
/*000*/	char        old_name[64];
/*064*/	char        old_name_again[64];	//not sure what the difference is
/*128*/	char        new_name[64];
/*192*/	uint32_t	unknown192;	        //set to 0
/*196*/	uint32_t	unknown196;	        //set to 1
};

/*
** Illusion a spawn
** Length: 168 Octets
** OpCode: Illusion
*/
struct spawnIllusionStruct
{
/*0000*/ uint32_t   spawnId;            // Spawn id of the target
/*0004*/ char       name[64];           // Name of the target
/*0068*/ uint32_t   race;               // New race
/*0072*/ uint8_t    gender;             // New gender (0=male, 1=female)
/*0073*/ uint8_t    texture;            // ???
/*0074*/ uint8_t    helm;               // ???
/*0075*/ uint8_t    unknown0077;        // ***Placeholder
/*0076*/ uint32_t   face;               // New face
/*0080*/ uint8_t    unknown0082[88];    // ***Placeholder
};


/*
**                 ShowEQ Specific Structures
*/

/*
** DB spawn struct (adds zone spawn was in)
*/

struct dbSpawnStruct
{
/*0000*/ struct spawnStruct spawn;      // Spawn Information
/*0258*/ char   zoneName[40];           // Zone Information
};

/*
** Pet spawn struct (pets pet and owner in one struct)
*/

struct petStruct
{
/*0000*/ struct spawnStruct owner;      // Pet Owner Information
/*0258*/ struct spawnStruct pet;        // Pet Infromation
};

/*
** Server System Message
** Length: Variable Length
** OpCode: SysMsgCode
*/

struct sysMsgStruct
{
/*0000*/ char     message[0];             // Variable length message
};

/*
** Emote text
** Length: Variable Text
** OpCode: emoteTextCode
*/

struct emoteTextStruct
{
/*0000*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0002*/ char     text[0];                // Emote `Text
};

/*
** Channel Message received or sent
** Length: 148 Octets + Variable Length + 4 Octets
** OpCode: ChannelMessageCode
*/

struct channelMessageStruct
{
/*0000*/ char     target[64];             // the target characters name
/*0064*/ char     sender[64];             // The senders name 
/*0128*/ uint32_t language;               // Language
/*0132*/ uint32_t chanNum;                // Channel
/*0136*/ int8_t   unknown0136[8];        // ***Placeholder
/*0144*/ uint32_t skillInLanguage;        // senders skill in language
/*0148*/ char     message[0];             // Variable length message
};

/*
** Formatted text messages
** Length: Variable Text
** OpCode: emoteTextCode
*/

struct formattedMessageStruct
{
/*0000*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0004*/ uint32_t messageFormat;          // Indicates the message format
/*0008*/ ChatColor messageColor;          // Message color
/*0012*/ char     messages[0];            // messages(NULL delimited)
/*0???*/ uint8_t  unknownXXXX[8];         // ***Placeholder
};

/*
** Simple text messages
** Length: 12 Octets
** OpCode: SimpleMessageCode
*/

struct simpleMessageStruct
{
/*0000*/ uint32_t  messageFormat;          // Indicates the message format
/*0004*/ ChatColor messageColor;                  // Message color
/*0008*/ uint32_t  unknown;                // ***Placeholder
/*0012*/
};

/*
** Special Message Struct
** Length: Variable Text
** OPCode: OP_SpecialMesg
*/

struct specialMessageStruct
{
  /*0000*/ uint8_t   unknown0000[3];  // message style?
  /*0003*/ ChatColor messageColor;    // message color
  /*0007*/ uint16_t  target;          // message target
  /*0009*/ uint16_t  padding;         // padding
  /*0011*/ char      source[0];       // message text
  /*0xxx*/ uint32_t  unknown0xxx[3];  //***Placeholder
  /*0yyy*/ char      message[0];      // message text
};

/*
** Guild MOTD Struct
** Length: Variable Text
** OPCode: OP_GuildMOTD
*/
struct guildMOTDStruct
{
  /*0000*/ uint32_t unknown0000;      //***Placeholder
  /*0004*/ char     target[64];       // motd target
  /*0068*/ char     sender[64];       // motd "sender" (who set it)
  /*0132*/ uint32_t unknown0132;      //***Placeholder
  /*0136*/ char     message[0];
};

/*
** Corpse location
** Length: 18 Octets
** OpCode: corpseLocCode
*/

struct corpseLocStruct
{
/*0000*/ uint32_t spawnId;
/*0004*/ float    x;
/*0008*/ float    y;
/*0012*/ float    z;
};

/*
** Consent request
** Length: Variable by length of the name of the consentee
*/

struct consentRequestStruct
{
/*0000*/ char consentee[0];        // Name of player who was consented
};

/*
** Consent Response
** Length: 193 Octets
*/

struct consentResponseStruct
{
/*0000*/ char consentee[64];        // Name of player who was consented
/*0064*/ char consenter[64];        // Name of player who consented
/*0128*/ uint8_t allow;             // 00 = deny, 01 = allow
/*0129*/ char corpseZoneName[64];   // Zone where the corpse is
/*0193*/
};

/*
** Grouping Infromation
** Length: 452 Octets
** OpCode: OP_GroupUpdate
*/

struct groupUpdateStruct
{
/*0000*/ int32_t  action;           // Group update action
/*0004*/ char     yourname[64];     // Group Member Names
/*0068*/ char     membername[64];   // Group leader name
/*0132*/ uint8_t  unknown0132[320]; // ***Placeholder
/*452*/
};


/*
** Grouping Infromation
** Length: 768 Octets
** OpCode: OP_GroupUpdate
*/

struct groupFullUpdateStruct
{
/*0000*/ int32_t  action;
/*0004*/ char     membernames[MAX_GROUP_MEMBERS][64]; // Group Member Names
/*0388*/ char     leader[64];                         // Group leader Name
/*0452*/ char     unknown0452[316];                   // ***Placeholder
/*0768*/
};

/*
** Grouping Invite
** Length 128 Octets
** Opcode OP_GroupInvite
*/

struct groupInviteStruct
{
/*0000*/ char     invitee[64];           // Invitee's Name
/*0064*/ char     inviter[64];           // Inviter's Name
/*0128*/
};

/*
** Grouping Invite Answer - Decline
** Length 129 Octets
** Opcode GroupDeclineCode
*/

struct groupDeclineStruct
{
/*0000*/ char     yourname[64];           // Player Name
/*0064*/ char     membername[64];         // Invited Member Name
/*0128*/ uint8_t  reason;                 // Already in Group = 1, Declined Invite = 3
/*0129*/
};

/*
** Grouping Invite Answer - Accept 
** Length 128 Octets
** Opcode OP_GroupFollow
*/

struct groupFollowStruct
{
/*0000*/ char     inviter[64];           // Inviter's Name
/*0064*/ char     invitee[64];           // Invitee's Member Name
/*0128*/
};

/*
** Group Disbanding
** Length 128 Octets
** Opcode 
*/

struct groupDisbandStruct
{
/*0000*/ char     yourname[64];           // Player Name
/*0064*/ char     membername[64];         // Invited Member Name
/*0128*/
};


/*
** Delete Spawn
** Length: 4 Octets
** OpCode: DeleteSpawnCode
*/

struct deleteSpawnStruct
{
/*0000*/ uint32_t spawnId;                // Spawn ID to delete
};

/*
** Remove Drop Item On Ground
** Length: 8 Octets
** OpCode: RemDropCode
*/

struct remDropStruct
{
/*0000*/ uint16_t dropId;                 // DropID - Guess
/*0002*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0004*/ uint16_t spawnId;                // Pickup ID - Guess
/*0006*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** Consider Struct
** Length: 28 Octets
** OpCode: considerCode
*/

struct considerStruct
{
/*0000*/ uint32_t playerid;               // PlayerID
/*0004*/ uint32_t targetid;               // TargetID
/*0008*/ int32_t  faction;                // Faction
/*0012*/ int32_t  level;                  // Level
/*0016*/ int32_t  curHp;                  // Current Hitpoints
/*0020*/ int32_t  maxHp;                  // Maximum Hitpoints
/*0024*/ int8_t   pvpCon;                 // Pvp con flag 0/1
/*0025*/ int8_t   unknown0025[3];         // unknown
/*0028*/
};

/*
** Spell Casted On
** Length: 36 Octets
** OpCode: castOnCode
*/

struct castOnStruct
{
/*0000*/ uint16_t targetId;               // Target ID
/*0002*/ uint8_t  unknown0002[2];         // ***Placeholder
/*0004*/ int16_t  sourceId;                // ***Source ID
/*0006*/ uint8_t  unknown0006[2];         // ***Placeholder
/*0008*/ uint8_t  unknown0008[24];        // might be some spell info?
/*0032*/ uint16_t spellId;                // Spell Id
/*0034*/ uint8_t  unknown0034[2];         // ***Placeholder
};

/*
** Spawn Death Blow
** Length: 32 Octets
** OpCode: NewCorpseCode
*/

struct newCorpseStruct
{
/*0000*/ uint32_t spawnId;                // Id of spawn that died
/*0004*/ uint32_t killerId;               // Killer
/*0008*/ uint32_t corpseid;               // corpses id
/*0012*/ int32_t  type;                   // corpse type?  
/*0016*/ uint32_t spellId;                // ID of Spell
/*0020*/ uint16_t zoneId;                 // Bind zone id
/*0022*/ uint16_t zoneInstance;           // Bind zone instance
/*0024*/ uint32_t damage;                 // Damage
/*0028*/ uint8_t  unknown0028[4];         // ***Placeholder
/*0032*/
};

/**
** Environmental damage (lava, falls)
** Length: 31 Octets
*/

struct environmentDamageStruct
{
/*0000*/ uint32_t spawnId;          // Who is taking the damage
/*0004*/ uint8_t unknown0004[2];
/*0006*/ uint32_t damage;           // how much damage?
/*0010*/ uint8_t unknown0010[12];
/*0022*/ uint8_t type;              // Damage type. FC = fall. FA = lava.
/*0023*/ uint8_t unknown0023[8];
};

/*
** Money Loot
** Length: 20 Octets
** OpCode: MoneyOnCorpseCode
*/

struct moneyOnCorpseStruct
{
/*0000*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0004*/ uint32_t platinum;               // Platinum Pieces
/*0008*/ uint32_t gold;                   // Gold Pieces
/*0012*/ uint32_t silver;                 // Silver Pieces
/*0016*/ uint32_t copper;                 // Copper Pieces
/*0020*/
};

/*
** Stamina
** Length: 8 Octets
** OpCode: staminaCode
*/

struct staminaStruct 
{
/*0000*/ uint32_t food;                     // Hunger, in ticks till next eat
/*0004*/ uint32_t water;                    // Thirst, in ticks till next eat
/*0008*/
};

/*
** Battle Code
** Length: 23 Octets
** OpCode: ActionCode
*/

// This can be used to gather info on spells cast on us
struct action2Struct
{
/*0000*/ uint16_t target;               // Target ID
/*0002*/ uint16_t source;               // Source ID
/*0004*/ uint8_t  type;                 // Bash, kick, cast, etc.
/*0005*/ int16_t  spell;                // SpellID
/*0007*/ int32_t  damage;
/*0011*/ uint8_t  unknown0011[12];  // ***Placeholder
/*0023*/
};

// This can be used to gather info on spells cast on us
struct actionStruct
{
/*0000*/ uint16_t target;                 // Target ID
/*0002*/ uint16_t source;                 // SourceID
/*0004*/ uint8_t  level;                  // Caster level
/*0005*/ uint8_t  unknown0005[17];        // ***Placeholder
/*0022*/ uint8_t  type;                   // Casts, Falls, Bashes, etc...
/*0023*/ int32_t  damage;                 // Amount of Damage
/*0027*/ int16_t  spell;                  // SpellID
/*0029*/ uint8_t  unknown0029[2];         // ***Placeholder
/*0031*/
};

/*
** client changes target struct
** Length: 4 Octets
** OpCode: clientTargetCode
*/

struct clientTargetStruct
{
/*0000*/ uint32_t newTarget;              // Target ID
/*0004*/ 
};

/*
** Info sent when you start to cast a spell
** Length: 20 Octets
** OpCode: StartCastCode
*/

struct startCastStruct 
{
/*0000*/ int32_t  slot;                   // ***Placeholder
/*0004*/ uint32_t spellId;                // Spell ID
/*0008*/ int32_t  inventorySlot;          // ***Placeholder
/*0012*/ uint32_t targetId;               // The current selected target
/*0016*/ uint8_t  unknown0018[4];         // ***Placeholder 
/*0020*/
};

/*
** New Mana Amount
** Length: 16 Octets
** OpCode: manaDecrementCode
*/

struct manaDecrementStruct
{
/*0000*/ int32_t newMana;                  // New Mana AMount
/*0004*/ int32_t unknown;
/*0008*/ int32_t spellId;                  // Last Spell Cast
/*0012*/ uint8_t unknown0012[4];
/*0016*/
};

/*
** Special Message
** Length: 4 Octets + Variable Text Length
** OpCode: SPMesgCode
*/
struct spMesgStruct
{
/*0000*/ int32_t msgType;                 // Type of message
/*0004*/ char    message[0];              // Message, followed by four Octets?
};

/*
** Spell Fade Struct
** Length: 10 Octets
** OpCode: SpellFadedCode
*/
struct spellFadedStruct
{
/*0000*/ uint32_t color;                  // color of the spell fade message
/*0004*/ char     message[0];             // fade message
/*0???*/ uint8_t  paddingXXX[3];          // always 0's 
};

/*
** Spell Action Struct
** Length: 8 Octets
** OpCode: BeginCastCode
*/
struct beginCastStruct
{
/*0000*/ uint16_t spawnId;                // Id of who is casting
/*0002*/ uint16_t spellId;                // Id of spell
/*0004*/ int16_t  param1;                 // Paramater 1
/*0006*/ int16_t  param2;                 // Paramater 2
/*0008*/
};

/*
** Spell Action Struct
** Length: 16 Octets
** OpCode: MemSpellCode
*/

struct memSpellStruct
{
/*0000*/ uint32_t slotId;                 // Slot spell is being memorized in
/*0004*/ uint32_t spellId;                // Id of spell
/*0008*/ int16_t  param1;                 // Paramater 1
/*0010*/ int16_t  param2;                 // Paramater 2
/*0012*/ uint8_t  unknown0012[4];         // *** Placeholder
};

/*
** Train Skill
** Length: 12 Octets
** OpCode: SkillTrainCode
*/

struct skillTrainStruct
{
/*0000*/ int32_t  playerid;               // player doing the training
/*0004*/ int32_t  type;                   // type of training?
/*0008*/ uint32_t skillId;                // Id of skill
/*0012*/
};

/*
** Skill Increment
** Length: 8 Octets
** OpCode: SkillIncCode
*/

struct skillIncStruct
{
/*0000*/ uint32_t skillId;                // Id of skill
/*0004*/ int32_t  value;                  // New value of skill
/*0008*/
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 14 Octets
** Opcode: WearChangeCode
*/

// ZBTEMP: Find newItemID
struct wearChangeStruct
{
/*0000*/ uint16_t spawnId;                // SpawnID
/*0002*/ Color_Struct color;              // item color
/*0006*/ uint8_t  wearSlotId;             // Slot ID
/*0007*/ uint8_t  unknown0005[7];         // unknown
/*0014*/
};

/*
** Level Update
** Length: 12 Octets
** OpCode: LevelUpUpdateCode
*/

struct levelUpUpdateStruct
{
/*0000*/ uint32_t level;                  // New level
/*0004*/ uint32_t levelOld;               // Old level
/*0008*/ uint32_t exp;                    // Current Experience
/*0012*/
};

/*
** Experience Update
** Length: 8 Octets
** OpCode: ExpUpdateCode
*/

struct expUpdateStruct
{
/*0000*/ uint32_t exp;                    // experience value  x/330
/*0004*/ uint32_t type;                   // 0=set, 2=update
/*0008*/
};

/*
** Alternate Experience Update
** Length: 12 Octets
** OpCode: AltExpUpdateCode
*/
struct altExpUpdateStruct
{
/*0000*/ uint32_t altexp;                 // alt exp x/330
/*0004*/ uint32_t aapoints;               // current number of AA points
/*0008*/ uint8_t  percent;                // percentage in integer form
/*0009*/ uint8_t  unknown0009[3];            // ***Place Holder
/*0012*/
};

/*
** Player Spawn Update
** Length: 9 Octets
** OpCode: SpawnUpdateCode
*/

struct SpawnUpdateStruct
{
/*0000*/ uint16_t spawnId;                // Id of spawn to update
/*0002*/ uint16_t subcommand;             // some sort of subcommand type
/*0004*/ int16_t  arg1;                   // first option
/*0006*/ int16_t  arg2;                   // second option
/*0008*/ uint8_t  arg3;                   // third option?
/*0009*/
};

/*
** NPC Hp Update
** Length: 10 Octets
** Opcode NpcHpUpdateCode
*/

struct hpNpcUpdateStruct
{
/*0000*/ int32_t curHP;
/*0004*/ int32_t maxHP;
/*0008*/ uint16_t spawnId;
/*0010*/ 
}; 

/*
** Inspecting Information
** Length: 1792 Octets
** OpCode: InspectDataCode
*/

struct inspectDataStruct
{
/*0000*/ uint8_t  unknown0002[72];        // ***Placeholder
/*0068*/ char     itemNames[21][64];      // 21 items with names 
                                          //    64 characters long.
/*1416*/ uint8_t  unknown1416[46];         // ***placeholder
/*1462*/ int16_t  icons[21];              // Icon Information
/*1504*/ char     mytext[200];            // Player Defined Text Info
/*1704*/ uint8_t  unknown0958[88];        // ***Placeholder
/*1792*/
};

/*
** Reading Book Information
** Length: Variable Length Octets
** OpCode: BookTextCode
*/

struct bookTextStruct
{
/*0000*/ uint16_t unknown0000;
/*0002*/ char     text[0];                  // Text of item reading
};

/*
** Interrupt Casting
** Length: 6 Octets + Variable Length Octets
** Opcode: BadCastCode
*/

struct badCastStruct
{
/*0000*/ uint32_t spawnId;                  // Id of who is casting
/*0004*/ char     message[0];               // Text Message
};

/*
** Random Number Request
** Length: 8 Octets
** OpCode: RandomCode
*/
struct randomReqStruct 
{
/*0000*/ uint32_t bottom;                 // Low number
/*0004*/ uint32_t top;                    // High number
};

/*
** Random Number Result
** Length: 76 Octets
** OpCode: RandomCode
*/
struct randomStruct 
{
/*0000*/ uint32_t bottom;                 // Low number
/*0004*/ uint32_t top;                    // High number
/*0008*/ uint32_t result;                 // result number
/*0012*/ char     name[64];               // name rolled by
/*0076*/
};

/*
** Player Position Update
** Length: 22 Octets
** OpCode: PlayerPosCode
*/

struct playerSpawnPosStruct
{
/*0000*/ uint16_t spawnId;          // spawn id of the thing moving
/*0002*/ signed   deltaHeading:10;  // change in heading
         signed   x:19;             // x coord
         signed   padding0002:3;    // ***Placeholder
/*0006*/ signed   y:19;             // y coord
         signed   animation:10;     // animation
         signed   padding0006:3;    // ***Placeholder
/*0010*/ signed   z:19;             // z coord
         signed   deltaY:13;        // change in y
/*0014*/ signed   deltaX:13;        // change in x
         unsigned heading:12;       // heading
         signed   padding0014:7;    // ***Placeholder
/*0018*/ signed   deltaZ:13;        // change in z
         signed   padding0018:19;   // ***Placeholder
/*0022*/
};

/*
** Self Position Update
** Length: 36 Octets
** OpCode: PlayerPosCode
*/

struct playerSelfPosStruct
{
/*0000*/ uint16_t spawnId;        // Player's spawn id
/*0002*/ uint8_t unknown0002[2];  // ***Placeholder (update time counter?)
/*0004*/ float y;                 // y coord
/*0008*/ float deltaZ;            // Change in z
/*0016*/ float deltaX;            // Change in x
/*0012*/ float deltaY;            // Change in y
/*0020*/ signed animation:10;     // animation
         signed deltaHeading:10;  // change in heading
         signed padding0020:12;   // ***Placeholder (mostly 1)
/*0024*/ float x;                 // x coord
/*0028*/ float z;                 // z coord
/*0034*/ unsigned heading:12;     // Directional heading
         unsigned padding0004:4;  // ***Placeholder
/*0032*/ uint8_t unknown0006[2];  // ***Placeholder
/*0036*/
};


/*
** Spawn Appearance
** Length: 8 Octets
** OpCode: spawnAppearanceCode
*/

struct spawnAppearanceStruct
{
/*0000*/ uint16_t spawnId;                // ID of the spawn
/*0002*/ uint16_t type;                   // Type of data sent
/*0004*/ uint32_t parameter;              // Values associated with the type
/*0008*/
};


/*
**               Structures that are not being currently used
 *               (except for logging)
*/

struct bindWoundStruct
{
/*0000*/ uint16_t playerid;             // TargetID
/*0002*/ uint8_t  unknown0002[2];       // ***Placeholder
/*0004*/ uint32_t hpmaybe;              // Hitpoints -- Guess
/*0008*/
};

struct inspectedStruct
{
/*0000*/ uint16_t inspectorid;         // Source ID
/*0002*/ uint8_t  unknown0002[2];      // ***Placeholder
/*0004*/ uint16_t inspectedid;         // Target ID - Should be you
/*0006*/ uint8_t  unknown0006[2];      // ***Placeholder
/*0008*/
};

struct attack1Struct
{
/*0000*/ uint16_t spawnId;                // Spawn ID
/*0002*/ int16_t  param1;                 // ***Placeholder
/*0004*/ int16_t  param2;                 // ***Placeholder
/*0006*/ int16_t  param3;                 // ***Placeholder
/*0008*/ int16_t  param4;                 // ***Placeholder
/*0010*/ int16_t  param5;                 // ***Placeholder
/*0012*/
};

struct attack2Struct
{
/*0000*/ uint16_t spawnId;                // Spawn ID
/*0002*/ int16_t  param1;                 // ***Placeholder
/*0004*/ int16_t  param2;                 // ***Placeholder
/*0006*/ int16_t  param3;                 // ***Placeholder
/*0008*/ int16_t  param4;                 // ***Placeholder
/*0010*/ int16_t  param5;                 // ***Placeholder
/*0012*/
};

struct newGuildInZoneStruct
{
/*0000*/ uint8_t  unknown0000[8];         // ***Placeholder
/*0008*/ char     guildname[56];          // Guildname
/*0064*/
};

struct moneyUpdateStruct
{
/*0000*/ uint32_t spawnid;            // ***Placeholder
/*0004*/ uint32_t cointype;           // Coin Type
/*0008*/ uint32_t amount;             // Amount
/*0012*/
};

/* Memorize slot operations, mem, forget, etc */

struct memorizeSlotStruct
{
/*0000*/ uint32_t slot;                     // Memorization slot (0-7)
/*0004*/ uint32_t spellId;                  // Id of spell 
                                            // (offset of spell in spdat.eff)
/*0008*/ uint32_t action;                   // 1-memming,0-scribing,2-forget
/*0012*/
};

struct cRunToggleStruct
{
/*0000*/ uint32_t status;                   //01=run  00=walk
};

struct cChatFiltersStruct
{
/*0000*/ uint32_t DamageShields;   //00=on  01=off
/*0004*/ uint32_t NPCSpells;       //00=on  01=off
/*0008*/ uint32_t PCSpells;        //00=all 01=off 02=grp
/*0012*/ uint32_t BardSongs;       //00=all 01=me  02=grp 03=off
/*0016*/ uint32_t Unused;
/*0020*/ uint32_t GuildChat;       //00=off 01=on
/*0024*/ uint32_t Socials;         //00=off 01=on
/*0028*/ uint32_t GroupChat;       //00=off 01=on
/*0032*/ uint32_t Shouts;          //00=off 01=on
/*0036*/ uint32_t Auctions;        //00=off 01=on
/*0040*/ uint32_t OOC;             //00=off 01=on
/*0044*/ uint32_t MyMisses;        //00=off 01=on
/*0048*/ uint32_t OthersMisses;    //00=off 01=on
/*0052*/ uint32_t OthersHits;      //00=off 01=on
/*0056*/ uint32_t AttackerMisses;  //00=off 01=on
/*0060*/ uint32_t CriticalSpells;  //00=all 01=me  02=off
/*0064*/ uint32_t CriticalMelee;   //00=all 01=me  02=off
/*0068*/
};

struct cOpenSpellBookStruct
{
/*0000*/ int32_t status; //01=open 00=close
/*0004*/
};

struct tradeSpellBookSlotsStruct
{
/*0000*/ uint32_t slot1;
/*0004*/ uint32_t slot2;
/*0008*/
};


/*
** serverLFGStruct
** Length: 10 Octets
** signifies LFG, maybe afk, role, ld, etc
*/

struct serverLFGStruct
{
/*0000*/ uint16_t spawnID;
/*0002*/ uint16_t unknown0004;
/*0004*/ uint16_t LFG;             //1=LFG
/*0006*/ uint16_t unknown0008;
/*0008*/
};

/*
** clientLFGStruct
** Length: 70 Octets
** signifies LFG, maybe afk, role, ld, etc
*/

struct clientLFGStruct
{
/*0000*/ uint8_t  name[64];
/*0064*/ uint16_t LFG;             //1=LFG
/*0066*/ uint16_t unknown0008;
};

/*
** buffStruct
** Length: 32 Octets
** 
*/

struct buffStruct
{
/*0000*/ uint32_t spawnid;        //spawn id
/*0004*/ uint8_t  unknown0004[4]; 
/*0008*/ uint32_t spellid;        // spellid
/*0012*/ uint32_t duration;       // duration
/*0016*/ uint8_t  unknown0012[4];
/*0020*/ uint32_t playerId;       // Player id who cast the buff
/*0024*/ uint32_t spellslot;      // spellslot
/*0028*/ uint32_t changetype;     // 1=buff fading,2=buff duration
/*0032*/
};

/*
** Guild Member Update structure 
** Length: 76 octets
**
*/

struct GuildMemberUpdate
{
/*000*/ uint32_t guildId;       // guild id
/*004*/ char     name[64];      // member name
/*068*/ uint16_t zoneId;        // zone id 
/*070*/ uint16_t zoneInstance;  // zone instance
/*072*/ uint32_t lastOn;        // time the player was last on.
/*076*/
};

/*
** Bazaar trader on/off struct
** Length: 8 Octets
**
*/
struct bazaarTraderRequest
{
/*000*/ uint32_t spawnId;       // Spawn id of person turning trader on/off
/*004*/ uint8_t mode;           // 0=off, 1=on
/*005*/ uint8_t uknown005[3];   // 
/*008*/
};

struct bazaarSearchQueryStruct 
{
  uint32_t mark;
  uint32_t type;
  char unknownXXX0[20]; // Value seems to always be the same
  char searchstring[64];
  uint32_t unknownXXX1;
  uint32_t unknownXXX2;
};

struct bazaarSearchResponseStruct 
{
  uint32_t mark;
  uint32_t count;
  uint32_t item_id;
  uint32_t player_id;
  uint32_t price;
  uint32_t status; // XXX Still poorly understood. 0=simple search
  char item_name[64]; // nul-padded name with appended "(count)"
};

/*
** Item Bazaar Search Result
** Length: Variable
** OpCode: BazaarSearch
*/
union bazaarSearchStruct
{
  uint32_t mark;
  struct bazaarSearchQueryStruct query;
  struct bazaarSearchResponseStruct response[];
};

/*******************************/
/* World Server Structs        */

/*
** Guild List (from world server)
** Length: 96 Octets
** used in: worldGuildList
*/

struct guildListStruct
{
/*0000*/ char     guildName[64];
};

/*
** Guild List (from world server)
** Length: 96064 Octets
*/
struct worldGuildListStruct
{
/*000*/ guildListStruct dummy;
/*064*/ guildListStruct guilds[MAX_GUILDS];
};

struct worldMOTDStruct
{
  /*002*/ char    message[0];
  /*???*/ uint8_t unknownXXX[3];
};

// Restore structure packing to default
#pragma pack()

#endif // EQSTRUCT_H

//. .7...6....,X....D4.M.\.....P.v..>..W....
//123456789012345678901234567890123456789012
//000000000111111111122222222223333333333444
