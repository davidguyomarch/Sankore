/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Test stub for UBGraphicsPolygonItem.
//
// The real UBGraphicsPolygonItem derives from QGraphicsPolygonItem + UBItem and
// pulls in the whole graphics-item / delegate / scene stack, which is far too
// heavy to link in a unit test. UBGraphicsStroke only ever calls originalWidth(),
// isNominalLine() and color() on it, so this stub provides exactly those, plus a
// small attachTo() helper that lets a test push the polygon into a stroke's
// (protected) polygon list — UBGraphicsPolygonItem is a friend of UBGraphicsStroke.
//
// It uses the SAME include guard as the real header (UBGRAPHICSPOLYGONITEM_H) so
// that including it first shadows the real header for any translation unit — this
// is what the UBGraphicsStroke_testable.cpp wrapper relies on. The test binary
// therefore sees a single, consistent definition of the class.

#ifndef UBGRAPHICSPOLYGONITEM_H
#define UBGRAPHICSPOLYGONITEM_H

#include <QColor>

#include "domain/UBGraphicsStroke.h"

class UBGraphicsPolygonItem
{
public:
    UBGraphicsPolygonItem() = default;

    void setOriginalWidth(qreal w) { mOriginalWidth = w; }
    qreal originalWidth() { return mOriginalWidth; }

    void setNominalLine(bool nominal) { mIsNominalLine = nominal; }
    bool isNominalLine() { return mIsNominalLine; }

    void setColor(const QColor& color) { mColor = color; }
    QColor color() const { return mColor; }

    // Helper for tests: adds this polygon to a stroke through the protected
    // UBGraphicsStroke::addPolygon(), which is accessible because this class is
    // declared friend in UBGraphicsStroke.
    void attachTo(UBGraphicsStroke* stroke) { stroke->addPolygon(this); }

private:
    qreal mOriginalWidth = 1.0;
    bool mIsNominalLine = true;
    QColor mColor = QColor(0, 0, 0, 255);
};

#endif // UBGRAPHICSPOLYGONITEM_H
