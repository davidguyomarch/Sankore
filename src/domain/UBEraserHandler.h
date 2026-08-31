/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBERASERHANDLER_H
#define UBERASERHANDLER_H

#include <QPointF>
#include <QSet>

class QGraphicsItem;
class UBGraphicsScene;

/**
 * @brief Eraser logic extracted from UBGraphicsScene.
 *
 * Phase 4 of the UBGraphicsScene decomposition (#111).
 *
 * Handles polygon splitting (legacy UBGraphicsPolygonItem) and
 * smooth stroke subtraction (UBSmoothStrokeItem) when the eraser
 * tool moves across drawn content.  Uses OpenMP parallelism for
 * the polygon intersection pass.
 *
 * The handler is stateless — it does NOT own mPreviousPoint or the
 * added/removed item sets.  Those are passed by reference so the
 * scene (and later UBInputRouter) keeps ownership.
 */
class UBEraserHandler
{
public:
    /**
     * Erase along the line from @p previousPoint to @p endPoint
     * with the given @p width.
     *
     * @param scene          The graphics scene (for items() and removeItem()).
     * @param previousPoint  Start of the erase segment — updated to @p endPoint on return.
     * @param endPoint       End of the erase segment.
     * @param width          Eraser width in scene coordinates.
     * @param addedItems     [out] Items created by polygon splitting (fragments).
     * @param removedItems   [out] Items fully or partially erased.
     */
    static void eraseLineTo(UBGraphicsScene* scene,
                            QPointF& previousPoint,
                            const QPointF& endPoint,
                            qreal width,
                            QSet<QGraphicsItem*>& addedItems,
                            QSet<QGraphicsItem*>& removedItems);
};

#endif // UBERASERHANDLER_H
