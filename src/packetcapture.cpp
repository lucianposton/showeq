/*
 * packetcapture.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

/* Implementation of Packet class */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "packetcapture.h"
#include "diagnosticmessages.h"
  
//----------------------------------------------------------------------
// PacketCaptureThread
//  start and stop the thread
//  get packets to the processing engine(dispatchPacket)
PacketCaptureThread::PacketCaptureThread() :
    m_playbackSpeed(0)
{
}

PacketCaptureThread::~PacketCaptureThread()
{
  // Drop the packets we have lying around
  pthread_mutex_lock (&m_pcache_mutex);

  struct packetCache *pc = m_pcache_first;
  struct packetCache* freeMe = NULL;

  while (pc)
  {
    freeMe = pc;
    pc = pc->next;

    free(freeMe);
  }

  m_pcache_first = NULL;
  m_pcache_last = NULL;
  m_pcache_closed = true;

  pthread_mutex_unlock (&m_pcache_mutex);
}

void PacketCaptureThread::setPlaybackSpeed(int playbackSpeed)
{
    if (playbackSpeed < -1)
    {
        m_playbackSpeed = -1;
    }
    else if (playbackSpeed > 9)
    {
        m_playbackSpeed = 9;
    }
    else if (playbackSpeed == 0)
    {
        // Fast as possible. But using 0 makes the UI unresponsive!
        m_playbackSpeed = 100;
    }
    else
    {
        m_playbackSpeed = playbackSpeed;
    }
}

void PacketCaptureThread::start(const char *device, const char *host, bool realtime, uint8_t address_type)
{

    char ebuf[256]; // pcap error buffer
    char filter_buf[256]; // pcap filter buffer 
    struct bpf_program bpp;
    struct sched_param sp;

    seqInfo("Initializing Packet Capture Thread: ");
    m_pcache_closed = false;

   // create pcap style filter expressions
   if (address_type == IP_ADDRESS_TYPE)
   {
      if (strcmp(host, AUTOMATIC_CLIENT_IP) == 0)
      {
	seqInfo("Filtering packets on device %s, searching for EQ client...", device);
	sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether proto 0x0800");
      }
      else
      {
	seqInfo("Filtering packets on device %s, IP host %s", device, host);
	sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and host %s and ether proto 0x0800", host);
      }
   }

   else if (address_type == MAC_ADDRESS_TYPE)
   {
     seqInfo("Filtering packets on device %s, MAC host %s", device, host);
     sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether host %s and ether proto 0x0800", host);
   }

   else
   {
     seqFatal("pcap_error:filter_string: unknown address_type (%d)", address_type);
     exit(0);
   }

   /* A word about pcap_open_live() from the docs
   ** to_ms specifies the read timeout in milliseconds.   The
   ** read timeout is used to arrange that the read not necessarily
   ** return immediately when a packet is seen, but that it wait
   ** for  some amount of time to allow more packets to arrive and 
   ** to read multiple packets from the OS kernel in one operation.
   ** Not all  platforms  support  a read timeout; on platforms that
   ** don't, the read timeout is ignored.
   ** 
   ** In Linux 2.4.x with the to_ms set to 0 we get packets immediatly,
   ** and thats what we need in this application, so don't change it!! 
   ** 
   ** a race condition exists between this thread and the main thread 
   ** any artificial delay in getting packets can cause filtering problems
   ** and cause us to miss new stream when the player zones.
   */
   // initialize the pcap object 
   m_pcache_pcap = pcap_open_live((char *) device, BUFSIZ, true, 0, ebuf);
#ifdef __FreeBSD__
   // if we're on FreeBSD, we need to call ioctl on the file descriptor
   // with BIOCIMMEDIATE to get the kernel Berkeley Packet Filter device
   // to return packets to us immediately, rather than holding them in
   // it's internal buffer... if we don't do this, we end up getting 32K
   // worth of packets all at once, at long intervals -- if someone
   // knows a less hacky way of doing this, I'd love to hear about it.
   // the problem here is that libpcap doesn't expose an API to do this
   // in any way
   int fd = *((int*)m_pcache_pcap);
   int temp = 1;
   if ( ioctl( fd, BIOCIMMEDIATE, &temp ) < 0 )
     seqWarn("PCAP couldn't set immediate mode on BSD" );
#endif
   if (!m_pcache_pcap)
   {
     seqWarn("pcap_error:pcap_open_live(%s): %s", device, ebuf);
     if ((getuid() != 0) && (geteuid() != 0))
       seqWarn("Make sure you are running ShowEQ as root.");
     exit(0);
   }

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
         seqWarn("Failed to set capture thread realtime.");
   }
}

