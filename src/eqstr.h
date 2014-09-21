/*
 * eqstr.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002-2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#ifndef _EQSTR_H_
#define _EQSTR_H_

#include <stdint.h>

#include <q3intdict.h>
#include <qstring.h>

//----------------------------------------------------------------------
// EQStr
class EQStr
{
 public:
  EQStr(int size = 8009);
  ~EQStr();

  bool load(const QString& eqstrFile);
  QString find(uint32_t formatid) const;
  QString message(uint32_t formatid) const;
  QString formatMessage(uint32_t formatid, 
			const char* arguments, size_t argslen) const;

 protected:
   Q3IntDict<QString> m_messageStrings;
   bool m_loaded;
};

#endif // _EQSTR_H_


