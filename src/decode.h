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

#define FLAG_COMP         0x1000 // Compressed packet
#define FLAG_COMBINED     0x2000 // Combined packet
#define FLAG_CRYPTO       0x4000 // Encrypted packet
#define FLAG_IMPLICIT     0x8000 // Packet with implicit length
#define FLAG_DECODE       ( FLAG_COMP | FLAG_COMBINED | FLAG_IMPLICIT | FLAG_CRYPTO )

#endif	// EQDECODE_H
