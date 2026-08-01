/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBSCENECONTEXT_H
#define UBSCENECONTEXT_H

#include <QColor>

class UBDrawingController;

/**
 * @brief Runtime context for UBGraphicsScene — replaces direct singleton access.
 *
 * In production, filled from UBApplication::boardController + UBSettings.
 * In tests, constructed with default values (no singletons needed).
 *
 * Values that change dynamically (zoom, tool) are accessed via pointers to
 * live values or via a lightweight provider interface.
 */
struct UBSceneContext
{
    // --- Scale factors (updated on zoom change) ---
    qreal systemScaleFactor = 1.0;
    qreal currentZoom = 1.0;

    // --- Pointer/eraser sizes (from UBSettings) ---
    qreal pointerDiameter = 40.0;
    qreal eraserFineWidth = 8.0;
    qreal eraserMediumWidth = 24.0;
    qreal eraserStrongWidth = 48.0;

    // --- Pen colors ---
    QColor penColorOnDarkBackground = QColor(Qt::white);
    QColor penColorOnLightBackground = QColor(Qt::blue);
    QColor markerColorOnDarkBackground = QColor(Qt::yellow);
    QColor markerColorOnLightBackground = QColor(Qt::green);

    // --- Drawing controller (nullable for tests) ---
    UBDrawingController* drawingController = nullptr;

    // --- Helper: get effective eraser width based on current settings ---
    qreal currentEraserWidth() const;

    // --- Helper: get current tool width adjusted for zoom ---
    qreal adjustedWidth(qreal rawWidth) const
    {
        return rawWidth / systemScaleFactor / currentZoom;
    }
};

#endif // UBSCENECONTEXT_H
