/*
 * datalocationmgr.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2003 Zaphod (dohpaz@users.sourceforge.net). 
 *
 */

#ifndef _DATALOCATIONMGR_H_
#define _DATALOCATIONMGR_H_

#include <qstring.h>
#include <qdir.h>
#include <qfileinfo.h>

class DataLocationMgr
{
 public:
  DataLocationMgr(const QString& homeSubDir);
  ~DataLocationMgr();
  bool setupUserDirectory();
  QFileInfo findExistingFile(const QString& subdir, const QString& filename,
			     bool caseSensitive = false, 
			     bool preferUser = true) const;
  QFileInfo findWriteFile(const QString& subdir, const QString& filename,
			  bool caseSensitive = true,
			  bool preferUser = true) const;

  QDir pkgDataDir(const QString& subdir) const;
  QDir userDataDir(const QString& subdir) const;

 protected:
  QFileInfo findFile(const QString& dir1, const QString& dir2,
		     const QString& subdir, const QString& filename,
		     bool caseSensitive = false) const;
  QFileInfo findFile(const QDir& dir, const QString& filename, 
		     bool caseSensitive = false, bool writable = false) const;
  QFileInfo findWriteFile(const QString& dir1, const QString& dir2,
			  const QString& subdir, const QString& filename,
			  bool caseSensitive = false) const;
  QDir findOrMakeSubDir(const QString& dir, const QString& subdir) const;

  QString m_pkgData;
  QString m_userData;
};

#endif // _DATALOCATIONMGR_H_


