/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 * Copyright (C) 2026 David Guyomarch
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



#include "UBStringUtils.h"
#include "UBPureFunctions.h"

#include <QRegularExpression>
#include <algorithm>

QStringList UBStringUtils::sortByLastDigit(const QStringList& sourceList)
{
    return UBPure::sortByLastDigit(sourceList);
}


QString UBStringUtils::netxDigitizedName(const QString& source)
{

    // we look for a set of digit after non digits and at the end
    QRegularExpression rx("\\D(\\d+)");

    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator it = rx.globalMatch(source);
    while (it.hasNext()) {
        match = it.next();
    }

    int digit = -1;

    if (match.hasMatch())
    {
        digit = match.captured(1).toInt();
    }

    QString ret(source);

    if (digit == -1)
    {
        return ret + " 1";
    }
    else
    {
        QString s("%1");
        s = s.arg(digit + 1);
        return ret.replace(match.captured(1), s);
    }
}

QString UBStringUtils::toCanonicalUuid(const QUuid& uuid)
{
    return UBPure::toCanonicalUuid(uuid);
}

QString UBStringUtils::toUtcIsoDateTime(const QDateTime& dateTime)
{
    return UBPure::toUtcIsoDateTime(dateTime);
}

QDateTime UBStringUtils::fromUtcIsoDate(const QString& dateString)
{
    return UBPure::fromUtcIsoDate(dateString);
}




