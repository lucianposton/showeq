/*
 * gdbmconv.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#ifndef DBCONV_H
#define DBCONV_H

#include <db_cxx.h>

#include <q3dict.h>
#include <qstring.h>

#include "dbcommon.h"

class DB3Iterator;

// A convenience class for handling details of accessing DB3 databases
class DB3Convenience
{
 public:
  // Constructor
  DB3Convenience();

  // Destructor
  ~DB3Convenience();

   // insert or update data under the key.
   bool Insert(QString dbName, Datum& key, Datum& data, bool update = true);

   // delete data under the key.
   bool Delete(QString dbName, Datum& key);

   // does an entry exist in the database
   bool IsEntryExist(QString dbName, Datum& key);

   // attempt to retrieve the data entry corresponding to key
   bool GetEntry(QString dbName, Datum& key, Datum& data);

   // reorganize the database
   bool Reorganize(QString dbName);

   // release database data
   void Release(Datum& data);

   // Close any cached database handles on the named DB
   void Close(QString dbNmae);

   // shutdown the entire convenience object
   void Shutdown();

   // DB3 file extension
   static QString extension() { return ".db"; }

   // get version info of underlying DB library
   static const char* Version();

 protected:
   
   Db* GetDatabase(QString dbName);

   DbEnv* m_dbEnv;
   Q3Dict<Db> m_dbDict;

   // declare friend
   friend class DB3Iterator;
};

// DB3Iterator is used to iterator over a DB3 database
class DB3Iterator
{
 public:
   // public constructor/destructor
   DB3Iterator();
   ~DB3Iterator();

   // Access methods

   // retrieves the first available key value (in hash order)
   // Note: Opens the database for read
   bool GetFirstKey(DB3Convenience* db3c, 
		    QString dbName, 
		    Datum& key);

   // retrieves the first available key value (in hash order)
   // Note: Opens the database for read
   bool GetFirstKey(QString dbName, 
		    Datum& key);

   // retrieves the next available key (in hash order)
   bool GetNextKey(Datum& nextkey);

   // retrieves the data corresponding to key from the database
   // Note: GetFirstKey must have been called at least once to
   //       open the database.
   bool GetData(Datum& data);

   // release database data
   void Release(Datum& data);
 
   // closes down the database
   void Done();

 private:
   // pointer to the DB3 database name for error reporting
   QString m_dbName;

   // pointer to the DB3 database
   Db* m_db;

   // pointer to the DB3 database cursor
   Dbc* m_dbc;

   // data associated with current position
   Dbt m_data;
};

#endif // DBCONV_H
