
/*
 * editor.cpp
 *
 * text file editor
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2002-2007 by the respective ShowEQ Developers
 */

/* Implementation of text editor class */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <qapplication.h>
#include <q3toolbar.h> 
#include <qstatusbar.h> 
#include <q3popupmenu.h> 
#include <qmenubar.h> 
#include <q3mainwindow.h> 
#include <q3filedialog.h> 
#include <qtoolbutton.h> 
#include <q3textstream.h> 
#include <qpaintdevice.h> 
#include <qobject.h> 
#include <q3multilineedit.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QPixmap>
#include <QCloseEvent>

#include "util.h"

#include "editor.h"

/* XPM */
static const char *filesave[] = {
"    14    14        4            1",
". c #040404",
"# c #808304",
"a c #bfc2bf",
"b c None",
"..............",
".#.aaaaaaaa.a.",
".#.aaaaaaaa...",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".##........##.",
".############.",
".##.........#.",
".##......aa.#.",
".##......aa.#.",
".##......aa.#.",
"b............."
};

/* XPM */
static const char *fileopen[] = {
"    16    13        5            1",
". c #040404",
"# c #808304",
"a c None",
"b c #f3f704",
"c c #f3f7f3",
"aaaaaaaaa...aaaa",
"aaaaaaaa.aaa.a.a",
"aaaaaaaaaaaaa..a",
"a...aaaaaaaa...a",
".bcb.......aaaaa",
".cbcbcbcbc.aaaaa",
".bcbcbcbcb.aaaaa",
".cbcb...........",
".bcb.#########.a",
".cb.#########.aa",
".b.#########.aaa",
"..#########.aaaa",
"...........aaaaa"
};

EditorWindow::EditorWindow(const char *fileName)
     : Q3MainWindow( 0, "ShowEQ - Editor", Qt::WDestructiveClose )
 {
     int id;
  
     QPixmap openIcon, saveIcon;
 
     fileTools = new Q3ToolBar( this, "file operations" );

     fileTools->setLabel( tr( "File Operations" ) );
 
        openIcon = QPixmap( fileopen );
	new QToolButton( openIcon, "Open File", QString::null,
                               this, SLOT(load()), fileTools, "open file" );
    
        saveIcon = QPixmap( filesave );
        new QToolButton( saveIcon, "Save File", QString::null,
                               this, SLOT(save()), fileTools, "save file" );
 
     Q3PopupMenu * file = new Q3PopupMenu( this );
     menuBar()->insertItem( "&File", file );
 
     id = file->insertItem( openIcon, "&Open",
                            this, SLOT(load()), CTRL+Key_O );
 
     id = file->insertItem( saveIcon, "&Save",
                            this, SLOT(save()), CTRL+Key_S );

     file->insertSeparator();
     file->insertItem( "&Close Editor", this, SLOT(close()), CTRL+Key_W );

     e = new Q3MultiLineEdit( this, "editor" );
     e->setFocus();
     setCentralWidget( e );

     statusBar()->message( "Ready", 2000 );

     resize( 600, 450 );
     filename = (QString)fileName;
     load(fileName);
 }

 EditorWindow::~EditorWindow()
 {
 }

 void EditorWindow::load()
 {
     QString fn = Q3FileDialog::getOpenFileName( QString::null, QString::null,
                                                this);
     if ( !fn.isEmpty() )
         load( fn );
     else
         statusBar()->message( "File Open Cancelled", 2000 );
 }

 void EditorWindow::load( const char *fileName )
 {
     QFile f( fileName );
     if ( !f.open( QIODevice::ReadOnly ) )
         return;

     e->setAutoUpdate( FALSE );
     e->clear();

     Q3TextStream t(&f);
     while ( !t.eof() ) {
         QString s = t.readLine();
         e->append( s );
     }
     f.close();

     e->setAutoUpdate( TRUE );
     e->repaint();
     e->setEdited( FALSE );
     setCaption( fileName );
     QString s;
     s.sprintf( "Opened %s", fileName );
     statusBar()->message( s, 2000 );
 }

 void EditorWindow::save()
 {
     if ( filename.isEmpty() ) {
         saveAs();
         return;
     }

     QString text = e->text();
     QFile f( filename );
     if ( !f.open( QIODevice::WriteOnly ) ) {
         statusBar()->message( QString("Could not write to %1").arg(filename),
                               2000 );
         return;
     }

     Q3TextStream t( &f );
     t << text;
     f.close();

     e->setEdited( FALSE );

     setCaption( filename );

     statusBar()->message( QString( "Saved %1" ).arg( filename ), 2000 );
 }

 void EditorWindow::saveAs()
 {
     QString fn = Q3FileDialog::getSaveFileName( QString::null, QString::null,
                                                this );
     if ( !fn.isEmpty() ) {
         filename = fn;
         save();
     } else {
         statusBar()->message( "Saving cancelled", 2000 );
     }
 }

 void EditorWindow::closeEvent( QCloseEvent* ce )
 {
     if ( !e->edited() ) {
         ce->accept();
         return;
     }

     switch( QMessageBox::information( this, "ShowEQ Editor",
                                       "The document has been changed since "
                                       "the last save.",
                                       "Save Now", "Cancel", "Lose Changes",
                                       0, 1 ) ) {
     case 0:
         save();
         ce->accept();
         break;
     case 1:
     default: // just for sanity
         ce->ignore();
         break;
     case 2:
         ce->accept();
         break;
     }
 }

#ifndef QMAKEBUILD
#include "editor.moc"
#endif 
