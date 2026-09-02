/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBKEYBOARDPALETTECOLORS_H
#define UBKEYBOARDPALETTECOLORS_H

#include <QColor>
#include <cmath>

/**
 * Color contract for the virtual keyboard (UBKeyboardPalette) — issue #263.
 *
 * The key faces are drawn from fixed light-grey PNG images
 * (centre-passive ~= RGB 235, centre-active ~= RGB 188). The glyph/label text
 * must therefore always use a dark, theme-independent pen; otherwise the
 * app-wide dark stylesheet (onSurface = white) leaves the labels unreadable.
 *
 * This header is intentionally dependency-free (QtGui only) so both the
 * production palette code and the unit tests can share the exact same contract.
 */
namespace UBKeyboardColors
{
    /** Approximate face color of a passive (unpressed) key. */
    inline QColor passiveKeyFace() { return QColor(235, 236, 237); }

    /** Approximate face color of an active (pressed) key. */
    inline QColor activeKeyFace() { return QColor(188, 189, 189); }

    /** Dark near-black color used to paint every key glyph/label. */
    inline QColor keyLabelColor() { return QColor(0x22, 0x22, 0x22); }

    /**
     * WCAG relative luminance of a color (0.0 = black, 1.0 = white).
     */
    inline double relativeLuminance(const QColor& c)
    {
        auto lin = [](double v) {
            v /= 255.0;
            return (v <= 0.03928) ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * lin(c.red()) + 0.7152 * lin(c.green()) + 0.0722 * lin(c.blue());
    }

    /**
     * WCAG contrast ratio between two colors (1.0 = none, 21.0 = black/white).
     */
    inline double contrastRatio(const QColor& a, const QColor& b)
    {
        double la = relativeLuminance(a);
        double lb = relativeLuminance(b);
        double hi = std::max(la, lb);
        double lo = std::min(la, lb);
        return (hi + 0.05) / (lo + 0.05);
    }
}

#endif // UBKEYBOARDPALETTECOLORS_H
