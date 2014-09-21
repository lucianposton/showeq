/*
 * eqstr.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002-2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */


#include "eqstr.h"
#include "diagnosticmessages.h"

#include <stdio.h>

#include <qregexp.h>
#include <qfile.h>
#include <qstringlist.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3CString>

EQStr::EQStr(int size)
  : m_messageStrings(size),
    m_loaded(false)
{
  // make sure strings get deleted
  m_messageStrings.setAutoDelete(true);
}

EQStr::~EQStr()
{
  // not really necessary, but just for completeness sake
  m_messageStrings.clear();
}

bool EQStr::load(const QString& fileName)
{
  // clear out any existing contents
  m_messageStrings.clear();

  // create a QFile on the file
  QFile formatFile(fileName);

  // open the file read only
  if (!formatFile.open(QIODevice::ReadOnly))
  {
    seqWarn("EQStr: Failed to open '%s'",
	    fileName.latin1());
    return false;
  }

  // allocate a QCString large enough to hold the entire file
  Q3CString textData(formatFile.size() + 1);
  
  // read in the entire file
  formatFile.readBlock(textData.data(), textData.size());
  
  // construct a regex to deal with either style line termination
  QRegExp lineTerm("[\r\n]{1,2}");
  
  // split the data into lines at the line termination
  QStringList lines = QStringList::split(lineTerm, 
					 QString::fromUtf8(textData), false);
  
  // start iterating over the lines
  QStringList::Iterator it = lines.begin();
  
  // first is the magic id string
  QString magicString = (*it++);
  int spc;
  uint32_t formatId;
  QString formatString;
  uint32_t maxFormatId = 0;
  
  // next skip over the count, etc...
  it++;
  
  // now iterate over the format lines
  for (; it != lines.end(); ++it)
  {
    // find the beginning space
    spc = (*it).find(' ');
    
    // convert the beginnign of the string to a ULong
    formatId = (*it).left(spc).toULong();
    
    if (formatId > maxFormatId) 
      maxFormatId = formatId;
    
    // insert the format string into the dictionary.
    m_messageStrings.insert(formatId, new QString((*it).mid(spc+1)));    
  }

  // note that strings are loaded
  m_loaded = true;

  seqInfo("Loaded %d message strings from '%s' maxFormat=%d",
	  m_messageStrings.count(), fileName.latin1(),
	  maxFormatId);
  
  return true;
}

QString EQStr::find(uint32_t formatid) const
{
  // attempt to find the message string
  QString* res = m_messageStrings.find(formatid);

  // if the message string was found, return it
  if (res)
    return *res;

  // otherwise return a NULL/Empty QString
  return QString();
}

QString EQStr::message(uint32_t formatid) const
{
  // attempt to find the message string
  QString* res = m_messageStrings.find(formatid);

  // if the message string was found, return it
  if (res)
    return *res;

  // otherwise return a fabricated string
  return QString("Unknown: ") + QString::number(formatid, 16);
}

QString EQStr::formatMessage(uint32_t formatid, 
			     const char* arguments, size_t argsLen) const
{
  QString* formatStringRes = m_messageStrings.find(formatid);

  QString tempStr;

  if (formatStringRes == NULL)
  {
    tempStr.sprintf( "Unknown: %04x: ",
		     formatid);
    tempStr += QString::fromUtf8(arguments);

    size_t totalArgsLen = strlen(arguments) + 1;
    
    const char* curMsg;
    while (totalArgsLen < argsLen)
    {
      curMsg = arguments + totalArgsLen;
      tempStr += QString(", ") + QString::fromUtf8(curMsg);
      totalArgsLen += strlen(curMsg) + 1;
    }
  }
  else
  {
    Q3ValueVector<QString> argList;
    argList.reserve(5); // reserve space for 5 elements to handle most common sizes
    
    // 
    size_t totalArgsLen = 0;
    const char* curArg;
    while (totalArgsLen < argsLen)
    {
      curArg = arguments + totalArgsLen;
      // insert argument into the argument list
      argList.push_back(QString::fromUtf8(curArg));
      totalArgsLen += strlen(curArg) + 1;
    }

    bool ok;
    int curPos;
    size_t substArg;
    int substArgValue;
    QString* substFormatStringRes;
    QString substFormatString;

    ////////////////////////////
    // replace template (%T) arguments in formatted string
    QString formatString = *formatStringRes;
    QRegExp rxt("%T(\\d{1,3})", true, false);

    // find first template substitution
    curPos = rxt.search(formatString, 0);

    while (curPos != -1)
    {
      substFormatStringRes = NULL;
      substArg = rxt.cap(1).toInt(&ok);
      if (ok && (substArg <= argList.size()))
      {
	substArgValue = argList[substArg-1].toInt(&ok);

	if (ok)
	  substFormatStringRes = m_messageStrings.find(substArgValue);
      }
      
      // replace template argument with subst string
      if (substFormatStringRes != NULL)
	formatString.replace(curPos, rxt.matchedLength(), *substFormatStringRes);
      else
	curPos += rxt.matchedLength(); // if no replacement string, skip over
      
      // find next substitution
      curPos = rxt.search(formatString, curPos);
    }

    ////////////////////////////
    // now replace substitution arguments in formatted string
    // NOTE: not using QString::arg() because not all arguments are always used
    //       and it will do screwy stuff in this situation
    QRegExp rx("%(\\d{1,3})", true, false);

    // find first template substitution
    curPos = rx.search(formatString, 0);

    while (curPos != -1)
    {
      substArg = rx.cap(1).toInt(&ok);

      // replace substitution argument with argument from list
      if (ok && (substArg <= argList.size()))
	formatString.replace(curPos, rx.matchedLength(), argList[substArg-1]);
      else
	curPos += rx.matchedLength(); // if no such argument, skip over

      // find next substitution
      curPos = rx.search(formatString, curPos);
    }

    return formatString;
  }

  return tempStr;
}
