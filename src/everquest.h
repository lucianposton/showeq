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

#include <stdint.h>

/*
** ShowEQ specific definitions
*/
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
#define LIST_MAXLIST                    19

/*
** MOB Spawn Type
*/
#define SPAWN_PLAYER                    0
#define SPAWN_NPC                       1
#define SPAWN_PC_CORPSE                 2
#define SPAWN_NPC_CORPSE                3
#define SPAWN_NPC_UNKNOWN               4
#define SPAWN_COINS                     5
#define SPAWN_DROP                      6
#define SPAWN_SELF                      10

/* 
** Diety List
*/
#define DEITY_UNKNOWN                   0
#define DEITY_AGNOSTIC			140
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

//Guessed:
#define DEITY_BERT			201	
#define DEITY_RODCET			212
#define DEITY_VEESHAN			216

//Team numbers for Deity teams
#define DTEAM_GOOD			1
#define DTEAM_NEUTRAL			0
#define DTEAM_EVIL			-1
#define DTEAM_OTHER			2

//Team numbers for Race teams
#define RTEAM_HUMAN			1
#define RTEAM_ELF			2
#define RTEAM_DARK			3
#define RTEAM_SHORT			4
#define RTEAM_OTHER			5

//Maximum limits of certain types of data
#define MAX_KNOWN_SKILLS                74
#define MAX_KNOWN_LANGS                 24

//Item Flags
#define ITEM_NORMAL1                    0x0031
#define ITEM_NORMAL2                    0x0036
#define ITEM_NORMAL3                    0x315f
#define ITEM_NORMAL4                    0x3336
#define ITEM_CONTAINER                  0x5400
#define ITEM_CONTAINER_PLAIN            0x5450
#define ITEM_BOOK                       0x7669

// Item spellId no spell value
#define ITEM_SPELLID_NOSPELL            0xffff

/*
** Compiler override to ensure
** byte aligned structures
*/
#pragma pack(1)

/*
**            Generic Structures used in specific
**                      structures below
*/

// OpCode stuff (For SINS Migrations)
struct opCode
{
  uint8_t code;
  uint8_t version;
	
  // kinda silly -- this is required for us to be able to stuff them in a QValueList
  bool operator== ( const struct opCode t ) const
  {
    return ( code == t.code && version == t.version );
  }
  bool operator== ( uint16_t opCode ) const
  {
    return ( *((uint16_t*)&code) == opCode );
  }
};
typedef struct opCode OpCode;

/*
** Buffs
** Length: 10 Octets
** Used in:
**    playerProfileStruct(2d20)
*/
struct spellBuff
{
/*0000*/ int8_t   unknown0000;            // ***Placeholder
/*0001*/ int8_t   level;                  // Level of person who casted buff
/*0002*/ int8_t   unknown0002;            // ***Placeholder
/*0003*/ int8_t   unknown0003;            // ***Placeholder
/*0004*/ int16_t  spell;                  // Spell
/*0006*/ int32_t  duration;               // Duration in ticks
};

