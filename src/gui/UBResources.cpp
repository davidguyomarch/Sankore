/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 * Copyright (C) 2026 David Guyomarch
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBResources.h"
#include <QFontDatabase>

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QSvgRenderer>
#include <QImage>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"
#include "qml/UBThemeManager.h"
#include "board/UBBoardController.h"
#include "controllers/UBToolController.h"


UBResources* UBResources::sSingleton = 0;

UBResources::UBResources(QObject* pParent)
 : QObject(pParent)
 , mSettings(nullptr)
{
    // mSettings initialized in buildFontList() — UBResources is created
    // before UBSettings singleton exists
}

UBResources::~UBResources()
{
    // NOOP
}

UBResources* UBResources::resources()
{
    if (!sSingleton)
    {
        sSingleton = new UBResources(UBApplication::staticMemoryCleaner);
        sSingleton->init();
        sSingleton->buildFontList();
    }

    return sSingleton;

}

void UBResources::init()
{
    // Static (non-themed) cursors — legacy raster assets that already read on
    // any background.
    eraserCursor    = QCursor(QPixmap(":/images/cursors/eraser.png"), 21, 21);
    markerCursor    = QCursor(QPixmap(":/images/cursors/marker.png"), 3, 30);
    pointerCursor   = QCursor(QPixmap(":/images/cursors/laser.png"), 2, 1);
    handCursor      = QCursor(Qt::OpenHandCursor);
    zoomInCursor    = QCursor(QPixmap(":/images/cursors/zoomIn.png"), 9, 9);
    zoomOutCursor   = QCursor(QPixmap(":/images/cursors/zoomOut.png"), 9, 9);
    arrowCursor     = QCursor(Qt::ArrowCursor);
    playCursor      = QCursor(QPixmap(":/images/cursors/play.png"), 6, 1);
    textCursor      = QCursor(Qt::ArrowCursor);
    richTextCursor  = QCursor(Qt::ArrowCursor);
    rotateCursor    = QCursor(QPixmap(":/images/cursors/rotate.png"), 16, 16);
    drawLineRulerCursor = QCursor(QPixmap(":/images/cursors/drawRulerLine.png"), 3, 12);

    // #439: SVG tool cursors (pen, OCR, fill) are built from monochrome Phosphor
    // glyphs. Build them tinted to the current theme so they stay visible on the
    // scene background (which follows dark/light mode too), and rebuild them when
    // the theme changes.
    buildThemedCursors();
    connect(UBThemeManager::instance(), &UBThemeManager::themeChanged,
            this, &UBResources::updateThemedCursors);
}

QPixmap UBResources::renderCursorSvg(const QString& svgResource, int size,
                                     const QColor& glyph, const QColor& outline)
{
    QSvgRenderer renderer(svgResource);
    if (!renderer.isValid())
        return QPixmap();

    // Render the raw glyph (alpha mask) once at the target size.
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    const int px = qMax(1, int(size * dpr));

    QImage mask(px, px, QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    {
        QPainter p(&mask);
        p.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&p, QRectF(0, 0, px, px));
    }

    // Tint the glyph: keep the alpha of the rendered shape, replace RGB.
    auto tint = [&](const QColor& c) {
        QImage out = mask;
        QPainter p(&out);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(out.rect(), c);
        p.end();
        return out;
    };

    QImage glyphImg = tint(glyph);
    QImage outlineImg = tint(outline);

    // Compose: draw the outline shifted by 1px in 8 directions to form a thin
    // halo, then the tinted glyph on top. This gives a contrasting border so the
    // cursor reads on both dark and light backgrounds.
    QImage composed(px, px, QImage::Format_ARGB32_Premultiplied);
    composed.fill(Qt::transparent);
    {
        QPainter p(&composed);
        const int d = qMax(1, int(dpr));
        for (int dx = -d; dx <= d; ++dx)
            for (int dy = -d; dy <= d; ++dy)
                if (dx != 0 || dy != 0)
                    p.drawImage(dx, dy, outlineImg);
        p.drawImage(0, 0, glyphImg);
    }

    QPixmap pix = QPixmap::fromImage(composed);
    pix.setDevicePixelRatio(dpr);
    return pix;
}

void UBResources::buildThemedCursors()
{
    auto* tm = UBThemeManager::instance();
    // #441: colours come from UBThemeManager roles (no hard-coded literals,
    // #297 ratchet). The glyph takes the theme foreground (`onSurface`: white on
    // dark, near-black on light); the outline halo takes the theme `surface`,
    // forced opaque, which is the contrasting background colour — so the cursor
    // reads whatever is underneath.
    QColor glyph = tm->onSurface();
    glyph.setAlpha(255);
    QColor outline = tm->surface();
    outline.setAlpha(255);

    const int sz = 32;
    QPixmap penPix  = renderCursorSvg(":/images/cursors/pen.svg", sz, glyph, outline);
    QPixmap ocrPix  = renderCursorSvg(":/images/cursors/ocr.svg", sz, glyph, outline);
    QPixmap fillPix = renderCursorSvg(":/icons/phosphor/paint-bucket.svg", sz, glyph, outline);

    // Fallbacks keep the previous behaviour if an SVG fails to render.
    if (penPix.isNull())
        penPix = QPixmap(":/images/cursors/pen.svg");
    if (ocrPix.isNull())
        ocrPix = QPixmap(":/images/cursors/ocr.svg").scaled(sz, sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (fillPix.isNull())
        fillPix = QPixmap(":/icons/phosphor/paint-bucket.svg").scaled(sz, sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    penCursor  = QCursor(penPix, 4, 28);
    ocrCursor  = QCursor(ocrPix, 4, 28);
    // #429-followup: hotspot near the bucket's spout (bottom-left of the glyph).
    fillCursor = QCursor(fillPix, 6, 26);
}

void UBResources::updateThemedCursors()
{
    buildThemedCursors();

    // Re-apply the active tool cursor so the change is visible immediately
    // without the user having to switch tools.
    if (UBApplication::boardController && UBToolController::toolController())
    {
        UBApplication::boardController->setToolCursor(
            UBToolController::toolController()->stylusTool());
    }
}

void UBResources::buildFontList()
{
    if (!mSettings)
        mSettings = UBSettings::settings();
    QString customFontDirectory = mSettings->applicationCustomFontDirectory();
    QStringList fontFiles = UBFileSystemUtils::allFiles(customFontDirectory);
    for (const QString& fontFile : fontFiles){
        int fontId = QFontDatabase::addApplicationFont(fontFile);
        mCustomFontList << QFontDatabase::applicationFontFamilies(fontId);
    }
}
