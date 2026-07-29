#ifndef TST_UBBOARDSUBCONTROLLERS_H
#define TST_UBBOARDSUBCONTROLLERS_H

#include <QObject>
#include <QtTest>

/**
 * @brief Unit tests for pure logic in board sub-controllers.
 *
 * Tests computeZoomRatio (zoom clamping) and truncate (text elision).
 * These are static methods with no dependencies on UBApplication.
 */
class TestUBBoardSubControllers : public QObject
{
    Q_OBJECT

private slots:
    void testComputeZoomRatio_normalZoom();
    void testComputeZoomRatio_clampedToMax();
    void testComputeZoomRatio_atBoundary();
    void testComputeZoomRatio_zoomOut();
    void testTruncate_shortText();
    void testTruncate_longText();
    void testTruncate_emptyText();
};

#endif // TST_UBBOARDSUBCONTROLLERS_H
