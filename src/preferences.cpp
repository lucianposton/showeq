// preferences.cpp
//	SINS distributed under GPL

// PreferenceFile
// A file comprised of name-value pairs
// This is a textfile implementation in the format:
//
// [Section1Identifier]
// PreferenceItem       PreferenceValue
//
// [Section2Identifier]
// PreferenceItem       PreferenceValue
//

// PreferenceFile is a generic class that implements a persistant config
// file in the format of:
//
// [Section1name]                 <-- start of similarly grouped options
// Identifier      value          <-- whitespace between Ident / value
//                                <-- empty line ends section
// [Section2name]                 <-- new section
// Identifier      value          <-- new ident/value pair
//
// To use this class inherit it and use the following member fuctions:
//    save()             - Saves all modified cache values to file
//    load()             - Loads values from file into cache
//    revert()           - Reloads values from file into cache (looses mods)
//    setPrefValue(...)  - Set an integer value persistant preference
//    setPrefValue(...)  - Get an integer value persistant preference
//    setPrefText(...)   - Set a string string persistant preference
//    getPrefText(...)   - Get a string value persistant preference
//
//    The 'set' methods take an option argument (defaults to TRUE) called
//    'persistent'.  If this is set to FALSE the changed value will not be
//    saved to the config file (unless set by another caller to persistent).
//    This is usefull if you have something like command line arguments which
//    should override the config file settings but not be saved.

// implementation notes --
//	we store the preferences as set of name-value pairs
//	each name-value pair is stored in a section dictionary (PreferenceSection)
//	all the sections are stored in a dictionary in the PreferenceFile object

#include <stdlib.h>
#include <string.h>
#include <ctype.h>   // isspace()
#include <stdio.h>   // fopen()
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "preferences.h"

void printContents( const QString& name, QAsciiDict<NameValuePair>& list )
{
	NameValuePair*	pref;
	QAsciiDictIterator<NameValuePair>	it( list );
	int				i = 1;

	printf( "Section: %s  -  %d items\n", (const char*)name, list.count() );

	while ( ( pref = it.current() ) )
	{
		printf( "%02d: Name '%s' Value '%s'\n", i, (const char*)pref->name(), (const char*)pref->value() );
		i++;
		++it;
	}
}

NameValuePair::NameValuePair( const QString& name, const QString& value, bool persistent )
{
	m_name = name;
	m_value = value;
	
	m_flags = k_none;
	if ( persistent )
		this->setFlag( k_persistent );

	m_name.stripWhiteSpace();
	m_name.replace( QRegExp( "\\s" ), "" );
	m_value.stripWhiteSpace();
}

PreferenceSection::PreferenceSection( const QString& name )
{
	m_sectionName = name;
}

PreferenceSection::~PreferenceSection()
{
	m_list.clear();
}

bool PreferenceSection::exists( const QString& name )
{
	const NameValuePair* nvPair;
	
	nvPair = m_list.find( name );
	if ( nvPair )
		return true;
	return false;
}	

bool PreferenceSection::exists( const NameValuePair& nvPair )
{
	return ( this->exists( nvPair.name() ) );
}

bool PreferenceSection::getPrefValue( const QString& name, QString& value, const QString& inDefault )
{
	const NameValuePair*		pref;
	
	pref = m_list.find( name );
	if ( pref )
	{
		value = pref->value();
		return true;
	}
	
	value = inDefault;
	return false;
}

void PreferenceSection::setPrefValue( const QString& name, const QString& value, bool persistent )
{
	NameValuePair*		pref;
	
	pref = m_list.find( name );
	if ( pref )
	{
		if ( persistent )
			pref->setFlag( NameValuePair::k_persistent );
		else
			pref->clearFlag( NameValuePair::k_persistent );
		pref->setValue( value );
	}
	else
	{
		pref = new NameValuePair( name, value, persistent );
		m_list.insert( name, pref );
	}
}

	
/*
void PreferenceSection::deletePref( const QString& name )
{
	m_list.remove( name );
}

void PreferenceSection::writeToStream( QTextStream& outStream )
{
	QAsciiDictIterator<NameValuePair>		iter( m_list );
					
	NameValuePair*		nvPair;
					
	while ( ( nvPair = iter.current() ) )
	{
		++iter;
		
		outStream << nvPair->name();
		
		int		nameLength = nvPair->name().length();
		while ( ++nameLength < 20 )
			outStream << ' ';
		outStream << nvPair->value();
		outStream << '\n';
	}
}
*/

