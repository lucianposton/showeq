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

#include "../conf.h"

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
#define MAX_KNOWN_SKILLS                74
#define MAX_KNOWN_LANGS                 25
#define MAX_SPELLBOOK_SLOTS             400
#define MAX_GROUP_MEMBERS               6

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

#define PLAYER_CLASSES     15
#define PLAYER_RACES       14

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
** Guild Update actions
*/
enum GuildUpdateAction
{
  GUA_Joined = 0,
  GUA_Left = 1,
  GUA_LastLeft = 6,
  GUA_FullGroupInfo = 7,
  GUA_Started = 9,
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
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t unused;
    } rgb;
    uint32_t color;
  };
};

/*
** Buffs
** Length: 16 Octets
** Used in:
**    charProfileStruct(2d20)
*/

struct spellBuff
{
/*0000*/  int8_t      slotid;             //
/*0001*/  int8_t      level;              // Level of person who casted buff
/*0002*/  uint16_t    effect;             //
/*0004*/  uint16_t    spellid;            // Spell
/*0006*/  int8_t      packing0006[2];     // Purely packing
/*0008*/  int32_t     duration;           // Duration in ticks
/*0012*/  int32_t     unknown0012;        //
/*0016*/  
};

/* 
 * Used in charProfileStruct
 * Size: 2 octets
 */
struct AA_Array
{
/*000*/ uint8_t AA;
/*001*/ uint8_t value;
};

/*
** Type:   Zone Change Request (before hand)
** Length: 70 Octets
** OpCode: ZoneChangeCode
*/
struct zoneChangeStruct
{
/*0000*/ char     name[64];		// Character Name
/*0064*/ uint16_t zoneId;               // zone Id
/*0066*/ uint16_t zoneInstance;         // zone Instance
/*0068*/ uint8_t unknown[8];            // unknown
/*0076*/
};

