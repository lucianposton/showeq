/*
 * packet.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

/* Implementation of Packet class */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <netdb.h>
#include <qtimer.h>
#include <string.h>
#include <qtextstream.h>
#include "everquest.h"
#include "packet.h"
#include "util.h"
#include "main.h"
#include "vpacket.h"
#include "itemdb.h"

//----------------------------------------------------------------------
// Macros

//#define DEBUG_PACKET
//#undef DEBUG_PACKET

// The following defines are used to diagnose packet handling behavior
// this define is used to diagnose packet processing (in decodePacket mostly)
//#define PACKET_PROCESS_DIAG 3 // verbosity level 0-n

// this define is used to diagnose cache handling (in decodePacket mostly)
//#define PACKET_CACHE_DIAG 3 // verbosity level (0-n)

// define this to be pedantic about what packets are acceptable (no CRC errors)
#define PACKET_PEDANTIC 2

// diagnose structure size changes
#define PACKET_PAYLOAD_SIZE_DIAG 1

// Packet version is a unique number that should be bumped every time packet
// structure (ie. encryption) changes.  It is checked by the VPacket feature
// (currently the date of the last packet structure change)
#define PACKETVERSION  40100

//----------------------------------------------------------------------
// constants

// Arbitrary cutoff used to determine the relationship between
// an arq sequence may be from a wrap and not just be really old.
const int16_t arqSeqWrapCutoff = 1024;

// Arbitrary cutoff used to determine when to give up on seeing
// an arq sequenced packet, and just move on...
const int16_t arqSeqGiveUp = 256;

//----------------------------------------------------------------------
// static variables
#ifdef PACKET_CACHE_DIAG
static size_t maxServerCacheCount = 0;
#endif

//----------------------------------------------------------------------
// global variables

//----------------------------------------------------------------------
// forward declarations
char *getTime (char *pchTime);

//----------------------------------------------------------------------
// Here begins the code
void searchForNum(unsigned char *data, int len, char *Name, char value){
    for(int n = 0; n< len; n++)
        if (data[n] == value)
            printf("%s contains %i at %i\n", Name, value, n);
}

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

#define ValidateDecodedPayload(codeName, structName) \
  validatePayloadSize(decodedDataLen, sizeof( structName ), codeName, \
		      "Decoded ", #codeName , #structName )
#else
#define ValidatePayload(code, struct) true
#endif

//----------------------------------------------------------------------
// EQPacketFormatRaw class methods
QString EQPacketFormatRaw::headerFlags(const QString& prefix, 
				       bool brief) const
{
  QString tmp;
  if (!prefix.isEmpty())
  {
    if (brief)
      tmp = prefix + ": ";
    else
      tmp = prefix + " Hdr (" + QString::number(flagsHi(), 16) + ", "
	+ QString::number(flagsLo(), 16) + "): ";
  }

  if (isARQ())
    tmp += "ARQ, ";
  if (isClosingLo())
    tmp += "closingLo, ";
  if (isFragment())
    tmp += "Fragment, ";
  if (isASQ())
    tmp += "ASQ, ";
  if (isSEQStart())
    tmp += "SEQStart, ";
  if (isClosingHi())
    tmp += "closingHi, ";
  if (isSEQEnd())
    tmp += "SEQEnd, ";
  if (isARSP())
    tmp += "ARSP, ";
  if (isSpecARQ())
    tmp += "SpecARQ, ";

  tmp += QString("seq: ") + QString::number(seq(), 16) + ", ";

  if (!brief)
  {
    if (skip() != 0)
      tmp += QString("Skip: ") + QString::number(skip()) + ", ";

    if (isARQ())
      tmp += QString("arq: ") + QString::number(arq(), 16) + ", ";
    
    if (isFragment())
    {
      tmp += QString("FragSeq: ") + QString::number(fragSeq(), 16)
	+ ", FragCur: " + QString::number(fragCur(), 16)
	+ ", FragTot: " + QString::number(fragTot(), 16) + ", ";
    }
  }

  return tmp;
}

//----------------------------------------------------------------------
// EQPacketFormat class methods
EQPacketFormat::EQPacketFormat(EQPacketFormat& packet, bool copy)
{
  if (!copy)
  {
    // the data is someone elses memory
    m_ownCopy = false;

    // just copy all their values
    m_packet = packet.m_packet;
    m_length = packet.m_length;
    m_postSkipData = packet.m_postSkipData;
    m_postSkipDataLength = packet.m_postSkipDataLength;
    m_payload = packet.m_payload;
    m_payloadLength = packet.m_payloadLength;
    m_arq = packet.m_arq;
  }
  else
  {
    init(packet.m_packet, packet.m_length, copy);
  }
}

EQPacketFormat::~EQPacketFormat()
{
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;
}

EQPacketFormat& EQPacketFormat::operator=(const EQPacketFormat& packet)
{
  // if this was a deep copy, delete the existing data
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;

  // initialize as deep as this object previously was
  init(packet.m_packet, packet.m_length, m_ownCopy);

  return *this;
}

void EQPacketFormat::init(EQPacketFormatRaw* packet,
			  uint16_t length,
			  bool copy)
{
  if (!copy)
  {
    // the data is someone elses memory
    m_ownCopy = false;
    
    // just copy the pointer
    m_packet = packet;

    // save the length
    m_length = length;
  }
  else
  {
    // this is our own copy
    m_ownCopy = true;

    // allocate memory for the copy
    // NOTE: We should use an allocater for this instead of normal new
    m_packet = (EQPacketFormatRaw*)new uint8_t[length];

    // copy the data
    memcpy((void*)m_packet, (void*)packet, length);

    // save the length
    m_length = length;
  }

  // finish initialization
  init();
}

void EQPacketFormat::init()
{
  // calculate position of first byte past the skipable data
  m_postSkipData = ((uint8_t*)m_packet) + m_packet->skip() + 2;

  // calculate the length of the rest of the data
  m_postSkipDataLength = m_length - (m_postSkipData - ((uint8_t*)m_packet));

  // get the location of the payload
  m_payload = m_packet->payload();

  // calculate the lenght of the payload (length - diff - len(checkSum))
  m_payloadLength = m_length - (m_payload - ((uint8_t*)m_packet)) - 4; 
    

  // make a local copy of the arq to speed up comparisons
  m_arq = eqntohuint16(&m_postSkipData[2]);
}

QString EQPacketFormat::headerFlags(const QString& prefix, 
				    bool brief) const
{
  QString tmp;

  if (brief)
    tmp = prefix + ": ";
  else
    tmp = prefix + " Hdr (" + QString::number(flagsHi(), 16) + ", "
      + QString::number(flagsLo(), 16) + "): ";

  tmp += m_packet->headerFlags("", true);

  if (!brief)
  {
    if (skip() != 0)
      tmp += QString("Skip: ") + QString::number(skip()) + ", ";
    
    if (isARQ())
      tmp += QString("arq: ") + QString::number(arq(), 16) + ", ";
    
    if (isFragment())
    {
      tmp += QString("FragSeq: ") + QString::number(fragSeq(), 16)
	+ ", FragCur: " + QString::number(fragCur(), 16)
	+ ", FragTot: " + QString::number(fragTot(), 16) + ", ";
    }

    tmp += QString("Opcode: ") + QString::number(eqntohuint16(payload()), 16);
  }

  return tmp;
}

//----------------------------------------------------------------------
// EQUDPIPPacketFormat class methods
EQUDPIPPacketFormat::EQUDPIPPacketFormat(uint8_t* data, 
					 uint32_t length, 
					 bool copy = false)
{
  uint8_t* ipdata;
  if (copy)
  {
    // allocate our own copy
    ipdata = new uint8_t[length];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)data, length);
  }
  else // just use the data that was passed in
    ipdata = data;

  // note whether or not this object ownw the memory
  m_ownCopy = copy;

  // initialize the object
  init(ipdata);
}

