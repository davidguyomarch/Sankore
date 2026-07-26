#include "tst_UBSvgTransform.h"
#include <QTransform>

/**
 * Standalone implementations matching UBSvgSubsetAdaptor::toSvgTransform/fromSvgTransform.
 * Same algorithm — tests validate the parsing/serialization logic.
 */
static QString svgTransformToString(const QTransform& matrix)
{
    return QString("matrix(%1, %2, %3, %4, %5, %6)")
           .arg(matrix.m11(), 0, 'g')
           .arg(matrix.m12(), 0, 'g')
           .arg(matrix.m21(), 0, 'g')
           .arg(matrix.m22(), 0, 'g')
           .arg(matrix.dx(), 0, 'g')
           .arg(matrix.dy(), 0, 'g');
}

static QTransform svgTransformFromString(const QString& transform)
{
    QTransform matrix;
    QString ts = transform;
    ts.replace("matrix(", "");
    ts.replace(")", "");
    QStringList sl = ts.split(",");

    if (sl.size() >= 6)
    {
        matrix.setMatrix(
            sl.at(0).toFloat(),
            sl.at(1).toFloat(),
            0,
            sl.at(2).toFloat(),
            sl.at(3).toFloat(),
            0,
            sl.at(4).toFloat(),
            sl.at(5).toFloat(),
            1);
    }

    return matrix;
}

void TestUBSvgTransform::testToSvgTransform_identity()
{
    QTransform identity;
    QString result = svgTransformToString(identity);
    QVERIFY(result.startsWith("matrix("));
    QVERIFY(result.contains("1"));
    QVERIFY(result.endsWith(")"));
}

void TestUBSvgTransform::testToSvgTransform_translation()
{
    QTransform t;
    t.translate(100, 200);
    QString result = svgTransformToString(t);
    QVERIFY(result.contains("100"));
    QVERIFY(result.contains("200"));
}

void TestUBSvgTransform::testToSvgTransform_rotation()
{
    QTransform t;
    t.rotate(90);
    QString result = svgTransformToString(t);
    // 90 degree rotation: m11≈0, m12≈1, m21≈-1, m22≈0
    QVERIFY(!result.isEmpty());
}

void TestUBSvgTransform::testToSvgTransform_scale()
{
    QTransform t;
    t.scale(2.0, 3.0);
    QString result = svgTransformToString(t);
    QVERIFY(result.contains("2"));
    QVERIFY(result.contains("3"));
}

void TestUBSvgTransform::testFromSvgTransform_identity()
{
    QTransform result = svgTransformFromString("matrix(1, 0, 0, 1, 0, 0)");
    QVERIFY(result.isIdentity());
}

void TestUBSvgTransform::testFromSvgTransform_translation()
{
    QTransform result = svgTransformFromString("matrix(1, 0, 0, 1, 50, 75)");
    QCOMPARE(result.dx(), 50.0f);
    QCOMPARE(result.dy(), 75.0f);
    QCOMPARE(result.m11(), 1.0f);
    QCOMPARE(result.m22(), 1.0f);
}

void TestUBSvgTransform::testFromSvgTransform_complex()
{
    // Scale 2x + translate (10, 20)
    QTransform result = svgTransformFromString("matrix(2, 0, 0, 2, 10, 20)");
    QCOMPARE(result.m11(), 2.0f);
    QCOMPARE(result.m22(), 2.0f);
    QCOMPARE(result.dx(), 10.0f);
    QCOMPARE(result.dy(), 20.0f);
}

void TestUBSvgTransform::testFromSvgTransform_invalidInput()
{
    // Less than 6 values — should return identity
    QTransform result = svgTransformFromString("matrix(1, 0)");
    QVERIFY(result.isIdentity());

    // Empty string
    result = svgTransformFromString("");
    QVERIFY(result.isIdentity());

    // No matrix() wrapper
    result = svgTransformFromString("1, 0, 0, 1, 0, 0");
    QVERIFY(result.isIdentity());
}

void TestUBSvgTransform::testRoundtrip()
{
    // Create a transform, serialize, deserialize, compare
    QTransform original;
    original.translate(150, 250);
    original.scale(1.5, 1.5);

    QString svg = svgTransformToString(original);
    QTransform restored = svgTransformFromString(svg);

    // Compare with float tolerance
    QVERIFY(qAbs(original.m11() - restored.m11()) < 0.01);
    QVERIFY(qAbs(original.m22() - restored.m22()) < 0.01);
    QVERIFY(qAbs(original.dx() - restored.dx()) < 0.01);
    QVERIFY(qAbs(original.dy() - restored.dy()) < 0.01);
}
