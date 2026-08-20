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

#ifndef UBSMOOTHSTROKEITEM_H
#define UBSMOOTHSTROKEITEM_H

#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QColor>
#include <QVector>
#include <QPointF>
#include <QUuid>

#include "UBItem.h"
#include "core/UB.h"

/**
 * @brief A single QPainterPath-based stroke item that replaces the legacy
 * architecture of N polygon items grouped into a UBGraphicsStrokesGroup.
 *
 * Points are accumulated during drawing via addPoint(), and the path is
 * reconstructed as smooth Bézier curves (Catmull-Rom to cubic conversion).
 * The result is a single, smooth, antialiased stroke rendered with RoundCap
 * and RoundJoin — no jointure artifacts.
 */
class UBSmoothStrokeItem : public QGraphicsPathItem, public UBItem, public UBGraphicsItem
{
public:
    UBSmoothStrokeItem(QGraphicsItem* parent = nullptr);
    virtual ~UBSmoothStrokeItem();

    // --- Drawing API (called during input) ---

    /** Append a point (in scene coordinates) with associated pressure [0..1]. */
    void addPoint(const QPointF& scenePos, qreal pressure = 1.0);

    /** Called at mouse/stylus release. Marks the stroke as complete. */
    void finalize();

    bool isFinalized() const { return mFinalized; }

    // --- Visual properties ---

    void setStrokeColor(const QColor& color);
    void setStrokeWidth(qreal width);

    void setColorOnDarkBackground(const QColor& c) { mColorOnDark = c; }
    void setColorOnLightBackground(const QColor& c) { mColorOnLight = c; }

    QColor colorOnDarkBackground() const { return mColorOnDark; }
    QColor colorOnLightBackground() const { return mColorOnLight; }

    qreal nominalWidth() const { return mNominalWidth; }

    // --- UBItem interface ---

    enum { Type = UBGraphicsItemType::SmoothStrokeItemType };

    int type() const override { return Type; }

    UBItem* deepCopy() const override;
    void copyItemParameters(UBItem* copy) const override;

    void setUuid(const QUuid& pUuid) override;

    UBGraphicsScene* scene() override;

    // --- Raw data access (for OCR, serialization) ---

    QVector<QPointF> rawPoints() const { return mRawPoints; }
    QVector<qreal> rawPressures() const { return mRawPressures; }

    // --- Eraser support ---

    /**
     * Subtract an eraser path from this stroke.
     * Returns true if the item should be removed (path became empty).
     */
    bool subtractPath(const QPainterPath& eraserPath);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    /** Rebuild the QPainterPath from raw points using Catmull-Rom → cubic Bézier. */
    void rebuildPath();

    QVector<QPointF> mRawPoints;
    QVector<qreal> mRawPressures;

    qreal mNominalWidth = 2.0;

    QColor mColorOnDark;
    QColor mColorOnLight;

    bool mFinalized = false;
};

#endif // UBSMOOTHSTROKEITEM_H
