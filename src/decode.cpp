/*
 * Decode.cpp
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#include <stdio.h>
#include <pthread.h>
#include <zlib.h>

#include <string.h>

#include <qapplication.h>

#include <qfile.h>
#include <qdatastream.h>

#include "main.h"
#include "decode.h"
#include "opcodes.h"
#include "libeq.h"

//#define DIAG_DECODE_ZLIB_ERROR

#define PKTBUF_LEN	65535

void *
ThreadFunc (void *param)
{
  EQDecode *decoder;

  // Point to the decoder object
  decoder = (EQDecode *)param;

  // Do our voodoo
  decoder->LocateKey();

  // Get outa here
  return(NULL);
}

EQDecode::EQDecode (QObject *parent = 0, const char *name = 0)
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
  m_decodeKey = 0;

  // We're not locating keys
  m_locateActive = false;

  // Create our queue mutex
  pthread_mutex_init (&m_mutexQueue, NULL);

#if HAVE_LIBEQ
  // Initialize LIBEQ
  if (!showeq_params->broken_decode)
    InitializeLibEQ(0, CharProfileCode, ZoneSpawnsCode, NewSpawnCode);

  // restore the decodeKey if the user requested it
  if (showeq_params->restoreDecodeKey)
  {
    QString fileName = showeq_params->saveRestoreBaseFilename + "Key.dat";
    QFile keyFile(fileName);
    if (keyFile.open(IO_ReadOnly))
    {
      QDataStream d(&keyFile);
      d >> m_decodeKey;

      fprintf(stderr, "Restored KEY: 0x%08x\n", m_decodeKey);
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

  // Kill our search thread if active
  if (m_locateActive)
  {
    // Request cancellation and wait for it if successful
    if (pthread_cancel(m_locateThread) == 0)
      pthread_join(m_locateThread, NULL);

    // Unlock our mutex (in case thread left it locked)
    pthread_mutex_unlock(&m_mutexQueue);
  }

  // NOTE: Since we know the locate thread is not running and since we are
  //       the main thread, we don't need to use the mutex to clear the queues.

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
  m_decodeKey = 0;

  // Let anyone interested know we changed it
  emit keyChanged();

  // We're no longer looking
  m_locateActive = false;
}

void
EQDecode::LocateKey ()
{
  unsigned int i;
  EQPktRec *player, *spawn;

  // Grab our player packet
  player = m_queuePlayerProfile[0];

#if HAVE_LIBEQ
  // Find a shiny new key
  i = 0;
  while (m_locateActive && !m_decodeKey && (i < m_queueSpawns.size()))
  {
    spawn = m_queueSpawns[i];
    m_decodeKey = FindKey(player->data, player->len, spawn->data, spawn->len);
    i++;
  }
#else
  m_decodeKey = 0;
#endif

  // Bail if we didn't find a key at all
  if (!m_decodeKey)
  {
    m_locateActive = false;
    return;
  }

  printf("LocateKey(): FOUND KEY: 0x%08x\n", m_decodeKey);

  // post the FoundKeyEvent to the main/GUI thread to be handled their
  QApplication::postEvent(this, new FoundKeyEvent());
}

void
EQDecode::FoundKey ()
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
  printf("Decrypting and dispatching with key: 0x%08x\n", m_decodeKey);
  emit keyChanged();

  // Check to see if we've been cancelled
  pthread_testcancel();

  // Lock our mutex
  pthread_mutex_lock(&m_mutexQueue);

  // Decode our player packet queue
  for (i = 0; i < m_queuePlayerProfile.size(); i++)
  {
    pkt = (EQPktRec *)m_queuePlayerProfile[i];
#if HAVE_LIBEQ
    decodedDataLen = PKTBUF_LEN;
    if (ProcessPacket(pkt->data, pkt->len, 
		      decodedData, &decodedDataLen, &m_decodeKey, ""))
      emit dispatchDecodedCharProfile(decodedData, decodedDataLen);
    else
#endif
      printf("Warning: Failed to decrypt queued Player Profile packet: %d.\n", i);
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
		      decodedData, &decodedDataLen, &m_decodeKey, ""))
      emit dispatchDecodedZoneSpawns(decodedData, decodedDataLen);
    else
#endif
      printf("Warning: Failed to decrypt queued Zone Spawns packet: %d.\n", i);
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
    DecodeSpawn(pkt->data, pkt->len, decodedData, &decodedDataLen, 
		m_decodeKey);
    emit dispatchDecodedNewSpawn(decodedData, decodedDataLen);
    free(pkt);
  }
#endif

  // Empty the queue
  m_queueSpawns.clear();

  // Unlock our mutex
  pthread_mutex_unlock(&m_mutexQueue);

  // Let them know we're done
  m_locateActive = false;

  if (showeq_params->saveDecodeKey)
  {
    QFile keyFile(showeq_params->saveRestoreBaseFilename + "Key.dat");
    if (keyFile.open(IO_WriteOnly))
    {
      QDataStream d(&keyFile);
      d << m_decodeKey;
    }
  }

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
  uint32_t prevKey;
  opcode = data[1] | (data[0] << 8);

  // Get the opcode of the current packet
  // Check to see if it is a compressed packet
  if ((opcode == CPlayerItemsCode) || (opcode == CDoorSpawnsCode))
  {
     uint8_t pbUncompressedData[PKTBUF_LEN-4];
     uint32_t nUncompressedLength;
     int nErrorCode;

     // Initialize the max buffer size for zlib
     nUncompressedLength = sizeof(pbUncompressedData);

     // Inflate the packet
     nErrorCode = InflatePacket( &data[4], len - 4, 
				 pbUncompressedData, &nUncompressedLength);
	
     if( nErrorCode )
     {
       // copy the first 4 bytes
       *(uint32_t*)decodedData = *(uint32_t*)data;

       // copy the uncompressed data to the rest of the buffer
       memcpy( decodedData+4, pbUncompressedData , nUncompressedLength );

       // size of decoded data
       *decodedDataLen = nUncompressedLength + 4;
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

  // Lock our mutex so the queues can't unwind before this packet is in
  // NOTE: We only need to lock when the queue's are being used
  if (m_locateActive)
     pthread_mutex_lock(&m_mutexQueue);
 
  prevKey = m_decodeKey;

  // Try processing the packet first
#if HAVE_LIBEQ
  if (player)
     result = ProcessPacket(data, len, 
			    decodedData, decodedDataLen, 
			    &m_decodeKey, cli, player->data, player->len);
  else
     result = ProcessPacket(data, len, 
			    decodedData, decodedDataLen, &m_decodeKey, cli);
#else
  result = 0;
#endif

  if (result)
  {
    // Release our mutex
    if (m_locateActive)
    {
       if (pthread_cancel(m_locateThread) == 0)
          pthread_join(m_locateThread, NULL);

       pthread_mutex_unlock(&m_mutexQueue);
    }
    // Check to see if we found a key
    if ( prevKey != m_decodeKey)
    {
      printf("DecodePacket(): FOUND KEY: 0x%08x\n", m_decodeKey);

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

  // Queue it up...  Allocate storage for our encrypted packet
  if ((opcode == CharProfileCode) || (opcode == ZoneSpawnsCode))
    pktrec = (EQPktRec *)malloc(sizeof(EQPktRec) + len);
  else if (opcode == NewSpawnCode)
    pktrec = (EQPktRec *)malloc(sizeof(EQPktRec) + len);

  // Bail if we can't get memory...
  if (!pktrec)
  {
    printf("Warning: Couldn't allocate memory to queue encrypted packet.\n");
    // Release our mutex
    if (m_locateActive)
      pthread_mutex_unlock(&m_mutexQueue);
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

  // Release the mutex and return if the locate thread is already running
  if (m_locateActive)
  {
    pthread_mutex_unlock(&m_mutexQueue);
    return(result);
  }

  // Start the locate thread if we have player/spawn packets available
  if (m_queuePlayerProfile.size() && m_queueSpawns.size())
  {
    // Tell everyone we're active and do it BEFORE creating the thread
    // whose initialization depends on it being non-zero
    m_locateActive = true;

    // Create the thread with standard scheduling parameters
    pthread_create(&m_locateThread, NULL, ThreadFunc, this);

    // And don't retain any results
    pthread_detach(m_locateThread); 
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
