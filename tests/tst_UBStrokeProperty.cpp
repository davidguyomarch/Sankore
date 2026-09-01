/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBStrokeProperty.h"
#include "domain/UBStrokeProperty.h"

#include <QColor>
#include <QPainter>
#include <QPixmap>

void TestUBStrokeProperty::testDefaults()
{
    UBStrokeProperty prop;

    QCOMPARE(prop.color(), QColor(0, 0, 0, 255));
    QCOMPARE(prop.thickness(), 1u);
    QCOMPARE(prop.penStyle(), Qt::SolidLine);
}

void TestUBStrokeProperty::testSetters()
{
    UBStrokeProperty prop;

    prop.setColor(QColor(10, 20, 30, 200));
    prop.setThickness(5);
    prop.setPenStyle(Qt::DashLine);

    QCOMPARE(prop.color(), QColor(10, 20, 30, 200));
    QCOMPARE(prop.thickness(), 5u);
    QCOMPARE(prop.penStyle(), Qt::DashLine);
}

void TestUBStrokeProperty::testCopyConstructor()
{
    UBStrokeProperty source;
    source.setColor(QColor(255, 128, 0));
    source.setThickness(7);
    source.setPenStyle(Qt::DotLine);

    UBStrokeProperty copy(source);

    QCOMPARE(copy.color(), source.color());
    QCOMPARE(copy.thickness(), source.thickness());
    QCOMPARE(copy.penStyle(), source.penStyle());
}

void TestUBStrokeProperty::testStrokeAppliesToPainterPen()
{
    UBStrokeProperty prop;
    prop.setColor(QColor(12, 34, 56));
    prop.setThickness(4);
    prop.setPenStyle(Qt::DashDotLine);

    QPixmap pixmap(20, 20);
    QPainter painter(&pixmap);

    prop.stroke(&painter);

    const QPen& pen = painter.pen();
    QCOMPARE(pen.color(), QColor(12, 34, 56));
    QCOMPARE(pen.width(), 4);
    QCOMPARE(pen.style(), Qt::DashDotLine);

    painter.end();
}
