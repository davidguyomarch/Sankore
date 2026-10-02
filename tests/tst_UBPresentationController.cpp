/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBPresentationController.h"

#include "core/UBPresentationController.h"

#include <QSignalSpy>

using State = UBPresentationController::State;

void TestUBPresentationController::testInitialStateIsBoard()
{
    UBPresentationController c;
    QCOMPARE(c.state(), State::Board);
}

void TestUBPresentationController::testSetStateChangesStateAndEmits()
{
    UBPresentationController c;
    qRegisterMetaType<UBPresentationController::State>("UBPresentationController::State");
    QSignalSpy spy(&c, &UBPresentationController::stateChanged);

    c.setState(State::DesktopAnnotation);

    QCOMPARE(c.state(), State::DesktopAnnotation);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).value<State>(), State::Board);
    QCOMPARE(args.at(1).value<State>(), State::DesktopAnnotation);

    // A further distinct transition emits again.
    c.setState(State::Documents);
    QCOMPARE(c.state(), State::Documents);
    QCOMPARE(spy.count(), 1);
}

void TestUBPresentationController::testSameStateIsNoOp()
{
    UBPresentationController c;
    qRegisterMetaType<UBPresentationController::State>("UBPresentationController::State");
    QSignalSpy spy(&c, &UBPresentationController::stateChanged);

    // Already Board — setting Board again must not emit nor change.
    c.setState(State::Board);
    QCOMPARE(c.state(), State::Board);
    QCOMPARE(spy.count(), 0);

    // Move to Web, then set Web again: still a single emission.
    c.setState(State::Web);
    c.setState(State::Web);
    QCOMPARE(c.state(), State::Web);
    QCOMPARE(spy.count(), 1);
}

void TestUBPresentationController::testStateNameCoversAllValues()
{
    QCOMPARE(QString(UBPresentationController::stateName(State::Board)), QString("Board"));
    QCOMPARE(QString(UBPresentationController::stateName(State::DesktopAnnotation)), QString("DesktopAnnotation"));
    QCOMPARE(QString(UBPresentationController::stateName(State::Documents)), QString("Documents"));
    QCOMPARE(QString(UBPresentationController::stateName(State::Web)), QString("Web"));
}
