/*
 * packetinfo.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2003-2004 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#include <stdio.h>

#include <qobject.h>
#include <qmetaobject.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qxml.h>

#include <map>

#include "packetinfo.h"
#include "packetcommon.h"
#include "everquest.h"
#include "diagnosticmessages.h"

//----------------------------------------------------------------------
// Macros

// this define is used to diagnose problems with packet dispatch
// #define  PACKET_DISPATCH_DIAG 1

//----------------------------------------------------------------------
// OPCodeXmlContentHandler declaration
class OPCodeXmlContentHandler : public QXmlDefaultHandler
{
public:
  OPCodeXmlContentHandler(const EQPacketTypeDB& typeDB, 
			  EQPacketOPCodeDB& opcodeDB);
  virtual ~OPCodeXmlContentHandler();
  
  // QXmlContentHandler overrides
  bool startDocument();
  bool startElement( const QString&, const QString&, const QString& , 
		     const QXmlAttributes& );
  bool characters(const QString& ch);
  bool endElement( const QString&, const QString&, const QString& );
  bool endDocument();
  
protected:
  const EQPacketTypeDB& m_typeDB;
  EQPacketOPCodeDB& m_opcodeDB;

  EQPacketOPCode* m_currentOPCode;
  EQPacketPayload* m_currentPayload;
  
  QString m_currentComment;

  bool m_inComment;
};


//----------------------------------------------------------------------
// EQPacketTypeDB
EQPacketTypeDB::EQPacketTypeDB()
  : m_typeSizeDict(127) // increase this number if we ever get >= 97 types
{
  // define the convenience macro used in the generated file
#define AddStruct(typeName) addStruct(#typeName, sizeof(typeName))

  // include the generated file
#include "s_everquest.h"

  // undefine the convenience macro
#undef AddStruct

  // these we add manually to handle strings and octet streams
  addStruct("char", sizeof(char));
  addStruct("uint8_t", sizeof(uint8_t));
  addStruct("none", 0);
  addStruct("unknown", 0);
}

EQPacketTypeDB::~EQPacketTypeDB()
{
}

size_t EQPacketTypeDB::size(const char* typeName) const
{
  // attempt to find the item in the type size dictionary
  size_t *size = m_typeSizeDict.find(typeName);

  // if it was found, return its size
  if (size)
    return *size;

  // return 0
  return 0;
}

bool EQPacketTypeDB::valid(const char* typeName) const
{
  // attempt to find the item in the type size dictionary
  size_t *size = m_typeSizeDict.find(typeName);

  return (size != 0);
}

void EQPacketTypeDB::list(void) const
{
  seqInfo("EQPacketTypeDB contains %d types (in %d buckets)",
	  m_typeSizeDict.count(), m_typeSizeDict.size());

  QAsciiDictIterator<size_t> it(m_typeSizeDict);

  while (it.current())
  {
    seqInfo("\t%s = %d", it.currentKey(), *(it.current()));
    ++it;
  }
}

void EQPacketTypeDB::addStruct(const char* typeName, size_t size)
{
  m_typeSizeDict.insert(typeName, new size_t(size));
}

//----------------------------------------------------------------------
// EQPacketDispatch
EQPacketDispatch::EQPacketDispatch(QObject* parent, const char* name)
  : QObject(parent, name)
{
}

EQPacketDispatch::~EQPacketDispatch()
{
}

void EQPacketDispatch::activate(const uint8_t* data, size_t len, uint8_t dir)
{
  emit signal(data, len, dir);
}

bool EQPacketDispatch::connect(const QObject* receiver, const char* member)
{
#ifdef PACKET_DISPATCH_DIAG
  seqDebug("Connecting '%s:%s' to '%s:%s' objects %s.",
	  className(), name(), receiver->className(), receiver->name(),
	  (const char*)member);
#endif

  return QObject::connect((QObject*)this, 
			  SIGNAL(signal(const uint8_t*, size_t, uint8_t)),
			  receiver, member);
}

bool EQPacketDispatch::disconnect(const QObject* receiver, const char* member)
{
  return QObject::disconnect((QObject*)this,
			     SIGNAL(signal(const uint8_t*, size_t, uint8_t)),
			     receiver, member);
}

//----------------------------------------------------------------------
// EQPacketPayload
EQPacketPayload::EQPacketPayload()
  : m_typeSize(0),
    m_sizeCheckType(SZC_None),
    m_dir(0x00)
{
}

EQPacketPayload::~EQPacketPayload()
{
}

bool EQPacketPayload::setType(const EQPacketTypeDB& db, 
			      const char* typeName)
{
  // first, check that it is a valid type
  if (!db.valid(typeName))
    return false;

  // valid type, ok, use it
  m_typeName = typeName;

  // get the types size
  m_typeSize = db.size(typeName);

  return true;
}

bool EQPacketPayload::match(const uint8_t* data, size_t size, 
			    uint8_t dir) const
{
  switch(m_sizeCheckType)
  {
  case SZC_None:
    return ((m_dir & dir) != 0);
  case SZC_Match:
    return (((m_dir & dir) != 0) &&
	    (m_typeSize == size));
  case SZC_Modulus:
    return (((m_dir & dir) != 0) &&
	    ((size % m_typeSize) == 0));
  default:
    break;
  }

  return false;
}


//----------------------------------------------------------------------
// EQPacketOPCode
EQPacketOPCode::EQPacketOPCode()
  : m_opcode(0),
    m_implicitLen(0)
{
  setAutoDelete(true);
}

EQPacketOPCode::EQPacketOPCode(uint16_t opcode, const QString& name)
  : m_opcode(opcode),
    m_implicitLen(0),
    m_name(name)
{
  setAutoDelete(true);
}

EQPacketOPCode::EQPacketOPCode(const EQPacketOPCode& opcode)
  : m_opcode(opcode.m_opcode),
    m_implicitLen(opcode.m_implicitLen),
    m_name(opcode.m_name),
    m_updated(opcode.m_updated)
{
  setAutoDelete(true);
}

EQPacketOPCode::~EQPacketOPCode()
{
}

EQPacketPayload* EQPacketOPCode::find(const uint8_t* data, size_t size, uint8_t dir) const
{
  EQPacketPayload* payload;

  // iterate over the payloads until a matching one is found
  EQPayloadListIterator it(*this);
  while ((payload = it.current()) != 0)
  {
    // if a match is found, return it.
    if (payload->match(data, size, dir))
      return payload;

    // iterate to the next payload
    ++it;
  }

  // no matches, return 0
  return 0;
}


//----------------------------------------------------------------------
// EQPacketOPCodeDB
EQPacketOPCodeDB::EQPacketOPCodeDB(int size)
  : m_opcodes(size) 
{
  m_opcodes.setAutoDelete(true);
  m_opcodesByName.setAutoDelete(false);
}

EQPacketOPCodeDB::~EQPacketOPCodeDB()
{
}

bool EQPacketOPCodeDB::load(const EQPacketTypeDB& typeDB, 
			    const QString& filename)
{
  // load opcodes

  // create XML content handler
  OPCodeXmlContentHandler handler(typeDB, *this);

  // create a file object on the file
  QFile xmlFile(filename);

  // create an XmlInputSource on the file
  QXmlInputSource source(&xmlFile);
  
  // create an XML parser
  QXmlSimpleReader reader;

  // set the content handler
  reader.setContentHandler(&handler);

  // parse the file
  return reader.parse(source);
}

bool EQPacketOPCodeDB::save(const QString& filename)
{
  // create QFile object
  QFile file(filename);

  // open the file for write only
  if (!file.open(IO_WriteOnly))
    return false;

  // create a QTextStream object on the QFile object
  QTextStream out(&file);
  
  // set the output encoding to be UTF8
  out.setEncoding(QTextStream::UnicodeUTF8);

  // set the number output to be left justified decimal
  out.setf(QTextStream::dec | QTextStream::left);

  // print document header
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl
      << "<!DOCTYPE seqopcodes SYSTEM \"seqopcodes.dtd\">" << endl
      << "<seqopcodes>" << endl;

  // set initial indent
  QString indent = "    ";

  EQPacketOPCode* currentOPCode;
  EQPacketPayload* currentPayload;

  typedef std::map<long, EQPacketOPCode*> OrderedMap;
  OrderedMap orderedOPCodes;

  // iterate over all the opcodes, inserting them into the ordered map
  QIntDictIterator<EQPacketOPCode> it(m_opcodes);
  while ((currentOPCode = it.current()) != NULL)
  {
    // insert into the ordered opcode map
    orderedOPCodes.insert(OrderedMap::value_type(currentOPCode->opcode(), 
						 currentOPCode));

    // get next opcode
    ++it;
  }

  OrderedMap::iterator oit;
  QCString opcodeString(256);
  static const char* dirStrs[] = { "client", "server", "both", };
  static const char* sztStrs[] = { "none", "match", "modulus", };

  // iterate over the ordered opcode map
  for (oit = orderedOPCodes.begin(); oit != orderedOPCodes.end(); ++oit)
  {
    currentOPCode = oit->second;

    // output the current opcode
    opcodeString.sprintf("%04x", currentOPCode->opcode());
    out << indent << "<opcode id=\"" << opcodeString << "\" name=\""
	<< currentOPCode->name() << "\"";
    if (currentOPCode->implicitLen())
      out << " implicitlen=\"" << currentOPCode->implicitLen() << "\"";
    if (!currentOPCode->updated().isEmpty())
      out << " updated=\"" << currentOPCode->updated() << "\"";
    out << ">" << endl;

    // increase the indent
    indent += "    ";

    // output the comments
    QStringList comments = currentOPCode->comments();
    for (QStringList::Iterator cit = comments.begin(); 
	 cit != comments.end(); ++cit)
      out << indent << "<comment>" << *cit << "</comment>" << endl;

    QCString dirStr;
    QCString sztStr;

    // iterate over the payloads
    QPtrListIterator<EQPacketPayload> pit(*currentOPCode);
    while ((currentPayload = pit.current()) != 0)
    {
      // output the payload
      out << indent << "<payload dir=\"" << dirStrs[currentPayload->dir()-1]
	  << "\" typename=\"" << currentPayload->typeName() 
	  << "\" sizechecktype=\""
	  << sztStrs[currentPayload->sizeCheckType()]
	  << "\"/>" << endl;

      ++pit;
    }

    // decrease the indent
    indent.remove(0, 4);

    // close the opcode entity
    out << indent << "</opcode>" << endl;
  }

  // output closing entity
  out << "</seqopcodes>" << endl;

  return true;
}

EQPacketOPCode* EQPacketOPCodeDB::add(uint16_t opcode, const QString& name)
{
  // Create the new opcode object
  EQPacketOPCode* newOPCode = new EQPacketOPCode(opcode, name);

  // insert the opcode into the opcode table
  m_opcodes.insert(opcode, newOPCode);

  // insert the object into the opcode by name table
  m_opcodesByName.insert(name, newOPCode);

  // return the opcode object
  return newOPCode;
}

void EQPacketOPCodeDB::list(void) const
{
  m_opcodes.statistics();

  seqInfo("EQPacketOPCodeDB contains %d opcodes (in %d buckets)",
	  m_opcodes.count(), m_opcodes.size());

  EQPacketOPCode* current;
  EQPacketPayload* currentPayload;

  // iterate over all the opcodes
  QIntDictIterator<EQPacketOPCode> it(m_opcodes);
  while ((current = it.current()) != NULL)
  {
    fprintf(stderr, "\tkey=%04lx opcode=%04x",
	    it.currentKey(), current->opcode());
    if (!current->name().isNull())
      fprintf(stderr, " name='%s'", current->name().latin1());

    if (current->implicitLen())
      fprintf(stderr, " implicitlen='%d'", current->implicitLen());
    
    if (!current->updated().isNull())
      fprintf(stderr, " updated='%s'", current->updated().latin1());

    fputc('\n', stderr);

    QStringList comments = current->comments();
    
    fprintf(stderr, "\t\t%d comment(s)\n", comments.count());

    for (QStringList::Iterator cit = comments.begin(); 
	 cit != comments.end(); ++cit)
      fprintf(stderr, "\t\t\t'%s'\n", (*cit).latin1());
    
    fprintf(stderr, "\t\t%d payload(s)\n", current->count());
    
    QPtrListIterator<EQPacketPayload> pit(*current);
    while ((currentPayload = pit.current()) != 0)
    {
      seqInfo("\t\t\tdir=%d typename=%s size=%d sizechecktype=%d",
	      currentPayload->dir(), (const char*)currentPayload->typeName(),
	      currentPayload->typeSize(), currentPayload->sizeCheckType());

      ++pit;
    }

    ++it;
  }
}

bool EQPacketOPCodeDB::remove(uint16_t opcode)
{
  // remove the opcode object from the opcodes table
  EQPacketOPCode* opcodeObj = m_opcodes.take(opcode);

  if (opcodeObj)
  {
    // remove it from the opcodes by name table
    m_opcodesByName.remove(opcodeObj->name());

    // delete the opcode object
    delete opcodeObj;

    return true;
  }

  return false;
}

bool EQPacketOPCodeDB::remove(const QString& opcodeName)
{
  // remove the opcode object from the opcodes table
  EQPacketOPCode* opcode = m_opcodesByName.take(opcodeName);

  // if found, remove it from the opcodes table
  if (opcode)
    return m_opcodes.remove(opcode->opcode());

  return false;
}

bool EQPacketOPCodeDB::move(uint16_t oldOPCode, uint16_t newOPCode)
{
  // attempt to take an existing opcode object out of the table
  EQPacketOPCode* opcode = m_opcodes.take(oldOPCode);

  // if failed to find an existing opcode object, return failure
  if (!opcode)
    return false;

  // set the new opcode value within the object
  opcode->setOPCode(newOPCode);

  // reinsert the object into the table under the new opcode id
  m_opcodes.insert(newOPCode, opcode);

  return true;
}


bool EQPacketOPCodeDB::move(const QString& oldOPCodeName,
			    const QString& newOPCodeName)
{
  // attempt to take an existing opcode object out of the table
  EQPacketOPCode* opcode = m_opcodesByName.take(oldOPCodeName);

  // if failed to find an existing opcode object, return failure
  if (!opcode)
    return false;

  // set the new opcode value within the object
  opcode->setName(newOPCodeName);

  // reinsert the object into the table under the new opcode id
  m_opcodesByName.insert(newOPCodeName, opcode);

  return true;
}

//----------------------------------------------------------------------
// OPCodeXmlContentHandler implementation
OPCodeXmlContentHandler::OPCodeXmlContentHandler(const EQPacketTypeDB& typeDB, 
						 EQPacketOPCodeDB& opcodeDB)
  : m_typeDB(typeDB),
    m_opcodeDB(opcodeDB)
{
}

OPCodeXmlContentHandler::~OPCodeXmlContentHandler()
{
}
  
  // QXmlContentHandler overrides
bool OPCodeXmlContentHandler::startDocument()
{
  // not in an opcode yet, so set the current OPCode object to NULL
  m_currentOPCode = NULL;
  m_currentPayload = NULL;
  m_inComment = false;;
  return true;
}

bool OPCodeXmlContentHandler::startElement(const QString&, const QString&, 
					   const QString& name, 
					   const QXmlAttributes& attr)
{
  if (name == "opcode")
  {
    bool ok = false;

    // get the index of the id attribute
    int index = attr.index("id");
    if (index == -1)
    {
      seqWarn("OPCodeXmlContentHandler::startElement(): opcode element without id!");
	      
      return false; // this is an error, something is wrong
    }

    // the id attribute is the opcode value
    uint16_t opcode = attr.value(index).toUShort(&ok, 16);

#if 0 // ZBTEMP
    opcode += 2;
#endif 

    if (!ok)
    {
      seqWarn("OPCodeXmlContentHandler::startElement(): opcode '%s' failed to convert to uint16_t (result: %#04x)",
	      attr.value(index).latin1(), opcode);

      return false; // this is an error
    }

    // get the index of the name attribute
    index = attr.index("name");
    
    // if name attribute was found, set the opcode objects name
    if (index == -1)
    {
      seqWarn("OPCodeXmlContentHandler::startElement(): opcode %#04x missing name parameter!",
	      opcode);

      return false;
    }

    // add/create the new opcode object
    m_currentOPCode = m_opcodeDB.add(opcode, attr.value(index));

    if (!m_currentOPCode)
    {
      seqWarn("Failed to add opcode %04x", opcode);
      return false;
    }


    // get the index of the updated attribute
    index = attr.index("updated");
    
    // if the updated attribute was found, set the objects updated field
    if (index != -1)
      m_currentOPCode->setUpdated(attr.value(index));

    // get the index of the implicitlen attribute
    index = attr.index("implicitlen");

    // if implicitlen attribute was found, set the objects implicitLen field
    if (index != -1)
      m_currentOPCode->setImplicitLen(attr.value(index).toUShort());

    return true;
  }

  if ((name == "comment") && (m_currentOPCode))
  {
    // clear any current comment
    m_currentComment = "";
    m_inComment = true;

    return true;
  }

  if ((name == "payload") && (m_currentOPCode))
  {
    // create a new payload object and make it the current one
    m_currentPayload = new EQPacketPayload();

    // add the payload object to the opcode
    m_currentOPCode->append(m_currentPayload);

    // check for direction attribute
    int index = attr.index("dir");

    // if an index attribute exists, then use it
    if (index != -1)
    {
      QString value = attr.value(index);

      if (value == "both")
	m_currentPayload->setDir(DIR_Client | DIR_Server);
      else if (value == "server")
	m_currentPayload->setDir(DIR_Server);
      else if (value == "client")
	m_currentPayload->setDir(DIR_Client);
    }

    // get the typename attribute
    index = attr.index("typename");

    // if a typename attribute exist, then set the payload type
    if (index != -1)
    {
      QString value = attr.value(index);
      
      if (!value.isEmpty())
      {
	if (!m_currentPayload->setType(m_typeDB, value))
	  seqWarn("Warning: Unknown payload typename '%s' for opcode '%04x",
		  value.latin1(), m_currentOPCode->opcode());
      }
    }

    // attempt to retrieve the sizechecktype
    index = attr.index("sizechecktype");

    // if a sizechecktype exists, then set the payload size check type
    if (index != -1)
    {
      QString value = attr.value(index);

      if (value.isEmpty() || (value == "none"))
	m_currentPayload->setSizeCheckType(SZC_None);
      else if (value == "match")
	m_currentPayload->setSizeCheckType(SZC_Match);
      else if (value == "modulus")
	m_currentPayload->setSizeCheckType(SZC_Modulus);
    }

    return true;
  }

  return true;
}

bool OPCodeXmlContentHandler::characters(const QString& ch)
{
  // if in a <comment>, add the current characters to it's text
  if (m_inComment)
    m_currentComment += ch;
    
  return true;
}

bool OPCodeXmlContentHandler::endElement(const QString&, const QString&, 
					 const QString& name)
{
  if (name == "opcode")
  {
    // not currently in an opcode, so set the current OPCode object to NULL
    m_currentOPCode = NULL;

    return true;
  }

  if ((name == "comment") && (m_inComment))
  {
    m_inComment = false;
    if (m_currentOPCode)
      m_currentOPCode->addComment(m_currentComment);
  }

  if ((name == "payload") && (m_currentPayload))
    m_currentPayload = NULL;

  return true;
}

bool OPCodeXmlContentHandler::endDocument()
{
  return true;
}

