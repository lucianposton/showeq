/*
 * packetlog.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2004 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2004,2007 Zaphod (dohpaz@users.sourceforge.net). 
 */

#include <qdatetime.h>

#include "packetlog.h"
#include "packetformat.h"
#include "packetinfo.h"
#include "decode.h"
#include "diagnosticmessages.h"

//----------------------------------------------------------------------
// PacketLog
PacketLog::PacketLog(EQPacket& packet, const QString& fname, 
		     QObject* parent, const char* name)
  : SEQLogger(fname, parent, name),
    m_packet(packet)
{
  m_timeDateFormat = "MMM dd yyyy hh:mm:ss:zzz";
}

PacketLog::~PacketLog()
{
}

inline QString opCodeToString(uint16_t opCode)
{
  QString tempStr;
  tempStr.sprintf("[OPCode: %#.04x]", opCode);

  // Flags are gone? Combined and implicit don't make sense anymore and
  // nothing is compressed or encrypted at this point...
  /*
  if (opCode & FLAG_DECODE)
  {
    tempStr += " ";
    if (opCode & FLAG_COMP)
      tempStr += "[FLAG_COMP]";
    if (opCode & FLAG_COMBINED)
      tempStr += "[FLAG_COMBINED]";
    if (opCode & FLAG_CRYPTO)
      tempStr += "[FLAG_CRYPTO]";
    if (opCode & FLAG_IMPLICIT)
      tempStr += "[FLAG_IMPLICIT]";
  }
  */

  return tempStr;
}

/* Returns string representation of numeric IP address */
QString PacketLog::print_addr (in_addr_t  addr)
{
#ifdef DEBUG_PACKET
   debug ("print_addr()");
#endif /* DEBUG_PACKET */
  QString paddr;

  if (addr == m_packet.clientAddr())
    paddr = "client";
  else
    paddr.sprintf( "%d.%d.%d.%d",
		   addr & 0x000000ff,
		   (addr & 0x0000ff00) >> 8,
		   (addr & 0x00ff0000) >> 16,
		   (addr & 0xff000000) >> 24);

   return paddr;
}

/* Makes a note in a log */
void PacketLog::logMessage(const QString& message)
{
  if (!open())
    return;
    
   outputf ("%s\n", (const char*)message);

   flush();
}

/* Logs packet data in a human-readable format */
void PacketLog::logData(const uint8_t* data,
			size_t       len,
			const QString& prefix)
{
  if (!open())
    return;

  if (!prefix.isEmpty())
    outputf("%s ", (const char*)prefix.utf8());

   outputf("[Size: %d] %s\n", 
	  len, 
	  (const char*)QDateTime::currentDateTime().toString(m_timeDateFormat).utf8());

   // make sure there is a len before attempting to print it
   if (len)
     outputData(len, data);
   else
     outputf("\n");

  flush();
}

/* Logs packet data in a human-readable format */
void PacketLog::logData(const uint8_t* data,
			size_t len,
			uint8_t dir,
			uint16_t opcode,
			const QString& origPrefix)
{
  if (!open())
    return;

  // timestamp
  m_out << QDateTime::currentDateTime().toString(m_timeDateFormat) << " ";

  // append direction and opcode information
  if (!origPrefix.isEmpty())
    m_out << origPrefix << " ";
  
  // data direction and size
  m_out << ((dir == DIR_Server) ? "[Server->Client] " : "[Client->Server] ")
      << "[Size: " << QString::number(len) << "]" << endl;

  // output opcode info
  m_out << opCodeToString(opcode) << endl;

  flush();

  // make sure there is a len before attempting to output it
  if (len)
    outputData(len, data);
  else
    m_out << endl;

  flush();
}

/* Logs packet data in a human-readable format */
void PacketLog::logData(const uint8_t* data,
			size_t len,
			uint8_t dir,
			uint16_t opcode,
			const EQPacketOPCode* opcodeEntry,
			const QString& origPrefix)
{
  if (!open())
    return;

  // timestamp
  m_out << QDateTime::currentDateTime().toString(m_timeDateFormat) << " ";

  // append direction and opcode information
  if (!origPrefix.isEmpty())
    m_out << origPrefix << " ";
  
  // data direction and size
  m_out << ((dir == DIR_Server) ? "[Server->Client] " : "[Client->Server] ")
      << "[Size: " << QString::number(len) << "]" << endl;

  // output opcode info
  m_out << opCodeToString(opcode) << endl;

  if (opcodeEntry)
  {
    m_out << "[Name: " << opcodeEntry->name() << "][Updated: " 
	  << opcodeEntry->updated() << "]";
    const EQPacketPayload* payload = opcodeEntry->find(data, len, dir);
    if (payload)
    {
      m_out << "[Type: " << payload->typeName() << " (" 
	    << payload->typeSize() << ")";
      switch (payload->sizeCheckType())
      {
      case SZC_Match:
	m_out << " ==]";
	break;
      case SZC_Modulus:
	m_out << " %]";
	break;
      case SZC_None:
	m_out << " nc]";
	break;
      default:
	m_out << " " << payload->sizeCheckType() << "]";
	break;
      }
    }

    m_out  << endl;
  }

  flush();

  // make sure there is a len before attempting to output it
  if (len)
    outputData(len, data);
  else
    m_out << endl;

  flush();
}