void PreferenceFile::writeToStream( const QAsciiDict<NameValuePair>& items, QTextStream& outStream )
{
	QAsciiDictIterator<NameValuePair>		iter( items );
					
	NameValuePair*		nvPair;
					
	while ( ( nvPair = iter.current() ) )
	{
		++iter;
		
		outStream << nvPair->name();
		
		int		nameLength = nvPair->name().length();
		while ( ++nameLength < 19 )
			outStream << ' ';
		outStream << ' ';
		outStream << nvPair->value();
		outStream << '\n';
	}
}




PreferenceFile::PreferenceFile( const QString& inFileName )
: QObject( NULL, "preference-file" ),
 m_sections( 199, false )
{
	m_filename = inFileName;
	this->load();
	m_modified = false;
}

bool PreferenceFile::isPreference( const QString& inName, const QString& inSection )
{
	PreferenceSection*	section = m_sections.find( inSection );
	
	if ( section )
		return ( section->exists( inName ) );
	return false;
}

bool PreferenceFile::getPrefValue( const QString& inName, const QString& inSection, QString& outValue, const QString& inDefault )
{
	PreferenceSection*	section = m_sections.find( inSection );
	
	if ( section )
	{
		QString		tmp;
	
		if ( section->getPrefValue( inName, tmp, inDefault ) )
		{
			outValue = tmp;
			return true;
		}
	}
	outValue = inDefault;	
	return false;
}

bool PreferenceFile::getPrefValue( const QString& inName, const QString& inSection, int& outValue, int inDefault )
{
	QString		tmp;
	
	if ( this->getPrefValue( inName, inSection, tmp ) )
	{
//		printf( "found %s: %s\n", (const char*)inName, (const char*)tmp );
		outValue = tmp.toInt();
		return true;
	}
	else
		outValue = inDefault;
	return false;
}

bool PreferenceFile::getPrefBool( const QString& inName, const QString& inSection, bool inDefault )
{
	int		nValue;

	if ( this->getPrefValue( inName, inSection, nValue, inDefault ) )
	{
		if ( nValue )
			return true;
		return false;
	}
	return inDefault;
}

int PreferenceFile::getPrefInt( const QString& inName, const QString& inSection, int inDefault )
{
//	printf( "getPrefInt - %s  %s\n", (const char*)inName, (const char*)inSection );
	
	int		nValue;

	if ( this->getPrefValue( inName, inSection, nValue, inDefault ) )
		return nValue;
	return inDefault;
}

QString PreferenceFile::getPrefString( const QString& inName, const QString& inSection, const QString& inDefault )
{
	QString		tmp;
	if ( this->getPrefValue( inName, inSection, tmp, inDefault ) )
		return tmp;
	return inDefault;
}

void PreferenceFile::setPrefValue( const QString& inName, const QString& inSection, const QString& inValue, bool persistent )
{
	PreferenceSection*	section = m_sections.find( inSection );
	
	if ( !section )
	{
		section = new PreferenceSection( inSection );
		m_sections.insert( inSection, section );
	}
	section->setPrefValue( inName, inValue, persistent );
	m_modified = true;
}

void PreferenceFile::setPrefBool( const QString& inName, const QString& inSection, bool inValue, bool persistent )
{
	QString		val = QString::number( inValue );
	
	this->setPrefValue( inName, inSection, val, persistent );
}

void PreferenceFile::setPrefInt( const QString& inName, const QString& inSection, int inValue, bool persistent )
{
	QString		val = QString::number( inValue );
	
	this->setPrefValue( inName, inSection, val, persistent );
}

void PreferenceFile::setPrefString( const QString& inName, const QString& inSection, const QString& inValue, bool persistent )
{
	this->setPrefValue( inName, inSection, inValue, persistent );
}



// revert()
// reload cache from file
void PreferenceFile::revert()
{
	m_sections.clear();
	this->load();
} 

