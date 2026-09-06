/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBHYPERLINK_H
#define TST_UBHYPERLINK_H

#include <QObject>
#include <QTest>

// #280 — hyperlink URL normalization for the text tool.
class TestUBHyperlink : public QObject
{
    Q_OBJECT

private slots:
    void testBareHostGetsHttps();
    void testExistingSchemeUntouched();
    void testMailtoUntouched();
    void testTrimmed();
    void testEmptyStaysEmpty();
};

#endif // TST_UBHYPERLINK_H
