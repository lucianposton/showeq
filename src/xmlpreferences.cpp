/*
 * xmlconv.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002-2003 Zaphod (dohpaz@users.sourceforge.net). 
 *     All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */


#include <qfile.h>
#include <qnamespace.h>
#include <qaccel.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#if QT_VERSION > 300
#include <qkeysequence.h>
#endif
#include <qtextstream.h>

#include "xmlpreferences.h"
#include "xmlconv.h"

const float seqPrefVersion = 1.0;
const char* seqPrefName = "seqpreferences";
const char* seqPrefSysId = "seqpref.dtd";
const int sectionHashSize = 31; // must be a prime number
const int preferenceHashSize = 31; // must be a prime number

XMLPreferences::XMLPreferences(const QString& defaultsFileName, 
			       const QString& inFileName)
  : m_defaultsFilename(defaultsFileName),
    m_filename(inFileName), 
    m_modified(0),
    m_runtimeSections(sectionHashSize),
    m_userSections(sectionHashSize),
    m_defaultsSections(preferenceHashSize)
{
  m_templateDoc.sprintf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE %s SYSTEM \"%s\">\n"
			"<seqpreferences version=\"%1.1f\">\n"
			"<!-- ============================================================= -->"
			"</seqpreferences>\n",
			seqPrefName, seqPrefSysId, seqPrefVersion);

  // automatically delete removed sections
  m_userSections.setAutoDelete(true);
  m_defaultsSections.setAutoDelete(true);
  
  // load the preferences
  load();
}

XMLPreferences::~XMLPreferences()
{
}

void XMLPreferences::load()
{
  // load the default preferences
  loadPreferences(m_defaultsFilename, m_defaultsSections);

  // load the user preferences
  loadPreferences(m_filename, m_userSections);
}

void XMLPreferences::save()
{
  // save the user preferences iff they've changed
  if (m_modified & User)
    savePreferences(m_filename, m_userSections);

  // save the user preferences iff they've changed
  if (m_modified & Defaults)
    savePreferences(m_defaultsFilename, m_defaultsSections);
}

void XMLPreferences::revert()
{
  // clear out all default preferecnes
  m_defaultsSections.clear();

  // clear out all user preferences
  m_userSections.clear();

  // load the default preferences
  loadPreferences(m_defaultsFilename, m_defaultsSections);

  // load the user preferences back in from the file
  loadPreferences(m_filename, m_userSections);
}

void XMLPreferences::loadPreferences(const QString& filename, 
				     PrefSectionDict& dict)
{
  QDomDocument doc(seqPrefName);
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
  {
    qWarning("Unable to open file: %s!", 
	     (const char*)filename);
    return;
  }

#if QT_VERSION < 300
  if (!doc.setContent(&f))
  {
    qWarning("Unable to set preference document to contents of file: %s!", 
	     (const char*)filename);
    f.close();
    return;
  }
#else
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;
  if (!doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn))
  {
    qWarning("Error processing file: %s!\n\t %s on line %d in column %d!", 
	     (const char*)filename, 
	     (const char*)errorMsg, errorLine, errorColumn);
    f.close();
    return;
  }
