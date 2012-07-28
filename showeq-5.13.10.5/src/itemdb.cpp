/*
 * itemdb.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 *  Copyright 2003-2007 by the respective ShowEQ Developers
 */

static char* itemdbid = "@(#) $Id: itemdb.cpp 664 2007-02-03 03:47:08Z dohpaz $ $Name$";

#include <unistd.h>
#include <stdio.h>

#include <qfileinfo.h>
#include <qstring.h>

#include "itemdb.h"
#include "util.h"
#include "datalocationmgr.h"

#ifdef USE_DB3
#include "gdbmconv.h"
#endif

#define UPGRADE_STATUS_UPDATE 100

// define the current version number for data in database
#define CURRENT_ITEM_FORMAT_VERSION 2

// forward declarations
QString& getField(QString& field, const char*& curPos);
QString& getLastField(QString& field, const char*& curPos);

// Base Item Database Entry structure from which all EQItemDBEntryData
// structures inherit for identity reasons
struct EQItemDBEntryData
{
   uint16_t m_entryFormatVersion;   // Specifies version of entry data format
   size_t m_entrySize;            // size of the whole database entry in bytes
};

struct EQItemDBEntryData_1 : public EQItemDBEntryData
{
 public:
  // public constructors (mostly for convenience)
  EQItemDBEntryData_1();
  EQItemDBEntryData_1(const EQItemDBEntryData &);
  
  // public member functions

  // initialize the DBEntryData using Datum from a database
  void Init(const Datum& data);

  // initialize the DBEntryData with empty/default values
  void Init();

  // public data members
  char   m_idfile[6];             // Not sure what this is used for, eg: IT63
  int16_t   m_flag;               // See itemStruct for values
  uint8_t  m_weight;              // Weight of item
  int8_t   m_nosave;              // Nosave flag 1=normal, 0=nosave, -1=spell?
  int8_t   m_nodrop;              // Nodrop flag 1=normal, 0=nodrop, -1=??
  uint8_t  m_size;                // Size of item
  uint16_t  m_iconNr;             // Icon Number
  uint32_t m_equipableSlots;      // Slots where this item goes
  int32_t  m_cost;                // Item cost in copper
  int8_t   m_STR;                 // Strength
  int8_t   m_STA;                 // Stamina
  int8_t   m_CHA;                 // Charisma
  int8_t   m_DEX;                 // Dexterity
  int8_t   m_INT;                 // Intelligence
  int8_t   m_AGI;                 // Agility
  int8_t   m_WIS;                 // Wisdom
  int8_t   m_MR;                  // Magic Resistance
  int8_t   m_FR;                  // Fire Resistance
  int8_t   m_CR;                  // Cold Resistance
  int8_t   m_DR;                  // Disease Resistance
  int8_t   m_PR;                  // Poison Resistance
  int8_t   m_HP;                  // Hitpoints
  int8_t   m_MANA;                // Mana
  int8_t   m_AC;                  // Armor Class
  uint8_t  m_light;               // Light effect of this item
  int8_t   m_delay;               // Weapon Delay
  int8_t   m_damage;              // Weapon Damage
  uint8_t  m_range;               // Range of weapon
  uint8_t  m_skill;               // Skill of this weapon
  int8_t   m_magic;               // Magic flag, 1(0001)=magic
  int8_t   m_level0;              // Casting level
  uint8_t  m_material;            // Material?
  uint32_t m_color;               // Amounts of RGB in original color
  uint16_t  m_spellId0;           // SpellID of special effect
  uint16_t  m_classes;            // Classes that can use this item
  uint16_t  m_races;              // Races that can use this item
  uint8_t  m_level;               // Casting level
  uint16_t  m_spellId;            // spellId of special effect
  int8_t   m_charges;             // number of charges of item effect (-1 = inf)
  uint8_t  m_numSlots;            // number of slots
  uint8_t  m_weightReduction;     // percentage weight reduction
  uint8_t  m_sizeCapacity;        // Maximum size item container can hold
};

struct EQItemDBEntryData_2 : public EQItemDBEntryData_1
{
 public:
  // public constructors (mostly for convenience)
  EQItemDBEntryData_2();
  EQItemDBEntryData_2(const EQItemDBEntryData &);
  
  // public member functions

  // initialize the DBEntryData using Datum from a database
  void Init(const Datum& data);

  // initialize the DBEntryData with empty/default values
  void Init();

  // public data members
  int8_t m_stackable;             // 1=stackable, 3=normal, 0=?
  int8_t m_effectType;            // 0=no, 1=click anywhere w/o class check,
                                  // 2=latent, 3=click anywhere EXPENDABLE,
                                  // 4=click worn, 5=click anywhere with
                                  // class check
  uint32_t m_castTime;            // cast time of effect
  uint16_t m_skillModId;          // ID of skill that item modifies
  int8_t m_skillModPercent;       // Percent that item modifies the skill
};

struct EQItemDBEntryData_3 : public EQItemDBEntryData
{
 public:
  // public member functions
  // initialize the DBEntryData using the serialized item info
  const char* Init(const char* serializedItem, ItemType itemType);

  // initialize the DBEntryData using Datum from a database
  void Init(const Datum& data);

  // initialize the DBEntryData with empty/default values
  void Init();

  // book keeping member to keep track of the last time this was updated
  time_t m_updated;

  // public data members - fields numbered by where they occur in the
  //  '|' delimited data
  /*005*/ ItemType m_itemType;            // item type
  /*006*/ // name - stored after the structure
  /*007*/ // lore name - stored after name after the structure
  /*008*/ char     m_idfile[6];           // Not sure what this is used for, eg: IT63
  /*009*/ uint32_t m_itemNr;
  /*010*/ uint8_t  m_weight;              // Weight of item
  /*011*/ int8_t   m_norent;              // Norent flag 1=normal, 0=no rent, -1=spell?
  /*012*/ int8_t   m_nodrop;              // Nodrop flag 1=normal, 0=nodrop, -1=??
  /*013*/ uint8_t  m_size;                // Size of item
  /*014*/ uint32_t m_equipableSlots;      // Slots where this item goes
  /*015*/ int32_t  m_cost;                // Item cost in copper
  /*016*/ uint16_t  m_iconNr;             // Icon Number
  /*017*/ int32_t  m_unknown018;
  /*018*/ uint32_t m_unknown019;
  /*019*/ uint32_t m_unknown020;
  /*020*/ bool     m_tradeskills;         // Tradeskill item?
  /*021*/ int8_t   m_CR;                  // Cold Resistance
  /*022*/ int8_t   m_DR;                  // Disease Resistance
  /*023*/ int8_t   m_PR;                  // Poison Resistance
  /*024*/ int8_t   m_MR;                  // Magic Resistance
  /*025*/ int8_t   m_FR;                  // Fire Resistance
  /*026*/ int8_t   m_STR;                 // Strength
  /*027*/ int8_t   m_STA;                 // Stamina
  /*028*/ int8_t   m_AGI;                 // Agility
  /*029*/ int8_t   m_DEX;                 // Dexterity
  /*030*/ int8_t   m_CHA;                 // Charisma
  /*031*/ int8_t   m_INT;                 // Intelligence
  /*032*/ int8_t   m_WIS;                 // Wisdom
  /*033*/ int8_t   m_HP;                  // Hitpoints
  /*034*/ int8_t   m_MANA;                // Mana
  /*035*/ int8_t   m_AC;                  // Armor Class
  /*036*/ uint32_t m_deity;
  /*037*/ uint16_t m_skillModId;          // ID of skill that item modifies
  /*038*/ int8_t   m_skillModPercent;     // Percent that item modifies the skill
  /*039*/ int32_t  m_baneDmg;             // Bane Damage
  /*040*/ uint32_t m_baneDmgRace;         // Bane Damage Race
  /*041*/ uint32_t m_baneDmgBody;         // Bane Damage Body 1=Humanoid,2=Lycanthrope,
                                          // 3=Undead,4=Giant,5=Construct,6=Extraplanar,
                                          // 7=Magical
  /*042*/ int8_t   m_magic;               // Magic flag, 1(0001)=magic
  /*043*/ int32_t  m_castTime;
  /*043*/ union 
  {
    /*044*/ uint8_t  m_level;            // Casting level
    /*044*/ uint8_t  m_haste;             // Haste % 
  };
  /*045*/ uint8_t  m_requiredLevel;       // Required Level to use item
  /*046*/ uint32_t m_bardSkillType;       // Bard Skill Type
  /*047*/ int32_t  m_bardSkillAmount;     // Bard Skill Amount
  /*048*/ uint8_t  m_light;               // Light effect of this item
  /*049*/ int8_t   m_delay;               // Weapon Delay
  /*050*/ uint8_t  m_recommendedLevel;    // Recommended level to use item
  /*051*/ uint8_t  m_recommendedSkill;    // Recommended skill to use item (primary skill)
  /*052*/ uint8_t  m_elemDmgType;         // Elemental Damage Type (1=magic, 2=fire)
  /*053*/ uint8_t  m_elemDmg;             // Elemental Damage
  /*054*/ int8_t m_effectType;            // 0=no, 1=click anywhere w/o class check,
                                          // 2=latent, 3=click anywhere EXPENDABLE,
                                          // 4=click worn, 5=click anywhere with
                                          // class check
  /*055*/ uint8_t  m_range;               // Range of weapon
  /*056*/ int8_t   m_damage;              // Weapon Damage
  /*057*/ uint32_t m_color;               // Amounts of RGB in original color
  /*058*/ uint32_t  m_classes;            // Classes that can use this item
  /*059*/ uint32_t  m_races;              // Races that can use this item
  /*060*/ uint32_t m_unknown061;
  /*061*/ uint16_t  m_spellId;            // SpellID of special effect
  /*062*/ int8_t   m_charges;             // number of charges of item effect (-1 = inf)
  /*063*/ uint8_t  m_skill;               // Skill of this weapon
  /*064*/ uint8_t  m_material;            // Material?
  /*065*/ int32_t  m_sellRate;            // Item sell rate
  /*066*/ uint32_t m_unknown067;
  /*067*/ union 
  {
    /*067*/ uint32_t m_fullfillment;      // Food fulfilment (How long it lasts)
    /*067*/ int16_t  m_clickyCastTime;    // Cast Time for clicky effects, in milliseconds
  };
  /*068*/ uint32_t m_unknown069;
  /*069*/ uint32_t m_unknown070;
  /*070*/ int32_t  m_focusId;		// Focus Effect Id
  /*071*/ int8_t   m_combatEffects;	// PoP: Combat Effects +
  /*072*/ int8_t   m_shielding;		// PoP: Shielding %
  /*073*/ int8_t   m_stunResist;	// PoP: Stun Resist %
  /*074*/ int8_t   m_strikeThrough;	// PoP: Strike Through %
  /*075*/ uint32_t m_unknown076;
  /*076*/ uint32_t m_unknown077;
  /*077*/ int8_t   m_spellShield;	// PoP: Spell Shield %
  /*078*/ int8_t   m_avoidance;		// PoP: Avoidance +
  /*079*/ int8_t   m_accuracy;		// PoP: Accuracy +
  /*080*/ uint32_t m_unknown081;
  /*081*/ int32_t  m_factionMod1;	// Faction Mod 1
  /*082*/ int32_t  m_factionMod2;	// Faction Mod 2
  /*083*/ int32_t  m_factionMod3;	// Faction Mod 3
  /*084*/ int32_t  m_factionMod4;	// Faction Mod 4
  /*085*/ int32_t  m_factionAmt1;	// Faction Amt 1
  /*086*/ int32_t  m_factionAmt2;	// Faction Amt 2
  /*087*/ int32_t  m_factionAmt3;	// Faction Amt 3
  /*088*/ int32_t  m_factionAmt4;	// Faction Amt 4
  /*089*/ char	   m_charmFile[32];	// file name of the charm
  /*090*/ uint32_t m_unknown091;
  /*091*/ uint8_t  m_augSlot1Type;	// LDoN: Augment Slot 1 Type
  /*092*/ uint8_t  m_augSlot2Type;	// LDoN: Augment Slot 2 Type
  /*093*/ uint8_t  m_augSlot3Type;	// LDoN: Augment Slot 3 Type
  /*094*/ uint8_t  m_augSlot4Type;	// LDoN: Augment Slot 4 Type
  /*095*/ uint8_t  m_augSlot5Type;	// LDoN: Augment Slot 5 Type
  /*096*/ uint32_t m_ldonTheme;
  /*097*/ uint32_t m_ldonPrice;
  /*098*/ uint32_t m_ldonSold;
  /*099*/ uint8_t  m_packType;		// 0:Small Bag, 1:Large Bag, 2:Quiver, 3:Belt Pouch ... there are 50 types
  /*100*/ uint8_t  m_numSlots;          // number of slots
  /*101*/ uint8_t  m_weightReduction;   // percentage weight reduction
  /*102*/ uint8_t  m_sizeCapacity;      // Maximum size item container can hold
  /*103*/ uint8_t  m_bookType;		// 0=rolled up note, 1=book
  /*104*/ uint32_t m_unknown105;
  /*105*/ char	   m_bookFile[16];	// file name of the book
  /*106*/ uint32_t m_unknown107;
  /*107*/ uint32_t m_unknown108;
};

