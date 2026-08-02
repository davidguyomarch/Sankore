/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBStrokeExtractor.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsStroke.h"

#include <QSet>
#include <QMap>

QVector<UBRecognitionStroke> UBStrokeExtractor::extractFromSelection(const QList<QGraphicsItem*>& items)
{
    QVector<UBRecognitionStroke> result;
    QSet<UBGraphicsStroke*> processedStrokes;
    QSet<QGraphicsItem*> processedGroups;

    // First pass: process StrokesGroups
    for (QGraphicsItem* item : items)
    {
        UBGraphicsStrokesGroup* group = dynamic_cast<UBGraphicsStrokesGroup*>(item);
        if (group)
        {
            processedGroups.insert(group);
            // Extract multiple logical strokes from the group (one per pen stroke)
            QVector<UBRecognitionStroke> groupStrokes = extractMultipleFromStrokesGroup(group);
            result.append(groupStrokes);
        }
    }

    // Second pass: individual polygons NOT inside an already-processed group
    for (QGraphicsItem* item : items)
    {
        if (dynamic_cast<UBGraphicsStrokesGroup*>(item))
            continue; // already processed above

        UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(item);
        if (polygon)
        {
            // Skip if this polygon's parent group was already processed
            if (polygon->parentItem() && processedGroups.contains(polygon->parentItem()))
                continue;

            if (polygon->stroke() && !processedStrokes.contains(polygon->stroke()))
            {
                processedStrokes.insert(polygon->stroke());
                UBRecognitionStroke recoStroke = extractFromPolygons(polygon->stroke()->polygons());
                if (!recoStroke.points.isEmpty())
                    result.append(recoStroke);
            }
        }
    }

    return result;
}

UBRecognitionStroke UBStrokeExtractor::extractFromStrokesGroup(UBGraphicsStrokesGroup* group)
{
    // NOTE: This function returns a single stroke for backward compatibility,
    // but for groups with multiple logical strokes, use extractMultipleFromStrokesGroup().
    UBRecognitionStroke result;

    if (!group)
        return result;

    // Collect all polygon items from the group's children
    QList<UBGraphicsPolygonItem*> polygons;
    for (QGraphicsItem* child : group->childItems())
    {
        UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);
        if (polygon)
            polygons.append(polygon);
    }

    return extractFromPolygons(polygons);
}

QVector<UBRecognitionStroke> UBStrokeExtractor::extractMultipleFromStrokesGroup(UBGraphicsStrokesGroup* group)
{
    QVector<UBRecognitionStroke> results;

    if (!group)
        return results;

    // Group polygons by their parent UBGraphicsStroke
    QMap<UBGraphicsStroke*, QList<UBGraphicsPolygonItem*>> strokeMap;
    QList<UBGraphicsPolygonItem*> orphanPolygons;

    for (QGraphicsItem* child : group->childItems())
    {
        UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);
        if (polygon)
        {
            UBGraphicsStroke* stroke = polygon->stroke();
            if (stroke)
                strokeMap[stroke].append(polygon);
            else
                orphanPolygons.append(polygon);
        }
    }

    // Create one UBRecognitionStroke per logical stroke
    for (auto it = strokeMap.begin(); it != strokeMap.end(); ++it)
    {
        UBRecognitionStroke recoStroke = extractFromPolygons(it.value());
        if (!recoStroke.points.isEmpty())
            results.append(recoStroke);
    }

    // Orphan polygons as a single stroke
    if (!orphanPolygons.isEmpty())
    {
        UBRecognitionStroke recoStroke = extractFromPolygons(orphanPolygons);
        if (!recoStroke.points.isEmpty())
            results.append(recoStroke);
    }

    return results;
}

UBRecognitionStroke UBStrokeExtractor::extractFromPolygons(const QList<UBGraphicsPolygonItem*>& polygons)
{
    UBRecognitionStroke result;

    if (polygons.isEmpty())
        return result;

    // Each polygon item represents a line segment of the stroke.
    // Extract the center line for each segment.
    bool first = true;
    for (UBGraphicsPolygonItem* polygon : polygons)
    {
        if (polygon->isNominalLine())
        {
            QLineF line = polygon->originalLine();
            if (first)
            {
                QPointF startScene = polygon->mapToScene(line.p1());
                result.points.append(startScene);
                first = false;
            }
            QPointF endScene = polygon->mapToScene(line.p2());
            result.points.append(endScene);
        }
        else
        {
            // Non-nominal polygon (modified by subtraction) — use center of bounding rect
            QPointF center = polygon->mapToScene(polygon->boundingRect().center());
            if (first)
            {
                result.points.append(center);
                first = false;
            }
            else
            {
                result.points.append(center);
            }
        }
    }

    return result;
}
