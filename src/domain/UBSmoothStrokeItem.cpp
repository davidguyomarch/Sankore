/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBSmoothStrokeItem.h"
#include "UBGraphicsScene.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

UBSmoothStrokeItem::UBSmoothStrokeItem(QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
{
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));
    setUuid(QUuid::createUuid());
}

UBSmoothStrokeItem::~UBSmoothStrokeItem()
{
    // NOOP
}

void UBSmoothStrokeItem::addPoint(const QPointF& scenePos, qreal pressure)
{
    // Convert scene coordinates to local item coordinates
    QPointF localPos = mapFromScene(scenePos);

    // --- Stabilizer: exponential moving average filter ---
    // Reduces micro-trembling from TNI sensors without adding perceptible latency
    const qreal alpha = 0.5; // smoothing factor (0 = no smoothing, 1 = no filter)
    if (!mRawPoints.isEmpty())
    {
        QPointF prev = mRawPoints.last();
        localPos = alpha * localPos + (1.0 - alpha) * prev;
        pressure = alpha * pressure + (1.0 - alpha) * mRawPressures.last();
    }

    // --- Minimum distance filter ---
    // Only add a point if it's far enough from the previous one.
    // Prevents point clusters in slow writing and ensures consistent density.
    const qreal minDistance = 3.0; // pixels
    if (!mRawPoints.isEmpty())
    {
        QPointF delta = localPos - mRawPoints.last();
        qreal dist = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
        if (dist < minDistance)
            return;
    }

    // Skip exact duplicate points (safety check)
    if (!mRawPoints.isEmpty() && mRawPoints.last() == localPos)
        return;

    mRawPoints.append(localPos);
    mRawPressures.append(qBound(0.0, pressure, 1.0));

    rebuildPath();
}

void UBSmoothStrokeItem::finalize()
{
    mFinalized = true;
    // Final rebuild to ensure the complete path is clean
    rebuildPath();
}

void UBSmoothStrokeItem::setLastPoint(const QPointF& scenePos, qreal pressure)
{
    QPointF localPos = mapFromScene(scenePos);

    if (mRawPoints.size() < 2)
    {
        // Not enough points yet — just add
        addPoint(scenePos, pressure);
        return;
    }

    // Replace the last point
    mRawPoints.last() = localPos;
    mRawPressures.last() = qBound(0.0, pressure, 1.0);

    rebuildPath();
}

void UBSmoothStrokeItem::setStrokeColor(const QColor& color)
{
    QPen p = pen();
    p.setColor(color);
    setPen(p);
}

void UBSmoothStrokeItem::setStrokeWidth(qreal width)
{
    mNominalWidth = width;
    QPen p = pen();
    p.setWidthF(width);
    setPen(p);
}

void UBSmoothStrokeItem::setUuid(const QUuid& pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant::fromValue(pUuid));
}

UBGraphicsScene* UBSmoothStrokeItem::scene()
{
    return dynamic_cast<UBGraphicsScene*>(QGraphicsPathItem::scene());
}

UBItem* UBSmoothStrokeItem::deepCopy() const
{
    UBSmoothStrokeItem* copy = new UBSmoothStrokeItem();
    copyItemParameters(copy);
    return copy;
}

void UBSmoothStrokeItem::copyItemParameters(UBItem* copy) const
{
    UBSmoothStrokeItem* cp = dynamic_cast<UBSmoothStrokeItem*>(copy);
    if (!cp)
        return;

    cp->mRawPoints = mRawPoints;
    cp->mRawPressures = mRawPressures;
    cp->mNominalWidth = mNominalWidth;
    cp->mColorOnDark = mColorOnDark;
    cp->mColorOnLight = mColorOnLight;
    cp->mFinalized = mFinalized;

    cp->setPen(pen());
    cp->setBrush(brush());
    cp->setPath(path());
    cp->setPos(pos());
    cp->setTransform(transform());
    cp->setFlag(QGraphicsItem::ItemIsMovable, true);
    cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
    cp->setData(UBGraphicsItemData::ItemLayerType, data(UBGraphicsItemData::ItemLayerType));
    cp->setData(UBGraphicsItemData::ItemOwnZValue, data(UBGraphicsItemData::ItemOwnZValue));
    cp->setZValue(zValue());
    cp->setUuid(QUuid::createUuid());
}

