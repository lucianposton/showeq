/*
 * cpreferences.cpp
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
 * To use this class inherit it and use the following member fuctions:
 *    Save()             - Saves all modified cache values to file
 *    Load()             - Loads values from file into cache
 *    Revert()           - Reloads values from file into cache (looses mods)
 *    SetPrefValue(...)  - Set an integer value persistant preference
 *    GetPrefValue(...)  - Get an integer value persistant preference
 *    SetPrefText(...)   - Set a string string persistant preference
 *    GetPrefText(...)   - Get a string value persistant preference
 *
 *    The 'Set' methods take an option argument (defaults to TRUE) called 
 *    'persistent'.  If this is set to FALSE the changed value will not be 
 *    saved to the config file (unless set by another caller to persistent).
 *    This is usefull if you have something like command line arguments which
 *    should override the config file settings but not be saved.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>   // isspace()
#include <stdio.h>   // fopen()
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "cpreferences.h"
#include "main.h"

#include <qstring.h>
#include <qapplication.h>
#include <qmessagebox.h>

//#define DEBUG
//#define DEBUGSAVE
//#define DEBUGLOAD

#undef DEBUG

//////////////////////////////////////////////////////////////////////
//                        CPreferenceFile                           //
//////////////////////////////////////////////////////////////////////

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
CPreferenceFile::CPreferenceFile(const char *inFileName)
{
   fFileName = strdup(inFileName);
   m_list = NULL;
   fModified = FALSE;		
   Load();
#ifdef DEBUG
   qDebug("Loaded Preferences:");
   dumpList(m_list);
#endif
} // end Constructor


//
// destructor
//
CPreferenceFile::~CPreferenceFile()
{
   if(fFileName)
      free ((char *)fFileName); 
   fFileName = NULL;
   if (m_list)
      delete m_list;
   m_list = NULL;
} // end Destructor 


//
// isPreference()
//
// return True if an item exists in the preference cache
//
bool
CPreferenceFile::isPreference(const char* inIdentifier)
{
   CPrefItem *pref;
   for( pref = m_list->first(); pref != 0; pref = m_list->next() ) 
      if (!strcasecmp(pref->Identifier(), inIdentifier))
        return TRUE;

   return FALSE;
}


//
// GetPrefBool()
//
// return True/False if a boolean pref exists and is set
// saves you the hastle of passing in a string and evaling it later
//
bool
CPreferenceFile::GetPrefBool(const char *inIdentifier, bool inDefault)
{
  char value[MAXLEN];
  int nValue;

  GetPrefValue(inIdentifier, nValue, inDefault);

  if (nValue)
    return TRUE;

  return FALSE;
}



//
// GetPrefInt()
//
// return int value if pref exists and is set
// saves you the hastle of passing in a string and evaling it later
//
int
CPreferenceFile::GetPrefInt(const char *inIdentifier, int inDefault)
{
  char value[MAXLEN];
  int nValue;

  GetPrefValue(inIdentifier, nValue, inDefault);

  return nValue;
}
          


//
// GetPrefString()
//
// return const char *
// Note that this returns a static string
//
const char*
CPreferenceFile::GetPrefString(const char *inIdentifier, const char *inDefault)
{
  m_cBuffer[0] = 0;

  /* GetPrefText returns a boolean.  If it can find the text we want,
     it returns true.  If it can't it returns false.  If it can't find
     what we want, we should return the default instead of m_cBuffer after
     GetPrefText has mangled it.  cpphack */
  if(!GetPrefText(inIdentifier, m_cBuffer, sizeof(m_cBuffer))) {
    if (inDefault) {
      return inDefault;
    }
  }
  return m_cBuffer;
}



//
// Revert()
//
// reload cache from file
//
void
CPreferenceFile::Revert()
{
   delete m_list;
   m_list = NULL;
   Load();

} // end Revert()


