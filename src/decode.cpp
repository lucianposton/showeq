/*
 * Decode.cpp
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

#include "main.h"
#include "decode.h"

uint16_t implicitlen (uint16_t opcode)
{
  switch (opcode) 
  {
  case 0x0017: // 23
    return 0x08; // 8
  case 0x0022: // 34
    return 0x12; // 18
  case 0x0039: // 57
    return 0x0c; // 12
  case 0x00bc: // 188
    return 0x08; // 8
  case 0x00db: // 219
    return 0x17; // 23
  case 0x00ec: // 236
    return 0x04; // 4
  case 0x00f9: // 249
    return 0x88; // 136
  case 0x00fa: // 250
    return 0x1f; // 31
  case 0x0125: // 293
    return 0x09; // 9
  case 0x0134: // 308
    return 0x04; // 4
  case 0x013a: // 314
    return 0x08; // 8
  case 0x0217: // 535
    return 0x03; // 3
  case 0x0218: // 536
    return 0x03; // 3
  case 0x022d: // 557
    return 0x06; // 6
  case 0x022e: // 558
    return 0x06; // 6
  default:
    return 0;
  } 
}
