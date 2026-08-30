/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBToolController.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "gui/UBMainWindow.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBShapeFactory.h"
#include "domain/UBAlignObjectManager.h"
#include "domain/UBEditableGraphicsPolygonItem.h"

#include <QAction>

// --- Singleton ---

UBToolController* UBToolController::sToolController = nullptr;

UBToolController* UBToolController::toolController()
{
    if (!sToolController)
        sToolController = new UBToolController();
    return sToolController;
}

void UBToolController::destroy()
{
    delete sToolController;
    sToolController = nullptr;
}

// --- Constructor ---

UBToolController::UBToolController(QObject* parent)
    : QObject(parent)
    , m_activeTool(Pen)
    , m_latestDrawingTool((UBStylusTool::Enum)-1)
    , m_shapesVisible(false)
    , m_isDesktopMode(false)
    , mSettings(UBSettings::settings())
{
    connect(mSettings, &UBSettings::colorContextChanged, this, &UBToolController::colorPaletteChanged);

    // Connect active scene changes
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged,
            this, &UBToolController::onActiveSceneChanged);

    // Connect keyboard shortcuts (QActions) to setActiveTool()
    auto connectTool = [this](QAction* action, int tool) {
        connect(action, &QAction::triggered, this, [this, tool](bool checked) {
            if (checked) setActiveTool(tool);
        });
    };
    auto* mw = UBApplication::mainWindow;
    connectTool(mw->actionPen,      Pen);
    connectTool(mw->actionEraser,   Eraser);
    connectTool(mw->actionMarker,   Marker);
    connectTool(mw->actionSelector, Selector);
    connectTool(mw->actionPlay,     Play);
    connectTool(mw->actionHand,     Hand);
    connectTool(mw->actionZoomIn,   ZoomIn);
    connectTool(mw->actionZoomOut,  ZoomOut);
    connectTool(mw->actionPointer,  Pointer);
    connectTool(mw->actionLine,     Line);
    connectTool(mw->actionText,     Text);
    connectTool(mw->actionCapture,  Capture);
    connectTool(mw->actionOcr,      Ocr);
}

// =========================================================================
// Active Tool
// =========================================================================

int UBToolController::activeTool() const
{
    return m_activeTool;
}

void UBToolController::setActiveTool(int tool)
{
    if (m_activeTool == tool)
        return;

    // Delegate to setStylusTool which handles all side-effects
    setStylusTool(tool);

    emit activeToolChanged();
    emit currentColorsChanged();
    emit currentColorIndexChanged();
    emit currentWidthIndexChanged();
}

void UBToolController::setStylusTool(int tool)
{
    if (tool == m_activeTool && tool != (int)UBStylusTool::Drawing)
        return;

    // Side-effect: deselect items or deactivate shape edition
    if (tool != UBStylusTool::Drawing)
    {
        if (UBApplication::boardController && UBApplication::boardController->activeScene())
            UBApplication::boardController->activeScene()->deselectAllItems();
    }
    else
    {
        if (UBApplication::boardController && UBApplication::boardController->activeScene())
        {
            for (QGraphicsItem *gi : UBApplication::boardController->activeScene()->selectedItems())
                UBShapeFactory::desactivateEditionMode(gi);
        }
    }

    // Remember latest drawing tool
    if (m_activeTool == UBStylusTool::Pen || m_activeTool == UBStylusTool::Marker
            || m_activeTool == UBStylusTool::Line)
    {
        m_latestDrawingTool = (UBStylusTool::Enum)m_activeTool;
    }

    // Emit width/color index signals for the new tool
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

    m_activeTool = tool;

    // Sync QAction checked states
    auto* mw = UBApplication::mainWindow;
    if (mw)
    {
        if (m_activeTool == UBStylusTool::Pen)          mw->actionPen->setChecked(true);
        else if (m_activeTool == UBStylusTool::Eraser)   mw->actionEraser->setChecked(true);
        else if (m_activeTool == UBStylusTool::Marker)   mw->actionMarker->setChecked(true);
        else if (m_activeTool == UBStylusTool::Selector)  mw->actionSelector->setChecked(true);
        else if (m_activeTool == UBStylusTool::Play)      mw->actionPlay->setChecked(true);
        else if (m_activeTool == UBStylusTool::Hand)      mw->actionHand->setChecked(true);
        else if (m_activeTool == UBStylusTool::ZoomIn)    mw->actionZoomIn->setChecked(true);
        else if (m_activeTool == UBStylusTool::ZoomOut)   mw->actionZoomOut->setChecked(true);
        else if (m_activeTool == UBStylusTool::Pointer)   mw->actionPointer->setChecked(true);
        else if (m_activeTool == UBStylusTool::Line)      mw->actionLine->setChecked(true);
        else if (m_activeTool == UBStylusTool::Text)      mw->actionText->setChecked(true);
        else if (m_activeTool == UBStylusTool::Capture)   mw->actionCapture->setChecked(true);
        else if (m_activeTool == UBStylusTool::Ocr)       mw->actionOcr->setChecked(true);
    }

    // Deactivate shape factory when switching away from Drawing
    if (m_activeTool != UBStylusTool::Drawing)
    {
        if (UBApplication::boardController)
            UBApplication::boardController->shapeFactory().desactivate();
    }

    emit stylusToolChanged(tool);
    emit colorPaletteChanged();

    deactivateCreationModeForGraphicsPathItems();
}

