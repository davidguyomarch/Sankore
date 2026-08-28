/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBVersion.h"
#include "frameworks/UBVersion.h"

void TestUBVersion::testDefaultConstructor()
{
    UBVersion v;
    QVERIFY(!v.isValid());
    QCOMPARE(v.toString(), QString("INVALID"));
}

void TestUBVersion::testValidVersionTwoParts()
{
    UBVersion v("4.1");
    QVERIFY(v.isValid());
    QCOMPARE(v.platformNumber(), 4);
    QCOMPARE(v.majorNumber(), 1);
    QCOMPARE(v.minorNumber(), 0);
    QCOMPARE(v.releaseStage(), ReleaseCandidate);
}

void TestUBVersion::testValidVersionThreeParts()
{
    UBVersion v("4.7.3");
    QVERIFY(v.isValid());
    QCOMPARE(v.platformNumber(), 4);
    QCOMPARE(v.majorNumber(), 7);
    QCOMPARE(v.minorNumber(), 3);
    QCOMPARE(v.releaseStage(), ReleaseCandidate);
}

void TestUBVersion::testValidVersionAlpha()
{
    UBVersion v("4.1.a.5");
    QVERIFY(v.isValid());
    QCOMPARE(v.platformNumber(), 4);
    QCOMPARE(v.majorNumber(), 1);
    QCOMPARE(v.releaseStage(), Alpha);
    QCOMPARE(v.minorNumber(), 5);
}

void TestUBVersion::testValidVersionBeta()
{
    UBVersion v("4.1.b.2");
    QVERIFY(v.isValid());
    QCOMPARE(v.platformNumber(), 4);
    QCOMPARE(v.majorNumber(), 1);
    QCOMPARE(v.releaseStage(), Beta);
    QCOMPARE(v.minorNumber(), 2);
}

void TestUBVersion::testValidVersionRC()
{
    UBVersion v("4.1.r.1");
    QVERIFY(v.isValid());
    QCOMPARE(v.platformNumber(), 4);
    QCOMPARE(v.majorNumber(), 1);
    QCOMPARE(v.releaseStage(), ReleaseCandidate);
    QCOMPARE(v.minorNumber(), 1);
}

void TestUBVersion::testInvalidVersion()
{
    UBVersion v1("invalid");
    QVERIFY(!v1.isValid());

    UBVersion v2("4");
    QVERIFY(!v2.isValid());

    UBVersion v3("");
    QVERIFY(!v3.isValid());

    UBVersion v4("abc.def");
    QVERIFY(!v4.isValid());
}

void TestUBVersion::testComparisonLessThan()
{
    UBVersion v1("4.1.2");
    UBVersion v2("4.1.3");
    QVERIFY(v1 < v2);
    QVERIFY(!(v2 < v1));
}

void TestUBVersion::testComparisonGreaterThan()
{
    UBVersion v1("4.2.0");
    UBVersion v2("4.1.9");
    QVERIFY(v1 > v2);
    QVERIFY(!(v2 > v1));
}

void TestUBVersion::testComparisonEqual()
{
    UBVersion v1("4.1.3");
    UBVersion v2("4.1.3");
    QVERIFY(v1 == v2);
    QVERIFY(!(v1 < v2));
    QVERIFY(!(v1 > v2));
}

void TestUBVersion::testComparisonDifferentPlatform()
{
    UBVersion v1("3.9.9");
    UBVersion v2("4.0.0");
    QVERIFY(v1 < v2);
    QVERIFY(v2 > v1);
}

void TestUBVersion::testComparisonDifferentMajor()
{
    UBVersion v1("4.1.9");
    UBVersion v2("4.2.0");
    QVERIFY(v1 < v2);
}

void TestUBVersion::testComparisonDifferentStage()
{
    UBVersion v1("4.1.a.5");
    UBVersion v2("4.1.b.5");
    UBVersion v3("4.1.r.5");
    QVERIFY(v1 < v2);
    QVERIFY(v2 < v3);
    QVERIFY(v1 < v3);
}

void TestUBVersion::testToString()
{
    UBVersion v("4.7.3");
    QCOMPARE(v.toString(), QString("4.7.3"));
}

void TestUBVersion::testToStringInvalid()
{
    UBVersion v;
    QCOMPARE(v.toString(), QString("INVALID"));
}