void PacketLog::logData(const EQUDPIPPacketFormat& packet)
{
  if (!open())
    return;

  QString sourceStr, destStr;
  if (packet.getIPv4SourceN() == m_packet.clientAddr())
    sourceStr = "client";
  else
    sourceStr = packet.getIPv4SourceA();

  if (packet.getIPv4DestN() == m_packet.clientAddr())
    destStr = "client";
  else
    destStr = packet.getIPv4DestA();
  
  m_out << QDateTime::currentDateTime().toString(m_timeDateFormat)
      << " [" << sourceStr << ":" << QString::number(packet.getSourcePort())
      << "->" << destStr << ":" << QString::number(packet.getDestPort()) 
      << "] [Size: " << QString::number(packet.getUDPPayloadLength()) << "]"
      << endl;

  uint16_t calcedCRC;

  if (! packet.hasCRC() || 
    packet.crc() == (calcedCRC = ::calcCRC16(
      packet.rawPacket(), packet.rawPacketLength()-2, 
        packet.getSessionKey())))
  {
    m_out << "[OPCode: 0x" << QString::number(packet.getNetOpCode(), 16) << "]";

    if (packet.hasArqSeq())
    {
      m_out << " [Seq: " << QString::number(packet.arqSeq(), 16) << "]";
    }

    if (packet.hasFlags())
    {
      m_out << " [Flags: " << QString::number(packet.getFlags(), 16) << "]";
    }

    if (packet.hasCRC())
    {
      m_out << " [CRC ok]" << endl;
    }

    m_out << endl;
  }
  else
  {
    m_out << "[BAD CRC (" << QString::number(calcedCRC, 16) 
	  << " != " << QString::number(packet.crc(), 16) 
	  << ")! Sessions crossed or unitialized or non-EQ packet! ]" << endl;
    m_out << "[SessionKey: " << QString::number(packet.getSessionKey(), 16) <<
      "]" << endl;
  }

  flush();

  // make sure there is a len before attempting to output it
  if (packet.payloadLength())
    outputData(packet.getUDPPayloadLength(), 
	      (const uint8_t*)packet.getUDPPayload());
  else
    m_out << endl;

  flush();
}

void PacketLog::printData(const uint8_t* data, size_t len, uint8_t dir,
			  uint16_t opcode, const QString& origPrefix)
{
  if (!origPrefix.isEmpty())
    ::printf("%s ", (const char*)origPrefix);
  else
    ::putchar('\n');
  
  ::printf("%s [Size: %u]%s\n",
	   ((dir == DIR_Server) ? "[Server->Client]" : "[Client->Server]"),
	   len, (const char*)opCodeToString(opcode));

  if (len)
  {
    for (size_t a = 0; a < len; a ++)
    {
      if ((data[a] >= 32) && (data[a] <= 126))
	::putchar(data[a]);
      else
	::putchar('.');
    }
    
    ::putchar('\n');
    
    for (size_t a = 0; a < len; a ++)
    {
      if (data[a] < 32)
	::putchar(data[a] + 95);
      else if (data[a] > 126)
	::putchar(data[a] - 95);
      else if (data[a] > 221)
	::putchar(data[a] - 190);
      else
	::putchar(data[a]);
    }
    
    ::putchar('\n');
    
    
    for (size_t a = 0; a < len; a ++)
      ::printf ("%02X", data[a]);

    ::printf("\n\n"); /* Adding an extra line break makes it easier
			 for people trying to decode the OpCodes to
			 tell where the raw data ends and the next
			 message begins...  -Andon */
  }
  else
    ::putchar('\n');
}

//----------------------------------------------------------------------
// PacketStreamLog
PacketStreamLog::PacketStreamLog(EQPacket& packet, const QString& fname, 
				 QObject* parent, const char* name)
  : PacketLog(packet, fname, parent, name),
    m_raw(true)
{
}

void PacketStreamLog::rawStreamPacket(const uint8_t* data, size_t len, 
				 uint8_t dir, uint16_t opcode)
{
  if (m_raw)
    logData(data, len, dir, opcode, "[Raw]");
}

void PacketStreamLog::decodedStreamPacket(const uint8_t* data, size_t len, 
					  uint8_t dir, uint16_t opcode, 
					  const EQPacketOPCode* opcodeEntry)
{
  //  if ((opcode != 0x0028) && (opcode != 0x003f) && (opcode != 0x025e))
    logData(data, len, dir, opcode, opcodeEntry, "[Decoded]");
}

