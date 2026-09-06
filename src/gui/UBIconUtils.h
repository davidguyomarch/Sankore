/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBICONUTILS_H
#define UBICONUTILS_H

#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QSvgRenderer>

/**
 * #285: helpers to use the Phosphor SVG icon set inside classic Qt widgets
 * (QAction / QToolButton / item-view decorations) with theme tinting.
 *
 * Phosphor SVGs are monochrome and declared with fill="currentColor", but
 * QSvgRenderer does not resolve currentColor — it would paint black. So we
 * render the glyph to a pixmap and tint it with the requested colour using a
 * SourceIn composition, which keeps the glyph's alpha and replaces its colour.
 *
 * Header-only on purpose: no QObject, no moc, usable from any widget.
 */
namespace UBIconUtils
{
    /// Render a Phosphor icon (by base name, e.g. "folder") tinted with `color`.
    /// `px` is the rendered square size in device-independent pixels.
    inline QPixmap phosphorPixmap(const QString& name, const QColor& color, int px = 32)
    {
        const QString path = QStringLiteral(":/icons/phosphor/") + name + QStringLiteral(".svg");

        QSvgRenderer renderer(path);
        if (!renderer.isValid())
            return QPixmap();

        QPixmap pm(px, px);
        pm.fill(Qt::transparent);
        {
            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            renderer.render(&p, QRectF(0, 0, px, px));
            // Tint: keep the glyph alpha, replace RGB with `color`.
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(pm.rect(), color);
        }
        return pm;
    }

    /// Convenience: a QIcon built from a tinted Phosphor glyph.
    inline QIcon phosphorIcon(const QString& name, const QColor& color, int px = 32)
    {
        QPixmap pm = phosphorPixmap(name, color, px);
        if (pm.isNull())
            return QIcon();
        return QIcon(pm);
    }
}

#endif // UBICONUTILS_H
