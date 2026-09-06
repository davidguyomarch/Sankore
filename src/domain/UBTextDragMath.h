/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBTEXTDRAGMATH_H
#define UBTEXTDRAGMATH_H

#include <QPointF>
#include <cmath>

/**
 * Pure decision logic for #279: a text box must be movable by dragging from
 * anywhere inside it (not just over the glyphs), while a plain click still
 * edits. The discriminator is the drag distance: once the pointer moves past
 * the platform drag threshold from the press point, the gesture is a box move
 * (setPos) rather than a text selection.
 *
 * This isolates the testable part; the actual mouse wiring lives in
 * UBGraphicsTextItem and is validated at runtime.
 */
namespace UBTextDrag
{
    /// Manhattan distance, matching Qt's QApplication::startDragDistance() test.
    inline int manhattan(const QPointF& a, const QPointF& b)
    {
        return static_cast<int>(std::abs(a.x() - b.x()) + std::abs(a.y() - b.y()));
    }

    /// True when a press-to-current move should be treated as dragging the whole
    /// box (rather than editing/selecting text): the pointer has travelled at
    /// least `dragThreshold` pixels from the press point. `alreadyDragging`
    /// latches the gesture so a drag keeps moving even if it briefly dips back
    /// under the threshold.
    inline bool shouldDragBox(const QPointF& pressScenePos,
                              const QPointF& currentScenePos,
                              int dragThreshold,
                              bool alreadyDragging)
    {
        if (alreadyDragging)
            return true;
        return manhattan(pressScenePos, currentScenePos) >= dragThreshold;
    }
}

#endif // UBTEXTDRAGMATH_H
