/*
 * netstream.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2004 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#ifndef _NETSTREAM_H_
#define _NETSTREAM_H_

#include <stdint.h>
#include <qstring.h>

class NetStream
{
 public:
  NetStream(const uint8_t* data, size_t length);
  ~NetStream();

  const uint8_t* data() { return m_data; }
  size_t length() { return m_length; }
  void reset();
  bool end() { return (m_pos >= m_lastPos); }

  uint8_t readUInt8();
  int8_t readInt8();
  uint16_t readUInt16();
  int16_t readInt16();
  uint32_t readUInt32();
  int32_t readInt32();
  QString readText();

 protected:
  const uint8_t* m_data;
  size_t m_length;
  const uint8_t* m_lastPos;
  const uint8_t* m_pos;
};

#endif // _NETSTREAM_H_


