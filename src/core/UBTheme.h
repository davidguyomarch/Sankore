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

#ifndef UBTHEME_H
#define UBTHEME_H

#include <QColor>
#include <QBrush>

/**
 * @brief Centralized design tokens for Open-Sankoré UI theming.
 *
 * All UI colors should reference these tokens rather than hardcoded values.
 * This enables future dark/light mode switching by changing values in one place.
 *
 * Naming convention follows Material Design roles:
 *   surface     — background of panels/containers
 *   onSurface   — text/icons on surface
 *   primary     — accent color for interactive elements
 *   border      — subtle separators
 */
namespace UBTheme
{
    // -- Surface colors (panels, palettes, containers) --
    inline const QColor surface()           { return QColor(42, 42, 42, 220); }
    inline const QColor surfaceOpaque()     { return QColor(32, 32, 32, 240); }
    inline const QColor surfaceVariant()    { return QColor(58, 58, 58); }       // #3A3A3A — inputs, buttons
    inline const QColor surfaceHover()      { return QColor(74, 74, 74); }       // #4A4A4A — hover states

    // -- Content colors (text, icons) --
    inline const QColor onSurface()         { return QColor(255, 255, 255); }
    inline const QColor onSurfaceDim()      { return QColor(221, 221, 221); }    // #DDDDDD — secondary text
    inline const QColor onSurfaceDisabled() { return QColor(128, 128, 128); }

    // -- Primary accent --
    inline const QColor primary()           { return QColor(74, 144, 217); }     // #4A90D9
    inline const QColor onPrimary()         { return QColor(255, 255, 255); }

    // -- Borders and dividers --
    inline const QColor border()            { return QColor(255, 255, 255, 25); }
    inline const QColor borderSubtle()      { return QColor(255, 255, 255, 40); }

    // -- Tab states --
    inline const QColor tabInactive()       { return QColor(60, 60, 60, 160); }
    inline const QColor tabActive()         { return surface(); }

    // -- Board-specific --
    inline const QColor crossDark()         { return QColor(44, 44, 44, 200); }
    inline const QColor crossLight()        { return QColor(165, 225, 255); }
    inline const QColor documentSizeMarkDark()  { return QColor(44, 44, 44, 200); }
    inline const QColor documentSizeMarkLight() { return QColor(241, 241, 241); }
    inline const QColor documentViewLight()     { return QColor(241, 241, 241); }

    // -- Eraser brush/pen --
    inline const QBrush eraserBrushLight()  { return QBrush(QColor(255, 255, 255, 30)); }
    inline const QBrush eraserBrushDark()   { return QBrush(QColor(127, 127, 127, 30)); }
    inline const QPen   eraserPenDark()     { return QPen(QColor(255, 255, 255, 63)); }
    inline const QPen   eraserPenLight()    { return QPen(QColor(0, 0, 0, 63)); }

    // -- Scrollbar --
    inline const QColor scrollHandle()      { return QColor(255, 255, 255, 80); }
    inline const QColor scrollHandleHover() { return QColor(255, 255, 255, 120); }

    // -- Selection --
    inline const QColor selectionBg()       { return primary(); }
    inline const QColor selectionFg()       { return onPrimary(); }

    // -- Tree view (document navigator) --
    inline const QColor treeViewBg()        { return QColor(42, 42, 42); }
}

#endif // UBTHEME_H
