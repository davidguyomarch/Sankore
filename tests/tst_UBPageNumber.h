/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBPAGENUMBER_H
#define TST_UBPAGENUMBER_H

#include <QObject>
#include <QTest>

// #357 — scene-index (0-based) <-> page-number (1-based) conversion.
class TestUBPageNumber : public QObject
{
    Q_OBJECT

private slots:
    void testSceneIndexToPageNumber();
    void testPageNumberToSceneIndex();
    void testRoundTrip();
};

#endif // TST_UBPAGENUMBER_H