// load()
// load file into cache
void PreferenceFile::load()
{
#ifdef DEBUG
//	printf( "PreferenceFile::Load()\n" );
#endif

	QFile			fp( m_filename );
	
	if ( !fp.open( IO_ReadOnly ) )
	{
		fprintf( stderr,\
"\n\
ERROR: Cannot open config file '%s'.\n\
Check that you manually installed the config file to the correct location,\n\
(usually " LOGDIR "/showeq.conf)\n\
as the installer does not copy it.\n\
\n\
It must be changed before it will work.  Refer to the comments in the\n\
conf/showeq.conf.dist file.\n",\
		(const char*)m_filename );
		exit( 1 );
	}

	QTextStream		input( &fp );
	
	QString			line;
	QString			sectionName;
	
	while ( !input.atEnd() )
	{
		line = input.readLine();

#ifdef DEBUG
//		printf( "PreferenceFile::Load(): '%s' - \n", (const char*)line );
#endif

		// treat lines beginning with # or ; as comments
		if ( ( line[ 0 ] == '#' ) || ( line[ 0 ] == ';' ) )
		{
#ifdef DEBUG
//			printf( "skipping comment\n" );
#endif
			continue;
		}

		// if section name
		if ( line[ 0 ] == '[' )
		{
			int len = -1;
			int p = line.find( ']' );
			if ( p )
				len = p - 1;
			sectionName = line.mid( 1, len );
#ifdef DEBUG
//			printf( " - grabbed section '%s'\n", (const char*)sectionName );
#endif
			continue;
		}

		// strip leading whitespace from line
		line = line.stripWhiteSpace();

		// parse the line and add it to the list
		if ( line.length() )
		{
			QString		name;
			QString		value;
			
			this->loadLine( line, name, value );
			
			// qualify data
			if ( !name.length() || !value.length() )
				continue;

			// if we were passed a section name, format the string to 'sectionname_name'
			this->setPrefValue( name, sectionName, value, true );
		}
		// otherwise blank line, end of section
		else
		{
#ifdef DEBUG
//			printf( " skipping\n" );
#endif
		}
	}

	fp.close();
	
	//fprintf(stdout, "Preferences loaded from '%s'\n", (const char*)m_filename);

	m_modified = false;

//	printf( "Loaded Preferences:\n" );
//	this->printContents();
}

void PreferenceFile::printContents()
{
	PreferenceSection*	pref;
	
	QAsciiDictIterator<PreferenceSection>	it( m_sections );

	while ( ( pref = it.current() ) )
	{
		::printContents( pref->m_sectionName, pref->m_list );
		++it;
	}
}


