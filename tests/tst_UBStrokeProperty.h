/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBSTROKEPROPERTY_H
#define TST_UBSTROKEPROPERTY_H

#include <QtTest>
#include <QObject>

class TestUBStrokeProperty : public QObject
{
    Q_OBJECT

private slots:
    void testDefaults();
    void testSetters();
    void testCopyConstructor();
    void testStrokeAppliesToPainterPen();
};

#endif // TST_UBSTROKEPROPERTY_H
