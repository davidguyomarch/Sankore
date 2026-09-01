/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBGraphicsStroke.h"

#include "domain/UBGraphicsStroke.h"
#include "UBGraphicsPolygonItem.h" // test stub (tests/stubs, prepended to INCLUDEPATH)

#include <memory>

namespace {
// Builds a stub polygon with the given attributes and attaches it to the stroke.
UBGraphicsPolygonItem* addPolygon(UBGraphicsStroke& stroke, qreal width,
                                  bool nominal, const QColor& color)
{
    auto* poly = new UBGraphicsPolygonItem();
    poly->setOriginalWidth(width);
    poly->setNominalLine(nominal);
    poly->setColor(color);
    poly->attachTo(&stroke);
    return poly;
}
}

void TestUBGraphicsStroke::testEmptyStroke()
{
    UBGraphicsStroke stroke;
    QVERIFY(stroke.polygons().isEmpty());
    QVERIFY(!stroke.hasPressure());
    QVERIFY(!stroke.hasAlpha());
}

void TestUBGraphicsStroke::testAddPolygon()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 1.0, true, Qt::black);

    QCOMPARE(stroke.polygons().size(), 2);
    QCOMPARE(stroke.polygons().at(0), p1);
    QCOMPARE(stroke.polygons().at(1), p2);

    qDeleteAll(stroke.polygons());
}

void TestUBGraphicsStroke::testAddPolygon_deduplicates()
{
    UBGraphicsStroke stroke;
    auto* p1 = new UBGraphicsPolygonItem();

    // Attaching the same polygon twice must not create a duplicate; addPolygon()
    // removes any existing occurrence before appending, so it ends up once, at the end.
    p1->attachTo(&stroke);
    auto* p2 = addPolygon(stroke, 1.0, true, Qt::black);
    p1->attachTo(&stroke);

    QCOMPARE(stroke.polygons().size(), 2);
    QCOMPARE(stroke.polygons().at(0), p2);
    QCOMPARE(stroke.polygons().at(1), p1);

    delete p1;
    delete p2;
}

void TestUBGraphicsStroke::testRemove()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 1.0, true, Qt::black);

    stroke.remove(p1);

    QCOMPARE(stroke.polygons().size(), 1);
    QCOMPARE(stroke.polygons().at(0), p2);

    delete p1;
    delete p2;
}

void TestUBGraphicsStroke::testRemove_absentPolygon()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, Qt::black);

    std::unique_ptr<UBGraphicsPolygonItem> notAdded(new UBGraphicsPolygonItem());
    stroke.remove(notAdded.get()); // no-op, must not crash or alter the list

    QCOMPARE(stroke.polygons().size(), 1);

    delete p1;
}

void TestUBGraphicsStroke::testClear()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 1.0, true, Qt::black);

    stroke.clear();
    QVERIFY(stroke.polygons().isEmpty());

    delete p1;
    delete p2;
}

void TestUBGraphicsStroke::testHasPressure_falseWhenUniform()
{
    UBGraphicsStroke stroke;
    // >2 polygons, all nominal lines with identical width => no pressure.
    auto* p1 = addPolygon(stroke, 2.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 2.0, true, Qt::black);
    auto* p3 = addPolygon(stroke, 2.0, true, Qt::black);

    QVERIFY(!stroke.hasPressure());

    delete p1; delete p2; delete p3;
}

void TestUBGraphicsStroke::testHasPressure_trueWhenWidthVaries()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 2.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 4.0, true, Qt::black); // different width
    auto* p3 = addPolygon(stroke, 2.0, true, Qt::black);

    QVERIFY(stroke.hasPressure());

    delete p1; delete p2; delete p3;
}

void TestUBGraphicsStroke::testHasPressure_trueWhenNotNominal()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 2.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 2.0, false, Qt::black); // not a nominal line
    auto* p3 = addPolygon(stroke, 2.0, true, Qt::black);

    QVERIFY(stroke.hasPressure());

    delete p1; delete p2; delete p3;
}

void TestUBGraphicsStroke::testHasPressure_falseWhenTooFewPolygons()
{
    UBGraphicsStroke stroke;
    // Only 2 polygons: the count() > 2 guard means hasPressure() is false even
    // when the widths differ.
    auto* p1 = addPolygon(stroke, 2.0, true, Qt::black);
    auto* p2 = addPolygon(stroke, 5.0, false, Qt::black);

    QVERIFY(!stroke.hasPressure());

    delete p1; delete p2;
}

void TestUBGraphicsStroke::testHasAlpha_falseWhenOpaque()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, QColor(0, 0, 0, 255)); // fully opaque

    QVERIFY(!stroke.hasAlpha());

    delete p1;
}

void TestUBGraphicsStroke::testHasAlpha_trueWhenTransparent()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, QColor(0, 0, 0, 128)); // semi-transparent

    QVERIFY(stroke.hasAlpha());

    delete p1;
}

void TestUBGraphicsStroke::testHasAlpha_falseWhenEmpty()
{
    UBGraphicsStroke stroke;
    QVERIFY(!stroke.hasAlpha());
}

void TestUBGraphicsStroke::testDeepCopy()
{
    UBGraphicsStroke stroke;
    auto* p1 = addPolygon(stroke, 1.0, true, Qt::black);

    // deepCopy() currently returns a fresh empty stroke (polygons are not cloned).
    std::unique_ptr<UBGraphicsStroke> copy(stroke.deepCopy());
    QVERIFY(copy != nullptr);
    QVERIFY(copy->polygons().isEmpty());

    delete p1;
}
