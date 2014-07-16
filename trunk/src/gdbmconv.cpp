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

#include "gdbmconv.h"
#include "util.h"

static void displayGDBMError(const char* operation, 
			     const char* filename, 
			     const char *level = "Warning")
{
  if (gdbm_errno != GDBM_NO_ERROR)
    fprintf(stderr, "GDBM %s: %s failed on file '%s': %s\n",
	    level, operation, filename, gdbm_strerror(gdbm_errno));
  else
    fprintf(stderr, "GDBM Other %s: %s failed on file '%s': %s\n",
	    level, operation, filename, strerror(errno));
}

// ********************
//  Note: Funky casts in arguments to gdbm_open effectively removing const
//  is due to the fact that although gdbm_open doesn't modify the passed
//  in string, the developers of GDBM didn't make the file name argument a
//  const char*, but instead have it as a char*.  Don't ask me why.  So 
//  after reading the GDBM source code, and realizing it was benign I've
//  committed the sin of casting away the const'ness.  
//
bool GDBMConvenience::Insert(QString dbName, Datum& key, Datum& data,
			     bool update)
{
  GDBM_FILE dbf;
  bool success = false;
  int flag = update ? GDBM_REPLACE : GDBM_INSERT;
  datum key_, data_;
  key_.dsize = key.size;
  key_.dptr = (char*)key.data;
  data_.dsize = data.size;
  data_.dptr = (char*)data.data;

  // append database extension
  dbName += ".dbm";

  // attempt to open the database for write
  if ((dbf = gdbm_open((char*)(const char*)dbName, 0, GDBM_WRCREAT, 0644, 0))
      != NULL)
  {
    // if successful, then attempt to insert or replace data under key
    switch (gdbm_store(dbf, key_, data_, flag))
      {
      case 0:
      case 1:
	success = true;
	break;
      default:
	displayGDBMError("Insert: gdbm_store", (const char*)dbName);
      }

    // close the database
    gdbm_close(dbf);
  }
  else
  {
    // display GDBM Error
    displayGDBMError("Insert: gdbm_open", (const char*)dbName);
    
    // attempt to diagnose the open for write failure
    diagFileWriteFail(dbName);
  }

  return success;
}

bool GDBMConvenience::Delete(QString dbName, Datum& key)
{
  GDBM_FILE dbf;
  bool success = false;
  datum key_;
  key_.dsize = key.size;
  key_.dptr = (char*)key.data;

  // append database extension
  dbName += ".dbm";

  // attempt to open the database for write
  if ((dbf = gdbm_open((char*)(const char*)dbName, 0, GDBM_WRITER, 0644, 0))
      != NULL)
  {
    // if successful, then attempt to insert or replace data under key
    if (gdbm_delete(dbf, key_) == 0)
      success = true;
    else
      displayGDBMError("Delete: gdbm_delete", (const char*)dbName);

    // close the database
    gdbm_close(dbf);
  }
  else
  {
    // display GDBM Error
    displayGDBMError("Delete: gdbm_open", (const char*)dbName);
    
    // attempt to diagnose the open for write failure
    diagFileWriteFail(dbName);
  }

  return success;
}

bool GDBMConvenience::IsEntryExist(QString dbName, Datum& key)
{
  GDBM_FILE dbf;
  datum data_;
  bool found = false;
  datum key_;
  key_.dsize = key.size;
  key_.dptr = (char*)key.data;

  // append database extension
  dbName += ".dbm";

  // attempt to open the database for read
  if ((dbf = gdbm_open((char*)(const char*)dbName, 0, GDBM_READER,
                       0644, 0)) != NULL)
  {
    // attempt to retrieve the entry from the database
    data_ = gdbm_fetch(dbf, key_);

    // close the database
    gdbm_close(dbf);

    // if there is data, then an entry exists.
    if (data_.dptr != NULL)
    {
      // free the data
      free(data_.dptr);
  
      // note the result
      found = true;
    }
  }

  return found;
}

bool GDBMConvenience::GetEntry(QString dbName, Datum& key, Datum& data)
{
  GDBM_FILE dbf;
  bool found = false;
  datum key_, data_;
  key_.dsize = key.size;
  key_.dptr = (char*)key.data;

  // append database extension
  dbName += ".dbm";

  // attmpet to open the database for read
  if ((dbf = gdbm_open((char*)(const char*)dbName, 0, GDBM_READER,
                       0644, 0)) != NULL)
  {
    // attempt to retrieve the entry from the database
    data_ = gdbm_fetch(dbf, key_);

    // close the database
    gdbm_close(dbf);

    // if data was found, then return true
    if (data_.dptr != NULL)
    {
      data.data = (void*)data_.dptr;
      data.size = data_.dsize;
      found = true;
    }
  }
  else
  {
    // display the notice (this isn't necessarily bad)
    displayGDBMError("GetEntry: gdbm_open", (const char*)dbName, "Notice");

    // attempt to diagnose the open for read failure
    diagFileReadFail(dbName);
  }
    
  return found;
}

