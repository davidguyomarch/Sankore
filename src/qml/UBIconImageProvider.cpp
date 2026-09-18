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
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

// #351 diagnostics: trace every icon request into startup.log so we can see,
// from the VM, whether the provider is even called and whether the SVG loads.
// TODO(remove): delete once the icon rendering is confirmed on the VM.
static void ubIconLog(const QString& line)
{
    QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
    if (f.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&f);
        out << "[ICON] " << line << "\n";
    }
}

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

    const QString qrcPath = QStringLiteral(":/icons/phosphor/") + iconName + QStringLiteral(".svg");
    QSvgRenderer renderer(qrcPath);

    bool rendered = false;
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
        rendered = true;
    }

    // #351 diagnostics.
    {
        // Sample the center pixel alpha to confirm something was actually drawn.
        const int centerAlpha = qAlpha(image.pixel(w / 2, h / 2));
        ubIconLog(QStringLiteral("id=%1 name=%2 tint=%3 svgValid=%4 size=%5x%6 centerA=%7")
                      .arg(id, iconName, tint.name(QColor::HexRgb))
                      .arg(rendered ? 1 : 0).arg(w).arg(h).arg(centerAlpha));
    }

    if (size)
        *size = image.size();

    return image;
}
