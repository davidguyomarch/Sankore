/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBHyperlink.h"
#include "domain/UBHyperlinkUtils.h"

void TestUBHyperlink::testBareHostGetsHttps()
{
    QCOMPARE(UBHyperlink::normalizeUrl("example.com"),
             QString("https://example.com"));
    QCOMPARE(UBHyperlink::normalizeUrl("www.sankore.org/page"),
             QString("https://www.sankore.org/page"));
}

void TestUBHyperlink::testExistingSchemeUntouched()
{
    QCOMPARE(UBHyperlink::normalizeUrl("http://example.com"),
             QString("http://example.com"));
    QCOMPARE(UBHyperlink::normalizeUrl("https://example.com"),
             QString("https://example.com"));
    QCOMPARE(UBHyperlink::normalizeUrl("ftp://files.example.com"),
             QString("ftp://files.example.com"));
}

void TestUBHyperlink::testMailtoUntouched()
{
    QCOMPARE(UBHyperlink::normalizeUrl("mailto:teacher@example.com"),
             QString("mailto:teacher@example.com"));
}

void TestUBHyperlink::testTrimmed()
{
    QCOMPARE(UBHyperlink::normalizeUrl("  example.com  "),
             QString("https://example.com"));
}

void TestUBHyperlink::testEmptyStaysEmpty()
{
    QCOMPARE(UBHyperlink::normalizeUrl(""), QString(""));
    QCOMPARE(UBHyperlink::normalizeUrl("   "), QString(""));
}
