/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBTEXTVERTICALALIGN_H
#define TST_UBTEXTVERTICALALIGN_H

#include <QObject>
#include <QTest>

// #278 — vertical alignment of text within its box (top / middle / bottom).
class TestUBTextVerticalAlign : public QObject
{
    Q_OBJECT

private slots:
    void testTopIsZeroOffset();
    void testMiddleIsHalfSlack();
    void testBottomIsFullSlack();
    void testContentTallerThanBoxNoOffset();
    void testFromIntClampsToValid();
};

#endif // TST_UBTEXTVERTICALALIGN_H
