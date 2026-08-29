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
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBPUREFUNCTIONS_H
#define UBPUREFUNCTIONS_H

#include <QPair>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QFontMetricsF>
#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>
#include <QMultiMap>

#include <cmath>
#include <algorithm>

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

// --- Zoom ---

/**
 * @brief Compute the effective zoom ratio, clamped to a maximum.
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

// --- Text ---

/**
 * @brief Elide text to fit within a given pixel width.
 */
inline QString truncateText(const QString& text, int maxWidth, const QFont& font)
{
    QFontMetricsF fontMetrics(font);
    return fontMetrics.elidedText(text, Qt::ElideRight, maxWidth);
}

/**
 * @brief Convert a QUuid to its canonical string (without braces).
 */
inline QString toCanonicalUuid(const QUuid& uuid)
{
    QString s = uuid.toString();
    if (s.startsWith("{"))
        s = s.right(s.length() - 1);
    if (s.endsWith("}"))
        s = s.left(s.length() - 1);
    return s;
}

/**
 * @brief Convert a QDateTime to UTC ISO 8601 string with trailing Z.
 */
inline QString toUtcIsoDateTime(const QDateTime& dateTime)
{
    QString isoStr = dateTime.toUTC().toString(Qt::ISODate);
    if (!isoStr.endsWith('Z'))
        isoStr += "Z";
    return isoStr;
}

/**
 * @brief Parse a UTC ISO 8601 date string to local QDateTime.
 */
inline QDateTime fromUtcIsoDate(const QString& dateString)
{
    return QDateTime::fromString(dateString, Qt::ISODate).toLocalTime();
}

// --- Geometry ---

/**
 * @brief Constrain a point to stay within a rectangle.
 */
inline QPointF pointConstrainedInRect(QPointF point, QRectF rect)
{
    return QPointF(
        qMax(rect.x(), qMin(rect.x() + rect.width(), point.x())),
        qMax(rect.y(), qMin(rect.y() + rect.height(), point.y()))
    );
}

/**
 * @brief Compute an inner square from a line segment and width.
 */
inline QRectF lineToInnerRect(const QLineF& pLine, const qreal& pWidth)
{
    qreal centerX = (pLine.x1() + pLine.x2()) / 2;
    qreal centerY = (pLine.y1() + pLine.y2()) / 2;
    qreal side = std::sqrt((pWidth * pWidth) / 2);
    qreal halfSide = side / 2;
    return QRectF(centerX - halfSide, centerY - halfSide, side, side);
}

// --- String sorting ---

/**
 * @brief Sort a list of filenames by the last digit sequence before the extension.
 */
inline QStringList sortByLastDigit(const QStringList& sourceList)
{
    QRegularExpression rx("\\D(\\d+)\\.");
    QMultiMap<int, QString> elements;

    for (const QString& source : sourceList)
    {
        QRegularExpressionMatch match;
        QRegularExpressionMatchIterator it = rx.globalMatch(source);
        while (it.hasNext())
            match = it.next();

        int digit = match.hasMatch() ? match.captured(1).toInt() : -1;
        elements.insert(digit, source);
    }

    QStringList result;
    QList<int> keys = elements.keys();
    std::sort(keys.begin(), keys.end());

    for (int key : keys)
    {
        QList<QString> values = elements.values(key);
        std::sort(values.begin(), values.end());
        for (const QString& val : values)
        {
            if (!result.contains(val))
                result << val;
        }
    }

    return result;
}

} // namespace UBPure

#endif // UBPUREFUNCTIONS_H
