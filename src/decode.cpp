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
  case 0x0022:
    return 0x08; // 8
  case 0x0028:
    return 0x12; // 18
  case 0x003f:
    return 0x0c; // 12
  case 0x00c0:
    return 0x0c; // 12
  case 0x00e4:
    return 0x17; // 23
  case 0x00f5:
    return 0x04; // 4
  case 0x0102:
    return 0x88; // 136
  case 0x0103:
    return 0x1f; // 31
  case 0x012e:
    return 0x09; // 9
  case 0x0131:
    return 0x08; // 8
  case 0x0142:
    return 0x04; // 4
  case 0x0231:
    return 0x03; // 3
  case 0x0232:
    return 0x03; // 3
  case 0x0247:
    return 0x06; // 6
  case 0x0248:
    return 0x06; // 6
  default:
    return 0;
  } 
}
