/*
 * libeq.h
 *
 * ShowEQ header file for libEQ API
 * http://seq.sourceforge.net
 */

#ifndef _LIBEQ_H_
#define _LIBEQ_H_

#if HAVE_LIBEQ
#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

  // Function to determine the zone key given a spawn and player packet
  uint32_t FindKey(const uint8_t* player, uint32_t plen, 
		   const uint8_t* spawn, uint32_t slen);
  
  // Function to decode the packet given a pre-determined key
  int ProcessPacket(const uint8_t* data, uint32_t len,
		    uint8_t* out, uint32_t* outlen,
		    uint64_t* key, const char* cli,
		    const uint8_t* player, uint32_t plen);
  
  // Function to decode a spawn packet given a pre-determined key
  void DecodeSpawn (const uint8_t* spawn, uint32_t slen, 
		    uint8_t* out, uint32_t* outlen,
		    uint64_t key);
  
  bool InitializeLibEQ(uint16_t cookie,
		       uint16_t charProfile,
		       uint16_t zoneSpawns,
		       uint16_t newSpawn);

#ifdef  __cplusplus
}
#endif
#else
#warning "HAVE_LIBEQ=0"
#endif // HAVE_LIBEQ

#endif // _LIBEQ_H_
