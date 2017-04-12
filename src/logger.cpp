/*
 * logger.cpp
 *
 * packet/data logging class
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <qstring.h>
#include <qlist.h>

#include "logger.h"

#include "util.h"

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

  if (!m_file.open(IO_Append | IO_WriteOnly, m_fp))
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

// prints up the passed in data to the file pointer
void SEQLogger::outputData(uint32_t len,
        const uint8_t* data)
{
    fprintData(m_fp, len, data);
}

#include "logger.moc"
