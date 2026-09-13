/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBThemeManager.h"

UBThemeManager* UBThemeManager::sInstance = nullptr;

UBThemeManager* UBThemeManager::instance()
{
    if (!sInstance)
        sInstance = new UBThemeManager();
    return sInstance;
}

UBThemeManager::UBThemeManager(QObject* parent)
    : QObject(parent)
{
    // Default to dark
    applyDarkTheme();
    mCurrentTheme = "dark";
}

void UBThemeManager::setCurrentTheme(const QString& theme)
{
    if (mCurrentTheme == theme)
        return;

    mCurrentTheme = theme;

    if (theme == "light" || theme == "classic")
        applyLightTheme();
    else
        applyDarkTheme();

    emit themeChanged();
}

void UBThemeManager::applyDarkTheme()
{
    mSurface = QColor(42, 42, 42, 220);       // #2A2A2ADC
    mSurfaceVariant = QColor(58, 58, 58);      // #3A3A3A
    mSurfaceHover = QColor(74, 74, 74);        // #4A4A4A
    mOnSurface = QColor(255, 255, 255);        // white
    mOnSurfaceDim = QColor(221, 221, 221);     // #DDDDDD
    mPrimary = QColor(74, 144, 217);           // #4A90D9
    mOnPrimary = QColor(255, 255, 255);        // white
    mBorder = QColor(255, 255, 255, 40);       // subtle white
    // #297: semantic roles
    mDisabled = QColor(136, 136, 136);         // #888888 muted foreground
    mError = QColor(120, 40, 60);              // desaturated red bg (dark)
    mOnError = QColor(255, 210, 220);          // light pink text on error
    mTooltipBase = QColor(58, 58, 58);         // dark tooltip bg
    mOnTooltip = QColor(255, 255, 255);        // white tooltip text
}

void UBThemeManager::applyLightTheme()
{
    mSurface = QColor(255, 255, 255, 245);     // near-white
    mSurfaceVariant = QColor(245, 245, 245);   // #F5F5F5
    mSurfaceHover = QColor(232, 232, 232);     // #E8E8E8
    mOnSurface = QColor(26, 26, 26);           // #1A1A1A
    mOnSurfaceDim = QColor(102, 102, 102);     // #666666
    mPrimary = QColor(37, 99, 235);            // #2563EB
    mOnPrimary = QColor(255, 255, 255);        // white
    mBorder = QColor(224, 224, 224);           // #E0E0E0
    // #297: semantic roles
    mDisabled = QColor(153, 153, 153);         // #999999 muted foreground
    mError = QColor(255, 179, 200);            // #FFB3C8 validation bg (kept)
    mOnError = QColor(85, 85, 85);             // #555555 text on error
    mTooltipBase = QColor(255, 255, 220);      // #FFFFDC classic tooltip (kept)
    mOnTooltip = QColor(0, 0, 0);              // black tooltip text
}

QString UBThemeManager::css(const QColor& c)
{
    // Qt stylesheets accept rgba(r,g,b,a) with a 0..255 alpha via rgba() only in
    // the 0..1 float form for the alpha; use #AARRGGBB when translucent, #RRGGBB
    // otherwise, both accepted by Qt Style Sheets.
    if (c.alpha() == 255)
        return c.name(QColor::HexRgb);       // #RRGGBB
    return c.name(QColor::HexArgb);          // #AARRGGBB
}
