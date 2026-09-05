/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBRESIZEGRIPGEOMETRY_H
#define UBRESIZEGRIPGEOMETRY_H

#include <QRectF>

/**
 * Pure geometry of the selection-frame resize grips (#277).
 *
 * The clickable resize zones used by UBGraphicsDelegateFrame::toolFromPos are
 * computed from the frame rectangle and the frame width. Previously each edge
 * grip was only `frameWidth` thick and, worse, was cut back by `frameWidth` at
 * each end (`left + frameWidth`, `width - 2*frameWidth`), so the extremities of
 * an edge did not resize and the thin band was hard to hit — users could only
 * grab near the middle (where the visual icon sits).
 *
 * These helpers make the band thicker (a configurable multiple of frameWidth,
 * clamped so it never exceeds the frame) and let each edge span its full
 * length. The bottom-right corner keeps a dedicated square for diagonal resize,
 * and toolFromPos tests it first, so widening the edges does not steal the
 * corner.
 *
 * Header-only, no Qt widget deps beyond QRectF — trivially unit-testable.
 */
namespace UBResizeGrip
{
    /// How many frame-widths thick the clickable edge band is (visual icon
    /// stays centered and unchanged; only the hit zone grows).
    inline qreal bandThickness(qreal frameWidth, qreal rectSpan)
    {
        qreal band = 2.0 * frameWidth;      // #277: thicker than the old 1x band
        // Never let the two opposite bands overlap/exceed the frame: cap at
        // just under half the span so left/right (or top/bottom) stay distinct.
        const qreal maxBand = rectSpan / 2.0 - 1.0;
        if (maxBand > 0.0 && band > maxBand)
            band = maxBand;
        if (band < frameWidth)
            band = frameWidth;              // never thinner than before
        return band;
    }

    inline QRectF bottomRightRect(const QRectF& rect, qreal frameWidth)
    {
        // Dedicated diagonal-resize corner (tested first in toolFromPos).
        return QRectF(rect.right() - frameWidth, rect.bottom() - frameWidth,
                      frameWidth, frameWidth);
    }

    inline QRectF leftRect(const QRectF& rect, qreal frameWidth)
    {
        const qreal band = bandThickness(frameWidth, rect.width());
        // Full height (no end cut-back): the whole left edge resizes.
        return QRectF(rect.left(), rect.top(), band, rect.height());
    }

    inline QRectF rightRect(const QRectF& rect, qreal frameWidth)
    {
        const qreal band = bandThickness(frameWidth, rect.width());
        return QRectF(rect.right() - band, rect.top(), band, rect.height());
    }

    inline QRectF topRect(const QRectF& rect, qreal frameWidth)
    {
        const qreal band = bandThickness(frameWidth, rect.height());
        return QRectF(rect.left(), rect.top(), rect.width(), band);
    }

    inline QRectF bottomRect(const QRectF& rect, qreal frameWidth)
    {
        const qreal band = bandThickness(frameWidth, rect.height());
        return QRectF(rect.left(), rect.bottom() - band, rect.width(), band);
    }
}

#endif // UBRESIZEGRIPGEOMETRY_H
