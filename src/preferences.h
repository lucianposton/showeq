// preferences.h
//		SINS Distributed under GPL

#ifndef __PREFERENCE_H__
#define __PREFERENCE_H__

// PreferenceFile is a generic class that implements a persistant config
// file in the format of:
//
// [Section1name]                 <-- start of similarly grouped options
// Identifier      value          <-- whitespace between Ident / value
//                                <-- empty line ends section
// [Section2name]                 <-- new section
// Identifier      value          <-- new ident/value pair
//
// Note:  if 'value' needs to contain spaces, enclose it in "quotes".
//        Upon saving to file, values are updated to the new settings as
//        they appear in the file.  If an item does not exist in the file 
//        it will be created with a comment as to the date and time created 
//        Items can also appear in list as 'SectionName_Identifier'. 
//
//  The following public members are available for use
//
//    save()             - Saves all modified cache values to file
//    load()             - Loads values from file into cache
//    revert()           - Reloads values from file into cache (looses mods)
//    getPrefBool(...)   - Get a boolean value from cache (accepts default)
//    getPrefInt(...)    - Get an int value from cache (accepts default)
//    getPrefString(...) - Get a string from cache (accepts default, staticstr)
//    isPreference(...)  - Returns true if a preference exists in cache
//    setPrefValue(...)  - Sets a numerical value in cache
//    setPrefText(...)   - Set a string value in cache
//
//  Note:  Both 'Set' members allow a 'persistent' flag to be passed in
//         which is defaulted to TRUE.  Setting this to FALSE makes that 
//         preference not get saved upon a save.  Any later Set with a 
//         TRUE value overrides this.  This is usefull for command line args 
//         which should override config file prefs but not overwrite them  

#include <qobject.h>
#include <qasciidict.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

class NameValuePair
{
public:
	enum NameValueFlag {
		k_none =		0x00000000,
		k_loaded =		0x00000001,
		k_modified =	0x00000002,
		k_persistent = 	0x00000004
	};

	NameValuePair( const QString& name, const QString& value, bool persistent = true );
	
	const QString& value() const { return m_value; }
	const QString& name() const { return m_name; }

	void setValue( const QString& inValue ) { m_value = inValue; }
	void setName( const QString& inValue ) { m_value = inValue; }
		
	void setFlag( NameValueFlag flag ) { m_flags |= flag; }
	void clearFlag( NameValueFlag flag ) { m_flags &= ~flag; }
	bool isFlagSet( NameValueFlag flag ) const { return ( ( m_flags & flag ) != 0 ); }
	 
	bool loaded() const { return this->isFlagSet( k_loaded ); }
	bool modified() const { return this->isFlagSet( k_modified ); }
	bool persistent() const { return this->isFlagSet( k_persistent ); }

protected:
	QString			m_value;
	QString			m_name;
	unsigned long	m_flags;
};

class PreferenceSection
{
public:
	PreferenceSection( const QString& sectionName );
	~PreferenceSection();
	
	const QString& sectionName() { return m_sectionName; }
	
	bool exists( const QString& name );
	bool exists( const NameValuePair& nvPair );
	
	bool getPrefValue( const QString& name, QString& string, const QString& inDefault = QString::null );
	void setPrefValue( const QString& name, const QString& value, bool persistent = true );

	QAsciiDict<NameValuePair>& getPrefs() { return m_list; }
protected:
//	void deletePref( const QString& name );
//	void writeToStream( QTextStream& stream );
	
	friend class			PreferenceFile;
	
	QString						m_sectionName;
	QAsciiDict<NameValuePair>	m_list;
};

// PreferenceFile
// A File comprised of NameValuePair Items
// This is a textfile implementation in the format:
//
// [Section1Identifier]
// PreferenceItem       PreferenceValue
//
// [Section2Identifier]
// PreferenceItem       PreferenceValue

class PreferenceFile: public QObject
{
Q_OBJECT

public:
	PreferenceFile( const QString& inFileName );

	bool isPreference( const QString& inName, const QString& inSection );

	bool getPrefValue( const QString& inName, const QString& inSection, QString& value, const QString& outDefault = QString::null );
	bool getPrefValue( const QString& inName, const QString& inSection, int& storage, int outDefault = -1 );
	bool getPrefBool( const QString& inName, const QString& inSection, bool def = false );
	int getPrefInt( const QString& inName, const QString& inSection, int def = -1 );
	QString getPrefString( const QString& inName, const QString& inSection, const QString& outDefault = QString::null );
	
	void setPrefValue( const QString& inName, const QString& inSection, const QString& inValue, bool per = true );
	void setPrefBool( const QString& inName, const QString& inSection, bool inValue, bool per = true );
	void setPrefInt( const QString& inName, const QString& inSection, int inValue, bool per = true );
	void setPrefString( const QString& inName, const QString& inSection, const QString& inValue = QString::null, bool per = true );

public slots:
	void save();
	void load();
	void revert();

protected:
	void removeNonPersistentPrefs( QAsciiDict<NameValuePair>& list );
	void writeToStream( const QAsciiDict<NameValuePair>& items, QTextStream& outStream );
	void loadLine( const QString& inputLine, QString& name, QString& value );
	
	void printContents();
	
	QAsciiDict<PreferenceSection>	m_sections;
	QString							m_filename;
	bool							m_modified;
};

#endif