//
// Load()
//
// Load file into cache
//
void
CPreferenceFile::Load()
{
   char *sType = NULL;
#ifdef DEBUG
  qDebug("CPreferenceFile::Load()");
#endif

   if (m_list == NULL)
   {
      m_list = new QList<CPrefItem>;	// List of CPreference's

      FILE * fp = fopen(fFileName, "r");
      if (fp)
      {
         char line[MAXLEN];
         char *p;

         for (;;)
         {
            line[0] = 0;
            fgets(line, sizeof(line), fp);
           
            // terminate on CF or LF 
            p = index (line, '\n');
            if (p)
              *p = 0;
            p = index (line, '\r');
            if (p)
              *p = 0;

#ifdef DEBUGLOAD
   printf("CPreferenceFile::Load(): '%s' - ", line);
#endif
            // treat lines beginning with # or ; as comments
            if ( (line[0] == '#') || (line[0] == ';'))
            {
#ifdef DEBUGLOAD
   printf("skipping comment\n");
#endif
//               line[0] = 0;
               continue;
            }

            // if section name
            if (line[0] == '[')
            {
              p = index(line, ']');
              if (p) 
                *p = 0;
              if (sType)
                free(sType);
              sType = strdup(line + 1);
#ifdef DEBUGLOAD
   printf(" - grabbed section '%s'\n", sType);
#endif
              continue;
            } // end if Section name

            // strip leading whitespace from line
            p = line;
            while(*p && isspace(*p)) p++;
           
            // Parse the line and add it to the list
            if (*line)
            {
               LoadLine(line, sType);
            }

            // otherwise blank line, end of section
            else
            {
#ifdef DEBUGLOAD
   printf(" skipping\n");
#endif
              if (sType)
                free(sType);
              sType = 0;
            }

            if (feof(fp))
               break;

         } // while lines in file

         fclose(fp);
//         fprintf(stdout, "Preferences loaded from '%s'\n", fFileName);
      } else {

        // Generate the error report
        QString qsError;                                                                                                                                                                                                                                                                                                       
        qsError.sprintf("ERROR: Cannot open config file '%s'\n\nPlease verify that you have manually installed the config file to the\ncorrect location, (usually " LOGDIR "/showeq.conf) as the\ninstaller does not copy it...\n\nIt must be changed before it will work!\nRefer to the comments in the conf/showeq.conf.dist file for more details.", fFileName);
        
        // Output the error to BOTH console AND message box.
        printf("\n" + qsError + "\n\n");
        
        // Construct pseudo app to gain access to QMessageBox...
        int    i = 0;
        char **c = NULL;

        QApplication qapp(i, c);

        // Pop-up a critical message box with the error...
        QMessageBox::critical(0, "Fatal error while loading ShowEQ!", qsError);

        // Exit the program since it will not function without the conf file.
        exit(1);

      }
      fModified = FALSE;
   } // if list

} // end Load()


