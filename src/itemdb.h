/*
 * itemdb.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#ifndef EQITEMDB_H
#define EQITEMDB_H

#include <stdlib.h>

#include <qobject.h>

#include "conf.h"
#include "everquest.h"
#ifdef USE_DB3
#include "db3conv.h"
#else
#include "gdbmconv.h"
#endif

// forward declarations
struct EQItemDBEntryData;
class EQItemDBIterator;

#ifdef USE_DB3
#define ITEMDBBASE DB3Convenience
#define ITEMDBITBASE DB3Iterator
#else
#define ITEMDBBASE GDBMConvenience
#define ITEMDBITBASE GDBMIterator
#endif

// EQItemDB is the singleton object that manages the 
// Item databases
class EQItemDB : public QObject, protected ITEMDBBASE
{
   Q_OBJECT

 public:
   typedef enum 
   {
     LORE_DB = 0, // is the db of LoreNames used for display (always on)
     NAME_DB = 1, // is the db for storage of the item name
     DATA_DB = 2, // is the db to store the data in ShowEQ format
     RAW_DATA_DB = 4 // contains the raw packet stream for an object
   };

   // Configuration methods
   // Gets and Sets the filenames of the databases
   QString GetDBFile(int dbType);
   bool SetDBFile(int dbType, const QString& dbFileName);

   // Gets and Sets the set of enabled databases
   int GetEnabledDBTypes();
   bool SetEnabledDBTypes(int dbTypes);

   // Upgrade existing data to new format
   bool Upgrade();

   // DB Access methods   
   // Add's items to the enabled databases
   bool AddItem(const itemStruct* item, 
		uint32_t size = sizeof(itemStruct),
		uint16_t flag = 0x0000, 
		bool updated = true);

   // Delete's an item from the enabled databases
   bool DeleteItem(uint16_t itemNr);

   // checks if the item exists in the LORE_DB
   bool ItemExist(uint16_t itemNr);

   // Retrieves the item name from the NAME_DB
   QString GetItemName(uint16_t itemNr);

   // Retrieves the item lore name from the LORE_DB
   QString GetItemLoreName(uint16_t itemNr);
   
   // Retrieves the item data from the DATA_DB
   bool GetItemData(uint16_t itemNr, class EQItemDBEntry** itemData);

   // Retrieves the raw byte stream data for the item
   // returns the size of the byte stream
   int GetItemRawData(uint16_t itemNr, unsigned char** itemData);

   // Reorganize the database (this should rarely be needed)
   bool ReorganizeDatabase(void);

   // shutdown the item database
   void Shutdown();

   // get version info of ItemDB
   static const char* Version();

   // Constructor/Destructor
   EQItemDB();
   ~EQItemDB();

 protected slots:
   void itemShop(const itemInShopStruct* items, uint32_t, uint8_t);
   void itemPlayerReceived(const itemOnCorpseStruct* itemc, uint32_t, uint8_t);
   void tradeItemOut(const tradeItemOutStruct* itemt, uint32_t, uint8_t);
   void tradeItemIn(const tradeItemInStruct* itemr, uint32_t, uint8_t);
   void tradeContainerIn(const tradeContainerInStruct* itemr, uint32_t, uint8_t);
   void tradeBookIn(const tradeBookInStruct* itemr, uint32_t, uint8_t);
   void summonedItem(const summonedItemStruct*, uint32_t, uint8_t);
   void summonedContainer(const summonedContainerStruct*, uint32_t, uint8_t);
   void playerItem(const playerItemStruct* itemp, uint32_t, uint8_t);
   void playerBook(const playerBookStruct* bookp, uint32_t, uint8_t);
   void playerContainer(const playerContainerStruct* containp, uint32_t, uint8_t);
 private:
   // which databases are enabled
   int m_dbTypesEnabled;

   // the filenames specifying the storage location of the databases
   QString m_ItemNameDB;
   QString m_ItemLoreDB;
   QString m_ItemDataDB;
   QString m_ItemRawDataDB;

   // declare friends
   friend class EQItemDBIterator;
};

// EQItemDBIterator is used to iterate over item numbers  in databases
//  Note: This class is only safe to access from a single thread
//        (maintains local state data)
//  Note: As GDBM is currently used, item numbers are retrieved in 
//        a hash order
//  Note: The database is held open for read from the call to 
//        GetFirstItemNumber() until either Done() is called
//        or the iterator is deleted.  During this time no
//        updates can occur to the selected DB.  So keep it brief
class EQItemDBIterator : protected ITEMDBITBASE
{
 public:
   // public constructor
  //  pass in the type of database to iterate over
   EQItemDBIterator(EQItemDB* pItemDB, int DBType);

   // public destructor
   ~EQItemDBIterator();
   
   // Get the first item number in the database
   bool GetFirstItemNumber(uint16_t* itemNr);

   // Get the next item number available in the database
   bool GetNextItemNumber(uint16_t* itemNr);

   // The methods below are provided to allow for faster access
   // to the already opened database and will only work
   // in the time between GetFirstItemNumber() is called
   // and opens the database and Done() is called to close it.
   // Only the ones appropriate for the database type
   // that the iterator was constructed for will work

   // Retrieves the string for the data (LORE_DB and NAME_DB only)
   bool GetItemData(QString& itemData);

   // Retrieves the item data (DATA_DB only)
   bool GetItemData(class EQItemDBEntry** itemData);

   // Retrieves the raw byte stream data for the item
   // returns the size of the byte stream (RAW_DATA_DB only)
   int GetItemData(unsigned char** itemData);
 
   // Closes up the database
   void Done();
 protected:
   // pointer to the ItemDB this iterator is associated with
   EQItemDB* m_pItemDB;

   // Which database is this
   int m_dbType;

   // storage of current item number
   int m_itemNr;
};

// EQItemDBEntry is the class used to access item data
class EQItemDBEntry
{
 public:
   // public constructor (uses datatype declared in itemdb.cpp)
   EQItemDBEntry(uint16_t itemNr, 
                 void* entryData);

   // public destructor so anyone can delete an instance
   ~EQItemDBEntry();

   // Accessor methods to retrieve data
   uint16_t GetUniqueItemNr() { return m_itemNr; }
   QString GetName() { return m_itemName; }
   QString GetLoreName() { return m_itemLore; }
   QString GetIdFile();
   int16_t GetFlag();
   uint8_t GetWeight();
   int8_t GetNoSave();
   int8_t GetNoDrop();
   uint8_t GetSize();
   uint16_t GetIconNr();
   uint32_t GetSlots();
   int32_t GetCost();
   int8_t GetSTR();
   int8_t GetSTA();
   int8_t GetCHA();
   int8_t GetDEX();
   int8_t GetINT();
   int8_t GetAGI();
   int8_t GetWIS();
   int8_t GetMR();
   int8_t GetFR();
   int8_t GetCR();
   int8_t GetDR();
   int8_t GetPR();
   int8_t GetHP();
   int8_t GetMana();
   int8_t GetAC();
   uint8_t GetLight();
   uint8_t GetDelay();
   uint8_t GetDamage();
   uint8_t GetRange();
   uint8_t GetSkill();
   int8_t GetMagic();
   int8_t GetLevel0();
   uint8_t GetMaterial();
   uint32_t GetColor();
   uint16_t GetSpellId0();
   uint16_t GetClasses();
   uint16_t GetRaces();
   uint8_t GetLevel();
   uint16_t GetSpellId();
   int8_t GetCharges();
   uint8_t GetNumSlots();
   uint8_t GetWeightReduction();
   uint8_t GetSizeCapacity();
   int8_t GetStackable();
   int8_t GetEffectType();
   QString GetEffectTypeString();
   uint32_t GetCastTime();
   uint16_t GetSkillModId();
   int8_t GetSkillModPercent();
   bool IsBook();
   bool IsContainer();
   
 protected:
   EQItemDBEntry();
      
 private:
   struct EQItemDBEntryData* m_itemEntryData;
   const char* m_itemName;
   const char* m_itemLore;
   uint16_t  m_itemNr; 
};

#endif // EQITEMDB_H




