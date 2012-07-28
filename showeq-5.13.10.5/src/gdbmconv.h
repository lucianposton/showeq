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

#ifndef GDBMCONV_H
#define GDBMCONV_H

#include <gdbm.h>
#include <qstring.h>

#include "dbcommon.h"

class GDBMIterator;

// A convenience class for handling details of accessing GDBM databases
class GDBMConvenience
{
 public:
  Datum test;

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

   // GDBM file extension
   static QString extension() { return ".dbm"; }

   // get version info of underlying DB library
   static const char* Version();
};

// GDBMIterator is used to iterator over a GDBM database
class GDBMIterator
{
 public:
   // public constructor/destructor
   GDBMIterator();
   ~GDBMIterator();

   // Access methods

   // retrieves the first available key value (in hash order)
   // Note: Opens the database for read
   bool GetFirstKey(GDBMConvenience* gdbmc, 
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
   // pointer to the GDBM database handle
   GDBM_FILE m_dbf;

   // reference to the current key
   datum m_key;
};

#endif // GDBMCONV_H




