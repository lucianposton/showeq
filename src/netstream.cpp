/*
 * netstream.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2004 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#include "netstream.h"
#include "packetcommon.h"

NetStream::NetStream(const uint8_t* data, size_t length)
  : m_data(data),
    m_length(length)
{
  m_lastPos = &m_data[m_length];

  reset();
}

NetStream::~NetStream()
{
}

void NetStream::reset()
{
  // reset position to the end of the string
  m_pos = m_data;
}

uint8_t NetStream::readUInt8()
{
  uint8_t val;

  // make sure there is enough data left
  if ((m_lastPos - m_pos) >= 1)
  {
    // convert the data and increment past it
    val = *m_pos;
    m_pos++;
  }
  else 
    val = 0; // just return 0 if no data left

  return val;
}

int8_t NetStream::readInt8()
{
  int8_t val;

  // make sure there is enough data left
  if ((m_lastPos - m_pos) >= 1)
  {
    // convert the data and increment past it
    val = *(int8_t*)m_pos;
    m_pos++;
  }
  else 
    val = 0; // just return 0 if no data left

  return val;
}

uint16_t NetStream::readUInt16()
{
  uint16_t val;

  // make sure there is enough data left
  if ((m_lastPos - m_pos) >= 2)
  {
    // convert the data and increment past it
    val = eqntohuint16(m_pos);
    m_pos += 2;
  }
  else 
    val = 0; // just return 0 if no data left

  return val;
}

int16_t NetStream::readInt16()
{
  int16_t val;

  // make sure there is enough data left
  if ((m_lastPos - m_pos) >= 2)
  {
    // convert the data and increment past it
    val = eqntohint16(m_pos);
    m_pos += 2;
  }
  else
    val = 0; // just return 0 if no data left

  return val;
}

uint32_t NetStream::readUInt32()
{
  uint32_t val;

  // make sure there is enough data left
  if ((m_lastPos - m_pos) >= 4)
  {
    // convert the data and increment past it
    val = eqntohuint32(m_pos);
    m_pos += 4;
  }
  else 
    val = 0; // just return 0 if no data left

  return val;
}

int32_t NetStream::readInt32()
{
  uint32_t val;

  // make sure there is enough data left
  if ((m_lastPos - m_pos) >= 4)
  {
    // convert the data and increment past it
    val = eqntohint32(m_pos);
    m_pos += 4;
  }
  else
    val = 0; // just return 0 if no data left

  return val;
}

QString NetStream::readText()
{
  // make sure there is data left
  if (m_pos < m_lastPos)
  {
    // note the starting positino
    const uint8_t* startPos = m_pos;
    
    // search for the end of the NULL terminated string
    while ((*m_pos != '\0') && (m_pos < m_lastPos))
      m_pos++;
    
    size_t len = m_pos - startPos;

    // skip over trailing null
    if (m_pos < m_lastPos)
      m_pos++;
    
    // return the result as a QString
    return QString::fromUtf8((const char*)startPos, len);
  }
  else
    return QString();
}



