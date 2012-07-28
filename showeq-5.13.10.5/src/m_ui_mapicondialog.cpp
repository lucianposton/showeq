/****************************************************************************
** MapIconDialog meta object code from reading C++ file 'ui_mapicondialog.h'
**
** Created: Wed Jun 27 20:42:22 2012
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "ui_mapicondialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MapIconDialog::className() const
{
    return "MapIconDialog";
}

QMetaObject *MapIconDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MapIconDialog( "MapIconDialog", &MapIconDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MapIconDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MapIconDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MapIconDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MapIconDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MapIconDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"apply", 0, 0 };
    static const QUMethod slot_1 = {"revert", 0, 0 };
    static const QUMethod slot_2 = {"init", 0, 0 };
    static const QUMethod slot_3 = {"destroy", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ "mapIcons", &static_QUType_ptr, "MapIcons", QUParameter::In }
    };
    static const QUMethod slot_4 = {"setMapIcons", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"mapIconCombo_activated", 1, param_slot_5 };
    static const QUMethod slot_6 = {"imagePenColor_clicked", 0, 0 };
    static const QUMethod slot_7 = {"imageBrushColor_clicked", 0, 0 };
    static const QUMethod slot_8 = {"highlightPenColor_clicked", 0, 0 };
    static const QUMethod slot_9 = {"highlightBrushColor_clicked", 0, 0 };
    static const QUMethod slot_10 = {"line0PenColor_clicked", 0, 0 };
    static const QUMethod slot_11 = {"line1PenColor_clicked", 0, 0 };
    static const QUMethod slot_12 = {"line2PenColor_clicked", 0, 0 };
    static const QUMethod slot_13 = {"walkPathPenColor_clicked", 0, 0 };
    static const QUMethod slot_14 = {"setupMapIconDisplay", 0, 0 };
    static const QUMethod slot_15 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "apply()", &slot_0, QMetaData::Public },
	{ "revert()", &slot_1, QMetaData::Public },
	{ "init()", &slot_2, QMetaData::Public },
	{ "destroy()", &slot_3, QMetaData::Public },
	{ "setMapIcons(MapIcons*)", &slot_4, QMetaData::Public },
	{ "mapIconCombo_activated(int)", &slot_5, QMetaData::Public },
	{ "imagePenColor_clicked()", &slot_6, QMetaData::Public },
	{ "imageBrushColor_clicked()", &slot_7, QMetaData::Public },
	{ "highlightPenColor_clicked()", &slot_8, QMetaData::Public },
	{ "highlightBrushColor_clicked()", &slot_9, QMetaData::Public },
	{ "line0PenColor_clicked()", &slot_10, QMetaData::Public },
	{ "line1PenColor_clicked()", &slot_11, QMetaData::Public },
	{ "line2PenColor_clicked()", &slot_12, QMetaData::Public },
	{ "walkPathPenColor_clicked()", &slot_13, QMetaData::Public },
	{ "setupMapIconDisplay()", &slot_14, QMetaData::Public },
	{ "languageChange()", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"MapIconDialog", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MapIconDialog.setMetaObject( metaObj );
    return metaObj;
}

void* MapIconDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MapIconDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool MapIconDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: apply(); break;
    case 1: revert(); break;
    case 2: init(); break;
    case 3: destroy(); break;
    case 4: setMapIcons((MapIcons*)static_QUType_ptr.get(_o+1)); break;
    case 5: mapIconCombo_activated((int)static_QUType_int.get(_o+1)); break;
    case 6: imagePenColor_clicked(); break;
    case 7: imageBrushColor_clicked(); break;
    case 8: highlightPenColor_clicked(); break;
    case 9: highlightBrushColor_clicked(); break;
    case 10: line0PenColor_clicked(); break;
    case 11: line1PenColor_clicked(); break;
    case 12: line2PenColor_clicked(); break;
    case 13: walkPathPenColor_clicked(); break;
    case 14: setupMapIconDisplay(); break;
    case 15: languageChange(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MapIconDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MapIconDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool MapIconDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
