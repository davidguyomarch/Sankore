/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBPUREFUNCTIONS_H
#define UBPUREFUNCTIONS_H

#include <QPair>
#include <QString>
#include <QFont>
#include <QFontMetricsF>

/**
 * @brief Pure utility functions with no side effects and no app dependencies.
 *
 * These can be included directly in test binaries without linking UBApplication,
 * UBSettings, or any other singleton. No Q_OBJECT, no signals, no state.
 *
 * Adding a function here requires:
 *   1. It must be pure (deterministic, no side effects)
 *   2. It must not depend on UBApplication, UBSettings, or any singleton
 *   3. It must only use Qt Core/Gui types (no Widgets beyond QFont)
 */
namespace UBPure
{

/**
 * @brief Compute the effective zoom ratio, clamped to a maximum.
 *
 * @param requestedRatio The zoom multiplier requested
 * @param currentViewScale Current m11() of the view transform
 * @param systemScaleFactor The system DPI scale factor
 * @param maxZoom Maximum allowed zoom level (default 9.0)
 * @return QPair(clampedZoomLevel, adjustedRatio)
 */
inline QPair<qreal, qreal> computeZoomRatio(qreal requestedRatio, qreal currentViewScale, qreal systemScaleFactor, qreal maxZoom = 9.0)
{
    qreal currentZoom = requestedRatio * currentViewScale / systemScaleFactor;
    qreal usedRatio = requestedRatio;

    if (currentZoom > maxZoom)
    {
        currentZoom = maxZoom;
        usedRatio = currentZoom * systemScaleFactor / currentViewScale;
    }

    return QPair<qreal, qreal>(currentZoom, usedRatio);
}

/**
 * @brief Elide text to fit within a given pixel width.
 *
 * @param text The text to potentially truncate
 * @param maxWidth Maximum width in pixels
 * @param font The font used for measurement
 * @return The elided text (with ellipsis if truncated)
 */
inline QString truncateText(const QString& text, int maxWidth, const QFont& font)
{
    QFontMetricsF fontMetrics(font);
    return fontMetrics.elidedText(text, Qt::ElideRight, maxWidth);
}

} // namespace UBPure

#endif // UBPUREFUNCTIONS_H