//----------------------------------------------------------------------
// UnknownPacketLog
UnknownPacketLog::UnknownPacketLog(EQPacket& packet, const QString& fname, 
				   QObject* parent, const char* name)
  : PacketLog(packet, fname, parent, name),
    m_view(false)
{
}

void UnknownPacketLog::packet(const uint8_t* data, size_t len, uint8_t dir, 
			      uint16_t opcode, 
			      const EQPacketOPCode* opcodeEntry, bool unknown)
{
  if (unknown)
  {
    logData(data, len, dir, opcode, opcodeEntry);
   
    if (m_view)
      printData(data, len, dir, opcode, "Unknown");
  }
}

//----------------------------------------------------------------------
// OpCodeMonitorPacketLog
OPCodeMonitorPacketLog::OPCodeMonitorPacketLog(EQPacket& packet, 
					       const QString& fname, 
					       QObject* parent, 
					       const char* name)
  : PacketLog(packet, fname, parent, name),
    m_log(false), 
    m_view(false)
{
}

void OPCodeMonitorPacketLog::init(QString monitoredOPCodes)
{
  if (monitoredOPCodes.isEmpty() || monitoredOPCodes == "0") /* DISABLED */
  {
    seqWarn("OpCode monitoring COULD NOT BE ENABLED!");
    seqWarn(">> Please check your showeq.xml file for a list entry under [OpCodeMonitoring]");
    return;
  }

  seqInfo("OpCode monitoring ENABLED...");
  seqInfo("Using list:\t%s",
	  (const char*)monitoredOPCodes);


  QString qsCommaBuffer (""); /* Construct these outside the loop so we don't have to construct
				 and destruct these variables during each iteration... */
  QString qsColonBuffer ("");
  
  int            iCommaPos      = 0;
  int            iColonPos      = 0;
  uint8_t        uiIterationID  = 0;
  
  for (uint8_t uiIndex = 0; (uiIndex < OPCODE_SLOTS) && !monitoredOPCodes.isEmpty(); uiIndex ++)
  {
    /* Initialize the variables with their default values */
    MonitoredOpCodeList      [uiIndex] [0] = 0; /* OpCode number (16-bit HEX) */
    MonitoredOpCodeList      [uiIndex] [1] = 3; /* Direction, DEFAULT: Client <--> Server */
    MonitoredOpCodeList      [uiIndex] [2] = 1; /* Show raw data if packet is marked as known */
    MonitoredOpCodeAliasList [uiIndex]     = "Monitored OpCode"; /* Name used when displaying the raw data */
    
    iCommaPos = monitoredOPCodes.find (",");
    
    if (iCommaPos == -1)
      iCommaPos = monitoredOPCodes.length ();
    
    qsCommaBuffer = monitoredOPCodes.left (iCommaPos);
    monitoredOPCodes.remove (0, iCommaPos + 1);
    
    uiIterationID = 0;
    
    int uiColonCount = qsCommaBuffer.count(":");
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
    
#if 1 // ZBTEMP
    seqDebug("opcode=%04x name='%s' dir=%d known=%d",
	     MonitoredOpCodeList [uiIndex] [0],
	     (const char*)MonitoredOpCodeAliasList [uiIndex],
	     MonitoredOpCodeList [uiIndex] [1],
	     MonitoredOpCodeList [uiIndex] [2]);
#endif
  }
}

void OPCodeMonitorPacketLog::packet(const uint8_t* data, size_t len, 
				    uint8_t dir, uint16_t opcode, 
				    const EQPacketOPCode* opcodeEntry, 
				    bool unknown)
{
  unsigned int uiOpCodeIndex = 0;
  unsigned int uiIndex = 0;
    
  for (; ((uiIndex < OPCODE_SLOTS) && (uiOpCodeIndex == 0)); uiIndex ++)
  {
    if (opcode == MonitoredOpCodeList[ uiIndex ][ 0 ])
    {
      if ((MonitoredOpCodeList[ uiIndex ][ 1 ] == dir) || 
	  (MonitoredOpCodeList[ uiIndex ][ 1 ] == 3))
      {
	if ((!unknown && (MonitoredOpCodeList[ uiIndex ][ 2 ] == 1)) 
	    || unknown)
	  uiOpCodeIndex = uiIndex + 1;
      }
    }
  }
  
  if (uiOpCodeIndex > 0)
  {
    QString opCodeName = MonitoredOpCodeAliasList[uiOpCodeIndex - 1].latin1();
    
    if (m_view)
      printData(data, len, dir, opcode, opCodeName);
    
    if (m_log)
      logData(data, len, dir, opcode, opcodeEntry,opCodeName);
  }
}

#ifndef QMAKEBUILD
#include "packetlog.moc"
#endif

