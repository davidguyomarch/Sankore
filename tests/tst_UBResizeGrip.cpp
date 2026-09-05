/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBResizeGrip.h"
#include "domain/UBResizeGripGeometry.h"

// A representative frame: 200 x 120, frameWidth 10.
static QRectF frame() { return QRectF(0, 0, 200, 120); }
static const qreal FW = 10.0;

void TestUBResizeGrip::testEdgeSpansFullLength()
{
    // Left/right edges must span the full height; top/bottom the full width.
    QCOMPARE(UBResizeGrip::leftRect(frame(), FW).height(), 120.0);
    QCOMPARE(UBResizeGrip::rightRect(frame(), FW).height(), 120.0);
    QCOMPARE(UBResizeGrip::topRect(frame(), FW).width(), 200.0);
    QCOMPARE(UBResizeGrip::bottomRect(frame(), FW).width(), 200.0);
}

void TestUBResizeGrip::testBandThickerThanFrameWidth()
{
    // The clickable band is now thicker than the old single frameWidth,
    // making the edge easy to grab (2x here since the frame is large enough).
    QCOMPARE(UBResizeGrip::leftRect(frame(), FW).width(), 2.0 * FW);
    QCOMPARE(UBResizeGrip::topRect(frame(), FW).height(), 2.0 * FW);
}

void TestUBResizeGrip::testExtremitiesAreInsideEdge()
{
    // #277 core: a point near the very top of the left edge (which the old
    // geometry excluded via `top + frameWidth`) must now be inside the left band.
    const QRectF r = frame();
    QVERIFY(UBResizeGrip::leftRect(r, FW).contains(QPointF(r.left() + 1, r.top() + 2)));
    // ... and near the bottom of the left edge too.
    QVERIFY(UBResizeGrip::leftRect(r, FW).contains(QPointF(r.left() + 1, r.bottom() - 2)));
    // Same for the top edge near its left/right extremities.
    QVERIFY(UBResizeGrip::topRect(r, FW).contains(QPointF(r.left() + 2, r.top() + 1)));
    QVERIFY(UBResizeGrip::topRect(r, FW).contains(QPointF(r.right() - 2, r.top() + 1)));
}

void TestUBResizeGrip::testBottomRightCornerReserved()
{
    // The bottom-right corner keeps a dedicated frameWidth square for diagonal
    // resize (toolFromPos tests it first).
    const QRectF r = frame();
    const QRectF br = UBResizeGrip::bottomRightRect(r, FW);
    QCOMPARE(br.width(), FW);
    QCOMPARE(br.height(), FW);
    QVERIFY(br.contains(QPointF(r.right() - 1, r.bottom() - 1)));
}

void TestUBResizeGrip::testBandsNeverOverlapOnSmallRect()
{
    // On a small frame, the band is capped so left and right bands stay
    // distinct (don't cover the whole width).
    const QRectF small(0, 0, 24, 24); // width 24, frameWidth 10 -> 2x would be 20 (>half)
    const qreal lw = UBResizeGrip::leftRect(small, FW).width();
    const qreal rw = UBResizeGrip::rightRect(small, FW).width();
    QVERIFY(lw + rw < small.width());   // must not span the full width
    QVERIFY(lw >= FW);                  // but never thinner than the old band
}