struct EQItemDBEntryData_4 : public EQItemDBEntryData_3
{
 public:
  // public member functions
  // initialize the DBEntryData using the serialized item info
  const char* Init(const char* serializedItem, ItemType itemType);

  // initialize the DBEntryData using Datum from a database
  void Init(const Datum& data);

  // initialize the DBEntryData with empty/default values
  void Init();

  uint8_t m_loreFlag;
  uint8_t m_pendingLoreFlag;
  uint8_t m_artifactFlag;
  uint8_t m_summonedFlag;
};

typedef EQItemDBEntryData_4 EQItemDBEntryData_Current;

////////////////////////////////////////////////////////////////////
// Life is easier if new versions can inherit from the immediate
// predecessor version.  Otherwise more work and is necessary in 
// the EQItemDBEntry class accessor methods and in the 
// EQItemDBEntryData child classes.  Therefore next one should be:
//  
// struct EQItemDBEntryData_5 : public EQItemDBEntryData_4
//

struct EQItemDBEntryRawData_1 : public EQItemDBEntryData
{
  // initialize the DBEntryData with empty/default values
  void Init(void);

  // public data members
  time_t   m_updated;
  char     m_serializedItem[0];
};

typedef EQItemDBEntryRawData_1 EQItemDBEntryRawData_Current;

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBEntryData class
EQItemDBEntryData_1::EQItemDBEntryData_1()
{
}

EQItemDBEntryData_1::EQItemDBEntryData_1(const EQItemDBEntryData& item)
{
  // verify that the version is correct
  if ((item.m_entryFormatVersion == 1) &&
      (item.m_entrySize == sizeof(EQItemDBEntryData_1)))
  {
    // just copy the data into ourselves en-masse
    memcpy((void*)this, &item, sizeof(*this));
  }
  else // otherwise make it empty since nothing is valid
    memset((void*)this, 0, sizeof(*this));
}


void EQItemDBEntryData_1::Init()
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 1; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_1);

   m_nodrop            = -1;
   m_nosave            = -1;
   m_spellId0          = ITEM_SPELLID_NOSPELL;
   m_spellId           = ITEM_SPELLID_NOSPELL;
}

void EQItemDBEntryData_1::Init(const Datum& data)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 1; 

   // by default copy the entire structure from the data
   size_t copySize = ((EQItemDBEntryData*)data.data)->m_entrySize;

   // but not if it's bigger then us...
   if (copySize > sizeof(EQItemDBEntryData_1))
     copySize = sizeof(EQItemDBEntryData_1);

   // now copy the entire other structure into ourselves
   memcpy((void*)this, data.data, copySize);
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_1);
}

// EQItemDBEntryData version 2
EQItemDBEntryData_2::EQItemDBEntryData_2()
{
}

EQItemDBEntryData_2::EQItemDBEntryData_2(const EQItemDBEntryData& item)
{
  // verify that the version is correct
  if ((item.m_entryFormatVersion == 2) &&
      (item.m_entrySize == sizeof(EQItemDBEntryData_2)))
  {
    // just copy the data into ourselves en-masse
    memcpy((void*)this, &item, sizeof(*this));
  }
  else if ((item.m_entryFormatVersion == 1) &&
	   (item.m_entrySize == sizeof(EQItemDBEntryData_1)))
  {
    // just copy the data into ourselves en-masse
    memcpy((void*)this, &item, sizeof(EQItemDBEntryData_1));
    
    // and clear out the missing pieces
    m_stackable = 0;
    m_effectType = 0;
    m_castTime = 0;
    m_skillModId = 0;
    m_skillModPercent = 0;
  }
  else // otherwise make it empty since nothing is valid
    memset((void*)this, 0, sizeof(*this));
}


void EQItemDBEntryData_2::Init()
{
  EQItemDBEntryData_1::Init();
  
   // Set the structure format version information
   m_entryFormatVersion = 2; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_2);

   m_stackable = 0;
   m_effectType = 0;
   m_castTime = 0;
   m_skillModId = 0;
   m_skillModPercent = 0;
}

void EQItemDBEntryData_2::Init(const Datum& data)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 2; 

   // by default copy the entire structure from the data
   size_t copySize = ((EQItemDBEntryData*)data.data)->m_entrySize;

   // but not if it's bigger then us...
   if (copySize > sizeof(EQItemDBEntryData_2))
     copySize = sizeof(EQItemDBEntryData_2);

   // now copy the entire other structure into ourselves
   memcpy((void*)this, data.data, copySize);
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_2);
}

