#include "tst_UBVisualRegression.h"

#include <QApplication>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <cmath>

void TestUBVisualRegression::initTestCase()
{
    // Determine paths relative to test binary
    QString baseDir = QCoreApplication::applicationDirPath() + "/../../visual";
    m_referencePath = baseDir + "/reference";
    m_actualPath = baseDir + "/actual";

    QDir().mkpath(m_referencePath);
    QDir().mkpath(m_actualPath);

    // Check if we're in generate mode (via environment variable)
    m_generateMode = qEnvironmentVariableIsSet("UB_GENERATE_VISUAL_REFERENCES");

    if (m_generateMode)
        qDebug() << "VISUAL TESTS: Running in GENERATE mode — updating reference images";

    // Load the global stylesheet
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qApp->setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}

void TestUBVisualRegression::cleanupTestCase()
{
}

double TestUBVisualRegression::computeRMSE(const QImage &a, const QImage &b)
{
    if (a.size() != b.size())
        return 1.0; // max error if sizes differ

    QImage imgA = a.convertToFormat(QImage::Format_ARGB32);
    QImage imgB = b.convertToFormat(QImage::Format_ARGB32);

    double sumSquaredError = 0.0;
    int pixelCount = imgA.width() * imgA.height();

    for (int y = 0; y < imgA.height(); ++y)
    {
        const QRgb *lineA = reinterpret_cast<const QRgb*>(imgA.constScanLine(y));
        const QRgb *lineB = reinterpret_cast<const QRgb*>(imgB.constScanLine(y));

        for (int x = 0; x < imgA.width(); ++x)
        {
            int dr = qRed(lineA[x]) - qRed(lineB[x]);
            int dg = qGreen(lineA[x]) - qGreen(lineB[x]);
            int db = qBlue(lineA[x]) - qBlue(lineB[x]);
            int da = qAlpha(lineA[x]) - qAlpha(lineB[x]);

            sumSquaredError += (dr * dr + dg * dg + db * db + da * da) / (4.0 * 255.0 * 255.0);
        }
    }

    return std::sqrt(sumSquaredError / pixelCount);
}

bool TestUBVisualRegression::compareImages(const QImage &actual, const QImage &reference, double threshold)
{
    double rmse = computeRMSE(actual, reference);
    if (rmse > threshold)
    {
        qWarning() << "RMSE:" << rmse << "(threshold:" << threshold << ")";
        return false;
    }
    return true;
}

void TestUBVisualRegression::saveActual(const QImage &image, const QString &name)
{
    QString path = m_actualPath + "/" + name + ".png";
    image.save(path);
    qDebug() << "Saved actual:" << path;
}

bool TestUBVisualRegression::checkOrGenerateReference(const QImage &actual, const QString &name, double threshold)
{
    QString refPath = m_referencePath + "/" + name + ".png";

    if (m_generateMode)
    {
        actual.save(refPath);
        qDebug() << "Generated reference:" << refPath;
        return true;
    }

    QImage reference(refPath);
    if (reference.isNull())
    {
        // No reference exists — save actual and skip (not a failure on first run)
        saveActual(actual, name);
        qWarning() << "No reference image found:" << refPath << "— run with --generate-visual-references";
        return true; // Don't fail, just warn
    }

    if (!compareImages(actual, reference, threshold))
    {
        saveActual(actual, name);
        return false;
    }

    return true;
}

// --- Test cases ---

void TestUBVisualRegression::testToolButtonRendering()
{
    // Create a QToolButton styled like our toolbar buttons
    QToolButton button;
    button.setText("Test");
    button.setIcon(QIcon(":/images/toolbar/svg/stylus.svg"));
    button.setIconSize(QSize(32, 32));
    button.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button.setFixedSize(64, 56);
    button.show();
    (void)QTest::qWaitForWindowExposed(&button);

    QImage capture = button.grab().toImage();
    QVERIFY2(checkOrGenerateReference(capture, "toolbutton_normal"),
             "Tool button rendering differs from reference");
}

void TestUBVisualRegression::testDarkToolBarStyle()
{
    // Create a toolbar with a few actions
    QToolBar toolbar;
    toolbar.setIconSize(QSize(32, 32));
    toolbar.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QAction *action1 = toolbar.addAction(QIcon(":/images/toolbar/svg/stylus.svg"), "Stylus");
    QAction *action2 = toolbar.addAction(QIcon(":/images/toolbar/svg/undo.svg"), "Undo");
    toolbar.addSeparator();
    QAction *action3 = toolbar.addAction(QIcon(":/images/toolbar/svg/redo.svg"), "Redo");

    Q_UNUSED(action1);
    Q_UNUSED(action2);
    Q_UNUSED(action3);

    toolbar.setFixedSize(300, 64);
    toolbar.show();
    (void)QTest::qWaitForWindowExposed(&toolbar);

    QImage capture = toolbar.grab().toImage();
    QVERIFY2(checkOrGenerateReference(capture, "toolbar_dark"),
             "Dark toolbar rendering differs from reference");
}

void TestUBVisualRegression::testScrollBarMinimal()
{
    // Create a vertical scrollbar
    QScrollBar scrollbar(Qt::Vertical);
    scrollbar.setRange(0, 100);
    scrollbar.setValue(30);
    scrollbar.setFixedSize(20, 200);
    scrollbar.show();
    (void)QTest::qWaitForWindowExposed(&scrollbar);

    QImage capture = scrollbar.grab().toImage();
    QVERIFY2(checkOrGenerateReference(capture, "scrollbar_vertical"),
             "Scrollbar rendering differs from reference");
}

void TestUBVisualRegression::testButtonStyles()
{
    // Create a styled button matching DockPaletteWidgetButton
    QPushButton button("Test Button");
    button.setObjectName("DockPaletteWidgetButton");
    button.setFixedSize(120, 36);
    button.show();
    (void)QTest::qWaitForWindowExposed(&button);

    QImage normalCapture = button.grab().toImage();
    QVERIFY2(checkOrGenerateReference(normalCapture, "button_dock_normal"),
             "Dock button rendering differs from reference");

    // Checked state
    button.setCheckable(true);
    button.setChecked(true);
    button.repaint();
    QTest::qWait(50);

    QImage checkedCapture = button.grab().toImage();
    QVERIFY2(checkOrGenerateReference(checkedCapture, "button_dock_checked"),
             "Dock button checked rendering differs from reference");
}
