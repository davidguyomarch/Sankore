/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBTextVerticalAlign.h"
#include "domain/UBTextVerticalAlign.h"

// Box 200 tall, content 60 tall -> slack 140.
void TestUBTextVerticalAlign::testTopIsZeroOffset()
{
    QCOMPARE(UBTextVAlign::verticalOffset(UBTextVAlign::Top, 200.0, 60.0), 0.0);
}

void TestUBTextVerticalAlign::testMiddleIsHalfSlack()
{
    QCOMPARE(UBTextVAlign::verticalOffset(UBTextVAlign::Middle, 200.0, 60.0), 70.0);
}

void TestUBTextVerticalAlign::testBottomIsFullSlack()
{
    QCOMPARE(UBTextVAlign::verticalOffset(UBTextVAlign::Bottom, 200.0, 60.0), 140.0);
}

void TestUBTextVerticalAlign::testContentTallerThanBoxNoOffset()
{
    // Content taller than (or equal to) box -> never push content off the top.
    QCOMPARE(UBTextVAlign::verticalOffset(UBTextVAlign::Bottom, 60.0, 200.0), 0.0);
    QCOMPARE(UBTextVAlign::verticalOffset(UBTextVAlign::Middle, 100.0, 100.0), 0.0);
}

void TestUBTextVerticalAlign::testFromIntClampsToValid()
{
    QCOMPARE(UBTextVAlign::fromInt(0), UBTextVAlign::Top);
    QCOMPARE(UBTextVAlign::fromInt(1), UBTextVAlign::Middle);
    QCOMPARE(UBTextVAlign::fromInt(2), UBTextVAlign::Bottom);
    QCOMPARE(UBTextVAlign::fromInt(99), UBTextVAlign::Top);  // invalid -> Top
    QCOMPARE(UBTextVAlign::fromInt(-1), UBTextVAlign::Top);
}
