#include "tst_UBDocumentProxy.h"
#include "stubs/UBDocumentProxy_stub.h"

#include <QDir>
#include <QUuid>

void TestUBDocumentProxy::initTestCase()
{
    // Ensure UBSettings stub singleton is initialized
    UBSettings::settings();
}

void TestUBDocumentProxy::cleanupTestCase()
{
    UBSettings::destroy();
}

void TestUBDocumentProxy::testMetadataGetSet()
{
    UBDocumentProxy proxy;

    // Test setting and retrieving metadata (typical teacher use case)
    proxy.setMetaData(UBSettings::documentName, QString::fromUtf8("Math\xc3\xa9matiques CE2"));
    QCOMPARE(proxy.metaData(UBSettings::documentName).toString(), QString::fromUtf8("Math\xc3\xa9matiques CE2"));

    proxy.setMetaData(UBSettings::sessionAuthors, "M. Dupont");
    QCOMPARE(proxy.metaData(UBSettings::sessionAuthors).toString(), QString("M. Dupont"));

    proxy.setMetaData(UBSettings::documentVersion, "4.7.0");
    QCOMPARE(proxy.metaData(UBSettings::documentVersion).toString(), QString("4.7.0"));

    // Test UUID stored in metadata
    QUuid testUuid = QUuid::createUuid();
    proxy.setUuid(testUuid);
    QCOMPARE(proxy.uuid(), testUuid);

    // Test creation date
    QString dateStr = "2024-01-15T10:30:00Z";
    proxy.setMetaData(UBSettings::documentDate, dateStr);
    QCOMPARE(proxy.metaData(UBSettings::documentDate).toString(), dateStr);
}

void TestUBDocumentProxy::testPageCountOperations()
{
    UBDocumentProxy proxy;

    // Initial page count is 0
    QCOMPARE(proxy.pageCount(), 0);

    // incPageCount from 0 should go to 1
    int result = proxy.incPageCount();
    QCOMPARE(result, 1);
    QCOMPARE(proxy.pageCount(), 1);

    // incPageCount from 1 should go to 2
    result = proxy.incPageCount();
    QCOMPARE(result, 2);
    QCOMPARE(proxy.pageCount(), 2);

    // incPageCount from 2 should go to 3
    result = proxy.incPageCount();
    QCOMPARE(result, 3);

    // decPageCount from 3 should go to 2
    result = proxy.decPageCount();
    QCOMPARE(result, 2);
    QCOMPARE(proxy.pageCount(), 2);

    // setPageCount explicitly
    proxy.setPageCount(10);
    QCOMPARE(proxy.pageCount(), 10);

    // decPageCount should not go below 0
    proxy.setPageCount(0);
    result = proxy.decPageCount();
    QCOMPARE(result, 0);
    QCOMPARE(proxy.pageCount(), 0);
}

void TestUBDocumentProxy::testPersistencePathConstruction()
{
    QString testPath = QDir::tempPath() + "/test_doc_" + QUuid::createUuid().toString();
    UBDocumentProxy proxy(testPath);

    // Path should be non-empty
    QVERIFY(!proxy.persistencePath().isEmpty());

    // Path should match what was set
    QCOMPARE(proxy.persistencePath(), testPath);

    // Changing path should update
    QString newPath = QDir::tempPath() + "/new_path_" + QUuid::createUuid().toString();
    proxy.setPersistencePath(newPath);
    QCOMPARE(proxy.persistencePath(), newPath);

    // Document should be marked as modified after path change
    QVERIFY(proxy.isModified());
}

void TestUBDocumentProxy::testUuidGenerationUniqueness()
{
    UBDocumentProxy proxy1;
    UBDocumentProxy proxy2;
    UBDocumentProxy proxy3;

    // Each proxy should have a unique UUID
    QVERIFY(proxy1.uuid() != proxy2.uuid());
    QVERIFY(proxy2.uuid() != proxy3.uuid());
    QVERIFY(proxy1.uuid() != proxy3.uuid());

    // UUIDs should be valid (non-null)
    QVERIFY(!proxy1.uuid().isNull());
    QVERIFY(!proxy2.uuid().isNull());
    QVERIFY(!proxy3.uuid().isNull());
}

void TestUBDocumentProxy::testCopyConstructor()
{
    UBDocumentProxy original;
    original.setPersistencePath("/tmp/test_original");
    original.setMetaData(UBSettings::documentName, "Test Document");
    original.setPageCount(5);

    UBDocumentProxy copy(original);

    // Copy should have same persistence path
    QCOMPARE(copy.persistencePath(), original.persistencePath());

    // Copy should have same metadata
    QCOMPARE(copy.metaData(UBSettings::documentName).toString(), QString("Test Document"));

    // Copy should have same page count
    QCOMPARE(copy.pageCount(), 5);
}