EQUDPIPPacketFormat::EQUDPIPPacketFormat(EQUDPIPPacketFormat& packet,
					 bool copy = false)
{
  // note whether or not this object ownw the memory
  m_ownCopy = copy;

  if (copy)
  {
    // allocate our own copy
    uint8_t* ipdata = new uint8_t[packet.m_dataLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_dataLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_dataLength = packet.m_dataLength;
    m_ip = packet.m_ip;
    m_udp = packet.m_udp;
  }
}

EQUDPIPPacketFormat::~EQUDPIPPacketFormat()
{
  if (m_ownCopy && (m_ip != NULL))
    delete [] (uint8_t*)m_ip;
}

EQUDPIPPacketFormat& EQUDPIPPacketFormat::operator=(const EQUDPIPPacketFormat& packet)
{
  if (m_ownCopy && (m_ip != NULL))
    delete [] (uint8_t*)m_ip;

  if (m_ownCopy)
  {
    // allocate our own copy
    uint8_t* ipdata = new uint8_t[packet.m_dataLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_dataLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_dataLength = packet.m_dataLength;
    m_ip = packet.m_ip;
    m_udp = packet.m_udp;
  }

  return *this;
}

void EQUDPIPPacketFormat::init(uint8_t* data)
{
  uint32_t ipHeaderLength, length;

  // we start at the IP header
  m_ip = (struct ip*)data;

  // retrieve the total length from the header
  m_dataLength = ntohs (m_ip->ip_len);

  // use this length to caclulate the rest
  length = m_dataLength;

  // skip past the IP header
  ipHeaderLength  = m_ip->ip_hl * 4;
  length  -= ipHeaderLength;
  data += ipHeaderLength;

  // get the UDP header
  m_udp   = (struct udphdr *) data;

  // skip over UDP header
  length  -= sizeof  (struct udphdr);
  data += (sizeof (struct udphdr));

  // initialize underlying EQPacketFormat with the UDP payload
  EQPacketFormat::init((EQPacketFormatRaw*)data, length, false);
}

QString EQUDPIPPacketFormat::headerFlags(bool brief) const
{
  QString tmp;
  tmp.sprintf("[%s:%d -> %s:%d]", 
	      (const char*)getIPv4SourceA(), getSourcePort(),
	      (const char*)getIPv4DestA(), getDestPort());

  return EQPacketFormat::headerFlags(tmp, brief);
}

//----------------------------------------------------------------------
// EQPacket class methods

/* EQPacket Class - Sets up packet capturing */
EQPacket::EQPacket (EQPlayer* pplayer, QObject * parent, const char *name):
QObject (parent, name)
{
   struct hostent *he;
   struct in_addr  ia;

   m_player = pplayer;
   m_parent = parent;

   if (!showeq_params->ip && !showeq_params->mac_address)
   {
      printf ("No address specified\n");
      exit(0);
   }

   m_logger = new PktLogger(showeq_params->PktLoggerFilename,
                            showeq_params->PktLoggerMask);

   if (showeq_params->ip)
   {
      /* Substitute "special" IP which is interpreted 
         to set up a different filter for picking up new sessions */

      if (strcasecmp(showeq_params->ip, "auto") == 0)
	inet_aton (AUTOMATIC_CLIENT_IP, &ia);
      else if (inet_aton (showeq_params->ip, &ia) == 0)
      {
         he = gethostbyname(showeq_params->ip);
         if (!he)
         {
            printf ("Invalid address; %s\n", showeq_params->ip);
            exit (0);
         }
         memcpy (&ia, he->h_addr_list[0], he->h_length);
      }
      m_client_addr = ia.s_addr;
      free((void*)showeq_params->ip);
      showeq_params->ip = strdup(inet_ntoa(ia));

      if (!strcmp(showeq_params->ip, AUTOMATIC_CLIENT_IP))
      {
	m_detectingClient = true;
	printf("Listening for first client seen.\n");
      }
      else
      {
	m_detectingClient = false;
	printf("Listening for client: %s\n", showeq_params->ip);
      }
   }

   if (!showeq_params->playbackpackets)
   {
      // create the pcap object and initialize, either with MAC or IP
      m_packetCapture = new PacketCaptureThread();
      if (strlen(showeq_params->mac_address) == 17)
         m_packetCapture->start(showeq_params->device, 
				showeq_params->mac_address, 
				showeq_params->realtime, MAC_ADDRESS_TYPE );
      else
         m_packetCapture->start(showeq_params->device, showeq_params->ip, 
				showeq_params->realtime, IP_ADDRESS_TYPE );
   }

   // Create the decoder object
   m_decode = new EQDecode (this, "decode");
   
   connect(m_decode, SIGNAL(keyChanged(void)),
	   this, SIGNAL(keyChanged(void)));

   m_busy_decoding     = false;
   m_serverArqSeqFound = false;
   m_zoning            = false; //start in non zoning state

   m_serverArqSeqExp  = 0;
   m_serverPort       = 0;
   m_clientPort       = 0;

   /* Initialize the time of day structure */
   memset( &m_eqTime, 0x00, sizeof(struct eqTimeOfDay) );

   m_viewUnknownData = false;

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
       showeq_params->recordpackets   = 0;
       showeq_params->playbackpackets = 0;
   }

   InitializeOpCodeMonitor();

   // a sanity check, if the user set it to below 32, they're prolly nuts
   if (showeq_params->arqSeqGiveUp >= 32)
       m_serverArqSeqGiveUp = showeq_params->arqSeqGiveUp;
   else
       m_serverArqSeqGiveUp = 32;

   m_packetCount = 0;

#if HAVE_LIBEQ
   if (showeq_params->broken_decode)
       printf("Running with broken decode set to true.\n");
#else
   printf("Running without libEQ.a linked in.\n");
#endif
}

EQPacket::~EQPacket()
{
#ifdef PACKET_CACHE_DIAG
  printf("SEQ: Maximum Packet Cache Used: %d\n", maxServerCacheCount);
#endif
  // try to close down VPacket cleanly
  if (m_vPacket != NULL)
  {
    // make sure any data is flushed to the file
    m_vPacket->Flush();

    // delete VPacket
    delete m_vPacket;
  }
}

void EQPacket::InitializeOpCodeMonitor (void)
{
  do
  {
    if (!showeq_params->monitorOpCode_Usage)
      break;

    QString qsMonitoredOpCodes = strdup(showeq_params->monitorOpCode_List);

    if (qsMonitoredOpCodes == "0") /* DISABLED */
      break;

    if (qsMonitoredOpCodes.isEmpty())
      break;

    QString qsCommaBuffer (""); /* Construct these outside the loop so we don't have to construct
                                   and destruct these variables during each iteration... */
    QString qsColonBuffer ("");

    int            iCommaPos      = 0;
    int            iColonPos      = 0;
    uint8_t        uiIterationID  = 0;

    for (uint8_t uiIndex = 0; uiIndex < OPCODE_SLOTS && !qsMonitoredOpCodes.isEmpty(); uiIndex ++)
    {
      /* Initialize the variables with their default values */
      MonitoredOpCodeList      [uiIndex] [0] = 0; /* OpCode number (16-bit HEX) */
      MonitoredOpCodeList      [uiIndex] [1] = 3; /* Direction, DEFAULT: Client <--> Server */
      MonitoredOpCodeList      [uiIndex] [2] = 1; /* Show raw data if packet is marked as known */
      MonitoredOpCodeAliasList [uiIndex]     = "Monitored OpCode"; /* Name used when displaying the raw data */

      iCommaPos = qsMonitoredOpCodes.find (",");

      if (iCommaPos == -1)
        iCommaPos = qsMonitoredOpCodes.length ();

      qsCommaBuffer = qsMonitoredOpCodes.left (iCommaPos);
      qsMonitoredOpCodes.remove (0, iCommaPos + 1);


      uiIterationID = 0;


      uint8_t uiColonCount = qsCommaBuffer.contains(":");
      iColonPos = qsCommaBuffer.find (":");

      if (iColonPos == -1)
        qsColonBuffer  = qsCommaBuffer;

      else
        qsColonBuffer = qsCommaBuffer.left (iColonPos);

      while (uiIterationID <= uiColonCount)
      {
        if (uiIterationID == 0)
          MonitoredOpCodeList [uiIndex] [0] = qsColonBuffer.toUInt (NULL, 16);

        else if (uiIterationID == 1)
          MonitoredOpCodeAliasList [uiIndex] = qsColonBuffer;

        else if (uiIterationID == 2)
          MonitoredOpCodeList [uiIndex] [1] = qsColonBuffer.toUInt (NULL, 10);

        else if (uiIterationID == 3)
          MonitoredOpCodeList [uiIndex] [2] = qsColonBuffer.toUInt (NULL, 10);

        qsCommaBuffer.remove (0, iColonPos + 1);

        iColonPos = qsCommaBuffer.find (":");

        if (iColonPos == -1)
          qsColonBuffer = qsCommaBuffer;

        else
            qsColonBuffer = qsCommaBuffer.left (iColonPos);

        uiIterationID ++;
      }
    }
    m_bOpCodeMonitorInitialized = true;
  } while ( "I like to use" == "break for flow control" );
}

/* Returns string representation of numeric IP address */
QString EQPacket::print_addr (in_addr_t  addr)
{
#ifdef DEBUG_PACKET
   debug ("print_addr()");
#endif /* DEBUG_PACKET */
  QString paddr;

  if (addr == m_client_addr)
    paddr = "client";
  else
  {
    paddr.sprintf ( "%d.%d.%d.%d",
                    addr & 0x000000ff,
                    (addr & 0x0000ff00) >> 8,
                    (addr & 0x00ff0000) >> 16,
                    (addr & 0xff000000) >> 24
                  );
   }

   return paddr;
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

  unsigned char buffer[1500]; 
  short size;
  struct ether_header *ep;

  /* fetch them from pcap */
  while ((size = m_packetCapture->getPacket(buffer)))
  {
    /* Get pointer to MAC layer header */
    ep = (struct ether_header *) buffer;
    /* If its not an IP packet, discard the packet */
    //if (ntohs (ep->ether_type) != ETHERTYPE_IP)
    //   continue;
    
    /* Now.. we know the rest is an IP udp packet concerning the
     * host in question, because pcap takes care of that.
     */
      
    /* Now we assume its an everquest packet */
    if (showeq_params->recordpackets)
    {
      time_t now = time(NULL);
      m_vPacket->Record((const char *) buffer, size, now, PACKETVERSION);
    }
      
    decodePacket (size - sizeof (struct ether_header),
		  (unsigned char *) buffer + sizeof (struct ether_header) );
  }

  /* Clear decoding flag */
  m_busy_decoding = false;
}

/* Reads packets and processes waiting packets */
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
	decodePacket ( size - sizeof (struct ether_header),
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

void EQPacket::logRawData ( const char   *filename,
                            unsigned char *data,
                            unsigned int   len
                          )
{
#ifdef DEBUG_PACKET
   debug ("logRawData()");
#endif /* DEBUG_PACKET */

   int file;
   //printf("FilePath: %s\n", fname);
   file = open (filename, O_CREAT | O_APPEND | O_RDWR);

   if (-1 == file)
   {
      fprintf(stderr,"\aUnable to open file: [%s]\n", filename);
      return;
   }

   write(file, data, len);

   close (file);
}

/* Logs packet data in a human-readable format */
bool EQPacket::logData ( const char*    filename,
			 uint32_t       len,
			 const uint8_t* data,
			 in_addr_t      saddr    = 0,
			 in_addr_t      daddr    = 0,
			 in_port_t      sport    = 0,
			 in_port_t      dport    = 0
			 )
{
#ifdef DEBUG_PACKET
   debug ("logData()");
#endif /* DEBUG_PACKET */

   FILE *lh;

   //printf("FilePath: %s\n", fname);
   lh = fopen (filename, "a");

   if (lh == NULL)
   {
      fprintf(stderr, "\aUnable to open file: [%s]\n", filename);
      return false;
   }

   time_t now;
   now = time (NULL);

   if (saddr)
   {
      fprintf (lh, "%s:%d->", (const char*)print_addr (saddr), sport);
      fprintf (lh, "%s:%d ", (const char*)print_addr (daddr), dport);
   }

   fprintf (lh, "(%d) %s", len, ctime (&now));

   fprintData(lh, len, data);

   fclose (lh);

   return true;
}

/* This function decides the fate of the Everquest packet */
/* and dispatches it to the correct packet handling function */
void EQPacket::decodePacket (int size, unsigned char *buffer)
{
#ifdef DEBUG_PACKET
  debug ("decodePacket()");
#endif /* DEBUG_PACKET */
  /* Setup variables */

  EQUDPIPPacketFormat packet(buffer, size, false);

  emit numPacket(++m_packetCount);
  
  if (showeq_params->logAllPackets)
  {
    if (!logData(showeq_params->GlobalLogFilename, 
		 packet.getRawPacketLength(), 
		 (const uint8_t*)packet.getRawPacket(), 
		 packet.getIPv4SourceN(), packet.getIPv4DestN(), 
		 packet.getSourcePort(), packet.getDestPort()))
    {
      /* Problem writing to logs, untoggle the GUI checkmark to
	 show that logging is disabled. */
      emit toggle_log_AllPackets();
    }
  }

  /* Chat and Login Server Packets, Discard for now */
  if ((packet.getDestPort() == 5999) || (packet.getSourcePort() == 5999))
    return;
  if ((packet.getDestPort() >= 15000) || (packet.getSourcePort() >= 15000))
    return;

  /* discard pure ack/req packets and non valid flags*/
  if (packet.flagsHi() < 0x08 || packet.flagsHi() > 0x46 || size < 10)
    return;    

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
  printf("%s\n", (const char*)packet.headerFlags((PACKET_PROCESS_DIAG < 3)));
  uint32_t crc = packet.calcCRC32();
  if (crc != packet.crc32())
    printf("CRC: Warning Packet seq = %d CRC (%08x) != calculated CRC (%08x)!\n",
	   packet.seq(), packet.crc32(), crc);
#endif

#ifdef PACKET_PEDANTIC
  if (!packet.isValid())
  {
#if defined(PACKET_PROCESS_DIAG) || (PACKET_PEDANTIC > 1)
    printf("INVALID PACKET: Bad CRC32 [%s:%d -> %s:%d] seq %04x len %d crc32 (%08x != %08x)\n",
	   (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
	   (const char*)packet.getIPv4DestA(), packet.getDestPort(),
	   packet.seq(), 
	   packet.payloadLength(),
	   packet.crc32(), packet.calcCRC32());
#endif
    return;
  }
#endif

  /* World Server  -> Client Packet */
  if (packet.getSourcePort() == 9000)
  {
    /* unless we really start paying attention to World Servers, we should 
       be able to safely ignore sequencing and arq */

    if (m_detectingClient)
    {
      free((void*)showeq_params->ip);
      showeq_params->ip = strdup ((const char*)packet.getIPv4DestA());
      m_client_addr = packet.getIPv4DestN();
      m_detectingClient = false;
      emit clientChanged(m_client_addr);
      printf("Client Detected: %s\n", showeq_params->ip);
    }

    dispatchWorldData (packet.payloadLength(), packet.payload(), DIR_SERVER);
    return;
  }
  else if (packet.getDestPort() == 9000)
  {
    /* unless we really start paying attention to World Servers, we should 
       be able to safely ignore sequencing and arq */
    if (m_detectingClient)
    {
      free((void*)showeq_params->ip);
      showeq_params->ip = strdup ((const char*)packet.getIPv4SourceA());
      m_client_addr = packet.getIPv4SourceN();
      m_detectingClient = false;
      emit clientChanged(m_client_addr);
      printf("Client Detected: %s\n", showeq_params->ip);
    }

    dispatchWorldData (packet.payloadLength(), packet.payload(), DIR_CLIENT);
    return;
  }

  // packet from the client
  if (packet.getIPv4SourceN() == m_client_addr)
  {
    if ( packet.isSEQStart() && (m_session_tracking_enabled == 1))
    {
        m_session_tracking_enabled = 2;

	m_clientPort = packet.getSourcePort();
	
	if (!showeq_params->playbackpackets)
	{
	  if (strlen(showeq_params->mac_address) == 17)
          {
	    m_packetCapture->setFilter(showeq_params->device, 
				       showeq_params->mac_address,
				       showeq_params->realtime, 
				       MAC_ADDRESS_TYPE, m_serverPort, 
				       m_clientPort);
	    printf ("Building new pcap filter: EQ Client %s, Client port %d, Zone Server port %d\n",
		    showeq_params->mac_address, m_clientPort, m_serverPort);
	  }
	  else
	  {
	    m_packetCapture->setFilter(showeq_params->device, 
				       showeq_params->ip,
				       showeq_params->realtime, 
				       IP_ADDRESS_TYPE, m_serverPort, 
				       m_clientPort);
	    printf ("Building new pcap filter: EQ Client %s, Client port %d, Zone Server port %d\n",
		    showeq_params->ip, m_clientPort, m_serverPort);
	  }
	}

	// notify that the client port has been latched
	emit sessionTrackingChanged(m_session_tracking_enabled);
	emit clientPortLatched(m_clientPort);
    }
    
    // only dispatch packets with ASQ that aren't fragmented
    if (packet.isASQ() && !packet.isFragment())
    {
      dispatchZoneData(packet.payloadLength(), 
		       packet.payload(), DIR_CLIENT);
      return;
    }
    return;
  }

  // packet destined for client
  else if (packet.getIPv4DestN() == m_client_addr)
  {
    if (packet.isClosingHi() && packet.isClosingLo())
    {
        m_serverArqSeqFound = false;
	if(m_session_tracking_enabled)
	{
	  m_session_tracking_enabled = 1;
	  emit sessionTrackingChanged(m_session_tracking_enabled);
	}
    }
    // process packets that don't have an arq sequence
    if (!packet.isARQ())
    {
      /* does not require sequencing, so straight to dispatch */
      dispatchZoneData (packet.payloadLength(), 
			packet.payload(), DIR_SERVER);
      return;
    }
    else if (packet.isARQ()) // process ARQ sequences
    {
      uint16_t serverArqSeq = packet.arq();
      emit seqReceive(serverArqSeq);
      
      /* this conditions should only be met once per zone, New Sequence */
      if (!m_serverArqSeqFound)
      {
	if (packet.isSEQStart())
	{
	  // hey, a SEQStart, use it's packet to set 
#ifdef PACKET_PROCESS_DIAG
	  printf("SEQ: SEQStart found, setting arq seq, %04x\n", serverArqSeq);
#endif
	  m_serverArqSeqExp = serverArqSeq;
	  m_serverArqSeqFound = true;
	  emit seqExpect(m_serverArqSeqExp);
	}
	else
	{
	  // the user must have started up with an EQ session already running
	  // we hope... /shrug
#ifdef PACKET_PROCESS_DIAG
	  printf("SEQ: New sequence found, setting arq seq, %04x\n",
		 serverArqSeq);
#endif
	  m_serverArqSeqExp = serverArqSeq;
	  m_serverArqSeqFound = true;
	  emit seqExpect(m_serverArqSeqExp);
	}
      }

      // is this the currently expected sequence, if so, do something with it.
      if (m_serverArqSeqExp == serverArqSeq)
      {
	m_serverArqSeqExp = serverArqSeq + 1;
	emit seqExpect(m_serverArqSeqExp);

#ifdef PACKET_PROCESS_DIAG
	printf("SEQ: Found next arq in data stream, incrementing arq seq, %04x\n", 
	       serverArqSeq);
#endif
	
	// if the packet isn't a fragment dispatch normally, otherwise to split
	if (!packet.isFragment())
	  dispatchZoneData (packet.payloadLength(),
			    packet.payload(), DIR_SERVER);
	else
	  dispatchZoneSplitData (packet);
      }
      // it's a packet from the future, add it to the cache
      else if ((serverArqSeq > m_serverArqSeqExp) || 
	       (serverArqSeq < (int32_t(m_serverArqSeqExp) - arqSeqWrapCutoff))) 
      {
#ifdef PACKET_PROCESS_DIAG
	printf("SEQ: out of order arq %04x, sending to cache, %04d\n", serverArqSeq, m_serverCache.size());
#endif
	
	// check if the entry already exists in the cache
	EQPacketMap::iterator it = m_serverCache.find(serverArqSeq);
	
	if (it == m_serverCache.end())
	{
	  // entry doesn't exist, so insert an entry into the cache
#ifdef PACKET_PROCESS_DIAG
	  printf("SEQ: Insert arq (%04x) into cache\n", serverArqSeq);
#endif
	  m_serverCache.insert(EQPacketMap::value_type(serverArqSeq, new EQPacketFormat(packet, true)));
	  emit cacheSize(m_serverCache.size());
	}
	else 
	{
	  // replacing an existing entry, make sure the new data is valid
	  if (packet.isValid())
	  {
#ifdef PACKET_PROCESS_DIAG
	    printf("SEQ: Update arq (%04x) in cache\n", serverArqSeq);
#endif
	    *it->second = packet;
	  }
#ifdef PACKET_PROCESS_DIAG
	  else
	    printf("SEQ: Not Updating arq (%04x) in cache, CRC error!\n",
		   serverArqSeq);
#endif
	}

#ifdef PACKET_CACHE_DIAG
	if (m_serverCache.size() > maxServerCacheCount)
	  maxServerCacheCount = m_serverCache.size();
#endif // PACKET_CACHE_DIAG
      }
	
      ////////////////////////////////////////////////////
      // Cache processing
      //

      // if the cache isn't empty, then check for the expected ARQ sequence
      if (!m_serverCache.empty())
      {
#if defined(PACKET_CACHE_DIAG)
	printf("SEQ: START checking cache, arq %04x, cache count %04d\n", 
	       m_serverArqSeqExp, m_serverCache.size());
#endif
	EQPacketMap::iterator it;
	EQPacketMap::iterator eraseIt;
	EQPacketFormat* pf;

	// check if the cache has grown large enough that we should give up
	// on seeing the current serverArqSeqExp
	// this should really only kick in for people with pathetic
	// network cards that missed the packet.
	if (m_serverCache.size() >= m_serverArqSeqGiveUp)
	{
	  // ok, if the expected server arq sequence isn't here yet, give up

	  // attempt to find the current expencted arq seq
	  it = m_serverCache.find(m_serverArqSeqExp);

	  // keep trying to find a new serverArqSeqExp if we haven't found a good
	  // one yet... 
	  while(it == m_serverCache.end())
	  {
#ifdef PACKET_CACHE_DIAG
	    printf("SEQ: Giving up on finding arq %04x in cache, skipping!\n", 
		   m_serverArqSeqExp);

#endif
	    // incremente the expected arq sequence number
	    m_serverArqSeqExp++;
	    emit seqExpect(m_serverArqSeqExp);

	    // attempt to find the new current expencted arq seq
	    it = m_serverCache.find(m_serverArqSeqExp);
	  }
	}
	else
	{
	  // haven't given up yet, just try to find the current serverArqSeqExp

	  // attempt to find the current expencted ARQ seq
	  it = m_serverCache.find(m_serverArqSeqExp);
	}


	// iterate over cache until we reach the end or run out of 
	// immediate followers
	while (it != m_serverCache.end())
	{
	  // get the PacketFormat for the iterator
	  pf = it->second;
	
	  // make sure this is the expected packet
	  // (we might have incremented to the one after the one returned
	  /// by find above).
	  if (pf->arq() != m_serverArqSeqExp)
	    break;

#ifdef PACKET_CACHE_DIAG
	  printf("SEQ: found next arq %04x in cache, cache count %04d\n",
		 m_serverArqSeqExp, m_serverCache.size());
#endif

#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
	  // validate the packet against a memory corruption
	  if (!pf->isValid())
	  {
	    // Something's screwed up
	    printf("SEQ: INVALID PACKET: Bad CRC32 in packet in cache with arq %04x!\n",
		   pf->arq());
	  }
#endif

	  // is it a fragment?
	  if (!pf->isFragment())
	  {
	    // no, dispatch normally
#ifdef PACKET_CACHE_DIAG
	    printf("SEQ: Dispatching arq %04x with pkt[0] %02x and Opcode: %04x from cache, cache count %04d\n", 
		   pf->arq(), pf->flagsHi(), eqntohuint16(pf->payload()),
		   m_serverCache.size());
#endif
	  
	    dispatchZoneData(pf->payloadLength(), 
			     pf->payload(),  
			     DIR_SERVER);
	  }
	  else
	  {
	    // yes, it's a fragment.  Dispatch to split
#ifdef PACKET_CACHE_DIAG
	    printf("SEQ: Dispatching to Split arq %04x with pkt[0] %02x from cache, cache count %04d\n", 
		   pf->arq(), pf->flagsHi(), m_serverCache.size());
#endif

	    dispatchZoneSplitData(*pf);
	  }

	  eraseIt = it;
	  
	  // increment the current position iterator
	  it++;

	  // erase the packet from the cache
	  m_serverCache.erase(eraseIt);
	  emit cacheSize(m_serverCache.size());

#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
	  // validate the packet against a memory corruption
	  if (!pf->isValid())
	  {
	    // Something's screwed up
	    printf("SEQ: INVALID PACKET: Bad CRC32 in packet in cache with arq %04x!\n",
		   pf->arq());
	  }
#endif
#ifdef PACKET_CACHE_DIAG
	  printf("SEQ: REMOVING arq %04x from cache, cache count %04d\n", 
		 pf->arq(), m_serverCache.size());
#endif
	  // delete the packet
	  delete pf;
	  
	  // increment the expected arq number
	  m_serverArqSeqExp++;
	  emit seqExpect(m_serverArqSeqExp);

	  if (m_serverArqSeqExp == 0)
	    it = m_serverCache.begin();
	}
      
#ifdef PACKET_CACHE_DIAG
	printf("SEQ: FINISHED checking cache, arq %04x, cache count %04d\n", 
	       m_serverArqSeqExp, m_serverCache.size());
#endif
      }
    }
    else
      return; /* sanity */
  }
}

/* Combines data from split packets */
void EQPacket::dispatchZoneSplitData (EQPacketFormat& pf)
{
#ifdef DEBUG_PACKET
   debug ("dispatchZoneSplitData()");
#endif /* DEBUG_PACKET */
   if (pf.isASQ())
   {
      /* Clear data */
      m_serverDataSize = 0;
   }

   if ((m_serverDataSize + pf.payloadLength()) > sizeof(m_serverData))
     printf("WARNING: ServerDataSize(%d) > sizeof(serverData)(%d)\n",
	    (m_serverDataSize + pf.payloadLength()), sizeof(m_serverData));

   // Add data
   memcpy((void*)(m_serverData + m_serverDataSize), (void*)pf.payload(), 
	  pf.payloadLength());

   m_serverDataSize += pf.payloadLength();

   /* Check if this is last part of data */
   if (pf.fragCur() == (pf.fragTot() - 1))
   {
#ifdef PACKET_PROCESS_DIAG
      printf("SEQ: seq complete, dispatching (opcode=%04x)\n", 
	     eqntohuint16(m_serverData));
#endif
      dispatchZoneData (m_serverDataSize, m_serverData, DIR_SERVER);
   }
}

///////////////////////////////////////////
//EQPacket::dispatchWorldData  
// note this dispatch gets just the payload
void EQPacket::dispatchWorldData (uint32_t len, uint8_t *data, 
				  uint8_t direction = 0)
{
#ifdef DEBUG_PACKET
  debug ("dispatchWorldData()");
#endif /* DEBUG_PACKET */
  if (len < 10)
    return;
  
  uint16_t opCode = eqntohuint16(data);

  if ((opCode == ZoneServerInfo) && (direction == DIR_SERVER))
  {
    printf(" ZoneServerInfo 0x%04x, m_client_addr %d, sessionTrack = %d\n", 
	   opCode, m_client_addr, m_session_tracking_enabled);
    uint16_t zone_server_port = eqntohuint16(data + 130);
    m_serverPort = zone_server_port;
    
    if (m_logger->isLoggingZoneServerInfo())
        m_logger->logZoneServerInfo(data,len,direction);

    // only reset packet filter if this is a live session
    if (!showeq_params->playbackpackets && (m_session_tracking_enabled < 2))
    {
      if (strlen(showeq_params->mac_address) == 17)
      {
	m_packetCapture->setFilter(showeq_params->device, 
				   showeq_params->mac_address,
				   showeq_params->realtime, 
				   MAC_ADDRESS_TYPE, zone_server_port, 0);
	printf ("Building new pcap filter: EQ Client %s, Zone Server port %d\n",
		showeq_params->mac_address, zone_server_port);
      }
      else
      {
	m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
				   showeq_params->realtime, IP_ADDRESS_TYPE,
				   zone_server_port, 0);
	printf ("Building new pcap filter: EQ Client %s, Zone Server port %d\n",
                 showeq_params->ip, zone_server_port);
      }
    }

    // notify that the server port has been latched
    emit serverPortLatched(m_serverPort);
    
    // we'll be waiting for a new SEQStart
    m_serverArqSeqFound = false;
    
    // clear out all the cache entries
    
    // first delete all the entries
    EQPacketMap::iterator it = m_serverCache.begin();
    EQPacketFormat* pf;
    fprintf(stderr, "Clearing Cache: Count: %d\n", m_serverCache.size());
    while (it != m_serverCache.end())
    {
      pf = it->second;
      delete pf;
      it++;
    }
    
    // now clear the cache
    printf ("Reseting sequence cache\n");
    m_serverCache.clear();
    emit cacheSize(0);
    return;
  }
}

void EQPacket::dispatchZoneData (uint32_t len, uint8_t *data, 
				 uint8_t dir)
{
#ifdef DEBUG_PACKET
    debug ("dispatchZoneData()");
#endif /* DEBUG_PACKET */

    QString  tempStr;
    int      decoded = 0;
    uint32_t decodedDataLen = 65536;
    uint8_t  decodedData[decodedDataLen];

    if (len < 0)
        return;

    /* pre-process the packets */
    decoded = m_decode->DecodePacket(data, len, decodedData,
                                     &decodedDataLen, showeq_params->ip);

    uint16_t opCode = eqntohuint16(data); // data[1] | (data[0] << 8);

    //Logging 
    if (showeq_params->logZonePackets)
        if (!logData (showeq_params->ZoneLogFilename, len, data))
            emit toggle_log_ZoneData(); //untoggle the GUI checkmark

    bool unk = true;

    /* Update EQ Time every 50 packets so we don't overload the CPU */
    
    if ( showeq_params->showEQTime && (m_packetCount % 50 == 0))
    {
        char timeMessage[30];
        time_t timeCurrent = time(NULL);

        struct timeOfDayStruct eqTime;
        getEQTimeOfDay( timeCurrent, &eqTime);

        if (eqTime.hour >= 0 && eqTime.minute >= 0)
        {
            sprintf(timeMessage,"EQTime [%02d:%s%d %s]",
                       (eqTime.hour % 12) == 0 ? 12 : (eqTime.hour % 12),
                       (eqTime.minute < 10) ? "0" : "",
                       (eqTime.minute),
                       (eqTime.hour >= 12) ? "pm" : "am"
                   );
            emit eqTimeChangedStr(QString (timeMessage));
        }
    }

    switch (opCode)
    {
        case CPlayerItemsCode:
        {
            if (m_logger->isLoggingCPlayerItems())
                m_logger->logCPlayerItems((cPlayerItemsStruct *)data,len,dir);

            if (CPlayerItemsVer != opCodeVersion)
                break;
	
	    unk = false;

	    if (!decoded)
	        break;

            cPlayerItemsStruct *citems;
            citems = (cPlayerItemsStruct *)(decodedData);
	
            // Make sure we do not divide by zero and there 
            // is something to process
           
            if (citems->count)
            {
                // Determine the size of a single structure in 
                // the compressed packet
              
                int nPacketSize=((decodedDataLen - 4)/citems->count);

                // See if it is the size that we expect
              
                int nVerifySize = sizeof(playerItemStruct);
	    
                if (nVerifySize == nPacketSize)
                {
                    if (!showeq_params->no_bank)
                    {
                        tempStr = "Item: Dispatching compressed Items " 
                                  "(count = " +
                                  QString::number(citems->count) + ")";

                        emit msgReceived(tempStr);
	      
                        for (int i=0; i < citems->count; i++)
                            dispatchZoneData( nPacketSize,
                                &citems->compressedData[i*nPacketSize], 
                                dir);

	                emit msgReceived(QString("Item: Finished dispatching"));
	             }
	             else
	             {
	                 // Quietly dispatch the compressed data
	           
                         for ( int i=0; i<citems->count; i++ )
                             dispatchZoneData( nPacketSize, 
                                &citems->compressedData[i*nPacketSize], 
                                dir);
	             }
	        }
            }
	    break;
        } /* end CPlayerItemCode */

        case ItemInShopCode:
        {
            if (m_logger->isLoggingItemInShop())
                m_logger->logItemInShop((itemInShopStruct *)data,len,dir);

            if (ItemInShopVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(ItemInShopCode, itemInShopStruct);
	    
            emit itemShop((const itemInShopStruct*)data);
	    
            break;
        } /* end ItemInShopCode */

        case MoneyOnCorpseCode:
        {
            if (m_logger->isLoggingMoneyOnCorpse())
                m_logger->logMoneyOnCorpse((moneyOnCorpseStruct *)data,len,dir);

	    if (MoneyOnCorpseVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(MoneyOnCorpseCode, moneyOnCorpseStruct);

            emit moneyOnCorpse((const moneyOnCorpseStruct*)data);

            break;
        } /* end MoneyOnCorpseCode */

        case ItemOnCorpseCode:
        {
            if (m_logger->isLoggingItemOnCorpse())
                m_logger->logItemOnCorpse((itemOnCorpseStruct *)data,len,dir);

            if (ItemOnCorpseVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(ItemOnCorpseCode, itemOnCorpseStruct);

	    emit itemPlayerReceived((const tradeItemInStruct *)data);

            break;
        } /* end ItemOnCorpseCode */

        case TradeItemOutCode:
        {
            if (m_logger->isLoggingTradeItemOut())
                m_logger->logTradeItemOut((tradeItemOutStruct *)data,len,dir);

            if (TradeItemOutVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(TradeItemOutCode, tradeItemOutStruct);

            emit tradeItemOut((const tradeItemOutStruct*)data);

            break;
        } /* end tradeItemCode */

        case TradeItemInCode:	// Item received by player
        {
            if (m_logger->isLoggingTradeItemIn())
                m_logger->logTradeItemIn((tradeItemInStruct *)data,len,dir);

            if (TradeItemInVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(TradeItemInCode, tradeItemInStruct);

            emit tradeItemIn((const tradeItemInStruct*)data);

            break;
        } /* end ItemTradeCode */
        
        case PlayerItemCode:
        {
            if (m_logger->isLoggingPlayerItem())
                m_logger->logPlayerItem((playerItemStruct *)data,len,dir);
 
            if (PlayerItemVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(PlayerItemCode, playerItemStruct);

            emit wearItem((const playerItemStruct*)data);

            break;
        }

        case SummonedItemCode:
        {
            if (m_logger->isLoggingSummonedItem())
                m_logger->logSummonedItem((summonedItemStruct *)data,len,dir);
    
            if (SummonedItemVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(SummonedItemCode, summonedItemStruct);
	
	    emit summonedItem((const summonedItemStruct*)data);

            break;
        }

        case CharProfileCode:	// Character Profile server to client
        {
            if (m_logger->isLoggingECharProfile())
                m_logger->logECharProfile((charProfileStruct *)data,len,dir);

            if (CharProfileVer != opCodeVersion)
                break;

            /* This is an encrypted packet type, so log the raw packet for
               detailed analysis */

            if (showeq_params->logEncrypted)
                logData(showeq_params->CharProfileCodeFilename, len, data);

            unk = false; // move above if to prevent duplicate logging
                         // not sure if its placement was intentional before.

            if (decoded && !showeq_params->broken_decode)
            {
	        printf("EQPacket::dispatchZoneData():CharProfileCode:Decoded\n");
	        // just call dispatchDecodedCharProfile (logged there as well)
	       
                dispatchDecodedCharProfile(decodedData, decodedDataLen);
            }
            else
	        printf("EQPacket::dispatchZoneData():CharProfileCode:Not Decoded\n");
            break;
        }

        case NewCorpseCode:
        {
            if (m_logger->isLoggingNewCorpse())
                m_logger->logNewCorpse((newCorpseStruct *)data,len,dir);

            if (NewCorpseVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(NewCorpseCode, newCorpseStruct);

            emit killSpawn((const newCorpseStruct*) data);

            break;
        } /* end CorpseCode */

        case DeleteSpawnCode:
        {
            if (m_logger->isLoggingDeleteSpawn())
                m_logger->logDeleteSpawn((deleteSpawnStruct *)data,len,dir);

            if (DeleteSpawnVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(DeleteSpawnCode, deleteSpawnStruct);

            emit deleteSpawn((const deleteSpawnStruct*)data);

            break;
        }

        case ChannelMessageCode:
        {
            if (m_logger->isLoggingChannelMessage())
                m_logger->logChannelMessage((channelMessageStruct *)data,len,dir);

            if (ChannelMessageVer != opCodeVersion)
                break;

            unk = false;

            emit channelMessage((const channelMessageStruct*)data,
                                (dir == DIR_CLIENT));

            break;
        }

        case NewSpawnCode:
        {
            if (m_logger->isLoggingENewSpawn())
                m_logger->logENewSpawn((newSpawnStruct *)data,len,dir);

            if (NewSpawnVer != opCodeVersion)
                break;

            /* This is one of the encrypted packet types, so log it */

            if (showeq_params->logEncrypted)
                logData(showeq_params->NewSpawnCodeFilename, len, data);

            // printf("NewSpawn received\n");

            if (!decoded || showeq_params->broken_decode)
                break;

            if (m_logger->isLoggingNewSpawn())
                m_logger->logNewSpawn((newSpawnStruct *)decodedData,len,dir);

            unk = ! ValidateDecodedPayload(NewSpawnCode, newSpawnStruct);

	    if (!m_zoning)
	        emit newSpawn((const newSpawnStruct*)decodedData);

            break;
        }

        case ZoneSpawnsCode:
        {
            if (m_logger->isLoggingEZoneSpawns())
                m_logger->logEZoneSpawns((zoneSpawnsStruct *)data,len,dir);

            // printf ("ZONESPAWNS1: %d\n", len);

            if (ZoneSpawnsVer != opCodeVersion)
                break;

            /* This is one of the encrypted packet types, so log it */

            if (showeq_params->logEncrypted)
                logData(showeq_params->ZoneSpawnsCodeFilename, len, data);
  
            m_logger->logZoneSpawnsTimestamp(); // save timestamp for later log

            unk = false; // move above break to prevent duplicate logging
                         // not sure if its placement was intentional before.

            if (!decoded || showeq_params->broken_decode)
                break;

            if (m_logger->isLoggingZoneSpawns())
                m_logger->logZoneSpawns((zoneSpawnsStruct *) 
                   decodedData, decodedDataLen, dir);
 
            emit zoneSpawns((const zoneSpawnsStruct*)decodedData, 
	                    decodedDataLen);

            break;
        }

        case TimeOfDayCode:
        {
            struct timeOfDayStruct *tday;

            if (m_logger->isLoggingTimeOfDay())
                m_logger->logTimeOfDay((timeOfDayStruct *)data,len,dir);

            if (TimeOfDayVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(TimeOfDayCode, timeOfDayStruct);

            tday = (struct timeOfDayStruct *) data;

            m_eqTime.zoneInTime.minute   = tday->minute;
            m_eqTime.zoneInTime.hour     = tday->hour;
            m_eqTime.zoneInTime.day      = tday->day;
            m_eqTime.zoneInTime.month    = tday->month;
            m_eqTime.zoneInTime.year     = tday->year;

            m_eqTime.packetReferenceTime = time(NULL);

            printf( "TIME: %02d:%02d %02d/%02d/%04d\n",
                    tday->hour,
                    tday->minute,
                    tday->month,
                    tday->day,
                    tday->year
                 );
            emit timeOfDay(tday);
            break;
        }

        case BookTextCode:
        {
            struct bookTextStruct *booktext;

            if (m_logger->isLoggingBookText())
                m_logger->logBookText((bookTextStruct *)data,len,dir);

            if (BookTextVer != opCodeVersion)
               break;

            unk = false;

            booktext = (struct bookTextStruct *) data;

            // printf ("BOOK: %s\n", booktext->text);

            break;
        }

        case RandomCode:
        {
            if (dir != DIR_CLIENT)
                break;

            if (m_logger->isLoggingRandom())
                m_logger->logRandom((randomStruct *)data,len,dir);

            if (RandomVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(RandomCode, randomStruct);

	    emit random((const randomStruct*)data);

            break;
        }

        case EmoteTextCode:
        {
            if (m_logger->isLoggingEmoteText())
                 m_logger->logEmoteText((emoteTextStruct *)data,len,dir);

            if (EmoteTextVer != opCodeVersion)
                break;

            unk = false;

	    emit emoteText((const emoteTextStruct*)data);

            break;
        }

        case CorpseLocCode:
        {
            if (m_logger->isLoggingCorpseLoc())
                m_logger->logCorpseLoc((corpseLocStruct *)data,len,dir);

	    if (CorpseLocCode != opCodeVersion)
	        break;

	    unk = ! ValidatePayload(CorpseLocCode, corpseLocStruct);
           
	    emit corpseLoc((const corpseLocStruct*)data);

            break;
        }
        
        case PlayerBookCode:
        {
            if (m_logger->isLoggingPlayerBook())
                m_logger->logPlayerBook((playerBookStruct *)data,len,dir);
            
            if (PlayerBookVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(PlayerBookCode, playerBookStruct);
 
            emit playerBook((const playerBookStruct*)data);

            break;
        }

        case PlayerContainerCode:
        {
            if (m_logger->isLoggingPlayerContainer())
                m_logger->logPlayerContainer((playerContainerStruct *)data,len,dir);

            if (PlayerContainerVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(PlayerContainerCode, playerContainerStruct);

            emit playerContainer((const playerContainerStruct *)data);

            break;
        }

        case InspectDataCode:
        {
            if (m_logger->isLoggingInspectData())
                m_logger->logInspectData((inspectDataStruct *)data,len,dir);

            if (InspectDataVer != opCodeVersion)
                break;

            unk = false;

            emit inspectData((const inspectDataStruct *)data);

            break;
        }

        case HPUpdateCode:
        {
            if (m_logger->isLoggingHPUpdate())
                m_logger->logHPUpdate((hpUpdateStruct *)data,len,dir);

            if (HPUpdateVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(HPUpdateCode, hpUpdateStruct);

            emit updateSpawnHP((const hpUpdateStruct*)data);

            break;
        }

        case SPMesgCode:
        {
            if (m_logger->isLoggingSPMesg())
                m_logger->logSPMesg((const spMesgStruct *)data,len,dir);

            // struct spMesgStruct * spmsg1 = (struct spMesgStruct *) data;
            // printf("Special (%d): '%s' (%ld)\n", 
            //     opCodeVersion, spmsg1->message, spmsg1->msgType);

            if (SPMesgVer != opCodeVersion)
                break;

            unk = false;

            emit spMessage((const spMesgStruct *)data);

            break;
        }

        case MemSpellCode:
        {
            if (m_logger->isLoggingMemSpell())
                m_logger->logMemSpell((const memSpellStruct *)data,len,dir);

            if (MemSpellVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(MemSpellCode, memSpellStruct);

            emit handleSpell((const memSpellStruct*)data, 
                             (dir == DIR_CLIENT));

            break;
        }

        case BeginCastCode:
        {
            if (m_logger->isLoggingBeginCast())
                m_logger->logBeginCast((beginCastStruct *)data,len,dir);

            if (BeginCastVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(BeginCastCode, beginCastStruct);

            emit beginCast((const beginCastStruct*)data, 
                           (dir == DIR_CLIENT));

            break;
        }

        case StartCastCode:
        {
            if (m_logger->isLoggingStartCast())
                m_logger->logStartCast((startCastStruct *)data,len,dir);

	    unk = ! ValidatePayload(StartCastCode, startCastStruct);

	    emit startCast((const startCastStruct*)data);

	    break;
        }

        case MobUpdateCode:
        {
            if (m_logger->isLoggingMobUpdate())
                m_logger->logMobUpdate((const mobUpdateStruct *)data,len,dir);

            if (MobUpdateVer != opCodeVersion)
                break;

            unk = false;

#ifdef PACKET_PAYLOAD_SIZE_DIAG
            struct mobUpdateStruct *updates;

            updates = (mobUpdateStruct *) (data);

            // verify size

            int updateSize = (len - 6) / updates->numUpdates;

            if (updateSize != sizeof(spawnPositionUpdate))
            {
                printf("WARNING: MobUpdateCode (dataLen:%d != "
                       "sizeof(spawnPositionUpdate):%d)!\n", 
                       updateSize, sizeof(spawnPositionUpdate));

	        unk = true;
	    }
#endif
	    if (!m_zoning)
	        emit updateSpawns((const mobUpdateStruct *)data);

            break;
        }

        case ExpUpdateCode:
        {
            if (m_logger->isLoggingExpUpdate())
                m_logger->logExpUpdate((const expUpdateStruct *)data,len,dir);

            if (ExpUpdateVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(ExpUpdateCode, expUpdateStruct);

            emit updateExp((const expUpdateStruct*)data);

            break;
        }

        case AltExpUpdateCode:
        {
            if (m_logger->isLoggingAltExpUpdate())
                m_logger->logAltExpUpdate((const altExpUpdateStruct *)data,len,dir);

            if (AltExpUpdateVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(AltExpUpdateCode, altExpUpdateStruct);

            emit updateAltExp((const altExpUpdateStruct*)data);

            break;
        }

        case LevelUpUpdateCode:
        {
            if (m_logger->isLoggingLevelUpUpdate())
                m_logger->logLevelUpUpdate((levelUpUpdateStruct *)data,len,dir);

            if (LevelUpUpdateVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(LevelUpUpdateCode, levelUpUpdateStruct);

            emit updateLevel((const levelUpUpdateStruct *)data);

            break;
        }

        case SkillIncCode:
        {
            if (m_logger->isLoggingSkillInc())
                m_logger->logSkillInc((skillIncStruct *)data,len,dir);

            if (SkillIncVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(SkillIncCode, skillIncStruct);

	    emit increaseSkill((const skillIncStruct *)data);

            break;
        }

        case DoorOpenCode:
        {
            if (m_logger->isLoggingDoorOpen())
                m_logger->logDoorOpen(data, len, dir);

            if (DoorOpenVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case IllusionCode:
        {
            if (m_logger->isLoggingIllusion())
                m_logger->logIllusion(data,len,dir);

            if (IllusionVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case BadCastCode:
        {
            if (m_logger->isLoggingBadCast())
                m_logger->logBadCast((badCastStruct *)data,len,dir);

            if (BadCastVer != opCodeVersion)
                break;

            unk = false; //! ValidatePayload(BadCastCode, badCastStruct);

            emit interruptSpellCast((const badCastStruct*)data);

            break;
        }

        case SysMsgCode:
        {
            if (m_logger->isLoggingSysMsg())
                m_logger->logSysMsg((sysMsgStruct *)data,len,dir);

            if (SysMsgVer != opCodeVersion)
                break;

            unk = false;

	    emit systemMessage((const sysMsgStruct*)data);

            break;
        }

        case ZoneChangeCode:
        {
            if (m_logger->isLoggingZoneChange())
                m_logger->logZoneChange((zoneChangeStruct *)data,len,dir);

            if (ZoneChangeVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(ZoneChangeCode, zoneChangeStruct);

            // in the process of zoning, server hasn't switched yet.

            m_zoning = true;

	    emit zoneChange((const zoneChangeStruct*)data, 
			    (dir == DIR_CLIENT));
#if HAVE_LIBEQ
            emit resetDecoder();
#endif
            break;
        }

        case ZoneEntryCode:
        {
            if (m_logger->isLoggingZoneEntry())
                m_logger->logZoneEntry(data, len, dir);

            if (ZoneEntryVer != opCodeVersion)
                break;

            // We're only interested in the server version

            if (dir == DIR_CLIENT)
	    {
	        unk = ! ValidatePayload(ZoneEntryCode, ClientZoneEntryStruct);
	        emit zoneEntry((const ClientZoneEntryStruct*)data);
#if HAVE_LIBEQ
                emit resetDecoder();
#endif
	        m_zoning = true;
	        break;
            }

            unk = ! ValidatePayload(ZoneEntryCode, ServerZoneEntryStruct);

            emit zoneEntry((const ServerZoneEntryStruct*)data);

            // server considers us in the other zone now
	      
            m_zoning = false;

            break;
        } /* end ZoneEntryCode */

        case NewZoneCode:
        {
            if (m_logger->isLoggingNewZone())
                m_logger->logNewZone((newZoneStruct *)data,len,dir);

            if (NewZoneVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(NewZoneCode, newZoneStruct);

	    emit zoneNew((const newZoneStruct*)data,
			 (dir == DIR_CLIENT));

	   // note that we're no longer Zoning
	   m_zoning = false;
	    
           if (m_vPacket)
              printf("New Zone at byte: %ld\n", m_vPacket->FilePos());

            break;
        }

        case PlayerPosCode:
        {
            if (m_logger->isLoggingPlayerPos())
                m_logger->logPlayerPos((playerPosStruct *)data,len,dir);

            if (PlayerPosVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(PlayerPosCode, playerPosStruct);

	    if (!m_zoning)
	        emit playerUpdate((const playerPosStruct*)data, 
		                  (dir == DIR_CLIENT));

            break;
        }

        case WearChangeCode:
        {
            if (m_logger->isLoggingWearChange())
                m_logger->logWearChange((wearChangeStruct *)data,len,dir);

            if (WearChangeVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(WearChangeCode, wearChangeStruct);

            emit spawnWearingUpdate ((const wearChangeStruct*)data);

            break;
        }

        case ActionCode:
        {
            if (m_logger->isLoggingAction())
                m_logger->logAction((actionStruct *)data,len,dir);

            if (ActionVer != opCodeVersion)
                break;

            unk = false;

            emit action2Message ((const action2Struct *)data);

#if 0
            struct actionStruct* action;
            action = (struct actionStruct*)(data);

            if( action->spell != -1 )
            {
               printf("%s(%d) casted %s on %s(%d) for (%d) points of damage\n",
                   m_Spawns[action->source].name, action->source, 
                   (const char*)spell_name(action->spell), 
                   m_Spawns[action->target].name, action->target, 
                   action->damage );
            }
#endif 

            break;
        }

        case CastOnCode:
        {
            if (m_logger->isLoggingCastOn())
                m_logger->logCastOn((castOnStruct *)data,len,dir);

            if (CastOnVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case ManaDecrementCode:
        {
            if (m_logger->isLoggingManaDecrement())
                m_logger->logManaDecrement((manaDecrementStruct *)data,len,dir);

            if (ManaDecrementVer != opCodeVersion)
               break;

            unk = ! ValidatePayload(ManaDecrementCode, manaDecrementStruct);

	    emit manaChange((struct manaDecrementStruct *)data);

            break;
        }

        case StaminaCode:
        {
            if (m_logger->isLoggingStamina())
                m_logger->logStamina((staminaStruct *)data,len,dir);

            if (StaminaVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(StaminaCode, staminaStruct);

	    emit updateStamina((const staminaStruct *)data);

            break;
        }

        case MakeDropCode:
        {
            if (m_logger->isLoggingMakeDrop())
                m_logger->logMakeDrop((makeDropStruct *)data,len,dir);

            if (MakeDropVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(MakeDropCode, makeDropStruct);

	    if (!m_zoning)
                emit newGroundItem((const makeDropStruct*)data, 
		                   (dir == DIR_CLIENT));

            break;
        }

        case RemDropCode:
        {
            if (m_logger->isLoggingRemDrop())
                m_logger->logRemDrop((remDropStruct *)data,len,dir);

            if (RemDropVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(RemDropCode, remDropStruct);

            emit removeGroundItem((const remDropStruct *)data);

            break;
        }

        case DropCoinsCode:
        {
            if (m_logger->isLoggingDropCoins())
                m_logger->logDropCoins((dropCoinsStruct *)data,len,dir);

            if (DropCoinsVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(DropCoinsCode, dropCoinsStruct);

	    if (!m_zoning)
                emit newCoinsItem((const dropCoinsStruct*)data);

            break;
        }

        case RemoveCoinsCode:
        {
            if (m_logger->isLoggingRemoveCoins())
                m_logger->logRemoveCoins((removeCoinsStruct *)data,len,dir);

            if (RemoveCoinsVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(RemoveCoinsCode, removeCoinsStruct);

            emit removeCoinsItem ((const removeCoinsStruct *)data);

            break;
        }

        case OpenVendorCode:
        {
            if (m_logger->isLoggingOpenVendor())
                m_logger->logOpenVendor(data,len,dir);

            if (OpenVendorVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case CloseVendorCode:
        {
            if (m_logger->isLoggingCloseVendor())
                m_logger->logCloseVendor(data,len,dir);

            if (CloseVendorVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case OpenGMCode:
        {
            if (m_logger->isLoggingOpenGM())
                m_logger->logOpenGM(data,len,dir);

            if (OpenGMVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case CloseGMCode:
        {
            if (m_logger->isLoggingCloseGM())
                m_logger->logCloseGM(data,len,dir);

            if (CloseGMVer != opCodeVersion)
                break;

            unk = false;

            break;
        }

        case SpawnAppearanceCode:
        {
            struct spawnAppearanceStruct * appearance;

            if (m_logger->isLoggingSpawnAppearance())
                m_logger->logSpawnAppearance((spawnAppearanceStruct *) data,len,dir);

            if (SpawnAppearanceVer != opCodeVersion)
                break;

            unk = false;

            appearance = (struct spawnAppearanceStruct *) (data);
            
            switch (appearance->type)
            {
                case 0x0003:
                {
                    /*
                     ** Invisibility Flag
                     ** 0x00 Visible
                     ** 0x01 Invisible
                    */

#if 0
                    if( m_Spawns.contains(appearance->spawnId) )
                        printf("%s ",m_Spawns[appearance->spawnId].name);
                    else
                        printf("spawn(%d) ", appearance->spawnId);

                    if(appearance->paramter)
                        printf("is now invisible\n");
                    else
                        printf("is now visible\n");
#endif 
                    break;
                }

                case 0x000e:
                {
                    /*
                     ** Shows how to display the other PCs and NPCs
                     ** 0x64 Standing Up
                     ** 0x6e Sitting Down
                    */
 
                    break;
                }

                case 0x0010:
                {
                    /*
                     ** This seems to be where the client is
                     ** assigned the players ID
                    */
		 
                    emit setPlayerID( appearance->paramter );

                    break;
                }

                case 0x0011:
                {
                    /*
                     ** This contains the current HP of the player.
                    */
#if 0
                    printf("Your hp is now %d.\n", 
                           (unsigned short)appearance->paramter );
#endif
                    break;
                }

                case 0x0013:
                {
                    /*
                     ** Not really sure.  Levitation is one of the events 
                     ** seen by this. 0x02 Levitating
                    */

#if 0
                    if ( m_Spawns.contains(appearance->spawnId) )
                        printf("%s ",m_Spawns[appearance->spawnId].name);
                    else
                        printf("spawn(%d) ", appearance->spawnId);

                    if (appearance->paramter & 0x0002)
                        printf("is now levitating\n");
                    else
                        printf("is no longer levitating\n");
#endif
 
                    break;
                }

                case 0x0015:
                {
                    /*
                     ** Used for anonymous and roleplaying
                     ** 0x01 Anonymous
                     ** 0x02 Role Playing
                    */

#if 0
                    if ( m_Spawns.contains(appearance->spawnId) )
                        printf("%s ",m_Spawns[appearance->spawnId].name);
                    else
                        printf("spawn(%d) ", appearance->spawnId);

                    if (appearance->paramter & 0x0002)
                        printf("is now role playing\n");
                    else if(appearance->paramter & 0x0001)
                        printf("is now anonymous\n");
                    else
                        printf("is no longer role playing or anonymous\n");
#endif
 
                    break;
                }
                
                default:
                {
#if 0
                    printf("%0.4x.%0.4x: ",
                           (unsigned short)appearance->type,
                           (unsigned short)appearance->unknown0008);

                    if ( m_Spawns.contains(appearance->spawnId) )
                        printf("%s: ",m_Spawns[appearance->spawnId].name);
                    else
                        printf("spawn(%d): ", appearance->spawnId);

                    printf("%0.8x\n", (unsigned short)appearance->paramter );
#endif
  
                    break;
                }
            }

            break;
        }
            
        case Attack2Code:
        {
            if (m_logger->isLoggingAttack2())
                m_logger->logAttack2((attack2Struct *)data,len,dir);

            if (Attack2Ver != opCodeVersion)
                break;

            unk = false;

            emit attack2Hand1 ((const attack2Struct *)data);

            break;
        }

        case ConsiderCode:
        {
            if (m_logger->isLoggingConsider())
                m_logger->logConsider((considerStruct *)data,len,dir);

            if (ConsiderVer != opCodeVersion)
                break;

            unk = false;

	    ValidatePayload(ConsiderCode, considerStruct);

            if (dir == DIR_CLIENT)
	        emit consRequest((const considerStruct*)data);
            else
                emit consMessage((const considerStruct*)data);

            break;
        }

        case NewGuildInZoneCode:
        {
            if (m_logger->isLoggingNewGuildInZone())
                m_logger->logNewGuildInZone((newGuildInZoneStruct *)data,len,dir);

            if (NewGuildInZoneVer != opCodeVersion)
               break;

            unk = false;

            break;
        }

        case MoneyUpdateCode:
        {  
            if (m_logger->isLoggingMoneyUpdate())
                m_logger->logMoneyUpdate((moneyUpdateStruct *)data,len,dir);

            if (MoneyUpdateVer != opCodeVersion)
               break;

            unk = false;

	    emit moneyUpdate((const moneyUpdateStruct*)data);

            break;
        }

        case MoneyThingCode:
        {
            if (m_logger->isLoggingMoneyThing())
                m_logger->logMoneyThing((moneyThingStruct *)data,len,dir);

            if (MoneyThingVer != opCodeVersion)
                break;

            unk = false;

	    emit moneyThing((const moneyThingStruct*)data);

            break;
        }

        case ClientTargetCode:
        {
            if (m_logger->isLoggingClientTarget())
                m_logger->logClientTarget((clientTargetStruct *)data,len,dir);

            if (ClientTargetVer != opCodeVersion)
                break;

            unk = ! ValidatePayload(ClientTargetCode, clientTargetStruct);

            emit clientTarget((const clientTargetStruct*) data);

            break;
        }

        case BindWoundCode:
        {
            if (m_logger->isLoggingBindWound())
                m_logger->logBindWound((bindWoundStruct *)data,len,dir);

            if (BindWoundVer != opCodeVersion)
                break;

            unk = false;
            
            break;
        }

        case CDoorSpawnsCode:
        {
            if (m_logger->isLoggingCDoorSpawns())
                m_logger->logCDoorSpawns((cDoorSpawnsStruct *)data,len,dir);

            if (CDoorSpawnsVer != opCodeVersion)
                break;

            unk = false;

            if (!decoded)
               break;

#ifdef PACKET_PAYLOAD_SIZE_DIAG
            cDoorSpawnsStruct* compressed;
            compressed = (cDoorSpawnsStruct *)(decodedData);

            // verify size

            int pSize = (decodedDataLen - 4) / compressed->count;

            if (pSize != sizeof(doorStruct))
            {
	        printf("WARNING: CompressedDoorSpawnCode (dataLen:%d "
                       "!= sizeof(doorStruct):%d)!\n", 
                       pSize, sizeof(doorStruct));

	        unk = true;
            }
#endif
            if (m_logger->isLoggingDoorSpawns())
                m_logger->logDoorSpawns((doorSpawnsStruct *) decodedData, decodedDataLen, dir);

            emit compressedDoorSpawn((const cDoorSpawnsStruct *)decodedData);

            break;
        }

        case GroupInfoCode:
        {
            if (m_logger->isLoggingGroupInfo())
                m_logger->logGroupInfo((groupInfoStruct *)data,len,dir);

            if (GroupInfoVer != opCodeVersion)
                break;

            // Too much still unknown.

            unk = ! ValidatePayload(GroupInfoCode, groupMemberStruct);

	    emit groupInfo((const groupMemberStruct*)data);

	    break;
        }
    } /* end switch(opCode) */
       
    if (unk && (m_logger->isLoggingUnknownOpcode()))
        m_logger->logUnknownOpcode(data,len,dir);

    if (showeq_params->logUnknownZonePackets && unk)
    {
        printf ("%04x - %d (%s)\n", opCode, len, 
	       ((dir == DIR_SERVER) ? 
		"Server --> Client" : "Client --> Server"));

        if (!logData (showeq_params->UnknownZoneLogFilename, len, data))
            emit toggle_log_UnknownData(); //untoggle the GUI checkmark
    }

    unsigned int uiOpCodeIndex = 0;

    if (showeq_params->monitorOpCode_Usage)
    {
        unsigned int uiIndex = 0;

        for (; uiIndex < OPCODE_SLOTS && uiOpCodeIndex == 0; uiIndex ++)
        {
            if (opCode == MonitoredOpCodeList[ uiIndex ][ 0 ])
            {
                if ((MonitoredOpCodeList[ uiIndex ][ 1 ] == dir) || 
                    (MonitoredOpCodeList[ uiIndex ][ 1 ] == 3))
                {
                    if (!unk && MonitoredOpCodeList[ uiIndex ][ 2 ] == 1 || unk)
                        uiOpCodeIndex = uiIndex + 1;
                }
            }
        }
   }

   if (m_viewUnknownData && unk || uiOpCodeIndex > 0)
   {
       if (len == 2)
           printf ("\n%s: %02x version %02x len %i\n\t", 
               uiOpCodeIndex > 0 ? 
                   MonitoredOpCodeAliasList[uiOpCodeIndex - 1].ascii() : 
                   "UNKNOWN", data[0], data[1], len);
       else
       {
           printf ("\n%s: %02x version %02x len %02d [%s] ID:%i\n\t", 
               uiOpCodeIndex > 0 ? 
                   MonitoredOpCodeAliasList[uiOpCodeIndex - 1].ascii() : 
                   "UNKNOWN", data[0], data[1], len, 
                   dir == 2 ? "Server --> Client" : "Client --> Server", 
                   data[3] * 256 + data[2]);

           for (uint32_t a = 0; a < len; a ++)
           {
               if ((data[a] >= 32) && (data[a] <= 126))
                   printf ("%c", data[a]);
               else
                   printf (".");
           }

           printf ("\n");

           for (uint32_t a = 0; a < len; a ++)
           {
               if (data[a] < 32)
                  printf ("%c", data[a] + 95);
               else if (data[a] > 126)
                  printf ("%c", data[a] - 95);
               else if (data[a] > 221)
                  printf ("%c", data[a] - 190);
               else
                  printf ("%c", data[a]);
           }

           printf ("\n\n"); /* Adding an extra line break makes it easier
                                for people trying to decode the OpCodes to
                                tell where the raw data ends and the next
                                message begins...  -Andon */
        }
    }
}

void EQPacket::setViewUnknownData (bool flag)
{
  m_viewUnknownData = flag;
}

int EQPacket::playbackSpeed(void)
{
  return m_vPacket->playbackSpeed();
}

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

    emit resetPacket(m_packetCount);  // this resets the packet average
    emit playbackSpeedChanged(speed);
  }
}

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
         break;

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

void EQPacket::dispatchDecodedCharProfile(const uint8_t* decodedData, 
					  uint32_t decodedDataLen)
{
  ValidateDecodedPayload(CharProfileCode, charProfileStruct);

  if (m_logger->isLoggingCharProfile())
      m_logger->logCharProfile((charProfileStruct *)decodedData,decodedDataLen,0);

  emit backfillPlayer((const charProfileStruct*)decodedData);
}

void EQPacket::dispatchDecodedNewSpawn(const uint8_t* decodedData, 
				       uint32_t decodedDataLen)
{
  if (m_logger->isLoggingNewSpawn())
      m_logger->logNewSpawn((newSpawnStruct *) decodedData, decodedDataLen, 0);

  ValidateDecodedPayload(NewSpawnCode, newSpawnStruct);

  newSpawnStruct* ndata = (newSpawnStruct*)decodedData;
  spawnStruct* sdata = &ndata->spawn;
  emit backfillSpawn(sdata);
}

void EQPacket::dispatchDecodedZoneSpawns(const uint8_t* decodedData, 
					 uint32_t decodedDataLen)
{
  zoneSpawnsStruct* zdata = (struct zoneSpawnsStruct *)(decodedData);
  int zoneSpawnsStructHeaderData = 
    ((uint8_t*)&zdata->spawn[0]) - decodedData;

  int zoneSpawnsStructPayloadCount = 
       (decodedDataLen - zoneSpawnsStructHeaderData) / sizeof(spawnZoneStruct);

#ifdef PACKET_PAYLOAD_SIZE_DIAG
  // validate payload size, decodedDataLen - zoneSpawnsStructHeader should be
  // an exact multiple of the size of spawnZoneStruct
  int remainder = 
    (decodedDataLen - zoneSpawnsStructHeaderData) % sizeof(spawnZoneStruct);
  if (remainder != 0)
  {
    printf("WARNING: Decoded ZoneSpawnsCode ((decodedDataLen:%d - zoneSpawnsStructHeaderData:%d) does not evenly contain %d spawnZoneStruct's:%d (remainder: %d)!\n",
	   decodedDataLen, zoneSpawnsStructHeaderData, 
	   zoneSpawnsStructPayloadCount, sizeof(spawnZoneStruct),
	   remainder);
  }
#endif

  if (m_logger->isLoggingZoneSpawns())
      m_logger->logZoneSpawns(zdata, decodedDataLen, 0);

  spawnStruct* sdata;
 
  for (int j = 0; j < zoneSpawnsStructPayloadCount; j++)
  {
    sdata = &zdata->spawn[j].spawn;
    emit backfillSpawn(sdata);
  }
}

int EQPacket::getEQTimeOfDay( time_t timeConvert, struct timeOfDayStruct *eqTimeOfDay )
{
   /* check to see if we have a reference time to go by. */
   if( m_eqTime.packetReferenceTime == 0 )
      return 0;

   unsigned long diff = timeConvert - m_eqTime.packetReferenceTime;

   /* There are 3 seconds per 1 EQ Minute */
   diff /= 3;

   /* The minutes range from 0 - 59 */
   diff += m_eqTime.zoneInTime.minute;
   eqTimeOfDay->minute = diff % 60;
   diff /= 60;

   // The hours range from 1-24
   // 1 = 1am
   // 2 = 2am
   // ...
   // 23 = 11 pm
   // 24 = 12 am
   //
   // Modify it so that it works from
   // 0-23 for our calculations
   diff += ( m_eqTime.zoneInTime.hour - 1);
   eqTimeOfDay->hour = (diff%24) + 1;
   diff /= 24;

   // The months range from 1-28
   // Modify it so that it works from
   // 0-27 for our calculations
   diff += ( m_eqTime.zoneInTime.day - 1 );
   eqTimeOfDay->day = (diff%28) + 1;
   diff /= 28;

   // The months range from 1-12
   // Modify it so that it works from
   // 0-11 for our calculations
   diff += ( m_eqTime.zoneInTime.month - 1 );
   eqTimeOfDay->month = (diff%12) + 1;
   diff /= 12;

   eqTimeOfDay->year = m_eqTime.zoneInTime.year + diff;

	 return 1;
}

char* getTime(char* pchTime)
{
   time_t clock;
   struct tm* current;
   time(&clock);
   current = localtime(&clock);

   sprintf(pchTime, "%02d.%02d.%02d", current->tm_hour, current->tm_min, current->tm_sec);

   return pchTime;
}

void EQPacket::monitorNextClient()
{
  m_detectingClient = true;
  free((void*)showeq_params->ip);
  showeq_params->ip = strdup(AUTOMATIC_CLIENT_IP);
  struct in_addr  ia;
  inet_aton (showeq_params->ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);
  printf("Listening for next client seen. (you must zone for this to work!)\n");

  emit (toggle_session_tracking());
  if (!showeq_params->playbackpackets)
    m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
			       showeq_params->realtime, 
			       DEFAULT_ADDRESS_TYPE, 0, 0);
}

void EQPacket::session_tracking()
{
   m_session_tracking_enabled = showeq_params->session_tracking;
   emit sessionTrackingChanged(m_session_tracking_enabled);
}
   
//----------------------------------------------------------------------
// PacketCaptureThread
//  start and stop the thread
//  get packets to the processing engine(decodePacket)
PacketCaptureThread::PacketCaptureThread()
{


}

PacketCaptureThread::~PacketCaptureThread()
{
}

void PacketCaptureThread::start(const char *device, const char *host, bool realtime, uint8_t address_type)
{

    char ebuf[256]; // pcap error buffer
    char filter_buf[256]; // pcap filter buffer 
    struct bpf_program bpp;
    struct sched_param sp;

   printf ("Initializing Packet Capture Thread: \n");

   // create pcap style filter expressions
   if (address_type == IP_ADDRESS_TYPE)
   {
      if (strcmp(host, AUTOMATIC_CLIENT_IP) == 0)
      {
          printf ("Filtering packets on device %s, searching for EQ client...\n", device);
          sprintf (filter_buf, "(udp[0:2] > 1024 or udp[2:2] > 1024) and ether proto 0x0800");
      }
      else
      {
          printf ("Filtering packets on device %s, IP host %s\n", device, host);
          sprintf (filter_buf, "(udp[0:2] > 1024 or udp[2:2] > 1024) and host %s and ether proto 0x0800", host);
      }
   }

   else if (address_type == MAC_ADDRESS_TYPE)
   {
      printf ("Filtering packets on device %s, MAC host %s\n", device, host);
      sprintf (filter_buf, "(udp[0:2] > 1024 or udp[2:2] > 1024) and ether host %s and ether proto 0x0800", host);
   }

   else
   {
      fprintf (stderr, "pcap_error:filter_string: unknown address_type (%d)\n", address_type);
      exit(0);
   }

   // initialize the pcap object 
   m_pcache_pcap = pcap_open_live((char *) device, 1500, true, 100, ebuf);

   if (!m_pcache_pcap)
   {
     fprintf(stderr, "pcap_error:pcap_open_live(%s): %s\n", device, ebuf);
     if ((getuid() != 0) && (geteuid() != 0))
       fprintf(stderr, "Make sure you are running ShowEQ as root.\n");
     exit(0);
   }

   setuid(getuid()); // give up root access if running suid root

   if (pcap_compile(m_pcache_pcap, &bpp, filter_buf, 1, 0) == -1)
   {
      pcap_perror (m_pcache_pcap, "pcap_error:pcap_compile");
      exit(0);
   }

   if (pcap_setfilter (m_pcache_pcap, &bpp) == -1)
   {
      pcap_perror (m_pcache_pcap, "pcap_error:pcap_setfilter");
      exit(0);
   }

   m_pcache_first = m_pcache_last = NULL;

   pthread_mutex_init (&m_pcache_mutex, NULL);
   pthread_create (&m_tid, NULL, loop, (void*)this);

   if (realtime)
   {
      memset (&sp, 0, sizeof (sp));
      sp.sched_priority = 1;
      if (pthread_setschedparam (m_tid, SCHED_RR, &sp) != 0)
         fprintf (stderr, "Failed to set capture thread realtime.");
   }
}

void* PacketCaptureThread::loop (void *param)
{
    PacketCaptureThread* myThis = (PacketCaptureThread*)param;
    pcap_loop (myThis->m_pcache_pcap, -1, packetCallBack, (u_char*)param);
    return NULL;
}

void PacketCaptureThread::packetCallBack(u_char * param, 
					 const struct pcap_pkthdr *ph,
					 const u_char *data)
{
    struct packetCache *pc;
    PacketCaptureThread* myThis = (PacketCaptureThread*)param;
    pc = (struct packetCache *) malloc (sizeof (struct packetCache) + ph->len);
    pc->len = ph->len;
    memcpy (pc->data, data, ph->len);
    pc->next = NULL;

    pthread_mutex_lock (&myThis->m_pcache_mutex);

    if (myThis->m_pcache_last)
       myThis->m_pcache_last->next = pc;

    myThis->m_pcache_last = pc;

    if (!myThis->m_pcache_first)
       myThis->m_pcache_first = pc;

    pthread_mutex_unlock (&myThis->m_pcache_mutex);
}

uint16_t PacketCaptureThread::getPacket(unsigned char *buff)
{
    uint16_t ret;
    struct packetCache *pc = NULL;

    pthread_mutex_lock (&m_pcache_mutex);

    ret = 0;

    pc = m_pcache_first;

    if (pc)
    {
        m_pcache_first = pc->next;

        if (!m_pcache_first)
           m_pcache_last = NULL;
    }

    pthread_mutex_unlock (&m_pcache_mutex);

    if (pc)
    {
       ret = pc->len;
       memcpy (buff, pc->data, ret);
       free (pc);
    }

    return ret;
}

void PacketCaptureThread::setFilter (const char *device,
                                     const char *hostname,
                                     bool realtime,
                                     uint8_t address_type,
                                     uint16_t zone_server_port,
				     uint16_t client_port
                                    )
{
    char filter_buf[256]; // pcap filter buffer 
    struct bpf_program bpp;
    struct sched_param sp;

    /* Listen to World Server or the specified Zone Server */
    if (address_type == IP_ADDRESS_TYPE && client_port)   
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[0:2] = %d or udp[2:2] = %d) and host %s and ether proto 0x0800", client_port, client_port, hostname);
    else if (address_type == IP_ADDRESS_TYPE && !client_port) 
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[0:2] = %d or udp[2:2] = %d) and host %s and ether proto 0x0800", zone_server_port, zone_server_port, hostname);
    else if (address_type == MAC_ADDRESS_TYPE && client_port)
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[0:2] = %d or udp[2:2] = %d) and ether host %s and ether proto 0x0800", client_port, client_port, hostname);
    else if (address_type == MAC_ADDRESS_TYPE && !client_port)
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[0:2] = %d or udp[2:2] = %d) and ether host %s and ether proto 0x0800", zone_server_port, zone_server_port, hostname);
    else
    {
          printf ("Filtering packets on device %s, searching for EQ client...\n", device);
          sprintf (filter_buf, "(udp[0:2] > 1024 or udp[2:2] > 1024) and ether proto 0x0800");
    }

    if (pcap_compile (m_pcache_pcap, &bpp, filter_buf, 1, 0) == -1)
    {
        printf("%s\n",filter_buf);
	pcap_perror(m_pcache_pcap, "pcap_error:pcap_compile_error");
        exit (0);
    }

    if (pcap_setfilter (m_pcache_pcap, &bpp) == -1)
    {
        pcap_perror(m_pcache_pcap, "pcap_error:pcap_setfilter_error");
        exit (0);
    }

    if (realtime)
    {
       memset (&sp, 0, sizeof (sp));
       sp.sched_priority = 1;
       if (pthread_setschedparam (m_tid, SCHED_RR, &sp) != 0)
           fprintf (stderr, "Failed to set capture thread realtime.");
    }
}

