/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBTHEMEMANAGER_H
#define UBTHEMEMANAGER_H

#include <QObject>
#include <QColor>
#include <QQmlEngine>

/**
 * @brief QML-accessible theme manager singleton.
 *
 * Exposes the current theme colors as bindable properties.
 * When the theme changes, all QML bindings update automatically.
 *
 * Usage in QML:
 *   import Sankore 1.0
 *   Rectangle { color: ThemeManager.surface }
 */
class UBThemeManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Surface colors
    Q_PROPERTY(QColor surface READ surface NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceVariant READ surfaceVariant NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceHover READ surfaceHover NOTIFY themeChanged)

    // Content colors
    Q_PROPERTY(QColor onSurface READ onSurface NOTIFY themeChanged)
    Q_PROPERTY(QColor onSurfaceDim READ onSurfaceDim NOTIFY themeChanged)

    // Accent
    Q_PROPERTY(QColor primary READ primary NOTIFY themeChanged)
    Q_PROPERTY(QColor onPrimary READ onPrimary NOTIFY themeChanged)

    // Borders
    Q_PROPERTY(QColor border READ border NOTIFY themeChanged)

    // State
    Q_PROPERTY(bool isDark READ isDark NOTIFY themeChanged)
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY themeChanged)

public:
    static UBThemeManager* instance();
    static UBThemeManager* create(QQmlEngine*, QJSEngine*) { return instance(); }

    // Theme switching
    QString currentTheme() const { return mCurrentTheme; }
    void setCurrentTheme(const QString& theme);
    bool isDark() const { return mCurrentTheme == "dark"; }

    // Color accessors
    QColor surface() const { return mSurface; }
    QColor surfaceVariant() const { return mSurfaceVariant; }
    QColor surfaceHover() const { return mSurfaceHover; }
    QColor onSurface() const { return mOnSurface; }
    QColor onSurfaceDim() const { return mOnSurfaceDim; }
    QColor primary() const { return mPrimary; }
    QColor onPrimary() const { return mOnPrimary; }
    QColor border() const { return mBorder; }

signals:
    void themeChanged();

private:
    explicit UBThemeManager(QObject* parent = nullptr);
    void applyDarkTheme();
    void applyLightTheme();

    static UBThemeManager* sInstance;

    QString mCurrentTheme;
    QColor mSurface;
    QColor mSurfaceVariant;
    QColor mSurfaceHover;
    QColor mOnSurface;
    QColor mOnSurfaceDim;
    QColor mPrimary;
    QColor mOnPrimary;
    QColor mBorder;
};

#endif // UBTHEMEMANAGER_H