const char* EQItemDBEntryData_3::Init(const char* serializedItem, 
				      ItemType itemType)
{
  QString field;
  field.setLength(64);
  const char* curPos = serializedItem; 

   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this)); 

   // Set the structure format version information
   m_entryFormatVersion = 3; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_3);

   // note the time this was updated
   time(&m_updated);

   m_itemType = itemType; // 006

   QString tmpString;

   // now start copying data as appropriate
   tmpString = getField(field, curPos); // get ID - 009
   strncpy(&m_idfile[0], tmpString.latin1(), sizeof(m_idfile)); 
   m_itemNr = getField(field, curPos).toULong();  // 010
   m_weight = uint8_t(getField(field, curPos).toUShort()); // 011
   m_norent = uint8_t(getField(field, curPos).toUShort());         
   m_nodrop = uint8_t(getField(field, curPos).toUShort());         
   m_size = uint8_t(getField(field, curPos).toUShort());           
   m_equipableSlots = getField(field, curPos).toULong(); 
   m_cost = getField(field, curPos).toLong();           
   m_iconNr = getField(field, curPos).toUShort();        
   m_unknown018 = getField(field, curPos).toLong();
   m_unknown019 = getField(field, curPos).toULong();
   m_unknown020 = getField(field, curPos).toULong();
   m_tradeskills = (getField(field, curPos).toUShort() != 0);    
   m_CR = int8_t(getField(field, curPos).toShort());             
   m_DR = int8_t(getField(field, curPos).toShort());             
   m_PR = int8_t(getField(field, curPos).toShort());             
   m_MR = int8_t(getField(field, curPos).toShort());             
   m_FR = int8_t(getField(field, curPos).toShort());             
   m_STR = int8_t(getField(field, curPos).toShort());            
   m_STA = int8_t(getField(field, curPos).toShort());            
   m_AGI = int8_t(getField(field, curPos).toShort());            
   m_DEX = int8_t(getField(field, curPos).toShort());            
   m_CHA = int8_t(getField(field, curPos).toShort());            
   m_INT = int8_t(getField(field, curPos).toShort());            
   m_WIS = int8_t(getField(field, curPos).toShort());            
   m_HP = int8_t(getField(field, curPos).toShort());             
   m_MANA = int8_t(getField(field, curPos).toShort());           
   m_AC = int8_t(getField(field, curPos).toShort());             
   m_deity = getField(field, curPos).toULong();
   m_skillModId = getField(field, curPos).toUShort();     
   m_skillModPercent = int8_t(getField(field, curPos).toShort());
   m_baneDmg = getField(field, curPos).toLong();        
   m_baneDmgRace = getField(field, curPos).toULong();    
   m_baneDmgBody = getField(field, curPos).toULong();  
   
   
   m_magic = int8_t(getField(field, curPos).toShort());          
   m_castTime = getField(field, curPos).toLong();
   
   m_level = uint8_t(getField(field, curPos).toUShort());       
   //m_haste;        
   
   m_requiredLevel = uint8_t(getField(field, curPos).toUShort());  
   m_bardSkillType = getField(field, curPos).toULong();  
   m_bardSkillAmount = getField(field, curPos).toLong();
   m_light = uint8_t(getField(field, curPos).toUShort());          
   m_delay = int8_t(getField(field, curPos).toShort());          
   m_recommendedLevel = uint8_t(getField(field, curPos).toUShort());
   m_recommendedSkill = uint8_t(getField(field, curPos).toUShort());
   m_elemDmgType = uint8_t(getField(field, curPos).toUShort());    
   m_elemDmg = uint8_t(getField(field, curPos).toUShort());        
   m_effectType = int8_t(getField(field, curPos).toShort());       
   
   
   
   m_range = uint8_t(getField(field, curPos).toUShort());          
   m_damage = int8_t(getField(field, curPos).toShort());         
   m_color = getField(field, curPos).toULong();          
   m_classes = getField(field, curPos).toULong();       
   m_races = getField(field, curPos).toULong();         
   m_unknown061 = getField(field, curPos).toULong();
   m_spellId = getField(field, curPos).toUShort();      
   m_charges = int8_t(getField(field, curPos).toShort());        
   m_skill = uint8_t(getField(field, curPos).toUShort());          
   m_material = uint8_t(getField(field, curPos).toUShort());       
   m_sellRate = getField(field, curPos).toULong();       
   m_unknown067 = getField(field, curPos).toULong();
   
   
   m_fullfillment = getField(field, curPos).toULong(); 
   //m_clickyCastTime;     
   
   m_unknown069 = getField(field, curPos).toULong();
   m_unknown070 = getField(field, curPos).toULong();
   m_focusId = getField(field, curPos).toLong();
   m_combatEffects = int8_t(getField(field, curPos).toShort());
   m_shielding = int8_t(getField(field, curPos).toShort());
   m_stunResist = int8_t(getField(field, curPos).toShort());
   m_strikeThrough = int8_t(getField(field, curPos).toShort());
   m_unknown076 = getField(field, curPos).toULong();
   m_unknown077 = getField(field, curPos).toULong();
   m_spellShield = int8_t(getField(field, curPos).toShort());
   m_avoidance = int8_t(getField(field, curPos).toShort());
   m_accuracy = int8_t(getField(field, curPos).toShort());
   m_unknown081 = getField(field, curPos).toULong();
   m_factionMod1 = getField(field, curPos).toLong();
   m_factionMod2 = getField(field, curPos).toLong();
   m_factionMod3 = getField(field, curPos).toLong();
   m_factionMod4 = getField(field, curPos).toLong();
   m_factionAmt1 = getField(field, curPos).toLong();
   m_factionAmt2 = getField(field, curPos).toLong();
   m_factionAmt3 = getField(field, curPos).toLong();
   m_factionAmt4 = getField(field, curPos).toLong();
   tmpString = getField(field, curPos);
   strncpy(&m_charmFile[0], tmpString.latin1(), sizeof(m_charmFile)); 
   m_charmFile[31] = '\0';
   m_unknown091 = getField(field, curPos).toULong();
   m_augSlot1Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot2Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot3Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot4Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot5Type = uint8_t(getField(field, curPos).toUShort());
   m_ldonTheme = getField(field, curPos).toULong();
   m_ldonPrice = getField(field, curPos).toULong();
   m_ldonSold = getField(field, curPos).toULong();
   m_packType = uint8_t(getField(field, curPos).toUShort());
   m_numSlots = uint8_t(getField(field, curPos).toUShort()); 
   m_sizeCapacity = uint8_t(getField(field, curPos).toUShort());   
   m_weightReduction = uint8_t(getField(field, curPos).toUShort());
   m_bookType = uint8_t(getField(field, curPos).toUShort());
   m_unknown105 = getField(field, curPos).toULong();
   tmpString = getField(field, curPos);
   strncpy(&m_bookFile[0], tmpString.latin1(), sizeof(m_bookFile)); 
   m_charmFile[15] = '\0';
   m_unknown107 = getField(field, curPos).toULong();
   m_unknown108 = getField(field, curPos).toULong();
   // ZBTEMP: add processing for these fields
   getField(field, curPos);
   getField(field, curPos);
   getField(field, curPos);
   getLastField(field, curPos);

   return curPos;
}

void EQItemDBEntryData_3::Init()
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 3; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_3);

   // note the time this was updated
   time(&m_updated);

   m_nodrop            = -1;
   m_norent            = -1;
   m_spellId          = ITEM_SPELLID_NOSPELL;
}

void EQItemDBEntryData_3::Init(const Datum& data)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 3; 

   // by default copy the entire structure from the data
   size_t copySize = ((EQItemDBEntryData*)data.data)->m_entrySize;

   // but not if it's bigger then us...
   if (copySize > sizeof(EQItemDBEntryData_3))
     copySize = sizeof(EQItemDBEntryData_3);

   // now copy the entire other structure into ourselves
   memcpy((void*)this, data.data, copySize);
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_3);
}

