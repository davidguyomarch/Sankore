/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: minimal generated ui_blackoutWidget.h for test builds (issue #244).
// UBDisplayManager::blackout() only touches iconButton, labelClickToReturn and
// setupUi(); the real generated form (layouts, palettes) is irrelevant to the
// screen-count logic under test. Shadows the build/*/ui/ui_blackoutWidget.h.
#ifndef UI_BLACKOUTWIDGET_H
#define UI_BLACKOUTWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>

QT_BEGIN_NAMESPACE

namespace Ui {

class BlackoutWidget
{
public:
    QToolButton* iconButton = nullptr;
    QLabel* labelClickToReturn = nullptr;

    void setupUi(QWidget* parent)
    {
        iconButton = new QToolButton(parent);
        labelClickToReturn = new QLabel(parent);
    }
};

} // namespace Ui

QT_END_NAMESPACE

#endif /* UI_BLACKOUTWIDGET_H */
