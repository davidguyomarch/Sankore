/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBDrawingController.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"

#include "domain/UBGraphicsScene.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "domain/UBEditableGraphicsPolygonItem.h"

#include "board/UBBoardView.h"

#include "gui/UBMainWindow.h"

UBDrawingController* UBDrawingController::sDrawingController = 0;


UBDrawingController* UBDrawingController::drawingController()
{
    if(!sDrawingController)
        sDrawingController = new UBDrawingController();

    return sDrawingController;
}

void UBDrawingController::destroy()
{
    if(sDrawingController)
        delete sDrawingController;
    sDrawingController = nullptr;
}

UBDrawingController::UBDrawingController(QObject * parent)
    : QObject(parent)
    , mActiveRuler(nullptr)
    , mStylusTool((UBStylusTool::Enum)-1)
    , mLatestDrawingTool((UBStylusTool::Enum)-1)
	, mIsDesktopMode(false)
{
    mSettings = UBSettings::settings();
    connect(mSettings, &UBSettings::colorContextChanged, this, &UBDrawingController::colorPaletteChanged);

    connect(UBApplication::mainWindow->actionPen, &QAction::triggered, this, &UBDrawingController::penToolSelected);
    connect(UBApplication::mainWindow->actionEraser, &QAction::triggered, this, &UBDrawingController::eraserToolSelected);
    connect(UBApplication::mainWindow->actionMarker, &QAction::triggered, this, &UBDrawingController::markerToolSelected);
    connect(UBApplication::mainWindow->actionSelector, &QAction::triggered, this, &UBDrawingController::selectorToolSelected);
    connect(UBApplication::mainWindow->actionPlay, &QAction::triggered, this, &UBDrawingController::playToolSelected);
    connect(UBApplication::mainWindow->actionHand, &QAction::triggered, this, &UBDrawingController::handToolSelected);
    connect(UBApplication::mainWindow->actionZoomIn, &QAction::triggered, this, &UBDrawingController::zoomInToolSelected);
    connect(UBApplication::mainWindow->actionZoomOut, &QAction::triggered, this, &UBDrawingController::zoomOutToolSelected);
    connect(UBApplication::mainWindow->actionPointer, &QAction::triggered, this, &UBDrawingController::pointerToolSelected);
    connect(UBApplication::mainWindow->actionLine, &QAction::triggered, this, &UBDrawingController::lineToolSelected);
    connect(UBApplication::mainWindow->actionText, &QAction::triggered, this, &UBDrawingController::textToolSelected);
    connect(UBApplication::mainWindow->actionCapture, &QAction::triggered, this, &UBDrawingController::captureToolSelected);
    connect(UBApplication::mainWindow->actionOcr, &QAction::triggered, this, &UBDrawingController::ocrToolSelected);

    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &UBDrawingController::onActiveSceneChanged);
}


UBDrawingController::~UBDrawingController()
{
    // NOOP
}


int UBDrawingController::stylusTool()
{
    return mStylusTool;
}


int UBDrawingController::latestDrawingTool()
{
    return mLatestDrawingTool;
}


