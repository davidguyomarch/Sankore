#include "tst_UBSvgTransform.h"
#include "adaptors/UBSvgTransformUtils.h"
#include <QTransform>

void TestUBSvgTransform::testToSvgTransform_identity()
{
    QTransform identity;
    QString result = UBSvgTransformUtils::toSvgTransform(identity);
    QVERIFY(result.startsWith("matrix("));
    QVERIFY(result.contains("1"));
    QVERIFY(result.endsWith(")"));
}

void TestUBSvgTransform::testToSvgTransform_translation()
{
    QTransform t;
    t.translate(100, 200);
    QString result = UBSvgTransformUtils::toSvgTransform(t);
    QVERIFY(result.contains("100"));
    QVERIFY(result.contains("200"));
}

void TestUBSvgTransform::testToSvgTransform_rotation()
{
    QTransform t;
    t.rotate(90);
    QString result = UBSvgTransformUtils::toSvgTransform(t);
    // 90 degree rotation: m11≈0, m12≈1, m21≈-1, m22≈0
    QVERIFY(!result.isEmpty());
}

void TestUBSvgTransform::testToSvgTransform_scale()
{
    QTransform t;
    t.scale(2.0, 3.0);
    QString result = UBSvgTransformUtils::toSvgTransform(t);
    QVERIFY(result.contains("2"));
    QVERIFY(result.contains("3"));
}

void TestUBSvgTransform::testFromSvgTransform_identity()
{
    QTransform result = UBSvgTransformUtils::fromSvgTransform("matrix(1, 0, 0, 1, 0, 0)");
    QVERIFY(result.isIdentity());
}

void TestUBSvgTransform::testFromSvgTransform_translation()
{
    QTransform result = UBSvgTransformUtils::fromSvgTransform("matrix(1, 0, 0, 1, 50, 75)");
    QCOMPARE(result.dx(), 50.0f);
    QCOMPARE(result.dy(), 75.0f);
    QCOMPARE(result.m11(), 1.0f);
    QCOMPARE(result.m22(), 1.0f);
}

void TestUBSvgTransform::testFromSvgTransform_complex()
{
    // Scale 2x + translate (10, 20)
    QTransform result = UBSvgTransformUtils::fromSvgTransform("matrix(2, 0, 0, 2, 10, 20)");
    QCOMPARE(result.m11(), 2.0f);
    QCOMPARE(result.m22(), 2.0f);
    QCOMPARE(result.dx(), 10.0f);
    QCOMPARE(result.dy(), 20.0f);
}

void TestUBSvgTransform::testFromSvgTransform_invalidInput()
{
    // Less than 6 values — should return identity
    QTransform result = UBSvgTransformUtils::fromSvgTransform("matrix(1, 0)");
    QVERIFY(result.isIdentity());

    // Empty string
    result = UBSvgTransformUtils::fromSvgTransform("");
    QVERIFY(result.isIdentity());

    // No matrix() wrapper
    result = UBSvgTransformUtils::fromSvgTransform("1, 0, 0, 1, 0, 0");
    QVERIFY(result.isIdentity());
}

void TestUBSvgTransform::testRoundtrip()
{
    // Create a transform, serialize, deserialize, compare
    QTransform original;
    original.translate(150, 250);
    original.scale(1.5, 1.5);

    QString svg = UBSvgTransformUtils::toSvgTransform(original);
    QTransform restored = UBSvgTransformUtils::fromSvgTransform(svg);

    // Compare with float tolerance
    QVERIFY(qAbs(original.m11() - restored.m11()) < 0.01);
    QVERIFY(qAbs(original.m22() - restored.m22()) < 0.01);
    QVERIFY(qAbs(original.dx() - restored.dx()) < 0.01);
    QVERIFY(qAbs(original.dy() - restored.dy()) < 0.01);
}
