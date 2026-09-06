/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBSmoothStrokeItem.h"
#include "domain/UBSmoothStrokeItem.h"

#include <QGraphicsScene>
#include <QPainterPath>

// --- addPoint + rebuildPath ---

void TestUBSmoothStrokeItem::testAddPoint_singlePoint()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.addPoint(QPointF(10, 20), 0.5);

    QCOMPARE(item.rawPoints().size(), 1);
    QCOMPARE(item.rawPressures().size(), 1);

    // Single point → dot (ellipse path)
    QVERIFY(!item.path().isEmpty());
}

void TestUBSmoothStrokeItem::testAddPoint_twoPoints()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(100, 0), 1.0);

    QCOMPARE(item.rawPoints().size(), 2);

    // Two points → line segment (moveTo + lineTo)
    QPainterPath path = item.path();
    QCOMPARE(path.elementCount(), 2); // moveTo + lineTo
}

void TestUBSmoothStrokeItem::testAddPoint_multiplePoints_bezierCurves()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    // Add 5 points → should produce 4 cubic Bézier segments
    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(50, 30), 0.8);
    item.addPoint(QPointF(100, 10), 0.9);
    item.addPoint(QPointF(150, 40), 0.7);
    item.addPoint(QPointF(200, 20), 1.0);

    QCOMPARE(item.rawPoints().size(), 5);
    QCOMPARE(item.rawPressures().size(), 5);

    // Path: 1 moveTo + 4 cubicTo (each cubicTo = 3 elements: CurveTo + 2 CurveToData)
    // Total: 1 + 4*3 = 13 elements
    QPainterPath path = item.path();
    QCOMPARE(path.elementCount(), 13);

    // First element is MoveTo
    QCOMPARE(path.elementAt(0).type, QPainterPath::MoveToElement);
    // Second element is CurveTo (start of first Bézier)
    QCOMPARE(path.elementAt(1).type, QPainterPath::CurveToElement);
}

void TestUBSmoothStrokeItem::testAddPoint_duplicateSkipped()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.addPoint(QPointF(10, 20), 1.0);
    item.addPoint(QPointF(10, 20), 1.0); // duplicate — should be skipped

    QCOMPARE(item.rawPoints().size(), 1);
}

// --- finalize ---

void TestUBSmoothStrokeItem::testFinalize_marksComplete()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    QVERIFY(!item.isFinalized());

    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(50, 50), 1.0);
    item.finalize();

    QVERIFY(item.isFinalized());
}

// --- Properties ---

void TestUBSmoothStrokeItem::testSetStrokeWidth()
{
    UBSmoothStrokeItem item;

    item.setStrokeWidth(5.0);
    QCOMPARE(item.nominalWidth(), 5.0);
    QCOMPARE(item.pen().widthF(), 5.0);
}

void TestUBSmoothStrokeItem::testSetStrokeColor()
{
    UBSmoothStrokeItem item;

    item.setStrokeColor(Qt::red);
    QCOMPARE(item.pen().color(), QColor(Qt::red));
}

void TestUBSmoothStrokeItem::testColorOnBackgrounds()
{
    UBSmoothStrokeItem item;

    item.setColorOnDarkBackground(Qt::cyan);
    item.setColorOnLightBackground(Qt::magenta);

    QCOMPARE(item.colorOnDarkBackground(), QColor(Qt::cyan));
    QCOMPARE(item.colorOnLightBackground(), QColor(Qt::magenta));
}

// --- subtractPath ---

void TestUBSmoothStrokeItem::testSubtractPath_partialErase()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.setStrokeWidth(4.0);
    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(200, 0), 1.0);
    item.finalize();

    // Erase with a path that does NOT touch the stroke (far away)
    QPainterPath eraserPath;
    eraserPath.addRect(90, 50, 20, 40); // y=50..90, well above the stroke at y=0

    QList<QGraphicsItem*> newFragments;
    bool shouldRemove = item.subtractPath(eraserPath, newFragments);

    // Item should NOT be removed (eraser missed)
    QVERIFY(!shouldRemove);
    // Path should still have content
    QVERIFY(!item.path().isEmpty());
}

void TestUBSmoothStrokeItem::testSubtractPath_fullErase()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.setStrokeWidth(2.0);
    item.addPoint(QPointF(10, 10), 1.0);
    item.addPoint(QPointF(20, 10), 1.0);
    item.finalize();

    // Erase everything — huge rectangle covering the entire stroke
    QPainterPath eraserPath;
    eraserPath.addRect(-100, -100, 500, 500);

    QList<QGraphicsItem*> newFragments;
    bool shouldRemove = item.subtractPath(eraserPath, newFragments);

    QVERIFY(shouldRemove);
}

