/*
 * messagefilter.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2003-2007 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#include "messagefilter.h"

#include "main.h"

//----------------------------------------------------------------------
// MessageFilter
MessageFilter::MessageFilter(const QString& name, 
			     uint64_t matchTypes, const QRegExp& regexp)
  : m_name(name), 
    m_types(matchTypes),
    m_regexp(regexp)
{
}

MessageFilter::MessageFilter(uint64_t matchTypes, const QRegExp& regexp)
  : m_name(regexp.pattern()),
    m_types(matchTypes), 
    m_regexp(regexp)
{
}

MessageFilter::MessageFilter(const MessageFilter& mf)
  : m_name(mf.name()),
    m_types(mf.types()),
    m_regexp(mf.regexp())
{
}

MessageFilter::~MessageFilter()
{
}


//----------------------------------------------------------------------
// MessageFilters
MessageFilters::MessageFilters(QObject* parent, const char* name)
  : QObject(parent, name)
{
  QString section("MessageFilters");
  QString nameSuffix("Name");
  QString patternSuffix("Pattern");
  QString typesSuffix("Types");
  QString number;
  QString preferenceName;
  QString filterName;
  QRegExp regexp;
  uint64_t types;
  for (int i = 0; i < maxMessageFilters; i++)
  {
    number.setNum(i);

    // retrieve the filter name
    preferenceName = number + nameSuffix;
    filterName = pSEQPrefs->getPrefString(preferenceName, section);

    // if filtername is empty, then no filter in this slot
    if (filterName.isEmpty())
    {
      m_filters[i] = 0;
      continue;
    }
    
    // retrieve the filter pattern
    preferenceName = number + patternSuffix;
    regexp.setPattern(pSEQPrefs->getPrefString(preferenceName, section, "."));
    
    // retrieve the filter types
    preferenceName = number + typesSuffix;
    types = pSEQPrefs->getPrefUInt64(preferenceName, section, 0);
    
    // ok, create the filter with the retrieved information
    m_filters[i] = new MessageFilter(filterName, types, regexp);
  }
}

MessageFilters::~MessageFilters()
{
  for (int i = 0; i < maxMessageFilters; i++)
    delete m_filters[i];
}

uint8_t MessageFilters::addFilter(const MessageFilter& filter)
{
  // find a vacant filter slot
  for (int i = 0; i < maxMessageFilters; i++)
  {
    // once a vacant slot is found, add the filter there
    if (!m_filters[i])
    {
      // allocate the filter
      m_filters[i] = new MessageFilter(filter);

      QString section("MessageFilters");
      QString number = QString::number(i);
      
      pSEQPrefs->setPrefString(number + "Name", section, filter.name());
      pSEQPrefs->setPrefString(number + "Pattern", section, 
			       filter.regexp().pattern());
      pSEQPrefs->setPrefUInt64(number + "Types", section, filter.types());
      
      // signal the addition of the new filter
      emit added(1 << i, i, *m_filters[i]);

      // return the slot
      return i;
    }
  }

  // return 0xFF for failure
  return 0xFF;
}

bool MessageFilters::remFilter(const MessageFilter& filter)
{
  // iterate over the filters
  for (int i = 0; i < maxMessageFilters; i++)
  {
    // if a match is found, remove it
    if (m_filters[i] && (*m_filters[i] == filter))
      return remFilter(i);
  }

  return false;
}

bool MessageFilters::remFilter(uint8_t filter)
{
  // if no filter existed, then return false
  if (!m_filters[filter])
    return false;

  // delete the filter
  delete m_filters[filter];

  // clear the filter slot
  m_filters[filter] = 0;

  QString section("MessageFilters");
  QString number = QString::number(filter);

  pSEQPrefs->setPrefString(number + "Name", section, "");
  pSEQPrefs->setPrefString(number + "Pattern", section, "");
  pSEQPrefs->setPrefUInt64(number + "Types", section, 0);

  // signal the filters removal
  emit removed(1 << filter, filter);
  
  return true;
}

bool MessageFilters::remFilter(const QString& name)
{
  // iterate over the filters
  for (int i = 0; i < maxMessageFilters; i++)
  {
    // if a match is found, remove it
    if (m_filters[i] && (m_filters[i]->name() == name))
      return remFilter(i);
  }

  // no matching filter found
  return false;
}

uint32_t MessageFilters::filterMessage(uint64_t messageTypeMask, 
				       const QString& message)
{
  // initialize to no matches (0)
  uint32_t mask = 0;

  // iterate over the message fitlers
  for (int i = 0; i < maxMessageFilters; i++)
  {
    // if a match is found, add it to the mask
    if (m_filters[i] && m_filters[i]->isFiltered(messageTypeMask, message))
      mask |= 1 << i;
  }

  // return the filter mask
  return mask;
}

#ifndef QMAKEBUILD
#include "messagefilter.moc"
#endif

