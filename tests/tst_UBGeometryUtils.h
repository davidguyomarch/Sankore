/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBGEOMETRYUTILS_H
#define TST_UBGEOMETRYUTILS_H

#include <QtTest>
#include <QObject>

class TestUBGeometryUtils : public QObject
{
    Q_OBJECT

private slots:
    // lineToPolygon (uniform width)
    void testLineToPolygon_horizontal();
    void testLineToPolygon_vertical();
    void testLineToPolygon_diagonal();
    void testLineToPolygon_zeroWidth();

    // lineToPolygon (variable width)
    void testLineToPolygon_variableWidth();

    // lineToInnerRect
    void testLineToInnerRect_basic();
    void testLineToInnerRect_centered();

    // pointConstrainedInRect (QPointF)
    void testPointConstrainedInRect_inside();
    void testPointConstrainedInRect_outside();
    void testPointConstrainedInRect_onEdge();

    // pointConstrainedInRect (QPoint)
    void testPointConstrainedInRectInt_inside();
    void testPointConstrainedInRectInt_outside();

    // crashPointList
    void testCrashPointList_noDuplicates();
    void testCrashPointList_withDuplicates();
    void testCrashPointList_allSame();
    void testCrashPointList_empty();

    // arcToPolygon
    void testArcToPolygon_basic();
    void testArcToPolygon_negativeAngle();
    void testArcToPolygon_fullCircle();

    // Constants
    void testConstants();
};

#endif // TST_UBGEOMETRYUTILS_H
