/*
 * itemdbtool.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#ifdef __linux__
#include <linux/version.h>
#endif

#include <sys/utsname.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>            // for long GNU-style options

#include <qfileinfo.h>

#include "conf.h"
#include "util.h"
#include "gdbmconv.h"
#include "itemdb.h"

#ifdef USE_DB3
#include "db3conv.h"
#endif

static char *id="@(#) $Id$";

///////////////////////////////////
// TODO:
//  - Add support to export data to CSV files

// Define the itemdbtool version number
#define ITEMDBTOOL_VERSION "1.0.2"

// Define how often to update the user in an operation
#define TOOL_STATUS_UPDATE 200

///////////////////////////////////
//   defines used for option processing
#define OPTION_LIST "dl::f:r:o:e:x:z:c:DRi:UuhvV"

///////////////////////////////////
// For long options without any short (single letter) equivalent, we'll
// assign single char nonprinting character equivalents, as is common
//   for many GNU utilities. 
#define   ITEMDB_LORE_FILENAME_OPTION   10
#define   ITEMDB_NAME_FILENAME_OPTION   11
#define   ITEMDB_DATA_FILENAME_OPTION   12
#define   ITEMDB_RAW_FILENAME_OPTION    13
#define   ITEMDB_DATABASES_ENABLED      14
#define   FORCE_ACTION                  16

static struct option option_list[] = {
  {"help",                     required_argument,  NULL,  'h'},
  {"version",                  no_argument,        NULL,  'v'},
  {"display",                  no_argument,        NULL,  'd'},
  {"list",                     optional_argument,  NULL,  'l'},
  {"delete-record",            no_argument,        NULL,  'D'},
  {"update-records",           no_argument,        NULL,  'U'},
  {"upgrade",                  no_argument,        NULL,  'u'},
  {"item",                     required_argument,  NULL,  'i'},
  {"import-flatfile",          required_argument,  NULL,  'f'},
  {"import-raw-GDBM",          required_argument,  NULL,  'r'},
  {"import-old-GDBM",          required_argument,  NULL,  'o'},
  {"export-raw-record",        required_argument,  NULL,  'e'},
  {"export-csv",               required_argument,  NULL,  'x'},
  {"export-lsv",               required_argument,  NULL,  'z'},
  {"reorganize",               no_argument,        NULL,  'R'},
  {"itemdb-lore-filename",     required_argument,  NULL, ITEMDB_LORE_FILENAME_OPTION},
  {"itemdb-name-filename",     required_argument,  NULL, ITEMDB_NAME_FILENAME_OPTION},
  {"itemdb-data-filename",     required_argument,  NULL, ITEMDB_DATA_FILENAME_OPTION},
  {"itemdb-raw-data-filename", required_argument,  NULL, ITEMDB_RAW_FILENAME_OPTION},
  {"itemdb-databases-enabled", required_argument,  NULL, ITEMDB_DATABASES_ENABLED},
  {"force",                    no_argument,        NULL, FORCE_ACTION},
  {0,                              0,                  0,     0}
};

///////////////////////////////////
// forward delcarations
void displayUsage();
void displayVersion(EQItemDB* itemDB);
void printstat(const QString filename, 
	       const char* message, 
	       bool oknoexist = false);
bool getConfirmation(void);
int upgradeItemDB(EQItemDB* itemDB);
int importFlatFile(EQItemDB* itemDB, 
		   const char* filename, 
		   uint16_t itemNr, 
		   bool force, 
		   bool updateRecords);
int importGDBM(EQItemDB* itemDB, 
	       const char* filename, 
	       uint16_t itemNr, 
	       bool force, 
	       bool updateRecords);
int exportRawRecord(EQItemDB* itemDB, 
		    const char* filename, 
		    uint16_t itemNr);
int exportRecordCSV(EQItemDB* itemDB, 
		    const char* filename, 
		    uint16_t itemNr,
		    char action);
int deleteRecord(EQItemDB* itemDB, 
		 uint16_t itemNr, 
		 bool force);
int reorganize(EQItemDB* itemDB);
int displayRecord(EQItemDB* itemDB, 
		  uint16_t itemNr);
int listRecords(EQItemDB* itemDB, 
		uint16_t itemNr, 
		QString search);

///////////////////////////////////
// global variables
const char* progname;

///////////////////////////////////
// and here the fun begins...
int
main (int argc, char *argv[])
{
  // option processing variables
  int  opt;
  int  option_index = 0;

  // should the action be forced
  bool forceAction = false;

  // should help be displayed
  bool displayHelp = false;

  // should version info be displayed
  bool displayVer = false;

  // the action the user has chosen to perform
  int  action = 0;

  // the argument to the action
  const char* action_arg = NULL;

  // should existing records be updated (default: No)
  bool updateRecords = false;

  // selected itemNr, 0 = no specific item selected
  uint16_t itemNr = 0;

  // enabled DBTypes (default to all)
  int dbTypes = (EQItemDB::LORE_DB | EQItemDB::NAME_DB | 
		 EQItemDB::DATA_DB | EQItemDB::RAW_DATA_DB);

  // search string
  QString search;

  // note program name for later use.
  progname = argv[0];

  // Get an instance of the EQItemDB
  EQItemDB* itemDB = new EQItemDB;
  
  // begin processing options
  while ((opt = getopt_long( argc,
			     argv,
			     OPTION_LIST,
			     option_list,
			     &option_index)) != -1)
    {
      switch(opt)
	{
	case 'd': // display an item
	case 'l': // list items
	case 'f': // import from a flat file db
	case 'r': // import from a itemrawdata.dbm file
	case 'o': // import from an old GDBM file
	case 'e': // export a raw data record
	case 'x': // export database as a CSV file
	case 'z': // export database as a label seperated list
	case 'D': // delete a record
	case 'R': // reorganize database
	case 'u': // upgrade ItemDB to current format
	  // these actions are mutually exclusive, if two chosen, display help
	  if (action != 0)
	    displayHelp = true;
	  else
	  {
	    // save action and it's argument
	    action = opt;
	    action_arg = optarg;
	  }
	  break;
	case 'i':
	  itemNr = (uint16_t)atoi(optarg);
	  break;
	case 'U':
	  updateRecords = true;
	  break;
	case 'v':
	case 'V':
	  displayVer = true;
	  break;
	case ITEMDB_LORE_FILENAME_OPTION:
	  itemDB->SetDBFile(EQItemDB::LORE_DB, optarg);
	  break;
	case ITEMDB_NAME_FILENAME_OPTION:
	  itemDB->SetDBFile(EQItemDB::NAME_DB, optarg);
	  break;
	case ITEMDB_DATA_FILENAME_OPTION:
	  itemDB->SetDBFile(EQItemDB::DATA_DB, optarg);
	  break;
	case ITEMDB_RAW_FILENAME_OPTION:
	  itemDB->SetDBFile(EQItemDB::RAW_DATA_DB, optarg);
	  break;
	case ITEMDB_DATABASES_ENABLED:
	  dbTypes = atoi(optarg);
	  break;
	case FORCE_ACTION:
	  forceAction = true;
	  break;
	case 'h': // display usage info
	default:
	  displayHelp = true;
	  break;
	}
    }

  // if no action selected, display help
  if ((action == 0) && !displayHelp && !displayVer)
  {
    fprintf(stderr, "No action specified\n");
    displayHelp = true;
  }

  // are there extra arguments
  if (optind < argc)
  {
    //yes, there are

    // is the action list with no search argument
    if ((action == 'l') && (action_arg == 0))
    {
      // yes, so use them for the search argument
      while (optind < argc)
	{
	  if (!search.isEmpty())
	    search += " ";
	  
	  // append extra arguments to search string
	  search += argv[optind++];
	}
    }
    else // otherwise display help
    {
      fprintf(stderr, "Extra arguments!\n");
      displayHelp = true;
    }
  }

  // if either the display or delete actions were specified and no 
  // item was specified display help.
  if (((action == 'd') || (action == 'D')) && 
      (itemNr == 0))
    displayHelp = true;

  // result to return from the program (the exit code).
  int result = 0;

  // display the help/usage information if required and exit
  if (displayHelp)
  {
    // display the usage information
    displayUsage(); 

    // return non-zero result
    result = 1;
  }
  else
  {
    // enable the set of dbtypes
    itemDB->SetEnabledDBTypes(dbTypes);
    
    // display the version information
    if (displayVer)
      displayVersion(itemDB);

    // perform the requested action
    switch(action)
    {
    case 'd': // display an item
      result = displayRecord(itemDB, itemNr);
      break;
    case 'l': // list items
    {
      if (action_arg)
	search = action_arg;
      result = listRecords(itemDB, itemNr, search);
      break;
    }
    case 'f': // import from a flat file db
      result = importFlatFile(itemDB, action_arg, itemNr, 
			      forceAction, updateRecords);
      break;
    case 'r': // import from a itemrawdata.dbm file
    case 'o': // import from an old GDBM file
      // the above two options are really the same
      result = importGDBM(itemDB, action_arg, itemNr, 
			  forceAction, updateRecords);
      break;
    case 'e': // export a raw data record
      result = exportRawRecord(itemDB, action_arg, itemNr);
      break;
    case 'x': // export database as a CSV file
      result = exportRecordCSV(itemDB, action_arg, itemNr, action);
      break;
    case 'z': // export database as a label seperated list
      result = exportRecordCSV(itemDB, action_arg, itemNr, action);
      break;
    case 'D': // delete a record
      result = deleteRecord(itemDB, itemNr, forceAction);
      break;
    case 'R': // reorganize database
      result = reorganize(itemDB);
      break;
    case 'u': // upgrade ItemDB to current format
      result = upgradeItemDB(itemDB);

    }
  }

  // shutdown the ItemDB
  itemDB->Shutdown();

  // delete this instance of ItemDB
  delete itemDB;

  return result;
}

void displayUsage()
{
  printf("Usage: %s <OPTIONS>\n", progname);
  printf("A program for manipulating ShowEQ's item databases.\n\n");
  printf("  -f <FILE>, --import-flatfile=FILE    import data from flat file FILE\n");
  printf("  -r <FILE>, --import-raw-GDBM=FILE    import data from raw data GDBM file FILE\n");
  printf("  -o <FILE>, --import-old-GDBM=FILE    import data from old GDBM file FILE\n");
  printf("  -e <FILE>, --export-raw-record=FILE  export raw data record(s) to flat file\n");
  printf("  -x <FILE>, --export-csv=FILE         export item data to a CSV File\n");
  printf("  -z <FILE>, --export-lsv=FILE         export item data to LABELED SV File\n");
  printf("  -d, --display                        display the specified item\n");
  printf("  -l [SEARCH], --list[SEARCH]          list items matching SEARCH or all\n");
  printf("  -D, --delete                         delete the specified item\n");
  printf("  -R                                   reorganizes the database files\n");
  printf("  -i <NUM>, --item=<NUM>               specify the item to manipulate\n");
  printf("  -U, --update-records                 specifies to update existing records.\n");
  printf("  --force                              force an action, don't confirm\n");
  printf("  --itemdb-lore-filename=FILE          Use FILE instead of itemlore.dbm\n");
  printf("  --itemdb-name-filename=FILE          Use FILE instead of itemname.dbm\n");
  printf("  --itemdb-data-filename=FILE          Use FILE instead of itemdata.dbm\n");
  printf("  --itemdb-raw-data-filename=FILE      Use FILE instead of itemrawdata.dbm\n");
  printf("  --itemdb-databases-enabled=DBS       enable DBS set of item databases\n");
  printf("  -h, --help                           display this help and exit\n");
  printf("  -v, --version                        output version information\n");
  printf("\n");
}

void printstat(const QString filename, const char* message, bool oknoexist)
{
  QFileInfo fileInfo(filename);
  
  if (fileInfo.exists())
  {
    if (fileInfo.isDir())
      printf("\t%s directory '%s'\n", message, (const char*)filename);
    else if (fileInfo.isFile())
      printf("\t%s file '%s'\n", message, (const char*)filename);
    else 
      printf ("\t%s file-like entity '%s'\n", 
	      message, (const char*)filename);

    printf("\t  UID: %d(%s), GID: %d(%s), Permissions: %c%c%c%c%c%c%c%c%c\n",
	   fileInfo.ownerId(), (const char*)fileInfo.owner(),
	   fileInfo.groupId(), (const char*)fileInfo.group(),
	   (fileInfo.permission(QFileInfo::ReadUser) ? 'r' : '-'),
	   (fileInfo.permission(QFileInfo::WriteUser) ? 'w' : '-'),
	   (fileInfo.permission(QFileInfo::ExeUser) ? 'x' : '-'),
	   (fileInfo.permission(QFileInfo::ReadGroup) ? 'r' : '-'),
	   (fileInfo.permission(QFileInfo::WriteGroup) ? 'w' : '-'),
	   (fileInfo.permission(QFileInfo::ExeGroup) ? 'x' : '-'),
	   (fileInfo.permission(QFileInfo::ReadOther) ? 'r' : '-'),
	   (fileInfo.permission(QFileInfo::WriteOther) ? 'w' : '-'),
	   (fileInfo.permission(QFileInfo::ExeOther) ? 'x' : '-'));

    printf("\t  Size: %d bytes\n", fileInfo.size());
  }
  else if (!oknoexist)
    printf("\tUnable to stat %s file '%s': %s (%d)\n", 
	   message, (const char*)filename, strerror(errno), errno);
}

void displayVersion(EQItemDB* itemDB)
{
  /////////////////////////////////
  // Display version information
  printf ("%s: Version %s\n", progname, ITEMDBTOOL_VERSION);
  printf ("\tPart of ShowEQ %s\n", VERSION);

  /////////////////////////////////
  // Display copyright information
  printf ("\tCopyright (C) 1999-2001 ShowEQ Contributors\n\n");
  
  printf ("\tShowEQ comes with NO WARRANTY.\n\n");
  
  printf ("\tYou may redistribute copies of ShowEQ under the terms of\n");
  printf ("\tThe GNU General Public License.\n");
  printf ("\tSee: http://www.gnu.org/copyleft/gpl.html for more details...\n\n");
  
  printf ("\tFor updates and information, please visit:\n"
	  "\t\thttp://seq.sourceforge.net\n\n");

  
  /////////////////////////////////
  // Display build information
  printf ("\tBuilt from '%s' on %s at %s\n", __FILE__, __DATE__, __TIME__);
  printf ("\tCVS: %s\n", id);
#ifdef __GNUC__
#ifdef __GNUC_PATCHLEVEL__
  printf ("\t\tUsing GCC version: %d.%d.%d\n", 
	  __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
  printf ("\t\tUsing GCC version: %d.%d\n", 
	  __GNUC__, __GNUC_MINOR__);
#endif
#elif defined(__VERSION__)
  printf ("\t\tUsing c++ version %s\n", __VERSION__);
#endif

#ifdef __KCC
  printf ("\t\tUsing a KAI compiler\n");
#endif

#ifdef __GLIBC__
  printf ("\t\tUsing glibc version: %d.%d\n", __GLIBC__, __GLIBC_MINOR__);
#endif

#ifdef QT_VERSION_STR
  printf ("\t\tUsing Qt version: %s\n", QT_VERSION_STR);
#endif
#ifdef __linux__
  printf ("\t\tUsing headers from linux version: %s\n",
	  UTS_RELEASE);
#endif
  printf ("\t\tUsing EQItemDB: %s\n", EQItemDB::Version());
  printf ("\t\tsizeof(itemStruct) was %d bytes\n", 
	  sizeof(itemStruct));
  printf ("\n");

  /////////////////////////////////
  // Display current system environment information
  struct utsname utsbuff;
  if (uname(&utsbuff) == 0)
    printf ("\tRunning on %s release %s for processor %s\n",
	    utsbuff.sysname, utsbuff.release, 
	    utsbuff.machine);
  printf ("\tUsing GDBM: %s\n", GDBMConvenience::Version());
#ifdef USE_DB3
  printf ("\tUsing DB3: %s\n", DB3Convenience::Version());
#endif
  printf ("\n");

  /////////////////////////////////
  // Display database file information
  printstat(LOGDIR, "LOGDIR");
  
  QString dbExt;

#ifdef USE_DB3
  dbExt = DB3Convenience::extension();
#else
  dbExt = GDBMConvenience::extension();
#endif

  QString filename = itemDB->GetDBFile(EQItemDB::DATA_DB) + dbExt;

  QFileInfo fileInfo(filename);
  printstat(fileInfo.dirPath(true), "dirPath(DATA_DB)");

  printstat(filename, "DATA_DB");
  printstat(itemDB->GetDBFile(EQItemDB::RAW_DATA_DB) + dbExt, 
	    "RAW_DATA_DB");
#ifndef USE_DB3
  // only exist with GDBM, and use is superceded, 
  // so it's ok if they don't exist
  printstat(itemDB->GetDBFile(EQItemDB::LORE_DB) + dbExt, "LORE_DB", true);
  printstat(itemDB->GetDBFile(EQItemDB::NAME_DB) + dbExt, "NAME_DB", true);
#endif
  printf ("\n");
}

bool getConfirmation(void)
{
  bool valid = false;
  const char* result;
  bool value = false;
  char buff[128];
  size_t buffvaluelen;

  while (!valid)
    {
      // display prompt
      printf("Please enter [Yes/n]: ");

      // get answer
      result = fgets(buff, sizeof(buff), stdin);

      // if error or EOF, answer is false
      if (result == NULL)
	valid = true;
      else
      {
	buffvaluelen = strlen(buff);

	// remove any trailing newline
	for (size_t i = 0; i < buffvaluelen; i++)
	  if ((buff[i] == '\n') ||
	      (buff[i] == '\r'))
	    buff[i] = '\0';

	if (strcmp(buff, "Yes") == 0)
	{
	  value = true;
	  valid = true;
	}
	else
	{
	  if ((buff[0] == 'N') || 
	      (buff[0] == 'n'))
	    valid = true;
	  else
	    printf("Answer '%s' is invalid!\n", buff);
	}
      }
    }

  return value;
}

int upgradeItemDB(EQItemDB* itemDB)
{
  if (itemDB->Upgrade())
    return 0;
  else
    return 1;
}

int importFlatFile(EQItemDB* itemDB, 
		   const char* filename, 
		   uint16_t itemNr, 
		   bool force,
		   bool update)
{
  int result = 0;
  int count = 0;
  int lookedat = 0;
  FILE* idb;
  itemStruct i;

  // if not forced, make sure the user really means it.
  if (!force)
  {
    if (itemNr == 0)
      printf("Are You Sure you want to import items from flat file '%s'\n",
	     filename);
    else
      printf("Are You Sure you want to import item %d from flat file '%s'\n",
	     itemNr, filename);

    if (!getConfirmation())
      return result;
  }

  // open the old flat file
  if ((idb = fopen (filename, "r")) != NULL)
  {
    int filesize;
    
    // set file position to end of file
    fseek(idb, 0, SEEK_END);
    
    // retrieve offset of end of file
    filesize = ftell(idb);

    // restore file position to the beginning of the file
    fseek(idb, 0, SEEK_SET);

    // the filesize must be a multiple of sizeof(itemStruct) otherwise 
    // it's corrupt
    if ((filesize % sizeof(itemStruct)) != 0)
    {
      fprintf(stderr, 
	      "%s: Error Flat File '%s' is corrupt!\n"
	      "\tThe size of the file (%d) is not a multiple of %d!\n",
	      progname, filename, filesize, sizeof(itemStruct));
      return 2;
    }

    // if an item number was specified, search for and only insert that item
    if (itemNr != 0)
    {
      bool found = false;
      
      // iterate through the records in the file
      while (fread (&i, sizeof (itemStruct), 1, idb))
      {
	// increment looked at count
	lookedat++;
	
	// display a status update
	if ((lookedat % TOOL_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\tExamined %d items from file '%s' so far...\n",
		  lookedat, filename);
	
	// if an item number was specified, check if this record is it
	if (itemNr != i.itemNr)
	  continue; // not the one, continue

	// found the entry being searched for
	found = true;

	// import item
	if (itemDB->AddItem(&i, update))
	  count++; // increment count
	else
	  fprintf(stderr, "%s: Failure on insert of Item %d from file %s\n",
		  progname, itemNr, filename);

	// found the one, finished
	break;
      }

      if (!found)
      {
	fprintf(stderr, "%s: Flat File '%s' doesn't contain item number %d!\n",
		progname, filename, itemNr);
	result = 3;
      }
    }
    else
    {
      // insert all records in the file

      // iterate through the records in the file
      while (fread (&i, sizeof (itemStruct), 1, idb))
      {
	// import item
	if (itemDB->AddItem(&i, update))
	  count++; // increment count
	else
	  fprintf(stderr, "%s: Failure on insert of Item %d from file '%s'\n",
		  progname, i.itemNr, filename);
	
	// display a status update
	if ((count % TOOL_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\tImported %d items from file '%s' so far...\n", 
		 count, filename);
      }
    }

    fprintf(stderr, "%s: Inserted %d item(s)\n", progname, count);
    
    // close the file
    fclose (idb);
  }
  else
  {
    fprintf(stderr, "%s: Error Opening Flat File: '%s': %s\n",
	    progname, filename, strerror(errno));
    result = 1;
  }

  return result;
}

int importGDBM(EQItemDB* itemDB, 
	       const char* filename, 
	       uint16_t itemNr, 
	       bool force,
	       bool update)
{
  int result = 0;
  Datum key, data;
  int count = 0;

  // if not forced, make sure the user really means it.
  if (!force)
  {
    if (itemNr == 0)
      printf("Are You Sure you want to import items from GDBM file '%s'\n",
	     filename);
    else
      printf("Are You Sure you want to import item %d from GDBM file '%s'\n",
	     itemNr, filename);

    if (!getConfirmation())
      return result;
  }

  if (itemNr != 0)
  {
    GDBMConvenience gdbm;

    // setup key datum to use for query
    key.size = sizeof(itemNr);
    key.data = (char*)&itemNr;

    // retrieve entry from GDBM file
    if (gdbm.GetEntry(filename, key, data))
    {
      // an entry has been found
      // make sure the entry is the correct size
      if (data.size == sizeof(itemStruct))
      {
	// yes, insert it if it doesn't already exist
	if (itemDB->AddItem((itemStruct*)data.data, update))
	  count++; // increment count
	else
	  fprintf(stderr, "%s: Failure on insert of Item %d from file %s\n",
		  progname, itemNr, filename);
      }
      else
      {
	fprintf(stderr, 
		"%s: Item %d from file '%s' has incorrect size %d (not %d)\n",
		progname, itemNr, filename, data.size, sizeof(itemStruct));
	result = 2;
      }

      // release the database data
      gdbm.Release(data);
    }
    else
    {
      fprintf(stderr,
	      "%s: Error retrieving item %d from file '%s'\n",
	      progname, itemNr, filename);
      result = 1;
    }
  }
  else
  {
    GDBMIterator gdbmit;
    bool hasData;
    int lookedat = 0;

    // initialize the iterator and retrieve the first key
    hasData = gdbmit.GetFirstKey(filename, key);

    if (!hasData)
    {
      fprintf(stderr, "%s: Couldn't retrieve GDBM file: %s\n",
	      progname, filename);
      return 3;
    }

    while(hasData)
    {
      // retrieve data associated with the key
      if (gdbmit.GetData(data))
      {
	// increment the count of items that have been looked at...
	lookedat++;

	// does the entry have the correct size?
	if (data.size == sizeof(itemStruct))
	{
	  // yes, insert item if it doesn't already exist
	  if (itemDB->AddItem((itemStruct*)data.data, update))
	    count++; // increment count
	  else
	    fprintf(stderr, "%s: Failure on insert of Item %d from file %s\n",
		    progname, *(uint16_t*)key.data, filename);
	}
	else
	{
	  // no, print warning
	  fprintf(stderr, 
		  "Warning Item %d from file '%s' has incorrect size %d (not %d): Not Inserted\n",
		  *(uint16_t*)key.data, filename, data.size, sizeof(itemStruct));
	  result = 4;
	}
	
	// display a status update
	if ((lookedat % TOOL_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\tExamined %d items and Imported %d items from file '%s' so far...\n", 
		 lookedat, count, filename);
	
	// release the data
	gdbmit.Release(data);
      }
      else
      {
	// no data associated with key
	fprintf(stderr,
		"Warning Item %d from file '%s' doesn't have data\n",
		*(uint16_t*)key.data, filename);
	result = 5;
      }
      
      // release the key
      gdbmit.Release(key);

      // retrieve the next key
      hasData = gdbmit.GetNextKey(key);
    }
  
    // release the last key
    gdbmit.Release(key);

    // shutdown the iterator
    gdbmit.Done();
  }

  fprintf(stderr, "%s: Inserted %d item(s)\n", progname, count);

  return result;
}

int exportRawRecord(EQItemDB* itemDB, 
		 const char* filename, 
		 uint16_t itemNr)
{
  int result = 0;
  FILE* outfile;
  int count = 0;
  int itemsize;
  unsigned char* rawData = NULL;

  // open the output file
  outfile = fopen(filename, "w");

  // if unable to open it, just error out now
  if (outfile == NULL)
  {
    fprintf(stderr, "%s: Unable to open output file '%s'\n",
	    progname, filename);
    return 1;
  }

  // if an item number was specified, just get that item
  if (itemNr != 0)
  {
    // retrieve item data
    itemsize = itemDB->GetItemRawData(itemNr, &rawData);
    
    // was there any data
    if ((itemsize <= 0) || (rawData == NULL))
    {
      // no  data, report the error
      fprintf(stderr, "%s: No Raw Data associated with item %d\n",
	      progname, itemNr);
      result = 2;
    }
    else
    {
      // attempt to write the data to the output file
      if (fwrite((const void*)rawData, itemsize, 1, outfile) == 1)
	count++;  // increment the count
      else
      {
	// unable to write, report the error
	fprintf(stderr, "%s: Unable to write raw data for item %d to file '%s'\n",
		progname, itemNr, filename);
	result = 3;
      }
    }
    
    // delete the data
    delete [] rawData;
  }
  else
  {
    // They didn't specify a specific item, so dump them all.

    // retrieve an iterator over the RAW_DATA_DB
    EQItemDBIterator* it = new EQItemDBIterator(itemDB, EQItemDB::RAW_DATA_DB);

    // make sure there is an iterator
    if (it == NULL)
    {
      // if unable to retrieve an iterator on the item raw data db
      // then nothing else to do
      fprintf(stderr, "%s: Unable to open iterator on item raw data db!\n",
	      progname);
      result = 4;
    }
    else
    {
      uint16_t nextItemNr, currentItemNr;
      bool hasNext;

      // retrieve the first item number
      hasNext = it->GetFirstItemNumber(&nextItemNr);

      // keep going until no more items
      while (hasNext)
      {
	// the next item number is now the current item number
	currentItemNr = nextItemNr;

	// attempt to retrive a new next item number
	hasNext = it->GetNextItemNumber(&nextItemNr);

	// retrieve item raw data
	itemsize = it->GetItemData(&rawData);

	// if there's no data, warn about it
	if ((itemsize <= 0) || (rawData == NULL))
	{
	  fprintf(stderr, "Warning: No Raw Data associated with item %d\n",
		  currentItemNr);
	  result = 5;
	}
	else
	{
	  // make sure the data is the correct size
	  if (itemsize != sizeof(itemStruct))
	  {
	    fprintf(stderr, 
		    "Warning: size (%d) of Raw Data for item %d is not equal sizeof(itemStruct) (%d): Record Not Written!\n",
		    itemsize, currentItemNr, sizeof(itemStruct));
	    result = 6;
		    
	  }
	  else
	  {
	    // attempt to write the data
	    if (fwrite((const void*)rawData, itemsize, 1, outfile) == 1)
	      count++;
	    else
	    {
	      fprintf(stderr, "%s: Unable to write raw data for item %d to file '%s'\n",
		      progname, itemNr, filename);
	      result = 7;
	    }
	  }
	
	  // display a status update
	  if ((count % TOOL_STATUS_UPDATE) == 0)
	    fprintf(stderr, "\tExported %d items to file '%s' so far...\n", 
		   count, filename);
	}
	
	delete [] rawData;
      }
    }
  }
  
  fprintf(stderr, "%s: Wrote %d itemStruct's (%d bytes each) to file '%s'.\n",
	  progname, count, sizeof(itemStruct), filename);

  return result;
}

int
dumpItemCSV(EQItemDB* itemDB, 
			FILE* fh,
		    uint16_t itemNr)
{
	QString nameString;
	QString loreString;
	bool hasEntry = false;
	EQItemDBEntry* entry = NULL;

	nameString = itemDB->GetItemName(itemNr);
	loreString = itemDB->GetItemLoreName(itemNr);
	hasEntry = itemDB->GetItemData(itemNr, &entry);

	fprintf (fh, "\"%d\",", itemNr);
	if (!nameString.isEmpty()) fprintf (fh, "\"%s\",", (const char*)nameString);
	else fprintf (fh, "\"\",");

	if (hasEntry)
	{
		if (loreString[0] == '*')
			{
				fprintf (fh, "\"%s\",", (const char*)loreString);
				fprintf (fh, "\"1\",");
			}
		else
			{
				fprintf (fh, "\"\",");
				fprintf (fh, "\"0\",");
			}
	
			fprintf (fh, "\"%.1f\",", (entry->GetWeight())/10.0);
			fprintf (fh, "\"%#06x\",", entry->GetFlag());
			fprintf (fh, "\"%d\",", entry->GetSize());
			fprintf (fh, "\"%d\",", entry->GetSlots());
			fprintf (fh, "\"%d\",", entry->GetIconNr());
	
			if (entry->GetNoDrop() == 0) fprintf (fh, "\"1\",");
			else fprintf (fh, "\"0\",");
	
			if (entry->GetNoSave() == 0) fprintf (fh, "\"1\",");
			else fprintf (fh, "\"0\",");
	
			if (entry->IsBook() == 0) fprintf (fh, "\"1\",");
			else fprintf (fh, "\"0\",");
			
				if (entry->GetMagic() == 1)	fprintf (fh, "\"1\",");
				else fprintf(fh, "\"0\",");
				if (entry->GetLight())		fprintf (fh, "\"%d\",", entry->GetLight());
				else fprintf(fh, "\"\",");
				if (entry->GetSTR())		fprintf (fh, "\"%d\",", entry->GetSTR());
				else fprintf(fh, "\"\",");
				if (entry->GetSTA())		fprintf (fh, "\"%d\",", entry->GetSTA());
				else fprintf(fh, "\"\",");
				if (entry->GetCHA())		fprintf (fh, "\"%d\",", entry->GetCHA());
				else fprintf(fh, "\"\",");
				if (entry->GetDEX())		fprintf (fh, "\"%d\",", entry->GetDEX());
				else fprintf(fh, "\"\",");
				if (entry->GetINT())		fprintf (fh, "\"%d\",", entry->GetINT());
				else fprintf(fh, "\"\",");
				if (entry->GetAGI())		fprintf (fh, "\"%d\",", entry->GetAGI());
				else fprintf(fh, "\"\",");
				if (entry->GetWIS())		fprintf (fh, "\"%d\",", entry->GetWIS());
				else fprintf(fh, "\"\",");
				if (entry->GetMR())			fprintf (fh, "\"%d\",", entry->GetMR());
				else fprintf(fh, "\"\",");
				if (entry->GetFR())			fprintf (fh, "\"%d\",", entry->GetFR());
				else fprintf(fh, "\"\",");
				if (entry->GetCR())			fprintf (fh, "\"%d\",", entry->GetCR());
				else fprintf(fh, "\"\",");
				if (entry->GetDR())			fprintf (fh, "\"%d\",", entry->GetDR());
				else fprintf(fh, "\"\",");
				if (entry->GetPR())			fprintf (fh, "\"%d\",", entry->GetPR());
				else fprintf(fh, "\"\",");
				if (entry->GetHP())			fprintf (fh, "\"%d\",", entry->GetHP());
				else fprintf(fh, "\"\",");
				if (entry->GetMana())		fprintf (fh, "\"%d\",", entry->GetMana());
				else fprintf(fh, "\"\",");
				if (entry->GetAC())			fprintf (fh, "\"%d\",", entry->GetAC());
				else fprintf(fh, "\"\",");
				if (entry->GetDelay())		fprintf (fh, "\"%d\",", entry->GetDelay());
				else fprintf(fh, "\"\",");
				if (entry->GetDamage())
				{
					fprintf (fh, "\"%d\",", entry->GetDamage());
					fprintf (fh, "\"%d\",", entry->GetSkill());
				}
				else
				{
					fprintf (fh, "\"\",");
					fprintf (fh, "\"\",");
				}
				if (entry->GetRange())		fprintf (fh, "\"%d\",", entry->GetRange());
				else fprintf(fh, "\"\",");
	
				if (entry->GetSpellId0()) 	fprintf (fh, "\"%d\",", entry->GetSpellId0());
				else fprintf (fh, "\"\",");
				if (entry->GetLevel())		fprintf (fh, "\"%d\",", entry->GetLevel());
				else fprintf (fh, "\"\",");
				if (entry->GetCharges())	fprintf (fh, "\"%d\",", entry->GetCharges());
				else fprintf (fh, "\"\",");
				
				fprintf (fh, "\"%d\",", entry->GetClasses());
	
				if (entry->IsContainer()) fprintf (fh, "\"1\",");
				else fprintf (fh, "\"0\",");
				
				if (entry->GetNumSlots())			fprintf (fh, "\"%d\",", entry->GetNumSlots());
				else fprintf (fh, "\"\",");
				if (entry->GetSizeCapacity())		fprintf (fh, "\"%d\",", entry->GetSizeCapacity());
				else fprintf (fh, "\"\",");
				if (entry->GetWeightReduction())	fprintf (fh, "\"%d\",", entry->GetWeightReduction());
				else fprintf (fh, "\"\",");
				
				if (!entry->IsContainer())	fprintf (fh, "\"%d\"", entry->GetRaces());
				else fprintf (fh, "\"\"");
	
			fprintf (fh, "\n");
	
			// don't need the entry anymore, delete it.
			delete entry;
		}
		else return -1;
	
		return 0;
}

int
dumpItemLabeled(EQItemDB* itemDB, 
			FILE* fh,
		    uint16_t itemNr)
{
	QString nameString;
	QString loreString;
	bool hasEntry = false;
	EQItemDBEntry* entry = NULL;

	nameString = itemDB->GetItemName(itemNr);
	loreString = itemDB->GetItemLoreName(itemNr);
	hasEntry = itemDB->GetItemData(itemNr, &entry);

	fprintf (fh, "itemNr:%d:", itemNr);
	if (!nameString.isEmpty())
		fprintf (fh, "Name:%s:", (const char*)nameString);

	if (hasEntry)
	{
		if (loreString[0] == '*')
		{
			fprintf (fh, "Lore:%s:", (const char*)loreString);
			fprintf (fh, "LORE:1:");
		}
		fprintf (fh, "Weight:%.1f:", (entry->GetWeight())/10.0);
		fprintf (fh, "Flag:%#06x:", entry->GetFlag());
		fprintf (fh, "Size:%d:", entry->GetSize());
		fprintf (fh, "Slot:%d:", entry->GetSlots());
		fprintf (fh, "Icon:%d:", entry->GetIconNr());
		if (entry->GetNoDrop() == 0)
			fprintf (fh, "NODROP:1:");
		if (entry->GetNoSave() == 0)
			fprintf (fh, "NORENT:1:");

		if (entry->IsBook() == 0)
		{
			if (entry->GetMagic() == 1)
				fprintf (fh, "MAGIC:1:");
			if (entry->GetLight())
				fprintf (fh, "Light:%d:", entry->GetLight());
			if (entry->GetSTR())
				fprintf (fh, "Str:%d:", entry->GetSTR());
			if (entry->GetSTA())
				fprintf (fh, "Sta:%d:", entry->GetSTA());
			if (entry->GetCHA())
				fprintf (fh, "Cha:%d:", entry->GetCHA());
			if (entry->GetDEX())
				fprintf (fh, "Dex:%d:", entry->GetDEX());
			if (entry->GetINT())
				fprintf (fh, "Int:%d:", entry->GetINT());
			if (entry->GetAGI())
				fprintf (fh, "Agi:%d:", entry->GetAGI());
			if (entry->GetWIS())
				fprintf (fh, "Wis:%d:", entry->GetWIS());
			if (entry->GetMR())
				fprintf (fh, "SvM:%d:", entry->GetMR());
			if (entry->GetFR())
				fprintf (fh, "SvF:%d:", entry->GetFR());
			if (entry->GetCR())
				fprintf (fh, "SvC:%d:", entry->GetCR());
			if (entry->GetDR())
				fprintf (fh, "SvD:%d:", entry->GetDR());
			if (entry->GetPR())
				fprintf (fh, "SvP:%d:", entry->GetPR());
			if (entry->GetHP())
				fprintf (fh, "HP:%d:", entry->GetHP());
			if (entry->GetMana())
				fprintf (fh, "Mana:%d:", entry->GetMana());
			if (entry->GetAC())
				fprintf (fh, "AC:%d:", entry->GetAC());
			if (entry->GetDelay())
				fprintf (fh, "Del:%d:", entry->GetDelay());
			if (entry->GetDamage())
			{
				fprintf (fh, "Dam:%d:", entry->GetDamage());
				fprintf (fh, "Skill:%d:", entry->GetSkill());
			}
			if (entry->GetRange())
				fprintf (fh, "Range:%d:", entry->GetRange());
			if (entry->GetSpellId0() != ITEM_SPELLID_NOSPELL && entry->GetSpellId0() != 0)
			{
				fprintf (fh, "Effect:%d:", entry->GetSpellId0());
				if (entry->GetLevel())
					fprintf (fh, "EffLvl:%d:", entry->GetLevel());
				if (entry->GetCharges())
					fprintf (fh, "EffChg:%d:", entry->GetCharges());
			}
			fprintf (fh, "Class:%d:", entry->GetClasses());
			if (entry->IsContainer())
			{
				if (entry->GetNumSlots())
					fprintf (fh, "ConSlot:%d:", entry->GetNumSlots());
				if (entry->GetSizeCapacity())
					fprintf (fh, "ConCap:%d:", entry->GetSizeCapacity());
				if (entry->GetWeightReduction())
					fprintf (fh, "ConWr:%d:", entry->GetWeightReduction());
			} else
			{
				// Race only if not a container
				fprintf (fh, "Race:%d:", entry->GetRaces());
			}
		}
		fprintf (fh, "\n");

		// don't need the entry anymore, delete it.
		delete entry;
	} else {
		return -1;
	}
	return 0;
}

int exportRecordCSV(EQItemDB* itemDB, 
	const char* filename,
	uint16_t itemNr,
	char action)
{
	int result = 0;
	FILE* outfile;
	int count = 0;

	// open the output file
	outfile = fopen(filename, "w");

	// if unable to open it, just error out now
	if (outfile == NULL)
	{
		fprintf(stderr, "%s: Unable to open output file '%s'\n", progname, filename);
			return 1;
	}

	// if an item number was specified, just get that item
	if (itemNr != 0)
	{
		// retrieve item data
		dumpItemCSV(itemDB, outfile, itemNr);
		count++;
	}
	else
	{
		// They didn't specify a specific item, so dump them all.

		// retrieve an iterator over the RAW_DATA_DB
		EQItemDBIterator* it = new EQItemDBIterator(itemDB, EQItemDB::RAW_DATA_DB);

		// make sure there is an iterator
		if (it == NULL)
		{
			// if unable to retrieve an iterator on the item raw data db
			// then nothing else to do
			fprintf(stderr, "%s: Unable to open iterator on item raw data db!\n", progname);
			result = 4;
		}
		else
		{
			uint16_t nextItemNr, currentItemNr;
			bool hasNext;

			// retrieve the first item number
			hasNext = it->GetFirstItemNumber(&nextItemNr);

			// keep going until no more items
			while (hasNext)
			{
				// the next item number is now the current item number
				currentItemNr = nextItemNr;

				// attempt to retrive a new next item number
				hasNext = it->GetNextItemNumber(&nextItemNr);
				// retrieve item data

				if (action == 'x') dumpItemCSV(itemDB, outfile, currentItemNr);
				else if (action == 'z') dumpItemLabeled(itemDB, outfile, currentItemNr);
				count++;
			}
		}
	}
	fprintf(stderr, "%s: Wrote %d items to file '%s'.\n",
		progname, count, filename);

	return result;
}

int deleteRecord(EQItemDB* itemDB, 
		 uint16_t itemNr, 
		 bool force)
{
  int result = 0;

  if (!force)
  {
    printf("Are You Sure you want to delete item: %d '%s'\n",
	   itemNr, (const char*)itemDB->GetItemLoreName(itemNr));
  
    if (!getConfirmation())
      return result;
  }
  
  // delete the item from the database
  if (itemDB->DeleteItem(itemNr))
    printf("Deleted Item: %d\n", itemNr);
  else
  {
    result = 1;
    fprintf(stderr, "%s: Failed to delete item: %d\n", progname, itemNr);
  }

  return result;
}

int reorganize(EQItemDB* itemDB)
{
  // just ask the ItemDB to reorganize the enabled databases
  if (itemDB->ReorganizeDatabase())
    printf("Databases Reorganized.\n");
  else 
    fprintf(stderr, "%s: Failed to reorganize databases!\n", progname);

  return 0;
}

int listRecords(EQItemDB* itemDB, uint16_t itemNr, QString search)
{
  // Display what is being done
  printf("Listing item(s)");
  if (!search.isEmpty())
    printf(" with names containing '%s'", (const char*)search);
  if (itemNr != 0)
    printf(" with item #%d", itemNr);
  printf("\n");

  // print the header
  printf("Item#  Name  (Info)\n");
  printf("-----  -------------------------------------------\n");

  // retrieve an iterator over the LORE_DB
  EQItemDBIterator* it = new EQItemDBIterator(itemDB, EQItemDB::DATA_DB);

  // if unsuccessful then can't do any more
  if (it == NULL)
  {
    printf ("Warning: No DataBase");
    return -1;
  }

  bool hasNext = true;  
  uint16_t currentItemNr;
  uint16_t nextItemNr;
  QString nameString;
  QString loreString;
  bool hasEntry = false;
  EQItemDBEntry* entry = NULL;
  int counter = 0;

  QString info;

  // retrieve the first number
  hasNext = it->GetFirstItemNumber(&nextItemNr);
  
  // iterate until there isn't a next item number
  while (hasNext)
  {
    // delete the entry from the last go around
    delete entry;

    // fill the entry with a NULL just in case
    //    entry = NULL;

    // the next item number is now the current item number
    currentItemNr = nextItemNr;

    // retrieve entry from iterator, since it is on the data database
    hasEntry = it->GetItemData(&entry);

    // attempt to retrieve a new next item number
    hasNext = it->GetNextItemNumber(&nextItemNr);

    if ((itemNr != 0) && (itemNr != currentItemNr))
      continue;

    // attempt to retrieve the item name and lore
    loreString = entry->GetLoreName();
    nameString = entry->GetName();

    // A search strin was specified
    if (!search.isEmpty())
    {
      if ((loreString.find(search, 0, false) == -1) &&
	  (nameString.find(search, 0, false) == -1))

	continue;
    }

    printf("%5d  ", currentItemNr);

    // only display a name if we have the item name
    if (!nameString.isEmpty())
      printf ("%s ", 
              (const char*)nameString);
    else
      printf("%s ",
	     (const char*)loreString);
    
    info = "";

    // if we have more data for this item, print it
    if (hasEntry)
    {
      if (entry->IsBook())
      {
	if (!info.isEmpty())
	  info += ", ";
	info += "BOOK";
      }
      if (entry->IsContainer())
      {
	if (!info.isEmpty())
	  info += ", ";
	info += "CONTAINER";
      }
      if (entry->GetNoDrop()==0)
      {
	if (!info.isEmpty())
	  info += ", ";
        info += "NO-DROP";
      }
      if (entry->GetNoSave()==0)
      {
	if (!info.isEmpty())
	  info += ", ";
        info += "NO-SAVE ";
      }
      if (entry->GetMagic()==1)
      {
	if (!info.isEmpty())
	  info += ", ";
        info += "MAGIC";
      }
      if (loreString[0] == '*')
      {
	if (!info.isEmpty())
	  info += ", ";
        info +="LORE";
      }

      if (entry->GetAC())
      {
	if (!info.isEmpty())
	  info += ", ";
	info += "AC: " + QString::number(entry->GetAC());
      }

      if (entry->GetDamage())
      {
	if (!info.isEmpty())
	  info += ", ";
	info += "Dmg: " + QString::number(entry->GetDamage());
      }

      if (entry->GetDelay())
      {
	if (!info.isEmpty())
	  info += ", ";
	info += "Dly: " + QString::number(entry->GetDelay());
      }

      if (entry->GetRange())
      {
	if (!info.isEmpty())
	  info += ", ";
	info += "Rng: " + QString::number(entry->GetRange());
      }

      if (entry->IsContainer())
      {
	if (entry->GetNumSlots())
	{
	  if (!info.isEmpty())
	    info += ", ";
	  info += "Slots: " + QString::number(entry->GetNumSlots());
	}

	if (entry->GetSizeCapacity())
	{
	  if (!info.isEmpty())
	    info += ", ";
	  info += "Cap: " + size_name(entry->GetSizeCapacity());
	}

	if (entry->GetWeightReduction())
	{
	  if (!info.isEmpty())
	    info += ", ";
	  info += "Rdc: " + 
	    QString::number(entry->GetWeightReduction()) + "%";
	}
      }
    }

    if (!info.isEmpty())
      printf("(%s)", (const char*)info);

    printf("\n");
    
    // increment counter;
    counter ++;
  }

  // delete the last entry since it won't get deleted at the top of the loop
  delete entry;

  // delete the iterator
  delete it;

  printf("--------------------------------------------------\n");
  printf("Found %d records\n", counter);

  return 0;
}

int displayRecord(EQItemDB* itemDB, 
		  uint16_t itemNr)
{
  QString nameString;
  QString loreString;
  bool hasEntry = false;
  EQItemDBEntry* entry = NULL;

  loreString = itemDB->GetItemLoreName(itemNr);
  nameString = itemDB->GetItemName(itemNr);
  hasEntry = itemDB->GetItemData(itemNr, &entry);

  printf ("Item ID: %d\n", itemNr);
 
  if (hasEntry)
    printf ("IconNr: %d\n", entry->GetIconNr());

  if (!nameString.isEmpty())
  {
    printf ("Name: %s\n", (const char*)nameString);
    printf ("Lore: %s\n", (const char*)loreString);
  }
  else
  {
    printf ("Lore: %s\n", (const char*)loreString);
  } 

  if (hasEntry)
  {
    printf ("Model: %s\n", (const char*)entry->GetIdFile());
    printf ("flag: 0x%4.4x\n", entry->GetFlag());
    printf ("MagicFlag: 0x%2.2x\n", entry->GetMagic());
    printf ("Weight: %.1f\n", (entry->GetWeight())/10.0);
    printf ("Flags: ");
    if (entry->IsBook())
      printf (" BOOK");
    if (entry->IsContainer())
      printf (" CONTAINER");
    if (entry->GetNoDrop() == 0)
      printf (" NO-DROP");
    if (entry->GetNoSave() == 0)
      printf (" NO-SAVE");
    if (entry->GetMagic() == 1)
      printf (" MAGIC");
    if (loreString[0] == '*')
      printf (" LORE");
    printf ("\n");
    printf ("Size: %s\n", (const char*)size_name(entry->GetSize()));
    printf ("Icon#: %d\n", entry->GetIconNr());
    printf ("Slots: %s\n", (const char*)print_slot (entry->GetSlots()));
    if (entry->GetNumSlots())
      printf ("Container Slots: %d\n", entry->GetNumSlots());
    printf ("Cost: %dcp\n", entry->GetCost());
    if (entry->GetSTR())
      printf ("Str: %d\n", entry->GetSTR());
    if (entry->GetSTA())
      printf ("Sta: %d\n", entry->GetSTA());
    if (entry->GetCHA())
      printf ("Cha: %d\n", entry->GetCHA());
    if (entry->GetDEX())
      printf ("Dex: %d\n", entry->GetDEX());
    if (entry->GetINT())
      printf ("Int: %d\n", entry->GetINT());
    if (entry->GetAGI())
      printf ("Agi: %d\n", entry->GetAGI());
    if (entry->GetWIS())
      printf ("Wis: %d\n", entry->GetWIS());
    if (entry->GetMR())
      printf ("Magic: %d\n", entry->GetMR());
    if (entry->GetFR())
      printf ("Fire: %d\n", entry->GetFR());
    if (entry->GetCR())
      printf ("Cold: %d\n", entry->GetCR());
    if (entry->GetDR())
      printf ("Disease: %d\n", entry->GetDR());
    if (entry->GetPR())
      printf ("Poison: %d\n", entry->GetPR());
    if (entry->GetHP())
      printf ("HP: %d\n", entry->GetHP());
    if (entry->GetMana())
      printf ("Mana: %d\n", entry->GetMana());
    if (entry->GetAC())
      printf ("AC: %d\n", entry->GetAC());
    if (entry->GetLight())
      printf ("Light: %d\n", entry->GetLight());
    if (entry->GetDelay())
      printf ("Delay: %d\n", entry->GetDelay());
    if (entry->GetDamage())
    {
      printf ("Damage: %d\n", entry->GetDamage());
      printf ("Skill: %s\n", (const char*)print_skill (entry->GetSkill()));
    }
    if (entry->GetRange())
      printf ("Range: %d\n", entry->GetRange());
    printf ("Material: 0x%2.2x (%s)\n", entry->GetMaterial(),
            (const char*)print_material (entry->GetMaterial()));
    printf("Color: 0x%8.8x\n",
           entry->GetColor());
    if (entry->GetSpellId0() != ITEM_SPELLID_NOSPELL)
      printf ("Effect1: %s\n", 
              (const char*)spell_name (entry->GetSpellId0()));
    if (entry->GetLevel())
      printf ("Casting Level: %d\n", entry->GetLevel());
    if (entry->GetCharges())
      printf ("Stack: %d\n", entry->GetCharges());
    if (entry->GetSpellId() != ITEM_SPELLID_NOSPELL)
      printf ("Effect2: %s\n", 
              (const char*)spell_name (entry->GetSpellId()));
    printf ("Class: %s\n", (const char*)print_classes (entry->GetClasses()));
    printf ("Race: %s\n", (const char*)print_races (entry->GetRaces()));
    if (entry->IsContainer())
    {
      if (entry->GetNumSlots())
        printf ("Container Slots: %d\n", entry->GetNumSlots());
      if (entry->GetSizeCapacity())
        printf ("Size Capacity: %s\n", (const char*)size_name(entry->GetSizeCapacity()));
      if (entry->GetWeightReduction())
        printf ("%% Weight Reduction: %d\n", entry->GetWeightReduction());
    }

    // don't need the entry anymore, delete it.
    delete entry;
  }

  unsigned char* rawData = NULL;
  int size = itemDB->GetItemRawData(itemNr, &rawData);

  if ((size > 0) && (rawData != NULL))
  {
    printf("Packet data: (%d octets)\n", size);
    if (size != sizeof(itemStruct))
      printf("Warning: (%d octets) != sizeof(itemStruct) (%d octets): "
	     "Data alignment is suspect!\n", 
	     size, sizeof(itemStruct));
    
    fprintData(stdout, size, rawData);

    delete [] rawData;
  }

  return 0;
}