#endif

  // do more processing here
 QDomElement docElem = doc.documentElement();
 DomConvenience conv(doc);
 QDomNodeList sectionList, propertyList;
 PreferenceDict* sectionDict;
 CommentDict* commentSectionDict;
 QString comment;
 QString* commentVal;
 QDomElement section;
 QDomElement property;
 QString sectionName;
 QString propertyName;
 QDomNode n;
 QDomElement valueElement;
 bool foundValue;

 sectionList = doc.elementsByTagName("section");
 for (uint i = 0; i < sectionList.length(); i++)
 {
   section = sectionList.item(i).toElement();
   if (!section.hasAttribute("name"))
   {
     qWarning("section without name!");
     continue;
   }

   sectionName = section.attribute("name");

   // see if the section exists in the dictionary
   sectionDict = dict.find(sectionName);

   // if not, then create it
   if (sectionDict == NULL)
   {
     // create the new preference dictionary
     sectionDict = new PreferenceDict(preferenceHashSize);

     // make sure the dictionary deletes removed properties
     sectionDict->setAutoDelete(true);

     // insert the preference dictionary into the section
     dict.insert(sectionName, sectionDict);
   }

   // see if comment section exists in the dictionary
   commentSectionDict = m_commentSections.find(sectionName);

   // if not, then create it
   if (commentSectionDict == NULL)
   {
     // create the new preference dictionary
     commentSectionDict = new CommentDict(preferenceHashSize);

     // make sure the dictionary deletes removed properties
     commentSectionDict->setAutoDelete(true);

     // insert the preference dictionary into the section
     m_commentSections.insert(sectionName, commentSectionDict);
   }

   propertyList = section.elementsByTagName("property");
   
   for (uint j = 0; j < propertyList.length(); j++)
   {
     property = propertyList.item(j).toElement();
     if (!property.hasAttribute("name"))
     {
       qWarning("property in section '%s' without name! Ignoring!",
		(const char*)sectionName);
       continue;
     }

     propertyName = property.attribute("name");

     foundValue = false;

     QVariant value;
     // iterate over the nodes under the property
     for (n = property.firstChild(); !n.isNull(); n = n.nextSibling())
     {
       if (!n.isElement())
	 continue;

       valueElement = n.toElement();

       if (valueElement.tagName() == "comment")
       {
	 // get comment if any
	 comment = valueElement.text();

	 // if there is a comment, cache it
         if (!comment.isEmpty())
         {
	   commentVal = commentSectionDict->find(propertyName);
	   
	   if (commentVal != NULL)
	     *commentVal = comment;
	   else
	     commentSectionDict->insert(propertyName, 
					new QString(comment));
	 }

	 continue;
       }

       if (!conv.elementToVariant(valueElement, value))
       {
	 qWarning("property '%s' in section '%s' with bogus value in tag '%s'!"
		  " Ignoring!",
		  (const char*)propertyName, (const char*)sectionName,
		  (const char*)valueElement.tagName());
	 
	 continue;
       }

       // found the value
       foundValue = true;
       
       // insert value into the section dictionary
       sectionDict->insert(propertyName, new QVariant(value));
       
       break;
     }

#if 0 // ZBTEMP : Support properties without values to get comments?
     if (!foundValue)
     {
       qWarning("property '%s' in section '%s' without value! Ignoring!",
		(const char*)propertyName, (const char*)sectionName);
       continue;
     }
#endif
   }
 }

  // close the file
  f.close();

#if 1 // ZBTEMP
  printf("Loaded preferences file: %s!\n", (const char*)filename);
#endif
}

