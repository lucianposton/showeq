/*
 * filter.cpp
 *
 * regex filter module
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

/* Implementation of filter class */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "everquest.h"
#include "filter.h"

#define MAXLEN   5000

//#define DEBUG_FILTER

/* FilterItem Class - allows easy creation / deletion of regex types */
FilterItem::FilterItem(const QString& filterPattern, int cflags)
{
  m_valid = false;

  m_minLevel = 0;
  m_maxLevel = 0;
  
  QString workString = filterPattern;
  
  QString regexString = workString;

  // find the semi-colon that seperates the regex from the level info
  int breakPoint = workString.find(';');

  // if no semi-colon, then it's all a regex
  if (breakPoint == -1)
    regexString = workString;
  else
  {
    // regex is the left most part of the string up to breakPoint characters
    regexString = workString.left(breakPoint);

    // the level string is everything else
    QString levelString = workString.mid(breakPoint + 1);

#ifdef DEBUG_FILTER 
    printf("regexString=%s\n", (const char*)regexString);
    printf("breakPoint=%d mid()=%s\n",
	   breakPoint, (const char*)levelString);
#endif

    // see if the level string is a range
    breakPoint = levelString.find('-');

    bool ok;
    int level;
    if (breakPoint == -1)
    {
      // no, level string is just a single level
      level = levelString.toInt(&ok);

      // only use the number if it's ok
      if (ok)
	m_minLevel = level;

#ifdef DEBUG_FILTER
      printf("filter min level decode levelStr='%s' level=%d ok=%1d minLevel=%d\n",
	     (const char*)levelString, level, ok, m_minLevel);
#endif
    }
    else
    {
      // it's a range.  The left most stuff before the hyphen is the minimum
      // level
      level = levelString.left(breakPoint).toInt(&ok);

      // only use the number if it's ok
      if (ok)
	m_minLevel = level;

#ifdef DEBUG_FILTER
      printf("filter min level decode levelStr='%s' level=%d ok=%1d minLevel=%d\n",
	     (const char*)levelString.left(breakPoint), level, ok, m_minLevel);
#endif

      // the rest of the string after the hyphen is the max
      levelString = levelString.mid(breakPoint + 1);

      // if a hyphen was specified, but no max value after it, it means
      // all values above min
      if (levelString.isEmpty())
	m_maxLevel = INT_MAX;
      else
      {
	// get the max level
	level = levelString.toInt(&ok);

	// only use the number if it's ok
	if (ok)
	  m_maxLevel = level;

#ifdef DEBUG_FILTER
      printf("filter max level decode levelStr='%s' level=%d ok=%1d maxLevel=%d\n",
	     (const char*)levelString, level, ok, m_maxLevel);
#endif
      }
    }
    
    // if no max level specified, or some dope set it below min, make it 
    // equal the min
    if(m_maxLevel < m_minLevel)
      m_maxLevel = m_minLevel;
  }

  // save the string as the name
  m_name = filterPattern;

#ifdef DEBUG_FILTER
  printf("regexString=%s minLevel=%d maxLevel=%d\n", 
	 (const char*)regexString, m_minLevel, m_maxLevel);
#endif

  // compile the regular expression
  switch (regcomp(&m_regexp, (const char*)regexString, cflags))
  {
    case 0:	// no error
      m_valid = true;
      break;
    case REG_BADRPT:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Invalid use of repetition operators such as using '*' as the first character.\n");
      break;
    case REG_BADBR:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Invalid use of back reference operator\n");
      break;
    case REG_EBRACE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Un-matched bracket list operators\n");
      break;
    case REG_EBRACK:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Un-matched bracket list operators.\n");
      break;
    case REG_ERANGE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Invalid use of the range operator, eg. the ending point of the range occurs prior to the starting point.\n");
      break;
    case REG_ECTYPE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Unknown character class name.\n");
      break;
    case REG_ECOLLATE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Invalid collating element.\n");
      break;
    case REG_EPAREN:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Un-matched parenthesis group operators.\n");
      break;
    case REG_ESUBREG:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Invalid back reference to a subexpression\n");
      break;
#ifndef __FreeBSD__
    case REG_EEND:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Non specific error.\n");
      break;
#endif
    case REG_EESCAPE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Trailing backslash.\n");
      break;
    case REG_BADPAT:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Invalid use of pattern operators such as group or list.\n");
      break;
#ifndef __FreeBSD__
    case REG_ESIZE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "Compiled regular expression requires a pattern buffer larger than 64 Kb.\n");
      break;
#endif
    case REG_ESPACE:
      fprintf(stderr, "Filter Error: '%s' - ", (const char*)filterPattern);
      fprintf(stderr, "The regex routines ran out of memory\n");
      break;
  } // end of error
}

