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
  case 0x0019:
    return 0x08; // 8
  case 0x0024:
    return 0x12; // 18
  case 0x003b:
    return 0x0c; // 12
  case 0x00bc:
    return 0x08; // 8
  case 0x00db:
    return 0x17; // 23
  case 0x00ec:
    return 0x04; // 4
  case 0x00f9:
    return 0x88; // 136
  case 0x00fa:
    return 0x1f; // 31
  case 0x0125:
    return 0x09; // 9
  case 0x0134:
    return 0x04; // 4
  case 0x013a:
    return 0x08; // 8
  case 0x0217:
    return 0x03; // 3
  case 0x0218:
    return 0x03; // 3
  case 0x022d:
    return 0x06; // 6
  case 0x022e:
    return 0x06; // 6
  default:
    return 0;
  } 
}
