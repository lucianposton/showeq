/*
 * logger.cpp
 *
 * packet/data logging class
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2003-2007 by the respective ShowEQ Developers
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <qstring.h>
#include <qlist.h>

#include "logger.h"

SEQLogger::SEQLogger(FILE *fp, QObject* parent, const char* name)
  : QObject(parent, name)
{
    m_fp = fp;
    m_errOpen = false;
}

SEQLogger::SEQLogger(const QString& fname, QObject* parent, const char* name)
  : QObject(parent, name)
{
    m_fp = NULL;
    m_filename = fname;
    m_errOpen = false;
}

bool SEQLogger::open()
{
  if (m_fp)
    return true;
  
  m_fp = fopen((const char*)m_filename,"a");
  
  if (!m_fp)
  { 
    if (!m_errOpen)
    {
      ::fprintf(stderr, "Error opening %s: %s (will keep trying)\n",
		(const char*)m_filename, strerror(errno));
      m_errOpen = true;
    }
    
    return false;
  }
 
  m_errOpen = false;

  if (!m_file.open(QIODevice::Append | QIODevice::WriteOnly, m_fp))
    return false;
  
  m_out.setDevice(&m_file);
  
  return true;
}

void SEQLogger::flush()
{ 
  m_file.flush();
}


int SEQLogger::outputf(const char *fmt, ...)
{
  va_list args;
  int count;
  
  if (!m_fp)
    return 0;
  
  va_start(args, fmt);
  count = vfprintf(m_fp, fmt, args);
  va_end(args);
  
  return count;
}

int SEQLogger::output(const void* data, int length)
{
  int i;
  int count = 0;
  unsigned char *ptr = (unsigned char *) data;
  
  for(i = 0; i < length; i++,ptr++)
    count += printf("%.2X", *ptr);
  
  return count;
}

// prints up the passed in data to the file pointer
void SEQLogger::outputData(uint32_t len,
			  const uint8_t* data)
{
  if (!m_fp)
    return;

  char hex[128];
  char asc[128];
  char tmp[32];
  
  hex[0] = 0;
  asc[0] = 0;
  unsigned int c;
  
  for (c = 0; c < len; c ++)
  {
    if ((!(c % 16)) && c)
    {
      fprintf (m_fp, "%03d | %s | %s \n", c - 16, hex, asc);
      hex[0] = 0;
      asc[0] = 0;
    }
    
    sprintf (tmp, "%02x ", data[c]);
    strcat (hex, tmp);
    
    if ((data[c] >= 32) && (data[c] <= 126))
      sprintf (tmp, "%c", data[c]);
    
    else
      strcpy (tmp, ".");
    
    strcat (asc, tmp);
    
  }
  
  if (c % 16)
    c = c - (c % 16);
  
  else
    c -= 16;
  
  fprintf (m_fp, "%03d | %-48s | %s \n\n", c, hex, asc);
}

#ifndef QMAKEBUILD
#include "logger.moc"
#endif