/*
** Client Zone Entry struct
** Length: 70 Octets
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
** Server Zone Entry struct
** Length: 390 Octets
** OpCode: ZoneEntryCode (when direction == server)
*/
struct ServerZoneEntryStruct
{
/*0000*/ uint32_t checksum;      // some kind of checksum
/*0004*/ uint8_t  gm;            // GM flag 0/1
/*0005*/ char	  name[64];      // Player name
/*0069*/ char     lastName[32];  // Players last name
/*0101*/ uint8_t  unknown101[7]; 
/*0108*/ float	  y;
/*0112*/ float	  x;
/*0116*/ float    z;
/*0120*/ float	  unknown088;
/*0124*/ float    unknown092;
/*0128*/ float    unknown096;
/*0132*/ uint8_t  unknown100[4];
/*0136*/ float    heading;
/*0140*/ float    unknown108;
/*0144*/ uint8_t  unknown112[88];
/*0232*/ uint8_t  walk_mode;     // 0=not walking,non-zero=walking
/*0233*/ uint8_t  linkdead;      // 1=LD,0=Not LD
/*0234*/ uint8_t  unknown202; 
/*0235*/ uint8_t  lfg;           // 1=LFG,0=Not FLG
/*0236*/ uint8_t  unknown204[4];
/*0240*/ union 
         {
	   struct 
	   {
	     /*0240*/ Color_Struct color_helmet;    // Color of helmet item
	     /*0244*/ Color_Struct color_chest;     // Color of chest item
	     /*0248*/ Color_Struct color_arms;      // Color of arms item
	     /*0252*/ Color_Struct color_bracers;   // Color of bracers item
	     /*0256*/ Color_Struct color_hands;     // Color of hands item
	     /*0260*/ Color_Struct color_legs;      // Color of legs item
	     /*0264*/ Color_Struct color_feet;      // Color of feet item
	     /*0268*/ Color_Struct color_primary;   // Color of primary item
	     /*0272*/ Color_Struct color_secondary; // Color of secondary item
	   } equipment_colors;
	   /*0240*/ Color_Struct colors[9];             // Array elements correspond to struct equipment_colors above
         };
/*276*/  union 
         {
	   struct 
	   {
	     /*0276*/ uint32_t equip_helmet;    // Equipment: Helmet Visual
	     /*0280*/ uint32_t equip_chest;     // Equipment: Chest Visual
	     /*0284*/ uint32_t equip_arms;      // Equipment: Arms Visual
	     /*0288*/ uint32_t equip_bracers;   // Equipment: Bracers Visual
	     /*0292*/ uint32_t equip_hands;     // Equipment: Hands Visual
	     /*0296*/ uint32_t equip_legs;      // Equipment: Legs Visual
	     /*0300*/ uint32_t equip_feet;      // Equipment: Feet Visual
	     /*0304*/ uint32_t equip_primary;   // Equipment: Primary Visual
	     /*0308*/ uint32_t equip_secondary; // Equipment: Secondary Visual
	   } equipment;
	   /*0276*/ Color_Struct equip[9];            // Array elements correspond to struct equipment above
         };
/*0312*/ uint16_t zoneId;
/*0314*/ uint16_t zoneInstance;
/*0316*/ uint8_t  unknown316[32];
										// They control movement
/*0348*/ float	runspeed;          // Speed when running
/*0352*/ float	unknown0352[2];    //
/*0360*/ float	size;              // Size of character
										// Changing size works, but then movement stops!
/*0364*/ float    walkspeed;       // Speed when walking
/*0368*/ uint8_t  unknown0368;     //
/*0369*/ uint8_t  haircolor;       // Hair color
/*0370*/ uint8_t  beardcolor;      // Beard color
/*0371*/ uint8_t  eyecolor1;       // Left eye color
/*0372*/ uint8_t  eyecolor2;       // Right eye color
/*0373*/ uint8_t  hairstyle;       // Hair style
/*0374*/ uint8_t  beard;           // Beard type
/*0375*/ uint8_t  face;            // Face type 
/*0376*/ uint8_t  level;           // Player's Level
/*0377*/ uint8_t  unknown0377;     //
/*0378*/ uint8_t  gender;          // 0=male, 1=female, 2=neutral
/*0379*/ uint8_t  pvp;             // PVP=1, No PVP=0
/*0380*/ uint8_t  invis;           // 1=invis, other=not invis (not sure about this..)
/*0381*/ uint8_t  unknown0381;     //
/*0382*/ uint8_t  class_;          // Player's Class
/*0383*/ uint8_t  unknown383[13];  
/*0396*/ uint32_t race;            // Player's Race
/*0400*/ uint8_t  anon;            // 0=non-anon,1=anon,2=role,3=role/anon 
/*0400*/ uint8_t  unknown400[27];
/*0428*/ uint32_t deity;           // Player's Deity
/*0432*/ uint8_t  unknown432[4];
/*0436*/ uint32_t guildId;         // Current guild
/*0440*/ uint8_t  unknown440[12];
/*0452*/
};

/*
** New Zone Code
** Length: 590 Octets
** OpCode: NewZoneCode
*/
struct newZoneStruct
{
/*0000*/ char    name[64];                 // Character name
/*0064*/ char    shortName[32];            // Zone Short Name
/*0096*/ char    longName[278];            // Zone Long Name
/*0310*/ uint8_t ztype;                    // Zone type
/*0311*/ uint8_t fog_red[4];               // Zone fog (red)
/*0315*/ uint8_t fog_green[4];             // Zone fog (green)
/*0319*/ uint8_t fog_blue[4];              // Zone fog (blue)
/*0374*/ uint8_t unknown0374[87];          // *** Placeholder
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
/*0520*/ uint8_t unknown0520[68];          // *** Placeholder
/*0588*/
};

