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

#ifdef __FreeBSD__
#include "packet.h"
#endif
#include <netinet/if_ether.h>

#include <qtimer.h>

#include "everquest.h"
#include "packet.h"
#include "packetcapture.h"
#include "packetformat.h"
#include "packetstream.h"
#include "packetinfo.h"
#include "vpacket.h"
#include "everquest.h"
#include "diagnosticmessages.h"

//----------------------------------------------------------------------
// Macros

//#define DEBUG_PACKET
//#undef DEBUG_PACKET

// The following defines are used to diagnose packet handling behavior
// this define is used to diagnose packet processing (in dispatchPacket mostly)
//#define PACKET_PROCESS_DIAG 3 // verbosity level 0-n

// this define is used to diagnose cache handling (in dispatchPacket mostly)
//#define PACKET_CACHE_DIAG 3 // verbosity level (0-n)

// diagnose opcode DB issues
//#define  PACKET_OPCODEDB_DIAG 1

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


//----------------------------------------------------------------------
// EQPacket class methods

/* EQPacket Class - Sets up packet capturing */

////////////////////////////////////////////////////
// Constructor
EQPacket::EQPacket(const QString& worldopcodesxml,
		   const QString& zoneopcodesxml,
		   uint16_t arqSeqGiveUp, 
		   QString device,
		   QString ip,
		   QString mac_address,
		   bool realtime,
		   bool session_tracking,
		   bool recordPackets,
		   bool playbackPackets,
		   int8_t playbackSpeed, 
		   QObject * parent, const char *name)
  : QObject (parent, name),
    m_packetCapture(NULL),
    m_vPacket(NULL),
    m_timer(NULL),
    m_busy_decoding(false),
    m_arqSeqGiveUp(arqSeqGiveUp),
    m_device(device),
    m_ip(ip),
    m_mac(mac_address),
    m_realtime(realtime),
    m_session_tracking(session_tracking),
    m_recordPackets(recordPackets),
    m_playbackPackets(playbackPackets),
    m_playbackSpeed(playbackSpeed)
{
  // create the packet type db
  m_packetTypeDB = new EQPacketTypeDB();

#ifdef PACKET_OPCODEDB_DIAG
  m_packetTypeDB->list();
#endif

  // create the world opcode db (with hash size of 29)
  m_worldOPCodeDB = new EQPacketOPCodeDB(29);

  // load the world opcode db
  if (!m_worldOPCodeDB->load(*m_packetTypeDB, worldopcodesxml))
    seqFatal("Error loading '%s'!", (const char*)worldopcodesxml);
  
#ifdef PACKET_OPCODEDB_DIAG
  m_worldOPCodeDB->list();
#endif

  //m_worldOPCodeDB->save("/tmp/worldopcodes.xml");

  // create the zone opcode db (with hash size of 211)
  m_zoneOPCodeDB = new EQPacketOPCodeDB(211);
  
  // load the zone opcode db
  if (!m_zoneOPCodeDB->load(*m_packetTypeDB, zoneopcodesxml))
    seqFatal("Error loading '%s'!", (const char*)zoneopcodesxml);

#ifdef PACKET_OPCODEDB_DIAG
  m_zoneOPCodeDB->list();
#endif

  //m_zoneOPCodeDB->save("/tmp/zoneopcodes.xml");
  
  // Setup the data streams

  // Setup client -> world stream
  m_client2WorldStream = new EQPacketStream(client2world, DIR_Client, 
					    m_arqSeqGiveUp, *m_worldOPCodeDB,
					    this, "client2world");
  connect(m_client2WorldStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2WorldStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)),
	  this,
	  SIGNAL(decodedWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)));
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
  m_world2ClientStream = new EQPacketStream(world2client, DIR_Server,
					    m_arqSeqGiveUp, *m_worldOPCodeDB,
					    this, "world2client");
  connect(m_world2ClientStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_world2ClientStream, 

 	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)),
	  this,
	  SIGNAL(decodedWorldPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)));
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
  m_client2ZoneStream = new EQPacketStream(client2zone, DIR_Client,
					  m_arqSeqGiveUp, *m_zoneOPCodeDB,
					  this, "client2zone");
  connect(m_client2ZoneStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_client2ZoneStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)),
	  this,
	  SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)));
  connect(m_client2ZoneStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*,bool)),
	  this,
	  SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*,bool)));
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
  m_zone2ClientStream = new EQPacketStream(zone2client, DIR_Server,
					   m_arqSeqGiveUp, *m_zoneOPCodeDB,
					   this, "zone2client");
  connect(m_zone2ClientStream, 
	  SIGNAL(rawPacket(const uint8_t*, size_t, uint8_t, uint16_t)),
	  this,
	  SIGNAL(rawZonePacket(const uint8_t*, size_t, uint8_t, uint16_t)));
  connect(m_zone2ClientStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)),
	  this,
	  SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*)));
  connect(m_zone2ClientStream, 
	  SIGNAL(decodedPacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*, bool)),
	  this,
	  SIGNAL(decodedZonePacket(const uint8_t*, size_t, uint8_t, uint16_t, const EQPacketOPCode*, bool)));
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
  if (m_ip.isEmpty() && m_mac.isEmpty())
    seqFatal("No address specified");
  
  if (!m_ip.isEmpty())
  {
    /* Substitute "special" IP which is interpreted 
       to set up a different filter for picking up new sessions */
    
    if (m_ip == "auto")
      inet_aton (AUTOMATIC_CLIENT_IP, &ia);
    else if (inet_aton (m_ip, &ia) == 0)
    {
      he = gethostbyname(m_ip);
      if (!he)
	seqFatal("Invalid address; %s", (const char*)m_ip);

      memcpy (&ia, he->h_addr_list[0], he->h_length);
    }
    m_client_addr = ia.s_addr;
    m_ip = inet_ntoa(ia);
    
    if (m_ip ==  AUTOMATIC_CLIENT_IP)
    {
      m_detectingClient = true;
      seqInfo("Listening for first client seen.");
    }
    else
    {
      m_detectingClient = false;
      seqInfo("Listening for client: %s",
	      (const char*)m_ip);
    }
  }
  
  if (!m_playbackPackets)
  {
    // create the pcap object and initialize, either with MAC or IP
    m_packetCapture = new PacketCaptureThread();
    if (m_mac.length() == 17)
      m_packetCapture->start(m_device, 
			     m_mac, 
			     m_realtime, MAC_ADDRESS_TYPE );
    else
      m_packetCapture->start(m_device,
			     m_ip, 
			     m_realtime, IP_ADDRESS_TYPE );
    emit filterChanged();
  }

  // if running setuid root, then give up root access, since the PacketCapture
  // is the only thing that needed it.
  if ((geteuid() == 0) && (getuid() != geteuid()))
    setuid(getuid()); 


  /* Create timer object */
  m_timer = new QTimer (this);
  
  if (!m_playbackPackets)
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
    
    if (m_recordPackets)
    {
      m_vPacket = new VPacket(filename, 1, true);
      // Must appear befire next call to getPrefString, which uses a static string
      seqInfo("Recording packets to '%s' for future playback", filename);
      
      if (pSEQPrefs->getPrefString("FlushPackets", section))
	m_vPacket->setFlushPacket(true);
    }
    else if (m_playbackPackets)
    {
      m_vPacket = new VPacket(filename, 1, false);
      m_vPacket->setCompressTime(pSEQPrefs->getPrefInt("CompressTime", section, 0));
      m_vPacket->setPlaybackSpeed(m_playbackSpeed);
      
      seqInfo("Playing back packets from '%s' at speed '%d'", filename,
	     
	     m_playbackSpeed);
    }
  }
  else
  {
    m_recordPackets = 0;
    m_playbackPackets = 0;
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
    if (m_recordPackets)
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
	seqWarn("Error:  The version of the packet stream has " \
		 "changed since '%s' was recorded - disabling playback",
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
    seqInfo("End of playback file '%s' reached."
	    "Playback Finished!",
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

  if (((packet.getDestPort() >= LoginServerMinPort) &&
       (packet.getDestPort() <= LoginServerMaxPort)) ||
      (packet.getSourcePort() >= LoginServerMinPort) &&
      (packet.getSourcePort() <= LoginServerMaxPort))
    return;

  /* discard pure ack/req packets and non valid flags*/
  if (packet.flagsHi() < 0x02 || packet.flagsHi() > 0x46 || size < 10)
  {
#if defined(PACKET_PROCESS_DIAG)
    seqDebug("discarding packet %s:%d ==>%s:%d flagsHi=%d size=%d",
	   (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
	   (const char*)packet.getIPv4DestA(), packet.getDestPort(),
	   packet.flagsHi(), size);
    seqDebug("%s", (const char*)packet.headerFlags(false));
#endif
    return;    
  }

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
  seqDebug("%s", (const char*)packet.headerFlags((PACKET_PROCESS_DIAG < 3)));
  uint32_t crc = packet.calcCRC32();
  if (crc != packet.crc32())
    seqWarn("CRC: Warning Packet seq = %d CRC (%08x) != calculated CRC (%08x)!",
	   packet.seq(), packet.crc32(), crc);
#endif
  
  if (!packet.isValid())
  {
    seqWarn("INVALID PACKET: Bad CRC32 [%s:%d -> %s:%d] seq %04x len %d crc32 (%08x != %08x)",
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
    m_ip = packet.getIPv4DestA();
    m_client_addr = packet.getIPv4DestN();
    m_detectingClient = false;
    emit clientChanged(m_client_addr);
    seqInfo("Client Detected: %s", (const char*)m_ip);
  }
  else if (m_detectingClient && packet.getDestPort() == WorldServerGeneralPort)
  {
    m_ip = packet.getIPv4SourceA();
    m_client_addr = packet.getIPv4SourceN();
    m_detectingClient = false;
    emit clientChanged(m_client_addr);
    seqInfo("Client Detected: %s", (const char*)m_ip);
  }
  /* end client detection */



  if (packet.getSourcePort() == WorldServerChatPort)
  {
    dispatchWorldChatData(packet.payloadLength(), packet.payload(), DIR_Server);
    
    return;
  }
  else if (packet.getDestPort() == WorldServerChatPort)
  {
    dispatchWorldChatData(packet.payloadLength(), packet.payload(), DIR_Client);
    
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
  if(!m_playbackPackets) 
  {
    m_packetCapture->setFilter(m_device, m_ip,
			       m_realtime, IP_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }

  seqInfo("EQPacket: SEQClosing detected, awaiting next zone session,  pcap filter: EQ Client %s",
	  (const char*)m_ip);
  
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

  if (!m_playbackPackets)
  {
    if (m_mac.length() == 17)
    {
      m_packetCapture->setFilter(m_device, 
				 m_mac,
				 m_realtime, 
				 MAC_ADDRESS_TYPE, 0, 
				 m_clientPort);
      emit filterChanged();
      seqInfo("EQPacket: SEQStart detected, pcap filter: EQ Client %s, Client port %d",
	      (const char*)m_mac, 
	      m_clientPort);
    }
    else
    {
      m_packetCapture->setFilter(m_device, 
				 m_ip,
				 m_realtime, 
				 IP_ADDRESS_TYPE, 0, 
				 m_clientPort);
      emit filterChanged();
      seqInfo("EQPacket: SEQStart detected, pcap filter: EQ Client %s, Client port %d",
	      (const char*)m_ip, 
	      m_clientPort);
    }
  }
  
  emit clientPortLatched(m_clientPort);
}

///////////////////////////////////////////
//EQPacket::dispatchWorldChatData  
// note this dispatch gets just the payload
void EQPacket::dispatchWorldChatData (size_t len, uint8_t *data, 
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
    seqDebug("%04x - %d (%s)", opCode, len,
	    ((dir == DIR_Server) ? 
	     "WorldChatServer --> Client" : "Client --> WorldChatServer"));
  }
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
  m_ip = ip;
  struct in_addr  ia;
  inet_aton (m_ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);
  
  resetEQPacket();
  
  seqInfo("Listening for IP client: %s", (const char*)m_ip);
  if (!m_playbackPackets)
  {
    m_packetCapture->setFilter(m_device, m_ip,
			       m_realtime, 
			       IP_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }
}

///////////////////////////////////////////
// Set the MAC address of the client to monitor
void EQPacket::monitorMACClient(const QString& mac)
{
  m_mac = mac;
  m_detectingClient = true;
  struct in_addr  ia;
  inet_aton (AUTOMATIC_CLIENT_IP, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  seqInfo("Listening for MAC client: %s", 
	 (const char*)m_mac);

  if (!m_playbackPackets)
  {
    m_packetCapture->setFilter(m_device, m_ip,
			       m_realtime, 
			       IP_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }
}

///////////////////////////////////////////
// Monitor the next client seen
void EQPacket::monitorNextClient()
{
  m_detectingClient = true;
  m_ip = AUTOMATIC_CLIENT_IP;
  struct in_addr  ia;
  inet_aton (m_ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  seqInfo("Listening for next client seen. (you must zone for this to work!)");

  if (!m_playbackPackets)
  {
    m_packetCapture->setFilter(m_device, NULL,
			       m_realtime, 
			       DEFAULT_ADDRESS_TYPE, 0, 0);
    emit filterChanged();
  }
}

///////////////////////////////////////////
// Monitor for packets on the specified device
void EQPacket::monitorDevice(const QString& dev)
{
  // set the device to use
  m_device = dev;

  // make sure we aren't playing back packets
  if (m_playbackPackets)
    return;

  // stop the current packet capture
  m_packetCapture->stop();

  // setup for capture on new device
  if (!m_ip.isEmpty())
  {
    struct hostent *he;
    struct in_addr  ia;

    /* Substitute "special" IP which is interpreted 
       to set up a different filter for picking up new sessions */
    
    if (m_ip == "auto")
      inet_aton (AUTOMATIC_CLIENT_IP, &ia);
    else if (inet_aton (m_ip, &ia) == 0)
    {
      he = gethostbyname(m_ip);
      if (!he)
	seqFatal("Invalid address; %s", (const char*)m_ip);

      memcpy (&ia, he->h_addr_list[0], he->h_length);
    }
    m_client_addr = ia.s_addr;
    m_ip = inet_ntoa(ia);
    
    if (m_ip ==  AUTOMATIC_CLIENT_IP)
    {
      m_detectingClient = true;
      seqInfo("Listening for first client seen.");
    }
    else
    {
      m_detectingClient = false;
      seqInfo("Listening for client: %s",
	     (const char*)m_ip);
    }
  }
 
  resetEQPacket();

  // restart packet capture
  if (m_mac.length() == 17)
    m_packetCapture->start(m_device, 
			   m_mac, 
			   m_realtime, MAC_ADDRESS_TYPE );
  else
    m_packetCapture->start(m_device, m_ip, 
			   m_realtime, IP_ADDRESS_TYPE );
  emit filterChanged();
}

///////////////////////////////////////////
// Set the session tracking state
void EQPacket::session_tracking(bool enable)
{
  m_session_tracking = enable;
  m_client2WorldStream->setSessionTracking(m_session_tracking);
  m_world2ClientStream->setSessionTracking(m_session_tracking);
  m_client2ZoneStream->setSessionTracking(m_session_tracking);
  m_zone2ClientStream->setSessionTracking(m_session_tracking);
  emit sessionTrackingChanged(m_session_tracking);

}

///////////////////////////////////////////
// Set the current ArqSeqGiveUp
void EQPacket::setArqSeqGiveUp(uint16_t giveUp)
{
  // a sanity check, if the user set it to below 32, they're prolly nuts
  if (giveUp >= 32)
    m_arqSeqGiveUp = giveUp;

  // a sanity check, if the user set it to below 32, they're prolly nuts
  if (m_arqSeqGiveUp >= 32)
    giveUp = m_arqSeqGiveUp;
  else
    giveUp = 32;

  m_client2WorldStream->setArqSeqGiveUp(giveUp);
  m_world2ClientStream->setArqSeqGiveUp(giveUp);
  m_client2ZoneStream->setArqSeqGiveUp(giveUp);
  m_zone2ClientStream->setArqSeqGiveUp(giveUp);
}

void EQPacket::setRealtime(bool val)
{
  m_realtime = val;
}

bool EQPacket::connect2(const QString& opcodeName, EQStreamPairs sp,
		 uint8_t dir, const char* payload,  EQSizeCheckType szt, 
		 const QObject* receiver, const char* member)
{
  bool res = false;

  if (sp & SP_World)
  {
    if (dir & DIR_Client)
      res = m_client2WorldStream->connect2(opcodeName, payload, szt, receiver, member);
    if (dir & DIR_Server)
      res = m_world2ClientStream->connect2(opcodeName, payload, szt, receiver, member);
  }
  if (sp & SP_Zone)
  {
    if (dir & DIR_Client)
      res = m_client2ZoneStream->connect2(opcodeName, payload, szt, receiver, member);
    if (dir & DIR_Server)
      res = m_zone2ClientStream->connect2(opcodeName, payload, szt, receiver, member);
  }

  return res;
}

///////////////////////////////////////////
// Reset EQPacket's state
void EQPacket::resetEQPacket()
{
  m_client2WorldStream->reset();
  m_client2WorldStream->setSessionTracking(m_session_tracking);
  m_world2ClientStream->reset();
  m_world2ClientStream->setSessionTracking(m_session_tracking);
  m_client2ZoneStream->reset();
  m_client2ZoneStream->setSessionTracking(m_session_tracking);
  m_zone2ClientStream->reset();
  m_zone2ClientStream->setSessionTracking(m_session_tracking);

  m_clientPort = 0;
  m_serverPort = 0;
  
  emit clientPortLatched(m_clientPort);
}

///////////////////////////////////////////
// Return the current pcap filter
const QString EQPacket::pcapFilter()
{
  // make sure we aren't playing back packets
  if (m_playbackPackets)
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