void XMLPreferences::savePreferences(const QString& filename, 
				     PrefSectionDict& dict)
{
  // open the existing preference file
  QDomDocument doc;
  QFile f(filename);
  bool loaded = false;
  if (f.open(IO_ReadOnly))
  {
#if QT_VERSION < 300
    if (doc.setContent(&f))
      loaded = true;
    else
    {
      qWarning("Unable to set preference document to contents of file: %s!", 
	       (const char*)filename);
    }
#else
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    if (doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn))
      loaded = true;
    else
    {
      qWarning("Error processing file: %s!\n\t %s on line %d in column %d!", 
	       (const char*)filename, 
	       (const char*)errorMsg, errorLine, errorColumn);

    }
#endif

    // close the file
    f.close();
  }

  // if no file was loaded, use the template document
  if (!loaded)
  {
#if QT_VERSION < 300
    if (doc.setContent(m_templateDoc))
      loaded = true;
#else
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    if (doc.setContent(m_templateDoc, false, &errorMsg, &errorLine, &errorColumn))
      loaded = true;
#endif
  }

  // if there was an existing file, rename it
  QFileInfo fileInfo(filename);
  if (fileInfo.exists())
  {
    QDir dir(fileInfo.dirPath(true));

    dir.rename(filename, filename + QString(".bak"));
  }

  // do more processing here
  QDomElement docElem = doc.documentElement();
  DomConvenience conv(doc);
  QDomNodeList sectionList, propertyList;
  PreferenceDict* sectionDict;
  QString sectionName;
  QString propertyName;
  QVariant* propertyValue;
  QDomElement e;
  QDomNode n;

  sectionList = docElem.elementsByTagName("section");

  QDictIterator<PreferenceDict> sdit(dict);
  for (; sdit.current(); ++sdit)
  {
    QDomElement section;
    sectionName = sdit.currentKey();
    sectionDict = sdit.current();

    // iterate over all the sections in the document
    for (uint i = 0; i < sectionList.length(); i++)
    {
      e = sectionList.item(i).toElement();
      if (!e.hasAttribute("name"))
      {
	qWarning("section without name!");
	continue;
      }

      //      printf("found section: %s\n", (const char*)section.attribute("name"));

      // is this the section?
      if (sectionName == e.attribute("name"))
      {
	// yes, done
	section = e;
	break;
      }
    }

    // if no section was found, create a new one
    if (section.isNull())
    {
      // create the section element
      section = doc.createElement("section");

      // set the name attribute of the section element
      section.setAttribute("name", sectionName);

      // append the new section to the document
      docElem.appendChild(section);
    }

    // iterate over all the properties in the section
    QDictIterator<QVariant> pdit(*sectionDict);
    for (; pdit.current(); ++pdit)
    {
      QDomElement property;
      propertyName = pdit.currentKey();
      propertyValue = pdit.current();

      // get all the property elements in the section
      propertyList = section.elementsByTagName("property");

      // iterate over all the property elements until a match is found
      for (uint j = 0; j < propertyList.length(); j++)
      {
	e = propertyList.item(j).toElement();
	if (!e.hasAttribute("name"))
	{
	  qWarning("property in section '%s' without name! Ignoring!",
		   (const char*)sectionName);
	  continue;
	}

	// is this the property being searched for?
	if (propertyName == e.attribute("name"))
	{
	  // yes, done
	  property = e;
	  break;
	}
      }

      // if no property was found, create a new one
      if (property.isNull())
      {
	// create the property element
	property = doc.createElement("property");

	// set the name attribute of the property element
	property.setAttribute("name", propertyName);

	// append the new property to the section
	section.appendChild(property);
      }

      QDomElement value;

      // iterate over the children
      for (n = property.firstChild();
	   !n.isNull();
	   n = n.nextSibling())
      {
	if (!n.isElement())
	  continue;

	// don't replace comments
	if (n.toElement().tagName() == "comment")
	  continue;

	// convert it to an element
	value = n.toElement();
	break;
      }

      // if no value element was found, create a new one
      if (value.isNull())
      {
	// create the value element, bogus type, will be filled in later
	value = doc.createElement("bogus");
	
	// append the new value to the property
	property.appendChild(value);
      }

      if (!conv.variantToElement(*propertyValue, value))
      {
	qWarning("Unable to set value element in section '%s' property '%s'!",
		 (const char*)propertyName, (const char*)propertyName);
      }
    }
  }

  // write the modified DOM to disk
  if (!f.open(IO_WriteOnly))
  {
    qWarning("Unable to open file for writing: %s!", 
	     (const char*)filename);
  }

  // open a Text Stream on the file
  QTextStream out(&f);

  // make sure stream is UTF8 encoded
  out.setEncoding(QTextStream::UnicodeUTF8);

  // save the document to the text stream
  QString docText;
  QTextStream docTextStream(&docText, IO_WriteOnly);
  doc.save(docTextStream, 4);

  // put newlines after comments (which unfortunately Qt's DOM doesn't track)
  QRegExp commentEnd("-->");
  docText.replace(commentEnd, "-->\n");

  // write the fixed document out to the file
  out << docText;

  // close the file
  f.close();

  printf("Finished saving preferences to file: %s\n",
	 (const char*)filename);
}

