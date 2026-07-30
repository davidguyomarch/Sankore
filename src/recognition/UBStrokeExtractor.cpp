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

    for (QGraphicsItem* item : items)
    {
        // Case 1: A StrokesGroup (contains multiple polygon items forming one stroke)
        UBGraphicsStrokesGroup* group = dynamic_cast<UBGraphicsStrokesGroup*>(item);
        if (group)
        {
            UBRecognitionStroke recoStroke = extractFromStrokesGroup(group);
            if (!recoStroke.points.isEmpty())
                result.append(recoStroke);
            continue;
        }

        // Case 2: Individual polygon item
        UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(item);
        if (polygon && polygon->stroke())
        {
            UBGraphicsStroke* stroke = polygon->stroke();
            if (!processedStrokes.contains(stroke))
            {
                processedStrokes.insert(stroke);
                UBRecognitionStroke recoStroke = extractFromPolygons(stroke->polygons());
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
    // Extract the center line (midpoint of originalLine) for each segment.
    // The first point is the start of the first segment.
    bool first = true;
    for (UBGraphicsPolygonItem* polygon : polygons)
    {
        if (!polygon->isNominalLine())
            continue;

        QLineF line = polygon->originalLine();

        if (first)
        {
            // Map to scene coordinates
            QPointF startScene = polygon->mapToScene(line.p1());
            result.points.append(startScene);
            first = false;
        }

        QPointF endScene = polygon->mapToScene(line.p2());
        result.points.append(endScene);
    }

    return result;
}