//
// Save()
//
// Save values from cache to file
//
void
CPreferenceFile::Save()
{
#ifdef DEBUG
   printf("CPreferenceFile::Save() %s\n", fModified?"Modified":"Not Modified");
   dumpList(m_list);
#endif

   if (fModified)
   {
      FILE *in;
      FILE *out;
      char msg[MAXLEN];
      char *p;
      int count = 0;
      bool bQuoted = FALSE;
      bool bSaved = FALSE;
 
      // Open source and destination files
      char *outfilename = NULL;
      const char *infilename = fFileName;

      outfilename = (char *) malloc(strlen(infilename) + 64);
      sprintf(outfilename, "%s%s", fFileName, ".new");
      in = fopen (infilename, "r");
      out = fopen (outfilename, "w+");
      if (in == 0)
      {
         fprintf (stderr, "Couldn't open current config file. '%s' - %s\n",
           infilename, strerror(errno));
      }
      if (out == 0)
      {
         fprintf (stderr, "Couldn't open new config file. '%s' - %s\n",
           outfilename, strerror(errno));
      }
      else
      {
         char *sType = NULL;

         // build a copy of all the args we want to save
         CPreferenceList *pTempList = new QList<CPrefItem>;
         CPrefItem *pref;
         for( pref = m_list->first(); pref != 0; pref=m_list->next() )
         {
            if (pref->Persistent())
              pTempList->append(pref);
         }
#ifdef DEBUGSAVE
printf("CPreferenceFile::Save() Saving list:\n");
dumpList(pTempList);
#endif

         if (in)
         {
           // Parse source file
           while (fgets (msg, sizeof(msg), in) != NULL)
           {
              // terminate on CF or LF 
              p = index (msg, '\n');
              if (p)
                *p = 0;
              p = index (msg, '\r');
              if (p)
                *p = 0;
                                       
              // end of section - dump all items left in list that belong here
              if (sType && !msg[0])
              {
                bool bRemoved;
                do
                {
                  bRemoved = FALSE;
                  // done copying filters that existed in file already
                  // dump whatever is left in list
                  for( pref = pTempList->first(); pref != 0; pref = pTempList->next() )
                  {
                     char *tempStr = strdup(pref->Identifier());
//                     p = index(tempStr, '_');
                     p = rindex(tempStr, '_');
                     if (p)
                       *p++ = 0;
                     if (!strcasecmp(sType, tempStr))
                     {
                        struct timeval tv;
                        if (!gettimeofday(&tv, NULL))
                           fprintf(out, "%s\t%s\t# Added %s", p, pref->Text(), 
                               ctime(&tv.tv_sec));
                        else
                           fprintf(out, "%s\t%s\t# Added\n", p,pref->Text());
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '%s\t%s' - added\n", 
         pref->Identifier(), pref->Text());
#endif
                        count++;
                        pTempList->remove(pref);
                        bRemoved = TRUE;
                        break; // repeat search for more to remove
                     } // if this belongs in this section
                  } // end for all prefs in unsaved list
                } while (bRemoved);

                free(sType);
                sType = 0;

              } // end if end of section

              // Check for section name
              if (msg[0] == '[')
              {
                p = index(msg, ']');
                if (p)
                  *p = 0;
                p = index(msg, '\r');
                if (p)
                  *p = 0;
                if (sType)
                  free(sType);
                sType = strdup(msg + 1);
                fprintf(out, "[%s]", sType);  // cr will get tacked on later
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '[%s]' - sectionname\n", sType);
#endif
                msg[0] = 0;  // skip this line
              } // end if section name

              // treat lines beginning with # or ; as comments
              if ( (msg[0] == '#') || (msg[0] == ';'))
              {
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '%s' - comment\n", msg);
#endif
                 fprintf(out, "%s", msg);
                 msg[0] = 0;
              } // end if comment

              if (msg[0])
              {
                 char *tempstr = strdup(msg);
                 char *valueend = 0;
                 char *name = 0;
                 char *value = 0;
                 char *ioLine = tempstr;

                 // strip leading whitespace from line
                 while(*ioLine && isspace(*ioLine)) ioLine++;
                 name = ioLine;

                 // advance to whitespace
                 while(*ioLine && !isspace(*ioLine)) ioLine++;
                 if(*ioLine)
                   *ioLine++ = 0;

                 // strip whitespace in front of value 
                 while(*ioLine && isspace(*ioLine)) ioLine++;
                 value = ioLine;

                 // if value is quoted, parse to end of it 
                 bQuoted = FALSE;
                 if (value[0] == '"')
                 {
                    bQuoted = TRUE;
                    value++;
                    char *endquote = index(value, '\"');
                    if (endquote) 
                    {
                      *endquote = 0;
                      ioLine = endquote;
                    }
                 }

                 // advance to whitespace and terminate
                 while(*ioLine && !isspace(*ioLine)) ioLine++;
                 if(*ioLine)
                   *ioLine = 0;

                // look for a match, if found put it in the file 
                //  and remove it from the list
                char *tempstr1;
                if (sType)
                {
                   tempstr1 = (char *) malloc(strlen(sType)+strlen(name)+2);
                   sprintf(tempstr1, "%s_%s", sType, name);
                }
                else
                   tempstr1 = strdup(name);
                bSaved = FALSE;
                for(pref = pTempList->first();pref != 0;pref=pTempList->next())
                {
                  // if this is the same identifier as the one in the file 
                  if (!strcmp(pref->Identifier(), tempstr1))
                  {
                     char *ioLine1 = msg;
                     char *restofline = 0;   // everything after value
                     char *whitespace = 0;   // start of whitespace after name

                     // advance to text (name)
                     while(*ioLine1 && isspace(*ioLine1)) ioLine1++; 
                     // advance to whitespace
                     while(*ioLine1 && !isspace(*ioLine1)) ioLine1++; 
                     whitespace = ioLine1;

                     // advance to text (value)
                     while(*ioLine1 && isspace(*ioLine1)) ioLine1++; 
                     bQuoted = (ioLine1[0] == '"');
                     *ioLine1++ = 0;  // terminate it

                     if (bQuoted)
                     {
                        char *endquote = index(ioLine1, '\"');
                        if (endquote) 
                        {
                          ioLine1 = endquote;
                          ioLine1++;
                        }
                     }

                     // advance to whitespace
                     while(*ioLine1 && !isspace(*ioLine1)) ioLine1++; 
                     restofline = ioLine1;
                     if (!bQuoted)
                     {
                       fprintf(out, "%s%s%s%s\n", name, 
                                 whitespace, pref->Text(), restofline);
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '%s%s%s%s\n' - updated\n", 
         name, whitespace, pref->Text(), restofline);  
#endif
                     }
                     else
                     {
                       fprintf(out, "%s%s\"%s\"%s\n", name, 
                                 whitespace, pref->Text(), restofline);
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '%s%s%s%s\n' - updated\n", 
         name, whitespace, pref->Text(), restofline);  
#endif
                     }
                     bSaved = TRUE;
                     count++;
                     pTempList->remove(pref);
                     break;  // match found, done looking
                  } // end if match

                } // end for all entries in temp list

                // if we didn't find a match, save whatever the line was
                if (!bSaved)
                {
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '%s\n' - saved non update\n", 
         msg);  
#endif
                  fprintf(out, "%s\n", msg);
                }
                free(tempstr);
                free(tempstr1);

              } // end if pref 
              else
              {
                fprintf(out, "\n");
              }
            }  // end while lines in source file          
