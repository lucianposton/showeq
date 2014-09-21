//Added by qt3to4:
#include <QPixmap>
/*
 * mapicondialog.ui.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 * 
 * Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 * 
 */

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void MapIconDialog::apply()
{
    QPen pen;
    QBrush brush;
    
    // get image settings
    m_currentMapIcon.setImage(m_useImage->isChecked());
    m_currentMapIcon.setImageStyle(MapIconStyle(m_imageImage->currentItem()));
    m_currentMapIcon.setImageSize(MapIconSize(m_imageSize->currentItem()));
    m_currentMapIcon.setImageFlash(m_imageFlash->isChecked());
    m_currentMapIcon.setImageUseSpawnColorPen(m_imageUseSpawnColorPen->isChecked());
    pen = m_currentMapIcon.imagePen();
    pen.setStyle(Qt::PenStyle(m_imagePenStyle->currentItem()));
    pen.setWidth(m_imagePenWidth->value());
    m_currentMapIcon.setImagePen(pen);
    m_currentMapIcon.setImageUseSpawnColorBrush(m_imageUseSpawnColorBrush->isChecked());
    brush = m_currentMapIcon.imageBrush();
    brush.setStyle(Qt::BrushStyle(m_imageBrushStyle->currentItem()));
    m_currentMapIcon.setImageBrush(brush);
    
    // get highlight settings
    m_currentMapIcon.setHighlight(m_useHighlight->isChecked());
    m_currentMapIcon.setHighlightStyle(MapIconStyle(m_highlightImage->currentItem()));
    m_currentMapIcon.setHighlightSize(MapIconSize(m_highlightSize->currentItem()));
    m_currentMapIcon.setHighlightFlash(m_highlightFlash->isChecked());
    m_currentMapIcon.setHighlightUseSpawnColorPen(m_highlightUseSpawnColorPen->isChecked());
    pen = m_currentMapIcon.highlightPen();
    pen.setStyle(Qt::PenStyle(m_highlightPenStyle->currentItem()));
    pen.setWidth(m_highlightPenWidth->value());
    m_currentMapIcon.setHighlightPen(pen);
    m_currentMapIcon.setHighlightUseSpawnColorBrush(m_highlightUseSpawnColorBrush->isChecked());
    brush = m_currentMapIcon.highlightBrush();
    brush.setStyle(Qt::BrushStyle(m_highlightBrushStyle->currentItem()));
    m_currentMapIcon.setHighlightBrush(brush);
 
    // get line settings
    m_currentMapIcon.setShowLine0(m_showLine0->isChecked());
    pen = m_currentMapIcon.line0Pen();
    pen.setStyle(Qt::PenStyle(m_line0PenStyle->currentItem()));
    pen.setWidth(m_line0PenWidth->value());
    m_currentMapIcon.setLine0Pen(pen);
    m_currentMapIcon.setLine1Distance(m_line1Distance->value());
    pen = m_currentMapIcon.line1Pen();
    pen.setStyle(Qt::PenStyle(m_line1PenStyle->currentItem()));
    pen.setWidth(m_line1PenWidth->value());
    m_currentMapIcon.setLine1Pen(pen);
    m_currentMapIcon.setLine2Distance(m_line2Distance->value());
    pen = m_currentMapIcon.line2Pen();
    pen.setStyle(Qt::PenStyle(m_line2PenStyle->currentItem()));
    pen.setWidth(m_line2PenWidth->value());
    m_currentMapIcon.setLine2Pen(pen);
    
    // get general settings
    m_currentMapIcon.setShowName(m_showName->isChecked());
    m_currentMapIcon.setShowWalkPath(m_showWalkPath->isChecked());
    m_currentMapIcon.setUseWalkPathPen(m_useWalkPathPen->isChecked());
    pen = m_currentMapIcon.walkPathPen();
    pen.setStyle(Qt::PenStyle(m_walkPathPenStyle->currentItem()));
    pen.setWidth(m_walkPathPenWidth->value());
    m_currentMapIcon.setWalkPathPen(pen);

    // make sure the map icons object has been set
    if (m_mapIcons)
    {
	// set the current map icon types settings
	m_mapIcons->setIcon(m_currentMapIconType, m_currentMapIcon);
	
	// retrieve the current map icon types settings (just to be sure)
	m_currentMapIcon = m_mapIcons->icon(m_currentMapIconType);
    }
    
    // re-setup the display 
    setupMapIconDisplay();
}

