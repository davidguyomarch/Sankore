#include "tst_UBMetadataDcSubsetAdaptor.h"
#include "core/UBSettingsData.h"
#include "adaptors/UBMetadataLoader.h"
#include "stubs/UBSettings_stub.h"
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

static QString writeMetadataRdf(const QString& dirPath, const QString& content)
{
    QFile file(dirPath + "/metadata.rdf");
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream out(&file);
    out << content;
    file.close();
    return dirPath;
}

static QString minimalRdf(const QString& title = "Test Document",
                          const QString& type = "My Group",
                          const QString& date = "2024-01-15T10:30:00Z",
                          const QString& size = "",
                          const QString& version = "",
                          const QString& updatedAt = "",
                          const QString& identifier = "")
{
    QString xml;
    xml += "<?xml version=\"1.0\"?>\n";
    xml += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" ";
    xml += "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" ";
    xml += "xmlns:ub=\"http://uniboard.mnemis.com/document\">\n";
    xml += "<rdf:Description rdf:about=\"urn:test:doc\">\n";
    xml += "  <dc:title>" + title + "</dc:title>\n";
    xml += "  <dc:type>" + type + "</dc:type>\n";
    xml += "  <dc:date>" + date + "</dc:date>\n";
    xml += "  <dc:format>image/svg+xml</dc:format>\n";
    if (!identifier.isEmpty())
        xml += "  <dc:identifier>" + identifier + "</dc:identifier>\n";
    if (!version.isEmpty())
        xml += "  <ub:version>" + version + "</ub:version>\n";
    if (!size.isEmpty())
        xml += "  <ub:size>" + size + "</ub:size>\n";
    if (!updatedAt.isEmpty())
        xml += "  <ub:updated-at>" + updatedAt + "</ub:updated-at>\n";
    xml += "</rdf:Description>\n";
    xml += "</rdf:RDF>\n";
    return xml;
}

void TestUBMetadataDcSubsetAdaptor::testLoadNonExistentPath()
{
    QMap<QString, QVariant> meta = UBMetadataLoader::load("/nonexistent/path");
    // Should return metadata with default size (no file found)
    QVERIFY(meta.contains(UBSettingsData::documentSize));
}

