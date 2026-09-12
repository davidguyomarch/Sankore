/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBINKCOLORUTILS_H
#define TST_UBINKCOLORUTILS_H

#include <QObject>
#include <QtTest>

/**
 * @brief Tests for UBInkColors::recoloredDefaultInk (#317).
 *
 * A default-black shape must become white when the background flips to dark
 * (and vice versa), while a user-chosen color and a transparent fill are
 * preserved.
 */
class TestUBInkColorUtils : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultBlackToWhiteOnDark();
    void testDefaultWhiteToBlackOnLight();
    void testUserColorPreserved();
    void testTransparentPreserved();
    void testIdempotentOnCorrectDefault();
};

#endif // TST_UBINKCOLORUTILS_H