/*
** Player Profile
** Length: 8454 Octets
** OpCode: CharProfileCode
*/
struct charProfileStruct
{
/*0000*/ uint32_t  checksum;           // 
/*0004*/ char      name[64];           // Name of player sizes not right
/*0068*/ char      lastName[32];       // Last name of player sizes not right
/*0100*/ uint32_t  gender;             // Player Gender - 0 Male, 1 Female
/*0104*/ uint32_t  race;               // Player race
/*0108*/ uint32_t  class_;             // Player class
/*0112*/ uint32_t  aapoints;           // unspent ability points? (wrong?)
/*0116*/ uint32_t  level;              // Level of player (might be one byte)
/*0120*/ uint32_t  bind_zone_id;       // Zone player is bound in
/*0124*/ float     bind_x;             // Bind loc x coord
/*0128*/ float	   bind_y;             // Bind loc y coord
/*0132*/ float	   bind_z;             // Bind loc z coord
/*0136*/ float     bind_heading;       // *** PLaceholder
/*0140*/ uint32_t  deity;              // deity
/*0144*/ uint32_t  guildID;            // guildID
/*0148*/ uint32_t  birthdayTime;       // character birthday
/*0152*/ uint32_t  lastSaveTime;       // character last save time
/*0156*/ uint32_t  timePlayedMin;      // time character played
/*0160*/ uint8_t   fatigue;            // Sta bar % depleted (ie. 30 = 70% sta)
/*0161*/ uint8_t   pvp;                // 1=pvp, 0=not pvp
/*0162*/ uint8_t   unknown162;         // *** Placeholder
/*0163*/ uint8_t   anon;               // 2=roleplay, 1=anon, 0=not anon
/*0164*/ uint8_t   gm;                 // 1=gm, 0=not gm
/*0165*/ uint8_t   unknown0165[47];    // *** Placeholder
/*0212*/ uint8_t   haircolor;          // Player hair color
/*0213*/ uint8_t   beardcolor;         // Player beard color
/*0214*/ uint8_t   eyecolor1;          // Player left eye color
/*0215*/ uint8_t   eyecolor2;          // Player right eye color
/*0216*/ uint8_t   hairstyle;          // Player hair style
/*0217*/ uint8_t   beard;              // Player beard type
/*0218*/ uint8_t   unknown0218[2];     // *** Placeholder
/*0220*/ uint32_t  item_material[9];   // Item texture/material of worn/held items
/*0256*/ uint8_t   unknown0256[52];    // *** Placeholder
/*0716*/ Color_Struct item_tint[9];    // RR GG BB 00
/*0344*/ AA_Array  aa_array[122];      // Length may not be right
/*0588*/ char 	   servername[64];     // length probably not right
/*0652*/ uint32_t  altexp;  	       // aaxp? (wrong?
/*0656*/ uint32_t  exp;                // Current Experience
/*0660*/ uint32_t  points;             // Unspent Practice points
/*0664*/ uint32_t  MANA;               // MANA
/*0668*/ uint32_t  curHp;              // current hp
/*0672*/ uint32_t  unknown0674;        // 0x05
/*0676*/ uint32_t  STR;                // Strength
/*0680*/ uint32_t  STA;                // Stamina
/*0684*/ uint32_t  CHA;                // Charisma
/*0688*/ uint32_t  DEX;                // Dexterity
/*0692*/ uint32_t  INT;                // Intelligence
/*0696*/ uint32_t  AGI;                // Agility
/*0700*/ uint32_t  WIS;                // Wisdom
/*0704*/ uint8_t   face;               // Player face
/*0705*/ uint8_t   unknown0705[47];    // *** Placeholder
/*0752*/ uint8_t   languages[28];      // List of languages (MAX_KNOWN_LANGS)
/*0780*/ uint8_t   unknown0780[4];     // All 0x00
/*0784*/ int32_t   sSpellBook[400];    // List of the Spells in spellbook
/*2384*/ uint8_t   unknown2384[448];   // all 0xff after last spell    
/*2832*/ int32_t   sMemSpells[8];      // List of spells memorized
/*2864*/ uint8_t   unknown2864[36];    // *** Placeholder
/*2900*/ float     x;                  // Players x position
/*2904*/ float     y;                  // Players y position
/*2908*/ float     z;                  // Players z position
/*2912*/ float     heading;            // Players heading   
/*2916*/ uint8_t   unknown2918[4];     //*** Placeholder    
/*2920*/ uint32_t  platinum;           // Platinum Pieces on player
/*2924*/ uint32_t  gold;               // Gold Pieces on player
/*2928*/ uint32_t  silver;             // Silver Pieces on player
/*2932*/ uint32_t  copper;             // Copper Pieces on player
/*2936*/ uint32_t  platinum_bank;      // Platinum Pieces in Bank
/*2940*/ uint32_t  gold_bank;          // Gold Pieces in Bank
/*2944*/ uint32_t  silver_bank;        // Silver Pieces in Bank
/*2948*/ uint32_t  copper_bank;        // Copper Pieces in Bank
/*2952*/ uint32_t  platinum_cursor;    // Platinum Pieces on cursor
/*2956*/ uint32_t  gold_cursor;        // Gold Pieces on cursor
/*2960*/ uint32_t  silver_cursor;      // Silver Pieces on cursor
/*2964*/ uint32_t  copper_cursor;      // Copper Pieces on cursor
/*2968*/ uint32_t  platinum_shared;    // Shared platinum pieces
/*2972*/ uint8_t   unknown2972[20];    // Unknown - all zero
/*2992*/ uint32_t  skills[74];         // List of skills (MAX_KNOWN_SKILLS)
/*3288*/ uint8_t   unknown3288[412];   //
/*3700*/ uint16_t  zoneId;             // see zones.h
/*3702*/ uint16_t  zoneInstance;       // 
/*3704*/ spellBuff buffs[15];          // Buffs currently on the player
/*3944*/ char      groupMembers[MAX_GROUP_MEMBERS][64];// all the members in group, including self 
/*4328*/ uint8_t   unknown4328[4];     // *** Placeholder
/*4332*/ uint32_t  ldon_guk_points;    // Earned Deepest Guk points
/*4336*/ uint32_t  ldon_mir_points;    // Earned Deepest Guk points
/*4340*/ uint32_t  ldon_mmc_points;    // Earned Deepest Guk points
/*4344*/ uint32_t  ldon_ruj_points;    // Earned Deepest Guk points
/*4348*/ uint32_t  ldon_tak_points;    // Earned Deepest Guk points
/*4352*/ uint8_t   unknown4352[24];    // *** Placeholder
/*4376*/ uint32_t  ldon_avail_points;  // Available LDON points
/*4380*/ uint8_t   unknown4380[540];   // *** Placeholder
/*4644*/	
};


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
** Length: 255 Octets 
** Used in: 
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/ 
struct spawnStruct
{
/*000*/ uint8_t NPC; // 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*001*/	int8_t  eyecolor1;			// Player left eye color
/*002*/	int8_t  eyecolor2;			// Player right eye color
/*003*/	int8_t  aa_title; // 0=none, 1=general, 2=archtype, 3=class
/*004*/	int32_t dye_rgb[7]; 			// armor dye colors
/*032*/ int8_t  unknown001[11]; 
/*043*/ uint8_t class_;
/*044*/ char unknown044[2]; // *** Placeholder
/*046*/ int8_t curHp;
/*047*/	int8_t afk; // 0=not afk, 1=afk
union {
/*048*/	int8_t equip_chest2;// Second place in packet for chest texture (usually 0xFF in live packets)
// Not sure why there are 2 of them, but it effects chest texture!
/*048*/	int8_t mount_color;// drogmor: 0=white, 1=black, 2=green, 3=red
// horse: 0=brown, 1=white, 2=black, 3=tan
};
/*049*/ int32_t race; // race
/*053*/ char    unknown053;
/*054*/ char    name[64]; // name
/*118*/ uint8_t haircolor; // hair color
/*119*/ uint8_t unknown119; 
/*120*/ uint8_t invis; // 0=visible,1=invisible
/*121*/ int8_t  maxHp; // max hp
/*122*/ uint8_t pvp; // 0=Not pvp,1=pvp
/*123*/ uint8_t level;
/*124*/ uint8_t lfg; // 0=Not lfg,1=lfg
/*125*/ uint16_t heading; // spawn heading
/*127*/ uint8_t  deltaHeading; // change in heading
/*128*/ uint8_t animation; // animation id
/*129*/ signed  deltaX:13;
        signed  x:19;
/*133*/ signed  y:19;
        signed  deltaZ:13;
/*137*/ signed  deltaY:13;
        signed  z:19;
/*141*/ int8_t	beardcolor;
/*142*/ int8_t	hairstyle;
/*143*/ int8_t	face;
/*144*/ int8_t	unknown144[6]; 
/*150*/ uint8_t light; // LightSource 0a=bo and ls, 0b=gls, 0c=fbe
/*151*/ float   size; // Size
/*155*/ uint8_t helm;
/*156*/ float   runspeed; //
/*160*/ uint8_t gm; // 0=not GM,1=GM
/*161*/ float   walkspeed; //
/*165*/ uint32_t guildID; // GuildID
/*169*/ uint8_t anon; // 0=normal,1=anon,2=roleplaying
/*170*/ uint8_t gender; // 0=male,1=female,2=other
/*171*/ uint16_t spawnId; // Id of spawn
/*173*/ char    unknown173[3];
/*176*/ char    lastName[32]; // lastname
/*208*/ int32_t equipment[9];
/*244*/ uint8_t linkdead; // 0=Not LD, 1=LD
/*245*/ uint32_t bodytype; // Bodytype
/*249*/	int8_t	guild_rank;
/*250*/ int8_t	unknown249[4]; 
/*254*/ uint32_t petOwnerId;
/*258*/ int16_t deity;
/*260*/ uint8_t unknown260[7];
/*267*/
};