#ifdef DEBUGSAVE
  printf("\nCPreferenceFile::Save() - done parsing file-saving leftovers\n\n");
  dumpList(pTempList);
#endif
          // done copying filters that existed in file already
          // dump whatever is left in list
          if (pTempList->count() > 0)
          {
             struct timeval tv;
             if (!gettimeofday(&tv, NULL))
                fprintf(out, "\n# Added Items %s", ctime(&tv.tv_sec));
             else
                fprintf(out, "\n# Added Items\n");
             for( pref = pTempList->first(); pref != 0; pref = pTempList->next() )
             {
                fprintf(out, "%s\t%s\n", pref->Identifier(), pref->Text());
#ifdef DEBUGSAVE
  fprintf(stdout, "CPreferenceFile::Save(): '%s\t%s\n", 
         pref->Identifier(), pref->Text());
#endif
             } // end for all items in remaining list
          } // if items to be added
          if (fflush (out))
            fprintf (stderr, "Couldn't flush file. '%s' - %s\n",
              outfilename, strerror(errno));
          if (fclose (out))
            fprintf (stderr, "Couldn't flush file. '%s' - %s\n",
              outfilename, strerror(errno));                  if (in)
            fclose (in);

//          printf ("file saved '%s'\n", outfilename);

#if 1
          // rename files
          char *tempstr = (char*) malloc( (strlen(infilename) * 2) 
                              + strlen(outfilename) + 25); // should be enough
          sprintf(tempstr, "cp %s %s.bak", infilename, infilename);
