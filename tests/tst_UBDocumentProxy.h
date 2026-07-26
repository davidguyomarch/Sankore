#ifndef TST_UBDOCUMENTPROXY_H
#define TST_UBDOCUMENTPROXY_H

#include <QtTest>
#include <QObject>

class TestUBDocumentProxy : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testMetadataGetSet();
    void testPageCountOperations();
    void testPersistencePathConstruction();
    void testUuidGenerationUniqueness();
    void testCopyConstructor();
    void testDefaultDocumentSize();
    void testDocumentName();
    void testTheSameDocument();
    void testDocumentDateAndLastUpdate();
    void testDefaultImageBackground();
    void testExternalFiles();
    void testExternalFilesClear_noLeak();
    void testSettingsInjection();
    void testDefaultSizeUsesInjectedSettings();
    void testInitUsesSettingsPageSize();
    void testMultipleProxiesDifferentSettings();
};

#endif // TST_UBDOCUMENTPROXY_H
