/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBToolController.h"

#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBShapeFactory.h"

UBToolController::UBToolController(QObject* parent)
    : QObject(parent)
    , m_activeTool(Pen)
    , m_shapesVisible(false)
{
    // Sync from UBDrawingController when it changes (keyboard shortcuts, old UI, etc.)
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged,
            this, &UBToolController::onExternalToolChanged);
    connect(UBDrawingController::drawingController(), &UBDrawingController::colorIndexChanged,
            this, &UBToolController::onExternalColorChanged);
    connect(UBDrawingController::drawingController(), &UBDrawingController::lineWidthIndexChanged,
            this, &UBToolController::onExternalWidthChanged);

    // Init from current state — default to Pen
    int currentTool = UBDrawingController::drawingController()->stylusTool();
    m_activeTool = (currentTool >= 0) ? currentTool : Pen;
}

// --- Active Tool ---

int UBToolController::activeTool() const
{
    return m_activeTool;
}

void UBToolController::setActiveTool(int tool)
{
    if (m_activeTool == tool)
        return;

    m_activeTool = tool;

    // Push to the legacy system
    if (tool == Drawing)
    {
        // For Drawing tool, activate via ShapeFactory
        UBApplication::boardController->shapeFactory().createEllipse(true);
    }
    else
    {
        UBDrawingController::drawingController()->setStylusTool(tool);
    }

    emit activeToolChanged();
    emit currentColorsChanged();
    emit currentColorIndexChanged();
    emit currentWidthIndexChanged();
}

// --- Pen ---

QColor UBToolController::penColor() const
{
    return UBDrawingController::drawingController()->currentToolColor();
}

int UBToolController::penColorIndex() const
{
    return UBSettings::settings()->penColorIndex();
}

void UBToolController::setPenColorIndex(int index)
{
    UBDrawingController::drawingController()->setColorIndex(index);
    emit penColorChanged();
}

int UBToolController::penWidthIndex() const
{
    return UBSettings::settings()->penWidthIndex();
}

void UBToolController::setPenWidthIndex(int index)
{
    UBDrawingController::drawingController()->setLineWidthIndex(index);
    emit penWidthChanged();
}

QList<QColor> UBToolController::penColors() const
{
    UBSettings* s = UBSettings::settings();
    bool dark = UBApplication::boardController->activeScene()
                && UBApplication::boardController->activeScene()->isDarkBackground();
    return dark ? s->boardPenDarkBackgroundSelectedColors->colors()
                : s->boardPenLightBackgroundSelectedColors->colors();
}

// --- Marker ---

QColor UBToolController::markerColor() const
{
    return UBDrawingController::drawingController()->currentToolColor();
}

int UBToolController::markerColorIndex() const
{
    return UBSettings::settings()->markerColorIndex();
}

void UBToolController::setMarkerColorIndex(int index)
{
    UBDrawingController::drawingController()->setColorIndex(index);
    emit markerColorChanged();
}

int UBToolController::markerWidthIndex() const
{
    return UBSettings::settings()->markerWidthIndex();
}

void UBToolController::setMarkerWidthIndex(int index)
{
    UBDrawingController::drawingController()->setLineWidthIndex(index);
    emit markerWidthChanged();
}

// --- Marker Colors ---

QList<QColor> UBToolController::markerColors() const
{
    UBSettings* s = UBSettings::settings();
    bool dark = UBApplication::boardController->activeScene()
                && UBApplication::boardController->activeScene()->isDarkBackground();
    return dark ? s->boardMarkerDarkBackgroundSelectedColors->colors()
                : s->boardMarkerLightBackgroundSelectedColors->colors();
}

// --- Tool-aware convenience accessors for DrawingPropsBar QML ---

QList<QColor> UBToolController::currentColors() const
{
    if (m_activeTool == Marker)
        return markerColors();
    return penColors(); // Pen, Line, and other drawing tools use pen colors
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
    emit currentColorIndexChanged();
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
    return UBSettings::settings()->eraserWidthIndex();
}

void UBToolController::setEraserWidthIndex(int index)
{
    UBDrawingController::drawingController()->setEraserWidthIndex(index);
    emit eraserWidthChanged();
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

// --- External sync (from keyboard shortcuts or old widgets) ---

void UBToolController::onExternalToolChanged(int tool)
{
    if (m_activeTool != tool)
    {
        m_activeTool = tool;
        emit activeToolChanged();
        emit currentColorsChanged();
        emit currentColorIndexChanged();
        emit currentWidthIndexChanged();
    }
}

void UBToolController::onExternalColorChanged(int /*index*/)
{
    emit penColorChanged();
    emit markerColorChanged();
    emit currentColorIndexChanged();
}

void UBToolController::onExternalWidthChanged(int /*index*/)
{
    emit penWidthChanged();
    emit markerWidthChanged();
    emit currentWidthIndexChanged();
}