void TestUBDocumentProxy::testDefaultDocumentSize()
{
    UBDocumentProxy proxy;

    // Default size should come from UBSettings stub (1380x1080)
    QSize defaultSize = proxy.defaultDocumentSize();
    QVERIFY(defaultSize.width() > 0);
    QVERIFY(defaultSize.height() > 0);

    // Setting a new size should emit signal
    QSignalSpy spy(&proxy, SIGNAL(defaultDocumentSizeChanged()));
    proxy.setDefaultDocumentSize(QSize(1920, 1080));
    QCOMPARE(proxy.defaultDocumentSize(), QSize(1920, 1080));
    QCOMPARE(spy.count(), 1);

    // Setting same size should not emit signal again
    proxy.setDefaultDocumentSize(QSize(1920, 1080));
    QCOMPARE(spy.count(), 1);
}

void TestUBDocumentProxy::testDocumentName()
{
    UBDocumentProxy proxy;

    // Name should be initialized (default is current date/time string)
    QVERIFY(!proxy.name().isEmpty());

    // Set a custom name
    proxy.setMetaData(UBSettings::documentName, QString::fromUtf8("Le\xc3\xa7on de g\xc3\xa9ographie"));
    QCOMPARE(proxy.name(), QString::fromUtf8("Le\xc3\xa7on de g\xc3\xa9ographie"));

    // Group name should be initially empty
    QCOMPARE(proxy.groupName(), QString(""));
}

void TestUBDocumentProxy::testTheSameDocument()
{
    UBDocumentProxy proxy1;
    proxy1.setPersistencePath("/tmp/doc_a");

    UBDocumentProxy proxy2;
    proxy2.setPersistencePath("/tmp/doc_a");

    UBDocumentProxy proxy3;
    proxy3.setPersistencePath("/tmp/doc_b");

    // Same path means same document
    QVERIFY(proxy1.theSameDocument(&proxy2));
    // Different path means different document
    QVERIFY(!proxy1.theSameDocument(&proxy3));
    // Null proxy should return false
    QVERIFY(!proxy1.theSameDocument(NULL));
}

void TestUBDocumentProxy::testDocumentDateAndLastUpdate()
{
    UBDocumentProxy proxy;

    // documentDate with no metadata should return current time (non-null)
    QDateTime date = proxy.documentDate();
    QVERIFY(date.isValid());

    // lastUpdate with no metadata should return current time (non-null)
    QDateTime update = proxy.lastUpdate();
    QVERIFY(update.isValid());

    // Set a date via metadata and verify retrieval
    QString isoDate = "2024-03-15T14:30:00Z";
    proxy.setMetaData(UBSettings::documentDate, isoDate);
    QDateTime docDate = proxy.documentDate();
    QVERIFY(docDate.isValid());

    // Set updatedAt and verify
    proxy.setMetaData(UBSettings::documentUpdatedAt, isoDate);
    QDateTime lastUpd = proxy.lastUpdate();
    QVERIFY(lastUpd.isValid());

    // Test metaDatas() returns full hash
    QHash<QString, QVariant> allMeta = proxy.metaDatas();
    QVERIFY(allMeta.contains(UBSettings::documentDate));
    QVERIFY(allMeta.contains(UBSettings::documentUpdatedAt));
}

void TestUBDocumentProxy::testDefaultImageBackground()
{
    UBDocumentProxy proxy;

    // Initially no default image background
    QCOMPARE(proxy.hasDefaultImageBackground(), false);

    // Set default image background flag
    proxy.setHasDefaultImageBackground(true);
    QCOMPARE(proxy.hasDefaultImageBackground(), true);

    // Reset it
    proxy.setHasDefaultImageBackground(false);
    QCOMPARE(proxy.hasDefaultImageBackground(), false);

    // Test setDefaultDocumentSize(int, int) overload
    QSignalSpy spy(&proxy, SIGNAL(defaultDocumentSizeChanged()));
    proxy.setDefaultDocumentSize(1024, 768);
    QCOMPARE(proxy.defaultDocumentSize(), QSize(1024, 768));
    QCOMPARE(spy.count(), 1);
}

void TestUBDocumentProxy::testExternalFiles()
{
    UBDocumentProxy proxy;

    // Initially empty
    QCOMPARE(proxy.externalFiles()->size(), 0);

    // Add external files
    UBDocumentExternalFile* file1 = new UBDocumentExternalFile();
    file1->setTitle("Guide enseignant");
    file1->setPath("/docs/guide.pdf");

    UBDocumentExternalFile* file2 = new UBDocumentExternalFile();
    file2->setTitle("Exercice");
    file2->setPath("/docs/exercice.odt");

    proxy.externalFilesAdd(file1);
    proxy.externalFilesAdd(file2);

    QCOMPARE(proxy.externalFiles()->size(), 2);
    QCOMPARE(proxy.externalFiles()->at(0)->title(), QString("Guide enseignant"));
    QCOMPARE(proxy.externalFiles()->at(1)->path(), QString("/docs/exercice.odt"));
}