#ifdef DEBUG
  printf("%s\n", tempstr);
#endif
          if (-1 == system(tempstr))
          {
            fprintf(stderr, "'%s' - failed\n", tempstr);
          }
          sprintf(tempstr, "mv -f %s %s", outfilename, infilename);
#ifdef DEBUG
   printf("%s\n", tempstr);
#endif
          if (-1 == system(tempstr))
          {
            fprintf(stderr, "'%s' - failed\n", tempstr);
          }
#endif
          free(tempstr);
          if (sType)
            free(sType);
        }
        if (outfilename)
          free(outfilename);
      } // end else file handles good

      fModified = FALSE;

   } // end if modified

} // end Save()


//
// LoadLine
//
// Process a line into a Identifier/Value pair
//
void 
CPreferenceFile::LoadLine(char *ioLine, char *sectionname)
{
   CPrefItem * newObj;
   char *name = 0;
   char *value = 0;

   // strip leading whitespace from line
   while(*ioLine && isspace(*ioLine)) ioLine++;
   name = ioLine;

   // advance to whitespace
   while(*ioLine && !isspace(*ioLine)) ioLine++;
   if(*ioLine)
     *ioLine++ = 0;

   // strip whitespace between name and value 
   while(*ioLine && isspace(*ioLine)) ioLine++;
   value = ioLine;

   // if value is quoted, parse to end of it 
   if (value[0] == '"')
   {
      value++;
      char *endquote = index(value, '\"');
      if (endquote) 
        *endquote = 0;
      ioLine = endquote;
   }

   else
   {
     // advance to the end of the value and null it
     while(*ioLine && !isspace(*ioLine)) ioLine++;
     if(*ioLine)
       *ioLine++ = 0;
   }

   // Qualify data
   if (!*name || !*value)
      return;

   // if we were passed a section name, format the string to 'sectionname_name'
   if (sectionname)
   {
     char *tempstr = (char *) malloc(strlen(sectionname) + strlen(name) + 2);
     sprintf(tempstr,"%s_%s", sectionname, name);
     newObj = new CPrefItem(tempstr, value);
     free(tempstr);
   }
   else
     newObj = new CPrefItem(name, value);

   m_list->append(newObj);

#ifdef DEBUGLOAD
   qDebug("LoadLine() '%s' - '%s' now appended to list %d items", 
      newObj->Identifier(), newObj->Text(), m_list->count());
#endif

} // end LoadLine


//
// SetPreferenceItem
//
// Store an Identifier/Value pair into the cache
//
void
CPreferenceFile::SetPreferenceItem(const char *inIdentifier, 
                                   const char *inString, bool bPersistent)
{
   CPrefItem *pref;
   for( pref = m_list->first(); pref != 0; pref = m_list->next() ) 
   {
      // if this is the identifier were looking for
      if (!strcasecmp(pref->Identifier(), inIdentifier))
      {
        // set its persistancy 
        pref->Persistent(bPersistent);

        // and its value has changed
        if (strcasecmp(pref->Text(), inString))
        {
#ifdef DEBUG
   qDebug("SetPreferenceItem() Modified '%s': prev '%s' new '%s'", 
           inIdentifier, pref->Text(), inString);
#endif
           pref->Text(inString);
           fModified = TRUE;

           break;  // break out of listsearch
        } // end if modified

        // not modified but we don't need to add it
        else
          break; 
      } // end if match

   } // end while prefs in list

   // If we never found it, we need to add it as a new item
   if (!pref)
   {
      pref = new CPrefItem(inIdentifier, inString, bPersistent);
      fModified = TRUE;
      m_list->append(pref);
#ifdef DEBUG
   printf("SetPreferenceItem() New Item '%s' - '%s' - count %d\n", 
      inIdentifier, inString, m_list->count());
#endif
   }

} // end SetPreferenceItem


