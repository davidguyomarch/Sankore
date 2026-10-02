/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBPRESENTATIONCONTROLLER_H
#define TST_UBPRESENTATIONCONTROLLER_H

#include <QObject>
#include <QtTest>

/**
 * @brief Tests for UBPresentationController (#399, ADR-0008 D2 brick 2).
 *
 * The presentation-state controller is the single source of truth for which
 * surface is on screen. Its state logic is Qt-UI-free and unit-testable:
 *  - the initial state is Board;
 *  - setState() changes the state and emits stateChanged(from, to);
 *  - a transition to the current state is a no-op (no signal) — this is the
 *    guard the later single entry point will rely on;
 *  - stateName() maps every enum value.
 */
class TestUBPresentationController : public QObject
{
    Q_OBJECT

private slots:
    void testInitialStateIsBoard();
    void testSetStateChangesStateAndEmits();
    void testSameStateIsNoOp();
    void testStateNameCoversAllValues();
};

#endif // TST_UBPRESENTATIONCONTROLLER_H
