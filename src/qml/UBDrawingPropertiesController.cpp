/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBDrawingPropertiesController.h"

#include "core/UBSettings.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardController.h"

UBDrawingPropertiesController::UBDrawingPropertiesController(QObject* parent)
    : QObject(parent)
    , mVisible(false)
{
    mSettings = UBSettings::settings();
    mDrawingController = UBDrawingController::drawingController();

    connect(mDrawingController, &UBDrawingController::stylusToolChanged, this, &UBDrawingPropertiesController::onStylusToolChanged);
    connect(mDrawingController, &UBDrawingController::colorPaletteChanged, this, &UBDrawingPropertiesController::onColorPaletteChanged);
    connect(mDrawingController, &UBDrawingController::colorIndexChanged, this, &UBDrawingPropertiesController::onColorIndexChangedFromController);
    connect(mDrawingController, &UBDrawingController::lineWidthIndexChanged, this, &UBDrawingPropertiesController::onLineWidthIndexChanged);
}

QString UBDrawingPropertiesController::activeTool() const
{
    int tool = mDrawingController->stylusTool();
    switch (tool) {
        case UBStylusTool::Pen: return "pen";
        case UBStylusTool::Line: return "pen";
        case UBStylusTool::Marker: return "marker";
        case UBStylusTool::Eraser: return "eraser";
        default: return "other";
    }
}

bool UBDrawingPropertiesController::isDrawingTool() const
{
    return mDrawingController->isDrawingTool();
}

QVariantList UBDrawingPropertiesController::colors() const
{
    QVariantList result;
    bool isDark = mSettings->isDarkBackground();

    QList<QColor> colorList;
    int tool = mDrawingController->stylusTool();
    if (tool == UBStylusTool::Marker)
        colorList = mSettings->markerColors(isDark);
    else
        colorList = mSettings->penColors(isDark);

    // Return only the first colorPaletteSize colors
    for (int i = 0; i < UBSettings::colorPaletteSize && i < colorList.size(); ++i)
        result.append(QVariant::fromValue(colorList.at(i)));

    return result;
}

int UBDrawingPropertiesController::colorIndex() const
{
    return mDrawingController->currentToolColorIndex();
}

int UBDrawingPropertiesController::widthIndex() const
{
    return mDrawingController->currentToolWidthIndex();
}

int UBDrawingPropertiesController::eraserWidthIndex() const
{
    return mSettings->eraserWidthIndex();
}

bool UBDrawingPropertiesController::visible() const
{
    int tool = mDrawingController->stylusTool();
    return (tool == UBStylusTool::Pen || tool == UBStylusTool::Line
         || tool == UBStylusTool::Marker || tool == UBStylusTool::Eraser);
}

void UBDrawingPropertiesController::setColorIndex(int index)
{
    // Route through UBBoardController which also handles tool auto-selection
    if (UBApplication::boardController)
        UBApplication::boardController->setColorIndex(index);
}

void UBDrawingPropertiesController::setWidthIndex(int index)
{
    mDrawingController->setLineWidthIndex(index);
}

void UBDrawingPropertiesController::setEraserWidthIndex(int index)
{
    mDrawingController->setEraserWidthIndex(index);
}

void UBDrawingPropertiesController::onStylusToolChanged(int /*tool*/)
{
    bool nowVisible = visible();
    if (nowVisible != mVisible)
    {
        mVisible = nowVisible;
        emit visibleChanged();
    }
    emit activeToolChanged();
    emit colorsChanged();
    emit colorIndexChanged();
    emit widthIndexChanged();
    emit eraserWidthIndexChanged();
}

void UBDrawingPropertiesController::onColorPaletteChanged()
{
    emit colorsChanged();
    emit colorIndexChanged();
}

void UBDrawingPropertiesController::onColorIndexChangedFromController(int /*index*/)
{
    emit colorIndexChanged();
}

void UBDrawingPropertiesController::onLineWidthIndexChanged(int /*index*/)
{
    emit widthIndexChanged();
}