QVariant* XMLPreferences::getPref(const QString& inName, 
				  const QString& inSection,
				  Persistence pers)
{
  PreferenceDict* sectionDict;
  QVariant* preference = NULL;

  if (pers & Runtime)
  {
    // see if the section exists in the dictionary
    sectionDict = m_runtimeSections.find(inSection);

    // if so, then see if the preference exists
    if (sectionDict != NULL)
    {
      preference = sectionDict->find(inName);
      if (preference != NULL)
	return preference;
    }
  }

  if (pers & User)
  {
    // see if the section exists in the dictionary
    sectionDict = m_userSections.find(inSection);
    
    // if so, then see if the preference exists
    if (sectionDict != NULL)
    {
      preference = sectionDict->find(inName);
      if (preference != NULL)
	return preference;
    }
  }

  if (pers & Defaults)
  {
    // see if the section exists in the defaults dictionary
    sectionDict = m_defaultsSections.find(inSection);
    
    // if so, then see if the preferences exists
    if (sectionDict != NULL)
    {
      preference = sectionDict->find(inName);
      if (preference != NULL)
	return preference;
    }
  }

  return preference;
}

void XMLPreferences::setPref(const QString& inName, const QString& inSection, 
			     const QVariant& inValue, Persistence pers)
{
  // set the preference in the appropriate section
  if (pers & Runtime)
    setPref(m_runtimeSections, inName, inSection, inValue);
  if (pers & User)
    setPref(m_userSections, inName, inSection, inValue);
  if (pers & Defaults)
    setPref(m_defaultsSections, inName, inSection, inValue);

  m_modified |= pers;
}

void XMLPreferences::setPref(PrefSectionDict& dict,
			     const QString& inName, const QString& inSection, 
			     const QVariant& inValue)
{
  PreferenceDict* sectionDict;
  QVariant* preference;

   // see if the section exists in the dictionary
  sectionDict = dict.find(inSection);

   // if not, then create it
  if (sectionDict == NULL)
  {
     // create the new preference dictionary
     sectionDict = new PreferenceDict(preferenceHashSize);

     // make sure the dictionary deletes removed properties
     sectionDict->setAutoDelete(true);

     // insert the preference dictionary into the section
     dict.insert(inSection, sectionDict);
  }

  preference = sectionDict->find(inName);

  // if preference exists, change it, otherwise create it
  if (preference != NULL)
    *preference = inValue;
  else
    sectionDict->insert(inName, new QVariant(inValue));
}

QString XMLPreferences::getPrefComment(const QString& inName, const QString& inSection)
{
 CommentDict* commentSectionDict;

 // see if comment section exists in the dictionary
 commentSectionDict = m_commentSections.find(inSection);

 if (commentSectionDict == NULL)
   return QString("");

 QString* comment = commentSectionDict->find(inName);
 
 if (comment != NULL)
   return *comment;

 return QString("");
}

bool XMLPreferences::isSection(const QString& inSection, 
			       Persistence pers)
{
  PreferenceDict* sectionDict;

  if (pers & Runtime)
  {
    // see if the section exists in the dictionary
    sectionDict = m_runtimeSections.find(inSection);

    // if so, then see if the preference exists
    if (sectionDict != NULL)
      return true;
  }

  if (pers & User)
  {
    // see if the section exists in the dictionary
    sectionDict = m_userSections.find(inSection);
    
    // if so, then see if the preference exists
    if (sectionDict != NULL)
      return true;
  }

  if (pers & Defaults)
  {
    // see if the section exists in the defaults dictionary
    sectionDict = m_defaultsSections.find(inSection);
    
    // if so, then see if the preferences exists
    if (sectionDict != NULL)
      return true;
  }

  return false;
}
			       
bool XMLPreferences::isPreference(const QString& inName, 
				  const QString& inSection,
				  Persistence pers)
{
  // try to retrieve the preference
  QVariant* preference = getPref(inName, inSection, pers);

  return (preference != NULL);
}

