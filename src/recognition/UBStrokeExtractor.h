/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBSTROKEEXTRACTOR_H
#define UBSTROKEEXTRACTOR_H

#include "IHandwritingRecognizer.h"
#include <QGraphicsItem>
#include <QList>

class UBGraphicsPolygonItem;
class UBGraphicsStrokesGroup;

/**
 * @brief Extracts recognition strokes from selected scene items.
 *
 * Handles both individual UBGraphicsPolygonItem and UBGraphicsStrokesGroup.
 * Extracts the center line of each polygon segment to reconstruct
 * the original stylus path.
 */
namespace UBStrokeExtractor
{
    /**
     * @brief Extract recognition strokes from a list of selected graphics items.
     * @param items Selected QGraphicsItems (may include StrokesGroups, PolygonItems, etc.)
     * @return Vector of UBRecognitionStroke ready for the recognizer
     */
    QVector<UBRecognitionStroke> extractFromSelection(const QList<QGraphicsItem*>& items);

    /**
     * @brief Extract a single recognition stroke from a strokes group.
     */
    UBRecognitionStroke extractFromStrokesGroup(UBGraphicsStrokesGroup* group);

    /**
     * @brief Extract multiple recognition strokes from a strokes group.
     * Each logical pen stroke within the group becomes a separate UBRecognitionStroke.
     * This is the preferred method for recognition (Windows Ink needs separate strokes).
     */
    QVector<UBRecognitionStroke> extractMultipleFromStrokesGroup(UBGraphicsStrokesGroup* group);

    /**
     * @brief Extract center points from a list of polygon items that form a stroke.
     */
    UBRecognitionStroke extractFromPolygons(const QList<UBGraphicsPolygonItem*>& polygons);
}

#endif // UBSTROKEEXTRACTOR_H
