/*
 * diagnosticmessages.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#include "diagnosticmessages.h"
#include "message.h"
#include "messages.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <qstring.h>

//----------------------------------------------------------------------
// constants
static const int SEQ_BUFFER_LENGTH = 8196;       // internal buffer length

//----------------------------------------------------------------------
// internal utility function
static int seqMessage(MessageType type, const char* format, va_list ap)
{
  char buff[SEQ_BUFFER_LENGTH];
  int ret = vsnprintf(buff, sizeof(buff), format, ap);
  Messages* messages = Messages::messages();

  // if the message object exists, use it, otherwise dump to stderr
  if (messages)
    messages->addMessage(type, buff);
  else 
    fprintf(stderr, "%s\n", buff);

  return ret;
}

//----------------------------------------------------------------------
// implementations
int seqDebug(const char* format, ...)
{
  va_list ap;
  int ret;
  va_start(ap, format);
  ret = seqMessage(MT_Debug, format, ap);
  va_end(ap);
  return ret;
}

int seqInfo(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int ret = seqMessage(MT_Info, format, ap);
  va_end(ap);
  return ret;
}

int seqWarn(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int ret = seqMessage(MT_Warning, format, ap);
  va_end(ap);
  return ret;
}

void seqFatal(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  seqMessage(MT_Warning, format, ap);
  va_end(ap);
  exit (-1);
}