FilterItem::~FilterItem(void)
{
  regfree(&m_regexp);
}

bool FilterItem::isFiltered(const QString& filterString, int level)
{
  regmatch_t match[1];
  // check the main filter string
  if (!regexec(&m_regexp, (const char*)filterString, 1, 
	       match, 0))
  {
    // is there is a level range component to this filter
    if ((m_minLevel > 0) || (m_maxLevel > 0))
    {
      if (m_maxLevel != m_minLevel)
      {
	if ((level >= m_minLevel) && (level <= m_maxLevel))
	  return true; // filter matched
      }
      else
      {
	if (level == m_minLevel)
	  return true;
      }
    }
    else 
      return true; // filter matched
  }

  return false;
}

/* Filter Class - Sets up regex filter */
Filter::Filter(int cFlags)
{
  m_cFlags = cFlags;

  loadFilters();

} // end Filter()

/* Filter Class - Sets up regex filter */
Filter::Filter(const QString& file, int cFlags)
{
  m_file = file;
  m_cFlags = cFlags;

  loadFilters();

} // end Filter()

/* Filter Class - Sets up regex filter */
Filter::Filter(const QString& file, const char* type, int cFlags)
{
  m_file = file;
  m_cFlags = cFlags;
  m_type = type;
  m_filterItems.setAutoDelete(true);

  loadFilters();

} // end Filter()

Filter::~Filter(void)
{
  // Free the list
  m_filterItems.clear();

} // end ~Filter()

void
Filter::changeFilterFile(const QString& newfile)
{
    m_file = newfile;

    loadFilters();
}

//
// loadFilters
//
// parses the filter file and builds filter list
//
bool
Filter::loadFilters(void)
{
#ifdef DEBUG_FILTER
   printf("loadFilter()\n");
#endif /* DEBUG_FILTER */

   FILE* in;
   char msg[MAXLEN + 1];
   char* p;

   // Free any current list
   m_filterItems.clear();

   if (m_file.isEmpty())
     return false;

   // Parse filter file
   in = fopen ((const char*)m_file, "r");

   if (in == 0)
   {
     if (errno == ENOENT)
     {
       printf (">> No such file: '%s', creating new one...\n", 
	       (const char*)m_file);
       fputs ("[Spawn]\n", fopen ((const char*)m_file, "a"));
     }
     else
     {
       fprintf (stderr, "Couldn't open filter file. '%s' - %s\n", 
		(const char*)m_file, strerror(errno));
     }
   }
   else
   {
     char *sType = NULL;
     
     while (fgets (msg, MAXLEN, in) != NULL)
     {
       // treat lines beginning with # or ; as comments
       if ( (msg[0] == '#') || (msg[0] == ';'))
       {
	 msg[0] = 0;
	 continue;
       }
       p = index (msg, '\n');
       if (p)
	 *p = 0;
       p = index (msg, '\r');
       if (p)
	 *p = 0;
       if (!msg[0])
	 continue;
       
       // if this filter uses different types make sure we are in ours
       if (!m_type.isEmpty())
       {
	 if (msg[0] == '[')
	 {
	   p = index(msg, ']');
	   if (p)
	     *p = 0;
	   if (sType)
	     free(sType);
	   sType = strdup(msg + 1);
	   continue;
	 }
	 
	 if (m_type == sType)
	   addFilter(msg);
       } 
       else if (msg[0])
	 addFilter(msg);
     }

     if (sType)
       free(sType);
     fclose (in);
   }

#ifdef DEBUG_FILTER
   if (!m_type.isEmpty())
     printf("Loaded %d filters from section '%s' in file '%s'\n", 
	    m_filterItems.count(), (const char*)m_type, (const char*)m_file);
   else 
     printf("Loaded %d filters from section '%s' in file '%s'\n", 
	    m_filterItems.count(), (const char*)m_file);
#endif

//listFilters();
   return true;

} // end loadFilter()