const char* EQItemDBEntryData_4::Init(const char* serializedItem, 
				      ItemType itemType)
{
  QString field;
  field.setLength(64);
  const char* curPos = serializedItem; 

   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this)); 

   // Set the structure format version information
   m_entryFormatVersion = 4; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_4);

   // note the time this was updated
   time(&m_updated);

   m_itemType = itemType; // 006

   QString tmpString;

   // now start copying data as appropriate
   tmpString = getField(field, curPos); // get ID - 009
   strncpy(&m_idfile[0], tmpString.latin1(), sizeof(m_idfile)); 
   m_itemNr = getField(field, curPos).toULong();  // 010
   m_weight = uint8_t(getField(field, curPos).toUShort()); // 011
   m_norent = uint8_t(getField(field, curPos).toUShort());         
   m_nodrop = uint8_t(getField(field, curPos).toUShort());         
   m_size = uint8_t(getField(field, curPos).toUShort());           
   m_equipableSlots = getField(field, curPos).toULong(); 
   m_cost = getField(field, curPos).toLong();           
   m_iconNr = getField(field, curPos).toUShort();        
   m_unknown018 = getField(field, curPos).toLong();
   m_unknown019 = getField(field, curPos).toULong();
   m_unknown020 = getField(field, curPos).toULong();
   m_tradeskills = (getField(field, curPos).toUShort() != 0);    
   m_CR = int8_t(getField(field, curPos).toShort());             
   m_DR = int8_t(getField(field, curPos).toShort());             
   m_PR = int8_t(getField(field, curPos).toShort());             
   m_MR = int8_t(getField(field, curPos).toShort());             
   m_FR = int8_t(getField(field, curPos).toShort());             
   m_STR = int8_t(getField(field, curPos).toShort());            
   m_STA = int8_t(getField(field, curPos).toShort());            
   m_AGI = int8_t(getField(field, curPos).toShort());            
   m_DEX = int8_t(getField(field, curPos).toShort());            
   m_CHA = int8_t(getField(field, curPos).toShort());            
   m_INT = int8_t(getField(field, curPos).toShort());            
   m_WIS = int8_t(getField(field, curPos).toShort());            
   m_HP = int8_t(getField(field, curPos).toShort());             
   m_MANA = int8_t(getField(field, curPos).toShort());           
   m_AC = int8_t(getField(field, curPos).toShort());             
   m_deity = getField(field, curPos).toULong();
   m_skillModId = getField(field, curPos).toUShort();     
   m_skillModPercent = int8_t(getField(field, curPos).toShort());
   m_baneDmg = getField(field, curPos).toLong();        
   m_baneDmgRace = getField(field, curPos).toULong();    
   m_baneDmgBody = getField(field, curPos).toULong();  
   
   
   m_magic = int8_t(getField(field, curPos).toShort());          
   m_castTime = getField(field, curPos).toLong();
   
   m_level = uint8_t(getField(field, curPos).toUShort());       
   //m_haste;        
   
   m_requiredLevel = uint8_t(getField(field, curPos).toUShort());  
   m_bardSkillType = getField(field, curPos).toULong();  
   m_bardSkillAmount = getField(field, curPos).toLong();
   m_light = uint8_t(getField(field, curPos).toUShort());          
   m_delay = int8_t(getField(field, curPos).toShort());          
   m_recommendedLevel = uint8_t(getField(field, curPos).toUShort());
   m_recommendedSkill = uint8_t(getField(field, curPos).toUShort());
   m_elemDmgType = uint8_t(getField(field, curPos).toUShort());    
   m_elemDmg = uint8_t(getField(field, curPos).toUShort());        
   m_effectType = int8_t(getField(field, curPos).toShort());       
   
   
   
   m_range = uint8_t(getField(field, curPos).toUShort());          
   m_damage = int8_t(getField(field, curPos).toShort());         
   m_color = getField(field, curPos).toULong();          
   m_classes = getField(field, curPos).toULong();       
   m_races = getField(field, curPos).toULong();         
   m_unknown061 = getField(field, curPos).toULong();
   m_spellId = getField(field, curPos).toUShort();      
   m_charges = int8_t(getField(field, curPos).toShort());        
   m_skill = uint8_t(getField(field, curPos).toUShort());          
   m_material = uint8_t(getField(field, curPos).toUShort());       
   m_sellRate = getField(field, curPos).toULong();       
   m_unknown067 = getField(field, curPos).toULong();
   
   
   m_fullfillment = getField(field, curPos).toULong(); 
   //m_clickyCastTime;     
   
   m_unknown069 = getField(field, curPos).toULong();
   m_unknown070 = getField(field, curPos).toULong();
   m_focusId = getField(field, curPos).toLong();
   m_combatEffects = int8_t(getField(field, curPos).toShort());
   m_shielding = int8_t(getField(field, curPos).toShort());
   m_stunResist = int8_t(getField(field, curPos).toShort());
   m_strikeThrough = int8_t(getField(field, curPos).toShort());
   m_unknown076 = getField(field, curPos).toULong();
   m_unknown077 = getField(field, curPos).toULong();
   m_spellShield = int8_t(getField(field, curPos).toShort());
   m_avoidance = int8_t(getField(field, curPos).toShort());
   m_accuracy = int8_t(getField(field, curPos).toShort());
   m_unknown081 = getField(field, curPos).toULong();
   m_factionMod1 = getField(field, curPos).toLong();
   m_factionMod2 = getField(field, curPos).toLong();
   m_factionMod3 = getField(field, curPos).toLong();
   m_factionMod4 = getField(field, curPos).toLong();
   m_factionAmt1 = getField(field, curPos).toLong();
   m_factionAmt2 = getField(field, curPos).toLong();
   m_factionAmt3 = getField(field, curPos).toLong();
   m_factionAmt4 = getField(field, curPos).toLong();
   tmpString = getField(field, curPos);
   strncpy(&m_charmFile[0], tmpString.latin1(), sizeof(m_charmFile)); 
   m_charmFile[31] = '\0';
   m_unknown091 = getField(field, curPos).toULong();
   m_augSlot1Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot2Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot3Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot4Type = uint8_t(getField(field, curPos).toUShort());
   m_augSlot5Type = uint8_t(getField(field, curPos).toUShort());
   m_ldonTheme = getField(field, curPos).toULong();
   m_ldonPrice = getField(field, curPos).toULong();
   m_ldonSold = getField(field, curPos).toULong();
   m_packType = uint8_t(getField(field, curPos).toUShort());
   m_numSlots = uint8_t(getField(field, curPos).toUShort()); 
   m_sizeCapacity = uint8_t(getField(field, curPos).toUShort());   
   m_weightReduction = uint8_t(getField(field, curPos).toUShort());
   m_bookType = uint8_t(getField(field, curPos).toUShort());
   m_unknown105 = getField(field, curPos).toULong();
   tmpString = getField(field, curPos);
   strncpy(&m_bookFile[0], tmpString.latin1(), sizeof(m_bookFile)); 
   m_charmFile[15] = '\0';
   m_unknown107 = getField(field, curPos).toULong();
   m_unknown108 = getField(field, curPos).toULong();
   m_loreFlag = uint8_t(getField(field, curPos).toUShort());
   m_pendingLoreFlag = uint8_t(getField(field, curPos).toUShort());
   m_artifactFlag = uint8_t(getField(field, curPos).toUShort());
   m_summonedFlag = uint8_t(getLastField(field, curPos).toUShort());

   return curPos;
}

void EQItemDBEntryData_4::Init()
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 4; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_4);

   // note the time this was updated
   time(&m_updated);

   m_nodrop            = -1;
   m_norent            = -1;
   m_spellId          = ITEM_SPELLID_NOSPELL;
}

void EQItemDBEntryData_4::Init(const Datum& data)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 4; 

   // by default copy the entire structure from the data
   size_t copySize = ((EQItemDBEntryData*)data.data)->m_entrySize;

   // but not if it's bigger then us...
   if (copySize > sizeof(EQItemDBEntryData_4))
     copySize = sizeof(EQItemDBEntryData_4);

   // now copy the entire other structure into ourselves
   memcpy((void*)this, data.data, copySize);
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_4);
}

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBEntryRawData_1 class
void EQItemDBEntryRawData_1::Init(void)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 1; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryRawData_1);

   // note the time this was updated
   time(&m_updated);
}

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDB class
EQItemDB::EQItemDB(const DataLocationMgr* dataLocMgr)
  : QObject(NULL, "eqitemdb"),
    m_dataLocMgr(dataLocMgr)
{
  // What types of item data is saved
  m_dbTypesEnabled = (DATA_DB|RAW_DATA_DB);
  m_logItems = false;
  m_logItemPackets = false;

  QFileInfo fileInfo;

  fileInfo = m_dataLocMgr->findWriteFile("logs", "items.log");
  m_itemsLog = fileInfo.absFilePath();

  fileInfo = m_dataLocMgr->findWriteFile("logs", "itempackets.log");
  m_itemPacketsLog = fileInfo.absFilePath();

  // construct item data file names
  m_ItemDataDB = m_dataLocMgr->findWriteFile(".", "itemdata2").absFilePath();
  m_ItemRawDataDB = m_dataLocMgr->findWriteFile(".", "itemrawdata2").absFilePath();
}

EQItemDB::~EQItemDB()
{
}

QString EQItemDB::GetDBFile(int dbType)
{
  switch(dbType)
  {
    case DATA_DB:
      return m_ItemDataDB;
    case RAW_DATA_DB:
      return m_ItemRawDataDB;
  };

  return "";
}
 
bool EQItemDB::SetDBFile(int dbType, const QString& dbFileName)
{
  bool result = true;

  switch(dbType)
  {
    case DATA_DB:
      m_ItemDataDB = dbFileName;
      break;
    case RAW_DATA_DB:
      m_ItemRawDataDB = dbFileName;
      break;
    default:
      // no such DB, so can't set it.
      result = false;
  };

  return result;
}

int EQItemDB::GetEnabledDBTypes()
{
  return m_dbTypesEnabled;
}

bool EQItemDB::SetEnabledDBTypes(int dbTypes)
{
  // make sure argument is within the set of acceptable types
  if ((dbTypes & ~(DATA_DB | RAW_DATA_DB)) != 0)
    return false;

  m_dbTypesEnabled = dbTypes;

  return true;
}

