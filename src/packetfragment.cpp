/*
 * packetfragment.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

/* Implementation of EQPacketFragmentSequence class */

#include "packetfragment.h"
#include "packetformat.h"
#include "diagnosticmessages.h"

//----------------------------------------------------------------------
// Macros

// diagnose fragmentation problems
//#define PACKET_PROCESS_FRAG_DIAG

//----------------------------------------------------------------------
// EQPacketFragmentSequence class methods

////////////////////////////////////////////////////
// Constructor
EQPacketFragmentSequence::EQPacketFragmentSequence()
  : m_streamid(unknown_stream),
    m_data(0),
    m_totalLength(0),
    m_dataSize(0),
    m_dataAllocSize(0)
{
}

////////////////////////////////////////////////////
// Constructor
EQPacketFragmentSequence::EQPacketFragmentSequence(EQStreamID streamid)
  : m_streamid(streamid),
    m_data(0),
    m_totalLength(0),
    m_dataSize(0),
    m_dataAllocSize(0)
{
}

////////////////////////////////////////////////////
// Destructor
EQPacketFragmentSequence::~EQPacketFragmentSequence()
{
  if (m_data)
    delete [] m_data;
}

////////////////////////////////////////////////////
// Reset the fragment sequence
void EQPacketFragmentSequence::reset()
{
#ifdef PACKET_PROCESS_FRAG_DIAG
   debug ("EQPacketFragmentSequence::reset() stream %d (complete fragment? %s)",
     m_streamid, (isComplete() ? "yes" : "no"));
#endif
  m_dataSize = 0;
  m_totalLength = 0;
}

////////////////////////////////////////////////////
// Add a fragment to the sequence
void EQPacketFragmentSequence::addFragment(EQProtocolPacket& packet)
{
#ifdef PACKET_PROCESS_FRAG_DIAG
   debug ("EQPacketFragmentSequence::addFragment() stream %d", m_streamid);
#endif

   // If dataSize isn't filled in, this is first fragment. Need to alloc.
   if (m_dataSize == 0)
   {
      // Buffer length is on the wire past the sequence number.
      m_totalLength = eqntohuint32(&(packet.payload()[2]));

      if (m_totalLength == 0)
      {
         seqWarn("Oversized packet fragment requested buffer of size 0 on stream %d OpCode %04x seq %04x",
           m_streamid, *(uint16_t*)(&packet.payload()[6]), packet.arqSeq());
      }
      else if (m_totalLength > m_dataAllocSize)
      {
        // Buffer isn't big enough. Enlargen it.
        if (m_data)
        {
          delete[] m_data;
        }
        m_dataAllocSize = m_totalLength;
        m_data = new uint8_t[m_dataAllocSize];

#ifdef PACKET_PROCESS_FRAG_DIAG
        seqDebug("EQPacketFragmentSequence::addFragment(): Allocating %d bytes for seq %04x, stream %d, OpCode 0x%04x",
          m_dataAllocSize, packet.arqSeq(), m_streamid, 
           *(uint16_t*)(&packet.payload()[6]));
#endif
      }
      
      // Now put in this fragment. Payload starts after seq, unknown, 
      // and alloc size.
#ifdef PACKET_PROCESS_FRAG_DIAG
      seqDebug("EQPacketFragmentSequence::addFragment(): Putting initial %d byte fragment into buffer for seq %04x, stream %d, OpCode 0x%04x",
             packet.payloadLength() - 6, packet.arqSeq(), m_streamid, 
             *(uint16_t*)(&packet.payload()[6]));
#endif
      memcpy(m_data, &packet.payload()[6], packet.payloadLength()-6);
      m_dataSize = packet.payloadLength() - 6;
   }
   else
   {
      // Add this fragment to the buffer. Payload starts immediately after seq.
#ifdef PACKET_PROCESS_FRAG_DIAG
      seqDebug("EQPacketFragmentSequence::addFragment(): Putting %d byte fragment into buffer for seq %04x, stream %d, OpCode 0x%04x",
             packet.payloadLength() - 2, packet.arqSeq(), m_streamid, 
             *(uint16_t*)(m_data));
#endif
      
      if (m_data + m_dataSize + packet.payloadLength() - 2 > m_data + m_dataAllocSize)
      {
        seqFatal("!!!! EQPacketFragmentSequence::addFragment(): buffer overflow adding in new fragment to buffer with seq %04x on stream %d, opcode %04x. Buffer is size %d and has been filled up to %d, but tried to add %d more!",
          packet.arqSeq(), m_streamid, *(uint16_t*)(m_data),
          m_dataAllocSize, m_dataSize, packet.payloadLength() - 2);
      }

      memcpy(m_data + m_dataSize, 
         &packet.payload()[2], packet.payloadLength() - 2);
      m_dataSize += (packet.payloadLength() - 2);
   }
}
