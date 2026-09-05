/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBRESIZEGRIP_H
#define TST_UBRESIZEGRIP_H

#include <QObject>
#include <QTest>

// #277 — the selection frame must be resizable from anywhere along an edge,
// not just the middle. These tests pin the pure grip geometry.
class TestUBResizeGrip : public QObject
{
    Q_OBJECT

private slots:
    void testEdgeSpansFullLength();
    void testBandThickerThanFrameWidth();
    void testExtremitiesAreInsideEdge();
    void testBottomRightCornerReserved();
    void testBandsNeverOverlapOnSmallRect();
};

#endif // TST_UBRESIZEGRIP_H
