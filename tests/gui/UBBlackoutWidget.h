/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: lightweight gui/UBBlackoutWidget.h for test builds (issue #244).
// Mirrors the real widget's signal/slot surface that UBDisplayManager connects
// to (activity() / doActivity()). QObject, so its moc is pre-generated into
// tests/premoc (see scripts/docker-build.sh + tests.pro).
#ifndef UBBLACKOUTWIDGET_H_
#define UBBLACKOUTWIDGET_H_

#include <QWidget>

class UBBlackoutWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UBBlackoutWidget(QWidget* parent = nullptr) : QWidget(parent) {}

signals:
    void activity();

public slots:
    void doActivity() { emit activity(); }
};

#endif /* UBBLACKOUTWIDGET_H_ */
