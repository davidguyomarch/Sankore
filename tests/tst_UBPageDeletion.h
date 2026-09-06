/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBPAGEDELETION_H
#define TST_UBPAGEDELETION_H

#include <QObject>
#include <QTest>

// #276 — deleting a page (with shapes + strokes) crashed because deleting the
// first page computed a new active-scene index of -1 (nIndex - 1), which
// setActiveDocumentScene did not clamp, leaving a dangling active scene.
// These tests pin the pure index math used to pick the new active page.
class TestUBPageDeletion : public QObject
{
    Q_OBJECT

private slots:
    void testDeleteFirstPage_staysAtZero();
    void testDeleteMiddlePage_staysAtSameIndex();
    void testDeleteLastPage_movesToNewLast();
    void testNeverNegative();
    void testNeverBeyondNewLast();
};

#endif // TST_UBPAGEDELETION_H
