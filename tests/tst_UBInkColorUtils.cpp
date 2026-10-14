/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBInkColorUtils.h"

#include "domain/UBInkColorUtils.h"

using namespace UBInkColors;

// nowLight = state AFTER the flip. Switching TO dark => nowLight = false.

void TestUBInkColorUtils::testDefaultBlackToWhiteOnDark()
{
    // Shape drawn black on a light background, background flips to dark.
    QColor r = recoloredDefaultInk(QColor(Qt::black), /*nowLight*/ false);
    QCOMPARE(r, QColor(Qt::white));
}

void TestUBInkColorUtils::testDefaultWhiteToBlackOnLight()
{
    // Shape white on dark, background flips to light.
    QColor r = recoloredDefaultInk(QColor(Qt::white), /*nowLight*/ true);
    QCOMPARE(r, QColor(Qt::black));
}

void TestUBInkColorUtils::testUserColorPreserved()
{
    // A user-chosen red must never be flipped, on either background.
    QColor red(220, 30, 30);
    QCOMPARE(recoloredDefaultInk(red, false), red);
    QCOMPARE(recoloredDefaultInk(red, true), red);
}

void TestUBInkColorUtils::testTransparentPreserved()
{
    QColor transparent(0, 0, 0, 0);
    QColor r = recoloredDefaultInk(transparent, false);
    QCOMPARE(r.alpha(), 0);
}

void TestUBInkColorUtils::testIdempotentOnCorrectDefault()
{
    // Black is already correct on a light background: leave it black.
    QCOMPARE(recoloredDefaultInk(QColor(Qt::black), /*nowLight*/ true), QColor(Qt::black));
    // White is already correct on a dark background: leave it white.
    QCOMPARE(recoloredDefaultInk(QColor(Qt::white), /*nowLight*/ false), QColor(Qt::white));
}

// #494 ---------------------------------------------------------------------

void TestUBInkColorUtils::testWantsDarkInkVariantFromLuminance()
{
    // A dark desktop → use the dark-background (bright) ink variant.
    QVERIFY(wantsDarkInkVariant(QColor(Qt::black)));
    QVERIFY(wantsDarkInkVariant(QColor(30, 30, 30)));
    // A light desktop → use the light-background ink variant.
    QVERIFY(!wantsDarkInkVariant(QColor(Qt::white)));
    QVERIFY(!wantsDarkInkVariant(QColor(230, 230, 230)));
}

void TestUBInkColorUtils::testActiveDarkInkVariantUsesPageFlagWithoutHint()
{
    // No desktop hint → follow the page's own dark/light flag (board behaviour).
    QCOMPARE(activeDarkInkVariant(/*pageIsDark*/ true,  /*hasHint*/ false, /*desktopDark*/ false), true);
    QCOMPARE(activeDarkInkVariant(/*pageIsDark*/ false, /*hasHint*/ false, /*desktopDark*/ true),  false);
}

void TestUBInkColorUtils::testActiveDarkInkVariantUsesHintWhenPresent()
{
    // Desktop hint present → override the page flag with the desktop state.
    QCOMPARE(activeDarkInkVariant(/*pageIsDark*/ false, /*hasHint*/ true, /*desktopDark*/ true),  true);
    QCOMPARE(activeDarkInkVariant(/*pageIsDark*/ true,  /*hasHint*/ true, /*desktopDark*/ false), false);
}
