
/*
 * editor.h
 *
 * text file editor
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */
 
#ifndef EDITOR_H
#define EDITOR_H

#include <qmainwindow.h>

class QMultiLineEdit;
class QToolBar;
class QPopupMenu;

class EditorWindow: public QMainWindow
{
    Q_OBJECT
public:
    EditorWindow();
    EditorWindow( const char *fileName );
    ~EditorWindow();

protected:
    void closeEvent( QCloseEvent* );

private slots:
    void load();
    void load( const char *fileName );
    void save();
    void saveAs();

private:
    QMultiLineEdit *e;
    QToolBar *fileTools;
    QString filename;
};


#endif

