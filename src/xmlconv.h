/*
 * xmlconv.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef XMLCONV_H
#define XMLCONV_H

#include <qvariant.h>
#include <qdom.h>

class DomConvenience
{
 public:
  DomConvenience(QDomDocument& doc);
  bool elementToVariant(const QDomElement& elem, QVariant& var);
  bool variantToElement(const QVariant& var, QDomElement& elem);

 protected:
  bool getBoolFromString(const QString& s, bool& ok);
  int getBase(const QDomElement& e);
  QColor getColor(const QDomElement& e);
  QString boolString(bool b);
  void clearAttributes(QDomElement& e);

 private:
  QDomDocument& m_doc;
};

#endif // XMLCONV_H
