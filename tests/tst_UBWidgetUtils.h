/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBWIDGETUTILS_H
#define TST_UBWIDGETUTILS_H

#include <QtTest>
#include <QObject>

class TestUBWidgetUtils : public QObject
{
    Q_OBJECT

private slots:
    void testNullTreeReturnsZero();
    void testNullItemReturnsZero();
    void testCollapsedItemIgnoresChildren();
    void testExpandedItemSumsChildren();
};

#endif // TST_UBWIDGETUTILS_H