bool
Filter::isFiltered(const QString& filterString, int level)
{
#ifdef DEBUG_FILTER
// printf("isFiltered(%s)\n", string);
#endif /* DEBUG_FILTER */

  FilterItem *re;

  // iterate over the filters checking for a match
  FilterListIterator it(m_filterItems);
  for (re = it.toFirst(); re != NULL; re = ++it)
  {
    if (re->isFiltered(filterString, level))
      return true;
  }
  
  return false;
}

bool
Filter::saveFilters(void)
{
  FILE *in;
  FILE *out;
  FilterItem* re;
  char msg[MAXLEN + 1];
  char *p;
  int count = 0;
  bool done  = false;
  bool start = false;
  
#ifdef DEBUG_FILTER
  printf("Filter::saveFilters(void)\n");
#endif
  // if no filename can't save
  if (m_file.isEmpty())
    return false;
  
  // Save filter file
  // Open source and destination files
  QString outfilename = NULL;
  QString infilename = m_file;
  
  outfilename = infilename + ".new";
  in = fopen ((const char*)infilename, "r");
  out = fopen ((const char*)outfilename, "w+");
  if (in == 0)
  {
    fprintf (stderr, "Couldn't open filter file. '%s' - %s\n", 
	     (const char*)infilename, strerror(errno));
  }
  if (out == 0)
  {
    fprintf (stderr, "Couldn't open filter file. '%s' - %s\n", 
	     (const char*)outfilename, strerror(errno));
    return false;
  }
  else
  {
    char *sType = NULL;
    
    if (in)
    {
      // Parse source file
      while (fgets (msg, MAXLEN, in) != NULL)
      {
	// terminate the line
	p = index (msg, '\n');
	if (p)
	  *p = 0;
	p = index (msg, '\r');
	if (p)
	  *p = 0;
	
	// terminate on CF or LF 
	// if this filter uses section names
	if (!m_type.isEmpty())
	{
	  // end of section - dump all items left in list that belong here
	  if (sType && !msg[0])
	  {
	    // if this is our section
	    if (m_type == sType)
	    {
	      // done copying filters that existed in file already
	      // dump whatever is left in list
	      FilterListIterator it(m_filterItems);
	      for (re = it.toFirst(); re != NULL; re = ++it)
	      {
#ifdef DEBUG_FILTER
		printf("OUT: '%s'\n", (const char*)re->name());
#endif
		fprintf(out, "%s\n", (const char*)re->name());
	      }
	      done = true;
	      start = false;
	    } // end if our section
	    free(sType);
	    sType = 0;
	  } // end if end of section
	}
	
	// treat lines beginning with # or ; as comments
	if ( (msg[0] == '#') || (msg[0] == ';'))
	{
#ifdef DEBUG_FILTER
	  printf("OUT: '%s'\n", msg);
#endif
	  fprintf(out, "%s", msg);
	  msg[0] = 0;
	}
	
	// preserve blank lines
	if (!msg[0])
	  fprintf(out, "\n");
	
	if (!m_type.isEmpty())
	{
	  // check for section name
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
#ifdef DEBUG_FILTER
	    printf("OUT: '[%s]'\n", sType);
#endif
	    fprintf(out, "[%s]\n", sType);
	    start = true;
	    msg[0] = 0;
	  }
	} // end if filter uses section names  
	
	if (msg[0])
	{
	  // if this is our section
	  if (sType && (m_type == sType))
	  { 
	    // look for a match, if found put it in the file and 
	    // remove it from the list
	    FilterListIterator it(m_filterItems);
	    for (re = it.toFirst(); re != NULL; re = ++it)
	    {
	      // if we found a match, output it
	      if (re->name() ==  msg)
	      {
#ifdef DEBUG_FILTER
		printf("OUT: '%s'\n", msg);
#endif
		fprintf(out, "%s\n", msg);
		count++;
		remFilter(msg);
		break;
	      }
	     } // for all items in list
	  } // end if our section
	  
	  // someone elses section, just output it without alteration
	  else
	  {
#ifdef DEBUG_FILTER
	    printf("OUT: '%s'\n", msg);
#endif
	    fprintf(out, "%s\n", msg);
	  }
	} // end if msg
      }  // end while lines in source file
    } // end if source file
    
    // if we still have filters in our list, we never found our section
    // add it
    if (!done)
    {
      if (!m_type.isEmpty() && !start)
      {
#ifdef DEBUG_FILTER
	printf("done parsing file... creating section '%s'\n", 
	       (const char*)m_type);
#endif
	fprintf(out, "\n[%s]\n", (const char*)m_type);
      }
       
      // done copying filters that existed in file already
      // dump whatever is left in list
      FilterListIterator it(m_filterItems);
      for (re = it.toFirst(); re != NULL; re = ++it)
      {
#ifdef DEBUG_FILTER
	printf("OUT: '%s'\n", (const char*)re->name());
#endif
	fprintf(out, "%s\n", (const char*)re->name());
      }
    }
    if (fflush (out))
      fprintf (stderr, "Couldn't flush filter file. '%s' - %s\n", 
	       (const char*)outfilename, strerror(errno));
    if (fclose (out))
      fprintf (stderr, "Couldn't flush filter file. '%s' - %s\n", 
	       (const char*)outfilename, strerror(errno));
    if (in)
      fclose (in);
    
    //  printf ("Filter file saved '%s'\n", outfilename);
    
    // rename files
    int result;
    QString bakFileName = infilename + ".bak";
    // rename input file to filename.bak
    result = rename((const char*)infilename,
		    (const char*)bakFileName);
    if (result != 0)
      fprintf(stderr, "rename '%s' to '%s' failed: (%d): %s\n",
	      (const char*)infilename, (const char*)bakFileName,
	      errno, strerror(errno));

    // rename the output file to the input filename
    result = rename((const char*)outfilename,
		    (const char*)infilename);

    if (result != 0)
      fprintf(stderr, "rename '%s' to '%s' failed: (%d): %s\n",
	      (const char*)outfilename, (const char*)infilename,
	      errno, strerror(errno));

     if (sType)
       free(sType);
  }
  
  // re-read the filters from file
  loadFilters();
  
  return true;
} // end saveFilters



