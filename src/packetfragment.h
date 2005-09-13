/*
 * packetfragment.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _PACKETFRAGMENT_H_
#define _PACKETFRAGMENT_H_

#include <sys/types.h>

#include "packetcommon.h"

class EQProtocolPacket;

//----------------------------------------------------------------------
// EQPacketFragmentSequence
class EQPacketFragmentSequence
{
 public:
  EQPacketFragmentSequence();
  EQPacketFragmentSequence(EQStreamID streamid);
  ~EQPacketFragmentSequence();
  void reset();
  void addFragment(EQProtocolPacket& packet);
  bool isComplete();

  uint8_t* data();
  size_t size();

 protected:
  EQStreamID m_streamid;
  uint8_t *m_data;
  uint32_t m_totalLength;
  size_t m_dataSize;
  uint32_t m_dataAllocSize;
};

inline bool EQPacketFragmentSequence::isComplete()
{
  return m_dataSize != 0 && m_totalLength == m_dataSize;
}

inline uint8_t* EQPacketFragmentSequence::data()
{
  return m_data;
}

inline size_t EQPacketFragmentSequence::size()
{
  return m_dataSize;
}

#endif // _PACKETFRAGMENT_H_
