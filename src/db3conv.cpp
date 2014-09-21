/*
 * gdbmconv.cpp
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <qstring.h>
#include <qfileinfo.h>

#include "db3conv.h"
#include "util.h"

static void displayDB3Error(int err,
			     const char* operation, 
			     const char* filename, 
			     const char *level = "Warning")
{
  fprintf(stderr, "DB3 %s: %s failed on file '%s': %s\n",
	  level, operation, filename, DbEnv::strerror(err));
}

DB3Convenience::DB3Convenience()
  : m_dbEnv(NULL)
{
  // avoid doing anything that can fail in constructor
}

DB3Convenience::~DB3Convenience()
{
  Shutdown();
}

Db* DB3Convenience::GetDatabase(QString dbName)
{
  // returned pointer to DB objects
  Db* retdbp = (Db*)NULL;

  // append database extension
  dbName += DB3Convenience::extension();

  // first see if we already have it in the cache
  retdbp = m_dbDict[dbName];

  // if we have it, then just return it
  if (retdbp != NULL)
    return retdbp;

  int ret = 0;
  bool openForReadOnly = false;
  int dbEnvFlags = DB_INIT_LOCK | DB_INIT_MPOOL | DB_INIT_LOG;
  int dbOpenFlags = 0;

  // ok, have to create it.
  // Get information about the file, if there is one
  QFileInfo fileInfo(dbName);

  // Get information about the directory the file should be in
  QFileInfo dirInfo(fileInfo.dirPath());

  // if the directory that the db will be opened in doesn't exist, just
  // warn and fail now
  if (!dirInfo.exists())
    {
      fprintf(stderr, "DB3Convenience: Data Directory '%s' doesn't exist.\n",
	      (const char*)dirInfo.absFilePath());

      // nothing more to do, just return NULL
      return (Db*)NULL;
    }

  // make sure the directory is actually a directory
  if (!dirInfo.isDir())
    {
      fprintf(stderr, "DB3Convenience: Data Directory '%s isn't a directory.\n",
	      (const char*)dirInfo.absFilePath());

      // nothing more to do, just return NULL
      return (Db*)NULL;
    }

  // does the file exist
  if (fileInfo.exists())
    { 
      // file exists
      // is the file readable
      if (!fileInfo.isReadable())
	{
	  // if the file isn't readable, no point in going on is there...
	  fprintf(stderr, "DB3Convenience: Data File '%s isn't readable.\n",
		  (const char*)dirInfo.absFilePath());
      
	  // nothing more to do, just return NULL
	  return (Db*)NULL;
	}

      // is the file not writable
      if (!fileInfo.isWritable())
	{
	  openForReadOnly = true;
	  dbOpenFlags |= DB_RDONLY;
	}
    }

  // is the directory writable
  if (dirInfo.isWritable())
  {
    // yes, directory is writable
    dbEnvFlags |= DB_CREATE; // | DB_RECOVER; // possibly do recovery?
    
    // specify to create the file
    dbOpenFlags |= DB_CREATE;

    // create the DbEnv if necessary
    if (m_dbEnv == NULL)
    {
      // create the database environment
      m_dbEnv = new DbEnv(DB_CXX_NO_EXCEPTIONS);
      
      // print additional diagnostics to stder
      m_dbEnv->set_errfile(stderr);
      m_dbEnv->set_errpfx("DB3Convenience");
      
      dbEnvFlags |= DB_CREATE;
      
      // setup the common database environment for all databases opened
      // using this object
      ret = m_dbEnv->open((const char*)dirInfo.absFilePath(), dbEnvFlags,
			  0664);
	
      if (ret != 0)
      {
	// display a human readable error
	fprintf(stderr, "DB3Convenience: DbEnv::open() failed: %s\n", 
		DbEnv::strerror(ret));
	  
	return (Db*)NULL;
      }
    }

    // create the database
    retdbp = new Db(m_dbEnv, DB_CXX_NO_EXCEPTIONS);
  }
  else
  {
    // can't write to the db_home directory, so open read only
    // and don't use an environment
    openForReadOnly = true;
    dbOpenFlags |= DB_RDONLY;

    // create the database
    retdbp = new Db(NULL, DB_CXX_NO_EXCEPTIONS);

    retdbp->set_errfile(stderr);
    retdbp->set_errpfx("DB3Convenience:");

#if 1 // can't do verify with transactions, logging, or locking
  // verify the database before we go any further
  ret = retdbp->verify((const char*)dbName, NULL, NULL, 0);

  if (ret != 0)
    {
      // display a human readable error
      fprintf(stderr, "DB3Convenience: Db::verify() failed on file '%s': %s\n", 
	      (const char*)dbName, DbEnv::strerror(ret));
      if (ret == DB_RUNRECOVERY)
	fprintf(stderr, "DB3Convenience: Please run db_recover on file '%s'\n",
		(const char*)dbName);

      // check if it's a file access problem
      if (openForReadOnly)
	diagFileReadFail(dbName);
      else
	diagFileWriteFail(dbName);

      // delete the database handle since it's not usable
      delete retdbp;

      // return NULL
      return (Db*)NULL;
    }
#endif
  }

  // set flags for the database opened using this Db (make sure DUP isn't set)
  ret = retdbp->set_flags(0);

  if (ret != 0)
    {
      // display a human readable error
      fprintf(stderr, 
	      "DB3Convenience: Db::set_flags(0) failed on file '%s': %s\n", 
	      (const char*)dbName, DbEnv::strerror(ret));

      // delete the database handle since it's not usable
      delete retdbp;

      // return NULL
      return (Db*)NULL;
    }


  // open the database
  //  ret = retdbp->open((const char*)dbName, NULL, DB_HASH, dbOpenFlags, 0664);
  ret = retdbp->open((const char*)dbName, NULL, DB_BTREE, dbOpenFlags, 0664);
    
  if (ret != 0)
    {
      // display a human readable error
      fprintf(stderr, "DB3Convenience: Db::open() failed on file '%s': %s\n", 
	      (const char*)dbName, DbEnv::strerror(ret));

      // check if it's a file access problem
      if (openForReadOnly)
	diagFileReadFail(dbName);
      else
	diagFileWriteFail(dbName);

      // delete the database handle since it's not usable
      delete retdbp;

      // return NULL
      return (Db*)NULL;
    }

  // if successfully retrieved database pointer cache it for future use 
  if (retdbp)
    m_dbDict.insert(dbName, retdbp);

  // return the database pointer
  return retdbp;
}

bool DB3Convenience::Insert(QString dbName, Datum& key, Datum& data,
			     bool update)
{
  Db* db = NULL;
  bool success = false;
  int ret;
  unsigned int flag = update ? 0 : DB_NOOVERWRITE;
  Dbt key_(key.data, key.size);
  Dbt data_(data.data, data.size);

  // attempt to open the database for write
  if ((db = GetDatabase(dbName)) != NULL)
  {
    // if successful, then attempt to insert or replace data under key
    switch (ret = db->put(NULL, &key_, &data_, flag))
      {
      case 0:
      case DB_KEYEXIST:
	success = true;
	break;
      default:
	displayDB3Error(ret, "Insert: put", (const char*)dbName);
      }
  }

  return success;
}

bool DB3Convenience::Delete(QString dbName, Datum& key)
{
  Db* db = NULL;
  bool success = false;
  int ret;
  Dbt key_(key.data, key.size);

  // attempt to open the database for write
  if ((db = GetDatabase(dbName)) != NULL)
  {
    // if successful, then attempt to delete key from database
    switch (ret = db->del(NULL, &key_, 0))
      {
      case 0:
	success = true;
	break;
      default:
	displayDB3Error(ret, "Delete: del", (const char*)dbName);
      }
  }

  return success;
}

bool DB3Convenience::IsEntryExist(QString dbName, Datum& key)
{
  Db* db = NULL;
  bool success = false;
  int ret;
  Dbt key_(key.data, key.size);
  Dbt data_;

  // attempt to open the database for write
  if ((db = GetDatabase(dbName)) != NULL)
  {
    // if successful, then attempt to insert or replace data under key
    switch (ret = db->get(NULL, &key_, &data_, 0))
      {
      case 0:
	success = true;
	break;
      case DB_NOTFOUND:
	success = false;
	break;
      default:
	displayDB3Error(ret, "IsEntryExist: get", (const char*)dbName);
      }
  }

  return success;
}

bool DB3Convenience::GetEntry(QString dbName, Datum& key, Datum& data)
{
  Db* db = NULL;
  bool success = false;
  int ret;
  Dbt key_(key.data, key.size);
  Dbt data_;
  
  // make the get call return memory allocated using malloc()
  data_.set_flags(DB_DBT_MALLOC);

  // attempt to open the database for write
  if ((db = GetDatabase(dbName)) != NULL)
  {
    // if successful, then attempt to insert or replace data under key
    switch (ret = db->get(NULL, &key_, &data_, 0))
      {
      case 0:
	success = true;
	data.data = data_.get_data();
	data.size = data_.get_size();
	break;
      case DB_NOTFOUND:
	success = false;
	break;
      default:
	displayDB3Error(ret, "GetEntry: get", (const char*)dbName);
      }
  }

  return success;
}

bool DB3Convenience::Reorganize(QString dbName)
{
  return true;
}

void DB3Convenience::Release(Datum& data)
{
   if (data.data != NULL)
   {
     // free the data
     free(data.data);
 
     // clear the pointer, just in case
     data.data = NULL;
   }
 }

void DB3Convenience::Close(QString dbName)
{
  // attempt to get the DB from the cache
  Db* db = m_dbDict[dbName];

  // if a database was found, close it.
  if (db != NULL)
  { 
    // close the database
    int ret = db->close(0);

    if (ret != 0)
      displayDB3Error(ret, "Close: close", (const char*)dbName);

    // remove the db from the dictionary
    m_dbDict.remove(dbName);

    // delete the db
    delete db;
  }
}

void DB3Convenience::Shutdown()
{
  // create an iterator over the dictionary of DB's
  Q3DictIterator<Db> it(m_dbDict);

  // attempt to get the DB from the cache
  Db* db;
  QString dbName;
  int ret;

  // iterate over the cached databases, shutting them down 1 at a time
  while ((db = it.current()) != NULL)
  {
    dbName = it.currentKey();

#if 0
    fprintf(stderr, "Closing database '%s', db = %08.8x\n",
	    (const char*)dbName, db);
#endif

    // close the database
    ret = db->close(0);

    if (ret != 0)
      displayDB3Error(ret, "Shutdown: close", (const char*)dbName);

    // increment to next database
    ++it;

    // remove the db from the dictionary
    m_dbDict.remove(dbName);

    // delete the db
    delete db;
  }

  // shutdown the database environment
  if (m_dbEnv != NULL)
  {
    // close the database environment
    m_dbEnv->close(0);

    // delete the environment
    delete m_dbEnv;

    // clear out the pointer to the environment
    m_dbEnv = NULL;
  }
}

const char* DB3Convenience::Version()
{
  return DbEnv::version(NULL, NULL, NULL);
}

DB3Iterator::DB3Iterator()
 : m_db(NULL),
   m_dbc(NULL)
{
}

DB3Iterator::~DB3Iterator()
{
  Done();
}

bool DB3Iterator::GetFirstKey(DB3Convenience* db3c, 
			      QString dbName, 
			      Datum& key)
{
  Db* db = NULL;
  bool success = false;
  int ret;
  Dbt key_;
  Dbt data_;

  // if database was open previously, then we're done with it.
  if ((m_dbc != NULL) || (m_db != NULL) || (m_data.get_data() != NULL))
    Done();
  if (db3c == NULL)
    return false;


  // use the DB3Convenience's copy of the database
  db = db3c->GetDatabase(dbName);

  if (db == NULL)
    return false;

  // stash the database name with extension for later error reporting
  m_dbName = dbName + DB3Convenience::extension();;

  // Acquire a cursor for the database
  ret = db->cursor(NULL, &m_dbc, 0);

  if (ret != 0)
  {
    displayDB3Error(ret, "GetFirstKey: cursor", (const char*)dbName);

    return false;
  }
  
  // make the get call return memory allocated using malloc()
  key_.set_flags(DB_DBT_MALLOC);
  data_.set_flags(DB_DBT_MALLOC);

  ret = m_dbc->get(&key_, &data_, DB_FIRST);

  // if there's nothing in the database, just return false
  if (ret == DB_NOTFOUND)
    return false;

  // display an error on any other error returns
  if (ret != 0)
  {
    displayDB3Error(ret, "GetFirstKey: get", (const char*)m_dbName);

    return false;
  }

  // set the return key
  key.data = key_.get_data();
  key.size = key_.get_size();

  // save the pointer to the data
  m_data = data_;

  return true;
}

bool DB3Iterator::GetNextKey(Datum& nextkey)
{
  // make sure there is a cursor object
  if (!m_dbc)
    return false;

  Db* db = NULL;
  bool success = false;
  int ret;
  Dbt key_;
  Dbt data_;

  // if there is any data left over that the user didn't get, then free it
  if (m_data.get_data() != NULL)
  {
    // free up the memory
    free(m_data.get_data());
    
    // note that it's been free'd
    m_data.set_data(NULL);
    m_data.set_size(0);
  }

  // make the get call return memory allocated using malloc()
  key_.set_flags(DB_DBT_MALLOC);
  data_.set_flags(DB_DBT_MALLOC);

  ret = m_dbc->get(&key_, &data_, DB_NEXT);

  // if there's nothing in the database, just return false
  if (ret == DB_NOTFOUND)
    return false;

  // display an error on any other error returns
  if (ret != 0)
  {
    displayDB3Error(ret, "GetNextKey: get", (const char*)m_dbName);

    return false;
  }

  // set the return key
  nextkey.data = key_.get_data();
  nextkey.size = key_.get_size();

  // save the pointer to the data
  m_data = data_;

  return true;
}

bool DB3Iterator::GetData(Datum& data)
{
  // make sure there is a cursor object
  if (!m_dbc)
    return false;

  if (m_data.get_data() == NULL)
    return false;

  data.data = m_data.get_data();
  data.size = m_data.get_size();

  m_data.set_data(NULL);
  m_data.set_size(0);
  
  return true;
}

void DB3Iterator::Release(Datum& data)
{
   // free any data associated with the data
   if (data.data != NULL)
   {
     // release the data
     free(data.data);

     // clear the pointer, just in case
     data.data = NULL;
   }
}

void DB3Iterator::Done()
{
  // close the database cursor 
  if (m_dbc != NULL)
  {
    m_dbc->close();
 
    m_dbc = NULL;
  }

  // close the database if opened privately by this iterator
  if (m_db != NULL)
  {
    // close the database
    int ret = m_db->close(0);

    if (ret != 0)
      displayDB3Error(ret, "Done: close", (const char*)m_dbName);
  }

  // if there is any data left over that the user didn't get, then free it
  if (m_data.get_data() != NULL)
  {
    // free up the memory
    free(m_data.get_data());
    
    // note that it's been free'd
    m_data.set_data(NULL);
    m_data.set_size(0);
  }
}