void UBDrawingController::setStylusTool(int tool)
{
    if (tool != mStylusTool)
    {
        //Ev-7 - NNE - 20140106
        if(tool != UBStylusTool::Drawing)
        {
            UBApplication::boardController->activeScene()->deselectAllItems();

            // hideSubPalettes disabled — legacy drawing palette removed
        }
        else
        {
            for (QGraphicsItem *gi : UBApplication::boardController->activeScene()->selectedItems())
            {
                UBShapeFactory::desactivateEditionMode(gi);
            }
        }        

        if (mStylusTool == UBStylusTool::Pen || mStylusTool == UBStylusTool::Marker
                || mStylusTool == UBStylusTool::Line)
        {
            mLatestDrawingTool = mStylusTool;
        }

        if (tool == UBStylusTool::Pen || tool == UBStylusTool::Line)
        {
             emit lineWidthIndexChanged(mSettings->penWidthIndex());
             emit colorIndexChanged(mSettings->penColorIndex());
        }
        else if (tool == UBStylusTool::Marker)
        {
            emit lineWidthIndexChanged(mSettings->markerWidthIndex());
            emit colorIndexChanged(mSettings->markerColorIndex());
        }

        mStylusTool = (UBStylusTool::Enum)tool;


        if (mStylusTool == UBStylusTool::Pen)
            UBApplication::mainWindow->actionPen->setChecked(true);
        else if (mStylusTool == UBStylusTool::Eraser)
            UBApplication::mainWindow->actionEraser->setChecked(true);
        else if (mStylusTool == UBStylusTool::Marker)
            UBApplication::mainWindow->actionMarker->setChecked(true);
        else if (mStylusTool == UBStylusTool::Selector)
            UBApplication::mainWindow->actionSelector->setChecked(true);
        else if (mStylusTool == UBStylusTool::Play)
            UBApplication::mainWindow->actionPlay->setChecked(true);
        else if (mStylusTool == UBStylusTool::Hand)
            UBApplication::mainWindow->actionHand->setChecked(true);
        else if (mStylusTool == UBStylusTool::ZoomIn)
            UBApplication::mainWindow->actionZoomIn->setChecked(true);
        else if (mStylusTool == UBStylusTool::ZoomOut)
            UBApplication::mainWindow->actionZoomOut->setChecked(true);
        else if (mStylusTool == UBStylusTool::Pointer)
            UBApplication::mainWindow->actionPointer->setChecked(true);
        else if (mStylusTool == UBStylusTool::Line)
            UBApplication::mainWindow->actionLine->setChecked(true);
        else if (mStylusTool == UBStylusTool::Text)
            UBApplication::mainWindow->actionText->setChecked(true);
        /* ALTI/AOU - 20140606 : RichTextEditor tool isn't available anymore.
        else if (mStylusTool == UBStylusTool::RichText)
            UBApplication::mainWindow->actionRichTextEditor->setChecked(true);
        */
        else if (mStylusTool == UBStylusTool::Capture)
            UBApplication::mainWindow->actionCapture->setChecked(true);
        else if (mStylusTool == UBStylusTool::Ocr)
            UBApplication::mainWindow->actionOcr->setChecked(true);

        if(mStylusTool != UBStylusTool::Drawing){
            UBApplication::boardController->shapeFactory().desactivate();
        }

        emit stylusToolChanged(tool);
        emit colorPaletteChanged();
    }

    //EV-7 : ce n'est pas de la responsabilité de cette méthode de le faire ... mais plus beaucoup de temps..
    deactivateCreationModeForGraphicsPathItems();

}


void UBDrawingController::onActiveSceneChanged()
{
    for (QGraphicsItem* gi : UBApplication::boardController->activeScene()->items())
    {
        if (gi->type() == UBGraphicsItemType::GraphicsPathItemType)
        {
            UBEditableGraphicsPolygonItem* path = dynamic_cast<UBEditableGraphicsPolygonItem*>(gi);
            if (path){
                path->setIsInCreationMode(false);
            }
        }
    }

    UBApplication::boardController->shapeFactory().terminateShape();
}

void UBDrawingController::deactivateCreationModeForGraphicsPathItems()
{
    for (QGraphicsItem* gi : UBApplication::boardController->activeScene()->items())
    {
        if (gi->type() == UBGraphicsItemType::GraphicsPathItemType)
        {
            UBEditableGraphicsPolygonItem* path = dynamic_cast<UBEditableGraphicsPolygonItem*>(gi);
            if (path){
                path->setIsInCreationMode(false);
                UBApplication::boardController->shapeFactory().desactivate();
                if (path->path().elementCount() < 2)
                    UBApplication::boardController->controlView()->scene()->removeItem(gi);
            }
        }
    }
    UBApplication::boardController->controlView()->resetCachedContent();
}

bool UBDrawingController::isDrawingTool()
{
    return (stylusTool() == UBStylusTool::Pen)
            || (stylusTool() == UBStylusTool::Marker)
            || (stylusTool() == UBStylusTool::Line);
}


int UBDrawingController::currentToolWidthIndex()
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
    {
        return mSettings->penWidthIndex();
    }
    else if (stylusTool() == UBStylusTool::Marker)
    {
        return mSettings->markerWidthIndex();
    }
    else
    {
        return -1;
    }
}