#if 0 // ZBTEMP
static bool checkDestinationFile(const QString pfx, const QString fileName)
{
 // Get information about the file, if there is one
  QFileInfo fileInfo(fileName);

  // Get information about the directory the file should be in
  QFileInfo dirInfo(fileInfo.dirPath());

  if (!dirInfo.exists())
  {
    fprintf(stderr, 
	    "%s: Data Directory '%s' doesn't exist.\n",
	    (const char*)pfx, (const char*)dirInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  if (!dirInfo.isDir())
  {
    fprintf(stderr, 
	    "%s: Data Directory '%s' isn't a directory.\n",
	    (const char*)pfx, (const char*)dirInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  // make sure the directory is writable by us
  if (!dirInfo.isWritable())
  {
    fprintf(stderr, 
	    "%s: Data Directory '%s' isn't writable.\n",
	    (const char*)pfx, (const char*)dirInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  // if the db file exists, but isn't writable, we can't upgrade
  if (fileInfo.exists() && !fileInfo.isWritable())
  {
    fprintf(stderr, 
	    "%s: Data File '%s' isn't writable.\n",
	    (const char*)pfx, (const char*)fileInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  return true;
}
#endif // ZBTEMP

bool EQItemDB::Upgrade()
{
#if 0 // ZBTEMP
  int ret;
  QString newname;
  QString oldname;
  // upgrade existing data sources to new format
  QString gdbmExt = GDBMConvenience::extension();

#ifdef USE_DB3
  QString db3Ext = DB3Convenience::extension();
#endif
  QString destFile;
  bool goodDest = false;

  /////////////////////////////////////////////////////////
  // first check for the Item Data DB upgrade from old GDBM to whichever new

  // Get information about the file, if there is one
  QFileInfo fileInfo(m_ItemLoreDB + gdbmExt);

  // if the old item lore name file doesn't exist, then nothing to do...
  if (fileInfo.exists())
  { 
#ifdef USE_DB3
    destFile = m_ItemDataDB + db3Ext;
#else 
    destFile = m_ItemDataDB + gdbmExt;
#endif 

    // check to make sure the destination for the new format DB is correct
    goodDest = checkDestinationFile("EQItemDB::Upgrade()", destFile);

    if (!goodDest)
    {
      fprintf(stderr, 
	      "Destination for upgraded DB '%s' is bad.\n"
	      "\tAborting upgrade!\n", (const char*)destFile);
      return false;
    }

    int srcNum = 2;
    bool itemNameExists = false;
    bool itemDataExists = false;

    // let the user know what's going on...
    fprintf(stderr, "Upgrading item database file format\n");
    fprintf(stderr, "\tSource 1: %s\n", (const char*)(m_ItemLoreDB + gdbmExt));

    oldname = m_ItemNameDB + gdbmExt;
    fileInfo.setFile(oldname);
    itemNameExists = fileInfo.exists();
    if (itemNameExists)
      fprintf(stderr, "\tSource %d: %s\n", 
	      srcNum++, (const char*)(oldname));

    oldname = m_ItemDataDB + gdbmExt;
    fileInfo.setFile(oldname);
    itemDataExists = fileInfo.exists();
    if (itemDataExists)
      fprintf(stderr, "\tSource %d: %s\n", 
	      srcNum++, (const char*)(oldname));

    fprintf(stderr, "\tDestination: %s\n",(const char*)destFile);

    // our database convenience class
    GDBMConvenience* gdbmconv;

#ifdef USE_DB3
    gdbmconv = new GDBMConvenience;
#else
    gdbmconv = (GDBMConvenience*)this;
#endif
    
    // iterator over a database file
    GDBMIterator gdbmit;
    bool hasData;
    Datum key;
    Datum loredata, namedata, datadata, newdata;
    int itemNr;
    int loreSize;
    int nameSize;
    int count = 0;
    
    // Initialize the iterator on the lore file and retrieve the first key
    hasData = gdbmit.GetFirstKey(m_ItemLoreDB, key);
    
    // iterate for as long as there is data
    while (hasData)
    {
      // get current Item Number
      itemNr = *(uint16_t*)key.data;
      
      // retrieve data
      if (gdbmit.GetData(loredata))
      {
	// successfully retrieved lore name data
	// retrieve item name data
	if (itemNameExists)
	  gdbmconv->GetEntry(m_ItemNameDB, key, namedata);
	
	// retrieve item data data
	if (itemDataExists)
	  gdbmconv->GetEntry(m_ItemDataDB, key, datadata);
	
	// calculate the size of the existing strings (must have space for NULL)
	loreSize = (loredata.size != 0) ? loredata.size : 1;
	nameSize = (namedata.size != 0) ? namedata.size : 1;
	
	// calculate the size of the new data item
	newdata.size = sizeof(EQItemDBEntryData_Current) + loreSize + nameSize;
	
	// allocate storage buffer
	unsigned char databuffer[newdata.size];
	
	// copy any existing item data, or initialize it to defaults
	if (datadata.data != NULL)
	  ((EQItemDBEntryData_Current*)databuffer)->Init(datadata);
	else
	  ((EQItemDBEntryData_Current*)databuffer)->Init();
	
	// copy existing item lore name data, or put NULL in it's place
	if (loredata.data != NULL)
	  strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current)),
		  (const char*)loredata.data, loredata.size);
	else
	  ((char*)(databuffer + sizeof(EQItemDBEntryData_Current)))[0] = '\0';
	
	// copy existing item name data, or put NULL in it's place
	if (namedata.data != NULL)
	  strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current) + loreSize),
		  (const char*)namedata.data, namedata.size);
	else
	  ((char*)(databuffer + sizeof(EQItemDBEntryData_Current) + loreSize))[0] = '\0';
	
	// put databuffer into datum data pointer
	newdata.data = (void*)databuffer;
	
	// insert the new data into the item data db, replacing it if it exists
	Insert(m_ItemDataDB, key, newdata, true);
	
	// increment count
	count++;
	
	// print a notification every n items
	if ((count % UPGRADE_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\t\tUpgraded %d items so far...\n", count);
	
	// release old data storage
	gdbmit.Release(loredata);
	gdbmconv->Release(namedata);
	gdbmconv->Release(datadata);
      }
      
      // release storage for the old key
      gdbmit.Release(key);
      
      // retrieve the next lore key
      hasData = gdbmit.GetNextKey(key);
    }

    // shutdown the iterator
    gdbmit.Done();
    
#ifdef USE_DB3
    // shutdown the GDBM convenience class
    gdbmconv->Shutdown();
  
    // don't need the GDBM convenience class anymore, delete it.
    delete gdbmconv;
#endif
    // let users know what's going on
    fprintf(stderr, "\t\tFlushing changes to disk.\n");

    // make sure everythings flushed to disk
    sync();

    // let users know what's going on
    fprintf(stderr, "\t\tRenaming old database files.\n");
    
    newname = m_ItemNameDB + gdbmExt + ".old";
    ret = rename((const char*)(m_ItemNameDB + gdbmExt), (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)(m_ItemNameDB + gdbmExt), (const char*)newname);
    }

    newname = m_ItemLoreDB + gdbmExt + ".old";
    ret = rename((const char*)(m_ItemLoreDB + gdbmExt), (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)(m_ItemLoreDB + gdbmExt), (const char*)newname);
    }

#ifdef USE_DB3
    newname = m_ItemDataDB + gdbmExt + ".old";
    ret = rename((const char*)(m_ItemDataDB + gdbmExt), (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)(m_ItemDataDB + gdbmExt), (const char*)newname);
    }
#endif

    fprintf(stderr, "Finished upgrading %d items in item database\n",
	    count);
  }

#ifdef USE_DB3
  /////////////////////////////////////////////////////////
  // now check for the Item Data DB upgrade
  oldname = m_ItemDataDB + gdbmExt;
  fileInfo.setFile(oldname);

  if (fileInfo.exists())
  {
    // construct the name of the destination file
    destFile = m_ItemDataDB + db3Ext;

    // check to make sure the destination for the new format DB is correct
    goodDest = checkDestinationFile("EQItemDB::Upgrade()", destFile);

    if (!goodDest)
    {
      fprintf(stderr, 
	      "Destination for upgraded DB '%s' is bad.\n"
	      "\tAborting upgrade!\n", (const char*)destFile);
      return false;
    }

    // let the user know what's going on...
    fprintf(stderr, "Upgrading item database file format\n");
    fprintf(stderr, "\tSource: %s\n", (const char*)oldname);
    fprintf(stderr, "\tDestination: %s\n",(const char*)destFile);

    // our database convenience class
    GDBMConvenience* gdbmconv;

    gdbmconv = new GDBMConvenience;
    
    // iterator over a database file
    GDBMIterator gdbmit;
    bool hasData;
    Datum key;
    Datum data;
    int count = 0;
    
    // Initialize the iterator on the lore file and retrieve the first key
    hasData = gdbmit.GetFirstKey(m_ItemDataDB, key);
    
    // iterate for as long as there is data
    while (hasData)
    {
      // retrieve data
      if (gdbmit.GetData(data))
      {
	// insert the new data into the item data db, replacing it if it exists
	Insert(m_ItemDataDB, key, data, true);
	
	// increment the count
	count++;
	
	// print a notification every n items
	if ((count % UPGRADE_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\t\tUpgraded %d items so far...\n", count);

	// release the data
	gdbmit.Release(data);
      }
      
      // release thekey
      gdbmit.Release(key);
      
      // retrieve the next lore key
      hasData = gdbmit.GetNextKey(key);
    }

    // shutdown the iterator
    gdbmit.Done();

    // shutdown the GDBM convenience class
    gdbmconv->Shutdown();
  
    // don't need the GDBM convenience class anymore, delete it.
    delete gdbmconv;

    // let users know what's going on
    fprintf(stderr, "\t\tFlushing changes to disk.\n");

    // make sure everythings flushed to disk
    sync();

    // let users know what's going on
    fprintf(stderr, "\t\tRenaming old database file.\n");

    newname = m_ItemDataDB + gdbmExt + ".old";
    ret = rename((const char*)oldname, (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)oldname, (const char*)newname);
    }

    fprintf(stderr, "Finished upgrading %d items in item database\n",
	    count);
  }

  /////////////////////////////////////////////////////////
  // now check for the Raw Item Data DB upgrade
  oldname = m_ItemRawDataDB + gdbmExt;
  fileInfo.setFile(oldname);

  if (fileInfo.exists())
  {
    // construct the name of the destination file
    destFile = m_ItemRawDataDB + db3Ext;

    // check to make sure the destination for the new format DB is correct
    goodDest = checkDestinationFile("EQItemDB::Upgrade()", destFile);

    if (!goodDest)
    {
      fprintf(stderr, 
	      "Destination for upgraded DB '%s' is bad.\n"
	      "\tAborting upgrade!\n", (const char*)destFile);
      return false;
    }

    // let the user know what's going on...
    fprintf(stderr, "Upgrading item database file format\n");
    fprintf(stderr, "\tSource: %s\n", (const char*)oldname);
    fprintf(stderr, "\tDestination: %s\n",(const char*)destFile);

    // our database convenience class
    GDBMConvenience* gdbmconv;

    gdbmconv = new GDBMConvenience;
    
    // iterator over a database file
    GDBMIterator gdbmit;
    bool hasData;
    Datum key;
    Datum data;
    int count = 0;
    
    // Initialize the iterator on the lore file and retrieve the first key
    hasData = gdbmit.GetFirstKey(m_ItemRawDataDB, key);
    
    // iterate for as long as there is data
    while (hasData)
    {
      // retrieve data
      if (gdbmit.GetData(data))
      {
	// insert the new data into the item data db, replacing it if it exists
	Insert(m_ItemRawDataDB, key, data, true);
	
	// increment the count
	count++;
	
	// print a notification every n items
	if ((count % UPGRADE_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\t\tUpgraded %d items so far...\n", count);

	// release the data
	gdbmit.Release(data);
      }
      
      // release thekey
      gdbmit.Release(key);
      
      // retrieve the next lore key
      hasData = gdbmit.GetNextKey(key);
    }

    // shutdown the iterator
    gdbmit.Done();

    // shutdown the GDBM convenience class
    gdbmconv->Shutdown();
  
    // don't need the GDBM convenience class anymore, delete it.
    delete gdbmconv;

    // let users know what's going on
    fprintf(stderr, "\t\tFlushing changes to disk.\n");

    // make sure everythings flushed to disk
    sync();

    // let users know what's going on
    fprintf(stderr, "\t\tRenaming old database file.\n");

    newname = m_ItemRawDataDB + gdbmExt + ".old";
    ret = rename((const char*)oldname, (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)oldname, (const char*)newname);
    }

    fprintf(stderr, "Finished upgrading %d items in raw item database\n",
	    count);
  }
#endif

#endif // ZBTEMP
  return true;
}

inline QString& getField(QString& field, const char*& curPos)
{
  const char* begPos = curPos;

  // find closing seperator for field
  while (*curPos != '|') 
    curPos++;
  
  // set the return string to be the value found
  field.setLatin1(begPos, curPos - begPos);
  
  // increment the current position past the closing seperator
  curPos++;

  // return the field for convenient usage
  return field; 
}

inline QString& getLastField(QString& field, const char*& curPos)
{
  const char* begPos = curPos;

  if (*curPos == '|')
  {
    field.setLatin1(begPos, 0);
    curPos++;
    return field;
  }

  // find closing seperator for field
  while ((*curPos != '\\') && (*curPos != '"'))
    curPos++;
  
  // set the return string to be the value found
  field.setLatin1(begPos, curPos - begPos);

  if (*curPos == '\\')
    curPos++;

  // return the field for convenient usage
  return field; 
}

bool EQItemDB::AddItem(const char* serializedItem, int level)
{
  const char* curPos = serializedItem; 
  const char* begPos;
  const char* endPos;
  int length = strlen(serializedItem);
  QString field;
  int fieldCount = 0; 
  // preallocate field to be able to hold a minimum size
  field.setLength(96);

  // right now we'll skip over initial instance variables
  getField(field, curPos); fieldCount++; // 1 - charges
  getField(field, curPos); fieldCount++;
  getField(field, curPos); fieldCount++; // 3 - equipped slot;
  getField(field, curPos); fieldCount++; // 4 - bazaar trader price
  getField(field, curPos); fieldCount++;
  getField(field, curPos); fieldCount++;

  // skip over optional backslash and leading quote.
  if (*curPos == '\\')
    curPos++;
  if (*curPos == '"')
    curPos++;
  
  begPos = curPos;

  // iterate over the first 3 field of data
  ItemType itemType = ItemType(getField(field, curPos).toULong());   // 6 - 1

  // these two fields help to determine the length of the allocated structure
  QString name = getField(field, curPos);                  // 7 - 2
  QString loreName = getField(field, curPos);              // 8 - 3
  
  // iterate over the data
  Datum key, data;
  bool result = true;
  uint32_t itemNr = 0xffffffff;

  // doing this just to add a scope level
  {
    // calculate the size necessary for storage
    data.size = sizeof(EQItemDBEntryData_Current) + name.length() + loreName.length() + 2;
    
    // allocate the storage buffer
    unsigned char databuffer[data.size];
    
    // initialize the EQItemDBEntryData_Current portion
    endPos = ((EQItemDBEntryData_Current*)databuffer)->Init(curPos, itemType);

    // get the item number form it
    itemNr = ((EQItemDBEntryData_Current*)databuffer)->m_itemNr;

    // increment past the closing quote
    curPos = endPos+1;
    
    // calculate the size of the lore string
    size_t loreSize = loreName.length() + 1;
    
    // calculate the size of the name string
    size_t nameSize = name.length() + 1;
    
    // add lore string after EQItemDBEntryData_Current
    strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current)),
	    loreName.latin1(), loreSize);
    
    // add name string after lore string
    strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current) + loreSize),
	    name.latin1(), nameSize);
    
    // finish setting up the data 
    data.data = (void*)databuffer;
    
    // setup key datum to use for queries and insertions
    // construct the key
    key.size = sizeof(itemNr);
    key.data = (void*)&itemNr;
    
    Insert(m_ItemDataDB, key, data, true);
  }
    
  // calculate the size of the core item data
  size_t itemLen = endPos - begPos;
  
  // log the item
  if (m_logItems)
    logItem(begPos, itemLen);

  // only add if database is enabled and no entry exists
  if (m_dbTypesEnabled & RAW_DATA_DB)
  {
    // setup datum to insert
    data.size = sizeof(EQItemDBEntryRawData_Current) + itemLen + 1;
    
    unsigned char databuffer2[data.size];

    // initialize the EQItemDBEntryRawData_Current portion
    ((EQItemDBEntryRawData_Current*)databuffer2)->Init();

    // copy the raw data portion
    strncpy((char*)databuffer2 + sizeof(EQItemDBEntryRawData_Current),
	    begPos, itemLen);

    // add terminating null
    databuffer2[sizeof(EQItemDBEntryRawData_Current) + itemLen] = '\0';

    // finish setting up the data
    data.data = (void*)databuffer2;

    // Insert the data into the DB
    Insert(m_ItemRawDataDB, key, data, true);
  }

  if (*curPos != '|')
  {
    fprintf(stderr, "Level: %d: Expected '|' but found '%c' at pos %d field %d\n",
	    level, *curPos, (curPos - begPos), fieldCount);
    return false; // unknown state, so return false just to be safe
  }

  curPos++;
  
  // iterate over items contained within the item
  for (int i = 0; i < 10; i++)
  {
    if (*curPos == '"')
    {
      curPos++;
      char* subItem = new char[length]; // yes, it's an over allocation, but for safety
      char* subPos = subItem;
      while (*curPos != '\0')
      {
	if (*curPos == '"')
	  break;
	else if ((*curPos == '\\') && (*(curPos+1) == '"'))
	  curPos++;

	*subPos++ = *curPos++;
      }
      *subPos = '\0';

      result = AddItem(subItem, level + 1);
      delete subItem;

      if (!result)
      {
	fprintf(stderr, "Level: %d: AddItem() Failed. Stopped processing sub-items at pos %d field %d container slot %d for safety!\n",
		level, (curPos - begPos), fieldCount, i);
	return false; // unknown state, so return false just to be safe
      }

      if (*curPos != '"')
      {
	fprintf(stderr, 
		"Level: %d: Expected '\"' but found '%c' at pos %d field %d\n",
		level, *curPos, (curPos - begPos), fieldCount);
	return false; // unknown state, so return false just to be safe
      }

      // skip closing quote "
      curPos++; 
    }
    
    // increment field count
    fieldCount++;

    if ((*curPos != '|') && (*curPos != '\0'))
    {
      fprintf(stderr, 
	      "Level: %d: Expected '|' or '\\0' but found '%c' at pos %d field %d\n",
	      level, *curPos, (curPos - begPos), fieldCount);
      return false; // unknown state, so return false just to be safe
    }

    // skip field seperator
    curPos++; 
  }

  return result;
}

