/*
 * vpacket.cpp
 *
 * ShowEQ Distributed under GPL
 * http://www.hackersquest.gomp.ch/
 */

/*
 * Orig Author - Maerlyn (MaerlynTheWiz@yahoo.com)
 * Date   - 3/11/00
 */

/*
 * VPacket
 *
 * VPacket is a generic class that allows recording and playback of
 * generic data with timestamps.  The data is stored in binary format
 * with headers around it so that it is retrieved in the same manner
 * (exact same 'chunks') that it was sent.  This makes a good class for
 * a packet capture
 *
 * The following public member functions are available:
 *
 * Plaback(...)                Fetch data from the buffer
 * Record(...)                 Record data to the buffer
 * Flush()                     Force a flush of data to the file
 * setPlaybackSpeed()          Set a playback rate (0=not timed, 1=1X, etc)
 * playbackSpeed()             Get the playback rate
 * EndOfData()                 Check for out of data
 *
 *
 * The intention of this class was to capture network packets to play back
 * at a later time.  Using without a file is untested, as well as using to
 * Fetch and Record simultaneously.  These features may not be implemented
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "vpacket.h"


//#define DEBUG_VPACKET

#undef DEBUG_VPACKET


//
// VPacket constructor
//
VPacket::VPacket(const char *name, 
		 int nPBSpeed, 
		 bool bRecord, 
		 int nBufSize)
{
   m_sFile = 0;
   m_fd = -1;
   m_nBufIndex = 0;
   m_nBufBytes = 0;
   m_nBufSize = nBufSize;
   m_nPlaybackSpeed = nPBSpeed;
   m_lStartTime = 0;
   m_nSequence = 0;
   m_bEndofFile = 0;
   m_lBytesIO = 0;
   m_bFlushPacket = false;
   m_nLastPacketTime = 0;
   m_nFirstPacketTime = 0;
   m_nLastTime = 0;
   m_nCompressTime = 0;
   m_bRecord = bRecord;

   // allocate buffer
   m_cBuffer = (char *) malloc(nBufSize);

   // open file
   if (name)
   {
     m_sFile = strdup(name);

     // Open file
     if (m_bRecord)
        m_fd = open(m_sFile, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
     else
        m_fd = open(m_sFile, O_RDONLY); 

     if (-1 == m_fd)
     {
       switch(errno)
       {
         case EEXIST:
           fprintf(stderr, "Error opening file '%s' - ", m_sFile);
           fprintf(stderr, "already exists.\n");
           break;
         case EISDIR:
           fprintf(stderr, "Error opening file '%s' - ", m_sFile);
           fprintf(stderr, "is a directory\n");
           break;
         case EACCES:
           fprintf(stderr, "Error opening file '%s' - ", m_sFile);
           fprintf(stderr, "permission denied\n");
           break;
         default:
           fprintf(stderr, "Error opening file '%s' - ", m_sFile);
           fprintf(stderr, "%d '%s'\n", errno, strerror(errno));
           break;
       } // end if error

       /* Seems like errors opening up a playback file ought to be fatal
        since the user can simply correct the error and try again. 
        cpphack */
       exit(1);

     } // end if file open ok
   } // end if filename

} // end constructor


//
// VPacket destructor
//
VPacket::~VPacket(void)
{
  if (-1 != m_fd)
    close(m_fd);
  if (m_sFile)
    free(m_sFile);
  if (m_cBuffer)
    free(m_cBuffer);
}


//
// Record
//
// Record a data buffer with timestamp
// If our internal buffer is full, make call to flush to file
//
int
VPacket::Record(const char *buff, int packetsize, time_t time, long version)
{
  int size;
  int bufsize;
  int headersize = sizeof(struct packet_struct);
  struct packet_struct *packet;

  bufsize = packetsize + sizeof(struct packet_struct);

  if (!m_cBuffer)
     return 0;

  // Make sure we can handle the packet
  if (bufsize > m_nBufSize)
  {
    fprintf(stderr, "VPacket::RecordPacket() - Buffer too small for %d byte packet (max %dbytes)\n", bufsize, m_nBufSize);
    return 0;
  }

  // if we need to write the buffer to make space, do so
  if ( (m_nBufSize - m_nBufIndex) < bufsize)
  {
     if (0 == writeBuffer())
     {
       fprintf(stderr, "Record() - writeBuffer Failed\n");
       return 0;
     }
  }
   
  // if we still don't have enough space error
  if ( (m_nBufSize - m_nBufIndex) < bufsize)
  {
     fprintf(stderr, "Record() - cant allocate enough buffer space\n");
     return 0;
  }
   
  // keep track of time
  if (!m_lStartTime)
    m_lStartTime = mTime();

  packet = (struct packet_struct *) (m_cBuffer + m_nBufIndex);
  packet->size = packetsize + headersize;
  packet->ms = mTime() - m_lStartTime;
#ifdef USEVERSION
  packet->version = version;
#endif
  packet->sequence = m_nSequence;
  memcpy(&packet->time, &time, sizeof(time_t));
  m_nBufIndex += headersize;
  memcpy(m_cBuffer + m_nBufIndex, buff, packetsize);
  m_nBufIndex += packetsize;
  m_nSequence++;

  size = packetsize;

#ifdef DEBUG_VPACKET
printf("Record: %06d:%06d: %04d bytes: 0x %02x%02x%02x%02x ... %02x%02x%02x%02x\n", 
  packet->sequence, packet->ms, size, 
  (unsigned char) packet->buffer[0], 
  (unsigned char) packet->buffer[1],
  (unsigned char) packet->buffer[2], 
  (unsigned char) packet->buffer[3],
  (unsigned char) packet->buffer[size - 4], 
  (unsigned char) packet->buffer[size - 3],
  (unsigned char) packet->buffer[size - 2], 
  (unsigned char) packet->buffer[size - 1]);
#endif

  // flushes every packet
  if (m_bFlushPacket)
     writeBuffer();

  return packetsize;

} // end Record


