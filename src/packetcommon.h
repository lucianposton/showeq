/*
 * packetcommon.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _PACKETCOMMON_H
#define _PACKETCOMMON_H

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if !defined(__BYTE_ORDER)
#if defined(BYTE_ORDER)
#define __BYTE_ORDER	BYTE_ORDER
#define __LITTLE_ENDIAN	LITTLE_ENDIAN
#define __BIG_ENDIAN	BIG_ENDIAN
#else
#error "BYTE_ORDER should be LITTLE_ENDIAN or BIG_ENDIAN (the latter is untested)"
#endif
#endif
#else
#include <stdint.h>
#endif

#ifdef __linux__
#include <endian.h>
#endif


//----------------------------------------------------------------------
// Constants
const char* const AUTOMATIC_CLIENT_IP = "127.0.0.0";

//----------------------------------------------------------------------
// Enumerated types
enum EQStreamID 
{
  unknown_stream = -1,
  client2world = 0, 
  world2client = 1,
  client2zone = 2, 
  zone2client = 3,
  MAXSTREAMS = 4,
};

//----------------------------------------------------------------------
// Useful inline functions
#if (defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN))
inline uint16_t eqntohuint16(const uint8_t* data)
{
  return (uint16_t)((data[0] << 8) | data[1]);
}

inline int16_t eqntohint16(const uint8_t* data)
{
  return (int16_t)((data[0] << 8) | data[1]);
}

inline  uint32_t eqntohuint32(const uint8_t* data)
{
  return (uint32_t)((data[0] << 24) | (data[1] << 16) |
		    (data[2] << 8) | data[3]);
}

inline int32_t eqntohint32(const uint8_t* data)
{
  return (int32_t)((data[0] << 24) | (data[1] << 16) |
		   (data[2] << 8) | data[3]);
}

inline uint16_t eqtohuint16(const uint8_t* data)
{
  return *(uint16_t*)data;
}

inline int16_t eqtohint16(const uint8_t* data)
{
  return *(int16_t*)data;
}

inline uint32_t eqtohuint32(const uint8_t* data)
{
  return *(uint32_t*)data;
}

inline int32_t eqtohint32(const uint8_t* data)
{
  return *(int32_t*)data;
}
#else
#warning "BigEndian hasn't been tested."
inline uint16_t eqntohuint16(const uint8_t* data)
{
  return *(uint16_t*)data;
}

inline int16_t eqntohint16(const uint8_t* data)
{
  return *(int16_t*)data;
}

inline uint32_t eqntohuint32(const uint8_t* data)
{
  return *(uint32_t*)data;
}

inline int32_t eqntohint32(const uint8_t* data)
{
  return *(int32_t*)data;
}

inline uint16_t eqtohuint16(const uint8_t* data)
{
  return (uint16_t)((data[0] << 8) | data[1]);
}

inline int16_t eqtohint16(const uint8_t* data)
{
  return (int16_t)((data[0] << 8) | data[1]);
}

inline  uint32_t eqtohuint32(const uint8_t* data)
{
  return (uint32_t)((data[0] << 24) | (data[1] << 16) |
		    (data[2] << 8) | data[3]);
}

inline int32_t eqtohint32(const uint8_t* data)
{
  return (int32_t)((data[0] << 24) | (data[1] << 16) |
		   (data[2] << 8) | data[3]);
}
#endif

#endif // _PACKETCOMMON_H