bool EQItemDB::DeleteItem(uint32_t itemNr)
{
  bool result = true;
  bool result2;
  Datum key;

  // setup key datum to use for queries and insertions
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;
  
  // delete data from ItemDataDB
    result = Delete(m_ItemDataDB, key);

  // only delete if database is enabled
  if ((m_dbTypesEnabled & RAW_DATA_DB) &&
      IsEntryExist(m_ItemRawDataDB, key))
  {
    result2 = Delete(m_ItemRawDataDB, key);

    if (!result2)
      result = result2;
  }

  return result;
}

bool EQItemDB::ItemExist(uint32_t itemNr)
{
  Datum key;
  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // so does the item exist in the Item Lore database
  return IsEntryExist(m_ItemDataDB, key);
}

QString EQItemDB::GetItemName(uint32_t itemNr)
{
  Datum key, data;
  QString result = "";

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to retrieve entry from the item name database
  if (GetEntry(m_ItemDataDB, key, data))
  {
    // get the size of the entry (can be different sizes in different versions)
    size_t entrySize = ((EQItemDBEntryData*)data.data)->m_entrySize;
    
    // calculate pointer to lore string
    const char* loreString = ((const char*)data.data) 
      + entrySize;
      
    // calculate pointer to name string
    result = ((const char*)data.data) + entrySize
      + 1 + strlen(loreString);
    
    // release the database memory
    Release(data);
  }

  return result;
}

QString EQItemDB::GetItemLoreName(uint32_t itemNr)
{
  Datum key, data;
  QString result;

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to retrieve entry from the item name database
  if (GetEntry(m_ItemDataDB, key, data))
  {
    // get the size of the entry (can be different sizes in different versions)
    size_t entrySize = ((EQItemDBEntryData*)data.data)->m_entrySize;
    
    // calculate pointer to lore string
    result = ((const char*)data.data) + entrySize;

    // release the database memory
    Release(data);
  }

  return result;
}

bool EQItemDB::GetItemData(uint32_t itemNr, class EQItemDBEntry** itemData)
{
  Datum key, data;
  bool result = false;

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to retrieve entry from the data database
  if (GetEntry(m_ItemDataDB, key, data))
  {
    // return a copy of the data
    if (itemData != NULL)
      *itemData = new EQItemDBEntry(itemNr, data.data);
    else
      Release(data); // if they don't want the data, just release it

    result = true;
  }
  else
  {
    // i no data, set itemData to NULL so they don't accidentally use it
    if (itemData != NULL)
      *itemData = (class EQItemDBEntry*)NULL;
  }  

  return result;
}

