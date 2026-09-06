/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBSMOOTHSTROKEITEM_H
#define TST_UBSMOOTHSTROKEITEM_H

#include <QObject>
#include <QTest>

class TestUBSmoothStrokeItem : public QObject
{
    Q_OBJECT

private slots:
    // addPoint + rebuildPath
    void testAddPoint_singlePoint();
    void testAddPoint_twoPoints();
    void testAddPoint_multiplePoints_bezierCurves();
    void testAddPoint_duplicateSkipped();

    // finalize
    void testFinalize_marksComplete();

    // Properties
    void testSetStrokeWidth();
    void testSetStrokeColor();
    void testColorOnBackgrounds();

    // subtractPath
    void testSubtractPath_partialErase();
    void testSubtractPath_fullErase();

    // setLastPoint (Line tool rubber-band)
    void testSetLastPoint_replaceEndpoint();
    void testSetLastPoint_onSinglePoint();

    // boundingRect
    void testBoundingRect_expandedForSoftEdge();

    // deepCopy
    void testDeepCopy();

    // type
    void testType();

    // #243 — a drawn stroke must own a delegate, otherwise selecting it
    // dereferences a null Delegate() in UBBoardView and crashes.
    void testHasDelegate_regression243();
};

#endif // TST_UBSMOOTHSTROKEITEM_H