/*
** Generic Door Struct
** Length: 52 Octets
** Used in: 
**    cDoorSpawnsStruct(f721)
**
*/

struct doorStruct
{
/*0000*/ char     name[16];        // Filename of Door?
/*0016*/ float    y;               // y loc
/*0020*/ float    x;               // x loc
/*0024*/ float    z;               // z loc
/*0028*/ float    heading;         // heading
/*0032*/ uint8_t  unknown0028[7]; // ***Placeholder
/*0039*/ int8_t   auto_return;
/*0040*/ uint8_t  initialState;
/*0041*/ uint8_t  unknown041[3];
/*0044*/ uint8_t  doorId;          // door's id #
/*0045*/ uint8_t  opentype;       
/*0046*/ uint8_t  size;           // size of door
/*0047*/ uint8_t holdstateforever;
/*0048*/ uint32_t zonePoint;
/*0052*/ uint8_t  unknown052[4]; // ***Placeholder
/*0056*/
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
/*0016*/ uint32_t zoneId;                 // ZoneID
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
** Length: 71 Octets + Variable Length + 4 Octets
** OpCode: ChannelMessageCode
*/

struct channelMessageStruct
{
/*0000*/ char     target[64];             // the target characters name
/*0064*/ char     sender[64];             // The senders name 
/*0128*/ uint32_t language;               // Language
/*0132*/ uint32_t chanNum;                // Channel
/*0136*/ int8_t   unknown0136[4];        // ***Placeholder
/*0140*/ uint32_t skillInLanguage;        // senders skill in language
/*0144*/ char     message[0];             // Variable length message
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
** Length: Variable Text
** OpCode: SimpleMessageCode
*/

struct simpleMessageStruct
{
/*0000*/ uint32_t  messageFormat;          // Indicates the message format
/*0005*/ ChatColor messageColor;                  // Message color
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
** Grouping Infromation
** Length: 136 Octets
** OpCode: OP_GroupUpdate
*/

struct groupUpdateStruct
{
/*0000*/ int32_t  action;
/*0004*/ char     yourname[64];           // Player Name
/*0068*/ char     membername[64];         // Goup Member Name
/*0132*/ uint32_t unknown0132;            // ***Placeholder
/*0136*/
};


/*
** Grouping Infromation
** Length: 452 Octets
** OpCode: OP_GroupUpdate
*/

struct groupFullUpdateStruct
{
/*0000*/ int32_t  action;
/*0004*/ char     membernames[MAX_GROUP_MEMBERS][64]; // Group Member Names
/*0388*/ char     leader[64];                         // Goup Member Name
/*0452*/
};

/*
** Grouping Invite
** Length 193 Octets
** Opcode OP_GroupInvite
*/

struct groupInviteStruct
{
/*0000*/ char     invitee[64];           // Invitee's Name
/*0064*/ char     inviter[64];           // Inviter's Name
/*0128*/ uint8_t  unknown0130[65];        // ***Placeholder
/*0193*/
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
** Length: 10 Octets
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
** Length: 30 Octets
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
** Length: 38 Octets
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
** Length: 18 Octets
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

/*
** Money Loot
** Length: 22 Octets
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
** Length: 14 Octets
** OpCode: staminaCode
*/

struct staminaStruct 
{
/*0000*/ uint32_t food;                     // (low more hungry 127-0)
/*0004*/ uint32_t water;                    // (low more thirsty 127-0)
/*0008*/ uint32_t fatigue;                  // (high more fatigued 0-100)
/*0012*/
};

/*
** Battle Code
** Length: 34 Octets
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
/*0012*/ int32_t  damage;                 // Amount of Damage
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
** Length: 18 Octets
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
** Length: 10 Octets
** OpCode: manaDecrementCode
*/

struct manaDecrementStruct
{
/*0000*/ int32_t newMana;                  // New Mana AMount
/*0004*/ int32_t unknown;
/*0008*/ int32_t spellId;                  // Last Spell Cast
/*0012*/
};

/*
** Special Message
** Length: 6 Octets + Variable Text Length
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
** Length: 10 Octets
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
** Length: 14 Octets
** OpCode: MemSpellCode
*/

struct memSpellStruct
{
/*0000*/ uint32_t slotId;                // Id of who is casting
/*0004*/ uint32_t spellId;                // Id of spell
/*0008*/ int16_t  param1;                 // Paramater 1
/*0010*/ int16_t  param2;                 // Paramater 2
};

/*
** Train Skill
** Length: 10 Octets
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
** Length: 10 Octets
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
** Length: 18 Octets
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
** Length: 14 Octets
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
** Length: 10 Octets
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
** Length: 5 Octets
** Opcode NpcHpUpdateCode
*/

struct hpNpcUpdateStruct
{
/*0002*/ uint16_t spawnId;
/*0004*/ int16_t maxHP;
/*0006*/ int16_t curHP;
/*0008*/ 
}; 

/*
** Inspecting Information
** Length: 1746 Octets
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
** Length: 10 Octets
** OpCode: RandomCode
*/
struct randomReqStruct 
{
/*0000*/ uint32_t bottom;                 // Low number
/*0004*/ uint32_t top;                    // High number
};

/*
** Random Number Result
** Length: 78 Octets
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
** Length: 20 Octets
** OpCode: PlayerPosCode
*/

struct playerSpawnPosStruct
{
/*0000*/ uint16_t spawnId;
/*0002*/ unsigned heading:12;
         signed   deltaHeading:10;
         unsigned animation:10;
/*0006*/ signed   deltaX:13;
         signed   x:19;
/*0010*/ signed   y:19;
         signed   deltaZ:13;
/*0014*/ signed   deltaY:13;
         signed   z:19;
/*0018*/
};

/*
** Self Position Update
** Length: 32 Octets
** OpCode: PlayerPosCode
*/

struct playerSelfPosStruct
{
/*0000*/ uint16_t spawnId;
/*0002*/ float z;
/*0006*/ float y;
/*0010*/ float deltaY;
/*0014*/ float x;
/*0018*/ float deltaX;
/*0022*/ float deltaZ;
/*0026*/ unsigned heading:12;
         signed deltaHeading:10;
         unsigned animation:10;
/*0030*/
};


/*
** Spawn Appearance
** Length: 10 Octets
** OpCode: spawnAppearanceCode
*/

struct spawnAppearanceStruct
{
/*0000*/ uint16_t spawnId;                // ID of the spawn
/*0002*/ uint16_t type;                   // Type of data sent
/*0004*/ uint32_t paramter;               // Values associated with the type
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
** Length: 28 Octets
** 
*/

struct buffStruct
{
/*0000*/ uint32_t spawnid;        //spawn id
/*0004*/ uint8_t  unknown0004[4]; 
/*0008*/ uint32_t spellid;        // spellid
/*0012*/ uint32_t duration;       // duration
/*0016*/ uint8_t  unknown0012[4];
/*0020*/ uint32_t spellslot;      // spellslot
/*0024*/ uint32_t changetype;     // 1=buff fading,2=buff duration
/*0028*/
};

/*
** Guild Member Update structure 
** Length: 76 octets
**
*/

struct GuildMemberUpdate
{
/*000*/ uint32_t type;          // update type (0xe3 seems to update zone and last on time)
/*004*/ char     name[64];      // member name
/*068*/ uint16_t zoneId;        // zone id 
/*070*/ uint16_t zoneInstance;  // zone instance
/*072*/ uint32_t lastOn;        // time the player was last on.
/*076*/
};

struct bazaarSearchQueryStruct 
{
  uint32_t mark;
  uint32_t type;
  char unknownXXX0[20]; // Value seems to always be the same
  char searchstring[64];
  uint32_t unknownXXX1;
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
** Length: 49158 Octets
** OpCode: GuildListCode 0x9221
*/
#define MAXGUILDS 512
struct worldGuildListStruct
{
/*000*/ guildListStruct dummy;
/*064*/ guildListStruct guilds[MAXGUILDS];
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
