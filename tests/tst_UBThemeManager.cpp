/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBThemeManager.h"

#include "qml/UBThemeManager.h"
#include "gui/UBKeyboardPaletteColors.h"  // reuse WCAG contrastRatio()

using UBKeyboardColors::contrastRatio;

void TestUBThemeManager::testCssFormatsRgbAndArgb()
{
    // Opaque -> #RRGGBB
    QCOMPARE(UBThemeManager::css(QColor(0x4A, 0x90, 0xD9)), QStringLiteral("#4a90d9"));
    // Translucent -> #AARRGGBB (alpha first)
    QColor translucent(42, 42, 42, 220);
    QCOMPARE(UBThemeManager::css(translucent), QStringLiteral("#dc2a2a2a"));
}

void TestUBThemeManager::testErrorRoleContrastBothThemes()
{
    auto* tm = UBThemeManager::instance();

    tm->setCurrentTheme("light");
    QVERIFY2(contrastRatio(tm->onError(), tm->error()) >= 3.0,
             "light error text/background contrast too low");

    tm->setCurrentTheme("dark");
    QVERIFY2(contrastRatio(tm->onError(), tm->error()) >= 3.0,
             "dark error text/background contrast too low");
}

void TestUBThemeManager::testTooltipRoleContrastBothThemes()
{
    auto* tm = UBThemeManager::instance();

    tm->setCurrentTheme("light");
    QVERIFY2(contrastRatio(tm->onTooltip(), tm->tooltipBase()) >= 4.5,
             "light tooltip text/background contrast below WCAG AA");

    tm->setCurrentTheme("dark");
    QVERIFY2(contrastRatio(tm->onTooltip(), tm->tooltipBase()) >= 4.5,
             "dark tooltip text/background contrast below WCAG AA");
}

void TestUBThemeManager::testThemeSwitchChangesRoles()
{
    auto* tm = UBThemeManager::instance();

    tm->setCurrentTheme("light");
    const QColor lightSurface = tm->surface();
    const QColor lightOnSurface = tm->onSurface();

    tm->setCurrentTheme("dark");
    QVERIFY(tm->surface() != lightSurface);
    QVERIFY(tm->onSurface() != lightOnSurface);
    QVERIFY(tm->isDark());
    // Note: tooltip is intentionally a dark modern look in BOTH themes (#297),
    // so it is not asserted to differ across themes here.
}
