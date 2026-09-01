/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBDrawingHandler.h"

#include <QLineF>

#include "UBGraphicsScene.h"
#include "UBGraphicsPolygonItem.h"
#include "UBGraphicsStroke.h"
#include "UBSceneContext.h"
#include "core/UB.h"
#include "core/UBSettings.h"
#include "frameworks/UBCoreGraphicsScene.h"
#include "frameworks/UBGeometryUtils.h"
#include "controllers/UBToolController.h"

UBDrawingHandler::UBDrawingHandler()
{
}

UBDrawingHandler::~UBDrawingHandler()
{
    if (mCurrentStroke && mCurrentStroke->polygons().empty())
    {
        delete mCurrentStroke;
        mCurrentStroke = nullptr;
    }
}

void UBDrawingHandler::moveTo(const QPointF& point)
{
    mPreviousPoint = point;
    mPreviousWidth = -1.0;
    mPreviousPolygonItems.clear();
    mArcPolygonItem = nullptr;
    mDrawWithCompass = false;
    mSmoothBuffer.clear();
    mSmoothWidths.clear();
}

void UBDrawingHandler::drawLineTo(UBGraphicsScene* scene, const UBSceneContext& ctx,
                                  const QPointF& endPoint, qreal width, bool bLineStyle)
{
    if (mPreviousWidth == -1.0)
        mPreviousWidth = width;

    // Skip zero-length segments (can produce degenerate polygons that crash subtraction)
    if (mPreviousPoint == endPoint)
        return;

    bool isDark = scene->isDarkBackground();
    UBGraphicsPolygonItem* polygonItem = lineToPolygonItem(ctx, isDark, QLineF(mPreviousPoint, endPoint), width);

    if (!polygonItem->brush().isOpaque())
    {
        // Subtract overlapping polygons to keep transparency
        int startIdx = qMax(0, mPreviousPolygonItems.size() - 10);
        for (int i = startIdx; i < mPreviousPolygonItems.size(); i++)
        {
            UBGraphicsPolygonItem* previous = mPreviousPolygonItems.value(i);
            polygonItem->subtract(previous);
        }
    }

    if (bLineStyle)
    {
        QSetIterator<QGraphicsItem*> itItems(scene->addedItemsRef());

        while (itItems.hasNext())
        {
            QGraphicsItem* item = itItems.next();
            scene->removeItem(item);
        }
        scene->addedItemsRef().clear();
    }

    mpLastPolygon = polygonItem;
    scene->addedItemsRef().insert(polygonItem);

    scene->addItem(polygonItem);

    if (!mCurrentStroke)
        mCurrentStroke = new UBGraphicsStroke();

    polygonItem->setStroke(mCurrentStroke);

    mPreviousPolygonItems.append(polygonItem);

    if (!bLineStyle)
    {
        mPreviousPoint = endPoint;
        mPreviousWidth = width;
    }
}

void UBDrawingHandler::drawSmoothedSegment(UBGraphicsScene* scene, const UBSceneContext& ctx,
                                           const QPointF& p0, const QPointF& p1,
                                           const QPointF& p2, const QPointF& p3,
                                           qreal w1, qreal w2, bool bLineStyle)
{
    const int subdivisions = 4;

    for (int i = 1; i <= subdivisions; i++)
    {
        qreal t = (qreal)i / (qreal)subdivisions;
        qreal t2 = t * t;
        qreal t3 = t2 * t;

        qreal q0 = -t3 + 2.0*t2 - t;
        qreal q1 =  3.0*t3 - 5.0*t2 + 2.0;
        qreal q2 = -3.0*t3 + 4.0*t2 + t;
        qreal q3 =  t3 - t2;

        qreal x = 0.5 * (p0.x()*q0 + p1.x()*q1 + p2.x()*q2 + p3.x()*q3);
        qreal y = 0.5 * (p0.y()*q0 + p1.y()*q1 + p2.y()*q2 + p3.y()*q3);

        qreal w = w1 + (w2 - w1) * t;

        QPointF interpolatedPoint(x, y);
        drawLineTo(scene, ctx, interpolatedPoint, w, bLineStyle);
    }
}