// easy/sleezy way to create common get methods
#define getPrefMethod(retType, Type, Def) \
  retType XMLPreferences::getPref##Type(const QString& inName, \
                                        const QString& inSection, \
                                        Def def, \
                                        Persistence pers) \
  { \
    QVariant* preference = getPref(inName, inSection, pers); \
    \
    if (preference) \
      return preference->to##Type(); \
    \
    return def; \
  } 

// implement common get methods
getPrefMethod(QString, String, const QString&);
getPrefMethod(int, Int, int);
getPrefMethod(uint, UInt, uint);
getPrefMethod(double, Double, double);
getPrefMethod(bool, Bool, bool);
getPrefMethod(QColor, Color, const QColor&);
getPrefMethod(QPen, Pen, const QPen&);
getPrefMethod(QBrush, Brush, const QBrush&);
getPrefMethod(QPoint, Point, const QPoint&);
getPrefMethod(QRect, Rect, const QRect&);
getPrefMethod(QSize, Size, const QSize&);
getPrefMethod(QFont, Font, const QFont&);
getPrefMethod(QSizePolicy, SizePolicy, const QSizePolicy&);
getPrefMethod(QCursor, Cursor, const QCursor&);
getPrefMethod(QStringList, StringList, const QStringList&);

// implement get methods that require special behavior
int XMLPreferences::getPrefKey(const QString& inName, 
			       const QString& inSection, 
			       const QString& def, 
			       Persistence pers)
{
  return getPrefKey(inName, inSection, 
		    QAccel::stringToKey(def) & ~Qt::UNICODE_ACCEL, pers);
}

int XMLPreferences::getPrefKey(const QString& inName, 
			       const QString& inSection, 
			       int def, 
			       Persistence pers)
{
  // try to retrieve the preference
  QVariant* preference = getPref(inName, inSection, pers);

  // if preference was retrieved, return it as a string
  if (preference != NULL)
  {
    int key = def;

    switch(preference->type())
    {
#if QT_VERSION >= 300
    case QVariant::KeySequence:
      key = preference->toInt();
      break;
#endif
    case QVariant::String:
      // convert it to a key
      key = QAccel::stringToKey(preference->toString());
      break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
      key = preference->toInt();
      break;
    default:
      qWarning("XMLPreferences::getPrefKey(%s, %s, %d): preference found,\n"
	       "\tbut type %s is not convertable to type key!",
	       (const char*)inName, (const char*)inSection, def,
	       preference->typeName());
    }

    // fix the key code (deal with Qt brain death)
    key &= ~Qt::UNICODE_ACCEL;

    // return the key
    return key;
  }

  // return the default value
  return def;
}


int64_t XMLPreferences::getPrefInt64(const QString& inName, 
				     const QString& inSection, 
				     int64_t def, 
				     Persistence pers)
{
  // try to retrieve the preference
  QVariant* preference = getPref(inName, inSection, pers);

  // if preference was retrieved, return it as a string
  if (preference != NULL)
  {
    int64_t value = def;

    switch(preference->type())
    {
    case QVariant::String:
      // convert it to a int64_t (in base 16)
      value = strtoll(preference->toString(), 0, 16);
      break;
    case QVariant::Int:
    case QVariant::UInt:
      value = preference->toInt();
      break;
    case QVariant::Double:
      value = int64_t(preference->toDouble());
      break;
    case QVariant::ByteArray:
      {
	QByteArray& ba = preference->asByteArray();
	if (ba.size() == sizeof(int64_t))
	  value = *(int64_t*)ba.data();
	break;
      }
    default:
      qWarning("XMLPreferences::getPrefInt64(%s, %s, %lld): preference found,\n"
	       "\tbut type %s is not convertable to type int64_t!",
	       (const char*)inName, (const char*)inSection, def,
	       preference->typeName());
    }

    // return the key
    return value;
  }

  // return the default value
  return def;
}