size_t EQItemDB::GetItemRawData(uint32_t itemNr, char** itemData)
{
  Datum key, data;
  size_t result = 0;

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to tretrieve entry from the raw data database
  if (GetEntry(m_ItemRawDataDB, key, data))
  {
    size_t copySize = data.size - sizeof(EQItemDBEntryRawData_Current);

    // return a copy of the data
    if (itemData != NULL) 
    {
      // allocate a memory to store the copy
      *itemData = new char[copySize];
      
      // copy the data (yes, it's obvious, but hey...)
      memcpy((void*)*itemData, 
	     (void*)((const char*)data.data + sizeof(EQItemDBEntryRawData_Current)), 
	     copySize);
    }

    // result is the size of the data, whether or not it's returned
    result = copySize;

    // free the database copy of the memory
    Release(data);
  }
  else // if item is not found set itemData to NULL, just in case
    if (itemData != NULL)
      *itemData = (char*)NULL; 

  return result;
}


size_t EQItemDB::GetItemRawData(uint32_t itemNr, 
				time_t& updated, char** itemData)
{
  Datum key, data;
  size_t result = 0;

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to tretrieve entry from the raw data database
  if (GetEntry(m_ItemRawDataDB, key, data))
  {
    size_t copySize = data.size - sizeof(EQItemDBEntryRawData_Current);

    // return a copy of the data
    if (itemData != NULL) 
    {
      // allocate a memory to store the copy
      *itemData = new char[copySize];
      
      // copy the data (yes, it's obvious, but hey...)
      memcpy((void*)*itemData, 
	     (void*)((const char*)data.data + sizeof(EQItemDBEntryRawData_Current)), 
	     copySize);
    }

    // return the time the item was last updated
    updated = ((EQItemDBEntryRawData_Current*)data.data)->m_updated;

    // result is the size of the data, whether or not it's returned
    result = copySize;

    // free the database copy of the memory
    Release(data);
  }
  else // if item is not found set itemData to NULL, just in case
    if (itemData != NULL)
      *itemData = (char*)NULL; 

  return result;
}

bool EQItemDB::ReorganizeDatabase(void)
{
  bool result, result2;

  // Reorganize Item Data database if enabled
  result = Reorganize(m_ItemDataDB);

  // Reorganize Item Raw Data database if enabled
  if (m_dbTypesEnabled & RAW_DATA_DB)
  {
    result2 = Reorganize(m_ItemRawDataDB);
    
    if (!result2)
      result = result2;
  }

  return result;
}

void EQItemDB::Shutdown()
{
#ifdef USE_DB3
  DB3Convenience::Shutdown();
#else
  GDBMConvenience::Shutdown();
#endif
}

const char* EQItemDB::Version()
{
  return itemdbid;
}


void EQItemDB::logItem(const char* serializedItem, size_t len)
{
  time_t now;
  now = time (NULL);
  
  FILE *lh;
  const char* filename = (const char*)m_itemsLog;
  lh = fopen (filename, "a");
  
  if (lh == NULL)
  {
    fprintf(stderr, "\aUnable to open file: [%s]\n", 
	    (const char*)filename);
    return;
  }

  fprintf(lh, "%s%*.*s\n\n",
	  ctime(&now), len, len, serializedItem);
  
  fclose(lh);
}

void EQItemDB::item(const uint8_t* data, size_t size, uint8_t)
{
  const itemPacketStruct* item = (const itemPacketStruct*)data;
  if (m_logItemPackets)
  {
    time_t now;
    now = time (NULL);

    FILE *lh;
    const char* filename = (const char*)m_itemPacketsLog;
    lh = fopen (filename, "a");
    
    if (lh == NULL)
    {
      fprintf(stderr, "\aUnable to open file: [%s]\n", 
	      (const char*)filename);
      return;
    }
    else
    {
      QString typeStr;
      switch (item->packetType)
      {
      case ItemPacketViewLink:
	typeStr = "ViewLink";
	break;
      case ItemPacketLoot:
	typeStr = "Loot";
	break;
      case ItemPacketTrade:
	typeStr = "Trade";
	break;
      case ItemPacketSummonItem:
	typeStr = "SummonItem";
	break;
      case ItemPacketMerchant:
	typeStr = "Merchant";
	break;
      case ItemPacketWorldContainer:
	typeStr = "WorldContainer";
	break;
      default:
	typeStr.setNum(item->packetType, 16);
      }
      
      
      fprintf(lh, "[Type:%s] [Fields:%d] %s%s\n\n", 
	      typeStr.latin1(), QString(item->serializedItem).contains('|'),
	      ctime(&now),  item->serializedItem);
      
      fclose(lh);
    }
  }

  AddItem(item->serializedItem); 
}

void EQItemDB::playerItem(const uint8_t* data, size_t size, uint8_t)
{
  const char* serializedItem = (const char*)data;
  FILE *lh = 0;
  time_t now;
  const char* ctimeStr = NULL;

  if (m_logItemPackets)
  {
    now = time (NULL);
    ctimeStr = ctime(&now);
    
    const char* filename = (const char*)m_itemPacketsLog;
    lh = fopen (filename, "a");
    
    if (!lh)
    {
      fprintf(stderr, "\aUnable to open file: [%s]\n", 
	      (const char*)filename);
      return;
    }
  } 

  const char* currentItem = serializedItem;
  const char* endItems = serializedItem + size;
  int i = 0;
  while (currentItem < endItems)
  {
    if (lh)
      fprintf(lh, "[Type:CharInventory] [Pos:%d] [Fields:%d] %s%s\n\n", 
	      i, QString(currentItem).contains('|'),
	      ctimeStr,  currentItem);

    AddItem(currentItem);
    currentItem += strlen(currentItem) + 1;
    i++;
  }

  if (lh)
    fclose(lh);
}

void EQItemDB::playerShroud(const uint8_t* data, size_t size, uint8_t dir)
{
    if (size != sizeof(spawnShroudOther))
    {
        // Shrouding self has items on it. It's just a flat serialization
        // like OP_ItemPlayerPacket
        spawnShroudSelf* shroud = (spawnShroudSelf*) data;

        playerItem(&shroud->items, 
            size - (sizeof(spawnShroudOther) + sizeof(playerProfileStruct)),
            dir);
    }
}

void EQItemDB::itemInfo(const uint8_t* data, size_t size, uint8_t)
{
  const itemInfoStruct* item = (const itemInfoStruct*)data;
  if (m_logItemPackets)
  {
    time_t now;
    now = time (NULL);
    
    FILE *lh;
    const char* filename = (const char*)m_itemPacketsLog;
    lh = fopen (filename, "a");
    
    if (!lh)
    {
      fprintf(stderr, "\aUnable to open file: [%s]\n", 
	      (const char*)filename);
      return;
    }
    else
    {
      fprintf(lh, "[Type:ItemInfo] [Fields:%d] %s%s\n\n", 
	      QString(item->serializedItem).contains('|'),
	      ctime(&now),  item->serializedItem);
      
      fclose(lh);
    }
  } 

  AddItem(item->serializedItem); 
}


////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBIterator class
EQItemDBIterator::EQItemDBIterator(EQItemDB* pItemDB, int dbType)
: m_pItemDB(pItemDB),
  m_dbType(dbType)
{
}

EQItemDBIterator::~EQItemDBIterator()
{
  Done();
}

bool EQItemDBIterator::GetFirstItemNumber(uint32_t* itemNr)
{
  bool result;
  Datum key;

  // if they passed in an itemNr pointer, set it 0 in case of failure
  if (itemNr)
    *itemNr = 0;

  // if we weren't initialized with an ItemDB, then nothing to do...
  if (m_pItemDB == NULL)
    return false;

  // retrieve the filename of the database
  QString dbName = m_pItemDB->GetDBFile(m_dbType);

  // if there isn't a file, obviously nothing to iterate over
  if (dbName.isEmpty())
    return false;

  // retrieve the first key
#ifdef USE_DB3
  result = GetFirstKey(m_pItemDB, dbName, key);
#else
  result = GetFirstKey(dbName, key);
#endif

  if (result)
  {
    // save the item number for later use
    m_itemNr = *(uint32_t*)key.data;

    // if successful and the caller wants the number, set it
    if (itemNr != NULL)
      *itemNr = m_itemNr;
  }

  // release the key
  Release(key);

  return result;
}

bool EQItemDBIterator::GetNextItemNumber(uint32_t* itemNr)
{
  bool result;
  Datum nextKey;

  // Retrieve the next key  
  result = GetNextKey(nextKey);

  // did it succeed
  if (result)
  {
    // save the item number for later use if successful
    m_itemNr = *(uint32_t*)nextKey.data;

    // if the caller requested the item number, return it
    if (itemNr != NULL)
      *itemNr = m_itemNr;
  }
  else if (itemNr)
    itemNr = 0;
    
  // release the key
  Release(nextKey);

  return result;
}

bool EQItemDBIterator::GetItemData(QString& itemData)
{
  Datum data;
  bool result;
  itemData = "";

  // make sure to only return strings for DB's containing strings
  // otherwise don't bother
  if (m_dbType != EQItemDB::DATA_DB)
    return false;

  // retrieve the data
  result = GetData(data);
  
  if (result)
  {
    // success, set the query result
    itemData = (const char*)data.data;

    // release the database copy
    Release(data);
  }
  
  return result;
}

