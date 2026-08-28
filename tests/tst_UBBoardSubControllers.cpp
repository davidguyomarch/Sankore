/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBBoardSubControllers.h"
#include <QFont>
#include <QPair>

// Forward declarations of testable static functions (implemented in stub)
namespace UBBoardZoomController {
    QPair<qreal, qreal> computeZoomRatio(qreal requestedRatio, qreal currentViewScale, qreal systemScaleFactor, qreal maxZoom);
}
namespace UBBoardToolbarController {
    QString truncate(const QString& text, int maxWidth, const QFont& font);
}

// --- Zoom ratio tests ---

void TestUBBoardSubControllers::testComputeZoomRatio_normalZoom()
{
    // Normal case: ratio 2x, current scale 1.0, system factor 1.0, max 9.0
    auto result = UBBoardZoomController::computeZoomRatio(2.0, 1.0, 1.0, 9.0);
    QCOMPARE(result.first, 2.0);   // currentZoom = 2.0
    QCOMPARE(result.second, 2.0);  // usedRatio unchanged
}

void TestUBBoardSubControllers::testComputeZoomRatio_clampedToMax()
{
    // Exceeds max: ratio 5x, current scale 2.0, system factor 1.0, max 9.0
    // currentZoom = 5 * 2 / 1 = 10 > 9
    auto result = UBBoardZoomController::computeZoomRatio(5.0, 2.0, 1.0, 9.0);
    QCOMPARE(result.first, 9.0);   // clamped to max
    // usedRatio = 9 * 1 / 2 = 4.5
    QCOMPARE(result.second, 4.5);
}

void TestUBBoardSubControllers::testComputeZoomRatio_atBoundary()
{
    // Exactly at max: ratio 9x, current scale 1.0, system factor 1.0
    auto result = UBBoardZoomController::computeZoomRatio(9.0, 1.0, 1.0, 9.0);
    QCOMPARE(result.first, 9.0);
    QCOMPARE(result.second, 9.0);  // not clamped (equal, not greater)
}

void TestUBBoardSubControllers::testComputeZoomRatio_zoomOut()
{
    // Zoom out: ratio 0.5, current scale 4.0, system factor 1.0
    // currentZoom = 0.5 * 4 / 1 = 2.0 (< 9)
    auto result = UBBoardZoomController::computeZoomRatio(0.5, 4.0, 1.0, 9.0);
    QCOMPARE(result.first, 2.0);
    QCOMPARE(result.second, 0.5);  // unchanged
}

// --- Truncate tests ---

void TestUBBoardSubControllers::testTruncate_shortText()
{
    QFont font("Arial", 12);
    QString result = UBBoardToolbarController::truncate("Hi", 200, font);
    QCOMPARE(result, QString("Hi"));  // short enough, no elision
}

void TestUBBoardSubControllers::testTruncate_longText()
{
    QFont font("Arial", 12);
    QString longText = "This is an extremely long toolbar text that should definitely be truncated";
    QString result = UBBoardToolbarController::truncate(longText, 48, font);
    // Should be shorter than original and end with ellipsis
    QVERIFY(result.length() < longText.length());
    QVERIFY(result.contains(QChar(0x2026)) || result.endsWith("..."));
}

void TestUBBoardSubControllers::testTruncate_emptyText()
{
    QFont font("Arial", 12);
    QString result = UBBoardToolbarController::truncate("", 48, font);
    QCOMPARE(result, QString(""));
}
