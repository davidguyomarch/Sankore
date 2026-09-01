/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBGRAPHICSSTROKE_H
#define TST_UBGRAPHICSSTROKE_H

#include <QtTest>
#include <QObject>

class TestUBGraphicsStroke : public QObject
{
    Q_OBJECT

private slots:
    void testEmptyStroke();
    void testAddPolygon();
    void testAddPolygon_deduplicates();
    void testRemove();
    void testRemove_absentPolygon();
    void testClear();
    void testHasPressure_falseWhenUniform();
    void testHasPressure_trueWhenWidthVaries();
    void testHasPressure_trueWhenNotNominal();
    void testHasPressure_falseWhenTooFewPolygons();
    void testHasAlpha_falseWhenOpaque();
    void testHasAlpha_trueWhenTransparent();
    void testHasAlpha_falseWhenEmpty();
    void testDeepCopy();
};

#endif // TST_UBGRAPHICSSTROKE_H
