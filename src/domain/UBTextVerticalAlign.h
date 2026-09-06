/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBTEXTVERTICALALIGN_H
#define UBTEXTVERTICALALIGN_H

#include <algorithm>

/**
 * Vertical alignment of the text content inside a text box (#278).
 *
 * The text box has a user-imposed height (boxHeight) that can be larger than
 * the actual text content (contentHeight). This decides where the content sits
 * vertically: anchored at the top (legacy behaviour), centered, or at the
 * bottom.
 *
 * The geometry (offset computation) is header-only and pure, so it is unit
 * testable without any Qt widget. The enum values are stable and serialized to
 * .ubz (0/1/2) — do not reorder.
 */
namespace UBTextVAlign
{
    enum Mode
    {
        Top    = 0,
        Middle = 1,
        Bottom = 2
    };

    /// Vertical offset (in item coordinates) to translate the text content by,
    /// so it aligns according to `mode` within a box of `boxHeight` when the
    /// content is `contentHeight` tall. Always >= 0 and never pushes content
    /// off the top (when the content is taller than the box, offset is 0).
    inline double verticalOffset(Mode mode, double boxHeight, double contentHeight)
    {
        const double slack = boxHeight - contentHeight;
        if (slack <= 0.0)
            return 0.0; // content fills or overflows the box: no offset

        double factor = 0.0;
        switch (mode)
        {
            case Top:    factor = 0.0; break;
            case Middle: factor = 0.5; break;
            case Bottom: factor = 1.0; break;
        }
        return slack * factor;
    }

    /// Clamp an arbitrary int (e.g. from SVG) to a valid Mode, defaulting to Top.
    inline Mode fromInt(int v)
    {
        if (v == Middle) return Middle;
        if (v == Bottom) return Bottom;
        return Top;
    }
}

#endif // UBTEXTVERTICALALIGN_H
