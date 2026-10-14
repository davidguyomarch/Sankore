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
#include <cmath>

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

    /**
     * WCAG relative luminance of a colour (0.0 = black, 1.0 = white).
     * (#494) Same formula as UBKeyboardColors::relativeLuminance; duplicated
     * here so the ink-colour helpers stay dependency-free and self-contained.
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
     * (#494) Decide whether a stroke drawn over a given AVERAGE background colour
     * should use its DARK-background ink variant.
     *
     * On the board, the day/night variant follows the page's own dark/light
     * flag. On the desktop overlay, the stroke sits over the real desktop, whose
     * luminance is unrelated to the board page flag — so we pick the variant
     * from the desktop's average luminance instead: a DARK desktop → use the
     * dark-background (bright) ink variant, a LIGHT desktop → the light variant.
     *
     * Returns true when the background is dark enough to warrant the
     * dark-background ink variant (luminance below the mid threshold).
     */
    inline bool wantsDarkInkVariant(const QColor& averageBackground)
    {
        return relativeLuminance(averageBackground) < 0.5;
    }

    /**
     * (#494) Decide the active day/night ink VARIANT at stroke-creation time.
     *
     * On the board, follow the page's own dark/light flag (@p pageIsDark). On
     * the desktop overlay, a luminance hint (@p hasDesktopHint) overrides it
     * with the real desktop's dark/light state (@p desktopIsDark). Pure so both
     * draw pipelines (UBInputRouter, UBDrawingHandler) and the test share it.
     */
    inline bool activeDarkInkVariant(bool pageIsDark,
                                     bool hasDesktopHint,
                                     bool desktopIsDark)
    {
        return hasDesktopHint ? desktopIsDark : pageIsDark;
    }
}

#endif // UBINKCOLORUTILS_H
