/*
 * Decode.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#include "decode.h"

uint16_t implicitlen (uint16_t opcode)
{
  switch (opcode) 
  {
  case 0x0021:
    return 0x08; // 8
  case 0x0027:
    return 0x12; // 18
  case 0x003e:
    return 0x0c; // 12
  case 0x00bf:
    return 0x0c; // 12
  case 0x00e2:
    return 0x17; // 23
  case 0x00f3:
    return 0x04; // 4
  case 0x0100:
    return 0x88; // 136
  case 0x0101:
    return 0x1f; // 31
  case 0x012c:
    return 0x09; // 9
  case 0x012f:
    return 0x08; // 8
  case 0x0140:
    return 0x04; // 4
  case 0x022d:
    return 0x03; // 3
  case 0x022e:
    return 0x03; // 3
  case 0x0243:
    return 0x06; // 6
  case 0x0244:
    return 0x06; // 6
  default:
    return 0;
  } 
}
