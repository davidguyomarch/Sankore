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

    // A pixel counts as "visible glyph" if it differs enough from the key face.
    // We use the WCAG contrast ratio of each pixel against the face; legible ink
    // needs a meaningful ratio, faint anti-aliased white does not.
    int visible = 0;
    int total = keySize.width() * keySize.height();
    for (int y = 0; y < img.height(); ++y)
    {
        const QRgb* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x)
        {
            QColor px(qRed(line[x]), qGreen(line[x]), qBlue(line[x]));
            if (contrastRatio(px, face) >= 2.0)
                ++visible;
        }
    }
    return static_cast<double>(visible) / total;
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
    // OLD behavior: white pen (inherited from the dark stylesheet) — glyph is
    // nearly invisible on the light-grey key face.
    double whiteVisibility = glyphVisibility(Qt::white);

    // FIXED behavior: the palette's dark label color renders a clearly visible glyph.
    double darkVisibility = glyphVisibility(keyLabelColor());

    QVERIFY2(whiteVisibility < 0.02,
             qPrintable(QStringLiteral("White glyph unexpectedly visible: %1").arg(whiteVisibility)));
    QVERIFY2(darkVisibility > 0.10,
             qPrintable(QStringLiteral("Dark glyph not visible enough: %1").arg(darkVisibility)));

    // And the fix must be a large improvement over the buggy rendering.
    QVERIFY(darkVisibility > whiteVisibility * 5.0);
}
