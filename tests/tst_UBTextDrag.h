/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBTEXTDRAG_H
#define TST_UBTEXTDRAG_H

#include <QObject>
#include <QTest>

// #279 — a text box must move by dragging from anywhere inside it, while a
// plain click still edits. These tests pin the pure drag-vs-click decision.
class TestUBTextDrag : public QObject
{
    Q_OBJECT

private slots:
    void testSmallMoveIsNotDrag();
    void testMovePastThresholdIsDrag();
    void testLatchedDragStaysDrag();
    void testExactThresholdIsDrag();
};

#endif // TST_UBTEXTDRAG_H
