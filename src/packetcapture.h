/*
 * packetcapture.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _PACKETCAPTURE_H_
#define _PACKETCAPTURE_H_

#include <pthread.h>

#ifdef __FreeBSD__
#include <sys/ioccom.h>
#endif

extern "C" { // fix for bpf not being c++ happy
#include <pcap.h>
}

#include <qstring.h>

#include "packetcommon.h"


//----------------------------------------------------------------------
// constants
// Used in the packet capture filter setup.  If address_type is
//   MAC_ADDRESS_TYPE, then we use the hostname string as a MAC address
// for the filter. cpphack
const uint8_t DEFAULT_ADDRESS_TYPE = 10;   /* These were chosen arbitrarily */
const uint8_t IP_ADDRESS_TYPE = 11;
const uint8_t MAC_ADDRESS_TYPE =  12;

//----------------------------------------------------------------------
// PacketCaptureThread
class PacketCaptureThread
{
 public:
         PacketCaptureThread();
         ~PacketCaptureThread();
         void start (const char *device, const char *host, bool realtime, uint8_t address_type);
	 void stop ();
         uint16_t getPacket (unsigned char *buff); 
         void setFilter (const char *device, const char *hostname, bool realtime,
                        uint8_t address_type, uint16_t zone_server_port, uint16_t client_port);
	 const QString getFilter();
         
 private:
         static void* loop(void *param);
         static void packetCallBack(u_char * param, const struct pcap_pkthdr *ph, const u_char *data);

         struct packetCache 
	 {
           struct packetCache *next;
           ssize_t len;
           unsigned char data[0];
         };
         struct packetCache *m_pcache_first;
         struct packetCache *m_pcache_last;

	 pthread_t m_tid;
         pthread_mutex_t m_pcache_mutex;

         pcap_t *m_pcache_pcap;

	 QString m_pcapFilter;
};

#endif // _PACKETCAPTURE_H_
