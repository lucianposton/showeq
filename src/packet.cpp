/*
 * packet.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

/* Implementation of Packet class */
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

#include <netinet/if_ether.h>

#include <qtimer.h>

#include "everquest.h"
#include "opcodes.h"
#include "packet.h"
#include "packetcapture.h"
#include "packetformat.h"
#include "packetstream.h"
#include "main.h"
#include "vpacket.h"

//----------------------------------------------------------------------
// Macros

//#define DEBUG_PACKET
//#undef DEBUG_PACKET

// The following defines are used to diagnose packet handling behavior
// this define is used to diagnose packet processing (in dispatchPacket mostly)
//#define PACKET_PROCESS_DIAG 3 // verbosity level 0-n

// this define is used to diagnose cache handling (in dispatchPacket mostly)
//#define PACKET_CACHE_DIAG 3 // verbosity level (0-n)

// diagnose structure size changes
#define PACKET_PAYLOAD_SIZE_DIAG 1

// Packet version is a unique number that should be bumped every time packet
// structure (ie. encryption) changes.  It is checked by the VPacket feature
// (currently the date of the last packet structure change)
#define PACKETVERSION  40100

//----------------------------------------------------------------------
// constants

const in_port_t WorldServerGeneralPort = 9000;
const in_port_t WorldServerChatPort = 9876;
const in_port_t LoginServerMinPort = 15000;
const in_port_t LoginServerMaxPort = 15010;
const in_port_t ChatServerPort = 5998;

//----------------------------------------------------------------------
// Here begins the code


////////////////////////////////////////////////////
// This code handles packet payload size validation
#ifdef PACKET_PAYLOAD_SIZE_DIAG
bool validatePayloadSize(int len, int size, uint16_t code,
			 const char* clarifier,
			 const char* codeName, const char* structName)
{
  // verify size
  if (len != size)
  {
    fprintf(stderr, "WARNING: %s%s (%04x) (dataLen:%d != sizeof(%s):%d)!\n",
	    clarifier, codeName, code, len, structName, size);
    return false;
  }
  return true;
}

