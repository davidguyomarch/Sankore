#ifndef TST_UBSVGTRANSFORM_H
#define TST_UBSVGTRANSFORM_H

#include <QObject>
#include <QtTest>

class TestUBSvgTransform : public QObject
{
    Q_OBJECT

private slots:
    void testToSvgTransform_identity();
    void testToSvgTransform_translation();
    void testToSvgTransform_rotation();
    void testToSvgTransform_scale();
    void testFromSvgTransform_identity();
    void testFromSvgTransform_translation();
    void testFromSvgTransform_complex();
    void testFromSvgTransform_invalidInput();
    void testRoundtrip();
};

#endif // TST_UBSVGTRANSFORM_H