bool UBSmoothStrokeItem::subtractPath(const QPainterPath& eraserPath)
{
    // Convert eraser path to local coordinates
    QPainterPath localEraserPath = mapFromScene(eraserPath);

    // Find which raw points are inside or near the eraser zone
    // A point is "erased" if the eraser path contains it (with stroke width tolerance)
    qreal tolerance = mNominalWidth / 2.0 + 1.0;

    QVector<bool> erased(mRawPoints.size(), false);
    bool anyErased = false;

    for (int i = 0; i < mRawPoints.size(); ++i)
    {
        // Check if this point or a small circle around it intersects the eraser
        QPainterPath pointCircle;
        pointCircle.addEllipse(mRawPoints[i], tolerance, tolerance);
        if (localEraserPath.intersects(pointCircle))
        {
            erased[i] = true;
            anyErased = true;
        }
    }

    if (!anyErased)
        return false; // eraser missed all points

    // Split into surviving segments (contiguous runs of non-erased points)
    QVector<QVector<QPointF>> segments;
    QVector<QVector<qreal>> segmentPressures;
    QVector<QPointF> currentSeg;
    QVector<qreal> currentPressures;

    for (int i = 0; i < mRawPoints.size(); ++i)
    {
        if (!erased[i])
        {
            currentSeg.append(mRawPoints[i]);
            currentPressures.append(mRawPressures[i]);
        }
        else
        {
            if (currentSeg.size() >= 2)
            {
                segments.append(currentSeg);
                segmentPressures.append(currentPressures);
            }
            currentSeg.clear();
            currentPressures.clear();
        }
    }
    if (currentSeg.size() >= 2)
    {
        segments.append(currentSeg);
        segmentPressures.append(currentPressures);
    }

    // If no segments survive, remove the item entirely
    if (segments.isEmpty())
        return true;

    // Keep the first segment in this item
    mRawPoints = segments[0];
    mRawPressures = segmentPressures[0];
    mFinalized = true;
    rebuildPath();

    // Create new items for additional segments
    UBGraphicsScene* ubScene = scene();
    for (int s = 1; s < segments.size(); ++s)
    {
        UBSmoothStrokeItem* newItem = new UBSmoothStrokeItem();
        newItem->setStrokeWidth(mNominalWidth);
        newItem->setStrokeColor(pen().color());
        newItem->setColorOnDarkBackground(mColorOnDark);
        newItem->setColorOnLightBackground(mColorOnLight);
        newItem->setZValue(zValue());
        newItem->setData(UBGraphicsItemData::ItemLayerType, data(UBGraphicsItemData::ItemLayerType));

        // Set raw points directly (they're already in local coords of the original item,
        // but the new item starts at origin so we need to map to scene then back)
        for (int i = 0; i < segments[s].size(); ++i)
        {
            // mapToScene converts local→scene, then new item will convert scene→local in addPoint
            QPointF scenePos = mapToScene(segments[s][i]);
            newItem->addPoint(scenePos, segmentPressures[s][i]);
        }
        newItem->finalize();

        if (ubScene)
            ubScene->addItem(newItem);
    }

    return false; // item was modified, not removed
}

void UBSmoothStrokeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen strokePen = pen();
    strokePen.setCapStyle(Qt::RoundCap);
    strokePen.setJoinStyle(Qt::RoundJoin);

    // --- Velocity-based width modulation ---
    // Compute per-point widths based on velocity between consecutive points.
    // Slow movement → thick (natural pen behavior at start of stroke)
    // Fast movement → thin (liaisons, quick strokes)
    if (mRawPoints.size() >= 3 && mNominalWidth >= 1.5)
    {
        QVector<qreal> widths(mRawPoints.size(), mNominalWidth);

        // Compute velocities (distance between consecutive points)
        QVector<qreal> velocities(mRawPoints.size(), 0.0);
        qreal maxVelocity = 1.0;
        for (int i = 1; i < mRawPoints.size(); ++i)
        {
            QPointF delta = mRawPoints[i] - mRawPoints[i - 1];
            velocities[i] = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
            if (velocities[i] > maxVelocity)
                maxVelocity = velocities[i];
        }
        velocities[0] = velocities.size() > 1 ? velocities[1] : 0.0;

        // Normalize and compute widths
        // width = nominal × (0.6 + 0.4 × (1 - velocityNorm))
        // Also factor in pressure if available
        for (int i = 0; i < mRawPoints.size(); ++i)
        {
            qreal vNorm = velocities[i] / maxVelocity; // 0..1
            qreal velocityFactor = 0.6 + 0.4 * (1.0 - vNorm);

            // Pressure modulation (if stylus provides it)
            qreal pressureFactor = 0.5 + 0.5 * mRawPressures[i]; // 0.5..1.0

            widths[i] = mNominalWidth * velocityFactor * pressureFactor;
            widths[i] = qMax(widths[i], 0.5); // minimum width
        }

        // Smooth the widths to avoid abrupt changes
        QVector<qreal> smoothWidths = widths;
        for (int pass = 0; pass < 2; ++pass)
        {
            for (int i = 1; i < smoothWidths.size() - 1; ++i)
                smoothWidths[i] = 0.25 * widths[i - 1] + 0.5 * widths[i] + 0.25 * widths[i + 1];
        }

        // Draw as a filled outline (variable width) using QPainterPathStroker per segment
        // Simplified approach: draw each segment with its own pen width
        QColor strokeColor = strokePen.color();

        // Soft-edge pass
        QColor softColor = strokeColor;
        softColor.setAlphaF(softColor.alphaF() * 0.25);
        for (int i = 0; i < mRawPoints.size() - 1; ++i)
        {
            QPen segPen(softColor, smoothWidths[i] + 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(segPen);
            painter->drawLine(mRawPoints[i], mRawPoints[i + 1]);
        }

        // Main pass
        for (int i = 0; i < mRawPoints.size() - 1; ++i)
        {
            qreal w = (smoothWidths[i] + smoothWidths[i + 1]) / 2.0;
            QPen segPen(strokeColor, w, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(segPen);
            painter->drawLine(mRawPoints[i], mRawPoints[i + 1]);
        }

        return; // skip fixed-width rendering below
    }

    // Fallback: fixed-width rendering for short strokes or thin lines

    // Soft-edge pass: draw a slightly wider, semi-transparent version first
    if (strokePen.widthF() >= 1.5)
    {
        QPen softPen = strokePen;
        QColor softColor = strokePen.color();
        softColor.setAlphaF(softColor.alphaF() * 0.3);
        softPen.setColor(softColor);
        softPen.setWidthF(strokePen.widthF() + 2.0);
        painter->setPen(softPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path());
    }

    // Main stroke pass
    painter->setPen(strokePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path());
}

QRectF UBSmoothStrokeItem::boundingRect() const
{
    // Expand by 1px margin for the soft-edge pass (pen + 2.0 wider → +1.0 each side)
    return QGraphicsPathItem::boundingRect().adjusted(-1.0, -1.0, 1.0, 1.0);
}

void UBSmoothStrokeItem::rebuildPath()
{
    const int n = mRawPoints.size();

    if (n < 2)
    {
        // Single point: draw a tiny dot (circle) so it's visible
        if (n == 1)
        {
            QPainterPath dotPath;
            qreal r = mNominalWidth / 2.0;
            dotPath.addEllipse(mRawPoints[0], r, r);
            setPath(dotPath);
        }
        return;
    }

    QPainterPath newPath;
    newPath.moveTo(mRawPoints[0]);

    if (n == 2)
    {
        newPath.lineTo(mRawPoints[1]);
    }
    else
    {
        // Catmull-Rom spline → Cubic Bézier conversion
        // For each segment [i, i+1], we compute Bézier control points from
        // the surrounding Catmull-Rom control points (i-1, i, i+1, i+2).
        for (int i = 0; i < n - 1; i++)
        {
            QPointF p0 = (i > 0) ? mRawPoints[i - 1] : mRawPoints[i];
            QPointF p1 = mRawPoints[i];
            QPointF p2 = mRawPoints[i + 1];
            QPointF p3 = (i + 2 < n) ? mRawPoints[i + 2] : mRawPoints[i + 1];

            // Catmull-Rom to cubic Bézier control points
            QPointF cp1 = p1 + (p2 - p0) / 6.0;
            QPointF cp2 = p2 - (p3 - p1) / 6.0;

            newPath.cubicTo(cp1, cp2, p2);
        }
    }

    setPath(newPath);
}
