/*
 * diagnosticmessages.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002-2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#ifndef _DIAGNOSTICMESSAGES_H_
#define _DIAGNOSTICMESSAGES_H_

int seqDebug(const char* format, ...);
int seqInfo(const char* format, ...);
int seqWarn(const char* format, ...);
void seqFatal(const char* format, ...);

#endif // _DIAGNOSTICMESSAGES_H_

