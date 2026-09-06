/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBBACKGROUNDGRID_H
#define TST_UBBACKGROUNDGRID_H

#include <QObject>
#include <QTest>

// #289 — French school rulings (Séyès, enlarged Séyès, double 3mm) as page
// backgrounds. These tests pin the pure line-geometry generation so the
// millimetre spacing, weights and margin line stay correct.
class TestUBBackgroundGrid : public QObject
{
    Q_OBJECT

private slots:
    void testPlainProducesNoLines();
    void testTypeTokenRoundTrip();
    void testTypeIntRoundTrip();
    void testIsRuled();
    void testGridIsUniform8mm();
    void testSeyesInterlineSpacing();
    void testSeyesHasVerticalAndMargin();
    void testSeyesLargeIsScaled();
    void testDoubleLine3mmPairs();
};

#endif // TST_UBBACKGROUNDGRID_H