uint64_t XMLPreferences::getPrefUInt64(const QString& inName, 
				       const QString& inSection, 
				       uint64_t def, 
				       Persistence pers)
{
  // try to retrieve the preference
  QVariant* preference = getPref(inName, inSection, pers);

  // if preference was retrieved, return it as a string
  if (preference != NULL)
  {
    uint64_t value = def;

    switch(preference->type())
    {
    case QVariant::String:
      // convert it to a uint64_t (in base 16)
      value = strtoull(preference->toString(), 0, 16);
      break;
    case QVariant::Int:
    case QVariant::UInt:
      value = preference->toInt();
      break;
    case QVariant::Double:
      value = uint64_t(preference->toDouble());
      break;
    case QVariant::ByteArray:
      {
	QByteArray& ba = preference->asByteArray();
	if (ba.size() == sizeof(uint64_t))
	  value = *(uint64_t*)ba.data();
	break;
      }
    default:
      qWarning("XMLPreferences::getPrefUInt64(%s, %s, %lld): preference found,\n"
	       "\tbut type %s is not convertable to type uint64_t!",
	       (const char*)inName, (const char*)inSection, def,
	       preference->typeName());
    }

    // return the key
    return value;
  }

  // return the default value
  return def;
}


QVariant XMLPreferences::getPrefVariant(const QString& inName, 
					const QString& inSection,
					const QVariant& outDefault,
					Persistence pers)
{
  // try to retrieve the preference
  QVariant* preference = getPref(inName, inSection, pers);

  // if preference was retrieved, return it as a string
  if (preference)
    return *preference;

  // return the default value
  return outDefault;
}

// generic common set preference method
#define setPrefMethod(Type, In) \
  void XMLPreferences::setPref##Type(const QString& inName, \
				     const QString& inSection, \
				     In inValue, \
				     Persistence pers) \
  { \
    setPref(inName, inSection, QVariant(inValue), pers); \
  } 

// create the methods for the types that can be handled the common way
setPrefMethod(String, const QString&);
setPrefMethod(Int, int);
setPrefMethod(UInt, uint);
setPrefMethod(Double, double);
setPrefMethod(Color, const QColor&);
setPrefMethod(Pen, const QPen&);
setPrefMethod(Brush, const QBrush&);
setPrefMethod(Point, const QPoint&);
setPrefMethod(Rect, const QRect&);
setPrefMethod(Size, const QSize&);
setPrefMethod(Font, const QFont&);
setPrefMethod(SizePolicy, const QSizePolicy&);
setPrefMethod(StringList, const QStringList&);

void XMLPreferences::setPrefBool(const QString& inName, 
				 const QString& inSection,
				 bool inValue,
				 Persistence pers)
{
  setPref(inName, inSection, QVariant(inValue, 0), pers);
}

void XMLPreferences::setPrefKey(const QString& inName,
				const QString& inSection,
				int inValue,
				Persistence pers)
{
#if QT_VERSION < 300
  setPref(inName, inSection, QVariant(QAccel::keyToString(inValue)), pers);
#else
  setPref(inName, inSection, QVariant(QKeySequence(inValue)), pers);
#endif
}

void XMLPreferences::setPrefInt64(const QString& inName,
				  const QString& inSection,
				  int64_t inValue,
				  Persistence pers)
{
  QByteArray ba;
  ba.duplicate((const char*)&inValue, sizeof(int64_t));
  setPref(inName, inSection, ba, pers);
}


void XMLPreferences::setPrefUInt64(const QString& inName,
				   const QString& inSection,
				   uint64_t inValue,
				   Persistence pers)
{
  QByteArray ba;
  ba.duplicate((const char*)&inValue, sizeof(uint64_t));
  setPref(inName, inSection, ba, pers);
}

void XMLPreferences::setPrefVariant(const QString& inName, 
				    const QString& inSection,
				    const QVariant& inValue, 
				    Persistence pers)
{
  setPref(inName, inSection, inValue, pers);
}