void MapIconDialog::revert()
{
    // revert the map icon data to the last backup
    m_currentMapIcon = m_currentMapIconBackup;
    
    // re-setup the display 
    setupMapIconDisplay();
}


void MapIconDialog::init()
{
  
    QString temp;
    // setup the map icons combo box
    for (int i = tIconTypeDrop; i <= tIconTypeMax; i++)
      m_mapIconCombo->insertItem(MapIcons::iconTypeName((MapIconType)i), i-1);
    
    int sizeWH = m_mapIconCombo->height() - 8;
    int size = sizeWH >> 1;
    QPoint point(size, size);
    // setup the image styles
    QPixmap pix(QSize(sizeWH+1, sizeWH+1));
    QPen pen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    for (int i = tIconStyleNone; i <= tIconStyleMax; i++)
    {
	pix.fill(Qt::white);
	QPainter p(&pix);
	p.setPen(pen);
	p.setBrush(QBrush(Qt::gray));
	MapIcon::paintIconImage(MapIconStyle(i), p, point, size, sizeWH);
	p.end();
      
	temp = MapIcon::iconStyleName((MapIconStyle)i);
	m_imageImage->insertItem(pix, temp, i);
	m_highlightImage->insertItem(pix, temp, i);
    }
        
    // setup the image sizes
    for (int i = tIconSizeNone; i <= tIconSizeMax; i++)
    {
	temp = MapIcon::iconSizeName((MapIconSize)i);
	m_imageSize->insertItem(temp, i);
	m_highlightSize->insertItem(temp, i);
    }
        
    const QString penStyleNames[] =
    {
	"None",
	"Solid",
	"Dash",
	"Dot",
	"Dash Dot",
	"Dash Dot Dot"
    };
    
    // setup pen style names
    pen = QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    for (int i = Qt::NoPen; i <= Qt::DashDotDotLine; i++)
    {
	pix.fill(Qt::white);
	QPainter p(&pix);
	pen.setStyle(Qt::PenStyle(i));
	p.setPen(pen);
	p.setBrush(QBrush(Qt::gray));
	p.drawLine(point.x() - size, point.y() - size,
		   point.x() + size, point.y() + size);
	p.end();

	m_imagePenStyle->insertItem(pix, penStyleNames[i], i);
	m_highlightPenStyle->insertItem(pix, penStyleNames[i], i);
	m_line0PenStyle->insertItem(pix, penStyleNames[i], i);
	m_line1PenStyle->insertItem(pix, penStyleNames[i], i);
	m_line2PenStyle->insertItem(pix, penStyleNames[i], i);
	m_walkPathPenStyle->insertItem(pix, penStyleNames[i], i);
    }
    
    const QString brushStyleNames[] = 
    {	
	"None",
	"Solid",
	"94% Fill",
	"88% Fill",
	"63% Fill",
	"50% Fill",
	"37% Fill",
	"12% Fill",
	"6 % Fill",
	"Horizontal Lines",
	"Vertical Lines",
	"Crossing Lines",
	"Diagonal Lines /",
	"Diagonal Lines \\",
	"Diagonal Cross Lines",
    };
 
    // setup brush style names
    pen = QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    pen.setWidth(0);
    for (int i = Qt::NoBrush; i <= Qt::DiagCrossPattern; i++)
    {
	pix.fill(Qt::white);
	QPainter p(&pix);
	p.setPen(pen);
	p.setBrush(QBrush(Qt::BrushStyle(i)));
	p.drawRect(point.x() - size, point.y() - size, sizeWH, sizeWH);
	p.end();

	m_imageBrushStyle->insertItem(pix, brushStyleNames[i], i);
	m_highlightBrushStyle->insertItem(pix, brushStyleNames[i], i);
    }
    
    // setup the display 
    setupMapIconDisplay();
}

