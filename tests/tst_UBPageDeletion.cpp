/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBPageDeletion.h"
#include "board/UBPageDeletionMath.h"

// Convention: after deleting page `deletedIndex` from a document that had
// `pageCountBefore` pages, we stay at the same slot (which now shows the page
// that followed the deleted one); if we deleted the last page, we move to the
// new last page. The result must always be a valid index in [0, newCount-1].

void TestUBPageDeletion::testDeleteFirstPage_staysAtZero()
{
    // 3 pages, delete index 0 -> new count 2, active index 0 (not -1, the #276 bug)
    QCOMPARE(ubActiveIndexAfterPageDeletion(0, 3), 0);
}

void TestUBPageDeletion::testDeleteMiddlePage_staysAtSameIndex()
{
    // 4 pages, delete index 1 -> new count 3, stay at index 1 (shows old page 2)
    QCOMPARE(ubActiveIndexAfterPageDeletion(1, 4), 1);
}

void TestUBPageDeletion::testDeleteLastPage_movesToNewLast()
{
    // 3 pages, delete index 2 (last) -> new count 2, active index 1 (new last)
    QCOMPARE(ubActiveIndexAfterPageDeletion(2, 3), 1);
}

void TestUBPageDeletion::testNeverNegative()
{
    // Deleting the only-ish edge cases must never yield a negative index.
    QVERIFY(ubActiveIndexAfterPageDeletion(0, 2) >= 0);
    QVERIFY(ubActiveIndexAfterPageDeletion(0, 1) >= 0);
}

void TestUBPageDeletion::testNeverBeyondNewLast()
{
    // Result must never exceed the new last index (pageCountBefore - 2).
    for (int count = 2; count <= 10; ++count)
        for (int del = 0; del < count; ++del)
        {
            int idx = ubActiveIndexAfterPageDeletion(del, count);
            QVERIFY(idx >= 0);
            QVERIFY(idx <= count - 2); // new last index after removing one page
        }
}
