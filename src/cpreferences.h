/*
 * cpreference.h
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

/*
 * Orig Author - Maerlyn (MaerlynTheWiz@yahoo.com)
 * Date   - 3/19/00
 */

/*
 * CPreferenceFile
 *
 * CPreferenceFile is a generic class that implements a persistant config
 * file in the format of:
 *
 * [Section1name]                 <-- start of similarly grouped options
 * Identifier      value          <-- whitespace between Ident / value
 *                                <-- empty line ends section
 * [Section2name]                 <-- new section
 * Identifier      value          <-- new ident/value pair
 *
 * Note:  if 'value' needs to contain spaces, enclose it in "quotes".
 *        Upon saving to file, values are updated to the new settings as
 *        they appear in the file.  If an item does not exist in the file 
 *        it will be created with a comment as to the date and time created 
 *        Items can also appear in list as 'SectionName_Identifier'. 
 *
 *  The following public members are available for use
 *
 *    Save()             - Saves all modified cache values to file
 *    Load()             - Loads values from file into cache
 *    Revert()           - Reloads values from file into cache (looses mods)
 *    GetPrefBool(...)   - Get a boolean value from cache (accepts default)
 *    GetPrefInt(...)    - Get an int value from cache (accepts default)
 *    GetPrefString(...) - Get a string from cache (accepts default, staticstr)
 *    isPreference(...)  - Returns true if a preference exists in cache
 *    SetPrefValue(...)  - Sets a numerical value in cache
 *    SetPrefText(...)   - Set a string value in cache
 *
 *  Note:  Both 'Set' members allow a 'persistent' flag to be passed in
 *         which is defaulted to TRUE.  Setting this to FALSE makes that 
 *         preference not get saved upon a save.  Any later Set with a 
 *         TRUE value overrides this.  This is usefull for command line args 
 *         which should override config file prefs but not overwrite them  
 */
 
#ifndef _CPREFERENCE_H_
#define _CPREFERENCE_H_

#include <qlist.h>       // only non portable dependency - can replace with any
                         // class that provides a list of pointers
#include <stdlib.h>
#include <string.h>

class CPrefItem;         // preference item, defined later
typedef QList<CPrefItem> CPreferenceList;

#define MAXLEN 2048    // max length of value or name

//
// CPreferenceFile
//
// A File comprised of CPreference Items
//
// This is a textfile implementation in the format:
//
// [Section1Identifier]
// PreferenceItem       PreferenceValue
//
// [Section2Identifier]
// PreferenceItem       PreferenceValue
//      
class CPreferenceFile
{
public:
   CPreferenceFile(const char *inFileName);
   ~CPreferenceFile();

   void Save();
   void Load();
   void Revert();

   bool        GetPrefBool(const char *name, bool def = FALSE);
   int         GetPrefInt(const char *name, int def = -1);
   const char* GetPrefString(const char *name, const char *def = NULL); 
   bool        isPreference(const char *name);
   void        SetPrefValue(const char *inTitle, int inValue, bool per = TRUE);
   void        SetPrefText(const char *inTitle, const char *inText, 
                                                 bool per = TRUE);

private:
   bool GetPrefText(const char *inTitle, 
                                char * outText, int inMaxLength, 
                                const char *outDefault = "");
   bool GetPrefValue(const char *inTitle, 
                                int & outValue, int outDefault = -1);
   void LoadLine(char * ioLine, char *sectionname);
   void SetPreferenceItem(const char *inIdentifier, const char *inString, 
                          bool bPersistent);
   void dumpList(CPreferenceList*);
   CPreferenceList* m_list;	// List of CPreferences
   const char * fFileName;
   bool fModified;
   char m_cBuffer[MAXLEN];
};


//
// CPrefItem
//
// An individual preference item 
//  
class CPrefItem
{
public:
   CPrefItem(const char * inIdentifier, const char * inText, bool 
             persistent = TRUE);
   ~CPrefItem();

   char *Text()            { return fText; };
   char *Identifier()      { return fIdentifier; };
   bool Loaded()           { return fLoaded; };
   bool Modified()         { return fModified; };
   bool Persistent()       { return fPersistent; };

   void Text(const char * inText)
                   { if (fText) free(fText); fText = strdup(inText); }
   void Identifier(const char *inText)
                   { if (fIdentifier) free(fIdentifier); 
                     fIdentifier = strdup(inText); }
   void Loaded(bool inBool)   { fLoaded = inBool; }
   void Modified(bool inBool) { fModified = inBool; }
   void Persistent(bool inBool) { fPersistent = inBool; }

private:
   char * fIdentifier;
   char * fText;
   bool fLoaded;
   bool fModified;
   bool fPersistent;
};

#endif
