/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBIconImageProvider.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QColor>
#include <QUrl>
#include <QUrlQuery>

UBIconImageProvider::UBIconImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage UBIconImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    // id is "<icon-name>?c=RRGGBB". QUrl parses the query for us; the path part
    // (icon name) may legitimately contain no slash.
    QString iconName = id;
    QColor tint(Qt::black);

    const int q = id.indexOf(QLatin1Char('?'));
    if (q >= 0)
    {
        iconName = id.left(q);
        const QUrlQuery query(id.mid(q + 1));
        const QString c = query.queryItemValue(QStringLiteral("c"));
        if (!c.isEmpty())
        {
            QColor parsed(QStringLiteral("#") + c);
            if (parsed.isValid())
                tint = parsed;
        }
    }

    int w = requestedSize.width()  > 0 ? requestedSize.width()  : 24;
    int h = requestedSize.height() > 0 ? requestedSize.height() : 24;

    QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QSvgRenderer renderer(QStringLiteral("qrc:/icons/phosphor/") + iconName + QStringLiteral(".svg"));
    // QSvgRenderer does not resolve the "qrc:" scheme; use the ":" resource path.
    if (!renderer.isValid())
        renderer.load(QStringLiteral(":/icons/phosphor/") + iconName + QStringLiteral(".svg"));

    if (renderer.isValid())
    {
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        renderer.render(&painter, QRectF(0, 0, w, h));

        // Tint: keep the icon's alpha (its shape) but replace RGB with the tint.
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(image.rect(), tint);
        painter.end();
    }

    if (size)
        *size = image.size();

    return image;
}
