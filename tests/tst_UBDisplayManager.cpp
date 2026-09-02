/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Regression tests for issue #244:
 *   "Multi-screen spans all displays at startup; should default to single-screen"
 *
 * The startup span is caused by UBDisplayManager defaulting to multi-screen
 * regardless of the (mis-keyed, true-defaulting) appUseMultiscreen setting.
 * These tests pin the intended behaviour: a freshly constructed manager honours
 * the setting, which defaults to single-screen (false).
 *
 * UBDisplayManager is compiled in isolation via stubs/UBDisplayManager_testable.cpp
 * with lightweight shims for its heavy dependencies (see that wrapper's comment).
 */

#include "tst_UBDisplayManager.h"

#include "stubs/UBSettings_stub.h"
#include "core/UBDisplayManager.h"

#include <QGuiApplication>
#include <QScreen>

void TestUBDisplayManager::initTestCase()
{
    // Ensure the shared stub settings singleton exists.
    UBSettings::settings();
}

void TestUBDisplayManager::cleanupTestCase()
{
    UBSettings::destroy();
}

// The core of #244: with the setting at its default (single-screen), a freshly
// constructed manager must NOT be in multi-screen mode. On the buggy code the
// constructor hard-set mUseMultiScreen = true, so this failed.
void TestUBDisplayManager::testDefaultsToSingleScreen()
{
    UBSettings* settings = UBSettings::settings();
    settings->appUseMultiscreen->set(QVariant(false));

    UBDisplayManager manager;
    QCOMPARE(manager.useMultiScreen(), false);
}

// When the setting is explicitly enabled, the manager must pick it up at
// construction time (drives the "only span when explicitly enabled" behaviour).
void TestUBDisplayManager::testSettingTrueEnablesMultiScreen()
{
    UBSettings* settings = UBSettings::settings();
    settings->appUseMultiscreen->set(QVariant(true));

    UBDisplayManager manager;
    QCOMPARE(manager.useMultiScreen(), true);

    // Restore the default for any following test.
    settings->appUseMultiscreen->set(QVariant(false));
}

// The runtime toggle (Preferences / multi-screen action) must flip the state.
void TestUBDisplayManager::testSetUseMultiScreenTogglesState()
{
    UBSettings* settings = UBSettings::settings();
    settings->appUseMultiscreen->set(QVariant(false));

    UBDisplayManager manager;
    QCOMPARE(manager.useMultiScreen(), false);

    manager.setUseMultiScreen(true);
    QCOMPARE(manager.useMultiScreen(), true);

    manager.setUseMultiScreen(false);
    QCOMPARE(manager.useMultiScreen(), false);
}

// In single-screen mode the manager reports exactly one usable screen,
// regardless of how many physical displays are attached.
void TestUBDisplayManager::testSingleScreenReportsOneScreen()
{
    UBSettings* settings = UBSettings::settings();
    settings->appUseMultiscreen->set(QVariant(false));

    UBDisplayManager manager;
    QCOMPARE(manager.numScreens(), 1);

    // In multi-screen mode it reports the actual attached screen count.
    manager.setUseMultiScreen(true);
    QCOMPARE(manager.numScreens(), QGuiApplication::screens().count());

    settings->appUseMultiscreen->set(QVariant(false));
}
