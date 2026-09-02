/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBKeyboardPaletteColors.h"

#include "gui/UBKeyboardPaletteColors.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <cmath>

using namespace UBKeyboardColors;

// Renders a glyph the same way UBKeyboardButton::paintContent does: a light-grey
// key face filled first, then the glyph drawn on top with the given pen color.
// Returns the fraction of pixels in the glyph area that visibly differ from the
// key face (a proxy for "is the glyph legible").
double TestUBKeyboardPaletteColors::glyphVisibility(const QColor& penColor)
{
    const QSize keySize(41, 41);
    const QColor face = passiveKeyFace();

    QImage img(keySize, QImage::Format_ARGB32);
    img.fill(face);

    {
        QPainter painter(&img);
        QFont f = painter.font();
        f.setPixelSize(24);
        f.setBold(true);
        painter.setFont(f);
        painter.setPen(penColor);
        painter.drawText(QRect(QPoint(0, 0), keySize), Qt::AlignCenter, QStringLiteral("W"));
    }

    // Return the PEAK contrast ratio of any rendered pixel against the key face.
    // Peak contrast is independent of font metrics / glyph coverage (which vary
    // across platforms), so it is a portable proxy for "does readable ink appear":
    // a dark pen leaves pixels near its own (high-contrast) color, a white pen
    // cannot exceed white-on-light-grey contrast no matter the glyph shape.
    double peak = 1.0;
    for (int y = 0; y < img.height(); ++y)
    {
        const QRgb* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x)
        {
            QColor px(qRed(line[x]), qGreen(line[x]), qBlue(line[x]));
            peak = std::max(peak, contrastRatio(px, face));
        }
    }
    return peak;
}

void TestUBKeyboardPaletteColors::testKeyLabelContrastWithFaces()
{
    const QColor label = keyLabelColor();

    double passiveRatio = contrastRatio(label, passiveKeyFace());
    double activeRatio = contrastRatio(label, activeKeyFace());

    // WCAG AA for normal text is 4.5:1. Dark label on light key must exceed it.
    QVERIFY2(passiveRatio >= 4.5,
             qPrintable(QStringLiteral("Label/passive-key contrast too low: %1").arg(passiveRatio)));
    QVERIFY2(activeRatio >= 4.5,
             qPrintable(QStringLiteral("Label/active-key contrast too low: %1").arg(activeRatio)));

    // Sanity: white (the buggy dark-theme pen) must FAIL this same contract,
    // which is exactly why #263 happened.
    double whitePassive = contrastRatio(Qt::white, passiveKeyFace());
    QVERIFY2(whitePassive < 4.5,
             "White label somehow passes contrast on a light key — test premise is wrong");
}

void TestUBKeyboardPaletteColors::testRenderedGlyphReadability()
{
    // Peak on-key contrast of the rendered glyph (font-metric independent).
    // OLD behavior: white pen (inherited from the dark stylesheet).
    double whitePeak = glyphVisibility(Qt::white);
    // FIXED behavior: the palette's dark label color.
    double darkPeak = glyphVisibility(keyLabelColor());

    // White ink on the light-grey key can never reach readable contrast: even
    // fully-white pixels are only ~1.2:1 against the face. Cap generously at 2:1.
    QVERIFY2(whitePeak < 2.0,
             qPrintable(QStringLiteral("White glyph reached unexpected contrast: %1").arg(whitePeak)));

    // The dark pen must produce clearly readable ink (WCAG AA >= 4.5:1). On the
    // #222 label this peaks well above 8:1 wherever the glyph is drawn.
    QVERIFY2(darkPeak >= 4.5,
             qPrintable(QStringLiteral("Dark glyph contrast too low: %1").arg(darkPeak)));

    // The fix must be a large improvement over the buggy rendering.
    QVERIFY(darkPeak > whitePeak * 2.0);
}
