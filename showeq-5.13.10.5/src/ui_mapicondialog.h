/****************************************************************************
** Form interface generated from reading ui file 'mapicondialog.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MAPICONDIALOG_H
#define MAPICONDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include "mapicon.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QFrame;
class QPushButton;
class QTabWidget;
class QWidget;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QSpinBox;

class MapIconDialog : public QDialog
{
    Q_OBJECT

public:
    MapIconDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MapIconDialog();

    QLabel* TextLabel7;
    QFrame* m_mapIconSettingsFrame;
    QPushButton* m_revert;
    QPushButton* m_apply;
    QTabWidget* m_tabWidget;
    QWidget* image;
    QFrame* frame4;
    QLabel* m_imageImageLabel;
    QLabel* TextLabel2_4;
    QComboBox* m_imageImage;
    QComboBox* m_imageSize;
    QCheckBox* m_imageFlash;
    QGroupBox* imageOutlineGroupBox_3;
    QLabel* TextLabel4_5_2;
    QLabel* TextLabel5_5_2;
    QFrame* m_imagePenColorSample;
    QCheckBox* m_imageUseSpawnColorPen;
    QComboBox* m_imagePenStyle;
    QSpinBox* m_imagePenWidth;
    QPushButton* m_imagePenColor;
    QGroupBox* imageFillGroupBox_3;
    QLabel* TextLabel3_4_2;
    QPushButton* m_imageBrushColor;
    QFrame* m_imageBrushColorSample;
    QCheckBox* m_imageUseSpawnColorBrush;
    QComboBox* m_imageBrushStyle;
    QCheckBox* m_useImage;
    QWidget* highlight;
    QCheckBox* m_useHighlight;
    QFrame* frame6;
    QLabel* TextLabel1_2_2;
    QCheckBox* m_highlightFlash;
    QComboBox* m_highlightSize;
    QLabel* TextLabel2_2_2;
    QComboBox* m_highlightImage;
    QGroupBox* highlightFillGroupBox_2;
    QCheckBox* m_highlightUseSpawnColorBrush;
    QLabel* TextLabel3_2_2_2;
    QPushButton* m_highlightBrushColor;
    QFrame* m_highlightBrushColorSample;
    QComboBox* m_highlightBrushStyle;
    QGroupBox* highlightOutlineGroupBox_2;
    QCheckBox* m_highlightUseSpawnColorPen;
    QComboBox* m_highlightPenStyle;
    QLabel* TextLabel5_2_2_2;
    QPushButton* m_highlightPenColor;
    QLabel* TextLabel4_2_2_2;
    QFrame* m_highlightPenColorSample;
    QSpinBox* m_highlightPenWidth;
    QWidget* lines;
    QGroupBox* line2GroupBox_2;
    QLabel* TextLabel6_2_3_2;
    QSpinBox* m_line2Distance;
    QPushButton* m_line2PenColor;
    QLabel* TextLabel4_3_2_2_4_2;
    QComboBox* m_line2PenStyle;
    QLabel* TextLabel5_3_2_2_4_2;
    QFrame* m_line2PenColorSample;
    QSpinBox* m_line2PenWidth;
    QGroupBox* line1GroupBox_2;
    QLabel* TextLabel6_4_2;
    QLabel* TextLabel4_3_2_4_2;
    QComboBox* m_line1PenStyle;
    QPushButton* m_line1PenColor;
    QLabel* TextLabel5_3_2_4_2;
    QFrame* m_line1PenColorSample;
    QSpinBox* m_line1Distance;
    QSpinBox* m_line1PenWidth;
    QGroupBox* line0GroupBox_2;
    QLabel* TextLabel4_3_4_2;
    QLabel* TextLabel5_3_4_2;
    QPushButton* m_line0PenColor;
    QFrame* m_line0PenColorSample;
    QCheckBox* m_showLine0;
    QComboBox* m_line0PenStyle;
    QSpinBox* m_line0PenWidth;
    QWidget* other;
    QGroupBox* walkPathLineGroupBox_2;
    QLabel* TextLabel4_3_2_2_2_2_2;
    QComboBox* m_walkPathPenStyle;
    QLabel* TextLabel5_3_2_2_2_2_2;
    QPushButton* m_walkPathPenColor;
    QFrame* m_walkPathPenColorSample;
    QCheckBox* m_useWalkPathPen;
    QSpinBox* m_walkPathPenWidth;
    QCheckBox* m_showName;
    QCheckBox* m_showWalkPath;
    QComboBox* m_mapIconCombo;
    QPushButton* m_close;

public slots:
    virtual void apply();
    virtual void revert();
    virtual void init();
    virtual void destroy();
    virtual void setMapIcons( MapIcons * mapIcons );
    virtual void mapIconCombo_activated( int id );
    virtual void imagePenColor_clicked();
    virtual void imageBrushColor_clicked();
    virtual void highlightPenColor_clicked();
    virtual void highlightBrushColor_clicked();
    virtual void line0PenColor_clicked();
    virtual void line1PenColor_clicked();
    virtual void line2PenColor_clicked();
    virtual void walkPathPenColor_clicked();
    virtual void setupMapIconDisplay();

protected:
    MapIcons* m_mapIcons;
    MapIcon m_currentMapIcon;
    MapIcon m_currentMapIconBackup;
    MapIconType m_currentMapIconType;


protected slots:
    virtual void languageChange();

};

#endif // MAPICONDIALOG_H
