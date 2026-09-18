/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBICONIMAGEPROVIDER_H
#define UBICONIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

/**
 * UBIconImageProvider — renders a Phosphor SVG icon, recolored, on the CPU.
 *
 * Why (#351/#352): recoloring the toolbar icons in QML with a GPU effect
 * (ColorOverlay, then MultiEffect, then a layer.effect) left the icons
 * invisible on the Windows test VM. That VM runs the x64 build under emulation
 * with no working GPU, so Qt Quick falls back to its software backend, which
 * does not run shader/layer effects — plain Rectangles paint (colors show) but
 * every effect-based icon does not. See ADR 0007.
 *
 * This provider does the SVG rasterization AND the tint with QPainter (CPU), so
 * QML only needs a plain Image with no effect. Works identically on the GPU and
 * software backends.
 *
 * URL form (see roleNames in QML):
 *   image://phosphor/<icon-name>?c=RRGGBB
 * e.g. image://phosphor/pen?c=1a1a1a
 * The color is the requested tint (hex, no leading '#'); omitted → black.
 * The requested size drives the raster resolution (falls back to 24x24).
 */
class UBIconImageProvider : public QQuickImageProvider
{
public:
    UBIconImageProvider();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
};

#endif // UBICONIMAGEPROVIDER_H