//------------------------------------------------------------------------
// Capture thread for offline packet capture. Input filename should be a
// tcpdump file. playbackSpeed is how fast to playback. 1 = realtime,
// 2 = 2x speed, 3 = 3x speed, etc. 0 = no throttle. -1 = paused.
//
void PacketCaptureThread::startOffline(const char* filename, int playbackSpeed)
{
    char ebuf[256]; // pcap error buffer

    seqInfo("Initializing Offline Packet Capture Thread: ");
    m_pcache_closed = false;

    // initialize the pcap object 
    m_pcache_pcap = pcap_open_offline(filename, ebuf);

    if (!m_pcache_pcap)
    {
        seqWarn("pcap_error:pcap_open_offline(%s): %s", filename, ebuf);
        exit(0);
    }

    // Set the speed
    setPlaybackSpeed(playbackSpeed);

    m_tvLastProcessedActual.tv_sec = 0;
    m_tvLastProcessedOriginal.tv_sec = 0;

    m_pcache_first = m_pcache_last = NULL;

    pthread_mutex_init(&m_pcache_mutex, NULL);
    pthread_create(&m_tid, NULL, loop, (void*)this);
}

void PacketCaptureThread::stop()
{
  // close the pcap session
  pcap_close(m_pcache_pcap);
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

    // Throttle offline playback properly if applicable.
    int speed = myThis->m_playbackSpeed;

    if (speed != 0)
    {
        if (speed == -1)
        {
            // We are paused. Need to wait for it to unpause.
            while ((speed = myThis->m_playbackSpeed) == -1)
            {
                sleep(1);
            }
        }

        // Playing back from a file. Need to honor playback speed and packet
        // timestamps properly.
        timeval now;

        if (gettimeofday(&now, NULL) == 0)
        {
            // Anchor the first run through.
            if (myThis->m_tvLastProcessedActual.tv_sec == 0)
            {
                myThis->m_tvLastProcessedActual = now;
            }
            if (myThis->m_tvLastProcessedOriginal.tv_sec == 0)
            {
                myThis->m_tvLastProcessedOriginal = ph->ts;
            }

            // The goal here is to make sure that time elapsed since last
            // packet / playbackSpeed > time elapsed between original
            // previous packet and this packet. If it is not, we need to sleep
            // for the difference.
            long usecDiffActual = 
                ((now.tv_sec - myThis->m_tvLastProcessedActual.tv_sec)*1000000 +
                (now.tv_usec - myThis->m_tvLastProcessedActual.tv_usec));
            long usecDiffOriginal =
                ((ph->ts.tv_sec - myThis->m_tvLastProcessedOriginal.tv_sec)*1000000 +
                (ph->ts.tv_usec - myThis->m_tvLastProcessedOriginal.tv_usec)) /
                    ((long) speed);

            if (usecDiffActual < usecDiffOriginal)
            {
                // Need to wait out the difference.
                timeval tvWait;
                
                tvWait.tv_usec = (usecDiffOriginal - usecDiffActual) % 1000000;
                tvWait.tv_sec = (usecDiffOriginal - usecDiffActual) / 1000000;

                select(1, NULL, NULL, NULL, &tvWait);
            }

            // And get ready for next one
            myThis->m_tvLastProcessedActual = now;
        }
    }

    myThis->m_tvLastProcessedOriginal = ph->ts;

    pthread_mutex_lock (&myThis->m_pcache_mutex);
   
    if (! myThis->m_pcache_closed)
    {
      if (myThis->m_pcache_last)
       myThis->m_pcache_last->next = pc;

      myThis->m_pcache_last = pc;

      if (!myThis->m_pcache_first)
       myThis->m_pcache_first = pc;
    }
    else
    {
      free(pc);
    }

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
                                     uint16_t zone_port,
				     uint16_t client_port
                                    )
{
    char filter_buf[256]; // pcap filter buffer 
    struct bpf_program bpp;
    struct sched_param sp;

    /* Listen to World Server or the specified Zone Server */
    if (address_type == IP_ADDRESS_TYPE && client_port)   
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and host %s and ether proto 0x0800", client_port, client_port, hostname);
    else if (address_type == IP_ADDRESS_TYPE && zone_port) 
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and host %s and ether proto 0x0800", zone_port, zone_port, hostname);
    else if (address_type == MAC_ADDRESS_TYPE && client_port)
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and ether host %s and ether proto 0x0800", client_port, client_port, hostname);
    else if (address_type == MAC_ADDRESS_TYPE && zone_port)
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and ether host %s and ether proto 0x0800", zone_port, zone_port, hostname);
    else if (hostname != NULL && !client_port && !zone_port)
         sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether proto 0x0800 and host %s", hostname);
    else
    {
      seqInfo("Filtering packets on device %s, searching for EQ client...", device);
      sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether proto 0x0800");
    }

    if (pcap_compile (m_pcache_pcap, &bpp, filter_buf, 1, 0) == -1)
    {
      seqWarn("%s",filter_buf);
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
	 seqWarn("Failed to set capture thread realtime.");
    }

    m_pcapFilter = filter_buf;
}

const QString PacketCaptureThread::getFilter()
{
  return m_pcapFilter;
}

