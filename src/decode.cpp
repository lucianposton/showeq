/*
 * Decode.cpp
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <netinet/in.h>

#include <qapplication.h>
#include <qfile.h>
#include <qdatastream.h>

#include "main.h"
#include "decode.h"
#include "opcodes.h"
#include "libeq.h"

//#define DIAG_DECODE_ZLIB_ERROR
//#define PKTBUF_LEN	65535
#define UNKKEY     0xffffffffffffffff

EQDecode::EQDecode (QObject *parent, const char *name)
: QObject (parent, name)
{
  // Link the packet class resetDecoder signal to our ResetDecoder slot
  connect (parent, SIGNAL(resetDecoder()), this, SLOT(ResetDecoder()));

  // Link our BackfillPlayer signal to the packet class backfillPlayer slot
  connect (this, SIGNAL(dispatchDecodedCharProfile(const uint8_t*, uint32_t)),
           parent, SLOT(dispatchDecodedCharProfile(const uint8_t*, uint32_t)));

  // Link our BackfillSpawn signal to the packet class backfillSpawn slot
  connect (this, SIGNAL(dispatchDecodedNewSpawn(const uint8_t*, uint32_t)),
           parent, SLOT(dispatchDecodedNewSpawn(const uint8_t*, uint32_t)));

  // Link our BackfillZoneSpawns signal to the packet class backfillSpawn slot
  connect (this, SIGNAL(dispatchDecodedZoneSpawns(const uint8_t*, uint32_t)),
           parent, SLOT(dispatchDecodedZoneSpawns(const uint8_t*, uint32_t)));

  // Our key is unknown
  m_decodeKey = UNKKEY;

#if HAVE_LIBEQ
  // Initialize LIBEQ
  if (!showeq_params->broken_decode)
    InitializeLibEQ(0, ntohs(CharProfileCode), ntohs(ZoneSpawnsCode), ntohs(NewSpawnCode));

  // restore the decodeKey if the user requested it
  if (showeq_params->restoreDecodeKey)
  {
    QString fileName = showeq_params->saveRestoreBaseFilename + "Key.dat";
    QFile keyFile(fileName);
    if (keyFile.open(IO_ReadOnly))
    {
      QDataStream d(&keyFile);
      d >> (Q_UINT64)m_decodeKey;

      fprintf(stderr, "Restored KEY: 0x%016llx\n", m_decodeKey);
    }
    else
      fprintf(stderr, "Failure loading %s: Unable to open!\n",
	      (const char*)fileName);
  }
#endif
}

void
EQDecode::ResetDecoder (void)
{
  EQPktRec *pkt;
  uint i;

  // Clear our player packet queue
  for (i = 0; i < m_queuePlayerProfile.size(); i++)
  {
    pkt = (EQPktRec *)m_queuePlayerProfile[i];
    free(pkt);
  }

  // Empty the player packet queue
  m_queuePlayerProfile.clear();

  // Clear our zone spawns packet queue
  for (i = 0; i < m_queueZoneSpawns.size(); i++)
  {
    pkt = (EQPktRec *)m_queueZoneSpawns[i];
    free(pkt);
  }

  // Empty the queue
  m_queueZoneSpawns.clear();

  // Clear our spawns packet queue
  for (i = 0; i < m_queueSpawns.size(); i++)
  {
    pkt = (EQPktRec *)m_queueSpawns[i];
    free(pkt);
  }

  // Empty the queue
  m_queueSpawns.clear();

  // Set our key to zero
  m_decodeKey = UNKKEY;

  // Let anyone interested know we changed it
  emit keyChanged();
}


void EQDecode::FoundKey ()
{
  unsigned int i;
  EQPktRec *pkt;
  uint32_t decodedDataLen = PKTBUF_LEN;
  uint8_t decodedData[decodedDataLen];

#if !HAVE_LIBEQ
  printf("BUG: libEQ not present, returning from FoundKey()\n");
  return;
#endif

  // Pass on the good news!
  printf("Decrypting and dispatching with key: 0x%016llx\n", m_decodeKey);
  emit keyChanged();

  emit startDecodeBatch();

  // Decode our player packet queue
  for (i = 0; i < m_queuePlayerProfile.size(); i++)
  {
    pkt = (EQPktRec *)m_queuePlayerProfile[i];
#if HAVE_LIBEQ
    decodedDataLen = PKTBUF_LEN;
    if (ProcessPacket(pkt->data, pkt->len, 
		      decodedData, &decodedDataLen, &m_decodeKey, "",
		      NULL, 0, NULL, 0))
      emit dispatchDecodedCharProfile(decodedData, decodedDataLen);
    else
    {
      m_decodeKey = UNKKEY;
      printf("Warning: Failed to decrypt queued Player Profile packet: %d.\n", i);
      return;
    }
#endif
    free(pkt);
  }

  // Empty the player packet queue
  m_queuePlayerProfile.clear();

  // Decode our zone spawns packet queue
  for (i = 0; i < m_queueZoneSpawns.size(); i++)
  {
    pkt = (EQPktRec *)m_queueZoneSpawns[i];
#if HAVE_LIBEQ
    decodedDataLen = PKTBUF_LEN;
    if (ProcessPacket(pkt->data, pkt->len, 
		      decodedData, &decodedDataLen, &m_decodeKey, "",
		      NULL, 0, NULL, 0))
      emit dispatchDecodedZoneSpawns(decodedData, decodedDataLen);
    else
    {
      m_decodeKey = UNKKEY;
      printf("Warning: Failed to decrypt queued Zone Spawns packet: %d.\n", i);
      return;
    }
#endif
    free(pkt);
  }

  // Empty the queue
  m_queueZoneSpawns.clear();

  // Decode our spawns packet queue
#if HAVE_LIBEQ
  for (i = 0; i < m_queueSpawns.size(); i++)
  {
    pkt = (EQPktRec *)m_queueSpawns[i];
    decodedDataLen = PKTBUF_LEN;
    if (ProcessPacket(pkt->data, pkt->len, 
		      decodedData, &decodedDataLen, &m_decodeKey, "",
		      NULL, 0, NULL, 0))
       emit dispatchDecodedNewSpawn(decodedData, decodedDataLen);
    else
    {
      m_decodeKey = UNKKEY;
      printf("Warning: Failed to decrypt queued New Spawns packet: %d.\n", i);
      return;
    }
#endif
    free(pkt);
  }

  // Empty the queue
  m_queueSpawns.clear();

  if (showeq_params->saveDecodeKey)
  {
    QFile keyFile(showeq_params->saveRestoreBaseFilename + "Key.dat");
    if (keyFile.open(IO_WriteOnly))
    {
      QDataStream d(&keyFile);
      d << (Q_UINT64)m_decodeKey;
    }
  }
  
  emit finishedDecodeBatch();

  // Get outa here
  return;
}

int EQDecode::DecodePacket(const uint8_t* data, uint32_t len, 
			   uint8_t* decodedData, uint32_t* decodedDataLen,
			   const char *cli)
{
  uint16_t opcode;
  EQPktRec *player, *pktrec;
  int result;
  uint64_t prevKey;
  opcode = *(uint16_t *)data;

  // Get the opcode of the current packet
  // Check to see if it is a compressed packet
  if ((opcode == CPlayerItemsCode) || (opcode == CDoorSpawnsCode) || (opcode == cItemInShopCode))
  {
     uint8_t pbUncompressedData[PKTBUF_LEN-6];
     uint32_t nUncompressedLength;
     int nErrorCode;

     // Initialize the max buffer size for zlib
     nUncompressedLength = sizeof(pbUncompressedData);

     // Inflate the packet
     nErrorCode = InflatePacket( &data[6], len - 6, 
				 pbUncompressedData, &nUncompressedLength);
	
     if( nErrorCode )
     {
       // copy the first 6 bytes
       memcpy (decodedData, data, 6);

       // copy the uncompressed data to the rest of the buffer
       memcpy( decodedData+6, pbUncompressedData , nUncompressedLength );

       // size of decoded data
       *decodedDataLen = nUncompressedLength + 6;
     }
     return nErrorCode;
  }

#if HAVE_LIBEQ
  if (showeq_params->broken_decode)
    return 0;
#else
  if (showeq_params->broken_decode)
  {
    printf("BUG: broken_decode should not be enabled if libEQ.a support not present\n");
    exit(0);
  }
#endif

  // Skip if its not an encrypted packet
  if (!((opcode == CharProfileCode) || (opcode == ZoneSpawnsCode) || (opcode == NewSpawnCode)))
     return 0;


  // Grab our player packet
  if (m_queuePlayerProfile.size())
     player = m_queuePlayerProfile[0];
  else
     player = NULL;

  prevKey = m_decodeKey;

  // Try processing the packet first
#if HAVE_LIBEQ
  if (player)
     result = ProcessPacket(data, len, 
			    decodedData, decodedDataLen, 
			    &m_decodeKey, cli, player->data, player->len, m_decodeHash, m_decodeHlen);
  else
     result = ProcessPacket(data, len, 
			    decodedData, decodedDataLen, &m_decodeKey, cli,
			    NULL, 0, NULL, 0);
#else
  result = 0;
#endif

  if (result && player) // only the end if there is a player
  {
    // Check to see if we found a key
    if ( prevKey != m_decodeKey)
    {
      printf("DecodePacket(): FOUND KEY: 0x%016llx\n", m_decodeKey);

      FoundKey();
    }
    return result;
  }
  else if (showeq_params->restoreDecodeKey)
  {
    // if the ProcessPacket failed and this is a restored session, clear
    // the restored decode key, it's probably invalid
    m_decodeKey = 0;

    // also note that the restored decode key is no longer being used
    showeq_params->restoreDecodeKey = false;
  }
  else if (result)
    return result;

  // Queue it up...  Allocate storage for our encrypted packet
  pktrec = (EQPktRec *)malloc(sizeof(EQPktRec) + len);

  // Bail if we can't get memory...
  if (!pktrec)
  {
    printf("Warning: Couldn't allocate memory to queue encrypted packet.\n");
    // Release our mutex
    return(result);
  }

  // Grab the packet info
  pktrec->len = len;
  memcpy(pktrec->data, data, len);

  // Queue the packet
  switch (opcode)
  {
    case CharProfileCode:
      m_queuePlayerProfile.insert(m_queuePlayerProfile.begin(), pktrec);
      break;

    case ZoneSpawnsCode:
      m_queueZoneSpawns.insert(m_queueZoneSpawns.begin(), pktrec);
      break;

    case NewSpawnCode:
      m_queueSpawns.insert(m_queueSpawns.begin(), pktrec);
      break;
  }

  // Get outa here...
  return(result);
}

bool EQDecode::event(QEvent * e)
{
  switch(e->type())
  {
  case FoundKeyEvent::Found:
    // call FoundKey() from the GUI thread
    FoundKey();
    return true;
  default:
    return QObject::event(e);
  }
}

int EQDecode::InflatePacket(const uint8_t *pbDataIn, uint32_t cbDataInLen, 
			    uint8_t* pbDataOut, uint32_t* pcbDataOutLen)
{
	z_stream zstream;
	int zerror = 0;
	int i;

	zstream.next_in = (Bytef*)pbDataIn;
	zstream.avail_in = cbDataInLen;
	zstream.next_out = (Bytef*)pbDataOut;
	zstream.avail_out = *pcbDataOutLen;
	zstream.zalloc = NULL;
	zstream.zfree = NULL;
	zstream.opaque = NULL;
	
	i = inflateInit2( &zstream, 15 ); 
	if (i != Z_OK) 
	{ 
		*pcbDataOutLen = 0;
		return 0;
	}

	zerror = inflate( &zstream, Z_FINISH );

	if(zerror == Z_STREAM_END)
	{
		inflateEnd( &zstream );
		*pcbDataOutLen = zstream.total_out;
		return 1;
	}
	else
	{
#ifdef DIAG_DECODE_ZLIB_ERROR
	  printf("InflatePacket: Failed! inflate() returned %d '%s'\n",
		 zerror, zstream.msg);
#endif
		zerror = inflateEnd( &zstream );
		*pcbDataOutLen = 0;
		return 0;
	}	
}

void EQDecode::setHash(uint8_t* data, uint32_t len)
{
      for (uint32_t i =0; i< len; i++)
          m_decodeHash[i] = data[i];

      m_decodeHlen = len;
}

void EQDecode::theKey(uint64_t key)
{
  if (m_decodeKey == key || key == UNKKEY)
      return;

  m_decodeKey = key;
  // post the FoundKeyEvent to the main/GUI thread to be handled their
  QApplication::postEvent(this, new FoundKeyEvent());
}

void EQDecode::loadKey()
{
  uint32_t tkey1, tkey2;
  uint64_t tkey64;

  // restore the decodeKey if the user requested it
    QString fileName = showeq_params->KeyBaseFilename;
    QFile keyFile(fileName);
    if (keyFile.open(IO_ReadOnly))
    {
      QDataStream d(&keyFile);
      d.setByteOrder(QDataStream::LittleEndian);

      d >> tkey1 >> tkey2;
      tkey64 = tkey2;
      m_decodeKey = (tkey64 <<32)| tkey1;

      fprintf(stderr, "Loaded KEY: 0x%016llx\n", m_decodeKey);
      // post the FoundKeyEvent to the main/GUI thread to be handled their
      QApplication::postEvent(this, new FoundKeyEvent());
    }
    else
      fprintf(stderr, "Failure loading %s: Unable to open!\n",
	      (const char*)fileName);
}

