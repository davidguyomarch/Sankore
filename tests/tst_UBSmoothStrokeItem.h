#ifndef TST_UBSMOOTHSTROKEITEM_H
#define TST_UBSMOOTHSTROKEITEM_H

#include <QObject>
#include <QTest>

class TestUBSmoothStrokeItem : public QObject
{
    Q_OBJECT

private slots:
    // addPoint + rebuildPath
    void testAddPoint_singlePoint();
    void testAddPoint_twoPoints();
    void testAddPoint_multiplePoints_bezierCurves();
    void testAddPoint_duplicateSkipped();

    // finalize
    void testFinalize_marksComplete();

    // Properties
    void testSetStrokeWidth();
    void testSetStrokeColor();
    void testColorOnBackgrounds();

    // subtractPath
    void testSubtractPath_partialErase();
    void testSubtractPath_fullErase();

    // deepCopy
    void testDeepCopy();

    // type
    void testType();
};

#endif // TST_UBSMOOTHSTROKEITEM_H