void TestUBMetadataDcSubsetAdaptor::testLoadBasicMetadata()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeMetadataRdf(dir.path(), minimalRdf("Mon cours de maths", "Sciences", "2024-03-20T14:00:00Z"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QCOMPARE(meta.value(UBSettingsData::documentName).toString(), QString("Mon cours de maths"));
    QCOMPARE(meta.value(UBSettingsData::documentGroupName).toString(), QString("Sciences"));
    QCOMPARE(meta.value(UBSettingsData::documentDate).toString(), QString("2024-03-20T14:00:00Z"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadDocumentSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01-01", "1920x1080"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QVERIFY(meta.contains(UBSettingsData::documentSize));
    QSize size = meta.value(UBSettingsData::documentSize).toSize();
    QCOMPARE(size.width(), 1920);
    QCOMPARE(size.height(), 1080);
}

void TestUBMetadataDcSubsetAdaptor::testLoadSizeMigration1024x768()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Old documents with 1024x768 should be migrated to pageSize setting
    writeMetadataRdf(dir.path(), minimalRdf("Old Doc", "Group", "2012-01-01", "1024x768"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QSize size = meta.value(UBSettingsData::documentSize).toSize();
    // Should use UBSettings::settings()->pageSize (default in stub is 1280x960)
    QSize expectedSize = UBSettings::settings()->pageSize->get().toSize();
    QCOMPARE(size, expectedSize);
}

void TestUBMetadataDcSubsetAdaptor::testLoadVersion()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01-01", "1920x1080", "4.7"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QCOMPARE(meta.value(UBSettingsData::documentVersion).toString(), QString("4.7"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadUpdatedAt()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01-01", "1920x1080", "", "2024-06-15T09:30:00Z"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QCOMPARE(meta.value(UBSettingsData::documentUpdatedAt).toString(), QString("2024-06-15T09:30:00Z"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadSessionFields()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString xml;
    xml += "<?xml version=\"1.0\"?>\n";
    xml += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" ";
    xml += "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" ";
    xml += "xmlns:ub=\"http://uniboard.mnemis.com/document\">\n";
    xml += "<rdf:Description>\n";
    xml += "  <dc:title>Session Test</dc:title>\n";
    xml += "  <dc:date>2024-01-01</dc:date>\n";
    xml += "  <ub:size>1920x1080</ub:size>\n";
    xml += "  <ub:SessionTitle>Introduction aux fractions</ub:SessionTitle>\n";
    xml += "  <ub:SessionAuthors>M. Dupont</ub:SessionAuthors>\n";
    xml += "  <ub:SessionObjectives>Comprendre les fractions</ub:SessionObjectives>\n";
    xml += "  <ub:SessionKeywords>maths,fractions</ub:SessionKeywords>\n";
    xml += "  <ub:SessionGradeLevel>CM2</ub:SessionGradeLevel>\n";
    xml += "  <ub:SessionSubjects>Mathematiques</ub:SessionSubjects>\n";
    xml += "  <ub:SessionType>Cours</ub:SessionType>\n";
    xml += "  <ub:SessionLicence>CC-BY-SA</ub:SessionLicence>\n";
    xml += "</rdf:Description>\n";
    xml += "</rdf:RDF>\n";

    writeMetadataRdf(dir.path(), xml);

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QCOMPARE(meta.value(UBSettingsData::sessionTitle).toString(), QString("Introduction aux fractions"));
    QCOMPARE(meta.value(UBSettingsData::sessionAuthors).toString(), QString("M. Dupont"));
    QCOMPARE(meta.value(UBSettingsData::sessionObjectives).toString(), QString("Comprendre les fractions"));
    QCOMPARE(meta.value(UBSettingsData::sessionKeywords).toString(), QString("maths,fractions"));
    QCOMPARE(meta.value(UBSettingsData::sessionGradeLevel).toString(), QString("CM2"));
    QCOMPARE(meta.value(UBSettingsData::sessionSubjects).toString(), QString("Mathematiques"));
    QCOMPARE(meta.value(UBSettingsData::sessionType).toString(), QString("Cours"));
    QCOMPARE(meta.value(UBSettingsData::sessionLicence).toString(), QString("CC-BY-SA"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadDateFormatShort()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Short date (< 10 chars) should get T00:00:00Z appended
    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01", "1920x1080"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    // The documentDate is set from the raw date element at the end
    QCOMPARE(meta.value(UBSettingsData::documentDate).toString(), QString("2024-01"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadMissingSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // No size in XML — should use default screen size
    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01-01"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QVERIFY(meta.contains(UBSettingsData::documentSize));
    QSize size = meta.value(UBSettingsData::documentSize).toSize();
    QVERIFY(size.width() > 0);
    QVERIFY(size.height() > 0);
}

void TestUBMetadataDcSubsetAdaptor::testLoadIdentifier()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01-01", "1920x1080", "", "", "urn:uuid:12345-abcde"));

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QCOMPARE(meta.value(UBSettingsData::documentIdentifer).toString(), QString("urn:uuid:12345-abcde"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadBackgroundImage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString xml;
    xml += "<?xml version=\"1.0\"?>\n";
    xml += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" ";
    xml += "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" ";
    xml += "xmlns:ub=\"http://uniboard.mnemis.com/document\">\n";
    xml += "<rdf:Description>\n";
    xml += "  <dc:title>BG Test</dc:title>\n";
    xml += "  <dc:date>2024-01-01</dc:date>\n";
    xml += "  <ub:size>1920x1080</ub:size>\n";
    xml += "  <ub:DocumentDefaultBackgroundImage>images/bg.png</ub:DocumentDefaultBackgroundImage>\n";
    xml += "  <ub:DocumentDefaultBackgroundImageDisposition>adjust</ub:DocumentDefaultBackgroundImageDisposition>\n";
    xml += "</rdf:Description>\n";
    xml += "</rdf:RDF>\n";

    writeMetadataRdf(dir.path(), xml);

    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path());

    QCOMPARE(meta.value(UBSettingsData::documentDefaultBackgroundImage).toString(), QString("images/bg.png"));
    QCOMPARE(meta.value(UBSettingsData::documentDefaultBackgroundImageDisposition).toString(), QString("adjust"));
}

void TestUBMetadataDcSubsetAdaptor::testLoadSizeMigrationWithCustomSettings()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Document with old 1024x768 size — should be migrated to settings pageSize
    writeMetadataRdf(dir.path(), minimalRdf("Old Doc", "Group", "2012-01-01", "1024x768"));

    // Create custom settings with a non-default page size
    UBSettings customSettings;
    customSettings.pageSize->set(QVariant(QSize(1600, 1200)));

    // Load with injected custom settings
    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path(), &customSettings);

    // Should use the CUSTOM pageSize (1600x1200), not the default (1280x960)
    QSize size = meta.value(UBSettingsData::documentSize).toSize();
    QCOMPARE(size, QSize(1600, 1200));
}

void TestUBMetadataDcSubsetAdaptor::testLoadWithNullSettings()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Document with old 1024x768 size
    writeMetadataRdf(dir.path(), minimalRdf("Test", "Group", "2024-01-01", "1024x768"));

    // Load with nullptr settings — should fall back to singleton
    QMap<QString, QVariant> meta = UBMetadataLoader::load(dir.path(), nullptr);

    // Should use the singleton's default pageSize
    QSize size = meta.value(UBSettingsData::documentSize).toSize();
    QSize expectedSize = UBSettings::settings()->pageSize->get().toSize();
    QCOMPARE(size, expectedSize);
}