bool UBToolController::isDrawingTool() const
{
    return (m_activeTool == UBStylusTool::Pen)
            || (m_activeTool == UBStylusTool::Marker)
            || (m_activeTool == UBStylusTool::Line);
}

int UBToolController::latestDrawingTool() const
{
    return m_latestDrawingTool;
}

// =========================================================================
// Scene change handlers
// =========================================================================

void UBToolController::onActiveSceneChanged()
{
    if (!UBApplication::boardController || !UBApplication::boardController->activeScene())
        return;

    for (QGraphicsItem* gi : UBApplication::boardController->activeScene()->items())
    {
        if (gi->type() == UBGraphicsItemType::GraphicsPathItemType)
        {
            UBEditableGraphicsPolygonItem* path = dynamic_cast<UBEditableGraphicsPolygonItem*>(gi);
            if (path)
                path->setIsInCreationMode(false);
        }
    }

    UBApplication::boardController->shapeFactory().terminateShape();
}

void UBToolController::deactivateCreationModeForGraphicsPathItems()
{
    if (!UBApplication::boardController || !UBApplication::boardController->activeScene())
        return;

    for (QGraphicsItem* gi : UBApplication::boardController->activeScene()->items())
    {
        if (gi->type() == UBGraphicsItemType::GraphicsPathItemType)
        {
            UBEditableGraphicsPolygonItem* path = dynamic_cast<UBEditableGraphicsPolygonItem*>(gi);
            if (path) {
                path->setIsInCreationMode(false);
                UBApplication::boardController->shapeFactory().desactivate();
                if (path->path().elementCount() < 2)
                    UBApplication::boardController->controlView()->scene()->removeItem(gi);
            }
        }
    }
    if (UBApplication::boardController->controlView())
        UBApplication::boardController->controlView()->resetCachedContent();
}

// =========================================================================
// Color / Width — pass-through to UBSettings
// =========================================================================

int UBToolController::currentToolWidthIndex() const
{
    if (m_activeTool == UBStylusTool::Pen || m_activeTool == UBStylusTool::Line)
        return mSettings->penWidthIndex();
    else if (m_activeTool == UBStylusTool::Marker)
        return mSettings->markerWidthIndex();
    return -1;
}

qreal UBToolController::currentToolWidth() const
{
    if (m_activeTool == UBStylusTool::Pen || m_activeTool == UBStylusTool::Line)
        return mSettings->currentPenWidth();
    else if (m_activeTool == UBStylusTool::Marker)
        return mSettings->currentMarkerWidth();
    return 0;
}

int UBToolController::currentToolColorIndex() const
{
    if (m_activeTool == UBStylusTool::Marker)
        return mSettings->markerColorIndex();
    return mSettings->penColorIndex();
}

