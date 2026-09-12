/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBINKCOLORUTILS_H
#define UBINKCOLORUTILS_H

#include <QColor>

/**
 * Pure helpers for recoloring default black/white ink on a day/night switch
 * (#317). Dependency-free (QtGui QColor only) so both the production renderer
 * and the unit tests share the exact same logic.
 */
namespace UBInkColors
{
    /**
     * Recolor a DEFAULT ink color (black/white) for the new background.
     *
     * Shapes store a single color (no light/dark pair like strokes), so we flip
     * only the color when it is the default ink of the PREVIOUS background:
     *   - white on a dark background,
     *   - black on a light background.
     * Any user-chosen color is returned unchanged (preserved). Fully transparent
     * colors (default empty fill) are left as-is.
     *
     * @param current  the item's current stroke/fill color
     * @param nowLight  the background state AFTER the flip (true = light)
     */
    inline QColor recoloredDefaultInk(const QColor& current, bool nowLight)
    {
        if (current.alpha() == 0)
            return current; // transparent default fill — nothing to recolor

        const QColor oldDefault = nowLight ? QColor(Qt::white) : QColor(Qt::black);
        const QColor newDefault = nowLight ? QColor(Qt::black) : QColor(Qt::white);

        if (current.rgb() == oldDefault.rgb())
            return newDefault;

        return current; // user-chosen color: preserve
    }
}

#endif // UBINKCOLORUTILS_H
