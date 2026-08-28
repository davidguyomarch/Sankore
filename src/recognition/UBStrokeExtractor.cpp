/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBStrokeExtractor.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsStroke.h"
#include "domain/UBSmoothStrokeItem.h"

#include <QSet>
#include <QPair>
#include <QVector>
#include <algorithm>

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

    // Third pass: UBSmoothStrokeItem (new QPainterPath-based strokes)
    for (QGraphicsItem* item : items)
    {
        UBSmoothStrokeItem* smoothItem = dynamic_cast<UBSmoothStrokeItem*>(item);
        if (smoothItem)
        {
            QVector<QPointF> rawPoints = smoothItem->rawPoints();
            if (!rawPoints.isEmpty())
            {
                UBRecognitionStroke recoStroke;
                // rawPoints are in item-local coords — map to scene
                for (const QPointF& p : rawPoints)
                    recoStroke.points.append(smoothItem->mapToScene(p));
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

    // Group polygons by their parent UBGraphicsStroke, preserving temporal order.
    // Use a QVector of pairs to maintain the order in which strokes first appear.
    QVector<QPair<UBGraphicsStroke*, QList<UBGraphicsPolygonItem*>>> orderedStrokes;
    QSet<UBGraphicsStroke*> seenStrokes;
    QList<UBGraphicsPolygonItem*> orphanPolygons;

    for (QGraphicsItem* child : group->childItems())
    {
        UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);
        if (polygon)
        {
            UBGraphicsStroke* stroke = polygon->stroke();
            if (stroke)
            {
                if (!seenStrokes.contains(stroke))
                {
                    seenStrokes.insert(stroke);
                    orderedStrokes.append({stroke, QList<UBGraphicsPolygonItem*>()});
                }
                // Find the entry and append polygon
                for (auto& pair : orderedStrokes)
                {
                    if (pair.first == stroke)
                    {
                        pair.second.append(polygon);
                        break;
                    }
                }
            }
            else
            {
                orphanPolygons.append(polygon);
            }
        }
    }

    // Create one UBRecognitionStroke per logical stroke, in temporal order
    for (const auto& pair : orderedStrokes)
    {
        UBRecognitionStroke recoStroke = extractFromPolygons(pair.second);
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

    // Reverse results because childItems() returns in reverse Z-order
    // (last drawn first). We want chronological order (first drawn first).
    std::reverse(results.begin(), results.end());

    return results;
}

UBRecognitionStroke UBStrokeExtractor::extractFromPolygons(const QList<UBGraphicsPolygonItem*>& polygons)
{
    UBRecognitionStroke result;

    if (polygons.isEmpty())
        return result;

    // Each polygon item represents a line segment of the stroke.
    // originalLine() contains the raw scene coordinates that were used to create the polygon.
    // We do NOT use mapToScene because originalLine values are already in scene space
    // (they were passed directly from mPreviousPoint/pEndPoint in drawLineTo).
    // Using mapToScene would double-transform them when the polygon is inside a group.
    bool first = true;
    for (UBGraphicsPolygonItem* polygon : polygons)
    {
        if (polygon->isNominalLine())
        {
            QLineF line = polygon->originalLine();
            if (first)
            {
                result.points.append(line.p1());
                first = false;
            }
            result.points.append(line.p2());
        }
        else
        {
            // Non-nominal polygon (eraser artifact) — use boundingRect center mapped to scene
            QPointF center = polygon->mapToScene(polygon->boundingRect().center());
            result.points.append(center);
            first = false;
        }
    }

    return result;
}
