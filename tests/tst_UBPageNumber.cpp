/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBPageNumber.h"
#include "domain/UBPageNumberUtils.h"

// The first scene (index 0) is page 1 for the user — this is the #357 fix:
// the "Go to page" picker previously showed the raw 0-based index.
void TestUBPageNumber::testSceneIndexToPageNumber()
{
    QCOMPARE(UBPageNumber::sceneIndexToPageNumber(0), 1);
    QCOMPARE(UBPageNumber::sceneIndexToPageNumber(1), 2);
    QCOMPARE(UBPageNumber::sceneIndexToPageNumber(9), 10);
}

// A user picking "page 1" must navigate to scene index 0, not 1 (the old
// off-by-one: picking "1" jumped to the second page).
void TestUBPageNumber::testPageNumberToSceneIndex()
{
    QCOMPARE(UBPageNumber::pageNumberToSceneIndex(1), 0);
    QCOMPARE(UBPageNumber::pageNumberToSceneIndex(2), 1);
    QCOMPARE(UBPageNumber::pageNumberToSceneIndex(10), 9);
}

void TestUBPageNumber::testRoundTrip()
{
    for (int sceneIndex = 0; sceneIndex < 25; ++sceneIndex) {
        const int page = UBPageNumber::sceneIndexToPageNumber(sceneIndex);
        QCOMPARE(UBPageNumber::pageNumberToSceneIndex(page), sceneIndex);
    }
}