// save()
// save values from cache to file
void PreferenceFile::save()
{
	if ( !m_modified )
		return;

#ifdef DEBUG
//	printf( "PreferenceFile::Save() %s\n", m_modified ? "Modified" : "Not Modified" );
//	this->printContents();
#endif

	// open source and destination files
	QString			outFilename = m_filename + ".new";
	QString			inFilename = m_filename;
	
	QFile			fileIn( inFilename );
	QFile			fileOut( outFilename );

	if ( !fileIn.open( IO_ReadOnly ) )
		fprintf( stderr, "Couldn't open current config file: '%s'\n", (const char*)inFilename );
	
	if ( !fileOut.open( IO_WriteOnly ) )
	{
		fprintf( stderr, "Couldn't open new config file: %s\n", (const char*)outFilename );
		return;
	}

	QTextStream			inStream( &fileIn );
	QTextStream			outStream( &fileOut );
	
	QString						line;
	QString						tmp;
	QString						sectionName;
	int							blankLines = 0;
	QAsciiDict<NameValuePair>	sectionPrefs;
	QAsciiDict<PreferenceSection>		sections = m_sections;
		
	while ( !inStream.atEnd() )
	{
		line = inStream.readLine();

#ifdef DEBUG
//		printf( "input: '%s'\n", (const char*)line );
#endif
		tmp = line.stripWhiteSpace();
		if ( !tmp.length() )
		{
			blankLines++;
			continue;
		}	
		
		// if section name
		if ( line[ 0 ] == '[' )
		{
			if ( sectionName.length() )
			{
				if ( sectionPrefs.count() )
				{
					// dump anything still in "section"
//					printf( "writing remaining %d items\n", sectionPrefs.count() );
					this->writeToStream( sectionPrefs, outStream );
					sectionPrefs.clear();
				}
				sections.remove( sectionName );
				sectionName = "";
			}
			while ( blankLines > 0 )
			{
				outStream << '\n';
				blankLines--;
			}
			
			int len = -1;
			int p = line.find( ']' );
			if ( p )
				len = p - 1;
			sectionName = line.mid( 1, len );
#ifdef DEBUG
//			printf( " - grabbed section '%s'\n", (const char*)sectionName );
#endif
			PreferenceSection*		section = sections.find( sectionName );
			if ( section )
			{
//				printf( "sectionPrefs old: %d\n", sectionPrefs.count() );
				sectionPrefs = section->getPrefs();
				this->removeNonPersistentPrefs( sectionPrefs );
//				printf( "sectionPrefs new: %d\n", sectionPrefs.count() );
			}
			else
				sectionPrefs.clear();

			outStream << line << '\n';
			continue;
		}

		while ( blankLines > 0 )
		{
			outStream << '\n';
			blankLines--;
		}
					
		// treat lines beginning with # or ; as comments
		if ( ( line[ 0 ] == '#' ) || ( line[ 0 ] == ';' ) )
		{
#ifdef DEBUG
//			printf( "skipping comment\n" );
#endif
			outStream << line << '\n';
			continue;
		}

		// strip leading whitespace from line
		QString		name;
		QString		value;
			
		this->loadLine( tmp, name, value );
		
		// qualify data
		if ( !name.length() )
		{
			outStream << line << '\n';
			continue;
		}
		
		QString		myValue;
		myValue = this->getPrefString( name, sectionName );
		if ( myValue == value )
		{
//			printf( "skipped saving, removed from sectionPrefs\n" );
			outStream << line << '\n';
			sectionPrefs.remove( name );
			continue;
		}

		unsigned long i = 0;
		// write any preceeding white-space back into the output file
		while ( line[ i ].isSpace() && i < line.length() )
			outStream << (char)((QChar)line[ i++ ]);
		// write the name back into the output file
		while ( !line[ i ].isSpace() && i < line.length() )
			outStream << (char)((QChar)line[ i++ ]);
		// write any whitespace in between name and value into output file
		while ( line[ i ].isSpace() && i < line.length() )
			outStream << (char)((QChar)line[ i++ ]);
		// write output value
		outStream << myValue;
		// write the rest of the line back out
		while ( !line[ i ].isSpace() && i < line.length() )
			i++;
		while ( i < line.length() )
			outStream << (char)((QChar)line[ i++ ]);
		outStream << '\n';
		
//		printf( "new value, wrote and removed\n" );
		sectionPrefs.remove( name );
	}
			
	fileIn.close();
	
	if ( sectionPrefs.count() )
	{
		// dump anything still in "section"
//		printf( "writing last section items: %d\n", sectionPrefs.count() );
		this->writeToStream( sectionPrefs, outStream );
		sectionPrefs.clear();
	}
	sections.remove( sectionName );
	
	if ( !sections.isEmpty() )
	{
		QAsciiDictIterator<PreferenceSection>		iter( sections );
		
		PreferenceSection*		section;
		while ( ( section = iter.current() ) )
		{
//			printf( "writing new section: %s\n", (const char*)section->sectionName() );
			++iter;
			outStream << '\n';
			outStream << "[" << section->sectionName() << "]" << '\n';
			sectionPrefs = section->getPrefs();
			this->writeToStream( sectionPrefs, outStream );
		}
	}
	fileOut.close();
	
	QString		command;
	command = "cp " + inFilename + " " + inFilename + ".bak";
	if ( -1 == system( command ) )
		fprintf( stderr, "'%s' - failed\n", (const char*)command );
	command = "mv -f " + outFilename + " " + inFilename;
	if ( -1 == system( command ) )
		fprintf( stderr, "'%s' - failed\n", (const char*)command );

	m_modified = false;

//	printf( "Saved preferences\n" );
}

void PreferenceFile::removeNonPersistentPrefs( QAsciiDict<NameValuePair>& list )
{
	QAsciiDictIterator<NameValuePair>		iter( list );
					
	NameValuePair*		nvPair;
					
	while ( ( nvPair = iter.current() ) )
	{
		++iter;
		
		if ( !nvPair->persistent() )
			list.remove( nvPair->name() );
	}
}

// loadLine
// Process a line into a Name/Value pair
void PreferenceFile::loadLine( const QString& line, QString& name, QString& value )
{
	QString					tmp;
	
	// strip leading whitespace from line
	tmp = line.simplifyWhiteSpace();
	
	int separatorIndex = tmp.find( ' ' );
	
	name = tmp.left( separatorIndex );
	value = tmp.mid( separatorIndex + 1 );
	
	int quoteIndex = value.find( '\"' );
	// if value is quoted, parse to end of it
	if ( quoteIndex == 0 )
	{
		value = value.mid( 1 );
		quoteIndex = value.find( '\"' );
		if ( quoteIndex != -1 )
			value = value.left( quoteIndex );
	}
	else
	{
		separatorIndex = value.find( '#');
		if ( separatorIndex != -1 )
			value = value.mid( 0, separatorIndex );
		else
		{
			separatorIndex = value.find( ';' );
			if ( separatorIndex != -1 )
				value = value.mid( 0, separatorIndex );
		}
	}
		
	name = name.stripWhiteSpace();
	value = value.stripWhiteSpace();
	
#ifdef DEBUG
//	printf( "Name:'%s' Value:'%s'\n", (const char*)name, (const char*)value  );
#endif
} 



