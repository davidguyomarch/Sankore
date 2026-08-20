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

    // Skip duplicate points
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
    return qobject_cast<UBGraphicsScene*>(QGraphicsPathItem::scene());
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

    QPainterPath currentPath = path();
    QPainterPath newPath = currentPath.subtracted(localEraserPath);

    if (newPath.isEmpty())
    {
        return true; // caller should remove this item
    }

    setPath(newPath);
    return false;
}

void UBSmoothStrokeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen strokePen = pen();
    strokePen.setCapStyle(Qt::RoundCap);
    strokePen.setJoinStyle(Qt::RoundJoin);

    // Soft-edge pass: draw a slightly wider, semi-transparent version first
    // This creates a smooth feathered edge similar to SMART Notebook rendering
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
