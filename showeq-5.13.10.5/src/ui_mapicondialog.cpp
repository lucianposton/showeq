/****************************************************************************
** Form implementation generated from reading ui file 'mapicondialog.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "ui_mapicondialog.h"

#include <qvariant.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "mapicondialog.ui.h"

/*
 *  Constructs a MapIconDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MapIconDialog::MapIconDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "MapIconDialog" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );

    TextLabel7 = new QLabel( this, "TextLabel7" );
    TextLabel7->setGeometry( QRect( 10, 10, 60, 20 ) );

    m_mapIconSettingsFrame = new QFrame( this, "m_mapIconSettingsFrame" );
    m_mapIconSettingsFrame->setGeometry( QRect( 10, 40, 540, 260 ) );
    m_mapIconSettingsFrame->setFrameShape( QFrame::StyledPanel );
    m_mapIconSettingsFrame->setFrameShadow( QFrame::Raised );

    m_revert = new QPushButton( m_mapIconSettingsFrame, "m_revert" );
    m_revert->setGeometry( QRect( 283, 231, 60, 22 ) );

    m_apply = new QPushButton( m_mapIconSettingsFrame, "m_apply" );
    m_apply->setGeometry( QRect( 171, 231, 60, 22 ) );

    m_tabWidget = new QTabWidget( m_mapIconSettingsFrame, "m_tabWidget" );
    m_tabWidget->setGeometry( QRect( 10, 10, 520, 210 ) );

    image = new QWidget( m_tabWidget, "image" );

    frame4 = new QFrame( image, "frame4" );
    frame4->setGeometry( QRect( 8, 24, 500, 40 ) );
    frame4->setFrameShape( QFrame::GroupBoxPanel );
    frame4->setFrameShadow( QFrame::Sunken );

    m_imageImageLabel = new QLabel( frame4, "m_imageImageLabel" );
    m_imageImageLabel->setGeometry( QRect( 10, 10, 40, 20 ) );

    TextLabel2_4 = new QLabel( frame4, "TextLabel2_4" );
    TextLabel2_4->setGeometry( QRect( 240, 10, 30, 20 ) );

    m_imageImage = new QComboBox( FALSE, frame4, "m_imageImage" );
    m_imageImage->setGeometry( QRect( 60, 10, 140, 22 ) );

    m_imageSize = new QComboBox( FALSE, frame4, "m_imageSize" );
    m_imageSize->setGeometry( QRect( 280, 10, 100, 22 ) );

    m_imageFlash = new QCheckBox( frame4, "m_imageFlash" );
    m_imageFlash->setGeometry( QRect( 410, 10, 80, 20 ) );

    imageOutlineGroupBox_3 = new QGroupBox( image, "imageOutlineGroupBox_3" );
    imageOutlineGroupBox_3->setGeometry( QRect( 10, 70, 500, 50 ) );

    TextLabel4_5_2 = new QLabel( imageOutlineGroupBox_3, "TextLabel4_5_2" );
    TextLabel4_5_2->setGeometry( QRect( 410, 20, 40, 20 ) );

    TextLabel5_5_2 = new QLabel( imageOutlineGroupBox_3, "TextLabel5_5_2" );
    TextLabel5_5_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_imagePenColorSample = new QFrame( imageOutlineGroupBox_3, "m_imagePenColorSample" );
    m_imagePenColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_imagePenColorSample->setFrameShape( QFrame::LineEditPanel );
    m_imagePenColorSample->setFrameShadow( QFrame::Raised );

    m_imageUseSpawnColorPen = new QCheckBox( imageOutlineGroupBox_3, "m_imageUseSpawnColorPen" );
    m_imageUseSpawnColorPen->setGeometry( QRect( 10, 20, 130, 20 ) );

    m_imagePenStyle = new QComboBox( FALSE, imageOutlineGroupBox_3, "m_imagePenStyle" );
    m_imagePenStyle->setGeometry( QRect( 280, 20, 120, 22 ) );

    m_imagePenWidth = new QSpinBox( imageOutlineGroupBox_3, "m_imagePenWidth" );
    m_imagePenWidth->setGeometry( QRect( 456, 20, 30, 23 ) );
    m_imagePenWidth->setMaxValue( 5 );

    m_imagePenColor = new QPushButton( imageOutlineGroupBox_3, "m_imagePenColor" );
    m_imagePenColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    imageFillGroupBox_3 = new QGroupBox( image, "imageFillGroupBox_3" );
    imageFillGroupBox_3->setGeometry( QRect( 10, 120, 500, 50 ) );

    TextLabel3_4_2 = new QLabel( imageFillGroupBox_3, "TextLabel3_4_2" );
    TextLabel3_4_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_imageBrushColor = new QPushButton( imageFillGroupBox_3, "m_imageBrushColor" );
    m_imageBrushColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    m_imageBrushColorSample = new QFrame( imageFillGroupBox_3, "m_imageBrushColorSample" );
    m_imageBrushColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_imageBrushColorSample->setFrameShape( QFrame::LineEditPanel );
    m_imageBrushColorSample->setFrameShadow( QFrame::Raised );

    m_imageUseSpawnColorBrush = new QCheckBox( imageFillGroupBox_3, "m_imageUseSpawnColorBrush" );
    m_imageUseSpawnColorBrush->setGeometry( QRect( 10, 20, 130, 20 ) );

    m_imageBrushStyle = new QComboBox( FALSE, imageFillGroupBox_3, "m_imageBrushStyle" );
    m_imageBrushStyle->setGeometry( QRect( 280, 20, 170, 22 ) );

    m_useImage = new QCheckBox( image, "m_useImage" );
    m_useImage->setGeometry( QRect( 10, 0, 100, 20 ) );
    m_tabWidget->insertTab( image, QString::fromLatin1("") );

    highlight = new QWidget( m_tabWidget, "highlight" );

    m_useHighlight = new QCheckBox( highlight, "m_useHighlight" );
    m_useHighlight->setGeometry( QRect( 10, 0, 110, 20 ) );

    frame6 = new QFrame( highlight, "frame6" );
    frame6->setGeometry( QRect( 8, 24, 500, 40 ) );
    frame6->setFrameShape( QFrame::GroupBoxPanel );
    frame6->setFrameShadow( QFrame::Sunken );

    TextLabel1_2_2 = new QLabel( frame6, "TextLabel1_2_2" );
    TextLabel1_2_2->setGeometry( QRect( 10, 10, 40, 20 ) );

    m_highlightFlash = new QCheckBox( frame6, "m_highlightFlash" );
    m_highlightFlash->setGeometry( QRect( 410, 10, 80, 20 ) );

    m_highlightSize = new QComboBox( FALSE, frame6, "m_highlightSize" );
    m_highlightSize->setGeometry( QRect( 280, 10, 90, 22 ) );

    TextLabel2_2_2 = new QLabel( frame6, "TextLabel2_2_2" );
    TextLabel2_2_2->setGeometry( QRect( 240, 10, 30, 20 ) );

    m_highlightImage = new QComboBox( FALSE, frame6, "m_highlightImage" );
    m_highlightImage->setGeometry( QRect( 60, 10, 140, 22 ) );

    highlightFillGroupBox_2 = new QGroupBox( highlight, "highlightFillGroupBox_2" );
    highlightFillGroupBox_2->setGeometry( QRect( 10, 120, 500, 50 ) );

    m_highlightUseSpawnColorBrush = new QCheckBox( highlightFillGroupBox_2, "m_highlightUseSpawnColorBrush" );
    m_highlightUseSpawnColorBrush->setGeometry( QRect( 10, 20, 130, 20 ) );

    TextLabel3_2_2_2 = new QLabel( highlightFillGroupBox_2, "TextLabel3_2_2_2" );
    TextLabel3_2_2_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_highlightBrushColor = new QPushButton( highlightFillGroupBox_2, "m_highlightBrushColor" );
    m_highlightBrushColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    m_highlightBrushColorSample = new QFrame( highlightFillGroupBox_2, "m_highlightBrushColorSample" );
    m_highlightBrushColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_highlightBrushColorSample->setFrameShape( QFrame::LineEditPanel );
    m_highlightBrushColorSample->setFrameShadow( QFrame::Raised );

    m_highlightBrushStyle = new QComboBox( FALSE, highlightFillGroupBox_2, "m_highlightBrushStyle" );
    m_highlightBrushStyle->setGeometry( QRect( 280, 20, 170, 22 ) );

    highlightOutlineGroupBox_2 = new QGroupBox( highlight, "highlightOutlineGroupBox_2" );
    highlightOutlineGroupBox_2->setGeometry( QRect( 10, 70, 500, 50 ) );

    m_highlightUseSpawnColorPen = new QCheckBox( highlightOutlineGroupBox_2, "m_highlightUseSpawnColorPen" );
    m_highlightUseSpawnColorPen->setGeometry( QRect( 10, 20, 130, 20 ) );

    m_highlightPenStyle = new QComboBox( FALSE, highlightOutlineGroupBox_2, "m_highlightPenStyle" );
    m_highlightPenStyle->setGeometry( QRect( 280, 20, 120, 22 ) );

    TextLabel5_2_2_2 = new QLabel( highlightOutlineGroupBox_2, "TextLabel5_2_2_2" );
    TextLabel5_2_2_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_highlightPenColor = new QPushButton( highlightOutlineGroupBox_2, "m_highlightPenColor" );
    m_highlightPenColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    TextLabel4_2_2_2 = new QLabel( highlightOutlineGroupBox_2, "TextLabel4_2_2_2" );
    TextLabel4_2_2_2->setGeometry( QRect( 410, 20, 40, 20 ) );

    m_highlightPenColorSample = new QFrame( highlightOutlineGroupBox_2, "m_highlightPenColorSample" );
    m_highlightPenColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_highlightPenColorSample->setFrameShape( QFrame::LineEditPanel );
    m_highlightPenColorSample->setFrameShadow( QFrame::Raised );

    m_highlightPenWidth = new QSpinBox( highlightOutlineGroupBox_2, "m_highlightPenWidth" );
    m_highlightPenWidth->setGeometry( QRect( 456, 20, 30, 23 ) );
    m_highlightPenWidth->setMaxValue( 5 );
    m_tabWidget->insertTab( highlight, QString::fromLatin1("") );

    lines = new QWidget( m_tabWidget, "lines" );

    line2GroupBox_2 = new QGroupBox( lines, "line2GroupBox_2" );
    line2GroupBox_2->setGeometry( QRect( 10, 120, 500, 50 ) );

    TextLabel6_2_3_2 = new QLabel( line2GroupBox_2, "TextLabel6_2_3_2" );
    TextLabel6_2_3_2->setGeometry( QRect( 10, 20, 60, 20 ) );

    m_line2Distance = new QSpinBox( line2GroupBox_2, "m_line2Distance" );
    m_line2Distance->setGeometry( QRect( 70, 20, 60, 23 ) );
    m_line2Distance->setMaxValue( 2000 );
    m_line2Distance->setLineStep( 5 );

    m_line2PenColor = new QPushButton( line2GroupBox_2, "m_line2PenColor" );
    m_line2PenColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    TextLabel4_3_2_2_4_2 = new QLabel( line2GroupBox_2, "TextLabel4_3_2_2_4_2" );
    TextLabel4_3_2_2_4_2->setGeometry( QRect( 410, 20, 40, 20 ) );

    m_line2PenStyle = new QComboBox( FALSE, line2GroupBox_2, "m_line2PenStyle" );
    m_line2PenStyle->setGeometry( QRect( 280, 20, 120, 22 ) );

    TextLabel5_3_2_2_4_2 = new QLabel( line2GroupBox_2, "TextLabel5_3_2_2_4_2" );
    TextLabel5_3_2_2_4_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_line2PenColorSample = new QFrame( line2GroupBox_2, "m_line2PenColorSample" );
    m_line2PenColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_line2PenColorSample->setFrameShape( QFrame::LineEditPanel );
    m_line2PenColorSample->setFrameShadow( QFrame::Raised );

    m_line2PenWidth = new QSpinBox( line2GroupBox_2, "m_line2PenWidth" );
    m_line2PenWidth->setGeometry( QRect( 456, 20, 30, 23 ) );
    m_line2PenWidth->setMaxValue( 5 );

    line1GroupBox_2 = new QGroupBox( lines, "line1GroupBox_2" );
    line1GroupBox_2->setGeometry( QRect( 10, 70, 500, 50 ) );

    TextLabel6_4_2 = new QLabel( line1GroupBox_2, "TextLabel6_4_2" );
    TextLabel6_4_2->setGeometry( QRect( 10, 20, 60, 20 ) );

    TextLabel4_3_2_4_2 = new QLabel( line1GroupBox_2, "TextLabel4_3_2_4_2" );
    TextLabel4_3_2_4_2->setGeometry( QRect( 410, 20, 40, 20 ) );

    m_line1PenStyle = new QComboBox( FALSE, line1GroupBox_2, "m_line1PenStyle" );
    m_line1PenStyle->setGeometry( QRect( 280, 20, 120, 22 ) );

    m_line1PenColor = new QPushButton( line1GroupBox_2, "m_line1PenColor" );
    m_line1PenColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    TextLabel5_3_2_4_2 = new QLabel( line1GroupBox_2, "TextLabel5_3_2_4_2" );
    TextLabel5_3_2_4_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_line1PenColorSample = new QFrame( line1GroupBox_2, "m_line1PenColorSample" );
    m_line1PenColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_line1PenColorSample->setFrameShape( QFrame::LineEditPanel );
    m_line1PenColorSample->setFrameShadow( QFrame::Raised );

    m_line1Distance = new QSpinBox( line1GroupBox_2, "m_line1Distance" );
    m_line1Distance->setGeometry( QRect( 70, 20, 60, 23 ) );
    m_line1Distance->setMaxValue( 2000 );
    m_line1Distance->setLineStep( 5 );

    m_line1PenWidth = new QSpinBox( line1GroupBox_2, "m_line1PenWidth" );
    m_line1PenWidth->setGeometry( QRect( 456, 20, 30, 23 ) );
    m_line1PenWidth->setMaxValue( 5 );

    line0GroupBox_2 = new QGroupBox( lines, "line0GroupBox_2" );
    line0GroupBox_2->setGeometry( QRect( 10, 20, 500, 50 ) );

    TextLabel4_3_4_2 = new QLabel( line0GroupBox_2, "TextLabel4_3_4_2" );
    TextLabel4_3_4_2->setGeometry( QRect( 410, 20, 40, 20 ) );

    TextLabel5_3_4_2 = new QLabel( line0GroupBox_2, "TextLabel5_3_4_2" );
    TextLabel5_3_4_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_line0PenColor = new QPushButton( line0GroupBox_2, "m_line0PenColor" );
    m_line0PenColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    m_line0PenColorSample = new QFrame( line0GroupBox_2, "m_line0PenColorSample" );
    m_line0PenColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_line0PenColorSample->setFrameShape( QFrame::LineEditPanel );
    m_line0PenColorSample->setFrameShadow( QFrame::Raised );

    m_showLine0 = new QCheckBox( line0GroupBox_2, "m_showLine0" );
    m_showLine0->setGeometry( QRect( 10, 20, 120, 20 ) );

    m_line0PenStyle = new QComboBox( FALSE, line0GroupBox_2, "m_line0PenStyle" );
    m_line0PenStyle->setGeometry( QRect( 280, 20, 120, 22 ) );

    m_line0PenWidth = new QSpinBox( line0GroupBox_2, "m_line0PenWidth" );
    m_line0PenWidth->setGeometry( QRect( 456, 20, 30, 23 ) );
    m_line0PenWidth->setMaxValue( 5 );
    m_tabWidget->insertTab( lines, QString::fromLatin1("") );

    other = new QWidget( m_tabWidget, "other" );

    walkPathLineGroupBox_2 = new QGroupBox( other, "walkPathLineGroupBox_2" );
    walkPathLineGroupBox_2->setGeometry( QRect( 10, 70, 500, 50 ) );

    TextLabel4_3_2_2_2_2_2 = new QLabel( walkPathLineGroupBox_2, "TextLabel4_3_2_2_2_2_2" );
    TextLabel4_3_2_2_2_2_2->setGeometry( QRect( 410, 20, 40, 20 ) );

    m_walkPathPenStyle = new QComboBox( FALSE, walkPathLineGroupBox_2, "m_walkPathPenStyle" );
    m_walkPathPenStyle->setGeometry( QRect( 280, 20, 120, 22 ) );

    TextLabel5_3_2_2_2_2_2 = new QLabel( walkPathLineGroupBox_2, "TextLabel5_3_2_2_2_2_2" );
    TextLabel5_3_2_2_2_2_2->setGeometry( QRect( 240, 20, 38, 20 ) );

    m_walkPathPenColor = new QPushButton( walkPathLineGroupBox_2, "m_walkPathPenColor" );
    m_walkPathPenColor->setGeometry( QRect( 140, 20, 60, 22 ) );

    m_walkPathPenColorSample = new QFrame( walkPathLineGroupBox_2, "m_walkPathPenColorSample" );
    m_walkPathPenColorSample->setGeometry( QRect( 210, 20, 20, 22 ) );
    m_walkPathPenColorSample->setFrameShape( QFrame::LineEditPanel );
    m_walkPathPenColorSample->setFrameShadow( QFrame::Raised );

    m_useWalkPathPen = new QCheckBox( walkPathLineGroupBox_2, "m_useWalkPathPen" );
    m_useWalkPathPen->setGeometry( QRect( 10, 20, 100, 20 ) );

    m_walkPathPenWidth = new QSpinBox( walkPathLineGroupBox_2, "m_walkPathPenWidth" );
    m_walkPathPenWidth->setGeometry( QRect( 456, 20, 30, 23 ) );
    m_walkPathPenWidth->setMaxValue( 5 );

    m_showName = new QCheckBox( other, "m_showName" );
    m_showName->setGeometry( QRect( 10, 10, 90, 20 ) );

    m_showWalkPath = new QCheckBox( other, "m_showWalkPath" );
    m_showWalkPath->setGeometry( QRect( 10, 40, 130, 20 ) );
    m_tabWidget->insertTab( other, QString::fromLatin1("") );

    m_mapIconCombo = new QComboBox( FALSE, this, "m_mapIconCombo" );
    m_mapIconCombo->setGeometry( QRect( 80, 10, 240, 22 ) );

    m_close = new QPushButton( this, "m_close" );
    m_close->setGeometry( QRect( 490, 10, 60, 22 ) );
    m_close->setDefault( TRUE );
    languageChange();
    resize( QSize(557, 306).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_apply, SIGNAL( clicked() ), this, SLOT( apply() ) );
    connect( m_revert, SIGNAL( clicked() ), this, SLOT( revert() ) );
    connect( m_close, SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( m_mapIconCombo, SIGNAL( activated(int) ), this, SLOT( mapIconCombo_activated(int) ) );
    connect( m_imagePenColor, SIGNAL( clicked() ), this, SLOT( imagePenColor_clicked() ) );
    connect( m_imageBrushColor, SIGNAL( clicked() ), this, SLOT( imageBrushColor_clicked() ) );
    connect( m_highlightPenColor, SIGNAL( clicked() ), this, SLOT( highlightPenColor_clicked() ) );
    connect( m_highlightBrushColor, SIGNAL( clicked() ), this, SLOT( highlightBrushColor_clicked() ) );
    connect( m_line0PenColor, SIGNAL( clicked() ), this, SLOT( line0PenColor_clicked() ) );
    connect( m_line1PenColor, SIGNAL( clicked() ), this, SLOT( line1PenColor_clicked() ) );
    connect( m_line2PenColor, SIGNAL( clicked() ), this, SLOT( line2PenColor_clicked() ) );
    connect( m_walkPathPenColor, SIGNAL( clicked() ), this, SLOT( walkPathPenColor_clicked() ) );

    // tab order
    setTabOrder( m_mapIconCombo, m_tabWidget );
    setTabOrder( m_tabWidget, m_useImage );
    setTabOrder( m_useImage, m_imageImage );
    setTabOrder( m_imageImage, m_imageSize );
    setTabOrder( m_imageSize, m_imageFlash );
    setTabOrder( m_imageFlash, m_imageUseSpawnColorPen );
    setTabOrder( m_imageUseSpawnColorPen, m_imagePenColor );
    setTabOrder( m_imagePenColor, m_imagePenStyle );
    setTabOrder( m_imagePenStyle, m_imagePenWidth );
    setTabOrder( m_imagePenWidth, m_imageUseSpawnColorBrush );
    setTabOrder( m_imageUseSpawnColorBrush, m_imageBrushColor );
    setTabOrder( m_imageBrushColor, m_imageBrushStyle );
    setTabOrder( m_imageBrushStyle, m_useHighlight );
    setTabOrder( m_useHighlight, m_highlightImage );
    setTabOrder( m_highlightImage, m_highlightSize );
    setTabOrder( m_highlightSize, m_highlightFlash );
    setTabOrder( m_highlightFlash, m_highlightUseSpawnColorPen );
    setTabOrder( m_highlightUseSpawnColorPen, m_highlightPenColor );
    setTabOrder( m_highlightPenColor, m_highlightPenStyle );
    setTabOrder( m_highlightPenStyle, m_highlightPenWidth );
    setTabOrder( m_highlightPenWidth, m_highlightUseSpawnColorBrush );
    setTabOrder( m_highlightUseSpawnColorBrush, m_highlightBrushColor );
    setTabOrder( m_highlightBrushColor, m_highlightBrushStyle );
    setTabOrder( m_highlightBrushStyle, m_showLine0 );
    setTabOrder( m_showLine0, m_line0PenColor );
    setTabOrder( m_line0PenColor, m_line0PenStyle );
    setTabOrder( m_line0PenStyle, m_line0PenWidth );
    setTabOrder( m_line0PenWidth, m_line1Distance );
    setTabOrder( m_line1Distance, m_line1PenColor );
    setTabOrder( m_line1PenColor, m_line1PenStyle );
    setTabOrder( m_line1PenStyle, m_line1PenWidth );
    setTabOrder( m_line1PenWidth, m_line2Distance );
    setTabOrder( m_line2Distance, m_line2PenColor );
    setTabOrder( m_line2PenColor, m_line2PenStyle );
    setTabOrder( m_line2PenStyle, m_line2PenWidth );
    setTabOrder( m_line2PenWidth, m_showName );
    setTabOrder( m_showName, m_showWalkPath );
    setTabOrder( m_showWalkPath, m_useWalkPathPen );
    setTabOrder( m_useWalkPathPen, m_walkPathPenColor );
    setTabOrder( m_walkPathPenColor, m_walkPathPenStyle );
    setTabOrder( m_walkPathPenStyle, m_walkPathPenWidth );
    setTabOrder( m_walkPathPenWidth, m_apply );
    setTabOrder( m_apply, m_revert );
    setTabOrder( m_revert, m_close );

    // buddies
    TextLabel7->setBuddy( m_mapIconCombo );
    m_imageImageLabel->setBuddy( m_imageImage );
    TextLabel2_4->setBuddy( m_imageSize );
    TextLabel4_5_2->setBuddy( m_imagePenWidth );
    TextLabel5_5_2->setBuddy( m_imagePenStyle );
    TextLabel3_4_2->setBuddy( m_imageBrushStyle );
    TextLabel1_2_2->setBuddy( m_highlightImage );
    TextLabel2_2_2->setBuddy( m_highlightSize );
    TextLabel3_2_2_2->setBuddy( m_highlightBrushStyle );
    TextLabel5_2_2_2->setBuddy( m_highlightPenStyle );
    TextLabel4_2_2_2->setBuddy( m_highlightPenWidth );
    TextLabel6_2_3_2->setBuddy( m_line2Distance );
    TextLabel4_3_2_2_4_2->setBuddy( m_line2PenWidth );
    TextLabel5_3_2_2_4_2->setBuddy( m_line2PenStyle );
    TextLabel6_4_2->setBuddy( m_line1Distance );
    TextLabel4_3_2_4_2->setBuddy( m_line1PenWidth );
    TextLabel5_3_2_4_2->setBuddy( m_line1PenStyle );
    TextLabel4_3_4_2->setBuddy( m_line0PenWidth );
    TextLabel5_3_4_2->setBuddy( m_line0PenStyle );
    TextLabel4_3_2_2_2_2_2->setBuddy( m_walkPathPenWidth );
    TextLabel5_3_2_2_2_2_2->setBuddy( m_walkPathPenStyle );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
MapIconDialog::~MapIconDialog()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MapIconDialog::languageChange()
{
    setCaption( tr( "Map Icon Config" ) );
    TextLabel7->setText( tr( "&Map Icon:" ) );
    m_revert->setText( tr( "Revert" ) );
    m_apply->setText( tr( "Apply" ) );
    m_imageImageLabel->setText( tr( "Image:" ) );
    TextLabel2_4->setText( tr( "Size:" ) );
    m_imageFlash->setText( tr( "Flash" ) );
    imageOutlineGroupBox_3->setTitle( tr( "&Outline" ) );
    TextLabel4_5_2->setText( tr( "Width:" ) );
    TextLabel5_5_2->setText( tr( "Style:" ) );
    m_imageUseSpawnColorPen->setText( tr( "Use Spawn Color" ) );
    m_imagePenColor->setText( tr( "Color..." ) );
    imageFillGroupBox_3->setTitle( tr( "&Fill" ) );
    TextLabel3_4_2->setText( tr( "Style:" ) );
    m_imageBrushColor->setText( tr( "Color..." ) );
    m_imageUseSpawnColorBrush->setText( tr( "Use Spawn Color" ) );
    m_useImage->setText( tr( "&Use Image" ) );
    m_tabWidget->changeTab( image, tr( "&Image" ) );
    m_useHighlight->setText( tr( "&Use Highlight" ) );
    TextLabel1_2_2->setText( tr( "Image:" ) );
    m_highlightFlash->setText( tr( "Flash" ) );
    TextLabel2_2_2->setText( tr( "Size:" ) );
    highlightFillGroupBox_2->setTitle( tr( "&Fill" ) );
    m_highlightUseSpawnColorBrush->setText( tr( "Use Spawn Color" ) );
    TextLabel3_2_2_2->setText( tr( "Style:" ) );
    m_highlightBrushColor->setText( tr( "Color..." ) );
    highlightOutlineGroupBox_2->setTitle( tr( "&Outline" ) );
    m_highlightUseSpawnColorPen->setText( tr( "Use Spawn Color" ) );
    TextLabel5_2_2_2->setText( tr( "Style:" ) );
    m_highlightPenColor->setText( tr( "Color..." ) );
    TextLabel4_2_2_2->setText( tr( "Width:" ) );
    m_tabWidget->changeTab( highlight, tr( "&Highlight" ) );
    line2GroupBox_2->setTitle( tr( "Line &2" ) );
    TextLabel6_2_3_2->setText( tr( "Distance:" ) );
    m_line2Distance->setSpecialValueText( tr( "Off" ) );
    m_line2PenColor->setText( tr( "Color..." ) );
    TextLabel4_3_2_2_4_2->setText( tr( "Width:" ) );
    TextLabel5_3_2_2_4_2->setText( tr( "Style:" ) );
    line1GroupBox_2->setTitle( tr( "Line &1" ) );
    TextLabel6_4_2->setText( tr( "Distance:" ) );
    TextLabel4_3_2_4_2->setText( tr( "Width:" ) );
    m_line1PenColor->setText( tr( "Color..." ) );
    TextLabel5_3_2_4_2->setText( tr( "Style:" ) );
    m_line1Distance->setSpecialValueText( tr( "Off" ) );
    line0GroupBox_2->setTitle( tr( "Line &0" ) );
    TextLabel4_3_4_2->setText( tr( "Width:" ) );
    TextLabel5_3_4_2->setText( tr( "Style:" ) );
    m_line0PenColor->setText( tr( "Color..." ) );
    m_showLine0->setText( tr( "Show" ) );
    m_tabWidget->changeTab( lines, tr( "&Lines" ) );
    walkPathLineGroupBox_2->setTitle( tr( "Walk Path Line &Override" ) );
    TextLabel4_3_2_2_2_2_2->setText( tr( "Width:" ) );
    TextLabel5_3_2_2_2_2_2->setText( tr( "Style:" ) );
    m_walkPathPenColor->setText( tr( "Color..." ) );
    m_useWalkPathPen->setText( tr( "Use Override" ) );
    m_showName->setText( tr( "Show &Name" ) );
    m_showWalkPath->setText( tr( "Show &Walk Path" ) );
    m_tabWidget->changeTab( other, tr( "&General" ) );
    m_close->setText( tr( "Close" ) );
}

