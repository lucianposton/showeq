/*
 * dbcommon.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

//
// NOTE: Trying to keep this file ShowEQ/Everquest independent to allow it
// to be reused for other Show{} style projects.  Any existing ShowEQ/EQ
// dependencies will be migrated out.
//

#ifndef DBCOMMON_H
#define DBCOMMON_H

#include <stdint.h>

class Datum
{
 public:
  // default constructor
  Datum()
  {
    data = NULL;
    size = 0;
  }

  // convenience constructor
  Datum(void* data_, uint32_t size_)
  {
    data = data_;
    size = size_;
  }

  // public data members
  void* data;
  uint32_t size;
};

#endif // DBCOMMON_H
