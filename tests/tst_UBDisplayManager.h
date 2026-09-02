/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBDISPLAYMANAGER_H
#define TST_UBDISPLAYMANAGER_H

#include <QObject>
#include <QtTest/QtTest>

class TestUBDisplayManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Issue #244: multi-screen must default to single-screen at startup.
    void testDefaultsToSingleScreen();
    void testSettingTrueEnablesMultiScreen();
    void testSetUseMultiScreenTogglesState();
    void testSingleScreenReportsOneScreen();
};

#endif // TST_UBDISPLAYMANAGER_H