qreal UBDrawingController::currentToolWidth()
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
    {
        return mSettings->currentPenWidth();
    }
    else if (stylusTool() == UBStylusTool::Marker)
    {
        return mSettings->currentMarkerWidth();
    }
    else
    {
        //failsafe
        return mSettings->currentPenWidth();
    }
}


void UBDrawingController::setLineWidthIndex(int index)
{
    if (stylusTool() == UBStylusTool::Marker)
    {
        mSettings->setMarkerWidthIndex(index);
    }
    else
    {
        mSettings->setPenWidthIndex(index);

        if(stylusTool() != UBStylusTool::Line
            && stylusTool() != UBStylusTool::Selector)
        {
            setStylusTool(UBStylusTool::Pen);
        }
    }

    emit lineWidthIndexChanged(index);
}


int UBDrawingController::currentToolColorIndex()
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
    {
        return mSettings->penColorIndex();
    }
    else if (stylusTool() == UBStylusTool::Marker)
    {
        return mSettings->markerColorIndex();
    }
    else
    {
        return -1;
    }
}


QColor UBDrawingController::currentToolColor()
{
    return toolColor(mSettings->isDarkBackground());
}


QColor UBDrawingController::toolColor(bool onDarkBackground)
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
    {
        return mSettings->penColor(onDarkBackground);
    }
    else if (stylusTool() == UBStylusTool::Marker)
    {
        return mSettings->markerColor(onDarkBackground);
    }
    else
    {
        //failsafe
        if (onDarkBackground)
        {
            return Qt::white;
        }
        else
        {
            return Qt::black;
        }
    }
}


void UBDrawingController::setColorIndex(int index)
{
    Q_ASSERT(index >= 0 && index < mSettings->colorPaletteSize);

    if (stylusTool() == UBStylusTool::Marker)
    {
        mSettings->setMarkerColorIndex(index);
    }
    else
    {
        mSettings->setPenColorIndex(index);
    }

    emit colorIndexChanged(index);
}


void UBDrawingController::setEraserWidthIndex(int index)
{
    setStylusTool(UBStylusTool::Eraser);
    mSettings->setEraserWidthIndex(index);
}

void UBDrawingController::setPenColor(bool onDarkBackground, const QColor& color, int pIndex)
{
    if (onDarkBackground)
    {
        mSettings->boardPenDarkBackgroundSelectedColors->setColor(pIndex, color);
    }
    else
    {
        mSettings->boardPenLightBackgroundSelectedColors->setColor(pIndex, color);
    }

    emit colorPaletteChanged();
}


void UBDrawingController::setMarkerColor(bool onDarkBackground, const QColor& color, int pIndex)
{
    if (onDarkBackground)
    {
        mSettings->boardMarkerDarkBackgroundSelectedColors->setColor(pIndex, color);
    }
    else
    {
        mSettings->boardMarkerLightBackgroundSelectedColors->setColor(pIndex, color);
    }

    emit colorPaletteChanged();
}


void UBDrawingController::setMarkerAlpha(qreal alpha)
{
    mSettings->boardMarkerLightBackgroundColors->setAlpha(alpha);
    mSettings->boardMarkerLightBackgroundSelectedColors->setAlpha(alpha);

    mSettings->boardMarkerDarkBackgroundColors->setAlpha(alpha);
    mSettings->boardMarkerDarkBackgroundSelectedColors->setAlpha(alpha);

    mSettings->boardMarkerAlpha->set(alpha);

    emit colorPaletteChanged();
}


void UBDrawingController::penToolSelected(bool checked)
{
    if (checked)
    {
        setStylusTool(UBStylusTool::Pen);

    }
}

void UBDrawingController::eraserToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Eraser);
}

void UBDrawingController::markerToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Marker);
}

void UBDrawingController::selectorToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Selector);
}

void UBDrawingController::playToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Play);
}

void UBDrawingController::handToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Hand);
}


void UBDrawingController::zoomInToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::ZoomIn);
}


void UBDrawingController::zoomOutToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::ZoomOut);
}


void UBDrawingController::pointerToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Pointer);
}


void UBDrawingController::lineToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Line);
}


void UBDrawingController::textToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Text);
}

void UBDrawingController::richTextToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::RichText);
}


void UBDrawingController::captureToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Capture);
}

void UBDrawingController::ocrToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Ocr);
}