void TestUBDocumentProxy::testExternalFilesClear_noLeak()
{
    UBDocumentProxy proxy;

    // Add files then clear — should not leak (files are deleted)
    for (int i = 0; i < 100; ++i) {
        UBDocumentExternalFile* file = new UBDocumentExternalFile();
        file->setTitle(QString("File %1").arg(i));
        file->setPath(QString("/path/%1.pdf").arg(i));
        proxy.externalFilesAdd(file);
    }
    QCOMPARE(proxy.externalFiles()->size(), 100);

    proxy.externalFilesClear();
    QCOMPARE(proxy.externalFiles()->size(), 0);

    // Can add again after clear
    UBDocumentExternalFile* newFile = new UBDocumentExternalFile();
    newFile->setTitle("New");
    newFile->setPath("/new.pdf");
    proxy.externalFilesAdd(newFile);
    QCOMPARE(proxy.externalFiles()->size(), 1);
}

void TestUBDocumentProxy::testSettingsInjection()
{
    UBDocumentProxy proxy;

    // By default, settings() returns the global singleton
    QVERIFY(proxy.settings() != nullptr);
    QCOMPARE(proxy.settings(), UBSettings::settings());

    // Create a separate settings instance for injection
    UBSettings customSettings;
    customSettings.pageSize->set(QVariant(QSize(800, 600)));

    // Inject custom settings
    proxy.setSettings(&customSettings);
    QCOMPARE(proxy.settings(), &customSettings);

    // Restore default
    proxy.setSettings(UBSettings::settings());
    QCOMPARE(proxy.settings(), UBSettings::settings());
}

void TestUBDocumentProxy::testDefaultSizeUsesInjectedSettings()
{
    // Create a custom settings with a specific page size
    UBSettings customSettings;
    customSettings.pageSize->set(QVariant(QSize(640, 480)));

    UBDocumentProxy proxy;

    // Inject the custom settings
    proxy.setSettings(&customSettings);

    // Clear the document size metadata so it falls back to settings
    proxy.setMetaData(UBSettings::documentSize, QVariant());

    // defaultDocumentSize() should now use the injected settings
    QSize size = proxy.defaultDocumentSize();
    QCOMPARE(size, QSize(640, 480));

    // Change the injected settings
    customSettings.pageSize->set(QVariant(QSize(1920, 1080)));
    proxy.setMetaData(UBSettings::documentSize, QVariant());
    size = proxy.defaultDocumentSize();
    QCOMPARE(size, QSize(1920, 1080));
}

void TestUBDocumentProxy::testInitUsesSettingsPageSize()
{
    // The default proxy init should use UBSettings::settings()->pageSize
    UBDocumentProxy proxy;

    QSize expectedSize = UBSettings::settings()->pageSize->get().toSize();
    QCOMPARE(proxy.defaultDocumentSize(), expectedSize);

    // Now change the singleton's pageSize and create a new proxy
    UBSettings::settings()->pageSize->set(QVariant(QSize(2560, 1440)));
    UBDocumentProxy proxy2;
    QCOMPARE(proxy2.defaultDocumentSize(), QSize(2560, 1440));

    // Restore default for other tests
    UBSettings::settings()->pageSize->set(QVariant(QSize(1280, 960)));
}

void TestUBDocumentProxy::testMultipleProxiesDifferentSettings()
{
    // Create two settings instances with different page sizes
    UBSettings settingsA;
    settingsA.pageSize->set(QVariant(QSize(800, 600)));

    UBSettings settingsB;
    settingsB.pageSize->set(QVariant(QSize(3840, 2160)));

    // Create proxies and inject different settings
    UBDocumentProxy proxyA;
    proxyA.setSettings(&settingsA);
    proxyA.setMetaData(UBSettings::documentSize, QVariant()); // clear cached size

    UBDocumentProxy proxyB;
    proxyB.setSettings(&settingsB);
    proxyB.setMetaData(UBSettings::documentSize, QVariant()); // clear cached size

    // Each proxy should use its own settings
    QCOMPARE(proxyA.defaultDocumentSize(), QSize(800, 600));
    QCOMPARE(proxyB.defaultDocumentSize(), QSize(3840, 2160));

    // They don't interfere with each other
    QVERIFY(proxyA.defaultDocumentSize() != proxyB.defaultDocumentSize());
}
