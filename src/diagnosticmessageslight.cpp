/*
 * diagnosticmessageslight.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#include "diagnosticmessages.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int seqDebug(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int ret = vfprintf(stderr, format, ap);
  fputs("\n", stderr);
  va_end(ap);
  return ret;
}

int seqInfo(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int ret = vfprintf(stderr, format, ap);
  fputs("\n", stderr);
  va_end(ap);
  return ret;
}

int seqWarn(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int ret = vfprintf(stderr, format, ap);
  fputs("\n", stderr);
  va_end(ap);
  return ret;
}

void seqFatal(const char* format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = vfprintf(stderr, format, ap);
  fputs("\n", stderr);
  va_end(ap);
  exit (-1);
}