bool EQItemDBIterator::GetItemData(class EQItemDBEntry** itemData)
{
  Datum data;
  bool result = false;

  // if the user is requesting data, then set it to NULL just in case
  if (itemData != NULL)
    *itemData = (class EQItemDBEntry*)NULL;

  // don't bother searching if request is for data not stored in this DB
  if (m_dbType != EQItemDB::DATA_DB)
    return false;

  // retrieve the data
  result = GetData(data);

  // if something was found, return it
  if (result)
  {
    // return a copy of the data
    if (itemData != NULL)
      *itemData = new EQItemDBEntry(m_itemNr, data.data);
    else
      Release(data); // if they don't want the data, just release it
  }
  
  return result;
}

int EQItemDBIterator::GetItemData(char** itemData)
{
  Datum data;
  int result = 0;
  bool found = false;

  // retrieve the data
  found = GetData(data);

  if (found)
  { 
    size_t copySize = data.size - sizeof(EQItemDBEntryRawData_Current);

    // data was found, did the caller want the data?
    if (itemData != NULL) 
    {
      // allocate a copy of the data
      *itemData = new char[data.size];

      // copy the data (yes, it's obvious, but hey...)
      memcpy((void*)*itemData, 
	     (void*)((const char*)data.data + sizeof(EQItemDBEntryRawData_Current)), 
	     copySize);
    }

    // return the size if successful, irregardless of if the caller wanted data
    result = copySize;

    // release the databases copy of the data
    Release(data);
  }
  else 
  {
    // if the caller requested, and no data, set itemData to NULL for safety
    if (itemData != NULL)
      *itemData = (char*)NULL;
  }

  return result;
}

void EQItemDBIterator::Done()
{ 
  // Have the iterator do it's schtick
#ifdef USE_DB3
  DB3Iterator::Done();
#else
  GDBMIterator::Done();
#endif
}

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBEntry class
EQItemDBEntry::EQItemDBEntry(uint32_t itemNr, 
                             void* entryData)
: m_itemEntryData((struct EQItemDBEntryData*)entryData),
  m_itemNr(itemNr)
  
{
  // lore name is stored after the entry
  m_itemLore = (((const char*)entryData) +  m_itemEntryData->m_entrySize);

  // and the item name is stored after that
  m_itemName = (((const char*)entryData) +  m_itemEntryData->m_entrySize 
		+ strlen(m_itemLore) + 1);
    
}

EQItemDBEntry::~EQItemDBEntry()
{
  if (m_itemEntryData)
    free(m_itemEntryData);
}

time_t EQItemDBEntry::GetUpdated()
{
   return ((EQItemDBEntryData_3*)m_itemEntryData)->m_updated;
}

QString EQItemDBEntry::GetIdFile()
{
   return ((EQItemDBEntryData_3*)m_itemEntryData)->m_idfile;
}

uint16_t EQItemDBEntry::GetEntryFormatVersion()
{
  return ((EQItemDBEntryData*)m_itemEntryData)->m_entryFormatVersion;
}

int16_t EQItemDBEntry::GetFlag() 
{ 
  return (int16_t)((EQItemDBEntryData_3*)m_itemEntryData)->m_itemType; 
}

uint8_t EQItemDBEntry::GetLoreFlag()
{
  if (((EQItemDBEntryData*)m_itemEntryData)->m_entryFormatVersion == 4)
    return ((EQItemDBEntryData_4*)m_itemEntryData)->m_loreFlag; 
  else
    return (m_itemLore[0] == '*');
}

uint8_t EQItemDBEntry::GetPendingLoreFlag()
{
  if (((EQItemDBEntryData*)m_itemEntryData)->m_entryFormatVersion == 4)
    return ((EQItemDBEntryData_4*)m_itemEntryData)->m_pendingLoreFlag; 
  else
    return (m_itemLore[0] == '~');
}

uint8_t EQItemDBEntry::GetSummonedFlag()
{
  if (((EQItemDBEntryData*)m_itemEntryData)->m_entryFormatVersion == 4)
    return ((EQItemDBEntryData_4*)m_itemEntryData)->m_summonedFlag; 
  else
    return (m_itemLore[0] == '&');
}

uint8_t EQItemDBEntry::GetArtifactFlag()
{
  if (((EQItemDBEntryData*)m_itemEntryData)->m_entryFormatVersion == 4)
    return ((EQItemDBEntryData_4*)m_itemEntryData)->m_artifactFlag; 
  else
    return (m_itemLore[0] == '#');
}

ItemType EQItemDBEntry::GetItemType() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_itemType; 
}

uint8_t EQItemDBEntry::GetWeight() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_weight; 
}

int8_t EQItemDBEntry::GetNoRent() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_norent; 
}

int8_t EQItemDBEntry::GetNoDrop() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_nodrop; 
}

uint8_t EQItemDBEntry::GetSize() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_size; 
}

uint16_t EQItemDBEntry::GetIconNr() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_iconNr; 
}

uint32_t EQItemDBEntry::GetSlots() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_equipableSlots; 
}

int32_t  EQItemDBEntry::GetCost() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_cost; 
}

int8_t   EQItemDBEntry::GetSTR() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_STR; 
}

int8_t   EQItemDBEntry::GetSTA() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_STA; 
}

int8_t   EQItemDBEntry::GetCHA() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_CHA; 
}

int8_t   EQItemDBEntry::GetDEX() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_DEX; 
}

int8_t   EQItemDBEntry::GetINT() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_INT; 
}

int8_t   EQItemDBEntry::GetAGI() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_AGI; 
}

int8_t   EQItemDBEntry::GetWIS() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_WIS; 
}

int8_t   EQItemDBEntry::GetMR() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_MR; 
}

int8_t   EQItemDBEntry::GetFR() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_FR; 
}

int8_t   EQItemDBEntry::GetCR() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_CR; 
}

int8_t   EQItemDBEntry::GetDR() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_DR; 
}

int8_t   EQItemDBEntry::GetPR() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_PR; 
}

int8_t   EQItemDBEntry::GetHP() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_HP; 
}

int8_t   EQItemDBEntry::GetMana() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_MANA; 
}

int8_t   EQItemDBEntry::GetAC() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_AC; 
}

uint8_t  EQItemDBEntry::GetLight() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_light; 
}

uint8_t  EQItemDBEntry::GetDelay() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_delay; 
}

uint8_t  EQItemDBEntry::GetDamage() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_damage; 
}

uint8_t  EQItemDBEntry::GetRange() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_range; 
}

uint8_t  EQItemDBEntry::GetSkill() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_skill; 
}

int8_t   EQItemDBEntry::GetMagic() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_magic; 
}

int8_t   EQItemDBEntry::GetLevel0() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_level; 
}

uint8_t  EQItemDBEntry::GetMaterial() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_material; 
}

uint32_t EQItemDBEntry::GetColor() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_color; 
}

uint16_t  EQItemDBEntry::GetClasses() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_classes; 
}

uint16_t  EQItemDBEntry::GetRaces() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_races; 
}

uint8_t  EQItemDBEntry::GetLevel() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_level; 
}

uint16_t  EQItemDBEntry::GetSpellId() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_spellId; 
}

int8_t   EQItemDBEntry::GetCharges()
{
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_charges;
}

uint8_t  EQItemDBEntry::GetNumSlots() 
{ 
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_numSlots; 
}

uint8_t  EQItemDBEntry::GetWeightReduction()
{
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_weightReduction;
}

uint8_t  EQItemDBEntry::GetSizeCapacity()
{
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_sizeCapacity;
}

int8_t EQItemDBEntry::GetStackable()
{
  if (m_itemEntryData->m_entryFormatVersion >= 3)
    return ((GetSpellId() == 0xffff) && (GetCharges() != 0));
  else
    return -1;
}


int8_t EQItemDBEntry::GetEffectType()
{
  return ((EQItemDBEntryData_3*)m_itemEntryData)->m_effectType;
}

QString EQItemDBEntry::GetEffectTypeString()
{
  static const char* effectNames[] = 
  {
    "No Effect",
    "Click anywhere w/o class check",
    "Latent/Worn",
    "Click anywhere expendable",
    "Click Worn",
    "Click anywhere w/ class check"
  };

  int effect = GetEffectType();
  if (effect == -1)
    return "Unknown (Old Data)";
  else if (effect < (int)(sizeof(effectNames) / sizeof (char*)))
    return effectNames[effect];
  else
    return QString::number(effect);
}

uint32_t EQItemDBEntry::GetCastTime()
{
  if (m_itemEntryData->m_entryFormatVersion >= 3)
    return ((EQItemDBEntryData_3*)m_itemEntryData)->m_castTime;
  else
    return 0;
}

uint16_t EQItemDBEntry::GetSkillModId()
{
  if (m_itemEntryData->m_entryFormatVersion == 3)
    return ((EQItemDBEntryData_3*)m_itemEntryData)->m_skillModId;
  else
    return 0;
}

int8_t EQItemDBEntry::GetSkillModPercent()
{
  if (m_itemEntryData->m_entryFormatVersion >= 3)
    return ((EQItemDBEntryData_3*)m_itemEntryData)->m_skillModPercent;
  else
    return 0;
}

bool   EQItemDBEntry::IsBook()
{
  return (((EQItemDBEntryData_3*)m_itemEntryData)->m_itemType == ItemTypeBook);
}

bool   EQItemDBEntry::IsContainer()
{
  return (((EQItemDBEntryData_3*)m_itemEntryData)->m_itemType == ItemTypeContainer);
}
	  
#ifndef QMAKEBUILD
#include "itemdb.moc"
#endif

