/*
 * Decode.cpp
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#include "main.h"
#include "decode.h"

uint16_t implicitlen (uint16_t opcode)
{
	switch (opcode) {
            case 0x0019: // 25
                    return 0x06; // 6 bytes
            case 0x001a: // 26
                    return 0x06; // 6 bytes
            case 0x0066: // 102
                    return 0x08; // 8 bytes
            case 0x00b1: // 177
                    return 0x17; // 23 bytes
            case 0x00ca: // 202
                    return 0x04; // 4 bytes
            case 0x00e0: // 224
                    return 0x88; // 136 bytes
            case 0x00e1: // 225
                    return 0x1f; // 31 bytes
            case 0x0115: // 277
                    return 0x09; // 9 bytes
            case 0x0124: // 292
                    return 0x04; // 4 bytes
            case 0x012b: // 299
                    return 0x08; // 8 bytes
            case 0x0136: // 310
                    return 0x12; // 18 bytes
            case 0x014a: // 330
                    return 0x08; // 8 bytes
            case 0x0242: // 578
                    return 0x0c; // 12 bytes
            case 0x0263: // 611
                    return 0x03; // 3 bytes
            case 0x0264: // 612
                    return 0x03; // 3 bytes
            default:
                    return 0;
    }
}

