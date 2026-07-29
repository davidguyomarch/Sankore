#ifndef TST_UBVISUALREGRESSION_H
#define TST_UBVISUALREGRESSION_H

#include <QObject>
#include <QtTest>

/**
 * @brief Visual regression tests for Open-Sankoré UI components.
 *
 * Captures widget screenshots and compares them against reference images.
 * Uses RMSE (Root Mean Square Error) with a configurable threshold.
 *
 * To generate/update reference images:
 *   UB_GENERATE_VISUAL_REFERENCES=1 OpenSankoreTests TestUBVisualRegression
 *
 * Reference images are stored in tests/visual/reference/
 * Actual images (on failure) are saved in tests/visual/actual/
 */
class TestUBVisualRegression : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testToolButtonRendering();
    void testDarkToolBarStyle();
    void testScrollBarMinimal();
    void testButtonStyles();

private:
    bool compareImages(const QImage &actual, const QImage &reference, double threshold = 0.05);
    double computeRMSE(const QImage &a, const QImage &b);
    void saveActual(const QImage &image, const QString &name);
    bool checkOrGenerateReference(const QImage &actual, const QString &name, double threshold = 0.05);

    QString m_referencePath;
    QString m_actualPath;
    bool m_generateMode = false;
};

#endif // TST_UBVISUALREGRESSION_H
