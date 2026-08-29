/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Unit tests for UBGraphicsScene drawing pipeline.
 * Uses UBSceneContext injection — no singletons needed.
 */

#ifndef TST_UBGRAPHICSSCENE_H
#define TST_UBGRAPHICSSCENE_H

#include <QObject>
#include <QTest>

class TestUBGraphicsScene : public QObject
{
    Q_OBJECT

private slots:
    void testDrawLineToCreatesPolygon();
    void testDrawLineToZeroLengthSkipped();
    void testMoveToResetsState();
    void testInputDevicePressAndRelease();
    void testMultipleSegmentsCreateStroke();
};

#endif // TST_UBGRAPHICSSCENE_H
