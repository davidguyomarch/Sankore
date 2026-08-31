/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBDRAWINGHANDLER_H
#define UBDRAWINGHANDLER_H

#include <QList>
#include <QLineF>
#include <QPointF>
#include <QPolygonF>
#include <QVector>

class QGraphicsItem;
class UBGraphicsPolygonItem;
class UBGraphicsScene;
class UBGraphicsStroke;
class UBSceneContext;
class UBSettings;

/**
 * @brief Drawing pipeline for pen, marker, line, and compass/arc tools.
 *
 * Phase 5 of the UBGraphicsScene decomposition (#111).
 *
 * Owns all drawing state: previous point/width, polygon history,
 * current stroke, arc polygon, smooth buffer (Catmull-Rom).
 * Methods operate on the scene via a UBGraphicsScene* for addItem/removeItem.
 */
class UBDrawingHandler
{
public:
    UBDrawingHandler();
    ~UBDrawingHandler();

    // --- Drawing API (called by scene facades and input routing) ---

    void moveTo(const QPointF& point);

    void drawLineTo(UBGraphicsScene* scene, const UBSceneContext& ctx,
                    const QPointF& endPoint, qreal width, bool bLineStyle);

    void drawSmoothedSegment(UBGraphicsScene* scene, const UBSceneContext& ctx,
                             const QPointF& p0, const QPointF& p1,
                             const QPointF& p2, const QPointF& p3,
                             qreal w1, qreal w2, bool bLineStyle);

    void flushSmoothBuffer(UBGraphicsScene* scene, const UBSceneContext& ctx, bool bLineStyle);

    void drawArcTo(UBGraphicsScene* scene, const UBSceneContext& ctx, UBSettings* settings,
                   const QPointF& centerPoint, qreal spanAngle);

    // --- Polygon factory (used by drawing and by CFF adaptor) ---

    UBGraphicsPolygonItem* lineToPolygonItem(const UBSceneContext& ctx, bool isDark,
                                             const QLineF& line, qreal width);

    UBGraphicsPolygonItem* arcToPolygonItem(const UBSceneContext& ctx, bool isDark,
                                            const QLineF& startRadius, qreal spanAngle, qreal width);

    UBGraphicsPolygonItem* polygonToPolygonItem(const UBSceneContext& ctx, bool isDark,
                                                const QPolygonF& polygon);

    // --- State accessors (used by input routing) ---

    QPointF& previousPoint() { return mPreviousPoint; }
    const QPointF& previousPoint() const { return mPreviousPoint; }

    UBGraphicsStroke* currentStroke() const { return mCurrentStroke; }
    void setCurrentStroke(UBGraphicsStroke* s) { mCurrentStroke = s; }

    UBGraphicsPolygonItem* lastPolygon() const { return mpLastPolygon; }
    void setLastPolygon(UBGraphicsPolygonItem* p) { mpLastPolygon = p; }

    bool drawWithCompass() const { return mDrawWithCompass; }
    void setDrawWithCompass(bool v) { mDrawWithCompass = v; }

    UBGraphicsPolygonItem* arcPolygonItem() const { return mArcPolygonItem; }

    QVector<QPointF>& smoothBuffer() { return mSmoothBuffer; }
    QVector<qreal>& smoothWidths() { return mSmoothWidths; }

    QList<UBGraphicsPolygonItem*>& previousPolygonItems() { return mPreviousPolygonItems; }

private:
    void initPolygonItem(UBGraphicsPolygonItem* item, const UBSceneContext& ctx, bool isDark);

    QPointF mPreviousPoint;
    qreal mPreviousWidth = -1.0;
    QList<UBGraphicsPolygonItem*> mPreviousPolygonItems;

    UBGraphicsStroke* mCurrentStroke = nullptr;
    UBGraphicsPolygonItem* mArcPolygonItem = nullptr;
    UBGraphicsPolygonItem* mpLastPolygon = nullptr;
    bool mDrawWithCompass = false;

    QVector<QPointF> mSmoothBuffer;
    QVector<qreal> mSmoothWidths;
};

#endif // UBDRAWINGHANDLER_H
