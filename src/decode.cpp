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
  case 0x0017:
    return 0x08; // 8
  case 0x0022:
    return 0x12; // 18
  case 0x0039:
    return 0x0c; // 12
  case 0x00ba:
    return 0x08; // 8
  case 0x00d9:
    return 0x17; // 23
  case 0x00ea:
    return 0x04; // 4
  case 0x00f7:
    return 0x88; // 136
  case 0x00f8:
    return 0x1f; // 31
  case 0x0123:
    return 0x09; // 9
  case 0x0132:
    return 0x04; // 4
  case 0x0138:
    return 0x08; // 8
  case 0x0215:
    return 0x03; // 3
  case 0x0216:
    return 0x03; // 3
  case 0x022b:
    return 0x06; // 6
  case 0x022c:
    return 0x06; // 6
  default:
    return 0;
  } 
}