#define ValidatePayload(codeName, structName) \
  validatePayloadSize(len, sizeof( structName ), codeName, \
		      "", #codeName , #structName )
#endif


//----------------------------------------------------------------------
// EQPacket class methods

/* EQPacket Class - Sets up packet capturing */

////////////////////////////////////////////////////
// Constructor
EQPacket::EQPacket (QObject * parent, const char *name)
  : QObject (parent, name),
    m_packetCapture(NULL),
    m_vPacket(NULL),
    m_timer(NULL),
    m_busy_decoding(false)
{
  // Setup the data streams

  // Setup client -> world stream
  m_client2WorldStream = new EQPacketStream(client2world, DIR_CLIENT, 
					    showeq_params->arqSeqGiveUp,
					    this, "client2world");
  connect(m_client2WorldStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2WorldStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(decodedWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2WorldStream, 
	  SIGNAL(dispatchData(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SLOT(dispatchWorldData(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2WorldStream, 
	  SIGNAL(cacheSize(int, int)),
	  this,
	  SIGNAL(cacheSize(int, int)));
  connect(m_client2WorldStream, 
	  SIGNAL(seqReceive(int, int)),
	  this,
	  SIGNAL(seqReceive(int, int)));
  connect(m_client2WorldStream, 
	  SIGNAL(seqExpect(int, int)),
	  this,
	  SIGNAL(seqExpect(int, int)));
  connect(m_client2WorldStream, 
	  SIGNAL(numPacket(int, int)),
	  this,
	  SIGNAL(numPacket(int, int)));
  
  // Setup world -> client stream
  m_world2ClientStream = new EQPacketStream(world2client, DIR_SERVER,
					    showeq_params->arqSeqGiveUp,
					    this, "world2client");
  connect(m_world2ClientStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_world2ClientStream, 

 	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(decodedWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_world2ClientStream, 
	  SIGNAL(dispatchData(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SLOT(dispatchWorldData(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_world2ClientStream, 
	  SIGNAL(cacheSize(int, int)),
	  this,
	  SIGNAL(cacheSize(int, int)));
  connect(m_world2ClientStream, 
	  SIGNAL(seqReceive(int, int)),
	  this,
	  SIGNAL(seqReceive(int, int)));
  connect(m_world2ClientStream, 
	  SIGNAL(seqExpect(int, int)),
	  this,
	  SIGNAL(seqExpect(int, int)));
  connect(m_world2ClientStream, 
	  SIGNAL(numPacket(int, int)),
	  this,
	  SIGNAL(numPacket(int, int)));

  // Setup client -> zone stream
  m_client2ZoneStream = new EQPacketStream(client2zone, DIR_CLIENT,
					  showeq_params->arqSeqGiveUp,
					  this, "client2zone");
  connect(m_client2ZoneStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2ZoneStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2ZoneStream, 
	  SIGNAL(dispatchData(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SLOT(dispatchZoneData(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2ZoneStream, 
	  SIGNAL(cacheSize(int, int)),
	  this,
	  SIGNAL(cacheSize(int, int)));
  connect(m_client2ZoneStream, 
	  SIGNAL(seqReceive(int, int)),
	  this,
	  SIGNAL(seqReceive(int, int)));
  connect(m_client2ZoneStream, 
	  SIGNAL(seqExpect(int, int)),
	  this,
	  SIGNAL(seqExpect(int, int)));
  connect(m_client2ZoneStream, 
	  SIGNAL(numPacket(int, int)),
	  this,
	  SIGNAL(numPacket(int, int)));

  // Setup zone -> client stream
  m_zone2ClientStream = new EQPacketStream(zone2client, DIR_SERVER,
					   showeq_params->arqSeqGiveUp,
					   this, "zone2client");
  connect(m_zone2ClientStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_zone2ClientStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_zone2ClientStream, 
	  SIGNAL(dispatchData(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SLOT(dispatchZoneData(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_zone2ClientStream, 
	  SIGNAL(cacheSize(int, int)),
	  this,
	  SIGNAL(cacheSize(int, int)));
  connect(m_zone2ClientStream, 
	  SIGNAL(seqReceive(int, int)),
	  this,
	  SIGNAL(seqReceive(int, int)));
  connect(m_zone2ClientStream, 
	  SIGNAL(seqExpect(int, int)),
	  this,
	  SIGNAL(seqExpect(int, int)));
  connect(m_zone2ClientStream, 
	  SIGNAL(numPacket(int, int)),
	  this,
	  SIGNAL(numPacket(int, int)));
  // Zone to client stream specific signals (session tracking non-sense)
  connect(m_zone2ClientStream, 
	  SIGNAL(sessionTrackingChanged(uint8_t)),
	  this,
	  SIGNAL(sessionTrackingChanged(uint8_t)));
  connect(m_zone2ClientStream, 
	  SIGNAL(lockOnClient(in_port_t, in_port_t)),
	  this,
	  SLOT(lockOnClient(in_port_t, in_port_t)));
  connect(m_zone2ClientStream, 
	  SIGNAL(closing()),
	  this,
	  SLOT(closeStream()));

  // Initialize convenient streams array
  m_streams[client2world] = m_client2WorldStream;
  m_streams[world2client] = m_world2ClientStream;
  m_streams[client2zone] = m_client2ZoneStream;
  m_streams[zone2client] = m_zone2ClientStream;

  // no client/server ports yet
  m_clientPort = 0;
  m_serverPort = 0;
  
  struct hostent *he;
  struct in_addr  ia;
  if (showeq_params->ip.isEmpty() && showeq_params->mac_address.isEmpty())
  {
    printf ("No address specified\n");
    exit(0);
  }
  
  if (!showeq_params->ip.isEmpty())
  {
    /* Substitute "special" IP which is interpreted 
       to set up a different filter for picking up new sessions */
    
    if (showeq_params->ip == "auto")
      inet_aton (AUTOMATIC_CLIENT_IP, &ia);
    else if (inet_aton (showeq_params->ip, &ia) == 0)
    {
      he = gethostbyname(showeq_params->ip);
      if (!he)
      {
	printf ("Invalid address; %s\n", (const char*)showeq_params->ip);
	exit (0);
      }
      memcpy (&ia, he->h_addr_list[0], he->h_length);
    }
    m_client_addr = ia.s_addr;
    showeq_params->ip = inet_ntoa(ia);
    
    if (showeq_params->ip ==  AUTOMATIC_CLIENT_IP)
    {
      m_detectingClient = true;
      printf("Listening for first client seen.\n");
    }
    else
    {
      m_detectingClient = false;
      printf("Listening for client: %s\n",
	     (const char*)showeq_params->ip);
    }
  }
  
  if (!showeq_params->playbackpackets)
  {
    // create the pcap object and initialize, either with MAC or IP
    m_packetCapture = new PacketCaptureThread();
    if (showeq_params->mac_address.length() == 17)
      m_packetCapture->start(showeq_params->device, 
			     showeq_params->mac_address, 
			     showeq_params->realtime, MAC_ADDRESS_TYPE );
    else
      m_packetCapture->start(showeq_params->device,
			     showeq_params->ip, 
			     showeq_params->realtime, IP_ADDRESS_TYPE );
    emit filterChanged();
  }
  
  /* Create timer object */
  m_timer = new QTimer (this);
  
  if (!showeq_params->playbackpackets)
    connect (m_timer, SIGNAL (timeout ()), this, SLOT (processPackets ()));
  else
    connect (m_timer, SIGNAL (timeout ()), this, SLOT (processPlaybackPackets ()));
  
  /* setup VPacket */
  m_vPacket = NULL;
  
  QString section = "VPacket";
  // First param to VPacket is the filename
  // Second param is playback speed:  0 = fast as poss, 1 = 1X, 2 = 2X etc
  if (pSEQPrefs->isPreference("Filename", section))
  {
    const char *filename = pSEQPrefs->getPrefString("Filename", section);
    
    if (showeq_params->recordpackets)
    {
      m_vPacket = new VPacket(filename, 1, true);
      // Must appear befire next call to getPrefString, which uses a static string
      printf("Recording packets to '%s' for future playback\n", filename);
      
      if (pSEQPrefs->getPrefString("FlushPackets", section))
	m_vPacket->setFlushPacket(true);
    }
    else if (showeq_params->playbackpackets)
    {
      m_vPacket = new VPacket(filename, 1, false);
      m_vPacket->setCompressTime(pSEQPrefs->getPrefInt("CompressTime", section, 0));
      m_vPacket->setPlaybackSpeed(showeq_params->playbackspeed);
      
      printf("Playing back packets from '%s' at speed '%d'\n", filename,
	     
	     showeq_params->playbackspeed);
    }
  }
  else
  {
    showeq_params->recordpackets = 0;
    showeq_params->playbackpackets = 0;
  }
}

////////////////////////////////////////////////////
// Destructor
EQPacket::~EQPacket()
{
  if (m_packetCapture != NULL)
  {
    // stop any packet capture 
    m_packetCapture->stop();

    // delete the object
    delete m_packetCapture;
  }

  // try to close down VPacket cleanly
  if (m_vPacket != NULL)
  {
    // make sure any data is flushed to the file
    m_vPacket->Flush();

    // delete VPacket
    delete m_vPacket;
  }

  if (m_timer != NULL)
  {
    // make sure the timer is stopped
    m_timer->stop();

    // delete the timer
    delete m_timer;
  }

  resetEQPacket();

  delete m_client2WorldStream;
  delete m_world2ClientStream;
  delete m_client2ZoneStream;
  delete m_zone2ClientStream;
}

/* Start the timer to process packets */
void EQPacket::start (int delay)
{
#ifdef DEBUG_PACKET
   debug ("start()");
#endif /* DEBUG_PACKET */
   m_timer->start (delay, false);
}

/* Stop the timer to process packets */
void EQPacket::stop (void)
{
#ifdef DEBUG_PACKET
   debug ("stop()");
#endif /* DEBUG_PACKET */
   m_timer->stop ();
}

/* Reads packets and processes waiting packets */
void EQPacket::processPackets (void)
{
  /* Make sure we are not called while already busy */
  if (m_busy_decoding)
     return;

  /* Set flag that we are busy decoding */
  m_busy_decoding = true;
  
  unsigned char buffer[BUFSIZ]; 
  short size;
  
  /* fetch them from pcap */
  while ((size = m_packetCapture->getPacket(buffer)))
  {
    /* Now.. we know the rest is an IP udp packet concerning the
     * host in question, because pcap takes care of that.
     */
      
    /* Now we assume its an everquest packet */
    if (showeq_params->recordpackets)
    {
      time_t now = time(NULL);
      m_vPacket->Record((const char *) buffer, size, now, PACKETVERSION);
    }
      
    dispatchPacket (size - sizeof (struct ether_header),
		  (unsigned char *) buffer + sizeof (struct ether_header) );
  }

  /* Clear decoding flag */
  m_busy_decoding = false;
}

////////////////////////////////////////////////////
// Reads packets and processes waiting packets from playback file
void EQPacket::processPlaybackPackets (void)
{
#ifdef DEBUG_PACKET
//   debug ("processPackets()");
#endif /* DEBUG_PACKET */
  /* Make sure we are not called while already busy */
  if (m_busy_decoding)
    return;

  /* Set flag that we are busy decoding */
  m_busy_decoding = true;

  unsigned char  buffer[8192];
  int            size;

  /* in packet playback mode fetch packets from VPacket class */
  time_t now;
  int timein = mTime();
  int i = 0;
    
  long version = PACKETVERSION;
  
  // decode packets from the playback buffer
  do
  {
    size = m_vPacket->Playback((char *) buffer, sizeof(buffer), &now, &version);
    
    if (size)
    {
      i++;
	
      if (PACKETVERSION == version)
      {
	dispatchPacket ( size - sizeof (struct ether_header),
		       (unsigned char *) buffer + sizeof (struct ether_header)
		       );
      }
      else
      {
	fprintf( stderr, "Error:  The version of the packet stream has " \
		 "changed since '%s' was recorded - disabling playback\n",
		 m_vPacket->getFileName());

	// stop the timer, nothing more can be done...
	stop();

	break;
      }
    }
    else
      break;
  } while ( (mTime() - timein) < 100);

  // check if we've reached the end of the recording
  if (m_vPacket->endOfData())
  {
    fprintf(stderr, "End of playback file '%s' reached.\n"
	    "Playback Finished!\n",
	    m_vPacket->getFileName());

    // stop the timer, nothing more can be done...
    stop();
  }

  /* Clear decoding flag */
  m_busy_decoding = false;
}

////////////////////////////////////////////////////
// This function decides the fate of the Everquest packet 
// and dispatches it to the correct packet stream for handling function
void EQPacket::dispatchPacket(int size, unsigned char *buffer)
{
#ifdef DEBUG_PACKET
  debug ("dispatchPacket()");
#endif /* DEBUG_PACKET */
  /* Setup variables */

  // Create an object to parse the packet
  EQUDPIPPacketFormat packet(buffer, size, false);

  // signal a new packet
  emit newPacket(packet);
  
  /* Chat and Login Server Packets, Discard for now */
  if ((packet.getDestPort() == ChatServerPort) ||
      (packet.getSourcePort() == ChatServerPort))
    return;

  if ( ((packet.getDestPort() >= LoginServerMinPort) ||
     (packet.getSourcePort() >= LoginServerMinPort)) &&
     ((packet.getDestPort() <= LoginServerMaxPort) ||
     (packet.getSourcePort() <= LoginServerMaxPort)) )
    return;

  /* discard pure ack/req packets and non valid flags*/
  if (packet.flagsHi() < 0x02 || packet.flagsHi() > 0x46 || size < 10)
  {
#if defined(PACKET_PROCESS_DIAG)
    printf("discarding packet %s:%d ==>%s:%d flagsHi=%d size=%d\n",
	   (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
	   (const char*)packet.getIPv4DestA(), packet.getDestPort(),
	   packet.flagsHi(), size);
    printf("%s\n", (const char*)packet.headerFlags(false));
#endif
    return;    
  }

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
  printf("%s\n", (const char*)packet.headerFlags((PACKET_PROCESS_DIAG < 3)));
  uint32_t crc = packet.calcCRC32();
  if (crc != packet.crc32())
    printf("CRC: Warning Packet seq = %d CRC (%08x) != calculated CRC (%08x)!\n",
	   packet.seq(), packet.crc32(), crc);
#endif
  
  if (!packet.isValid())
  {
    printf("INVALID PACKET: Bad CRC32 [%s:%d -> %s:%d] seq %04x len %d crc32 (%08x != %08x)\n",
	   (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
	   (const char*)packet.getIPv4DestA(), packet.getDestPort(),
	   packet.seq(), 
	   packet.payloadLength(),
	   packet.crc32(), packet.calcCRC32());
    return;
  }

  /* Client Detection */
  if (m_detectingClient && packet.getSourcePort() == WorldServerGeneralPort)
  {
    showeq_params->ip = packet.getIPv4DestA();
    m_client_addr = packet.getIPv4DestN();
    m_detectingClient = false;
    emit clientChanged(m_client_addr);
    printf("Client Detected: %s\n", (const char*)showeq_params->ip);
  }
  else if (m_detectingClient && packet.getDestPort() == WorldServerGeneralPort)
  {
    showeq_params->ip = packet.getIPv4SourceA();
    m_client_addr = packet.getIPv4SourceN();
    m_detectingClient = false;
    emit clientChanged(m_client_addr);
    printf("Client Detected: %s\n", (const char*)showeq_params->ip);
  }
  /* end client detection */



  if (packet.getSourcePort() == WorldServerChatPort)
  {
    dispatchWorldChatData(packet.payloadLength(), packet.payload(), DIR_SERVER);
    
    return;
  }
  else if (packet.getDestPort() == WorldServerChatPort)
  {
    dispatchWorldChatData(packet.payloadLength(), packet.payload(), DIR_CLIENT);
    
    return;
  }

  /* stream identification */
  if (packet.getIPv4SourceN() == m_client_addr)
  {
    if (packet.getDestPort() == WorldServerGeneralPort)
      m_client2WorldStream->handlePacket(packet);
    else 
      m_client2ZoneStream->handlePacket(packet);
  }
  else if (packet.getIPv4DestN() == m_client_addr)
  {
    if (packet.getSourcePort() == WorldServerGeneralPort)
      m_world2ClientStream->handlePacket(packet);
    else 
      m_zone2ClientStream->handlePacket(packet);
  }

  return;
} /* end dispatchPacket() */

////////////////////////////////////////////////////
// Handle zone2client stream closing
void EQPacket::closeStream()
{
  // reseting the pcap filter to a non-exclusive form allows us to beat 
  // the race condition between timer and processing the zoneServerInfo
  if(!showeq_params->playbackpackets) 
  {
    m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
			       showeq_params->realtime, IP_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }

  printf ("EQPacket: SEQClosing detected, awaiting next zone session,  pcap filter: EQ Client %s\n",
	  (const char*)showeq_params->ip);
  
  // we'll be waiting for a new SEQStart for ALL streams
  // it seems we only ever see a proper closing sequence from the zone server
  // so reset all packet sequence caches 
  resetEQPacket();
}

////////////////////////////////////////////////////
// Locks onto a specific client port (for session tracking)
void EQPacket::lockOnClient(in_port_t serverPort, in_port_t clientPort)
{
  m_serverPort = serverPort;
  m_clientPort = clientPort;

  if (!showeq_params->playbackpackets)
  {
    if (showeq_params->mac_address.length() == 17)
    {
      m_packetCapture->setFilter(showeq_params->device, 
				 showeq_params->mac_address,
				 showeq_params->realtime, 
				 MAC_ADDRESS_TYPE, 0, 
				 m_clientPort);
      emit filterChanged();
      printf ("EQPacket: SEQStart detected, pcap filter: EQ Client %s, Client port %d\n",
	      (const char*)showeq_params->mac_address, 
	      m_clientPort);
    }
    else
    {
      m_packetCapture->setFilter(showeq_params->device, 
				 showeq_params->ip,
				 showeq_params->realtime, 
				 IP_ADDRESS_TYPE, 0, 
				 m_clientPort);
      emit filterChanged();
      printf ("EQPacket: SEQStart detected, pcap filter: EQ Client %s, Client port %d\n",
	      (const char*)showeq_params->ip, 
	      m_clientPort);
    }
  }
  
  emit clientPortLatched(m_clientPort);
}


///////////////////////////////////////////
// Dispatches World data packets based on the opCode
void EQPacket::dispatchWorldData(const uint8_t *data, size_t len, 
				 uint8_t direction, uint16_t opCode)
{
#ifdef DEBUG_PACKET
  debug ("dispatchWorldData()");
#endif /* DEBUG_PACKET */

  bool unk = true;
  
  switch (opCode)
  {
  case OP_GuildList: // old GuildListCode:
    {
      if (direction != DIR_SERVER)
	break;
      
      unk = ! ValidatePayload(OP_GuildList, worldGuildListStruct);
      if (unk)
	break;
      
      emit worldGuildList((const char*)data, len);
      
      break;
    } /* end OP_GuildList */
    
  case OP_MOTD: // old MOTDCode:
    {
      if (direction != DIR_SERVER)
	break;
      
      unk = false;
      
      emit worldMOTD((const worldMOTDStruct*)data, len, direction);
      
      break;
    } 
    
  default:
    {
      unk = true;
      break;
    }
  }
} // end dispatchWorld

///////////////////////////////////////////
//EQPacket::dispatchWorldChatData  
// note this dispatch gets just the payload
void EQPacket::dispatchWorldChatData (uint32_t len, uint8_t *data, 
				      uint8_t dir)
{
#ifdef DEBUG_PACKET
  debug ("dispatchWorldChatData()");
#endif /* DEBUG_PACKET */
  if (len < 10)
    return;
  
  uint16_t opCode = eqntohuint16(data);

  switch (opCode)
  {
  default:
    printf ("%04x - %d (%s)\n", opCode, len,
	    ((dir == DIR_SERVER) ? 
	     "WorldChatServer --> Client" : "Client --> WorldChatServer"));
  }
}

///////////////////////////////////////////
// Dispatches Zone data packets based on the opCode
void EQPacket::dispatchZoneData(const uint8_t *data, size_t len, 
				uint8_t dir, uint16_t opCode)
{
#ifdef DEBUG_PACKET
    debug ("dispatchZoneData()");
#endif /* DEBUG_PACKET */

    QString  tempStr;

    bool unk = true;

    switch (opCode)
      {
      case OP_ClientUpdate: // old PlayerPosCode:
        {
#ifdef PACKET_PAYLOAD_SIZE_DIAG
	  if ((len != sizeof(playerSpawnPosStruct)) &&
	      (len != sizeof(playerSelfPosStruct)))
	  {
	    fprintf(stderr, "WARNING: OP_ClientUpdate (%04x) (dataLen: %d != sizeof(playerSpawnPosStruct):%d or sizeof(playerSpawnSelfStruct):%d)\n",
		    OP_ClientUpdate, len, 
		    sizeof(playerSpawnPosStruct), sizeof(playerSelfPosStruct));
	    unk = true;
	  }
	  else
	    unk = false;
#else
	  unk = false;
#endif
	  
	  if (len == sizeof(playerSpawnPosStruct))
	    emit playerUpdate((const playerSpawnPosStruct*)data, len, dir);
	  else if (len == sizeof(playerSelfPosStruct))
	    emit playerUpdate((const playerSelfPosStruct*)data, len, dir);
	  else
	    unk = true;
	  
	  break;
        }

      case OP_MobUpdate: // old MobUpdateCode:
        {
	  unk = ! ValidatePayload(OP_MobUpdate, spawnPositionUpdate);
	  
	  emit updateSpawns((const spawnPositionUpdate *)data, len, dir);
	  
	  break;
        }
	
      case OP_WearChange: // old SpawnUpdateCode:
        {
	  unk = ! ValidatePayload(OP_WearChange, SpawnUpdateStruct);
	  SpawnUpdateStruct *su = (SpawnUpdateStruct*)data;
//	    printf("SpawnUpdateCode(id=%d, sub=%d, arg1=%d, arg2=%d)\n", 
//		   su->spawnId, su->subcommand, 
//		   su->arg1, su->arg2);
	    /* Belith - I believe this is depreciated no? Doesn't work anyway ;) */
	  switch(su->subcommand) {
	  case 17:
	    emit updateSpawnMaxHP(su, len, dir);
	    break;
	  }
	  
	  break;
	  emit updateSpawnInfo(su, len, dir);
        }

      case OP_SpawnAppearance: // old SpawnAppearanceCode:
        {
	  unk = false;
	  
	  emit spawnAppearance((const spawnAppearanceStruct*)data, len, dir);
	  break;
        }
	
      case OP_CommonMessage: // old ChannelMessageCode:
	{
	  unk = false;
	  
	  emit channelMessage((const channelMessageStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_FormattedMessage: // old FormattedMessageCode:
	{
	  unk = false;
	  
	  emit formattedMessage((const formattedMessageStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_SimpleMessage: // old SimpleMessageCode:
	{
	  unk = false;

	  emit simpleMessage((const simpleMessageStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_SpecialMesg:
	{
	  unk = false;
	  
	  emit specialMessage((const specialMessageStruct*)data, len, dir);

	  break;
	}
	
      case OP_GuildMOTD:
	{
	  unk = false;
	  
	  emit guildMOTD((const guildMOTDStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_Death: // old NewCorpseCode:
	{
	  unk = ! ValidatePayload(OP_Death, newCorpseStruct);

	  emit killSpawn((const newCorpseStruct*) data, len, dir);
	  
	  break;
	} /* end CorpseCode */
	
      case OP_DeleteSpawn: // old DeleteSpawnCode:
	{
	  unk = ! ValidatePayload(OP_DeleteSpawn, deleteSpawnStruct);
	  
	  emit deleteSpawn((const deleteSpawnStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_ItemLinkResponse: // old ItemInfoCode:
	{
	  unk = false;
	  
	  if (dir == DIR_SERVER)
	    emit itemInfo((const itemInfoStruct*)data, len, dir);
	  else
	    emit itemInfoReq((const itemInfoReqStruct*)data, len, dir);
	}
	
      case OP_ItemPacket: // old ItemCode:
	{
	  unk = false;
	  
	  if (dir == DIR_SERVER)
	    emit item((const itemPacketStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_ItemPlayerPacket:
	{
	  unk = false;
	  
	  if (dir == DIR_SERVER)
	    emit playerItem((const char*)data, len, dir);
	  
	  break;
	}

      case OP_NewSpawn: // old NewSpawnCode:
	{
	  unk = ! ValidatePayload(OP_NewSpawn, newSpawnStruct);
	  
	  emit newSpawn((const newSpawnStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_ItemTextFile: // old BookTextCode:
	{
	  unk = false;
	  
	  printf("BOOK: '%s'\n", ((const bookTextStruct *)data)->text);
	  emit bookText((const bookTextStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_MoneyOnCorpse: // old MoneyOnCorpseCode:
	{
	  unk = ! ValidatePayload(OP_MoneyOnCorpse, moneyOnCorpseStruct);
	  
	  emit moneyOnCorpse((const moneyOnCorpseStruct*)data, len, dir);
	  
	  break;
	} /* end MoneyOnCorpseCode */
	
      case OP_RandomReply: // old RandomCode:
        {
	  unk = ! ValidatePayload(OP_RandomReply, randomStruct);
	  
	  emit random((const randomStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_RandomReq: // RandomReqCode:
        {
	  unk = ! ValidatePayload(OP_RandomReq, randomReqStruct);
	  
	  emit random((const randomReqStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_Emote: // old EmoteEmoteTextCode:
        {
	  unk = false;
	  
	  emit emoteText((const emoteTextStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_CorpseLocResponse: // old CorpseLocCode:
        {
	  unk = ! ValidatePayload(OP_CorpseLocResponse, corpseLocStruct);
	  
	  emit corpseLoc((const corpseLocStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_InspectAnswer: // old InspectDataCode:
        {
	  unk = ! ValidatePayload(OP_InspectAnswer, inspectDataStruct);
	  
	  emit inspectData((const inspectDataStruct *)data, len, dir);
	  
	  break;
        }
	
      case OP_HPUpdate: // old NpcHpUpdateCode:
	{
	  unk = ! ValidatePayload(OP_HPUpdate, hpNpcUpdateStruct);
	  
	  emit updateNpcHP((const hpNpcUpdateStruct*)data, len, dir);
	  
	  break;
	}
	
      case SPMesgCode:
        {
	  unk = false;
	  
	  emit spMessage((const spMesgStruct *)data, len, dir);
	  
	  break;
        }
	
      case OP_MemorizeSpell: // old MemSpellCode:
        {
	  unk = ! ValidatePayload(OP_MemorizeSpell, memSpellStruct);
	  
	  emit handleSpell((const memSpellStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_BeginCast: // old BeginCastCode
        {
	  unk = ! ValidatePayload(OP_BeginCast, beginCastStruct);
	  
	  emit beginCast((const beginCastStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_CastSpell: // old StartCastCode:
        {
	  unk = ! ValidatePayload(OP_CastSpell, startCastStruct);
	  
	  emit startCast((const startCastStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_BuffFadeMsg: // old SpellFadeCode:
	{
	  unk = false;
	  
	  emit spellFaded((const spellFadedStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_ExpUpdate: // old ExpUpdateCode:
        {
	  unk = ! ValidatePayload(OP_ExpUpdate, expUpdateStruct);
	  
	  emit updateExp((const expUpdateStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_LevelUpdate: // old LevelUpUpdateCode:
        {
	  unk = ! ValidatePayload(OP_LevelUpdate, levelUpUpdateStruct);
	  
	  emit updateLevel((const levelUpUpdateStruct *)data, len, dir);
	  
	  break;
        }
	
      case OP_SkillUpdate: // old SkillIncCode
        {
	  unk = ! ValidatePayload(OP_SkillUpdate, skillIncStruct);
	  
	  emit increaseSkill((const skillIncStruct *)data, len, dir);
	  
	  break;
        }
	
      case OP_MoveDoor: // old DoorOpenCode:
        {
	  unk = false;
	  
	  emit doorOpen(data, len, dir);
	  
	  break;
        }
	
      case OP_Illusion: // old IllusionCode:
        {
	  unk = false;
	  
	  emit illusion(data, len, dir);
	  
	  break;
        }
	
      case OP_ZoneChange: // old ZoneChangeCode:
        {
	  unk = ! ValidatePayload(OP_ZoneChange, zoneChangeStruct);
	  
	  // in the process of zoning, server hasn't switched yet.
	  
	  emit zoneChange((const zoneChangeStruct*)data, len, dir);
	  break;
        }
	
      case OP_ZoneEntry: // old ZoneEntryCode:
        {
	  // We're only interested in the server version
	  
	  if (dir == DIR_CLIENT)
	  {
	    unk = ! ValidatePayload(OP_ZoneEntry, ClientZoneEntryStruct);
	    emit zoneEntry((const ClientZoneEntryStruct*)data, len, dir);
	    break;
	  }
	  
	  unk = ! ValidatePayload(OP_ZoneEntry, ServerZoneEntryStruct);
	  
	  emit zoneEntry((const ServerZoneEntryStruct*)data, len, dir);
	  
	  break;
        } /* end ZoneEntryCode */
	
      case OP_NewZone: // old - NewZoneCode:
        {
	  unk = ! ValidatePayload(OP_NewZone, newZoneStruct);
	  
	  emit zoneNew((const newZoneStruct*)data, len, dir);
	  
	  if (m_vPacket)
	    printf("New Zone at byte: %ld\n", m_vPacket->FilePos());
	  
	  break;
        }
	
      case OP_PlayerProfile:	// Character Profile server to client - old CharProfileCode
	{
	  unk = false;
	  
	  ValidatePayload(OP_PlayerProfile, charProfileStruct);
	  
	  emit backfillPlayer((const charProfileStruct*)data, len, DIR_SERVER);
	  
	  break;
	}
	
      case OP_ZoneSpawns: // ZoneSpawnsCode:
	{
	  unk = false; 
	  
	  emit zoneSpawns((const zoneSpawnsStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_TimeOfDay: // old TimeOfDayCode:
	{
	  unk = ! ValidatePayload(OP_TimeOfDay, timeOfDayStruct);
	  
	  emit timeOfDay((const timeOfDayStruct*)data, len, dir);
	  
	  break;
	}
	
      case WearChangeCode:
        {
	  unk = ! ValidatePayload(WearChangeCode, wearChangeStruct);
	  
	  emit spawnWearingUpdate ((const wearChangeStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_Action:
	{
	  unk = ! ValidatePayload(OP_Action, actionStruct);
	  
	  emit action((const actionStruct*)data, len, dir);
	  
	  break;
	}
	
      case OP_CastBuff: // old ActionCode:
        {
	  unk = false;
	  
	  emit action2Message ((const action2Struct *)data, len, dir);
	  
	  break;
        }
	
      case OP_Stamina: /// old StaminaCode:
        {
	  unk = ! ValidatePayload(OP_Stamina, staminaStruct);
	  
	  emit updateStamina((const staminaStruct *)data, len, dir);
	  
	  break;
        }
	
      case OP_GroundSpawn: // old MakeDropCode:
        {
#ifdef PACKET_PAYLOAD_SIZE_DIAG
	  if ((len != sizeof(makeDropStruct)) &&
	      (len != 0))
	    {
	      fprintf(stderr, "WARNING: OP_GroundSpawn (%04x) (dataLen: %d != sizeof(makeDropStruct):%d or 0)\n",
		      OP_GroundSpawn, len, 
		      sizeof(makeDropStruct));
	      unk = true;
	    }
	  else
	    unk = false;
#else
	  unk = false;
#endif
	  
	  emit newGroundItem((const makeDropStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_ClickObject: // Old RemDropCode:
        {
	  unk = ! ValidatePayload(OP_ClickObject, remDropStruct);
	  
	  emit removeGroundItem((const remDropStruct *)data, len, dir);
	  
	  break;
        }
	
      case OP_ShopRequest: // old OpenVendorCode:
        {
	  unk = false;
	  
	  emit openVendor(data, len, dir);
	  
	  break;
        }
	
      case OP_ShopEnd: // old CloseVendorCode:
        {
	  unk = false;
	  
	  emit closeVendor(data, len, dir);
	  
	  break;
        }
	
      case OP_GMTraining: // old OpenGMCode:
        {
	  unk = false;
	  
	  emit openGM(data, len, dir);
	  
	  break;
        }
	
      case OP_GMEndTrainingResponse: // old CloseGMCode:
        {
	  unk = false;
	  
	  emit closeGM(data, len, dir);
	  
	  break;
        }
	
      case OP_Consider: // old ConsiderCode:
        {
	  unk = false;
	  
	  ValidatePayload(OP_Consider, considerStruct);
	  
	  emit consMessage((const considerStruct*)data, len, dir);
	  
	  break;
        }
	
      case OP_TargetMouse: // old ClientTargetCode:
        {
	  unk = ! ValidatePayload(OP_TargetMouse, clientTargetStruct);
	  
	  emit clientTarget((const clientTargetStruct*) data, len, dir);
	  
	  break;
        }
	
      case OP_SpawnDoor: // old DoorSpawnsCode:
        {
	  unk = false;
	  
#ifdef PACKET_PAYLOAD_SIZE_DIAG
	  // verify size
	  
	  if (len % sizeof(doorStruct) != 0)
          {
	    printf("WARNING: OP_SpawnDoor (%.04x) (dataLen:%d "
		   "%% sizeof(doorStruct):%d) != 0!\n", 
		   OP_SpawnDoor, len, sizeof(doorStruct));
	    
	    unk = true;
	    break;
            }
#endif
	  int nDoors = len / sizeof(doorStruct);
	  const DoorSpawnsStruct *doorsStruct = (const DoorSpawnsStruct *)data;
	  for (int i = 0; i < nDoors; i++) {
	    emit newDoorSpawn(&doorsStruct->doors[i], len, dir);
	  }
	  
	  emit newDoorSpawns(doorsStruct, len, dir);
	  
	  break;
        }

      case OP_Buff: // old BuffDropCode: 
	{
	  unk = ! ValidatePayload(OP_Buff, buffStruct);
	  
	  emit buff((const buffStruct*)data, len, dir);
	  
	  // this is the server 'buff fading' AND the client 'cancel buff'
	  break;
	}
	
      case OP_Logout: // no contents
	{
	  unk = false;
	  
	  emit logOut(data, len, dir);
	  
	  break;
	}
	
      case OP_SendZonePoints:
	{
	  unk = false;
#ifdef PACKET_PAYLOAD_SIZE_DIAG
	  const zonePointsStruct* zp = (const zonePointsStruct*)data;
	  // verify size
	  if (((len - sizeof(zp->count) - sizeof(zp->unknown0xxx)) 
	       % sizeof(zonePointStruct)) != 0)
	  {
	    fprintf(stderr, "WARNING: OP_SendZonePoints (%04x) (dataLen: %d %% sizeof(zonePointStruct):%d) != 0!\n",
		    OP_SendZonePoints, len, sizeof(zonePointStruct));
	    unk = true;
	    break;
	  }
#endif

	  emit zonePoints((const zonePointsStruct*)data, len, dir);

	  break;
	}
	
      case OP_GuildMemberList: // old GuildMemberListCode
	{
	  unk = false;
	  break;
	}
	
      case OP_GuildMemberUpdate: // old GuildMemberUpdateCode:
	{
	  unk = false;
	  break;
	}
	
      case OP_SetRunMode: // old cRunToggleCode:
	{
	  //unk = ! ValidatePayload(cRunToggleCode, cRunToggleStruct);
	  //emit cRunToggle((const cRunToggleStruct*)data, len, dir);
	  unk = false;
	  break;
	}
	
      case OP_Jump: // old cJumpCode:
	{
	  //no data
	  unk = false;
	  break;
	}
	
      case OP_Camp: // old cStartCampingCode:
	{
	  //no data
	  unk = false;
	  break;
	}
	
      case OP_SenseHeading: // old cSenseHeadingCode:
	{
	  //no data
	  unk = false;
	  break;
	}
	
      case OP_Forage: // old ForageCode:
	{
	  //no data
	  unk = false;
	  break;
	}

#if 0 // ZBTEMP	: If we don't bother setting unk, don't bother processing
      case OP_ConsiderCorpse: //unknown contents // old cConCorpseCode:  
	{
	  //unk = ! ValidatePayload(cConCorpseCode, cConCorpseStruct);
	  //emit cConCorpse((const cConCorpseStruct*)data, len, dir);
	  break;
	}
	
      case OP_LootRequest:  //unknown contents - old cLootCorpseCode
	{
	  //unk = ! ValidatePayload(cLootCorpseCode, cLootCorpseStruct);
	  //emit cLootCorpse((const cLootCorpseStruct*)data, len, dir);
	  break;
	}
	
      case OP_EndLootRequest:  //unknown contents - old cDoneLootingCode
	{
	  //unk = ! ValidatePayload(cDoneLootingCode, cDoneLootingStruct);
	  //emit cDoneLooting((const cDoneLootingStruct*)data, len, dir);
	  break;
	}
	
      case OP_LootComplete:  //unknown contents - old sDoneLootingCode
	{
	  //unk = ! ValidatePayload(sDoneLootingCode, sDoneLootingStruct);
	  //emit sDoneLooting((const sDoneLootingStruct*)data, len, dir);
	  break;
	}
	
      case OP_WhoAllRequest:  //unknown contents - old WhoAllReqCode
	{
	  //unk = ! ValidatePayload(cWhoAllCode, cWhoAllStruct);
	  //emit cWhoAll((const cWhoAllStruct*)data, len, dir);
	  break;
	}
	
      case OP_WhoAllResponse: // old sWhoAllOutputCode: unknown contents
	{
	  //unk = ! ValidatePayload(sWhoAllOutputCode, sWhoAllOutputStruct);
	  //emit sWhoAllOutput((const sWhoAllOutputStruct*)data, len, dir);
	  break;
	}
      
      case OP_ShopPlayerBuy:  //unknown contents - old BuyItemCode
	{
	  //unk = ! ValidatePayload(xBuyItemCode, xBuyItemStruct);
	  //emit xBuyItem((const xBuyItemStruct*)data, len, dir);
	  //both the client command and the server acknowledgement when buying
	  break;
	}
#endif // ZBTEMP

#if 0 // ZBTEMP: OPCode Graveyard
      case CastOnCode:
        {
	  unk = false;
	  
	  emit castOn((castOnStruct*)data, len, dir);
	  
	  break;
        }
	
      case ManaDecrementCode:
        {
	  unk = ! ValidatePayload(ManaDecrementCode, manaDecrementStruct);
	  
	  emit manaChange((struct manaDecrementStruct *)data, len, dir);
	  
	  break;
        }
	
      case BadCastCode:
        {
	  unk = false; //! ValidatePayload(BadCastCode, badCastStruct);
	  
	  emit interruptSpellCast((const badCastStruct*)data, len, dir);
	  
	  break;
        }
	
      case SysMsgCode:
        {
	  unk = false;
	  
	  emit systemMessage((const sysMsgStruct*)data, len, dir);
	  
	  break;
        }
	
      case AltExpUpdateCode:
        {
	  unk = ! ValidatePayload(AltExpUpdateCode, altExpUpdateStruct);
	  
	  emit updateAltExp((const altExpUpdateStruct*)data, len, dir);
	  
	  break;
        }
	
      case Attack2Code:
        {
	  unk = false;
	  
	  emit attack2Hand1 ((const attack2Struct *)data, len, dir);
	  
	  break;
        }
	
      case NewGuildInZoneCode:
        {
	  unk = false;
	  
	  break;
        }
	
      case MoneyUpdateCode:
        {  
	  unk = false;
	  
	  emit moneyUpdate((const moneyUpdateStruct*)data, len, dir);
	  
	  break;
        }
	
      case MoneyThingCode:
        {
            unk = false;
	    
	    emit moneyThing((const moneyThingStruct*)data, len, dir);
	    
            break;
        }
	
      case BindWoundCode:
        {
	  unk = false;
	  
	  emit bindWound((bindWoundStruct*)data, len, dir);
	  
	  break;
        }
	
      case GroupInfoCode:
        {
	  // Too much still unknown.
	  
	  unk = ! ValidatePayload(GroupInfoCode, groupMemberStruct);
	  
	  emit groupInfo((const groupMemberStruct*)data, len, dir);
	  
	  break;
        }
	
      case GroupInviteCode:
        {
	  unk = ! ValidatePayload(GroupInviteCode, groupInviteStruct);
	  
	  emit groupInvite((const groupInviteStruct*)data, len, dir);
	  
	  break;
        }
	
      case GroupDeclineCode:
        {
	  unk = ! ValidatePayload(GroupDeclineCode, groupDeclineStruct);
	  
	  emit groupDecline((const groupDeclineStruct*)data, len, dir);
	  
	  break;
        }
	
      case GroupAcceptCode:
        {
	  unk = ! ValidatePayload(GroupAcceptCode, groupAcceptStruct);
	  
	  emit groupAccept((const groupAcceptStruct*)data, len, dir);
	  
	  break;
        }
	
      case GroupDeleteCode:
        {
	  unk = ! ValidatePayload(GroupDeleteCode, groupDeleteStruct);
	  
	  emit groupDelete((const groupDeleteStruct*)data, len, dir);
	  
	  break;
        }
	
      case CharUpdateCode:
        {
	  break;
        }
	
      case cChatFiltersCode:
	{
	  //unk = ! ValidatePayload(cChatFiltersCode, cChatFiltersStruct);
	  //emit cChatFilters((const cChatFiltersStruct*)data, len, dir);
	  unk = false;
	  break;
	}
	
      case cOpenSpellBookCode:
	{
	  //unk = ! ValidatePayload(cOpenSpellBookCode, cOpenSpellBookStruct);
	  //emit cOpenSpellBook((const cOpenSpellBookStruct*)data, len, dir);
	  unk = false;
	  break;
	}
	
      case OP_SwapSpell: // old TradeSpellBookSlotsCode:
	{
	  unk = ! ValidatePayload(OP_SwapSpell, tradeSpellBookSlotsStruct);
	  emit tradeSpellBookSlots((const tradeSpellBookSlotsStruct*)data, len, dir);
	  break;
	}
	
      case sSpellFizzleRegainCode:  //unknown contents, also comes when you Forage
	{
	  //unk = ! ValidatePayload(sSpellFizzleRegainCode, sSpellFizzleRegainStruct);
	  //emit sSpellFizzleRegain((const sSpellFizzleRegainStruct*)data, len, dir);
	  break;
	}
	
      case sSpellInterruptedCode:  //unknown contents
	{
	  //unk = ! ValidatePayload(sSpellInterruptedCode, sSpellInterruptedStruct);
	  //emit sSpellInterrupted((const sSpellInterruptedStruct*)data, len, dir);
	  break;
	}
	
      case cHideCode:
	{
	  //no data
	  unk = false;
	  break;
	}
	
      case cSneakCode:
	{
	  //no data
	  unk = false;
	  break;
	}
	
      case cTrackCode:
	{
	  //no data
	  unk = false;
	  break;
	}
#endif // ZBTEMP // Currently dead opcodes
	
      default:
        {
        }
      } /* end switch(opCode) */

    emit decodedZonePacket(data, len, dir, opCode, unk);
}

///////////////////////////////////////////
// Returns the current playback speed
int EQPacket::playbackSpeed(void)
{
  if (m_vPacket)
    return m_vPacket->playbackSpeed();

  return 1; // if not a vpacket stream, then is realtime
}

///////////////////////////////////////////
// Set the packet playback speed
void EQPacket::setPlayback(int speed)
{
  if (m_vPacket)
  {
    m_vPacket->setPlaybackSpeed(speed);
    
    QString string("");
    
    if (speed == 0)
      string.sprintf("Playback speed set Fast as possible");

    else if (speed < 0)
       string.sprintf("Playback paused (']' to resume)");

    else
       string.sprintf("Playback speed set to %d", speed);

    emit stsMessage(string, 5000);

    emit resetPacket(m_client2WorldStream->packetCount(), client2world);
    emit resetPacket(m_world2ClientStream->packetCount(), world2client);
    emit resetPacket(m_client2ZoneStream->packetCount(), client2zone);
    emit resetPacket(m_zone2ClientStream->packetCount(), zone2client);

    emit playbackSpeedChanged(speed);
  }
}

///////////////////////////////////////////
// Increment the packet playback speed
void EQPacket::incPlayback(void)
{
  if (m_vPacket)
  {
    int x = m_vPacket->playbackSpeed();
    
    switch(x)
      {
	// if we were paused go to 1X not full speed
      case -1:
	x = 1;
	break;
	
	// can't go faster than full speed
      case 0:
	return;
	
      case 9:
	x = 0;
	break;
	
      default:
	x += 1;
	break;
      }
    
    setPlayback(x);
  }
}

///////////////////////////////////////////
// Decrement the packet playback speed
void EQPacket::decPlayback(void)
{
  if (m_vPacket)
  {
    int x = m_vPacket->playbackSpeed();
    switch(x)
      {
	// paused
      case -1:
	return;
	break;
	
	// slower than 1 is paused
      case 1:
	x = -1;
	break;
	
	// if we were full speed goto 9
      case 0:
	x = 9;
	break;
	
      default:
	x -= 1;
	break;
      }
    
    setPlayback(x);
  }
}

///////////////////////////////////////////
// Set the IP address of the client to monitor
void EQPacket::monitorIPClient(const QString& ip)
{
  showeq_params->ip = ip;
  struct in_addr  ia;
  inet_aton (showeq_params->ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);
  
  resetEQPacket();
  
  printf("Listening for IP client: %s\n", (const char*)showeq_params->ip);
  if (!showeq_params->playbackpackets)
  {
    m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
			       showeq_params->realtime, 
			       IP_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }
}

///////////////////////////////////////////
// Set the MAC address of the client to monitor
void EQPacket::monitorMACClient(const QString& mac)
{
  showeq_params->mac_address = mac;
  m_detectingClient = true;
  struct in_addr  ia;
  inet_aton (AUTOMATIC_CLIENT_IP, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  printf("Listening for MAC client: %s\n", 
	 (const char*)showeq_params->mac_address);

  if (!showeq_params->playbackpackets)
  {
    m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
			       showeq_params->realtime, 
			       IP_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }
}

///////////////////////////////////////////
// Monitor the next client seen
void EQPacket::monitorNextClient()
{
  m_detectingClient = true;
  showeq_params->ip = AUTOMATIC_CLIENT_IP;
  struct in_addr  ia;
  inet_aton (showeq_params->ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  printf("Listening for next client seen. (you must zone for this to work!)\n");

  if (!showeq_params->playbackpackets)
  {
    m_packetCapture->setFilter(showeq_params->device, NULL,
			       showeq_params->realtime, 
			       DEFAULT_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }
}

///////////////////////////////////////////
// Monitor for packets on the specified device
void EQPacket::monitorDevice(const QString& dev)
{
  // set the device to use
  showeq_params->device = dev;

  // make sure we aren't playing back packets
  if (showeq_params->playbackpackets)
    return;

  // stop the current packet capture
  m_packetCapture->stop();

  // setup for capture on new device
  if (!showeq_params->ip.isEmpty())
  {
    struct hostent *he;
    struct in_addr  ia;

    /* Substitute "special" IP which is interpreted 
       to set up a different filter for picking up new sessions */
    
    if (showeq_params->ip == "auto")
      inet_aton (AUTOMATIC_CLIENT_IP, &ia);
    else if (inet_aton (showeq_params->ip, &ia) == 0)
    {
      he = gethostbyname(showeq_params->ip);
      if (!he)
      {
	printf ("Invalid address; %s\n", (const char*)showeq_params->ip);
	exit (0);
      }
      memcpy (&ia, he->h_addr_list[0], he->h_length);
    }
    m_client_addr = ia.s_addr;
    showeq_params->ip = inet_ntoa(ia);
    
    if (showeq_params->ip ==  AUTOMATIC_CLIENT_IP)
    {
      m_detectingClient = true;
      printf("Listening for first client seen.\n");
    }
    else
    {
      m_detectingClient = false;
      printf("Listening for client: %s\n",
	     (const char*)showeq_params->ip);
    }
  }
 
  resetEQPacket();

  // restart packet capture
  if (showeq_params->mac_address.length() == 17)
    m_packetCapture->start(showeq_params->device, 
			   showeq_params->mac_address, 
			   showeq_params->realtime, MAC_ADDRESS_TYPE );
  else
    m_packetCapture->start(showeq_params->device, showeq_params->ip, 
			   showeq_params->realtime, IP_ADDRESS_TYPE );
  emit filterChanged();
}

///////////////////////////////////////////
// Set the session tracking state
void EQPacket::session_tracking()
{
  m_client2WorldStream->setSessionTracking(showeq_params->session_tracking);
  m_world2ClientStream->setSessionTracking(showeq_params->session_tracking);
  m_client2ZoneStream->setSessionTracking(showeq_params->session_tracking);
  m_zone2ClientStream->setSessionTracking(showeq_params->session_tracking);
  emit sessionTrackingChanged(showeq_params->session_tracking);

}

///////////////////////////////////////////
// Set the current ArqSeqGiveUp
void EQPacket::setArqSeqGiveUp(int giveUp)
{
  // a sanity check, if the user set it to below 32, they're prolly nuts
  if (giveUp >= 32)
    showeq_params->arqSeqGiveUp = giveUp;

  // a sanity check, if the user set it to below 32, they're prolly nuts
  if (showeq_params->arqSeqGiveUp >= 32)
    giveUp = showeq_params->arqSeqGiveUp;
  else
    giveUp = 32;

  m_client2WorldStream->setArqSeqGiveUp(giveUp);
  m_world2ClientStream->setArqSeqGiveUp(giveUp);
  m_client2ZoneStream->setArqSeqGiveUp(giveUp);
  m_zone2ClientStream->setArqSeqGiveUp(giveUp);
}

///////////////////////////////////////////
// Reset EQPacket's state
void EQPacket::resetEQPacket()
{
  m_client2WorldStream->reset();
  m_client2WorldStream->setSessionTracking(showeq_params->session_tracking);
  m_world2ClientStream->reset();
  m_world2ClientStream->setSessionTracking(showeq_params->session_tracking);
  m_client2ZoneStream->reset();
  m_client2ZoneStream->setSessionTracking(showeq_params->session_tracking);
  m_zone2ClientStream->reset();
  m_zone2ClientStream->setSessionTracking(showeq_params->session_tracking);

  m_clientPort = 0;
  m_serverPort = 0;
  
  emit clientPortLatched(m_clientPort);
}

///////////////////////////////////////////
// Return the current pcap filter
const QString EQPacket::pcapFilter()
{
  // make sure we aren't playing back packets
  if (showeq_params->playbackpackets)
    return QString("Playback");

  return m_packetCapture->getFilter();
}


int EQPacket::packetCount(int stream)
{
  return m_streams[stream]->packetCount();
}

uint8_t EQPacket::session_tracking_enabled(void)
{
  return m_zone2ClientStream->sessionTracking();
}

size_t EQPacket::currentCacheSize(int stream)
{
  return m_streams[stream]->currentCacheSize();
}

uint16_t EQPacket::serverSeqExp(int stream)
{
  return m_streams[stream]->arqSeqExp();
}