//
// remFilter
//
// Remove a filter from the list
void
Filter::remFilter(const QString& filterPattern)
{
   FilterItem *re;

   // Find a match in the list and the one previous to it
   //while(re)
   FilterListIterator it(m_filterItems);
   for (re = it.toFirst(); re != NULL; re = ++it)
   {
     if (re->name() == filterPattern) // if match
     {
       // remove the filter
       m_filterItems.remove(re);

#ifdef DEBUG_FILTER
printf("Removed '%s' from List\n", (const char*)filterPattern);
#endif
       break;
     }
   } //end For
}


//
// addFilter
//
// Add a filter to the list
//
bool 
Filter::addFilter(const QString& filterPattern)
{
  FilterItem* re;

  // no duplicates allowed
  if (findFilter(filterPattern))
    return false;

  re = new FilterItem(filterPattern, m_cFlags);

  // append it to the end of the list
  m_filterItems.append(re);

#ifdef DEBUG_FILTER
printf("Added Filter '%s'\n", filterPattern);
#endif

 return re->valid(); 
} // end addFilter

//
// findFilter
//
// Find a filter in the list
//
FilterItem *
Filter::findFilter(const QString& filterPattern)
{
  FilterItem* re;

  FilterListIterator it(m_filterItems);
  for (re = it.toFirst(); re != NULL; re = ++it)
    if (re->name() ==  filterPattern)
      return re;

  return NULL;
}

void
Filter::listFilters(void)
{
  FilterItem *re;

#ifdef DEBUG_FILTER
//  printf("Filter::listFilters\n");
#endif

  FilterListIterator it(m_filterItems);
  for (re = it.toFirst(); re != NULL; re = ++it)
    printf("'%s'\n", (const char*)re->name());
}