//
// SetPrefText()
//
// Send a text preference to the preference file cache
//
void 
CPreferenceFile::SetPrefText(const char *inTitle, const char *inText, 
                             bool bPersistent)
{
   const char * theText = inText ? inText : "";

   SetPreferenceItem(inTitle, theText, bPersistent);
}

//
// SetPrefValue()
//
// Send an int preference to the preference file cache
//
void 
CPreferenceFile::SetPrefValue(const char *inTitle, int inValue, 
                              bool bPersistent)
{
   char msg[32];
   sprintf(msg, "%i", inValue);
   SetPrefText(inTitle, msg, bPersistent);			
}


//
// GetPrefText()
//
// Get a text preference from the preference file cache
//
bool 
CPreferenceFile::GetPrefText(const char *inTitle, char *outText, int inMaxLen, 
      const char *inDefault)
{
   const char *listItem = 0;
   CPrefItem *pref;
   for( pref = m_list->first(); pref != 0; pref = m_list->next() ) 
   {
      if (!strcasecmp(pref->Identifier(), inTitle))
      {
        listItem = pref->Text();
        break;
      }
   }

   if (listItem)
   {
      strncpy(outText, listItem, inMaxLen);
      return TRUE;
   }

   if (inDefault != NULL)
      strncpy(outText, inDefault, inMaxLen);
   return FALSE;

} // end GetPrefText()


//
// GetPrefValue()
//
// Get an int preference from the preference file cache
//
bool 
CPreferenceFile::GetPrefValue(const char *inTitle, int & outValue, 
                              int inDefault)
{
   char aNum[32];
   bool found = GetPrefText(inTitle, aNum, sizeof(aNum));
   if (found)
      outValue = atol(aNum);
   else
      outValue = inDefault;
   return found;
}

//
// dumplist()
//
// display the current cache (debugging)
//
void
CPreferenceFile::dumpList(CPreferenceList *list)
{
   const char *listItem = 0;
   CPrefItem *pref;
   int i = 1;

   printf("dumpList() %d items in list\n", list->count());

   for( pref = list->first(); pref != 0; pref = list->next() ) 
   {
      printf("%02d: Name '%s' Value '%s'\n",i,pref->Identifier(), pref->Text());
      i++;
   }
}



//
// CPrefItem
//
// An individual preference item in raw form
//
CPrefItem::CPrefItem(const char * inIdentifier, const char * inText, 
                     bool bPersistent)
{
   // remove leading whitespace from items
   char *inText_begin = (char *) inText;
   while(*inText_begin && isspace(*inText_begin)) inText_begin++;
   char *inIdentifier_begin = (char *) inIdentifier;
   while(*inIdentifier_begin && isspace(*inIdentifier_begin)) 
       inIdentifier_begin++;

   fIdentifier = strdup(inIdentifier_begin);
   fText = strdup(inText_begin);
   fLoaded = FALSE;
   fPersistent = bPersistent;

   // trim trailing whitespace from items
   char *end = fText + strlen(fText); 
   if (strlen(fText)) end--;
   while ( (end != fText) && isspace(*end) ) end--;
   if (!isspace(*end) && *end) end++;
   *end = 0;

   end = fIdentifier + strlen(fIdentifier); 
   if (strlen(fIdentifier)) end--;
   while ( (end != fIdentifier) && isspace(*end) ) end--;
   if (!isspace(*end)) end++;
   *end = 0;
}


CPrefItem::~CPrefItem()
{
   if (fIdentifier)
      free (fIdentifier); 
   fIdentifier = NULL;
   if (fText)
      free(fText);
   fText = NULL;
}

