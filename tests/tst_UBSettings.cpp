#include "tst_UBSettings.h"
#include "core/UBSettingsData.h"
#include "stubs/UBSettings_stub.h"
#include "stubs/UBPlatformUtils_stub.h"

#include <QSettings>
#include <QDir>
#include <QUuid>
#include <QFile>

void TestUBSettings::initTestCase()
{
    // Set unique organization/app name to isolate from real settings
    QCoreApplication::setOrganizationName("OpenSankoreTest_" + QUuid::createUuid().toString());
    QCoreApplication::setApplicationName("UBSettingsTest");

    // Create a temp .ini file path for QSettings testing
    mTempIniPath = QDir::tempPath() + "/ubsettings_test_" + QUuid::createUuid().toString() + ".ini";

    // Initialize the stub singleton
    UBSettings::settings();
}

void TestUBSettings::cleanupTestCase()
{
    UBSettings::destroy();

    // Clean up temp .ini file
    QFile::remove(mTempIniPath);
}

void TestUBSettings::testDirectoryPathsNonEmpty()
{
    // Directory path methods should return non-empty valid paths
    QString dataDir = UBSettings::userDataDirectory();
    QVERIFY(!dataDir.isEmpty());
    QVERIFY(dataDir.contains("/"));

    QString docDir = UBSettings::userDocumentDirectory();
    QVERIFY(!docDir.isEmpty());
    QVERIFY(docDir.contains("/"));

    QString imgDir = UBSettings::userImageDirectory();
    QVERIFY(!imgDir.isEmpty());
    QVERIFY(imgDir.contains("/"));
}

void TestUBSettings::testSettingsReadWriteRoundtrip()
{
    // Test QSettings read/write roundtrip using a temp .ini file
    QSettings settings(mTempIniPath, QSettings::IniFormat);

    // Write values
    settings.setValue("Board/PenWidth", 3);
    settings.setValue("Board/MarkerWidth", 12);
    settings.setValue("App/Language", "fr");
    settings.setValue("Document/LastPath", "/home/teacher/documents");
    settings.sync();

    // Read back and verify
    QSettings readSettings(mTempIniPath, QSettings::IniFormat);
    QCOMPARE(readSettings.value("Board/PenWidth").toInt(), 3);
    QCOMPARE(readSettings.value("Board/MarkerWidth").toInt(), 12);
    QCOMPARE(readSettings.value("App/Language").toString(), QString("fr"));
    QCOMPARE(readSettings.value("Document/LastPath").toString(), QString("/home/teacher/documents"));
}

void TestUBSettings::testDefaultValueBehavior()
{
    // When a setting key doesn't exist, should return the provided default
    QSettings settings(mTempIniPath, QSettings::IniFormat);

    QVariant result = settings.value("NonExistent/Key", "default_value");
    QCOMPARE(result.toString(), QString("default_value"));

    QVariant intResult = settings.value("NonExistent/IntKey", 42);
    QCOMPARE(intResult.toInt(), 42);

    QVariant boolResult = settings.value("NonExistent/BoolKey", true);
    QCOMPARE(boolResult.toBool(), true);

    // Test with UBSetting class
    UBSetting* testSetting = new UBSetting(0, "TestDomain", "TestKey", QVariant(99));
    QCOMPARE(testSetting->get().toInt(), 99);  // Should return default

    testSetting->set(QVariant(42));
    QCOMPARE(testSetting->get().toInt(), 42);  // Should return set value

    // Reset should return to default
    testSetting->reset();
    QCOMPARE(testSetting->get().toInt(), 99);

    delete testSetting;
}

void TestUBSettings::testBoardColorSettings()
{
    // Test that board color-related settings work via QSettings
    QSettings settings(mTempIniPath, QSettings::IniFormat);

    // Write pen colors for dark background
    QColor penDark(Qt::yellow);
    settings.setValue("Board/PenColorOnDarkBackground/1", penDark.name());
    settings.sync();

    // Read back
    QSettings readSettings(mTempIniPath, QSettings::IniFormat);
    QString colorName = readSettings.value("Board/PenColorOnDarkBackground/1").toString();
    QColor readColor(colorName);
    QCOMPARE(readColor, penDark);

    // Write marker colors for light background
    QColor markerLight(255, 0, 0, 128);  // semi-transparent red
    settings.setValue("Board/MarkerColorOnLightBackground/1", markerLight.name());
    settings.sync();

    QSettings readSettings2(mTempIniPath, QSettings::IniFormat);
    QString markerColorName = readSettings2.value("Board/MarkerColorOnLightBackground/1").toString();
    QColor readMarkerColor(markerColorName);
    QCOMPARE(readMarkerColor.red(), 255);
    QCOMPARE(readMarkerColor.green(), 0);
    QCOMPARE(readMarkerColor.blue(), 0);
}

