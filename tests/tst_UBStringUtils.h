/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBSTRINGUTILS_H
#define TST_UBSTRINGUTILS_H

#include <QtTest>
#include <QObject>

class TestUBStringUtils : public QObject
{
    Q_OBJECT

private slots:
    void testSortByLastDigit();
    void testSortByLastDigit_emptyList();
    void testSortByLastDigit_noDigits();
    void testNetxDigitizedName();
    void testNetxDigitizedName_noDigit();
    void testNetxDigitizedName_withDigit();
    void testToCanonicalUuid();
    void testToCanonicalUuid_noBraces();
    void testToUtcIsoDateTime();
    void testFromUtcIsoDate();
    void testFromUtcIsoDate_roundtrip();
    // #236 — version shown in Preferences must be a clean number
    void testCleanVersion_stripsLeadingV();
    void testCleanVersion_stripsTrailingDot();
    void testCleanVersion_tagAndTrailingDot();
    void testCleanVersion_devFallback();
    void testCleanVersion_alreadyClean();
    void testCleanVersion_commitHashUnchanged();
};

#endif // TST_UBSTRINGUTILS_H