/*
** Generic Item structure
** Length: 244 Octets
** Used in:
**    itemShopStruct(0c20), itemReceivedPlayerStruct(5220),
**    itemPlayerStruct(6421), bookPlayerStruct(6521),
**    containerPlayerStruct(6621), summonedItemStruct(7821),
**    tradeItemStruct(df20),
*/
struct itemStruct
{
/*0000*/ char      name[35];                // Name of item
/*0035*/ char      lore[60];                // Lore text
/*0095*/ char      idfile[6];               // Not sure what this is used for
/*0101*/ int16_t   flag;                    // Flag value indicating type of item:
  // 0x0031 - Normal Item - Only seen once on GM summoned food
  // 0x0036 - Normal Item (all scribed spells, Velium proc weapons, and misc.)
  // 0x315f - Normal Item
  // 0x3336 - Normal Item
  // 0x5400 - Container (Combine, Player made, Weight Reducing, etc...)
  // 0x5450 - Container, plain ordinary newbie containers
  // 0x7669 - Book item 
/*0103*/ int8_t    unknown0103[22];         // Placeholder
/*0125*/ uint8_t   weight;                  // Weight of item
/*0126*/ int8_t    nosave;                  // Nosave flag 1=normal, 0=nosave, -1=spell?
/*0127*/ int8_t    nodrop;                  // Nodrop flag 1=normal, 0=nodrop, -1=??
/*0128*/ uint8_t   size;                    // Size of item
/*0129*/ uint8_t   unknown0129;             // ***Placeholder
/*0130*/ uint16_t  itemNr;                  // Unique Item number
/*0132*/ uint16_t  iconNr;                  // Icon Number
/*0134*/ int16_t   equipSlot;               // Current Equip slot
/*0136*/ uint32_t  equipableSlots;          // Slots where this item goes
/*0140*/ int32_t   cost;                    // Item cost in copper
/*0144*/ uint8_t   unknown0144[28];         // ***Placeholder
union // 0172-291 have different meanings depending on flags
{
  // note, each of the following 2 structures must be kept of equal size
  struct // Common Item Structure (everything but books (flag != 0x7669)
  {
    /*0172*/ int8_t   STR;                 // Strength
    /*0173*/ int8_t   STA;                 // Stamina
    /*0174*/ int8_t   CHA;                 // Charisma
    /*0175*/ int8_t   DEX;                 // Dexterity
    /*0176*/ int8_t   INT;                 // Intelligence
    /*0177*/ int8_t   AGI;                 // Agility
    /*0178*/ int8_t   WIS;                 // Wisdom
    /*0179*/ int8_t   MR;                  // Magic Resistance
    /*0180*/ int8_t   FR;                  // Fire Resistance
    /*0181*/ int8_t   CR;                  // Cold Resistance
    /*0182*/ int8_t   DR;                  // Disease Resistance
    /*0183*/ int8_t   PR;                  // Poison Resistance
    /*0184*/ int8_t   HP;                  // Hitpoints
    /*0185*/ int8_t   MANA;                // Mana
    /*0186*/ int8_t   AC;                  // Armor Class
    /*0187*/ uint8_t  unknown0187[2];      // ***Placeholder
    /*0189*/ uint8_t  light;               // Light effect of this item
    /*0190*/ uint8_t  delay;               // Weapon Delay
    /*0191*/ uint8_t  damage;              // Weapon Damage
    /*0192*/ uint8_t  unknown0192;         // ***Placeholder
    /*0193*/ uint8_t  range;               // Range of weapon
    /*0194*/ uint8_t  skill;               // Skill of this weapon
    /*0195*/ int8_t   magic;               // Magic flag
    //   00  (0000)  =   ???
    //   01  (0001)  =  magic
    //   12  (1100)  =   ???
    //   14  (1110)  =   ???
    //   15  (1111)  =   ???
    /*0196*/ int8_t   level0;              // Casting level
    /*0197*/ uint8_t  material;            // Material?
    /*0198*/ uint8_t  unknown0198[2];      // ***Placeholder
    /*0200*/ uint32_t color;               // Amounts of RGB in original color
    /*0204*/ uint8_t  unknown0204[2];      // ***Placeholder
    /*0206*/ uint16_t spellId0;            // SpellID of special effect
    /*0208*/ uint16_t classes;             // Classes that can use this item
    /*0210*/ uint8_t  unknown0210[2];      // ***Placeholder
    union // 0212-0216 have different meanings depending on flags
    {
      // note, each of the following 2 structures must be kept of equal size
      struct // normal non-containers
      {
	/*0212*/ uint16_t races;           // Races that can use this item
	/*0214*/ int8_t   unknown0214[3];  // ***Placeholder
      } normal;
      struct // containers flag == 0x5400 or 0x5450
      {
	/*0212*/ int8_t   unknown0212;     // ***Placeholder
	/*0213*/ uint8_t  numSlots;        // number of slots in container
	/*0214*/ int8_t   unknown0214;     // ***Placeholder
	/*0215*/ int8_t   sizeCapacity;    // Maximum size item container can hold
	/*0216*/ uint8_t  weightReduction; // % weight reduction of container
      } container;
    };
    /*0217*/ uint8_t  level;               // Casting level
    union // 0218 has different meanings depending on an unknown indicator
    {
      /*0218*/ int8_t   number;            // Number of items in stack
      /*0218*/ int8_t   charges;           // Number of charges (-1 = unlimited)
    };
    /*0219*/ int8_t   unknown0219;         // ***Placeholder
    /*0220*/ uint16_t spellId;             // spellId of special effect
    /*0222*/ int8_t   unknown0222[70];     // ***Placeholder
  } common;
  struct // Book Structure (flag == 0x7669)
  {
    /*0172*/ int8_t   unknown0172[3];      // ***Placeholder
    /*0175*/ char     file[15];            // Filename of book text on server
    /*0190*/ int8_t   unknown0190[102];    // ***Placeholder
  } book;
};
};

// Convenience inlines for itemStruct
inline bool isItemBook(const struct itemStruct& i) { return (i.flag == ITEM_BOOK); }
inline bool IsItemContainer(const struct itemStruct& i)
{
  return ((i.flag == ITEM_CONTAINER) || (i.flag == ITEM_CONTAINER_PLAIN));
}

/*
** Generic Spawn Update
** Length: 15 Octets
** Used in:
**
*/
struct spawnPositionUpdate
{
/*0000*/ uint16_t spawnId;                // Id of spawn to update
/*0002*/ int8_t   animation;              // Animation spawn is currently using
/*0003*/ int8_t   heading;                // Heading
/*0004*/ int8_t   deltaHeading;           // Heading Change
/*0005*/ int16_t  yPos;                   // New Y position of spawn
/*0007*/ int16_t  xPos;                   // New X position of spawn
/*0009*/ int16_t  zPos;                   // New Z position of spawn
/*0011*/ int32_t  deltaY:10,              // Y Velocity
                  spacer1:1,              // ***Placeholder
                  deltaZ:10,              // Z Velocity
                  spacer2:1,              // ***Placeholder
                  deltaX:10;              // Z Velocity
};

/* 
** Generic Spawn Struct 
** Length: 172 Octets 
** Used in: 
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/ 
struct spawnStruct 
{ 
/*0000*/ uint8_t  unknown0000[49];        // Placeholder 
/*0049*/ int8_t   heading;                // Current Heading 
/*0050*/ int8_t   deltaHeading;           // Delta Heading 
/*0051*/ int16_t  yPos;                   // Y Position 
/*0053*/ int16_t  xPos;                   // X Position 
/*0055*/ int16_t  zPos;                   // Z Position 
/*0057*/ int32_t  deltaY:10,              // Velocity Y 
                  spacer1:1,              // Placeholder 
                  deltaZ:10,              // Velocity Z 
                  spacer2:1,              // ***Placeholder 
                  deltaX:10;              // Velocity X 
