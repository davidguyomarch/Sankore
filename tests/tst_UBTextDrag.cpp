/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBTextDrag.h"
#include "domain/UBTextDragMath.h"

static const int THRESHOLD = 8; // matches QApplication::setStartDragDistance(8)

void TestUBTextDrag::testSmallMoveIsNotDrag()
{
    // A tiny move (below threshold) is a click -> edit, not a box drag.
    QVERIFY(!UBTextDrag::shouldDragBox(QPointF(100, 100), QPointF(103, 102),
                                       THRESHOLD, /*alreadyDragging*/false));
}

void TestUBTextDrag::testMovePastThresholdIsDrag()
{
    // Moving well past the threshold starts a box drag.
    QVERIFY(UBTextDrag::shouldDragBox(QPointF(100, 100), QPointF(120, 100),
                                      THRESHOLD, false));
    // Manhattan distance combines both axes.
    QVERIFY(UBTextDrag::shouldDragBox(QPointF(100, 100), QPointF(105, 105),
                                      THRESHOLD, false));
}

void TestUBTextDrag::testLatchedDragStaysDrag()
{
    // Once dragging, a brief dip back under the threshold keeps dragging.
    QVERIFY(UBTextDrag::shouldDragBox(QPointF(100, 100), QPointF(101, 100),
                                      THRESHOLD, /*alreadyDragging*/true));
}

void TestUBTextDrag::testExactThresholdIsDrag()
{
    // At exactly the threshold it counts as a drag (>=).
    QVERIFY(UBTextDrag::shouldDragBox(QPointF(0, 0), QPointF(THRESHOLD, 0),
                                      THRESHOLD, false));
}
