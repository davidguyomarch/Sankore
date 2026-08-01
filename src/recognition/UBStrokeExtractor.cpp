/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBStrokeExtractor.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsStroke.h"

#include <QSet>

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
            UBRecognitionStroke recoStroke = extractFromStrokesGroup(group);
            if (!recoStroke.points.isEmpty())
                result.append(recoStroke);
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
