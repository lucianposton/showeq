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
  case 0x0022: // 34
    return 0x08; // 8
  case 0x0028: // 40
    return 0x12; // 18
  case 0x003f: // 63
    return 0x0c; // 12
  case 0x00c0: // 192
    return 0x0c; // 12
  case 0x00e4: // 228
    return 0x17; // 23
  case 0x00f5: // 245
    return 0x04; // 4
  case 0x0102: // 258
    return 0x88; // 136
  case 0x0103: // 259
    return 0x1f; // 31
  case 0x012e: // 302
    return 0x09; // 9
  case 0x0131: // 305
    return 0x08; // 8
  case 0x0142: // 322
    return 0x04; // 4
  case 0x021e: // 542
    return 0x03; // 3
  case 0x021f: // 543
    return 0x03; // 3
  case 0x0234: // 564
    return 0x06; // 6
  case 0x0235: // 565
    return 0x06; // 6
  default:
    return 0;
  } 
}
