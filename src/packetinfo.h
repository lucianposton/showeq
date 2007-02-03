/*
 * packetinfo.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#ifndef _PACKETINFO_H_
#define _PACKETINFO_H_

#include <stdint.h>

#include <qobject.h>
#include <q3ptrdict.h>
#include <q3ptrlist.h>
#include <q3asciidict.h>
#include <q3intdict.h>
#include <q3cstring.h>
#include <q3strlist.h>
#include <qstringlist.h>
#include <q3dict.h>
#include <q3textstream.h>

//----------------------------------------------------------------------
// forward declarations
class EQPacketOPCodeDB;
class EQPacketPayload;

//----------------------------------------------------------------------
// Enumerated types

// EQSizeCheckType - type of sizecheck that EQPacketPayload will perform
enum EQSizeCheckType
{
  SZC_None = 0,             // No payload size check
  SZC_Match,                // Exact size match
  SZC_Modulus,              // Payload is a modulus of the type size
  SZC_Variable = SZC_None,  // Variable length packet (effectively none)
};


//----------------------------------------------------------------------
// EQPacketTypeDB
class EQPacketTypeDB
{
 public:
  EQPacketTypeDB();
  ~EQPacketTypeDB();
  
  size_t size(const char* typeName) const;
  bool valid(const char* typeName) const;
  void list(void) const;

 protected:
  void addStruct(const char* typeName, size_t);

  Q3AsciiDict<size_t> m_typeSizeDict;
};

//----------------------------------------------------------------------
// EQPacketDispatch
class EQPacketDispatch : public QObject
{
  Q_OBJECT
 public:
  EQPacketDispatch(QObject* parent = 0, const char* name = 0);
  virtual ~EQPacketDispatch();

  void activate(const uint8_t*, size_t, uint8_t);
  bool connect(const QObject* receiver, const char* member = 0);
  bool disconnect(const QObject* receiver, const char* member = 0);

 signals:
  void signal(const uint8_t*, size_t, uint8_t);

 protected:

 private:
  // disable copy constructor and operator=
  EQPacketDispatch(const EQPacketDispatch&);
  EQPacketDispatch& operator=(const EQPacketDispatch&);
};

//----------------------------------------------------------------------
// EQPacketPayload
class EQPacketPayload
{
 public:
  EQPacketPayload();
  ~EQPacketPayload();
  
  const Q3CString& typeName() const;
  bool setType(const EQPacketTypeDB& db, const char* typeName);
  size_t typeSize() const;
  EQSizeCheckType sizeCheckType() const;
  void setSizeCheckType(EQSizeCheckType sizeCheckType);
  uint8_t dir() const;
  void setDir(uint8_t dir);

  bool match(const uint8_t* data, size_t size, uint8_t dir) const;

 protected:
  Q3CString m_typeName;
  size_t m_typeSize;
  EQSizeCheckType m_sizeCheckType;
  uint8_t m_dir;
};

// Payload list typedef
typedef Q3PtrList<EQPacketPayload> EQPayloadList;
typedef Q3PtrListIterator<EQPacketPayload> EQPayloadListIterator;

inline const Q3CString& EQPacketPayload::typeName() const
{
  return m_typeName;
}

inline size_t EQPacketPayload::typeSize() const
{
  return m_typeSize;
}

inline EQSizeCheckType EQPacketPayload::sizeCheckType() const
{
  return m_sizeCheckType;
}

inline void EQPacketPayload::setSizeCheckType(EQSizeCheckType sizeCheckType)
{
  m_sizeCheckType = sizeCheckType;
}

inline uint8_t EQPacketPayload::dir() const
{
  return m_dir;
}

inline void EQPacketPayload::setDir(uint8_t dir)
{
  m_dir = dir;
}

//----------------------------------------------------------------------
// EQPacketOPCode
class EQPacketOPCode : public EQPayloadList
{
 public:
  EQPacketOPCode();
  EQPacketOPCode(uint16_t, const QString& name);
  EQPacketOPCode(const EQPacketOPCode& opcode);
  ~EQPacketOPCode();

  void setOPCode(uint16_t opcode);
  uint16_t opcode(void) const;
  void setImplicitLen(uint16_t len);
  uint16_t implicitLen(void) const;
  void setName(const QString& name);
  const QString& name() const;
  void setUpdated(const QString& updated);
  const QString& updated(void) const;
  void addComment(const QString& comment);
  bool removeComment(const QString& comment);
  void clearComments(void);
  const QStringList& comments() const;
  EQPacketPayload* find(const uint8_t* data, size_t size, uint8_t dir) const;

 protected:
  uint16_t m_opcode;
  uint16_t m_implicitLen;
  QString m_name;
  QString m_updated;
  QStringList m_comments;
};

inline void EQPacketOPCode::setOPCode(uint16_t opcode)
{
  m_opcode = opcode;
}

inline uint16_t EQPacketOPCode::opcode(void) const
{
  return m_opcode;
}

inline void EQPacketOPCode::setImplicitLen(uint16_t len)
{
  m_implicitLen = len;
}

inline uint16_t EQPacketOPCode::implicitLen(void) const
{
  return m_implicitLen;
}

inline void EQPacketOPCode::setName(const QString& name)
{
  m_name = name;
}

inline const QString& EQPacketOPCode::name() const
{
  return m_name;
}

inline void EQPacketOPCode::setUpdated(const QString& updated)
{
  m_updated = updated;
}

inline const QString& EQPacketOPCode::updated(void) const
{
  return m_updated;
}

inline void EQPacketOPCode::addComment(const QString& comment)
{
  // append comment to the end of the list
  m_comments.append(comment);
}

inline bool EQPacketOPCode::removeComment(const QString& comment)
{
  // find the comment
  QStringList::iterator it = m_comments.find(comment);

  // was the comment found?
  if (it != m_comments.end())
  {
    // yes, remove it and return success
    m_comments.remove(it);
    return true;
  }

  // comment not found, return failure
  return false;
}

inline void EQPacketOPCode::clearComments(void)
{
  // clear the comments
  m_comments.clear();
}

inline const QStringList& EQPacketOPCode::comments() const
{
  // return a const version of the comments object
  return m_comments;
}

//----------------------------------------------------------------------
// EQPacketOPCodeDB
class EQPacketOPCodeDB
{
 public:
  EQPacketOPCodeDB(int size);
  ~EQPacketOPCodeDB();

  bool load(const EQPacketTypeDB& typeDB, const QString& filename);
  bool save(const QString& filename);
  void list(void) const;
  void clear(void);
  EQPacketOPCode* add(uint16_t opcode, const QString& opcodeName);
  EQPacketOPCode* edit(uint16_t opcode);
  EQPacketOPCode* edit(const QString& opcodeName);
  bool remove(uint16_t opcode);
  bool remove(const QString& opcodeName);
  bool move(uint16_t oldOPCode, uint16_t newOPCode);
  bool move(const QString& oldOPCodeName, const QString& newOPCodeName);
  const EQPacketOPCode* find(uint16_t opcode) const;
  const EQPacketOPCode* find(const QString& opcodeName) const;
  const Q3IntDict<EQPacketOPCode> opcodes() const;

 protected:
  Q3IntDict<EQPacketOPCode> m_opcodes;
  Q3Dict<EQPacketOPCode> m_opcodesByName;
};

inline void EQPacketOPCodeDB::clear(void)
{
  m_opcodesByName.clear();
  m_opcodes.clear();
}

inline EQPacketOPCode* EQPacketOPCodeDB::edit(uint16_t opcode)
{
  // attempt to find the opcode object
  return m_opcodes.find(opcode);
}

inline EQPacketOPCode* EQPacketOPCodeDB::edit(const QString& name)
{
  // attempt to find the opcode object
  return m_opcodesByName.find(name);
}

inline const EQPacketOPCode* EQPacketOPCodeDB::find(uint16_t opcode) const
{
  // attempt to find the opcode object
  return m_opcodes.find(opcode);
}

inline const EQPacketOPCode* EQPacketOPCodeDB::find(const QString& opcode) const
{
  // attempt to find the opcode object
  return m_opcodesByName.find(opcode);
}

inline const Q3IntDict<EQPacketOPCode> EQPacketOPCodeDB::opcodes() const
{
  return m_opcodes;
}

#endif // _PACKETINFO_H_
