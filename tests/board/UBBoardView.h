/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: lightweight board/UBBoardView.h for test builds (issue #244).
// UBDisplayManager only uses UBBoardView pointers upcast to QWidget*, so a
// trivial QWidget subclass is enough. Shadows the heavy real header via the
// tests/ include-path priority in tests.pro.
#ifndef UBBOARDVIEW_H_
#define UBBOARDVIEW_H_

#include <QWidget>

class UBBoardView : public QWidget
{
public:
    explicit UBBoardView(QWidget* parent = nullptr) : QWidget(parent) {}
};

#endif /* UBBOARDVIEW_H_ */