//
// Playback 
//
// Retrieve a data buffer with timestamp
// If our internal buffer empty, make call to read from file
//
int
VPacket::Playback(char *buff, int bufsize, time_t *time, long *version)
{
  int size = 0;
  int headersize = sizeof(struct packet_struct);
  struct packet_struct *packet;

  // if we don't have a buffer or we are paused (speed < 0) return
  if (!m_cBuffer || (m_nPlaybackSpeed < 0))
     return 0;

  // if we don't have enough data in buffer fill it
  if (m_nBufBytes <= headersize)
    fillBuffer();

  // if we still don't have enough data return 0 (probable eof)
  if (m_nBufBytes <= headersize)
    return 0;

  packet = (struct packet_struct *) (m_cBuffer + m_nBufIndex);

  // if packet isn't completely in buffer fill buff
  if (packet->size > m_nBufBytes)
  {
    if (fillBuffer())
      packet = (struct packet_struct *) (m_cBuffer + m_nBufIndex);
  }

  // if packet still isn't completely in buffer return 0  (probable eof)
  if (packet->size > m_nBufBytes)
    return 0;

  // start timer
  if (!m_nFirstPacketTime || !m_nLastPacketTime)
  {
//printf("Got first packet time of %d\n", m_nLastPacketTime);
    m_nLastPacketTime = packet->ms;
    m_nFirstPacketTime = packet->ms;
  }

  // keep track of time
  if (!m_lStartTime)
  {
    m_lStartTime = mTime() + m_nLastPacketTime;
//printf("Got start time of %d\n", m_lStartTime);
  }

  if (!m_nLastTime)
     m_nLastTime = mTime();

  // ok now we have an entire packet in the static buffer

  //  if  next time for packet < time since last packet
  int pktDelta = packet->ms - m_nLastPacketTime;
//  int pktDelta = (packet->ms - m_nFirstPacketTime) - m_nLastPacketTime;
  int delta = mTime() - m_nLastTime;
  if (m_nPlaybackSpeed != 0)
  {
    if ( (pktDelta / m_nPlaybackSpeed) > delta)
    {
//printf("waiting\n");
       if (m_nCompressTime && (pktDelta > m_nCompressTime))
       {
//printf("dropped timestamp on delta %d\n", pktDelta);
       }
       else 
         return 0;
    }
  }
//printf("Packet delta %4d, time delta %4d, speed %d, compress %d\n", 
//          pktDelta / m_nPlaybackSpeed,
//          delta, m_nPlaybackSpeed, m_nCompressTime);

//printf("Time %d, next %d - %d\n", mTime() - m_lStartTime, packet->ms, m_lStartTime);
  if (packet->size < headersize)
  {
    fprintf(stderr, 
	    "Playback(): File corrupt! Aborting playback!\n" 
	    "\tPacket size %d < packet header size %d\n",
	    packet->size, headersize);
    m_bEndofFile = 1;
    
    return 0;
  }

  size = packet->size - headersize;
  m_nLastPacketTime = packet->ms;
  m_nLastTime = mTime();

  // if the passed buffer is too small return 0
  if (bufsize < size)
  {
    fprintf(stderr, "Playback() - Buffer too small for packet\n");
    return 0;
  }

  memcpy(buff, packet->buffer, size); 
  memcpy(time, (void *) &packet->time, sizeof(time_t));
#ifdef USEVERSION
  if (version)
     *version = packet->version;
#endif

  // Advance buffer past this packet
  m_nBufIndex += (size + headersize);
  m_nBufBytes -= (size + headersize);
  m_nSequence++;

#ifdef DEBUG_VPACKET
printf("Playback: T%06d: S%06d: Pt%06d: %04d bytes: 0x %02x%02x%02x%02x ... %02x%02x%02x%02x\n", 
  mTime() - m_lStartTime, packet->sequence, packet->ms, size, 
  (unsigned char) buff[0], 
  (unsigned char) buff[1],
  (unsigned char) buff[2], 
  (unsigned char) buff[3],
  (unsigned char) buff[size - 4], 
  (unsigned char) buff[size - 3],
  (unsigned char) buff[size - 2], 
  (unsigned char) buff[size - 1]);
#endif

  return size;
     
} // end Playback 


