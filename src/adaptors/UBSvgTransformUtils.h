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

#ifndef UBSVGTRANSFORMUTILS_H
#define UBSVGTRANSFORMUTILS_H

#include <QString>
#include <QTransform>

/**
 * @brief Free functions for SVG matrix transform serialization/deserialization.
 *
 * Extracted from UBSvgSubsetAdaptor private static methods so they can be
 * unit-tested directly without pulling in the full adaptor dependency graph.
 */
namespace UBSvgTransformUtils
{
    /**
     * Serialize a QTransform to an SVG "matrix(m11, m12, m21, m22, dx, dy)" string.
     */
    QString toSvgTransform(const QTransform& matrix);

    /**
     * Deserialize an SVG "matrix(...)" string back to a QTransform.
     * Returns identity if the input is invalid or has fewer than 6 components.
     */
    QTransform fromSvgTransform(const QString& transform);
}

#endif // UBSVGTRANSFORMUTILS_H
