/*
 * decode.h
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#ifndef EQDECODE_H
#define EQDECODE_H

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif
#include <deque>
#include <vector>
//#include <qobject.h>
//#include <qevent.h>

#include "everquest.h"

//#define FLAG_COMP         0x2000
//#define FLAG_COMBINED     0x4000
//#define FLAG_IMPLICIT     0x8000
//#define FLAG_CRYPTO       0x1000
//#define FLAG_COMP         0x0800
//#define FLAG_COMBINED     0x8000
//#define FLAG_IMPLICIT     0x4000
//#define FLAG_CRYPTO       0x2000
#define FLAG_COMP         0x1000 // Compressed packet
#define FLAG_COMBINED     0x2000 // Combined packet
#define FLAG_IMPLICIT     0x8000 // Packet with implicit length
#define FLAG_CRYPTO       0x4000 // Encrypted packet
#define FLAG_DECODE       ( FLAG_COMP | FLAG_COMBINED | FLAG_IMPLICIT | FLAG_CRYPTO )

struct packetbuff {
    uint8_t   *data;
    uint32_t  len;
    bool      allocated;
};

typedef std::vector<packetbuff> packetList;

void initDecode();
void stopDecode();
packetList decodePacket(uint8_t *data, uint32_t len, uint8_t dir);
uint16_t implicitlen (uint16_t opcode);

#endif	// EQDECODE_H