QColor UBToolController::currentToolColor() const
{
    return toolColor(mSettings->isDarkBackground());
}

QColor UBToolController::toolColor(bool onDarkBackground) const
{
    if (m_activeTool == UBStylusTool::Pen || m_activeTool == UBStylusTool::Line)
        return mSettings->penColor(onDarkBackground);
    else if (m_activeTool == UBStylusTool::Marker)
        return mSettings->markerColor(onDarkBackground);
    return onDarkBackground ? Qt::white : Qt::black;
}

// --- Pen ---

QColor UBToolController::penColor() const
{
    return currentToolColor();
}

int UBToolController::penColorIndex() const
{
    return mSettings->penColorIndex();
}

void UBToolController::setPenColorIndex(int index)
{
    mSettings->setPenColorIndex(index);
    emit penColorChanged();
    emit currentColorIndexChanged();
}

int UBToolController::penWidthIndex() const
{
    return mSettings->penWidthIndex();
}

void UBToolController::setPenWidthIndex(int index)
{
    mSettings->setPenWidthIndex(index);
    emit penWidthChanged();
    emit currentWidthIndexChanged();
}

QList<QColor> UBToolController::penColors() const
{
    bool dark = UBApplication::boardController && UBApplication::boardController->activeScene()
                && UBApplication::boardController->activeScene()->isDarkBackground();
    return dark ? mSettings->boardPenDarkBackgroundSelectedColors->colors()
                : mSettings->boardPenLightBackgroundSelectedColors->colors();
}

void UBToolController::setPenColor(bool onDarkBackground, const QColor& color, int pIndex)
{
    if (onDarkBackground)
        mSettings->boardPenDarkBackgroundSelectedColors->setColor(pIndex, color);
    else
        mSettings->boardPenLightBackgroundSelectedColors->setColor(pIndex, color);
    emit colorPaletteChanged();
}

// --- Marker ---

QColor UBToolController::markerColor() const
{
    return currentToolColor();
}

int UBToolController::markerColorIndex() const
{
    return mSettings->markerColorIndex();
}

void UBToolController::setMarkerColorIndex(int index)
{
    mSettings->setMarkerColorIndex(index);
    emit markerColorChanged();
    emit currentColorIndexChanged();
}

int UBToolController::markerWidthIndex() const
{
    return mSettings->markerWidthIndex();
}

void UBToolController::setMarkerWidthIndex(int index)
{
    mSettings->setMarkerWidthIndex(index);
    emit markerWidthChanged();
    emit currentWidthIndexChanged();
}

QList<QColor> UBToolController::markerColors() const
{
    bool dark = UBApplication::boardController && UBApplication::boardController->activeScene()
                && UBApplication::boardController->activeScene()->isDarkBackground();
    return dark ? mSettings->boardMarkerDarkBackgroundSelectedColors->colors()
                : mSettings->boardMarkerLightBackgroundSelectedColors->colors();
}

void UBToolController::setMarkerColor(bool onDarkBackground, const QColor& color, int pIndex)
{
    if (onDarkBackground)
        mSettings->boardMarkerDarkBackgroundSelectedColors->setColor(pIndex, color);
    else
        mSettings->boardMarkerLightBackgroundSelectedColors->setColor(pIndex, color);
    emit colorPaletteChanged();
}

void UBToolController::setMarkerAlpha(qreal alpha)
{
    mSettings->boardMarkerLightBackgroundColors->setAlpha(alpha);
    mSettings->boardMarkerLightBackgroundSelectedColors->setAlpha(alpha);
    mSettings->boardMarkerDarkBackgroundColors->setAlpha(alpha);
    mSettings->boardMarkerDarkBackgroundSelectedColors->setAlpha(alpha);
    mSettings->boardMarkerAlpha->set(alpha);
    emit colorPaletteChanged();
}

// --- Index setters (slot versions for signal connections) ---

void UBToolController::setLineWidthIndex(int index)
{
    if (m_activeTool == UBStylusTool::Marker)
        mSettings->setMarkerWidthIndex(index);
    else
        mSettings->setPenWidthIndex(index);
    emit lineWidthIndexChanged(index);
    emit currentWidthIndexChanged();
}

