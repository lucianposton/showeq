/*
 *  libEQ.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <netinet/in.h>

#include <qapplication.h>
#include <qfile.h>
#include <qdatastream.h>

#include "decode.h"
#include "opcodes.h"

int64_t decode_key = 0;
bool valid_key = false;

uint8_t* decodeOpCode (uint8_t *data, uint32_t *len);

void initDecode() {
    valid_key = true;
    decode_key = 0;
}

void stopDecode() {
    valid_key = false;
}

packetList decodePacket(uint8_t *data, uint32_t len) {
    packetList results;
    
    uint16_t opCode = *((uint16_t *)data);
    
    if (opCode & FLAG_CRYPTO || opCode & FLAG_COMP) {
	data = decodeOpCode (data, &len);
	if (data == NULL)
	    return results;
	opCode = *((uint16_t *)data);
    }
    
    // this works, but could really use a cleanup - mvern
    while (len > 2) {
	if (opCode & FLAG_COMBINED) {
	    bool repeatop = false;
	    uint8_t *dptr = data+2;
	    uint32_t left = len-2;
	    int32_t count = 1;
	    
	    if (opCode & FLAG_IMPLICIT) {
		opCode = opCode & ~FLAG_IMPLICIT;
		repeatop = true;
	    }
	    //printf ("unrolling 0x%04x\n", opCode);
	    opCode = opCode & ~FLAG_COMBINED;
	    
	    while (count > 0) {
		uint16_t size;
		
		size = implicitlen(opCode);
		if (size == 0) { // Not an implicit length opcode
		    if (dptr[0] == 0xff) { // > 255 length
			left--;
			dptr++;
			size = ntohs(*((uint16_t *)dptr));
			left -= 2;
			dptr += 2;
		    } else { // single octet length
			size = dptr[0];
			left--;
			dptr++;
		    }
		}
		//printf ("size is %d\n", size);
		
		if (size > left) {
		    printf("error: size > left (size=%d, left=%d, opcode=0x%04x)\n", 
			   size, left, opCode);
		    results.clear();
		    return results;
		}
		
		uint8_t *pkt = new uint8_t[size+2];
		memcpy (pkt, &opCode, 2);
		memcpy (pkt+2, dptr, size);
		//printf ("sending: 0x%04x, %d\n", opCode, size);

		// queue packet for return
		packetbuff result;
		result.data = pkt;
		result.len = size + 2;
		result.allocated = true;
		results.push_back(result);

		dptr += size;
		left -= size;
		
		count--;
		if (repeatop) {
		  count = dptr[0];
		  dptr++;
		  left--;
		  repeatop = false;
		  //printf ("repeating %d times\n", count);
		}
	    }
	    
	    if (left > 2) {
		opCode = *((unsigned short *)dptr);
		//printf ("doing leftover: 0x%04x, %d\n", opCode, left);
		if (opCode & FLAG_COMBINED)
		{
		    data = dptr;
		    len = left;
		    continue;
	      } else {
		  packetbuff result;
		  result.data = dptr;
		  result.len = left;
		  result.allocated = false;
		  results.push_back(result);
		  return results;
	      }
	    }
	    return results;
	} else {
	    break;
	}
    }
    packetbuff result;
    result.data = data;
    result.len = len;
    result.allocated = false;
    results.push_back(result);
    
    return results;
}

uint8_t* decodeOpCode (uint8_t *data, uint32_t *len)
{
    uint16_t opCode = *(uint16_t *)data;
    bool s_encrypt = opCode & FLAG_CRYPTO;
    bool compressed = opCode & FLAG_COMP;

    if (s_encrypt)
    {
        if (!valid_key)
	    return NULL;
	    
//	printf ("decoding 0x%04x with 0x%08x\n", opCode, decode_key);
	
	int64_t offset = (decode_key % 5) + 7;
	*((int64_t *)(data+offset)) ^= decode_key;
	decode_key ^= *((int64_t *)(data+offset));
	decode_key += *len - 2;
	opCode &= ~FLAG_CRYPTO;
	memcpy (data, &opCode, 2);
    }

    if (compressed)
    {
        static uint8_t decompressed[200000];
	uint32_t dcomplen = 199998;
	uint32_t retval;
	
	retval = uncompress (decompressed+2, (uLongf*)&dcomplen, &data[2], (*len)-2);
	if (retval != 0)
	{
	    if (s_encrypt)
		printf("Lost sync, relog or zone to reset\n");
	        valid_key = false;

	    printf ("uncompress failed on 0x%04x: %s\nno further attempts will be made until zone.", opCode, zError (retval));
	    return NULL;
	}
//	printf ("clean uncompress on 0x%04x: %s\n", opCode, zError (retval));
	opCode &= ~FLAG_COMP;
	memcpy (decompressed, &opCode, 2);
	data = decompressed;
	*len = dcomplen+2;
    }

    return data;
}
