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
#define SPAWN_NPC_CORPSE                2
#define SPAWN_PC_CORPSE                 3
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
#define MAX_AA                          239
#define MAX_BANDOLIERS                  4
#define MAX_POTIONS_IN_BELT             4
#define MAX_TRIBUTES                    5
#define MAX_DISCIPLINES                 50
#define DECRYPTION_KEY_SIZE             11

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
#define COMBAT_INVULNERABLE				-5
#define COMBAT_SHIELD_ABSORB			-6

#define PLAYER_CLASSES     16
#define PLAYER_RACES       15

#define BASE_ZEM_VALUE 0.30

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

/**
 * Recast timer types. Used as an off set to charProfileStruct timers.
 */
enum RecastTypes
{
  RecastTimer0 = 0,
  RecastTimer1,
  WeaponHealClickTimer, // 2
  MuramiteBaneNukeClickTimer, // 3
  RecastTimer4,
  DispellClickTimer, // 5 (also click heal orbs?)
  EpicTimer, // 6
  OoWBPClickTimer, // 7
  VishQuestClassItemTimer, // 8
  HealPotionTimer, // 9
  RecastTimer10,
  RecastTimer11,
  RecastTimer12,
  RecastTimer13,
  RecastTimer14,
  RecastTimer15,
  RecastTimer16,
  RecastTimer17,
  RecastTimer18,
  ModRodTimer, // 19
  MAX_RECAST_TYPES // 20
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
/*0003*/uint8_t use_tint; // if there is a tint, this is 0xff
    } rgb;
/*0000*/uint32_t color;
  };
};

struct Color_Profile_Struct
{
    union
    {
        struct
        {
            /*00*/ Color_Struct color_helmet;    // Color of helmet item
            /*04*/ Color_Struct color_chest;     // Color of chest item
            /*08*/ Color_Struct color_arms;      // Color of arms item
            /*12*/ Color_Struct color_bracers;   // Color of bracers item
            /*16*/ Color_Struct color_hands;     // Color of hands item
            /*20*/ Color_Struct color_legs;      // Color of legs item
            /*24*/ Color_Struct color_feet;      // Color of feet item
            /*28*/ Color_Struct color_primary;   // Color of primary item
            /*32*/ Color_Struct color_secondary; // Color of secondary item
        } equipment_colors;
        /*00*/ Color_Struct colors[9];
    };
/*36*/
};

struct Material_Profile_Struct
{
    union
    {
        struct
        {
            /*00*/ uint32_t material_helmet;    // material of helmet item
            /*04*/ uint32_t material_chest;     // material of chest item
            /*08*/ uint32_t material_arms;      // material of arms item
            /*12*/ uint32_t material_bracers;   // material of bracers item
            /*16*/ uint32_t material_hands;     // material of hands item
            /*20*/ uint32_t material_legs;      // material of legs item
            /*24*/ uint32_t material_feet;      // material of feet item
            /*28*/ uint32_t material_primary;   // material of primary item
            /*32*/ uint32_t material_secondary; // material of secondary item
        } equipment_materials;
        /*00*/ uint32_t materials[9];
    };
/*36*/
};

