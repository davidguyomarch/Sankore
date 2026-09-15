/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBTHEMEMANAGER_H
#define TST_UBTHEMEMANAGER_H

#include <QObject>
#include <QtTest>

/**
 * @brief Tests for UBThemeManager semantic roles and the css() helper (#297).
 *
 * #297 centralizes all UI colors in UBThemeManager. These tests pin the
 * contract of the newly added semantic roles (disabled / error / tooltip) and
 * the QWidget-stylesheet helper css(), so widgets can rely on them:
 *  - css() formats opaque colors as #RRGGBB and translucent as #AARRGGBB;
 *  - error and tooltip roles keep a readable text/background contrast (WCAG)
 *    in both light and dark themes;
 *  - switching theme actually changes the role colors.
 */
class TestUBThemeManager : public QObject
{
    Q_OBJECT

private slots:
    void testCssFormatsRgbAndArgb();
    void testErrorRoleContrastBothThemes();
    void testTooltipRoleContrastBothThemes();
    void testThemeSwitchChangesRoles();
};

#endif // TST_UBTHEMEMANAGER_H
