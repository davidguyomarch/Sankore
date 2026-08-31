/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBEraserHandler.h"

#include <QGraphicsScene>
#include <QPainterPath>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

#include "UBGraphicsScene.h"
#include "UBGraphicsPolygonItem.h"
#include "UBSmoothStrokeItem.h"
#include "frameworks/UBGeometryUtils.h"

void UBEraserHandler::eraseLineTo(UBGraphicsScene* scene,
                                  QPointF& previousPoint,
                                  const QPointF& endPoint,
                                  qreal width,
                                  QSet<QGraphicsItem*>& addedItems,
                                  QSet<QGraphicsItem*>& removedItems)
{
    const QLineF line(previousPoint, endPoint);
    previousPoint = endPoint;

    const QPolygonF eraserPolygon = UBGeometryUtils::lineToPolygon(line, width);
    const QRectF eraserBoundingRect = eraserPolygon.boundingRect();

    QPainterPath eraserPath;
    eraserPath.addPolygon(eraserPolygon);

    // Get all the items that are intersecting with the eraser path
    QList<QGraphicsItem*> collidItems = scene->items(eraserBoundingRect, Qt::IntersectsItemBoundingRect);

    // --- Split legacy polygon items at the eraser boundary ---
    QList<UBGraphicsPolygonItem*> intersectedItems;

    typedef QList<QPolygonF> POLYGONSLIST;
    QList<POLYGONSLIST> intersectedPolygons;

    #pragma omp parallel for
    for (int i = 0; i < collidItems.size(); i++)
    {
        UBGraphicsPolygonItem* pi = qgraphicsitem_cast<UBGraphicsPolygonItem*>(collidItems[i]);
        if (pi == nullptr)
            continue;

        QPainterPath itemPainterPath;
        itemPainterPath.addPolygon(pi->sceneTransform().map(pi->polygon()));

        if (eraserPath.contains(itemPainterPath))
        {
            #pragma omp critical
            {
                intersectedItems << pi;
                intersectedPolygons << QList<QPolygonF>();
            }
        }
        else if (eraserPath.intersects(itemPainterPath))
        {
            QPainterPath newPath = itemPainterPath.subtracted(eraserPath);
            #pragma omp critical
            {
                intersectedItems << pi;
                intersectedPolygons << newPath.simplified().toFillPolygons(pi->sceneTransform().inverted());
            }
        }
    }

    for (int i = 0; i < intersectedItems.size(); i++)
    {
        UBGraphicsPolygonItem* intersectedPolygonItem = intersectedItems[i];

        if (!intersectedPolygons[i].empty())
        {
            for (int j = 0; j < intersectedPolygons[i].size(); j++)
            {
                UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem(intersectedPolygons[i][j], nullptr);

                intersectedPolygonItem->copyItemParameters(polygonItem);
                polygonItem->setStroke(intersectedPolygonItem->stroke());
                polygonItem->setStrokesGroup(intersectedPolygonItem->strokesGroup());
                intersectedPolygonItem->strokesGroup()->addToGroup(polygonItem);
                addedItems << polygonItem;
            }
        }

        removedItems << intersectedPolygonItem;

        QTransform t;
        bool bApplyTransform = false;
        if (intersectedPolygonItem->strokesGroup())
        {
            if (intersectedPolygonItem->strokesGroup()->parentItem())
            {
                bApplyTransform = true;
                t = intersectedPolygonItem->sceneTransform();
            }
            intersectedPolygonItem->strokesGroup()->removeFromGroup(intersectedPolygonItem);
        }

        scene->removeItem(intersectedPolygonItem);
        if (bApplyTransform)
            intersectedPolygonItem->setTransform(t);
    }

    if (!intersectedItems.empty())
        scene->setModified(true);

    // --- Split smooth stroke items at the eraser boundary ---
    for (int i = 0; i < collidItems.size(); i++)
    {
        UBSmoothStrokeItem* smoothItem = dynamic_cast<UBSmoothStrokeItem*>(collidItems[i]);
        if (!smoothItem)
            continue;

        QList<QGraphicsItem*> newFragments;
        if (smoothItem->subtractPath(eraserPath, newFragments))
        {
            // Path became empty — remove item entirely
            removedItems << smoothItem;
            scene->removeItem(smoothItem);
        }
        else if (!newFragments.isEmpty())
        {
            // Item was split — track fragments for undo
            for (QGraphicsItem* fragment : newFragments)
                addedItems << fragment;
            scene->setModified(true);
        }
    }
}