bool GDBMConvenience::Reorganize(QString dbName)
{
  GDBM_FILE dbf;

  bool success = false;

  // append database extension
  dbName += ".dbm";

  // attempt to open the database for write
  if ((dbf = gdbm_open((char*)(const char*)dbName, 0, GDBM_WRITER, 0644, 0))
      != NULL)
  {
    // if successful, then attempt to insert or replace data under key
    if (gdbm_reorganize(dbf) == 0)
      success = true;
    else
      displayGDBMError("Reorganize: gdbm_reorganize", (const char*)dbName);

    // close the database
    gdbm_close(dbf);
  }
  else
  {
    // display GDBM Error
    displayGDBMError("Reorganize: gdbm_open", (const char*)dbName);
    
    // attempt to diagnose the open for write failure
    diagFileWriteFail(dbName);
  }

  return success;
}

void GDBMConvenience::Release(Datum& data)
{
   if (data.data != NULL)
   {
     // free the data
     free(data.data);
 
     // clear the pointer, just in case
     data.data = NULL;
   }
 }

void GDBMConvenience::Close(QString dbName)
{
  // nothing to do because GDBM Convenience doesn't keep open pointers to
  // databases
}

void GDBMConvenience::Shutdown()
{
  // nothing to do because GDBM Convenience doesn't keep open pointers to
  // databases
}

const char* GDBMConvenience::Version()
{
  return gdbm_version;
}

GDBMIterator::GDBMIterator()
 : m_dbf(NULL)
{
}

GDBMIterator::~GDBMIterator()
{
  // close the database if opened
  if (m_dbf != NULL)
    gdbm_close(m_dbf);
}

bool GDBMIterator::GetFirstKey(GDBMConvenience* gdbmc, 
			       QString dbName,
			       Datum& key)
{
  return GetFirstKey(dbName, key);
}

bool GDBMIterator::GetFirstKey(QString dbName, Datum& key)
{
  datum key_;

  // close any existing db file
  if (m_dbf != NULL)
    gdbm_close(m_dbf);

  // append database extension
  dbName += ".dbm";

  // open the specified DB file for reading
  m_dbf = gdbm_open((char*)(const char*)dbName, 0, GDBM_READER,
                    0644, 0);

  // if failed, then nothing to read
  if (m_dbf == NULL)
  {
    displayGDBMError("GetFirstKey: gdbm_open", (const char*)dbName);

    // attempt to diagnose the open for read failure
    diagFileReadFail(dbName);

    return false;
  }

  // get the first key from the database
  key_ = gdbm_firstkey(m_dbf);

  // if key.dptr is not NULL then there is a record
  if (key_.dptr != NULL)
  {
    key.data = (void*)key_.dptr;
    key.size = key_.dsize;
    
    // allocate a copy so we know where we are
    m_key.dsize = key_.dsize;
    m_key.dptr = (char*)malloc(key_.dsize);
    memcpy((void*)m_key.dptr, (void*)key_.dptr, key_.dsize);

    // return the success
    return true;
  }
 
  return false;
}

bool GDBMIterator::GetNextKey(Datum& nextkey)
{
  // if no database open, then no next key.
  if (m_dbf == NULL)
    return false;

  // if there is no current key, can't find a next one.
  if (m_key.dptr == NULL)
    return false;

  datum nextkey_;

  // retrieve the next key from the database
  nextkey_ = gdbm_nextkey(m_dbf, m_key);

  // free current key;
  free(m_key.dptr);

  // if nextkey.dptr is not NULL then there are more records
  if (nextkey_.dptr != NULL)
  {
    nextkey.data = (void*)nextkey_.dptr;
    nextkey.size = nextkey_.dsize;

    // allocate a copy so we know where we are
    m_key.dsize = nextkey_.dsize;
    m_key.dptr = (char*)malloc(nextkey_.dsize);
    memcpy((void*)m_key.dptr, (void*)nextkey_.dptr, nextkey_.dsize);
    
    return true;
  }

  // clear out the former current key
  m_key.dsize = 0;
  m_key.dptr = NULL;

  return false;
}

bool GDBMIterator::GetData(Datum& data)
{
  datum data_;

  // if no database open, then can't get any data from it
  if (m_dbf == NULL)
    return false;

  // if there is no current key, the no data
  if (m_key.dptr == NULL)
    return false;

  // retrieve the data from the database
  data_ = gdbm_fetch(m_dbf, m_key);

  // if data.dptr is not NULL then there's data.
  if (data_.dptr != NULL)
  {
    data.data = (void*)data_.dptr;
    data.size = data_.dsize;
    return true;
  }

  return false;
}

void GDBMIterator::Release(Datum& data)
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

void GDBMIterator::Done()
{
  // if a databse is open, close it.
  if (m_dbf != NULL)
  {
    // close the database
    gdbm_close(m_dbf);

    // clear the pointer to it
    m_dbf = (GDBM_FILE)NULL;
  }

  // if a key is left over, clean it up
  if (m_key.dptr != NULL)
  {
    // free the memory
    free(m_key.dptr);

    // note that the key has been cleaned up
    m_key.dptr = (char*)NULL;
    m_key.dsize = 0;
  }
}

