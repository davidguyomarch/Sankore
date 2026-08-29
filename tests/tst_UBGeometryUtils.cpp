/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBGeometryUtils.h"
#include "frameworks/UBGeometryUtils.h"

#include <cmath>

// --- lineToPolygon (uniform width) ---

void TestUBGeometryUtils::testLineToPolygon_horizontal()
{
    QLineF line(0, 0, 100, 0);
    QPolygonF polygon = UBGeometryUtils::lineToPolygon(line, 10.0);

    // Polygon should not be empty
    QVERIFY(!polygon.isEmpty());

    // Bounding rect should encompass the line with width
    QRectF bounds = polygon.boundingRect();
    QVERIFY(bounds.width() >= 100.0);
    QVERIFY(bounds.height() >= 10.0);
}

void TestUBGeometryUtils::testLineToPolygon_vertical()
{
    QLineF line(50, 0, 50, 200);
    QPolygonF polygon = UBGeometryUtils::lineToPolygon(line, 8.0);

    QVERIFY(!polygon.isEmpty());

    QRectF bounds = polygon.boundingRect();
    QVERIFY(bounds.height() >= 200.0);
    QVERIFY(bounds.width() >= 8.0);
}

void TestUBGeometryUtils::testLineToPolygon_diagonal()
{
    QLineF line(0, 0, 100, 100);
    QPolygonF polygon = UBGeometryUtils::lineToPolygon(line, 5.0);

    QVERIFY(!polygon.isEmpty());
    QVERIFY(polygon.size() > 2);
}

void TestUBGeometryUtils::testLineToPolygon_zeroWidth()
{
    QLineF line(0, 0, 100, 0);
    QPolygonF polygon = UBGeometryUtils::lineToPolygon(line, 0.0);

    // With zero width, polygon should still be created (degenerate case)
    // Just verify it doesn't crash
    Q_UNUSED(polygon);
}

// --- lineToPolygon (variable width) ---

void TestUBGeometryUtils::testLineToPolygon_variableWidth()
{
    QPointF start(0, 0);
    QPointF end(100, 0);
    QPolygonF polygon = UBGeometryUtils::lineToPolygon(start, end, 5.0, 15.0);

    QVERIFY(!polygon.isEmpty());

    // The polygon should be wider at the end than at the start
    QRectF bounds = polygon.boundingRect();
    QVERIFY(bounds.height() >= 15.0);
}

// --- lineToInnerRect ---

void TestUBGeometryUtils::testLineToInnerRect_basic()
{
    QLineF line(0, 0, 100, 0);
    QRectF rect = UBGeometryUtils::lineToInnerRect(line, 10.0);

    QVERIFY(rect.isValid());
    // Side = sqrt(width^2 / 2) = sqrt(50) ≈ 7.07
    qreal expectedSide = sqrt(100.0 / 2.0);
    QVERIFY(qAbs(rect.width() - expectedSide) < 0.01);
    QVERIFY(qAbs(rect.height() - expectedSide) < 0.01);
}

void TestUBGeometryUtils::testLineToInnerRect_centered()
{
    QLineF line(0, 0, 100, 100);
    QRectF rect = UBGeometryUtils::lineToInnerRect(line, 20.0);

    // Center should be at midpoint of line
    QPointF center = rect.center();
    QVERIFY(qAbs(center.x() - 50.0) < 0.01);
    QVERIFY(qAbs(center.y() - 50.0) < 0.01);
}

// --- pointConstrainedInRect (QPointF) ---

void TestUBGeometryUtils::testPointConstrainedInRect_inside()
{
    QRectF rect(0, 0, 100, 100);
    QPointF point(50, 50);
    QPointF result = UBGeometryUtils::pointConstrainedInRect(point, rect);
    QCOMPARE(result, point);
}

void TestUBGeometryUtils::testPointConstrainedInRect_outside()
{
    QRectF rect(0, 0, 100, 100);

    // Point to the left
    QPointF left(-10, 50);
    QPointF resultLeft = UBGeometryUtils::pointConstrainedInRect(left, rect);
    QCOMPARE(resultLeft.x(), 0.0);
    QCOMPARE(resultLeft.y(), 50.0);

    // Point above
    QPointF above(50, -20);
    QPointF resultAbove = UBGeometryUtils::pointConstrainedInRect(above, rect);
    QCOMPARE(resultAbove.x(), 50.0);
    QCOMPARE(resultAbove.y(), 0.0);

    // Point below-right
    QPointF belowRight(150, 200);
    QPointF resultBR = UBGeometryUtils::pointConstrainedInRect(belowRight, rect);
    QCOMPARE(resultBR.x(), 100.0);
    QCOMPARE(resultBR.y(), 100.0);
}

