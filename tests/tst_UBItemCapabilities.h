/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBITEMCAPABILITIES_H
#define TST_UBITEMCAPABILITIES_H

#include <QObject>
#include <QtTest>

/**
 * @brief Tests for UBItemMenu::baseMenuEntries (ADR-0010 / #454).
 *
 * The pure capability -> ordered-menu-entries mapping that the item delegate
 * uses to compose the base "…" context menu. Verifies the fixed entries, the
 * flag-gated entries, and the exact order, for the per-type capability profiles
 * of the current (unchanged) behaviour.
 */
class TestUBItemCapabilities : public QObject
{
    Q_OBJECT

private slots:
    void testLockAndVisibleAlwaysPresent();
    void testOrderIsStable();
    void testGatedEntriesHiddenByDefault();
    void testShapeProfile();
    void testPdfProfile();
    void testImageProfile();
    void testWidgetProfile();
};

#endif // TST_UBITEMCAPABILITIES_H
