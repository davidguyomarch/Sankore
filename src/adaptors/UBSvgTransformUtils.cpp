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

#include "UBSvgTransformUtils.h"
#include <QStringList>

namespace UBSvgTransformUtils
{

QString toSvgTransform(const QTransform& matrix)
{
    return QString("matrix(%1, %2, %3, %4, %5, %6)")
           .arg(matrix.m11(), 0, 'g')
           .arg(matrix.m12(), 0, 'g')
           .arg(matrix.m21(), 0, 'g')
           .arg(matrix.m22(), 0, 'g')
           .arg(matrix.dx(), 0, 'g')
           .arg(matrix.dy(), 0, 'g');
}

QTransform fromSvgTransform(const QString& transform)
{
    QTransform matrix;
    QString ts = transform;
    ts.replace("matrix(", "");
    ts.replace(")", "");
    QStringList sl = ts.split(",");

    if (sl.size() >= 6)
    {
        matrix.setMatrix(
            sl.at(0).toFloat(),
            sl.at(1).toFloat(),
            0,
            sl.at(2).toFloat(),
            sl.at(3).toFloat(),
            0,
            sl.at(4).toFloat(),
            sl.at(5).toFloat(),
            1);
    }

    return matrix;
}

} // namespace UBSvgTransformUtils