/*0061*/ uint8_t  unknown0061[1];         // ***Placeholder 
/*0062*/ uint16_t spawnId;                // Id of new spawn 
/*0064*/ uint8_t  unknown0064[2];         // ***Placeholder 
/*0066*/ uint16_t petOwnerId;             // Id of pet owner (0 if not a pet) 
/*0068*/ int16_t  maxHp;                  // Maximum hp's of Spawn 
/*0070*/ int16_t  curHp;                  // Current hp's of Spawn // GuildID now 
/*0072*/ uint8_t  race;                   // Race 
/*0073*/ uint8_t  NPC;                    // NPC type: 0=Player, 1=NPC, 2=Player Corpse, 3=Monster Corpse, 4=???, 5=Unknown Spawn,10=Self 
/*0074*/ uint8_t  class_;                 // Class 
/*0075*/ uint8_t  Gender;                 // Gender Flag, 0 = Male, 1 = Female, 2 = Other 
/*0076*/ uint8_t  level;                  // Level of spawn (might be one byte) 
/*0077*/ uint8_t  unknown0077[4];         // ***Placeholder 
/*0081*/ uint8_t  light;                  // Light emitting 
/*0082*/ uint8_t  unknown0082[10];        // ***Placeholder 
/*0092*/ uint16_t equipment[9];           // Equipment worn 
/*0110*/ char     name[30];               // Name of spawn (len is 30 or less) 
/*0140*/ char     lastname[20];           // Last Name of player 
/*0160*/ uint8_t  unknown0160[2];         // ***Placeholder 
/*0162*/ uint8_t  deity;                  // Deity. 
/*0163*/ uint8_t  unknown0163[1];         // ***Placeholder 
/*0164*/ uint8_t  unknown0164[8];         // ***Placeholder 
}; 

/* 
** Zone Spawn Struct 
** Length: 176 Octets 
** Used in: 
**    zoneSpawnStruct
**
*/ 
struct spawnZoneStruct
{
/*0000*/ uint8_t  unknown0000[4];
/*0004*/ spawnStruct spawn;
};

/*
** Generic Door Struct
** Length: 156 Octets
** Used in: 
**    compressedDoorStruct(f721)
**
*/
struct doorStruct
{
/*0000*/ char   name[8];                  // Filename of Door?
/*0008*/ int8_t unknown[40];              // ***Placeholder
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
/*0156*/ char   zoneName[40];           // Zone Information
};

/*
** Pet spawn struct (pets pet and owner in one struct)
*/
struct petStruct
{
/*0000*/ struct spawnStruct owner;      // Pet Owner Information
/*0156*/ struct spawnStruct pet;        // Pet Infromation
};


/*
**                 Specific Structures defining OpCodes
*/

/*
** Drop Coins
** Length: 114 Octets
** OpCode: dropCoinsCode
*/
struct dropCoinsStruct
{
/*0000*/ int8_t   opCode;                 // 0x07
/*0001*/ int8_t	  version;		  // 0x20
/*0002*/ uint8_t  unknown0002[24];        // ***Placeholder
/*0026*/ uint16_t dropId;                 // Drop ID
/*0028*/ uint8_t  unknown0028[22];        // ***Placeholder
/*0050*/ uint32_t amount;                 // Total Dropped
/*0054*/ uint8_t  unknown0054[4];         // ***Placeholder
/*0058*/ float    yPos;                   // Y Position
/*0062*/ float    xPos;                   // X Position
/*0066*/ float    zPos;                   // Z Position
/*0070*/ uint8_t  unknown0070[12];        // blank space
/*0082*/ int8_t   type[15];               // silver gold whatever
/*0097*/ uint8_t  unknown0097[17];        // ***Placeholder
};

/*
** Channel Message received or sent
** Length: 71 Octets + Variable Length + 4 Octets
** OpCode: ChannelMessageCode
*/
struct channelMessageStruct
{
/*0000*/ int8_t   opCode;                 // 0x07
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint16_t unknown0002[16];        // ***Placeholder
/*0034*/ int8_t   sender[23];             // The senders name (len might be wrong)
/*0057*/ uint8_t  unknown0057[9];         // ***Placeholder
/*0066*/ uint8_t  language;               // Language
/*0067*/ int8_t   unknown0064;            // ***Placeholder
/*0068*/ uint8_t  chanNum;                // Channel
/*0069*/ uint8_t  unknown0069[5];         // ***Placeholder
/*0074*/ char     message[0];             // Variable length message
};

