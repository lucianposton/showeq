/*
 * pcap.c
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>

#ifdef	__FreeBSD__
#include <netinet/in_systm.h>   
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>  
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#else	/* !FREEBSD */
#include <net/if.h>
#include <net/if_packet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#endif	/* FREEBSD */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <pcap.h>
#include <pthread.h>

/* For some definitions common between this code and external code.
   cpphack
*/
#include "pcap_local.h"

struct packetCache
{
  struct packetCache *next;
  ssize_t len;
  unsigned char data[0];
};

pthread_mutex_t pcache_mutex;
struct packetCache *pcache_first;
struct packetCache *pcache_last;
pcap_t *pcache_pcap;

void EQPacketWipeBuffer(void)
{
    struct packetCache *pc = NULL;
    pthread_mutex_lock(&pcache_mutex);

    while(pcache_first)
    {
      pc = pcache_first;
      pcache_first = pc->next;
      if (!pcache_first)
         pcache_last = NULL;
      free (pc);
    }
    pthread_mutex_unlock (&pcache_mutex);
}



unsigned int EQPacketGetPacket (unsigned char *buff)
{
    int ret;
    struct packetCache *pc = NULL;

    pthread_mutex_lock (&pcache_mutex);

    ret = 0;

    pc = pcache_first;

    if (pc)
    {
        pcache_first = pc->next;

        if (!pcache_first)
           pcache_last = NULL;
    }

    pthread_mutex_unlock (&pcache_mutex);

    if (pc)
    {
       ret = pc->len;
       memcpy (buff, pc->data, ret);
       free (pc);
    }

    return ret;
}

void EQPacketCallback (u_char * param, const struct pcap_pkthdr *ph, const u_char * data)
{
    struct packetCache *pc;

    pc = (struct packetCache *) malloc (sizeof (struct packetCache) + ph->len);
    pc->len = ph->len;
    memcpy (pc->data, data, ph->len);
    pc->next = NULL;

    pthread_mutex_lock (&pcache_mutex);

    if (pcache_last)
       pcache_last->next = pc;

    pcache_last = pc;

    if (!pcache_first)
       pcache_first = pc;

    pthread_mutex_unlock (&pcache_mutex);
}


void *EQPacketThread (void *param)
{
  pcap_loop (pcache_pcap, -1, EQPacketCallback, NULL);
}


void EQPacketInitPcap (const char *device, const char *hostname, int realtime, int address_type)
{
    char buff[4096];
    struct bpf_program bpp;
    pthread_t tid;
    struct sched_param sp;

   if (address_type == MAC_ADDRESS_TYPE)
   {
      printf ("Opening packet capture on device %s, MAC host %s\n", device, hostname);
   }
   else if (address_type == IP_ADDRESS_TYPE)
   {
      printf ("Opening packet capture on device %s, IP host %s\n", device, hostname);
   }
   else
   {
      fprintf (stderr, "Internal error: strange address_type value (%d)\n", address_type);
      exit(0); /* Probably should be exit(1), but everyone else seems to use 0 */
   }

   pcache_pcap = pcap_open_live ((char *) device, 1500, 0x0100, 0, buff);

   if (!pcache_pcap)
   {
      fprintf (stderr, "PCAP failed to open device %s: %s\n", device, buff);
      exit (0);
   }

    /* The string buffer "buff" is the rule compiled by pcap_compile to tell
       pcap how to select packets.  The documentation on filters supplied
       with pcap is awful, however tcpdump uses the same pcap interface and
       its filters are well documented. cpphack */
  
   if (address_type == MAC_ADDRESS_TYPE) {
      /* New MAC-based filtering.  MAC must be in 00:00:00:00:00:00 format.
         Had to use address_type since this module has no access to the 
         showeq_params structure.  cpphack */
      sprintf (buff, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether host %s", hostname);
  
   }
   else
   {
      /* Modified to auto-detect IP.  Listen for udp dest port 9000 to detect
         an active EQ session address.  Not perfect, but pretty solid.  May
         get false positives if some other prog is talking to port 9000. */
      sprintf (buff, "udp[2:2] = 9000 or (udp[0:2] > 1024 and udp[2:2] > 1024 and host %s)", hostname);
   }

   if (pcap_compile (pcache_pcap, &bpp, buff, 1, 0) == -1)
   {
      fprintf (stderr,"PCAP failed to compile filter for device %s: %s\n", device, pcap_geterr (pcache_pcap));
      exit (0);
   }

   if (pcap_setfilter (pcache_pcap, &bpp) == -1)
   {
      fprintf (stderr, "PCAP failed to apply filter on device %s: %s\n", device, pcap_geterr (pcache_pcap));
      exit (0);
   }

   pcache_first = pcache_last = NULL;

   pthread_mutex_init (&pcache_mutex, NULL);
   pthread_create (&tid, NULL, EQPacketThread, NULL);

   if (realtime)
   {
      memset (&sp, 0, sizeof (sp));
      sp.sched_priority = 1;
      if (pthread_setschedparam (tid, SCHED_RR, &sp) != 0)
         fprintf (stderr, "Failed to set capture thread realtime.");
   }
}


void EQPacketRefilter (const char *device, const char *hostname, int realtime, int address_type, unsigned int zoneServ_port)
{
    char buff[4096];
    struct bpf_program bpp;
    pthread_t tid;
    struct sched_param sp;

    /* Listen to World Server or the specified Zone Server */
    sprintf (buff, "(udp[0:2] = 9000 or udp[0:2] = %d or udp[2:2] = %d) and host %s", zoneServ_port, zoneServ_port, hostname);

    if (pcap_compile (pcache_pcap, &bpp, buff, 1, 0) == -1)
    {
       fprintf (stderr, "PCAP failed to compile filter for device %s: %s\n", device, pcap_geterr (pcache_pcap));
       exit (0);
    }

    if (pcap_setfilter (pcache_pcap, &bpp) == -1)
    {
       fprintf (stderr, "PCAP failed to apply filter on device %s: %s\n", device, pcap_geterr (pcache_pcap));
       exit (0);
    }

    if (realtime)
    {
       memset (&sp, 0, sizeof (sp));
       sp.sched_priority = 1;
       if (pthread_setschedparam (tid, SCHED_RR, &sp) != 0)
	  fprintf (stderr, "Failed to set capture thread realtime.");
    }
}