void TestUBGeometryUtils::testPointConstrainedInRect_onEdge()
{
    QRectF rect(0, 0, 100, 100);
    QPointF point(0, 50);
    QPointF result = UBGeometryUtils::pointConstrainedInRect(point, rect);
    QCOMPARE(result, point);
}

// --- pointConstrainedInRect (QPoint) ---

void TestUBGeometryUtils::testPointConstrainedInRectInt_inside()
{
    QRect rect(0, 0, 100, 100);
    QPoint point(50, 50);
    QPoint result = UBGeometryUtils::pointConstrainedInRect(point, rect);
    QCOMPARE(result, point);
}

void TestUBGeometryUtils::testPointConstrainedInRectInt_outside()
{
    QRect rect(0, 0, 100, 100);
    QPoint point(200, -50);
    QPoint result = UBGeometryUtils::pointConstrainedInRect(point, rect);
    QCOMPARE(result.x(), 100);
    QCOMPARE(result.y(), 0);
}

// --- crashPointList ---

void TestUBGeometryUtils::testCrashPointList_noDuplicates()
{
    QVector<QPointF> points;
    points << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 2);

    UBGeometryUtils::crashPointList(points);
    QCOMPARE(points.size(), 3);
}

void TestUBGeometryUtils::testCrashPointList_withDuplicates()
{
    QVector<QPointF> points;
    points << QPointF(0, 0) << QPointF(0, 0) << QPointF(1, 1) << QPointF(1, 1) << QPointF(2, 2);

    UBGeometryUtils::crashPointList(points);
    QCOMPARE(points.size(), 3);
    QCOMPARE(points.at(0), QPointF(0, 0));
    QCOMPARE(points.at(1), QPointF(1, 1));
    QCOMPARE(points.at(2), QPointF(2, 2));
}

void TestUBGeometryUtils::testCrashPointList_allSame()
{
    QVector<QPointF> points;
    points << QPointF(5, 5) << QPointF(5, 5) << QPointF(5, 5);

    UBGeometryUtils::crashPointList(points);
    QCOMPARE(points.size(), 1);
}

void TestUBGeometryUtils::testCrashPointList_empty()
{
    QVector<QPointF> points;
    UBGeometryUtils::crashPointList(points);
    QCOMPARE(points.size(), 0);
}

// --- arcToPolygon ---

void TestUBGeometryUtils::testArcToPolygon_basic()
{
    QLineF startRadius(QPointF(0, 0), QPointF(100, 0));
    QPolygonF polygon = UBGeometryUtils::arcToPolygon(startRadius, 90.0, 10.0);

    QVERIFY(!polygon.isEmpty());
    QVERIFY(polygon.size() > 2);
}

void TestUBGeometryUtils::testArcToPolygon_negativeAngle()
{
    QLineF startRadius(QPointF(0, 0), QPointF(100, 0));
    QPolygonF polygon = UBGeometryUtils::arcToPolygon(startRadius, -90.0, 10.0);

    QVERIFY(!polygon.isEmpty());
    QVERIFY(polygon.size() > 2);
}

void TestUBGeometryUtils::testArcToPolygon_fullCircle()
{
    QLineF startRadius(QPointF(0, 0), QPointF(50, 0));
    // Angle > 360 should produce a full ring (overlap case)
    QPolygonF polygon = UBGeometryUtils::arcToPolygon(startRadius, 400.0, 8.0);

    QVERIFY(!polygon.isEmpty());
    QVERIFY(polygon.size() > 2);
}

// --- Constants ---

void TestUBGeometryUtils::testConstants()
{
    QCOMPARE(UBGeometryUtils::centimeterGraduationHeight, 15);
    QCOMPARE(UBGeometryUtils::halfCentimeterGraduationHeight, 10);
    QCOMPARE(UBGeometryUtils::millimeterGraduationHeight, 5);
    QCOMPARE(UBGeometryUtils::millimetersPerCentimeter, 10);
    QCOMPARE(UBGeometryUtils::millimetersPerHalfCentimeter, 5);
    QVERIFY(qAbs(UBGeometryUtils::inchSize - 25.4f) < 0.01f);
}