void UBToolController::setColorIndex(int index)
{
    Q_ASSERT(index >= 0 && index < mSettings->colorPaletteSize);
    if (m_activeTool == UBStylusTool::Marker)
        mSettings->setMarkerColorIndex(index);
    else
        mSettings->setPenColorIndex(index);
    emit colorIndexChanged(index);
}

// --- Tool-aware convenience ---

QList<QColor> UBToolController::currentColors() const
{
    if (m_activeTool == Marker)
        return markerColors();
    return penColors();
}

int UBToolController::currentColorIndex() const
{
    if (m_activeTool == Marker)
        return markerColorIndex();
    return penColorIndex();
}

void UBToolController::setCurrentColorIndex(int index)
{
    if (m_activeTool == Marker)
        setMarkerColorIndex(index);
    else
        setPenColorIndex(index);

    emit penColorChanged();
    emit currentColorIndexChanged();
    emit currentColorsChanged();
}

int UBToolController::currentWidthIndex() const
{
    if (m_activeTool == Eraser)
        return eraserWidthIndex();
    if (m_activeTool == Marker)
        return markerWidthIndex();
    return penWidthIndex();
}

void UBToolController::setCurrentWidthIndex(int index)
{
    if (m_activeTool == Eraser)
        setEraserWidthIndex(index);
    else if (m_activeTool == Marker)
        setMarkerWidthIndex(index);
    else
        setPenWidthIndex(index);
    emit currentWidthIndexChanged();
}

// --- Eraser ---

int UBToolController::eraserWidthIndex() const
{
    return mSettings->eraserWidthIndex();
}

void UBToolController::setEraserWidthIndex(int index)
{
    setStylusTool(UBStylusTool::Eraser);
    mSettings->setEraserWidthIndex(index);
    emit eraserWidthChanged();
    emit currentWidthIndexChanged();
}

// --- Drawing Props visibility ---

bool UBToolController::showDrawingProps() const
{
    return (m_activeTool == Pen || m_activeTool == Marker
         || m_activeTool == Line || m_activeTool == Eraser);
}

// --- Shapes ---

bool UBToolController::shapesVisible() const
{
    return m_shapesVisible;
}

void UBToolController::setShapesVisible(bool visible)
{
    if (m_shapesVisible == visible)
        return;
    m_shapesVisible = visible;
    emit shapesVisibleChanged();
}

void UBToolController::toggleShapes()
{
    setShapesVisible(!m_shapesVisible);
}

void UBToolController::createShape(const QString& shape)
{
    auto& factory = UBApplication::boardController->shapeFactory();

    if (shape == "ellipse")         factory.createEllipse(true);
    else if (shape == "rectangle")  factory.createRectangle(true);
    else if (shape == "triangle")   factory.createRegularPolygon(3);
    else if (shape == "diamond")    factory.createRegularPolygon(4);
    else if (shape == "hexagon")    factory.createRegularPolygon(6);
    else if (shape == "star")       factory.createRegularPolygon(5);
    else if (shape == "circle")     factory.createCircle(true);
    else if (shape == "square")     factory.createSquare(true);
    else if (shape == "line")       factory.createLine(true);

    m_activeTool = Drawing;
    emit activeToolChanged();
    emit stylusToolChanged(Drawing);
}

void UBToolController::activateFillTool()
{
    setActiveTool(ChangeFill);
}

void UBToolController::applyStrokeToSelection()
{
    auto& factory = UBApplication::boardController->shapeFactory();
    factory.setStrokeColor(currentToolColor());
    factory.updateFillingPropertyOnSelectedItems();
}

void UBToolController::alignSelection()
{
    UBAlignObjectManager mgr;
    mgr.horizontalAlign();
}

// --- Actions ---

void UBToolController::undo()
{
    if (UBApplication::undoStack)
        UBApplication::undoStack->undo();
}

void UBToolController::redo()
{
    if (UBApplication::undoStack)
        UBApplication::undoStack->redo();
}
