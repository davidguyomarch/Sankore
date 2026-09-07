/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBBACKGROUNDGRID_H
#define UBBACKGROUNDGRID_H

#include <QRectF>
#include <QString>
#include <vector>
#include <cmath>

/**
 * #289: French school rulings as page background types.
 *
 * This header holds the *pure* geometry of every background ruling so it can
 * be unit-tested without a QPainter / QGraphicsScene. The actual drawing
 * (pen widths, colours, zoom fading) lives in UBBackgroundRenderer, which
 * calls generateLines() and paints the returned segments.
 *
 * Coordinate model
 * ----------------
 * The board uses scene units. Historically the square grid step is
 * UBSettings::crossSize (32 units) and a Séyès cell is 8 mm, so we calibrate
 * 8 mm == 32 units, i.e. 4 units per millimetre. Keeping this mapping means
 * the existing square grid renders exactly as before (Grid == 8 mm cell) and
 * every ruling is sized relative to real millimetres.
 */
namespace UBBackgroundGrid
{
    /// The kind of ruling drawn behind the page.
    enum class Type
    {
        Plain = 0,       ///< no ruling (blank page)
        Grid = 1,        ///< uniform square grid (legacy "crossed" background)
        Seyes = 2,       ///< French Séyès: 8 mm cells, 2 mm interlines, vertical lines
        SeyesLarge = 3,  ///< enlarged Séyès (1.5x) for beginners
        DoubleLine3mm = 4///< maternelle double ruling: line pairs 3 mm apart
    };

    /// Calibration: 8 mm == 32 scene units (UBSettings::crossSize) → 4 u/mm.
    inline constexpr double unitsPerMm() { return 32.0 / 8.0; }

    /// Convert a Type to/from the persisted integer (SVG attribute value).
    inline int toInt(Type t) { return static_cast<int>(t); }
    inline Type fromInt(int v)
    {
        switch (v)
        {
        case 1: return Type::Grid;
        case 2: return Type::Seyes;
        case 3: return Type::SeyesLarge;
        case 4: return Type::DoubleLine3mm;
        default: return Type::Plain;
        }
    }

    /// Stable string token used in the .ubz SVG (forward/backward friendly).
    inline const char* toToken(Type t)
    {
        switch (t)
        {
        case Type::Grid:          return "grid";
        case Type::Seyes:         return "seyes";
        case Type::SeyesLarge:    return "seyes-large";
        case Type::DoubleLine3mm: return "double-3mm";
        case Type::Plain:         default: return "plain";
        }
    }
    inline Type fromToken(const QString& s)
    {
        if (s == QLatin1String("grid"))        return Type::Grid;
        if (s == QLatin1String("seyes"))       return Type::Seyes;
        if (s == QLatin1String("seyes-large")) return Type::SeyesLarge;
        if (s == QLatin1String("double-3mm"))  return Type::DoubleLine3mm;
        return Type::Plain;
    }

    /// True for any ruling that shows lines (i.e. maps to legacy crossed=true).
    inline bool isRuled(Type t) { return t != Type::Plain; }

    /// Weight of a line, mapped by the renderer to a pen width/colour.
    enum class Weight
    {
        Minor = 0,  ///< faint interline (2 mm guides)
        Major = 1,  ///< strong line (cell boundary)
        Margin = 2  ///< the vertical margin line (red on Séyès)
    };

    enum class Orientation { Horizontal, Vertical };

    struct Line
    {
        Orientation orientation;
        Weight weight;
        double pos;   ///< scene coordinate: y for Horizontal, x for Vertical

        bool operator==(const Line& o) const
        {
            return orientation == o.orientation && weight == o.weight
                   && std::abs(pos - o.pos) < 1e-6;
        }
    };

    /// Snap `origin`-aligned first coordinate at or below `start` for a given step.
    inline double firstAtOrBelow(double start, double step)
    {
        return std::floor(start / step) * step;
    }

    /**
     * Compute every line to draw for `type` within the visible `rect`.
     *
     * `upm` is units-per-mm (defaults to the board calibration). The returned
     * lines are ordered horizontals first (top→bottom) then verticals
     * (left→right); Minor lines are emitted before the Major line they sit
     * under only incidentally — callers should paint Minor first if overlap
     * matters, but positions are exact so order is not visually significant.
     *
     * For Plain, no lines are produced.
     */
    inline std::vector<Line> generateLines(Type type, const QRectF& rect,
                                           double upm = unitsPerMm())
    {
        std::vector<Line> lines;
        if (type == Type::Plain)
            return lines;

        const double x0 = rect.x();
        const double y0 = rect.y();
        const double x1 = rect.x() + rect.width();
        const double y1 = rect.y() + rect.height();

        auto addH = [&](double y, Weight w) { lines.push_back({Orientation::Horizontal, w, y}); };
        auto addV = [&](double x, Weight w) { lines.push_back({Orientation::Vertical, w, x}); };

        if (type == Type::Grid)
        {
            // Uniform 8 mm square grid — identical to the legacy crossed grid.
            const double step = 8.0 * upm; // == UBSettings::crossSize
            for (double y = firstAtOrBelow(y0, step); y < y1; y += step)
                addH(y, Weight::Major);
            for (double x = firstAtOrBelow(x0, step); x < x1; x += step)
                addV(x, Weight::Major);
            return lines;
        }

        if (type == Type::Seyes || type == Type::SeyesLarge)
        {
            // Séyès: major horizontal lines every 8 mm, with 3 faint interlines
            // 2 mm apart between them; vertical major lines every 8 mm; a single
            // red margin line 40 mm (5 cells) from the left of the page origin 0.
            const double scale = (type == Type::SeyesLarge) ? 1.5 : 1.0;
            const double cell = 8.0 * upm * scale;    // major spacing
            const double inter = 2.0 * upm * scale;   // interline spacing

            // Horizontal: emit interlines everywhere, promote every 4th (cell
            // boundary) to Major so the 8 mm rhythm reads clearly.
            const double hStart = firstAtOrBelow(y0, cell);
            for (double base = hStart; base < y1 + cell; base += cell)
            {
                for (int k = 0; k < 4; ++k)
                {
                    const double y = base + k * inter;
                    if (y < y0 || y >= y1) continue;
                    addH(y, (k == 0) ? Weight::Major : Weight::Minor);
                }
            }

            // Vertical major lines every cell.
            for (double x = firstAtOrBelow(x0, cell); x < x1; x += cell)
                addV(x, Weight::Major);

            // Red margin line at a fixed 40 mm from page origin (x==0), drawn
            // only when visible.
            const double marginX = 40.0 * upm * scale;
            if (marginX >= x0 && marginX < x1)
                addV(marginX, Weight::Margin);

            return lines;
        }

        if (type == Type::DoubleLine3mm)
        {
            // Maternelle double ruling: pairs of lines 3 mm apart, pairs spaced
            // 3 mm from each other (writing band 3 mm, gap 3 mm → 6 mm period).
            const double band = 3.0 * upm;   // line pair inner spacing
            const double period = 6.0 * upm; // pair-to-pair period
            for (double base = firstAtOrBelow(y0, period); base < y1 + period; base += period)
            {
                const double top = base;
                const double bottom = base + band;
                if (top >= y0 && top < y1)    addH(top, Weight::Major);
                if (bottom >= y0 && bottom < y1) addH(bottom, Weight::Minor);
            }
            return lines;
        }

        return lines;
    }
}

#endif // UBBACKGROUNDGRID_H
