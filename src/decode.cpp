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
	switch (opcode) {
		case 0x0017: // 23
			return 0x08; // 8
		case 0x0022: // 34
			return 0x12; // 18
		case 0x0039: // 57
			return 0x0c; // 12
		case 0x00c7: // 199
			return 0x08; // 8
		case 0x00e7: // 231
			return 0x17; // 23
		case 0x00f8: // 248
			return 0x04; // 4
		case 0x0110: // 272
			return 0x88; // 136
		case 0x0111: // 273
			return 0x1f; // 31
		case 0x0145: // 325
			return 0x09; // 9
		case 0x0154: // 340
			return 0x04; // 4
		case 0x015a: // 346
			return 0x08; // 8
		case 0x0243: // 579
			return 0x03; // 3
		case 0x0244: // 580
			return 0x03; // 3
		case 0x025d: // 605
			return 0x06; // 6
		case 0x025e: // 606
			return 0x06; // 6
		default:
			 return 0;
	}
}