void UBDrawingHandler::flushSmoothBuffer(UBGraphicsScene* scene, const UBSceneContext& ctx, bool bLineStyle)
{
    int n = mSmoothBuffer.size();
    if (n < 2)
    {
        mSmoothBuffer.clear();
        mSmoothWidths.clear();
        return;
    }

    if (n >= 4)
    {
        drawLineTo(scene, ctx, mSmoothBuffer.last(), mSmoothWidths.last(), bLineStyle);
    }
    else if (n == 3)
    {
        drawSmoothedSegment(scene, ctx,
                           mSmoothBuffer[0], mSmoothBuffer[0], mSmoothBuffer[1], mSmoothBuffer[2],
                           mSmoothWidths[0], mSmoothWidths[1], bLineStyle);
        drawLineTo(scene, ctx, mSmoothBuffer[2], mSmoothWidths[2], bLineStyle);
    }
    else
    {
        drawLineTo(scene, ctx, mSmoothBuffer.last(), mSmoothWidths.last(), bLineStyle);
    }

    mSmoothBuffer.clear();
    mSmoothWidths.clear();
}

void UBDrawingHandler::drawArcTo(UBGraphicsScene* scene, const UBSceneContext& ctx, UBSettings* settings,
                                 const QPointF& centerPoint, qreal spanAngle)
{
    mDrawWithCompass = true;
    if (mArcPolygonItem)
    {
        scene->addedItemsRef().remove(mArcPolygonItem);
        scene->removeItem(mArcPolygonItem);
        mArcPolygonItem = nullptr;
    }
    qreal penWidth = settings->currentPenWidth();
    penWidth /= ctx.systemScaleFactor();
    penWidth /= ctx.currentZoom();

    bool isDark = scene->isDarkBackground();
    mArcPolygonItem = arcToPolygonItem(ctx, isDark, QLineF(centerPoint, mPreviousPoint), spanAngle, penWidth);
    mArcPolygonItem->setStroke(mCurrentStroke);
    scene->addedItemsRef().insert(mArcPolygonItem);
    scene->addItem(mArcPolygonItem);

    scene->setDocumentUpdated();
}

UBGraphicsPolygonItem* UBDrawingHandler::lineToPolygonItem(const UBSceneContext& ctx, bool isDark,
                                                           const QLineF& line, qreal width)
{
    UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem(line, width);
    initPolygonItem(polygonItem, ctx, isDark);
    return polygonItem;
}

UBGraphicsPolygonItem* UBDrawingHandler::arcToPolygonItem(const UBSceneContext& ctx, bool isDark,
                                                          const QLineF& startRadius, qreal spanAngle, qreal width)
{
    QPolygonF polygon = UBGeometryUtils::arcToPolygon(startRadius, spanAngle, width);
    return polygonToPolygonItem(ctx, isDark, polygon);
}

UBGraphicsPolygonItem* UBDrawingHandler::polygonToPolygonItem(const UBSceneContext& ctx, bool isDark,
                                                              const QPolygonF& polygon)
{
    UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem(polygon);
    initPolygonItem(polygonItem, ctx, isDark);
    return polygonItem;
}

void UBDrawingHandler::initPolygonItem(UBGraphicsPolygonItem* polygonItem,
                                       const UBSceneContext& ctx, bool isDark)
{
    QColor colorOnDarkBG;
    QColor colorOnLightBG;

    if (!ctx.drawingController)
        return;

    if (ctx.drawingController->stylusTool() == UBStylusTool::Marker)
    {
        colorOnDarkBG = ctx.markerColorOnDarkBackground();
        colorOnLightBG = ctx.markerColorOnLightBackground();
    }
    else
    {
        colorOnDarkBG = ctx.penColorOnDarkBackground();
        colorOnLightBG = ctx.penColorOnLightBackground();
    }

    if (isDark)
        polygonItem->setColor(colorOnDarkBG);
    else
        polygonItem->setColor(colorOnLightBG);

    polygonItem->setColorOnDarkBackground(colorOnDarkBG);
    polygonItem->setColorOnLightBackground(colorOnLightBG);

    polygonItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));
}
