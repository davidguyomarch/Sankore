/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBSCENECONTEXT_H
#define UBSCENECONTEXT_H

#include <QColor>

class UBDrawingController;
class UBBoardController;

/**
 * @brief Runtime context for UBGraphicsScene — replaces direct singleton access.
 *
 * In production, holds a pointer to the board controller for live value access.
 * In tests, uses static default values (no singletons needed).
 */
struct UBSceneContext
{
    // --- Live board controller pointer (null in tests) ---
    class UBBoardController* boardController = nullptr;

    // --- Drawing controller (nullable for tests) ---
    UBDrawingController* drawingController = nullptr;

    // --- Static values (used when boardController is null, i.e. in tests) ---
    qreal testSystemScaleFactor = 1.0;
    qreal testCurrentZoom = 1.0;
    qreal pointerDiameter = 40.0;
    qreal eraserFineWidth = 8.0;
    qreal eraserMediumWidth = 24.0;
    qreal eraserStrongWidth = 48.0;
    QColor penColorOnDarkBackground = QColor(Qt::white);
    QColor penColorOnLightBackground = QColor(Qt::blue);
    QColor markerColorOnDarkBackground = QColor(Qt::yellow);
    QColor markerColorOnLightBackground = QColor(Qt::green);

    // --- Live accessors (delegates to boardController or uses test values) ---
    qreal systemScaleFactor() const;
    qreal currentZoom() const;

    // --- Helper: get current eraser width based on current settings ---
    qreal currentEraserWidth() const;

    // --- Helper: get current tool width adjusted for zoom ---
    qreal adjustedWidth(qreal rawWidth) const
    {
        return rawWidth / systemScaleFactor() / currentZoom();
    }
};

#endif // UBSCENECONTEXT_H