// --- deepCopy ---

void TestUBSmoothStrokeItem::testSetLastPoint_replaceEndpoint()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(100, 0), 1.0);

    QCOMPARE(item.rawPoints().size(), 2);

    // Replace endpoint
    item.setLastPoint(QPointF(50, 50), 0.8);

    // Still 2 points
    QCOMPARE(item.rawPoints().size(), 2);
    // Last pressure updated
    QCOMPARE(item.rawPressures().last(), 0.8);
    // Path should be a line to (50, 50) in local coords
    QPainterPath path = item.path();
    QCOMPARE(path.elementCount(), 2);
}

void TestUBSmoothStrokeItem::testSetLastPoint_onSinglePoint()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.addPoint(QPointF(10, 20), 1.0);
    QCOMPARE(item.rawPoints().size(), 1);

    // setLastPoint on single-point item should add a second point
    item.setLastPoint(QPointF(80, 90), 0.5);
    QCOMPARE(item.rawPoints().size(), 2);
}

void TestUBSmoothStrokeItem::testBoundingRect_expandedForSoftEdge()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.setStrokeWidth(5.0);
    // Add multiple points to overcome the stabilizer EMA filter
    // Each successive point gets closer to the target due to the filter
    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(50, 0), 1.0);
    item.addPoint(QPointF(100, 0), 1.0);
    item.addPoint(QPointF(150, 0), 1.0);
    item.addPoint(QPointF(200, 0), 1.0);

    QRectF bounds = item.boundingRect();

    // After stabilizer filtering, the path won't reach 200 exactly but should
    // extend well beyond 100. The bounding rect includes pen width + margin.
    QVERIFY(bounds.left() < 0.0);
    QVERIFY(bounds.right() > 100.0);
    QVERIFY(bounds.top() < -2.0);
    QVERIFY(bounds.bottom() > 2.0);
}

void TestUBSmoothStrokeItem::testDeepCopy()
{
    QGraphicsScene scene;
    UBSmoothStrokeItem item;
    scene.addItem(&item);

    item.setStrokeWidth(3.5);
    item.setStrokeColor(QColor(255, 128, 0));
    item.setColorOnDarkBackground(Qt::yellow);
    item.setColorOnLightBackground(Qt::blue);
    item.addPoint(QPointF(0, 0), 1.0);
    item.addPoint(QPointF(50, 25), 0.8);
    item.addPoint(QPointF(100, 0), 0.6);
    item.finalize();

    UBItem* copyBase = item.deepCopy();
    UBSmoothStrokeItem* copy = dynamic_cast<UBSmoothStrokeItem*>(copyBase);

    QVERIFY(copy != nullptr);
    QCOMPARE(copy->nominalWidth(), 3.5);
    QCOMPARE(copy->pen().color(), QColor(255, 128, 0));
    QCOMPARE(copy->colorOnDarkBackground(), QColor(Qt::yellow));
    QCOMPARE(copy->colorOnLightBackground(), QColor(Qt::blue));
    QCOMPARE(copy->rawPoints().size(), 3);
    QCOMPARE(copy->rawPressures().size(), 3);
    QVERIFY(copy->isFinalized());

    // UUID should be different (new copy)
    QVERIFY(copy->uuid() != item.uuid());

    // Path should match
    QCOMPARE(copy->path().elementCount(), item.path().elementCount());

    delete copy;
}

// --- type ---

void TestUBSmoothStrokeItem::testType()
{
    UBSmoothStrokeItem item;
    QCOMPARE(item.type(), static_cast<int>(UBGraphicsItemType::SmoothStrokeItemType));
}

// --- #243 regression: the item must own a delegate ---

void TestUBSmoothStrokeItem::testHasDelegate_regression243()
{
    // Before the fix, the constructor never called setDelegate(), so Delegate()
    // returned nullptr. Selecting the stroke then ran, in UBBoardView,
    //   dynamic_cast<UBGraphicsItem*>(item)->Delegate()->startUndoStep();
    // which crashed on the null delegate. Text items didn't crash because they
    // create a delegate. This pins the invariant: a stroke owns a delegate.
    UBSmoothStrokeItem item;
    UBGraphicsItem* asUbItem = dynamic_cast<UBGraphicsItem*>(&item);
    QVERIFY(asUbItem != nullptr);            // the cast that UBBoardView performs
    QVERIFY(asUbItem->Delegate() != nullptr); // the pointer it then dereferences
}