/*
** Remove Coins
** Length: 10 Octets
** OpCode: removeCoinsCode
*/
struct removeCoinsStruct
{
/*0000*/ int8_t   opCode;                 // 0x08
/*0001*/ int8_t	  version;                // 0x20
/*0002*/ uint16_t dropId;                 // Drop ID - Guess
/*0004*/ int8_t   unknown0004[2];         // ***Placeholder
/*0006*/ uint16_t spawnId;                // Spawn Pickup
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** Item In Shop
** Length: 255 Octets
** OpCode: ItemInShopCode
*/
struct itemShopStruct
{
/*0000*/ int8_t   opCode;                 // 0x0c
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int8_t   unknown0002[4];         // Shopkeeper ID must be in here
/*0006*/ int8_t   itemType;               // 0 - item, 1 - container, 2 - book
/*0007*/ struct itemStruct item;          // Refer to itemStruct for members
/*0251*/ int8_t   unknown0251[4];         // ***Placeholder
};

/*
** Server System Message
** Length: Variable Length
** OpCode: SysMsgCode
*/
struct systemMessageStruct
{
/*0000*/ int8_t   opCode;                 // 0x14
/*0001*/ int8_t   version;                // 0x20
/*0002*/ char     message[0];             // Variable length message
};

/*
** Emote text
** Length: Variable Text
** OpCode: emoteTextCode
*/
struct emoteTextStruct
{
/*0000*/ int8_t   opCode;                 // 0x15
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int8_t   unknown2[2];            // ***Placeholder
/*0004*/ char     text[0];                // Emote `Text
};

/*
** Corpse location
** Length: Variable Text
** OpCode: corpseLocCode
*/
struct corpseLocStruct
{
/*0000*/ int8_t   opCode;                 // 0x21
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t spawnId;
/*0004*/ uint16_t unknown0;
/*0006*/ float    xPos;
/*0010*/ float    yPos;
/*0014*/ float    zPos;
};

/*
** Grouping Infromation
** Length: 230 Octets
** OpCode: groupinfoCode
*/
struct groupMemberStruct
{
/*0000*/ int8_t   opCode;                 // 0x26
/*0001*/ int8_t	  version;                // 0x40
/*0002*/ char     yourname[32];           // Player Name
/*0034*/ char     membername[32];         // Goup Member Name
/*0066*/ int8_t   unknown0066[35];        // ***Placeholder
/*0101*/ int8_t   bgARC;                  // Add = 2, Remove = 3, Clear = 0- Bad Guess-ATB
/*0102*/ int8_t   unknown0102[83];        // ***Placeholder
/*0185*/ int8_t   oper;                    // Add = 4, Remove = 3
/*0186*/ int8_t   ARC2;                   // ?? -  Add = c8, remove 1 = c5, clear = 01
/*0187*/ int8_t   unknown0187[43];        // ***Placeholder
};

/*
** Client Zone Entry struct
** Length: 38 Octets
** OpCode: ZoneEntryCode (when direction == client)
*/
struct ClientZoneEntryStruct
{
/*0000*/ int8_t   opCode;                 // 0x29
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int32_t  dwUnknown0002;          // ***Placeholder
/*0006*/ char     name[20];               // Player firstname
/*0026*/ int32_t  dwUnknown0026;          // ***Placeholder
/*0030*/ int32_t  dwUnknown0030;          // ***Placeholder
/*0034*/ int32_t  dwUnknown0034;          // ***Placeholder
};

/*
** Server Zone Entry struct
** Length: 322 Octets
** OpCode: ZoneEntryCode (when direction == server)
*/
struct ServerZoneEntryStruct
{
/*0000*/ int8_t   opCode;                 // 0x29
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int8_t   unknown0002[5];         // ***Placeholder
/*0007*/ char     name[30];               // Player first name
/*0037*/ char     zoneShortName[15];      // Zone Short Name
/*0052*/ int8_t   unknown0052[107];       // ***Placeholder
/*0159*/ uint8_t  _class;                  // Player's Class
/*0160*/ uint8_t  race;                   // Player's Race
/*0161*/ uint8_t  unknown0161[2];         // ***Placeholder
/*0163*/ uint8_t  level;                  // Player's Level
/*0164*/ uint8_t  unknown0164[148];       // ***Placeholder, could be usefull things here
/*0312*/ uint8_t  deity;                  // Player's Deity
/*0313*/ int8_t   unknown0313;            // ***Placeholder
/*0314*/ int8_t   unknown0314[8];         // ***Placeholder
};

/*
** Delete Spawn
** Length: 6 Octets
** OpCode: DeleteSpawnCode
*/
struct deleteSpawnStruct
{
/*0000*/ int8_t   opCode;                 // 0x2a
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t spawnId;                // Spawn ID to delete
/*0004*/ uint8_t  unknown[2];             // ***Placeholder
};

/*
** Player Profile
** Length: 8106 Octets
** OpCode: CharProfileCode
*/
struct playerProfileStruct
{
/*0000*/ int8_t   opCode;                 // 0x2d
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0006*/ char     name[30];               // Name of player
/*0036*/ char     lastName[20];           // Last name of player
/*0056*/ uint8_t  unknown0056[2];         // ***Placeholder
/*0058*/ uint8_t  race;                   // Player race
/*0059*/ int8_t   unknown1;               // ***Placeholder
/*0060*/ uint8_t  class_;                 // Player class
/*0061*/ uint8_t  gender;                 // Player gender
/*0062*/ uint8_t  level;                  // Level of player (might be one byte)
/*0063*/ uint8_t  unknown0063[3];         // ***Placeholder
/*0066*/ uint32_t exp;                    // Current Experience
/*0070*/ uint8_t  unknown0070[2];         // ***Placeholder
/*0072*/ uint16_t MANA;                   // MANA
/*0074*/ uint8_t  unknown0074[51];        // ***Placeholder
/*0125*/ uint8_t  STR;                    // Strength
/*0126*/ uint8_t  STA;                    // Stamina
/*0127*/ uint8_t  CHA;                    // Charisma
/*0128*/ uint8_t  DEX;                    // Dexterity
/*0129*/ uint8_t  INT;                    // Intelligence
/*0130*/ uint8_t  AGI;                    // Agility
/*0131*/ uint8_t  WIS;                    // Wisdom
/*0132*/ uint8_t  languages[24];          // List of languages (MAX_KNOWN_LANGS)
/*0156*/ uint8_t  unknown0156[494];       // ***Placeholder//518
/*0650*/ struct spellBuff buffs[15];      // Buffs currently on the player
/*0800*/ int8_t   unknown0800[1080];      // ***Placeholder
/*1880*/ int16_t  sSpellBook[256];        // List of the Spells scribed in the spell book
/*2392*/ int16_t  sMemSpells[8];          // List of spells memorized
/*2408*/ int8_t   unknown2408[54];        // ***Placeholder
/*2462*/ uint32_t platinum;               // Platinum Pieces on player
/*2466*/ uint32_t gold;                   // Gold Pieces on player
/*2470*/ uint32_t silver;                 // Silver Pieces on player
/*2474*/ uint32_t copper;                 // Copper Pieces on player
/*2478*/ uint32_t platinumBank;           // Platinum Pieces in Bank
/*2482*/ uint32_t goldBank;               // Gold Pieces in Bank
/*2486*/ uint32_t silverBank;             // Silver Pieces in Bank
/*2490*/ uint32_t copperBank;             // Copper Pieces in Bank
/*2494*/ uint8_t  unknown2494[16];        // ***Placeholder
/*2510*/ uint8_t  skills[74];             // List of skills (MAX_KNOWN_SKILLS)
/*2584*/ uint8_t  unknown2584[154];       // ***Placeholder
/*2702*/ char     GUILD[144];             // Guild Info -- Length wrong
/*2846*/ uint8_t  unknown2846[1376];      // ***Placeholder
/*4222*/ char     GroupMembers[5][48];    // List of all the members in the players group
/*4462*/ uint8_t  unknown4462[3644];      // ***PlaceHolder
};

/*
** Drop Item On Ground
** Length: 242 Octets
** OpCode: MakeDropCode
*/
struct dropThingOnGround
{
/*0000*/ int8_t   opCode;                 // 0x35
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint8_t  unknown0002[8];         // ***Placeholder
/*0010*/ uint16_t itemNr;                 // Item ID
/*0012*/ uint8_t  unknown0012[2];         // ***Placeholder
/*0014*/ uint16_t dropId;                 // DropID
/*0016*/ uint8_t  unknown2[26];           // ***Placeholder
/*0042*/ float    yPos;                   // Y Position
/*0046*/ float    xPos;                   // X Position
/*0050*/ float    zPos;                   // Z Position
/*0054*/ uint8_t  unknown3[4];            // ***Placeholder
/*0058*/ char     idFile[16];             // ACTOR ID
/*0074*/ uint8_t  unknown4[168];          // ***Placeholder
};

/*
** Remove Drop Item On Ground
** Length: 10 Octets
** OpCode: RemDropCode
*/
struct removeThingOnGround
{
/*0000*/ int8_t   opCode;                 // 0x36
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t dropId;                 // DropID - Guess
/*0004*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0006*/ uint16_t spawnId;                // Pickup ID - Guess
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** Consider Struct
** Length: 26 Octets
** OpCode: considerCode
*/
struct considerStruct{
/*0000*/ int8_t   opCode;                 // 0x37
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint16_t playerid;               // PlayerID
/*0004*/ int8_t   unknown1[2];            // ***Placeholder
/*0006*/ uint16_t targetid;               // TargetID
/*0008*/ int8_t   unknown2[2];            // ***Placeholder
/*0010*/ int32_t  faction;                // Faction
/*0014*/ int32_t  level;                  // Level
/*0018*/ int32_t  curHp;                  // Current Hitpoints
/*0022*/ int32_t  maxHp;                  // Maximum Hitpoints
/*0026*/ int32_t  unknown0026;            // unknown
};

/*
** Spell Casted On
** Length: 38 Octets
** OpCode: castOnCode
*/
struct castOnStruct
{
/*0000*/ int8_t   opCode;                 // 0x46
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t targetId;               // Target ID
/*0004*/ int8_t   unknown0004[2];         // ***Placeholder
/*0006*/ int16_t  sourceId;                // ***Source ID
/*0008*/ int8_t   unknown0008[2];         // ***Placeholder
/*0010*/ int8_t   unknown0010[24];        // might be some spell info?
/*0034*/ uint16_t spellId;                // Spell Id
/*0036*/ int8_t   unknown0036[2];         // ***Placeholder
};


/*
** New Spawn
** Length: 158 Octets
** OpCode: NewSpawnCode
*/
struct newSpawnStruct
{
/*0000*/ int8_t   opCode;                 // 0x46
/*0001*/ int8_t   version;                // 0x21
/*0002*/ int32_t  unknown0002;            // ***Placeholder
/*0006*/ struct spawnStruct spawn;        // Spawn Information
};

/*
** Spawn Death Blow
** Length: 18 Octets
** OpCode: CorpseCode
*/
struct spawnKilledStruct
{
/*0000*/ int8_t   opCode;                 // 0x4a
/*0001*/ int8_t   verison;                // 0x20
/*0002*/ uint16_t spawnId;                // Id of spawn that died
/*0004*/ uint16_t killerId;               // Killer
/*0006*/ int8_t   unknown0006[4];         // ***Placeholder
/*0010*/ uint16_t spellId;                // ID of Spell
/*0012*/ int8_t   type;                   // Spell, Bash, Hit, etc...
/*0013*/ int8_t   unknown0017;            // ***Placeholder
/*0014*/ uint16_t damage;                 // Damage
/*0016*/ int8_t   unknown0018[2];         // ***Placeholder
};

/*
** Money Loot
** Length: 22 Octets
** OpCode: MoneyOnCorpseCode
*/
struct moneyOnCorpseStruct
{
/*0000*/ int8_t   opCode;                 // 0x50
/*0001*/ int8_t   verison;                // 0x20
/*0002*/ int8_t   unknown0002[4];         // ***Placeholder
/*0006*/ uint32_t platinum;               // Platinum Pieces
/*0010*/ uint32_t gold;                   // Gold Pieces
/*0014*/ uint32_t silver;                 // Silver Pieces
/*0018*/ uint32_t copper;                 // Copper Pieces
};

/*
** Item received by the player
** Length: 246 Octets
** OpCode: ItemOnCorpseCode and ItemTradeCode
*/
struct itemReceivedPlayerStruct
{
/*0000*/ int8_t   opCode;                 // 0x52
/*0001*/ int8_t   version;                // 0x20
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Stamina
** Length: 8 Octets
** OpCode: staminaCode
*/
struct staminaStruct {
/*0000*/ int8_t  opCode;                   // 0x57
/*0001*/ int8_t  version;                  // 0x21
/*0002*/ int16_t food;                     // (low more hungry 127-0)
/*0004*/ int16_t water;                    // (low more thirsty 127-0)
/*0006*/ int16_t fatigue;                  // (high more fatigued 0-100)
};

/*
** Battle Code
** Length: 30 Octets
** OpCode: ActionCode
*/
struct actionStruct
{
/*0000*/ int8_t   opCode;                 // 0x58
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t target;                 // Target ID
/*0004*/ int8_t   unknown0004[2];         // ***Placeholder
/*0006*/ uint16_t source;                 // SourceID
/*0008*/ int8_t   unknown0008[2];         // ***Placeholder
/*0010*/ int8_t   type;                   // Casts, Falls, Bashes, etc...
/*0011*/ int8_t   unknown0011;            // ***Placeholder
/*0012*/ int16_t  spell;                  // SpellID
/*0014*/ int32_t  damage;                 // Amount of Damage
/*0018*/ int8_t   unknown0018[12];        // ***Placeholder
};

/*
** New Zone Code
** Length: 402 Octets
** OpCode: NewZoneCode
*/
struct newZoneStruct
{
/*0000*/ int8_t  opCode;                   // 0x5b
/*0001*/ int8_t  version;                  // 0x20
/*0002*/ char    charName[30];             // Character name
/*0032*/ char    shortName[15];            // Zone Short Name
/*0047*/ uint8_t unknown0047[5];           // *** Placeholder
/*0052*/ char    longName[180];            // Zone Long Name
/*0232*/ uint8_t unknown0232[170];         // *** Placeholder
};

/*
** Zone Spawns
** Length: 6Octets + Variable Length Spawn Data
** OpCode: ZoneSpawnsCode
*/
struct zoneSpawnsStruct
{
/*0000*/ int8_t opCode;                     // 0x61
/*0001*/ int8_t version;                    // 0x21
/*0002*/ struct spawnZoneStruct spawn[0];   // Variable number of spawns
};

/*
** client changes target struct
** Length: 6 Octets
** OpCode: clientTargetCode
*/
struct clientTargetStruct
{
/*0000*/ int8_t   opCode;                 // 0x62
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint16_t newTarget;              // Target ID
/*0004*/ uint16_t unknown0004;            // ***Placeholder
};

/*
** Item belonging to a player
** Length: 246 Octets
** OpCode: PlayerItemCode
*/
struct itemPlayerStruct
{
/*0000*/ int8_t   opCode;                 // 0x64
/*0001*/ int8_t   version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Book belonging to player
** Length: 205 Octets
** OpCode: PlayerBookCode
*/
struct bookPlayerStruct
{
/*0000*/ int8_t   opCode;                 // 0x65
/*0001*/ int8_t   version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Container Struct
** Length: 216 Octets
** OpCode: PlayerContainerCode
**
*/
struct containerPlayerStruct
{
/*0000*/ int8_t   opCode;                 // 0x66
/*0001*/ int8_t   version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Summoned Item - Player Made Item?
** Length: 244 Octets
** OpCode: summonedItemCode
*/
struct summonedItemStruct
{
/*0000*/ int8_t   opCode;                 // 0x78
/*0001*/ int8_t   version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Info sent when you start to cast a spell
** Length: 18 Octets
** OpCode: StartCastCode
*/
struct castStruct
{
/*0000*/ int8_t   opCode;                 // 0x7e
/*0001*/ int8_t   version;                // 0x21
/*0002*/ int16_t  unknown0002;            // ***Placeholder
/*0004*/ uint16_t spellId;                // Spell ID
/*0006*/ int16_t  unknown0006;            // ***Placeholder
/*0008*/ int16_t  unknown0008;            // ***Placeholder
/*0010*/ uint32_t targetId;               // The current selected target
/*0014*/ int8_t   unknown0014[4];         // ***Placeholder
};

/*
** New Mana Amount
** Length: 18 Octets
** OpCode: manaDecrementCode
*/
struct manaDecrementStruct
{
/*0000*/ int8_t  opCode;                   // 0x7f
/*0001*/ int8_t  version;			// 0x21
/*0002*/ int16_t newMana;                  // New Mana AMount
/*0004*/ int16_t spellID;                  // Last Spell Cast
};

/*
** Special Message
** Length: 6 Octets + Variable Text Length
** OpCode: SPMesgCode
*/
struct spMesgStruct
{
/*0000*/ int8_t  opCode;                  // 0x80
/*0001*/ int8_t  version;                 // 0x21
/*0002*/ int32_t msgType;                 // Type of message
/*0006*/ char    message[0];              // Message, followed by four Octets?
};

/*
** Spell Action Struct
** Length: 10 Octets
** OpCode: BeginCastCode
*/
struct beginCastStruct
{
/*0000*/ int8_t   opCode;                 // 0x82
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint16_t spawnId;                // Id of who is casting
/*0004*/ uint16_t spellId;                // Id of spell
/*0006*/ int16_t  param1;                 // Paramater 1
/*0008*/ int16_t  param2;                 // Paramater 2
};

/*
** Spell Action Struct
** Length: 14 Octets
** OpCode: MemSpellCode
*/
struct spellCastStruct
{
/*0000*/ int8_t   opCode;                 // 0x82
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint16_t spawnId;                // Id of who is casting
/*0004*/ int16_t  unknown0004;            // ***Placeholder
/*0006*/ uint16_t spellId;                // Id of spell
/*0008*/ int16_t  param1;                 // Paramater 1
/*0010*/ int16_t  param2;                 // Paramater 2
/*0012*/ int16_t  param3;                 // Parameter 3
};

/*
** Skill Increment
** Length: 10 Octets
** OpCode: SkillIncCode
*/
struct skillIncreaseStruct
{
/*0000*/ int8_t   opCode;                 // 0x89
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint16_t skillId;                // Id of skill
/*0004*/ int8_t   unknown0004[2];         // ***Placeholder
/*0006*/ int16_t  value;                  // New value of skill
/*0008*/ int8_t   unknown0008[2];         // ***Placeholder
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 14 Octets
** Opcode: WearChangeCode
*/
struct wearChangeStruct
{
/*0000*/ int8_t   opCode;                 // 0x92
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t spawnId;                // SpawnID
/*0004*/ int8_t   unknown0004[2];         // ***Placeholder
/*0006*/ int8_t   wearSlotId;             // Slot
/*0007*/ uint8_t  newItemId;              // Item ID
/*0008*/ int8_t   unknown0008[10];        // first few Octets react to clothing changes
};

/*
** Level Update
** Length: 14 Octets
** OpCode: LevelUpUpdateCode
*/
struct levelUpStruct
{
/*0000*/ int8_t   opCode;                 // 0x98
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint32_t level;                  // New level
/*0006*/ uint32_t levelOld;               // Old level
/*0010*/ uint32_t exp;                    // Current Experience
};

/*
** Experience Update
** Length: 14 Octets
** OpCode: ExpUpdateCode
*/
struct expUpdateStruct
{
/*0000*/ int8_t   opCode;                 // 0x99
/*0001*/ int8_t   version;                // 0x21
/*0002*/ uint32_t exp;                    // Current experience value
};

/*
** Spawn Position Update
** Length: 6 Octets + Number of Updates * 15 Octets
** OpCode: MobUpdateCode
*/
struct spawnPositionUpdateStruct
{
/*0000*/ int8_t   opCode;                 // 0xa1 - Used to be 0x85
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int32_t  numUpdates;             // Number of SpawnUpdates
/*0006*/ struct spawnPositionUpdate       // Spawn Position Update
                     spawnUpdate[0];
};

/*
** Type:   Zone Change Request (before hand)
** Length: 70 Octets
** OpCode: ZoneChangeCode
*/
struct zoneChangeStruct
{
/*0000*/ int8_t opCode;                 // 0xa3
/*0001*/ int8_t version;                // 0x20
/*0002*/ char charName[32];		// Character Name
/*0034*/ char zoneName[16];		// Zone Short Name
/*0050*/ int8_t unknown0050[20];	// *** Placeholder
};

/*
** Spawn HP Update
** Length: 14 Octets
** OpCode: HPUpdateCode
*/
struct spawnHpUpdateStruct
{
/*0000*/ int8_t   opCode;                 // 0xb2
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t spawnId;                // Id of spawn to update
/*0004*/ int16_t  unknown0004;            // ***Placeholder
/*0006*/ int16_t  curHp;                  // Current hp of spawn
/*0008*/ int16_t  unknown0008;            // ***Placeholder
/*0010*/ int16_t  maxHp;                  // Maximum hp of spawn
/*0012*/ int16_t  unknown0012;            // ***Placeholder
};

/*
** Inspecting Information
** Length: 1046 Octets
** OpCode: InspectDataCode
*/
struct inspectingStruct
{
/*0000*/ int8_t   opCode;                 // 0xb6
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int8_t   unknown0002[40];        // ***Placeholder
/*0042*/ int8_t   itemNames[21][32];      // 21 items with names 32 int8_tacters long.
/*0714*/ int16_t  icons[21];              // Icon Information
/*0756*/ int8_t   unknown0756[2];         // ***Placeholder
/*0758*/ char     mytext[200];            // Player Defined Text Info
/*0958*/ int8_t   unknown0958[88];        // ***Placeholder
};

/*
** Reading Book Information
** Length: Variable Length Octets
** OpCode: BookTextCode
*/
struct bookTextStruct
{
/*0000*/ int8_t opCode;                   // 0xce
/*0001*/ int8_t version;                  // 0x20
/*0002*/ char   text[0];                  // Text of item reading
};

/*
** Interrupt Casting
** Length: 6 Octets + Variable Length Octets
** Opcode: BadCastCode
*/
struct interruptCastStruct
{
/*0000*/ int8_t   opCode;                   // 0xd3
/*0001*/ int8_t   version;                  // 0x21
/*0002*/ uint16_t spawnId;                  // Id of who is casting
/*0004*/ char     message[0];               // Text Message
};

/*
** Info sent when trading an item
** Length: 258 Octets
** OpCode: tradeItemCode
*/
struct tradeItemStruct
{
/*0000*/ int8_t   opCode;                 // 0xdf
/*0001*/ int8_t   version;                // 0x20
/*0002*/ int8_t   unknown0002[6];         // ***Placeholder
/*0008*/ int8_t   itemtype;               // Type of item
/*0009*/ struct itemStruct item;          // Refer to itemStruct for members
/*0253*/ int8_t   unknown0253[5];         // ***Placeholder
};

/*
** Random Number Request
** Length: 10 Octets
** OpCode: RandomCode
*/
struct randomStruct 
{
/*0000*/ int8_t   opCode;                 // 0xe7
/*0001*/ int8_t	  version;                // 0x21
/*0002*/ uint32_t bottom;                 // Low number
/*0006*/ uint32_t top;                    // High number
};

/*
** Time of Day
** Length: 8 Octets
** OpCode: TimeOfDayCode
*/
struct timeOfDayStruct
{
/*0000*/ int8_t   opCode;                 // 0xf2
/*0001*/ int8_t	  version;                // 0x20
/*0002*/ int8_t   hour;                   // Hour (1-24)
/*0003*/ int8_t   minute;                 // Minute (0-59)
/*0004*/ int8_t   day;                    // Day (1-28)
/*0005*/ int8_t   month;                  // Month (1-12)
/*0006*/ uint16_t year;                   // Year
};

/*
** Player Position Update
** Length: 17 Octets
** OpCode: PlayerPosCode
*/
struct playerUpdateStruct
{
/*0000*/ int8_t   opCode;                 // 0xf3
/*0001*/ int8_t   version;                // 0x20
/*0002*/ uint16_t spawnId;                // Id of player
/*0004*/ int8_t   unknown0004[1];         // ***Placeholder
/*0005*/ int8_t   heading;                // Current heading of player
/*0006*/ int8_t   deltaHeading;           // Heading Change
/*0007*/ int16_t  yPos;                   // Players Y Position
/*0009*/ int16_t  xPos;                   // Players X Position
/*0011*/ int16_t  zPos;                   // Players Z Position
/*0013*/ int32_t  deltaY:10,              // Y Velocity
                  spacer1:1,              // ***Placeholder
                  deltaZ:10,              // Z Velocity
                  spacer2:1,              // ***Placeholder
                  deltaX:10;              // X Velocity
};

/*
** Spawn Appearance
** Length: 14 Octets
** OpCode: spawnAppearanceCode
*/
struct spawnAppearance
{
/*0000*/ int8_t   opCode;                 // 0xf5
/*0001*/ int8_t	  version;                // 0x20
/*0002*/ uint16_t spawnId;                // ID of the spawn
/*0002*/ int16_t  unknown0004;            // ***Placeholder
/*0006*/ int16_t  type;                   // Type of data sent
/*0008*/ int16_t  unknown0008;            // ***Placeholder
/*0010*/ uint32_t paramter;               // Values associated with the type
};

/*
** Compressed Packets
** Length: Variable Octets
** Opcodes: CompressedPlayerItem
*/
struct compressedPacket
{
/*0000*/ int8_t	  opCode;                 // 0xf6 or 0xf7
/*0001*/ int8_t	  version;                // 0x21
/*0002*/ uint16_t count;                  // Number of packets contained
/*0004*/ uint8_t  compressedData[0];      // All the packets compressed together
};

/*
** Compressed Door Struct
** Length: 4 + (count * sizeof(doorStruct)) Octets
** OpCode: CompressedDoorSpawnCode
*/
struct compressedDoorStruct
{
/*0000*/ int8_t   opCode;                // 0xf7
/*0001*/ int8_t   version;               // 0x21
/*0002*/ uint16_t count;                 // number of doors
/*0004*/ struct doorStruct doors[0];     // door structures
};



/*
**               Structures that are not being currently used
*/
struct bindWoundStruct
{
/*0000*/ int8_t   opCode;               // ????
/*0001*/ int8_t	  version;		// ????
/*0002*/ uint16_t  playerid;            // TargetID
/*0004*/ int8_t   unknown0004[2];       // ***Placeholder
/*0006*/ int32_t  hpmaybe;              // Hitpoints -- Guess
};

struct inspectedStruct
{
/*0000*/ int8_t   opCode;              // ????
/*0001*/ int8_t	  version;             // ????
/*0002*/ uint16_t inspectorid;         // Source ID
/*0004*/ uint8_t  unknown0004[2];      // ***Placeholder
/*0006*/ uint16_t inspectedid;         // Target ID - Should be you
/*0008*/ uint8_t  unknown0008[2];      // ***Placeholder
};


struct attack1Struct
{
/*0000*/ int8_t   opCode;                 // ????
/*0001*/ int8_t   version;                // ????
/*0002*/ uint16_t spawnId;                // Spawn ID
/*0004*/ int16_t  unknown1;               // ***Placeholder
/*0006*/ int16_t  unknown2;               // ***Placeholder
/*0008*/ int16_t  unknown3;               // ***Placeholder
/*0010*/ int16_t  unknown4;               // ***Placeholder
/*0012*/ int16_t  unknown5;               // ***Placeholder
};

struct attack2Struct{
/*0000*/ int8_t   opCode;                 // ????
/*0001*/ int8_t	  version;                // ????
/*0002*/ uint16_t spawnId;                // Spawn ID
/*0004*/ int16_t  unknown1;               // ***Placeholder
/*0006*/ int16_t  unknown2;               // ***Placeholder
/*0008*/ int16_t  unknown3;               // ***Placeholder
/*0010*/ int16_t  unknown4;               // ***Placeholder
/*0012*/ int16_t  unknown5;               // ***Placeholder
};

struct newGuildInZoneStruct
{
/*0000*/ int8_t   opCode;                 // ????
/*0001*/ int8_t	  version;                // ????
/*0002*/ int8_t   unknown0002[8];         // ***Placeholder
/*0010*/ char     guildname[56];          // Guildname
};

struct moneyUpdateStruct{
/*0000*/ int8_t   opCode;                 // ????
/*0001*/ int8_t	  version;                // ????
/*0002*/ uint16_t unknown0002;            // ***Placeholder
/*0004*/ int8_t   unknown0004[2];         // ***Placeholder
/*0006*/ uint8_t  cointype;               // Coin Type
/*0007*/ int8_t   unknown0007[3];         // ***Placeholder
/*0010*/ uint32_t amount;                 // Amount
};

/* Memorize slot operations, mem, forget, etc */
struct memorizeSlotStruct
{
/*0000*/ int8_t   opCode;                   // ????
/*0001*/ int8_t   version;                  // ????
/*0002*/ int8_t   slot;                     // Memorization slot (0-7)
/*0003*/ int8_t   unknown0003[3];           // ***Placeholder
/*0006*/ uint16_t spellId;                 // Id of spell (offset of spell in spdat.eff)
/*0008*/ int8_t   unknown0008[6];           // ***Placeholder 00,00,01,00,00,00
};

// Restore structure packing to default
#pragma pack()


#endif // EQSTRUCT_H

//. .7...6....,X....D4.M.\.....P.v..>..W....
//123456789012345678901234567890123456789012
//000000000111111111122222222223333333333444