//
// fillBuffer
//
// read from disk file into static buffer
// returns num of bytes available in buffer
//
int
VPacket::fillBuffer(void)
{
  int size;

  if (!m_cBuffer)
     return 0;

  // if no file or we know were at EOF return
  if (m_fd == -1)
     m_bEndofFile = 1;
  if (m_bEndofFile)
    return 0;

  // Move current data to beginning of buffer
  memmove(m_cBuffer, m_cBuffer + m_nBufIndex, m_nBufBytes);
  m_nBufIndex = 0;

  // read
  size = read(m_fd, m_cBuffer + m_nBufBytes, m_nBufSize - m_nBufBytes);

  if (size != (m_nBufSize - m_nBufBytes))
  {
     m_bEndofFile = 1;
     fprintf(stderr, "VPacket: short read from file '%s' (%d of %d bytes)\n", m_sFile, size, (m_nBufSize - m_nBufBytes));
  }

  if (-1 == size)
  {
    switch(errno)
    {
       case ENOSPC:
         fprintf(stderr, "Error reading from file '%s' - ", m_sFile);
         fprintf(stderr, "Disk full\n");
         break;
       default:
         fprintf(stderr, "Error reading from file '%s' - ", m_sFile);
         fprintf(stderr, "%d '%s'\n", errno, strerror(errno));
         break;
    }
  }
  else
    m_nBufBytes += size;

  m_lBytesIO += size;

#ifdef DEBUG_VPACKET
  printf("fillBuffer: %06d - %d bytes read, %d bytes in buffer, EOF=%d\n", 
    m_lBytesIO, size, m_nBufBytes, m_bEndofFile);
#endif

  return size;

} // end fillBuffer 


//
// writeBuffer
//
// write static buffer to disk file
// returns num of bytes written to disk 
//
int
VPacket::writeBuffer(void)
{
  int size;

  if (!m_cBuffer)
     return 0;

  // if no file return
  if (m_fd == -1)
    return 0;

  // if not recording, return
  if (!m_bRecord)
    return 0;

  // write 
  size = write(m_fd, m_cBuffer, m_nBufIndex);

  if (size != m_nBufIndex)
  {
    switch(errno)
    {
       case ENOSPC:
         fprintf(stderr, "Error writing to file '%s' - ", m_sFile);
         fprintf(stderr, "Disk full\n");
         break;
       default:
         fprintf(stderr, "Error writing to file '%s' - ", m_sFile);
         fprintf(stderr, "%d '%s'\n", errno, strerror(errno));
         break;
    }
  }

  // write is complete, adjust buffer

  // Move current data to beginning of buffer
  m_nBufIndex = 0;
  m_lBytesIO += size;

#ifdef DEBUG_VPACKET
  printf("writeBuffer: %06d - %d bytes written\n", m_lBytesIO, size);
#endif

  return size;

} // end writeBuffer 


//
// mTime
//
// return time in miliseconds
//
int
VPacket::mTime(void)
{
  static long basetime = 0;
  struct timeval TimeNow;
  struct timezone Zone;
  int rt;

  gettimeofday(&TimeNow, &Zone);

  if (basetime == 0)
    basetime = TimeNow.tv_sec;

  rt = (TimeNow.tv_sec - basetime) * 1000 + TimeNow.tv_usec / 1000L;

  return rt;
} // end mTime


//
// flush
//
// flush the file
int
VPacket::flush(void)
{
  int ret = -1;

  if (m_fd != -1)
  {
#ifdef __FreeBSD__
    ret = fsync(m_fd);
#else
    ret = fdatasync(m_fd);
#endif

    if (ret == -1)
    {
      switch(errno)
      {
        default:
          fprintf(stderr, "Error flushing file '%s' - ", m_sFile);
          fprintf(stderr, "%d '%s'\n", errno, strerror(errno));
          break;
      }
    }
  }

  return ret;
}

void
VPacket::setPlaybackSpeed(int speed)
{
  if (m_nPlaybackSpeed == -1)
    m_nFirstPacketTime = 0;
  m_nPlaybackSpeed = speed;
  m_nLastPacketTime = 0;
  m_nLastTime = 0;
}