void MapIconDialog::destroy()
{

}

void MapIconDialog::setMapIcons(MapIcons* mapIcons)
{
    // set the map icons member
    m_mapIcons = mapIcons;
    
    // set the current map icon type
    m_currentMapIconType = MapIconType(m_mapIconCombo->currentItem() + 1);
    m_currentMapIcon = m_mapIcons->icon(m_currentMapIconType);
    m_currentMapIconBackup = m_currentMapIcon;
    
    // setup the map icon display
    setupMapIconDisplay();
}

void MapIconDialog::mapIconCombo_activated( int id)
{
    // validate state and input
    if (!m_mapIcons || (id >= tIconTypeMax))
	return;
    
    // set the current map icon
    m_currentMapIconType = MapIconType(id + 1);
    m_currentMapIcon = m_mapIcons->icon(m_currentMapIconType);
    m_currentMapIconBackup = m_currentMapIcon;
    
    // setup the display 
    setupMapIconDisplay();
}

void MapIconDialog::imagePenColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.imagePen().color(),
			                  this, caption() + " Image Outline");
    
    if (color.isValid())
    {
	QPen pen = m_currentMapIcon.imagePen();
	pen.setColor(color);
	m_currentMapIcon.setImagePen(pen);
	m_imagePenColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::imageBrushColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.imageBrush().color(),
			                  this, caption() + " Image Fill");
    
    if (color.isValid())
    {
	QBrush brush = m_currentMapIcon.imageBrush();
	brush.setColor(color);
	m_currentMapIcon.setImageBrush(brush );
	m_imageBrushColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::highlightPenColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.highlightPen().color(),
			                  this, caption() + " Highlight Outline");
    
    if (color.isValid())
    {
	QPen pen = m_currentMapIcon.highlightPen();
	pen.setColor(color);
	m_currentMapIcon.setHighlightPen(pen);
	m_highlightPenColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::highlightBrushColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.highlightBrush().color(),
			                  this, caption() + " Highlight Fill");
    
    if (color.isValid())
    {
	QBrush brush = m_currentMapIcon.highlightBrush();
	brush.setColor(color);
	m_currentMapIcon.setHighlightBrush(brush );
	m_highlightBrushColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::line0PenColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.line0Pen().color(),
			                  this, caption() + " Line 0");
    
    if (color.isValid())
    {
	QPen pen = m_currentMapIcon.line0Pen();
	pen.setColor(color);
	m_currentMapIcon.setLine0Pen(pen);
	m_line0PenColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::line1PenColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.line1Pen().color(),
			                  this, caption() + " Line 1");
    
    if (color.isValid())
    {
	QPen pen = m_currentMapIcon.line1Pen();
	pen.setColor(color);
	m_currentMapIcon.setLine1Pen(pen);
	m_line1PenColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::line2PenColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.line2Pen().color(),
			                  this, caption() + " Line 2");
    
    if (color.isValid())
    {
	QPen pen = m_currentMapIcon.line2Pen();
	pen.setColor(color);
	m_currentMapIcon.setLine2Pen(pen);
	m_line2PenColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::walkPathPenColor_clicked()
{
    QColor color = QColorDialog::getColor(m_currentMapIcon.walkPathPen().color(),
			                  this, caption() + " Walk Path Line");
    
    if (color.isValid())
    {
	QPen pen = m_currentMapIcon.walkPathPen();
	pen.setColor(color);
	m_currentMapIcon.setWalkPathPen(pen);
	m_walkPathPenColorSample->setPaletteBackgroundColor(color);
    }
}

void MapIconDialog::setupMapIconDisplay()
{
    // setup image fields
    m_useImage->setChecked(m_currentMapIcon.image());
    m_imageImage->setCurrentItem(m_currentMapIcon.imageStyle());
    m_imageSize->setCurrentItem(m_currentMapIcon.imageSize());
    m_imageFlash->setChecked(m_currentMapIcon.imageFlash());
    m_imageUseSpawnColorPen->setChecked(m_currentMapIcon.imageUseSpawnColorPen());
    m_imagePenColorSample->setPaletteBackgroundColor(m_currentMapIcon.imagePen().color());
    m_imagePenStyle->setCurrentItem(m_currentMapIcon.imagePen().style());
    m_imagePenWidth->setValue(m_currentMapIcon.imagePen().width());
    m_imageUseSpawnColorBrush->setChecked(m_currentMapIcon.imageUseSpawnColorBrush());
    m_imageBrushColorSample->setPaletteBackgroundColor(m_currentMapIcon.imageBrush().color());
    m_imageBrushStyle->setCurrentItem(m_currentMapIcon.imageBrush().style());
    
    // setup highlight fields
    m_useHighlight->setChecked(m_currentMapIcon.highlight());
    m_highlightImage->setCurrentItem(m_currentMapIcon.highlightStyle());
    m_highlightSize->setCurrentItem(m_currentMapIcon.highlightSize());
    m_highlightFlash->setChecked(m_currentMapIcon.highlightFlash());
    m_highlightUseSpawnColorPen->setChecked(m_currentMapIcon.highlightUseSpawnColorPen());
    m_highlightPenColorSample->setPaletteBackgroundColor(m_currentMapIcon.highlightPen().color());
    m_highlightPenStyle->setCurrentItem(m_currentMapIcon.highlightPen().style());
    m_highlightPenWidth->setValue(m_currentMapIcon.highlightPen().width());
    m_highlightUseSpawnColorBrush->setChecked(m_currentMapIcon.highlightUseSpawnColorBrush());
    m_highlightBrushColorSample->setPaletteBackgroundColor(m_currentMapIcon.highlightBrush().color());
    m_highlightBrushStyle->setCurrentItem(m_currentMapIcon.highlightBrush().style());
    
    // setup lines
    m_showLine0->setChecked(m_currentMapIcon.showLine0());
    m_line0PenColorSample->setPaletteBackgroundColor(m_currentMapIcon.line0Pen().color());
    m_line0PenStyle->setCurrentItem(m_currentMapIcon.line0Pen().style());
    m_line0PenWidth->setValue(m_currentMapIcon.line0Pen().width());
    m_line1Distance->setValue(m_currentMapIcon.line1Distance());
    m_line1PenColorSample->setPaletteBackgroundColor(m_currentMapIcon.line1Pen().color());
    m_line1PenStyle->setCurrentItem(m_currentMapIcon.line1Pen().style());
    m_line1PenWidth->setValue(m_currentMapIcon.line1Pen().width());
    m_line2Distance->setValue(m_currentMapIcon.line2Distance());
    m_line2PenColorSample->setPaletteBackgroundColor(m_currentMapIcon.line2Pen().color());
    m_line2PenStyle->setCurrentItem(m_currentMapIcon.line2Pen().style());
    m_line2PenWidth->setValue(m_currentMapIcon.line2Pen().width());
    
    // setup other
    m_showName->setChecked(m_currentMapIcon.showName());
    m_showWalkPath->setChecked(m_currentMapIcon.showWalkPath());
    m_useWalkPathPen->setChecked(m_currentMapIcon.useWalkPathPen());
    m_walkPathPenColorSample->setPaletteBackgroundColor(m_currentMapIcon.walkPathPen().color());
    m_walkPathPenStyle->setCurrentItem(m_currentMapIcon.walkPathPen().style());
    m_walkPathPenWidth->setValue(m_currentMapIcon.walkPathPen().width());
}