void TestUBSettings::testToolWidthSettings()
{
    // Test tool width settings via QSettings
    QSettings settings(mTempIniPath, QSettings::IniFormat);

    // Set pen widths
    settings.setValue("Board/PenFineWidth", 1.5);
    settings.setValue("Board/PenMediumWidth", 3.0);
    settings.setValue("Board/PenStrongWidth", 8.0);

    // Set marker widths
    settings.setValue("Board/MarkerFineWidth", 12.0);
    settings.setValue("Board/MarkerMediumWidth", 24.0);
    settings.setValue("Board/MarkerStrongWidth", 48.0);

    settings.sync();

    // Read back and verify
    QSettings readSettings(mTempIniPath, QSettings::IniFormat);
    QCOMPARE(readSettings.value("Board/PenFineWidth").toDouble(), 1.5);
    QCOMPARE(readSettings.value("Board/PenMediumWidth").toDouble(), 3.0);
    QCOMPARE(readSettings.value("Board/PenStrongWidth").toDouble(), 8.0);
    QCOMPARE(readSettings.value("Board/MarkerFineWidth").toDouble(), 12.0);
    QCOMPARE(readSettings.value("Board/MarkerMediumWidth").toDouble(), 24.0);
    QCOMPARE(readSettings.value("Board/MarkerStrongWidth").toDouble(), 48.0);
}

void TestUBSettings::testStaticConstantsDefined()
{
    // Verify static constants are properly defined and non-empty
    QVERIFY(!UBSettingsData::documentGroupName.isEmpty());
    QVERIFY(!UBSettingsData::documentName.isEmpty());
    QVERIFY(!UBSettingsData::documentSize.isEmpty());
    QVERIFY(!UBSettingsData::documentIdentifer.isEmpty());
    QVERIFY(!UBSettingsData::documentVersion.isEmpty());
    QVERIFY(!UBSettingsData::documentUpdatedAt.isEmpty());
    QVERIFY(!UBSettingsData::documentDate.isEmpty());

    QVERIFY(!UBSettingsData::sessionTitle.isEmpty());
    QVERIFY(!UBSettingsData::sessionAuthors.isEmpty());
    QVERIFY(!UBSettingsData::sessionObjectives.isEmpty());
    QVERIFY(!UBSettingsData::sessionKeywords.isEmpty());
    QVERIFY(!UBSettingsData::sessionGradeLevel.isEmpty());
    QVERIFY(!UBSettingsData::sessionSubjects.isEmpty());
    QVERIFY(!UBSettingsData::sessionType.isEmpty());
    QVERIFY(!UBSettingsData::sessionLicence.isEmpty());

    QVERIFY(!UBSettings::uniboardDocumentNamespaceUri.isEmpty());
    QVERIFY(UBSettings::uniboardDocumentNamespaceUri.startsWith("http"));

    QVERIFY(!UBSettingsData::documentDefaultBackgroundImage.isEmpty());
    QVERIFY(!UBSettingsData::documentDefaultBackgroundImageDisposition.isEmpty());
}

void TestUBSettings::testPageSizeSetting()
{
    UBSettings* settings = UBSettings::settings();
    QVERIFY(settings != 0);
    QVERIFY(settings->pageSize != 0);

    // Default page size should be valid
    QSize pageSize = settings->pageSize->get().toSize();
    QVERIFY(pageSize.width() > 0);
    QVERIFY(pageSize.height() > 0);

    // Default is 1380x1080
    QCOMPARE(pageSize, QSize(1380, 1080));

    // Setting a new page size should work
    settings->pageSize->set(QVariant(QSize(1920, 1080)));
    QCOMPARE(settings->pageSize->get().toSize(), QSize(1920, 1080));

    // Reset should return to default
    settings->pageSize->reset();
    QCOMPARE(settings->pageSize->get().toSize(), QSize(1380, 1080));
}
