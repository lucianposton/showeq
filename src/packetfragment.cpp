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

//#define DEBUG_PACKET
//#undef DEBUG_PACKET

// diagnose structure size changes
//#define PACKET_PROCESS_FRAG_DIAG

//----------------------------------------------------------------------
// EQPacketFragmentSequence class methods

////////////////////////////////////////////////////
// Constructor
EQPacketFragmentSequence::EQPacketFragmentSequence()
  : m_streamid(unknown_stream),
    m_data(0),
    m_dataSize(0),
    m_dataAllocSize(0),
    m_seq(0),
    m_current(0),
    m_total(0)
{
}

////////////////////////////////////////////////////
// Constructor
EQPacketFragmentSequence::EQPacketFragmentSequence(EQStreamID streamid)
  : m_streamid(streamid),
    m_data(0),
    m_dataSize(0),
    m_dataAllocSize(0),
    m_seq(0),
    m_current(0),
    m_total(0)
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
  m_dataSize = 0;
  m_seq = 0;
  m_current = 0;
  m_total = 0;
}

////////////////////////////////////////////////////
// Add a fragment to the sequence
void EQPacketFragmentSequence::addFragment(EQPacketFormat& pf)
{
#ifdef DEBUG_PACKET
   debug ("EQPacketFragmentSequence::addFragment()");
#endif /* DEBUG_PACKET */

#ifdef PACKET_PROCESS_FRAG_DIAG
   seqDebug("EQPacketFragmentSequence::addFragment(): pf.arq 0x%04x, pf.fragSeq 0x%04x, pf.fragCur 0x%04x, pf.fragTot 0x%04x", pf.arq(), pf.fragSeq(), pf.fragCur(), pf.fragTot());
#endif /* PACKET_PROCESS_FRAG_DIAG */

   // fragments with ASQ signify the beginning of a new series
   // warn if previous series is incomplete
   // clear and allocate space for the new series
   if (pf.isASQ())
   {
      if (m_dataSize)
      {
         if (!pf.fragSeq() == 0) // surpress WARNING for duplicate SEQStart/fragment start (e.g.0x3a)
         {
	   seqWarn("EQPacketFragmentSequence::addFragment(): WARNING OpCode 0x%04x will not be processed due to loss",
                   eqntohuint16(m_data));
	   seqWarn("EQPacketFragmentSequence::addFragment(): recieved new fragment seq 0x%04x before completion of 0x%04x",
                   pf.fragSeq(), m_seq);
         }
      }

      size_t needs = (pf.fragTot() * pf.payloadLength());
      if (needs > m_dataAllocSize)
      {
	delete [] m_data;
	m_dataAllocSize = needs;
	m_data = new uint8_t[m_dataAllocSize]; // should be an over estimate
      }

      m_dataSize = 0;
      m_seq = pf.fragSeq();
      m_current = pf.fragCur();
      m_total = pf.fragTot();
      

#ifdef PACKET_PROCESS_FRAG_DIAG
      seqDebug("EQPacketFragmentSequence::addFragment(): Allocating %d bytes for fragmentSeq %d, stream %d, OpCode 0x%04x",
             (pf.fragTot() * pf.payloadLength()), pf.fragSeq(), m_streamid, eqntohuint16(m_data));
#endif
   }

   if (m_data != NULL)
   {
      if (pf.fragSeq() != m_seq || pf.fragCur() != m_current)
      {
	seqWarn("EQPacketFragmentSequence::addFragment: WARNING OpCode 0x%04x will not be processed due to loss",
             eqntohuint16(m_data)); 
	seqWarn("EQPacketFragmentSequence::addFragment(): recieved Out-Of-Order fragment seq 0x%04x (0x%04x) expected 0x%04x",
              pf.fragCur(), pf.fragSeq(), m_current);
         return;
      }
      else if ((m_dataSize + pf.payloadLength()) > m_dataAllocSize)
      {
	// theoretically should never get here, but...
	// calculate needed size
	size_t needs = m_dataSize + pf.payloadLength();

	// allocate new buffer
	uint8_t* newData = new uint8_t[needs];

	// copy old data
	memcpy((void*)newData, (void*)m_data, m_dataSize);

	// delete old data
	delete [] m_data;

	m_dataAllocSize = needs;
	m_data = newData;
      }
      else
      {
         memcpy((void*)(m_data + m_dataSize), (void*)pf.payload(), pf.payloadLength());

         m_dataSize += pf.payloadLength();
         m_current = pf.fragCur()+1;

         return;
      }
#ifdef PACKET_PROCESS_FRAG_DIAG
   }
   else
   {
     seqWarn("EQPacketFragmentSequence::addFragment(): recieved fragment component (fragSeq 0x%04x, fragCur 0x%04x) before fragment start",
	    pf.fragSeq(), pf.fragCur());
#endif
   }
}