struct Equip_Struct
{
/*00*/  uint32_t        material;
/*04*/  Color_Struct    color;
/*08*/
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
/*0012*/  int16_t     dmg_shield_remaining; // remaining amount of dmg absorb on runes
/*0013*/  int8_t      persistant_buff;      // prolly not real, used for perm illusions
/*0014*/  int8_t      reserved;             // proll not real, reserved will use for something else later
/*0016*/  uint32_t    playerId;             // Global id of caster (for wear off)
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

struct FaceChange_Struct
{
/*000*/ uint8_t haircolor;
/*001*/ uint8_t beardcolor;
/*002*/ uint8_t eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
/*003*/ uint8_t eyecolor2;
/*004*/ uint8_t hairstyle;
/*005*/ uint8_t beard;	// vesuvias
/*006*/ uint8_t face;
/*007*/
//vesuvias:
//there are only 10 faces for barbs changing woad just
//increase the face value by ten so if there were 8 woad
//designs then there would be 80 barb faces
};

struct FaceChangeResponse_Struct
{
/*000*/ uint8_t haircolor;
/*001*/ uint8_t beardcolor;
/*002*/ uint8_t eyecolor1;
/*003*/ uint8_t eyecolor2;
/*004*/ uint8_t hairstyle;
/*005*/ uint8_t beard;
/*006*/ uint8_t face;
/*007*/ uint8_t unknown007[12]; // probably has spawn id
/*019*/
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
/*0068*/ float    y;
/*0072*/ float    x;
/*0076*/ float    z;
/*0080*/ uint32_t zone_reason;  //0x0A == death
/*0084*/ int32_t  success; // =0 client->server, =1 server->client, -X=specific error
                           // eqemu's common/eq_constants.h has error codes
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
** Length: 19592 Octets
** OpCode: CharProfileCode
*/
struct charProfileStruct
{
/*00000*/ uint32_t  checksum;           //
/*00004*/ uint32_t  gender;             // Player Gender - 0 Male, 1 Female
/*00008*/ uint32_t  race;               // Player race
/*00012*/ uint32_t  class_;             // Player class
/*00016*/ uint32_t  unknown00016;       // ***Placeholder
/*00020*/ uint8_t   level;              // Level of player
/*00021*/ uint8_t   level1;             // Level of player (again?)
/*00022*/ uint8_t   unknown00022[2];    // ***Placeholder
/*00024*/ BindStruct binds[5];          // Bind points (primary is first)
/*00124*/ uint32_t  deity;              // deity
/*00128*/ uint32_t  intoxication;       // Alcohol level (in ticks till sober?)
/*00132*/ uint32_t  spellSlotRefresh[MAX_SPELL_SLOTS]; // Refresh time (millis)
/*00168*/ uint8_t unknown0166[4];
/*00172*/ uint8_t   haircolor;          // Player hair color
/*00173*/ uint8_t   beardcolor;         // Player beard color
/*00174*/ uint8_t   eyecolor1;          // Player left eye color
/*00175*/ uint8_t   eyecolor2;          // Player right eye color
/*00176*/ uint8_t   hairstyle;          // Player hair style
/*00177*/ uint8_t   beard;              // Player beard type
/*00178*/ uint8_t unknown00178[10];
/*00188*/ uint32_t  item_material[9];   // Item texture/material of worn items
/*00224*/ uint8_t unknown00224[48];
/*00272*/ Color_Struct item_tint[9];    // RR GG BB 00
/*00308*/ AA_Array  aa_array[MAX_AA];   // AAs
/*02220*/ uint8_t unknown02220[4];
/*02224*/ uint32_t  points;             // Unspent Practice points
/*02228*/ uint32_t  MANA;               // Current MANA
/*02232*/ uint32_t  curHp;              // Current HP without +HP equipment
/*02236*/ uint32_t  STR;                // Strength
/*02240*/ uint32_t  STA;                // Stamina
/*02244*/ uint32_t  CHA;                // Charisma
/*02248*/ uint32_t  DEX;                // Dexterity
/*02252*/ uint32_t  INT;                // Intelligence
/*02256*/ uint32_t  AGI;                // Agility
/*02260*/ uint32_t  WIS;                // Wisdom
/*02264*/ uint8_t   face;               // Player face
/*02265*/ uint8_t unknown02264[47];
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
/*04760*/ uint8_t unknown04760[236];
/*04996*/ uint32_t  toxicity;           // Potion Toxicity (15=too toxic, each potion adds 3)
/*05000*/ uint32_t  thirst;             // Drink (ticks till next drink)
/*05004*/ uint32_t  hunger;             // Food (ticks till next eat)
/*05008*/ spellBuff buffs[MAX_BUFFS];   // Buffs currently on the player
/*05508*/ uint32_t  disciplines[MAX_DISCIPLINES]; // Known disciplines
/*05708*/ uint8_t unknown05008[360];
/*06068*/ uint32_t recastTimers[MAX_RECAST_TYPES]; // Timers (GMT of last use)
/*06148*/ uint32_t  endurance;          // Current endurance
/*06152*/ uint32_t  aa_spent;           // Number of spent AA points
/*06156*/ uint32_t  aa_unspent;         // Unspent AA points
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
/*13052*/ uint8_t   pvp;                // 1=pvp, 0=not pvp
/*13053*/ uint8_t   anon;               // 2=roleplay, 1=anon, 0=not anon     
/*13054*/ uint8_t   gm;                 // 0=no, 1=yes (guessing!)
/*13055*/ int8_t    guildstatus;        // 0=member, 1=officer, 2=guildleader
/*13056*/ uint8_t unknown13054[12];
/*13068*/ uint32_t  exp;                // Current Experience
/*13072*/ uint8_t unknown13072[12];
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
/*13244*/ uint8_t unknown13244[12];
/*13256*/ uint32_t  autosplit;          // 0 = off, 1 = on
/*13260*/ uint8_t unknown13260[16];
/*13276*/ uint16_t  zoneId;             // see zones.h
/*13278*/ uint16_t  zoneInstance;       // Instance id
/*13280*/ char      groupMembers[MAX_GROUP_MEMBERS][64];// all the members in group, including self 
/*13664*/ char      groupLeader[64];    // Leader of the group ?
/*13728*/ uint8_t unknown13728[660];
/*14388*/ uint32_t  leadAAActive;       // 0 = leader AA off, 1 = leader AA on
/*14392*/ uint8_t unknown14392[4];
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
/*14616*/ uint8_t unknown14616[8];
/*14624*/ uint32_t  expGroupLeadAA;     // Current group lead exp points (format though??)
/*14628*/ uint32_t unknown14628;
/*14632*/ uint32_t  expRaidLeadAA;      // Current raid lead AA exp points (format though??)
/*14636*/ uint32_t  groupLeadAAUnspent; // Unspent group lead AA points
/*14640*/ uint32_t  raidLeadAAUnspent;  // Unspent raid lead AA points
/*14644*/ uint32_t  leadershipAAs[MAX_LEAD_AA]; // Leader AA ranks
/*14772*/ uint8_t unknown14772[128];
/*14900*/ uint32_t  airRemaining;       // Air supply (seconds)
/*14904*/ uint8_t unknown14904[4608];
/*19512*/ uint32_t expAA;               // Exp earned in current AA point
/*19516*/ uint8_t unknown19516[40];
/*19556*/ uint32_t currentRadCrystals;  // Current count of radiant crystals
/*19560*/ uint32_t careerRadCrystals;   // Total count of radiant crystals ever
/*19564*/ uint32_t currentEbonCrystals; // Current count of ebon crystals
/*19568*/ uint32_t careerEbonCrystals;  // Total count of ebon crystals ever
/*19572*/ uint8_t  groupAutoconsent;    // 0=off, 1=on
/*19573*/ uint8_t  raidAutoconsent;     // 0=off, 1=on
/*19574*/ uint8_t  guildAutoconsent;    // 0=off, 1=on
/*19575*/ uint8_t  unknown19575[5];     // ***Placeholder (6/29/2005)
/*19580*/ uint32_t showhelm;            // 0=no, 1=yes
/*19584*/ uint8_t  unknown19584[4];     // ***Placeholder (10/27/2005)
/*19588*/ uint32_t unknown19588;        // *** Placeholder
/*19584*/ 
};

#if 0
// The following seem to be totally gone from charProfileStruct (9/13/05)
/*2384*/ char      title[32];          // Current character title
/*2352*/ char      servername[32];     // server the char was created on
/*2416*/ char      suffix[32];         // Current character suffix
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
** Length: 385 Octets 
** Used in: 
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/ 
struct spawnStruct
{
/*0000*/ uint8_t unknown0000;
/*0001*/ uint8_t  gm;             // 0=no, 1=gm
/*0002*/ uint8_t unknown0003;
/*0003*/ int8_t   aa_title;       // 0=none, 1=general, 2=archtype, 3=class
/*0004*/ uint8_t unknown0004;
/*0005*/ uint8_t  anon;           // 0=normal, 1=anon, 2=roleplay
/*0006*/ uint8_t  face;	          // Face id for players
/*0007*/ char     name[64];       // Player's Name
/*0071*/ int16_t  deity;          // Player's Deity
/*0073*/ uint16_t unknown0073;
/*0075*/ float    size;           // Model size
/*0079*/ uint32_t unknown0079;
/*0083*/ uint8_t  NPC;            // 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*0084*/ uint8_t  invis;          // Invis (0=not, 1=invis)
/*0085*/ uint8_t  haircolor;      // Hair color
/*0086*/ uint8_t  curHp;          // Current hp %%% wrong
/*0087*/ uint8_t  max_hp;         // (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
/*0088*/ uint8_t  findable;       // 0=can't be found, 1=can be found
/*0089*/ uint8_t unknown0089[5];
/*0094*/ signed   deltaHeading:10;// change in heading
         signed   x:19;           // x coord
         signed   padding0054:3;  // ***Placeholder
/*0098*/ signed   y:19;           // y coord
         signed   animation:10;   // animation
         signed   padding0058:3;  // ***Placeholder
/*0102*/ signed   z:19;           // z coord
         signed   deltaY:13;      // change in y
/*0106*/ signed   deltaX:13;      // change in x
         unsigned heading:12;     // heading
         signed   padding0066:7;  // ***Placeholder
/*0110*/ signed   deltaZ:13;      // change in z
         signed   padding0070:19; // ***Placeholder
/*0114*/ uint8_t  eyecolor1;      // Player's left eye color
/*0115*/ uint8_t unknown0115[24];
/*0139*/ uint8_t  showhelm;       // 0=no, 1=yes
/*0140*/ uint8_t unknown0140[4];
/*0144*/ uint8_t  is_npc;         // 0=no, 1=yes
/*0145*/ uint8_t  hairstyle;      // Hair style
/*0146*/ uint8_t  beard;          // Beard style (not totally, sure but maybe!)
/*0147*/ uint8_t unknown0147[4];
/*0151*/ uint8_t  level;          // Spawn Level
/*0152*/ uint8_t  unknown0259[4]; // ***Placeholder
/*0156*/ uint8_t  beardcolor;     // Beard color
/*0157*/ char     suffix[32];     // Player's suffix (of Veeshan, etc.)
/*0189*/ uint32_t petOwnerId;     // If this is a pet, the spawn id of owner
/*0193*/ int8_t   guildrank;      // 0=normal, 1=officer, 2=leader
/*0194*/ uint8_t unknown0194[3];
/*0197*/ union 
         {
             struct 
             {
               /*0197*/ uint32_t equip_helmet;    // Equipment: Helmet Visual
               /*0201*/ uint32_t equip_chest;     // Equipment: Chest Visual
               /*0205*/ uint32_t equip_arms;      // Equipment: Arms Visual
               /*0209*/ uint32_t equip_bracers;   // Equipment: Bracers Visual
               /*0213*/ uint32_t equip_hands;     // Equipment: Hands Visual
               /*0217*/ uint32_t equip_legs;      // Equipment: Legs Visual
               /*0221*/ uint32_t equip_feet;      // Equipment: Feet Visual
               /*0225*/ uint32_t equip_primary;   // Equipment: Primary Visual
               /*0229*/ uint32_t equip_secondary; // Equipment: Secondary Visual
             } equip;
             /*0197*/ uint32_t equipment[9];  // Array elements correspond to struct equipment above
         };
/*0233*/ float    runspeed;       // Speed when running
/*0036*/ uint8_t  afk;            // 0=no, 1=afk
/*0238*/ uint32_t guildID;        // Current guild
/*0242*/ char     title[32];      // Title
/*0274*/ uint8_t unknown0274;
/*0275*/ uint8_t  set_to_0xFF[8]; // ***Placeholder (all ff)
/*0283*/ uint8_t  helm;           // Helm texture
/*0284*/ uint32_t race;           // Spawn race
/*0288*/ uint32_t unknown0288;
/*0292*/ char     lastName[32];   // Player's Lastname
/*0324*/ float    walkspeed;      // Speed when walking
/*0328*/ uint8_t unknown0328;
/*0329*/ uint8_t  is_pet;         // 0=no, 1=yes
/*0330*/ uint8_t  light;          // Spawn's lightsource %%% wrong
/*0331*/ uint8_t  class_;         // Player's class
/*0332*/ uint8_t  eyecolor2;      // Left eye color
/*0333*/ uint8_t unknown0333;
/*0334*/ uint8_t  gender;         // Gender (0=male, 1=female)
/*0335*/ uint8_t  bodytype;       // Bodytype
/*0336*/ uint8_t unknown0336[3];
union 
{
/*0339*/ int8_t equip_chest2;     // Second place in packet for chest texture (usually 0xFF in live packets)
                                  // Not sure why there are 2 of them, but it effects chest texture!
/*0339*/ int8_t mount_color;      // drogmor: 0=white, 1=black, 2=green, 3=red
                                  // horse: 0=brown, 1=white, 2=black, 3=tan
};
/*0340*/ uint32_t spawnId;        // Spawn Id
/*0344*/ uint8_t unknown0344[4];
/*0348*/ union 
         {
             struct 
             {
                 /*0348*/ Color_Struct color_helmet;    // Color of helmet item
                 /*0352*/ Color_Struct color_chest;     // Color of chest item
                 /*0356*/ Color_Struct color_arms;      // Color of arms item
                 /*0360*/ Color_Struct color_bracers;   // Color of bracers item
                 /*0364*/ Color_Struct color_hands;     // Color of hands item
                 /*0368*/ Color_Struct color_legs;      // Color of legs item
                 /*0372*/ Color_Struct color_feet;      // Color of feet item
                 /*0376*/ Color_Struct color_primary;   // Color of primary item
                 /*0380*/ Color_Struct color_secondary; // Color of secondary item
             } equipment_colors;
             /*0348*/ Color_Struct colors[9]; // Array elements correspond to struct equipment_colors above
         };
/*0384*/ uint8_t  lfg;            // 0=off, 1=lfg on
/*0385*/
};

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
/*0000*/ uint16_t   spawnId;
/*0002*/ int32_t    y:19,
                    z:19,
                    x:19,
                    u3:7;
         uint16_t   heading:12;
/*0010*/ uint8_t    unused2:4;  // Always f0
/*0012*/
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
** Adjust hp of a spawn
** Length: 3 Octets
** OpCode: MobHealth
*/
struct mobHealthStruct {
/*0000*/    uint16_t spawnId;           // spawn id of target
/*0002*/    uint8_t  hp;                // hp percentage
/*0003*/
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

struct NewMOTD_Struct
{
  /*0000*/ char motd[0]; // variable length
};

/*
** Guild MOTD Struct
** Length: Variable Text
** OPCode: OP_GuildMOTD
*/
struct GuildMOTD_Struct
{
/*0000*/	uint32_t	unknown0;
/*0004*/	char	name[64];
/*0068*/	char	setby_name[64];
/*0132*/	uint32_t	unknown132;
/*0136*/	char	motd[512];
/*0648*/
};

struct GuildCommand_Struct
{
/*0000*/    char othername[64];
/*0064*/    char myname[64];
/*0128*/    uint16_t guildeqid;
/*0130*/    uint8_t unknown[2]; // for guildinvite all 0's, for remove 0=0x56, 2=0x02
/*0132*/    uint32_t officer;
/*0136*/
};

struct GuildInviteAccept_Struct
{
	char inviter[64];
	char newmember[64];
	uint32_t response;
	uint32_t guildeqid;
};

struct NewGuildCreated_Struct
{
/*0000*/    uint32_t guildeqid;
/*0004*/    char name[64];
/*0068*/
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
/*0129*/ char corpseZoneName[32];   // Zone where the corpse is
/*0161*/
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
/*0452*/ char     unknown0452[320];                   // ***Placeholder
/*0772*/
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

struct Death_Struct
{
/*000*/ uint32_t    spawnId;
/*004*/ uint32_t    killerId;
/*008*/ uint32_t    corpseid;
/*012*/ uint32_t    unknown012; // NPC kills NPC = 1
                                // NPC kills PC = ?
                                // NPC kills player = 9
                                // player kills NPC = 1
                                // player kills PC = 93
                                // player kills player = ?
                                // PC kills ? = ?
                                // player drowns = 47
                                // player falls (pain and suffering) = 9
/*016*/ uint32_t    spellId;
/*020*/ uint32_t    type;   // attack type, 0xE7 (231) for spell
/*024*/ uint32_t    damage;
/*028*/ uint32_t    unknown028; // Always 0
};

struct BecomeCorpse_Struct
{
    uint32_t    spawn_id;
    float y;
    float x;
    float z;
};

struct ZonePlayerToBind_Struct
{
/*000*/ uint16_t bind_zone_id;
/*002*/ uint16_t bind_instance_id;
/*004*/ float x;
/*008*/ float y;
/*012*/ float z;
/*016*/ float heading;
/*020*/ char zone_name[1]; // variable length null-terminated string
};

struct GMSummon_Struct
{
/*000*/ char        charname[64];
/*064*/ char        gmname[64];
/*128*/ uint32_t    success;
/*132*/ uint32_t    zoneID;
/*136*/ float       y;
/*140*/ float       x;
/*144*/ float       z;
/*148*/ uint32_t    unknown2; // E0 E0 56 00
/*152*/
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

struct Surname_Struct
{
/*0000*/	char name[64];
/*0064*/	uint32_t unknown0064; // 0 for client request. 1 for server confirm
/*0068*/	char lastname[32];
/*0100*/
};

struct GMLastName_Struct
{
	char name[64];
	char gmname[64];
	char lastname[64];
	uint16_t unknown[4];	// 0x00, 0x00
					    // 0x01, 0x00 = Update the clients
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

// this is what prints the You have been struck. and the regular
// melee messages like You try to pierce, etc.  It's basically the melee
// and spell damage message
//
// Drowning: Involves 2 packets: first DIR=client, then DIR=server.
// Falling: Involves only 1 DIR=client packet.
// Melee: Involves only 1 DIR=server packet.
struct CombatDamage_Struct
{
/* 00 */	uint16_t	target;
/* 02 */	uint16_t	source;
/* 04 */	uint8_t	type; //slashing, etc.  231 (0xE7) for spells
/* 05 */	uint16_t	spell; // spell id
/* 07 */	int32_t	damage; // Seems to always be 0 for spells with player as source. negative for damage shields
/* 11 */	float force; // Amplitude of push
            // Also used as sequence tied to OP_Action
/* 15 */	union { float to_float; uint32_t to_uint32_t; } meleepush_xy;
/* 19 */	float meleepush_z;
/* 23 */
};

// This is what causes the caster to animate and the target to
// get the particle effects around them when a spell is cast.
// Also causes a buff icon when a second OP_Action is received
// with make_buff_icon set to 4.
struct actionStruct
{
/*0000*/ uint16_t target;                 // Target ID
/*0002*/ uint16_t source;                 // SourceID
/*0004*/ uint16_t  level;                  // Caster level
/*0006*/ uint16_t instrument_mod;
/*0008*/ uint32_t bard_focus_id;
/*0012*/ uint16_t unknown16;
// some kind of sequence that's the same in both actions
// as well as the combat damage, to tie em together?
/*0014*/ uint32_t sequence;
/*0018*/ uint32_t unknown18;
/*0022*/ uint8_t  type;                   // Casts (0xe7), Falls, Bashes, etc...
/*0023*/ int32_t  damage;                 // Amount of Damage
/*0027*/ int16_t  spell;                  // SpellID
/*0029*/ uint8_t  unknown0029;            // ***Placeholder
// this field seems to be some sort of success flag, if it's 4
/*0030*/ uint8_t  make_buff_icon;         // if 4, make buff icon
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

struct GMTrainee_Struct
{
/*000*/ uint32_t npcid;
/*004*/ uint32_t playerid;
/*008*/ uint32_t skills[100];
/*408*/ uint8_t unknown408[40];
/*448*/
};

struct GMSkillChange_Struct
{
/*000*/ uint16_t		npcid;
/*002*/ uint8_t		unknown1[2];    // something like PC_ID, but not really. stays the same thru the session though
/*004*/ uint16_t       skillbank;      // 0 if normal skills, 1 if languages
/*006*/ uint8_t		unknown2[2];
/*008*/ uint16_t		skill_id;
/*010*/ uint8_t		unknown3[2];
/*012*/
};

struct GMTrainEnd_Struct
{
/*000*/ uint32_t npcid;
/*004*/ uint32_t playerid;
/*008*/
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
/*0002*/ uint16_t materialId;             // Material ID
/*0004*/ Color_Struct color;              // item color
/*0008*/ uint8_t  wearSlotId;             // Slot ID
/*0009*/
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
/*0008*/ uint32_t exp;                    // bogus data?
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

/**
 * Leadership AA update
 * Length: 32 Octets
 * OpCode: LeadExpUpdate
 */
struct leadExpUpdateStruct
{
/*0000*/ uint32_t unknown0000;          // All zeroes?
/*0004*/ uint32_t groupLeadExp;         // Group leadership exp value
/*0008*/ uint32_t unspentGroupPoints;   // Unspent group points
/*0012*/ uint32_t unknown0012;          // Type?
/*0016*/ uint32_t unknown0016;          // All zeroes?
/*0020*/ uint32_t raidLeadExp;          // Raid leadership exp value
/*0024*/ uint32_t unspentRaidPoints;    // Unspent raid points
/*0028*/ uint32_t unknown0028;
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

struct Inspect_Struct
{
    uint32_t TargetID;
    uint32_t PlayerID;
};

// This is where the Text is sent to the client.
// Use ` as a newline character in the text.
struct BookText_Struct
{
    uint8_t window;      // where to display the text (0xFF means new window)
    uint8_t type;        //type: 0=scroll, 1=book, 2=item info.. prolly others.
    char    booktext[0]; // Variable Length
};
// This is the request to read a book.
// This is just a "text file" on the server
// or in our case, the 'name' column in our books table.
struct BookRequest_Struct
{
    uint8_t window;	// where to display the text (0xFF means new window)
    uint8_t type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
    char    txtfile[0];	// Variable length
};

struct QuestReward_Struct
{
/*000*/ uint32_t	mob_id;	// ID of mob awarding the client
/*004*/ uint32_t	target_id;
/*008*/ uint32_t	exp_reward;
/*012*/ uint32_t	faction;
/*016*/ int32_t		faction_mod;
/*020*/ uint32_t	copper;		// Gives copper to the client
/*024*/ uint32_t	silver;		// Gives silver to the client
/*028*/ uint32_t	gold;		// Gives gold to the client
/*032*/ uint32_t	platinum;	// Gives platinum to the client
/*036*/ uint32_t	item_id;
/*040*/ uint32_t	unknown040;
/*044*/ uint32_t	unknown044;
/*048*/ uint32_t	unknown048;
/*052*/ uint32_t	unknown052;
/*056*/ uint32_t	unknown056;
/*060*/ uint32_t	unknown060;
/*064*/ uint32_t	unknown064;
/*068*/
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

struct MoneyUpdate_Struct
{
    int32_t platinum;
    int32_t gold;
    int32_t silver;
    int32_t copper;
};

struct BankerChange_Struct
{
    uint32_t	platinum;
    uint32_t	gold;
    uint32_t	silver;
    uint32_t	copper;
    uint32_t	platinum_bank;
    uint32_t	gold_bank;
    uint32_t	silver_bank;
    uint32_t	copper_bank;
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

struct SetMeditateMode_Struct
{
    uint8_t mode; // bool as 0 or 1
};

struct List_Struct
{
    uint8_t op; // 1 = client->zone: when arg=0, info requested via /list
                //                   when arg=1, confirm add to list
                //     zone->client: arg indicates current queue length
                // 2 = zone->client: afk check request
                // 3 = client->zone: remove from list, occurs when client
                //                   receives an afk check request while
                //                   an afk check popup is active
    uint32_t arg;
};

struct SetRunMode_Struct
{
	uint8_t mode;
	uint8_t unknown[3];
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


struct LFG_Appearance_Struct
{
/*0000*/ uint32_t   spawn_id;		// ID of the client
/*0004*/ uint8_t    lfg;				// 1=LFG, 0=Not LFG
/*0005*/ char       unknown0005[3];	//
/*0008*/
};

struct LFG_Struct
{
/*000*/ uint32_t unknown000;
/*004*/ uint32_t value; // 0x00 = off 0x01 = on
/*008*/ uint32_t unknown008;
/*012*/ uint32_t unknown012;
/*016*/ char	name[64];
/*080*/
};

struct LFP_Struct
{
/*000*/ uint32_t	Unknown000;
/*004*/ uint8_t	Action;
/*005*/ uint8_t	MatchFilter;
/*006*/ uint16_t	Unknown006;
/*008*/ uint32_t	FromLevel;
/*012*/ uint32_t	ToLevel;
/*016*/ uint32_t	Classes;
/*020*/ char	Comments[64];
/*084*/
};

/*
** buffStruct
** Length: 32 Octets
** 
*/

struct buffStruct
{
/*0000*/ uint32_t spawnid;        //spawn id
/*0004*/ uint8_t  effect_type;    // 0 = no buff, 2 = buff, 4 = inverse affects of buff
/*0005*/ uint8_t  level;
/*0006*/ uint8_t  bard_modifier;
/*0007*/ uint8_t  unknown003;     // MQ2 used to call this "damage shield" -- don't see client referencing it, so maybe server side DS type tracking?
/*0008*/ uint32_t spellid;        // spellid
/*0012*/ uint32_t duration;       // duration
/*0016*/ uint32_t counters;       // single book keeping value (counters, rune/vie)
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
  struct bazaarSearchResponseStruct response[0];
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

struct LoginInfo_Struct
{
/*000*/ char    uid[7];
/*007*/ char    key[DECRYPTION_KEY_SIZE];
/*018*/ uint8_t unknownXYZ[446];
/*464*/
};

struct Who_All_Struct
{
/*000*/ char        whom[64];
/*064*/ uint32_t    wrace;      // FF FF = no race
/*068*/ uint32_t    wclass;     // FF FF = no class
/*072*/ uint32_t    lvllow;     // FF FF = no numbers
/*076*/ uint32_t    lvlhigh;    // FF FF = no numbers
/*080*/ uint32_t    gmlookup;   // FF FF = not doing /who all gm
/*084*/ uint32_t    guildid;
/*088*/ uint8_t     unknown076[64];
/*152*/
};

struct PetCommand_Struct
{
/*000*/ uint32_t    command;
/*004*/ uint32_t    target;
/*008*/
};

struct Charm_Struct
{
/*00*/  uint32_t    owner_id;
/*04*/  uint32_t    pet_id;
/*08*/  uint32_t    command;    // 1: make pet, 0: release pet
/*12*/
};

struct Animation_Struct
{
/*00*/  uint16_t    spawnid;
/*02*/  uint8_t     speed;
/*03*/  uint8_t     action;
/*04*/
};

struct PlayerState_Struct
{
/*00*/  uint32_t    spawn_id;
/*04*/  uint32_t    state;
};

struct Consume_Struct
{
/*0000*/ uint32_t   slot;
/*0004*/ uint32_t   auto_consumed; // 0xffffffff = auto-eating, e7030000 when right click
/*0008*/ uint8_t    c_unknown1[4];
/*0012*/ uint8_t    type; // 0x01=Food 0x02=Water
/*0013*/ uint8_t    unknown13[3];
};

struct MoveItem_Struct
{
/*0000*/ uint32_t   from_slot;
/*0004*/ uint32_t   to_slot; // 0xffffffff = delete
/*0008*/ uint32_t   number_in_stack;
/*0012*/
};

struct WhoAllPlayer
{
    uint32_t	formatstring;
    uint32_t	pidstring;
    char*	name;
    uint32_t	rankstring;
    char*	guild;
    uint32_t	unknown80[2];
    uint32_t	zonestring;
    uint32_t	zone;
    uint32_t	class_;
    uint32_t	level;
    uint32_t	race;
    char*	account;
    uint32_t	unknown100;
};

struct WhoAllReturnStruct
{
    uint32_t	id;
    uint32_t	playerineqstring;
    char	line[27];
    uint8_t	unknown35; //0A
    uint32_t	unknown36;//0s
    uint32_t	playersinzonestring;
    uint32_t	unknown44[2]; //0s
    uint32_t	unknown52;//1
    uint32_t	unknown56;//1
    uint32_t	playercount;//1
    struct WhoAllPlayer player[0];
};

struct AutoAttack_Struct
{
/*00*/    uint32_t    id; // 0 = off, 1 = on
};

struct WeaponEquip1_Struct
{
/*00*/  uint32_t    spawn_id;
/*04*/  uint32_t    unknown04; // 0s
/*08*/  uint32_t    state; // unequip=0xffffffff, equip=0
/*12*/
};

struct SetServerFilter_Struct
{
    uint32_t    filters[29]; //see enum eqFilterType in eqemu
};

struct AckPacket_Struct
{
    uint32_t    filters; // 0s?
};

struct SendTributes_Struct
{
    uint32_t    unknown00;
};

struct SendGuildTributes_Struct
{
    uint32_t    unknown00;
};

struct TributeTimer_Struct
{
    uint32_t    time_left;
};

struct TributeInfo_Struct
{
    uint32_t    active;		//0 == inactive, 1 == active
    uint32_t    tributes[MAX_TRIBUTES];	//-1 == NONE
    uint32_t    tiers[MAX_TRIBUTES];		//all 00's
    uint32_t    tribute_master_id;
};

struct Weather_Struct
{
    uint32_t    val1;	//generall 0x000000FF
    uint32_t    type;	//0x31=rain, 0x02=snow(i think), 0 = normal
    uint32_t    mode;
};

struct WeatherShort_Struct
{
    uint32_t    type;
    uint32_t    intensity;
};

struct Camp_Struct
{
    uint32_t    unknown00; // 0s?
};

struct LogServer_Struct
{
/*000*/ uint32_t	unknown000;
/*004*/ uint32_t	unknown004;
/*008*/ uint32_t	unknown008;
/*012*/ uint32_t	unknown012;	// htonl(1) on live
/*016*/ uint32_t	unknown016;	// htonl(1) on live
/*020*/ uint8_t	unknown020[12];
/*032*/ char	worldshortname[32];
/*064*/ uint8_t	unknown064[32];
/*096*/ char	unknown096[16];	// 'pacman' on live
/*112*/ char	unknown112[16];	// '64.37,148,36' on live
/*126*/ uint8_t	unknown128[48];
/*176*/ uint32_t	unknown176;	// htonl(0x00002695)
/*180*/ char	unknown180[80];	// 'eqdataexceptions@mail.station.sony.com' on live
/*260*/ uint8_t	unknown260;	// 0x01 on live
/*261*/ uint8_t	unknown261;	// 0x01 on live
/*262*/ uint8_t	unknown262[4];
/*266*/
};

struct ApproveWorld_Struct
{
    uint8_t unknown544[544];
};

struct EnterWorld_Struct
{
/*000*/ char        name[64];
/*064*/ uint32_t    tutorial;		// 01 on "Enter Tutorial", 00 if not
/*068*/ uint32_t    return_home;		// 01 on "Return Home", 00 if not
};

struct PostEnterWorld_Struct
{
    uint8_t unknown996[996];
};

struct ExpansionInfo_Struct
{
    uint8_t unknown46[46];
};

struct CharacterSelectEntry_Struct
{
/*0000*/    char Name[64];
/*0064*/    uint8_t Class;
/*0065*/    uint32_t Race;
/*0069*/    uint8_t Level;
/*0070*/    uint8_t ShroudClass;
/*0071*/    uint32_t ShroudRace;
/*0075*/    uint16_t Zone;
/*0077*/    uint16_t Instance;
/*0079*/    uint8_t Gender;
/*0080*/    uint8_t Face;
/*0081*/    Equip_Struct Equip[9];
/*0153*/    uint8_t Unknown15; // Seen FF
/*0154*/    uint8_t Unknown19; // Seen FF
/*0155*/    uint32_t DrakkinTattoo;
/*0159*/    uint32_t DrakkinDetails;
/*0163*/    uint32_t Deity;
/*0167*/    uint32_t PrimaryIDFile;
/*0171*/    uint32_t SecondaryIDFile;
/*0175*/    uint8_t HairColor;
/*0176*/    uint8_t BeardColor;
/*0177*/    uint8_t EyeColor1;
/*0178*/    uint8_t EyeColor2;
/*0179*/    uint8_t HairStyle;
/*0180*/    uint8_t Beard;
/*0181*/    uint8_t GoHome; // Seen 0 for new char and 1 for existing
/*0182*/    uint8_t Tutorial; // Seen 1 for new char or 0 for existing
/*0183*/    uint32_t DrakkinHeritage;
/*0187*/    uint8_t Unknown1; // Seen 0
/*0188*/    uint8_t Enabled; // Originally labeled as 'CharEnabled' - unknown purpose and setting
/*0189*/    uint32_t LastLogin;
/*0193*/    uint8_t Unknown2; // Seen 0
/*0194*/
};

struct CharacterSelect_Struct
{
/*0000*/    uint32_t Race[10];				// Characters Race
/*0040*/    Color_Profile_Struct colors[10]; // Characters Equipment Colors - packet requires length for 10 characters..but, client is limited to 8
/*0400*/    uint8_t BeardColor[10];			// Characters beard Color
/*0410*/    uint8_t HairStyle[10];			// Characters hair style
/*0420*/    Material_Profile_Struct Equip[10];		// Characters texture array
/*0780*/    uint32_t SecondaryIDFile[10];		// Characters secondary IDFile number
/*0820*/    uint8_t Unknown820[10];			// 10x ff
/*0830*/    uint8_t Unknown830[2];			// 2x 00
/*0832*/    uint32_t Deity[10];				// Characters Deity
/*0872*/    uint8_t GoHome[10];				// 1=Go Home available, 0=not
/*0882*/    uint8_t Tutorial[10];				// 1=Tutorial available, 0=not
/*0892*/    uint8_t Beard[10];				// Characters Beard Type
/*0902*/    uint8_t Unknown902[10];			// 10x ff
/*0912*/    uint32_t PrimaryIDFile[10];		// Characters primary IDFile number
/*0952*/    uint8_t HairColor[10];			// Characters Hair Color
/*0962*/    uint8_t Unknown0962[2];			// 2x 00
/*0964*/    uint32_t Zone[10];				// Characters Current Zone
/*1004*/    uint8_t Class[10];				// Characters Classes
/*1014*/    uint8_t Face[10];					// Characters Face Type
/*1024*/    char Name[10][64];				// Characters Names
/*1664*/    uint8_t Gender[10];				// Characters Gender
/*1674*/    uint8_t EyeColor1[10];			// Characters Eye Color
/*1684*/    uint8_t EyeColor2[10];			// Characters Eye 2 Color
/*1694*/    uint8_t Level[10];				// Characters Levels
/*1704*/
};

struct ZoneServerInfo_Struct
{
/*0000*/    char        ip[128];
/*0128*/    uint16_t    port;
/*0130*/    char        unknown1[0]; // variable length
};

struct RaidAdd_Struct
{
/*000*/ uint32_t    action;	//=0
/*004*/ char        player_name[64];	//should both be the player's name
/*068*/ char        leader_name[64];
/*132*/ uint8_t     _class;
/*133*/ uint8_t     level;
/*134*/ uint8_t     has_group;
/*135*/ uint8_t     unknown135;	//seems to be 0x42 or 0
};

struct Stun_Struct
{
    uint32_t duration; // Duration of stun
};

struct InterruptCast_Struct
{
    uint32_t    spawnid;
    uint32_t    messageid;
    char        message[0]; // variable length. Now sure how this flexible array is allocated...
};

struct BindWound_Struct
{
/*002*/ uint16_t	to;			// TargetID
/*004*/ uint16_t	unknown2;		// ***Placeholder
/*006*/ uint16_t	type;
/*008*/ uint16_t	unknown6;
};

struct MoveDoor_Struct
{
    uint8_t	doorid;
    uint8_t	action;
};

struct CombatAbility_Struct
{
    uint32_t target;		//the ID of the target mob
    uint32_t atk;
    uint32_t skill;
};

struct Taunt_Struct
{
/*000*/ uint32_t    spawn_id;
};

struct PickPocket_Struct
{
/*000*/ uint32_t    to;
/*004*/ uint32_t    from;
/*008*/ uint16_t    myskill;
/*010*/ uint8_t     type; // -1 you are being picked, 0 failed , 1 = plat, 2 = gold, 3 = silver, 4 = copper, 5 = item
/*011*/ uint8_t     unknown1; // 0 for response, unknown for input
/*012*/ uint32_t    coin;
/*016*/ uint8_t     lastsix[2];
/*018*/
};

struct PickPocketResponse_Struct {
/*000*/ uint32_t    to;
/*004*/ uint32_t    from;
/*008*/ uint32_t    myskill;
/*012*/ uint32_t    type;
/*016*/ uint32_t    coin;
/*020*/ char        itemname[64];
/*084*/
};

struct Track_Struct
{
    uint32_t entityid;
    float distance;
};

struct TrackTarget_Struct
{
    uint32_t	EntityID;
};


struct Translocate_Struct
{
/*000*/	uint32_t	ZoneID;
/*004*/	uint32_t	SpellID;
/*008*/	uint32_t	unknown008; //Heading ?
/*012*/	char		Caster[64];
/*076*/	float		y;
/*080*/	float		x;
/*084*/	float		z;
/*088*/	uint32_t	Complete;
};

struct TradeRequest_Struct
{
/*00*/  uint32_t to_mob_id;
/*04*/  uint32_t from_mob_id;
/*08*/
};

struct TradeBusy_Struct
{
/*00*/  uint32_t to_mob_id;
/*04*/  uint32_t from_mob_id;
/*08*/  uint8_t type;			// Seen 01
/*09*/  uint8_t unknown09;	// Seen EF (239)
/*10*/  uint8_t unknown10;	// Seen FF (255)
/*11*/  uint8_t unknown11;	// Seen FF (255)
/*12*/
};

struct TradeAccept_Struct
{
/*00*/  uint32_t from_mob_id;
/*04*/  uint32_t unknown4;		//seems to be garbage
/*08*/
};

struct TradeCoin_Struct
{
    uint32_t	trader;
    uint8_t		slot;
    uint16_t	unknown5;
    uint8_t		unknown7;
    uint32_t	amount;
};
struct CancelTrade_Struct {
/*00*/  uint32_t fromid;
/*04*/  uint32_t action;
/*08*/
};

struct LootRequest_Struct
{
/*00*/  uint32_t    entity_id;
/*04*/
};

struct EndLootRequest_Struct
{
/*00*/  uint32_t    entity_id;
/*04*/
};

struct LootingItem_Struct
{
/*000*/ uint32_t	lootee;
/*002*/ uint32_t	looter;
/*004*/ uint16_t	slot_id;
/*006*/ uint8_t	unknown3[2];
/*008*/ int32_t	auto_loot;
};

struct LevelAppearance_Struct
{
    uint32_t	spawn_id;
    uint32_t	parm1;
    uint32_t	value1a;
    uint32_t	value1b;
    uint32_t	parm2;
    uint32_t	value2a;
    uint32_t	value2b;
    uint32_t	parm3;
    uint32_t	value3a;
    uint32_t	value3b;
    uint32_t	parm4;
    uint32_t	value4a;
    uint32_t	value4b;
    uint32_t	parm5;
    uint32_t	value5a;
    uint32_t	value5b;
/*64*/
};

struct MoveCoin_Struct
{
    int32_t from_slot;
    int32_t to_slot; // 0xffffffff when deleting
    int32_t cointype1;
    int32_t cointype2;
    int32_t amount;
};

struct	ItemViewRequest_Struct
{
/*000*/ uint32_t    item_id;
/*004*/ uint32_t    augments[5];
/*024*/ uint32_t    link_hash;
/*028*/ char        unknown028[16];
};

struct Save_Struct
{
    uint8_t unknown00[192];
};

struct Arrow_Struct
{
/*000*/ uint32_t	type;		//unsure on name, seems to be 0x1, dosent matter
/*005*/ uint8_t	unknown004[12];
/*016*/ float	src_y;
/*020*/ float	src_x;
/*024*/ float	src_z;
/*028*/ uint8_t	unknown028[12];
/*040*/ float	velocity;		//4 is normal, 20 is quite fast
/*044*/ float	launch_angle;	//0-450ish, not sure the units, 140ish is straight
/*048*/ float	tilt;		//on the order of 125
/*052*/ uint8_t	unknown052[8];
/*060*/ float	arc;
/*064*/ uint8_t	unknown064[12];
/*076*/ uint32_t	source_id;
/*080*/ uint32_t	target_id;	//entity ID
/*084*/ uint32_t	item_id;	//1 to about 150ish
/*088*/ uint32_t	unknown088;	//seen 125, dosent seem to change anything..
/*092*/ uint32_t	unknown092;	//seen 16, dosent seem to change anything
/*096*/ uint8_t	unknown096[5];
/*101*/ char	model_name[16];
/*117*/ uint8_t	unknown117[19];
};

struct Merchant_Purchase_Struct
{
/*000*/ uint32_t	npcid;			// Merchant NPC's entity id
/*004*/ uint32_t	itemslot;		// Player's entity id
/*008*/ uint32_t	quantity;
/*012*/ uint32_t	price;
};

struct Merchant_Sell_Struct
{
/*000*/ uint32_t	npcid;			// Merchant NPC's entity id
/*004*/ uint32_t	playerid;		// Player's entity id
/*008*/ uint32_t	itemslot;
/*012*/ uint32_t	unknown12;
/*016*/ uint32_t	quantity;
/*020*/ uint32_t	price;
};

struct Merchant_Click_Struct
{
/*000*/ uint32_t    npcid;			// Merchant NPC's entity id
/*004*/ uint32_t    playerid;
/*008*/ uint32_t    command;		//1=open, 0=cancel/close
/*012*/ float       rate;			//cost multiplier, dosent work anymore
};

struct Merchant_DelItem_Struct
{
/*000*/ uint32_t	npcid;			// Merchant NPC's entity id
/*004*/ uint32_t	playerid;		// Player's entity id
/*008*/ uint32_t	itemslot;
/*012*/ uint32_t	unknown012;
};

struct SpellEffect_Struct
{
/*000*/ uint32_t EffectID;
/*004*/ uint32_t EntityID;
/*008*/ uint32_t EntityID2;	// EntityID again
/*012*/ uint32_t Duration;		// In Milliseconds
/*016*/ uint32_t FinishDelay;	// In Milliseconds - delay for final part of spell effect
/*020*/ uint32_t Unknown020;	// Seen 3000
/*024*/ uint8_t Unknown024;	// Seen 1 for SoD
/*025*/ uint8_t Unknown025;	// Seen 1 for Live
/*026*/ uint16_t Unknown026;	// Seen 1157 and 1177 - varies per char
/*028*/
};

/*
** Click Object Action Struct
** Response to client clicking on a World Container (ie, forge)
** also sent by the client when they close the container.
**
*/
struct ClickObjectAction_Struct
{
/*00*/  uint32_t    player_id;	// Entity Id of player who clicked object
/*04*/  uint32_t    drop_id;	// Zone-specified unique object identifier
/*08*/  uint32_t    open;		// 1=opening, 0=closing
/*12*/  uint32_t    type;		// See object.h, "Object Types"
/*16*/  uint32_t    unknown16;	// set to 0xA
/*20*/  uint32_t    icon;		// Icon to display for tradeskill containers
/*24*/  uint32_t    unknown24;	//
/*28*/  char        object_name[64]; // Object name to display
/*92*/
};

struct NewCombine_Struct
{
/*00*/  int16_t container_slot;
/*02*/  int16_t guildtribute_slot;
/*04*/
};

struct Assist_Struct
{
/*00*/	uint32_t	entity_id;
/*04*/
};

struct NameGeneration_Struct
{
/*0000*/	uint32_t	race;
/*0004*/	uint32_t	gender;
/*0008*/	char		name[64];
/*0072*/
};

struct ApproveName_Struct
{
/*000*/ char        character_name[64];
/*064*/ uint32_t    character_race;
/*068*/ uint32_t    character_class;
/*072*/ uint32_t    unknown068;
/*076*/
};

struct CharCreate_Struct
{
/*0000*/    uint32_t	class_;
/*0004*/    uint32_t	haircolor;	// Might be hairstyle
/*0008*/    uint32_t	beardcolor;	// Might be beard
/*0012*/    uint32_t	beard;		// Might be beardcolor
/*0016*/    uint32_t	gender;
/*0020*/    uint32_t	race;
/*0024*/    uint32_t	start_zone;
    // 0 = odus
    // 1 = qeynos
    // 2 = halas
    // 3 = rivervale
    // 4 = freeport
    // 5 = neriak
    // 6 = gukta/grobb
    // 7 = ogguk
    // 8 = kaladim
    // 9 = gfay
    // 10 = felwithe
    // 11 = akanon
    // 12 = cabalis
    // 13 = shar vahl
/*0028*/    uint32_t	hairstyle;	// Might be haircolor
/*0032*/    uint32_t	deity;
/*0036*/    uint32_t	STR;
/*0040*/    uint32_t	STA;
/*0044*/    uint32_t	AGI;
/*0048*/    uint32_t	DEX;
/*0052*/    uint32_t	WIS;
/*0056*/    uint32_t	INT;
/*0060*/    uint32_t	CHA;
/*0064*/    uint32_t	face;		// Could be unknown0076
/*0068*/    uint32_t	eyecolor1;	//its possiable we could have these switched
/*0073*/    uint32_t	eyecolor2;	//since setting one sets the other we really can't check
/*0076*/    uint32_t	tutorial;
/*0080*/
};

struct Duel_Struct
{
	uint32_t duel_initiator;
	uint32_t duel_target;
};

struct DisciplineTimer_Struct
{
/*00*/ uint32_t	TimerID;
/*04*/ uint32_t	Duration;
/*08*/ uint32_t	Unknown08;
};

struct Resurrect_Struct
{
/*000*/ uint32_t    unknown00;
/*004*/ uint16_t    zone_id;
/*006*/ uint16_t    instance_id;
/*008*/ float       y;
/*012*/ float       x;
/*016*/ float       z;
/*020*/ uint32_t    unknown20;
/*024*/ char        your_name[64];
/*088*/ uint32_t    unknown88;
/*092*/ char        rezzer_name[64];
/*156*/ uint32_t    spellid;
/*160*/ char        corpse_name[64];
/*224*/ uint32_t    action;
/*228*/
};

struct ControlBoat_Struct
{
/*000*/	uint32_t	boatId;			// entitylist id of the boat
/*004*/	bool	TakeControl;	// 01 if taking control, 00 if releasing it
/*007*/	char	unknown[3];		// no idea what these last three bytes represent
};

// Restore structure packing to default
#pragma pack()

#endif // EQSTRUCT_H

//. .7...6....,X....D4.M.\.....P.v..>..W....
//123456789012345678901234567890123456789012
//000000000111111111122222222223333333333444
