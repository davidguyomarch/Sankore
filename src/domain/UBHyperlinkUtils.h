/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBHYPERLINKUTILS_H
#define UBHYPERLINKUTILS_H

#include <QString>

/**
 * Pure helpers for hyperlink insertion in text items (#280).
 * Header-only, no widget deps — unit testable.
 */
namespace UBHyperlink
{
    /// Normalise a user-typed URL: trims it and prepends "https://" when no
    /// scheme is present, so "example.com" becomes "https://example.com" while
    /// "http://x", "https://x", "mailto:a@b" and "ftp://x" are left untouched.
    /// An empty input stays empty (caller decides whether to insert).
    inline QString normalizeUrl(const QString& rawUrl)
    {
        QString url = rawUrl.trimmed();
        if (url.isEmpty())
            return url;
        if (url.contains(QStringLiteral("://")) || url.startsWith(QStringLiteral("mailto:")))
            return url;
        return QStringLiteral("https://") + url;
    }
}

#endif // UBHYPERLINKUTILS_H
