/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBStringUtils.h"
#include "frameworks/UBStringUtils.h"
#include <QTimeZone>

void TestUBStringUtils::testSortByLastDigit()
{
    QStringList input;
    input << "page003.svg" << "page001.svg" << "page002.svg";

    QStringList result = UBStringUtils::sortByLastDigit(input);

    QCOMPARE(result.size(), 3);
    QCOMPARE(result.at(0), QString("page001.svg"));
    QCOMPARE(result.at(1), QString("page002.svg"));
    QCOMPARE(result.at(2), QString("page003.svg"));
}

void TestUBStringUtils::testSortByLastDigit_emptyList()
{
    QStringList input;
    QStringList result = UBStringUtils::sortByLastDigit(input);
    QVERIFY(result.isEmpty());
}

void TestUBStringUtils::testSortByLastDigit_noDigits()
{
    QStringList input;
    input << "alpha.svg" << "beta.svg" << "gamma.svg";

    QStringList result = UBStringUtils::sortByLastDigit(input);

    // All items should have digit -1, sorted alphabetically
    QCOMPARE(result.size(), 3);
    QCOMPARE(result.at(0), QString("alpha.svg"));
    QCOMPARE(result.at(1), QString("beta.svg"));
    QCOMPARE(result.at(2), QString("gamma.svg"));
}

void TestUBStringUtils::testNetxDigitizedName()
{
    QString source = "Document 1";
    QString result = UBStringUtils::netxDigitizedName(source);
    QCOMPARE(result, QString("Document 2"));
}

void TestUBStringUtils::testNetxDigitizedName_noDigit()
{
    QString source = "Document";
    QString result = UBStringUtils::netxDigitizedName(source);
    QCOMPARE(result, QString("Document 1"));
}

void TestUBStringUtils::testNetxDigitizedName_withDigit()
{
    QString source = "Page 99";
    QString result = UBStringUtils::netxDigitizedName(source);
    QCOMPARE(result, QString("Page 100"));
}

void TestUBStringUtils::testToCanonicalUuid()
{
    QUuid uuid("{12345678-1234-1234-1234-123456789abc}");
    QString result = UBStringUtils::toCanonicalUuid(uuid);

    QVERIFY(!result.startsWith("{"));
    QVERIFY(!result.endsWith("}"));
    QCOMPARE(result, QString("12345678-1234-1234-1234-123456789abc"));
}

void TestUBStringUtils::testToCanonicalUuid_noBraces()
{
    // QUuid::toString() always adds braces, so this tests the removal
    QUuid uuid = QUuid::createUuid();
    QString result = UBStringUtils::toCanonicalUuid(uuid);

    QVERIFY(!result.contains("{"));
    QVERIFY(!result.contains("}"));
    QCOMPARE(result.length(), 36); // UUID without braces is 36 chars
}

void TestUBStringUtils::testToUtcIsoDateTime()
{
    QDateTime dt(QDate(2013, 6, 15), QTime(10, 30, 0), QTimeZone::UTC);
    QString result = UBStringUtils::toUtcIsoDateTime(dt);

    QVERIFY(result.endsWith("Z"));
    QVERIFY(result.contains("2013-06-15"));
    QVERIFY(result.contains("10:30:00"));
}

void TestUBStringUtils::testFromUtcIsoDate()
{
    QString dateStr = "2013-06-15T10:30:00Z";
    QDateTime result = UBStringUtils::fromUtcIsoDate(dateStr);

    QVERIFY(result.isValid());
    // The result is converted to local time, so we convert back to UTC for comparison
    QDateTime utcResult = result.toUTC();
    QCOMPARE(utcResult.date(), QDate(2013, 6, 15));
    QCOMPARE(utcResult.time(), QTime(10, 30, 0));
}

void TestUBStringUtils::testFromUtcIsoDate_roundtrip()
{
    QDateTime original(QDate(2024, 1, 15), QTime(14, 45, 30), QTimeZone::UTC);
    QString isoStr = UBStringUtils::toUtcIsoDateTime(original);
    QDateTime restored = UBStringUtils::fromUtcIsoDate(isoStr);

    QCOMPARE(restored.toUTC(), original);
}


// #236 — the version displayed in Preferences comes from
// UBApplication::applicationVersion(), which now normalises the raw build
// version via UBStringUtils::cleanVersion. These cases pin the expected output.

void TestUBStringUtils::testCleanVersion_stripsLeadingV()
{
    // git describe on an exact tag returns "v4.3.0"; the user must see "4.3.0".
    QCOMPARE(UBStringUtils::cleanVersion("v4.3.0"), QString("4.3.0"));
}

void TestUBStringUtils::testCleanVersion_stripsTrailingDot()
{
    // LONG_VERSION with an empty SVN_VERSION yields a trailing dot.
    QCOMPARE(UBStringUtils::cleanVersion("4.3.0."), QString("4.3.0"));
}

void TestUBStringUtils::testCleanVersion_tagAndTrailingDot()
{
    QCOMPARE(UBStringUtils::cleanVersion("v4.3.0."), QString("4.3.0"));
}

void TestUBStringUtils::testCleanVersion_devFallback()
{
    // Untagged/dev builds: the "-dev" suffix must survive, only the dot goes.
    QCOMPARE(UBStringUtils::cleanVersion("0.0.0-dev."), QString("0.0.0-dev"));
}

void TestUBStringUtils::testCleanVersion_alreadyClean()
{
    QCOMPARE(UBStringUtils::cleanVersion("4.3.0"), QString("4.3.0"));
}

void TestUBStringUtils::testCleanVersion_commitHashUnchanged()
{
    // A bare commit hash (git describe --always fallback) has no leading v and
    // no trailing dot, so it passes through unchanged. cleanVersion cannot
    // recover a number here — that is prevented upstream by deriving from
    // UBVERSION (the .pro/tag) rather than git describe.
    QCOMPARE(UBStringUtils::cleanVersion("8764929e"), QString("8764929e"));
}
