#ifndef TST_UBFOREIGNOBJECTSHELPER_H
#define TST_UBFOREIGNOBJECTSHELPER_H

#include <QObject>
#include <QtTest>

class TestUBForeignObjectsHelper : public QObject
{
    Q_OBJECT

private slots:
    void testStrIdFrom_validUuid();
    void testStrIdFrom_noUuid();
    void testStrIdFrom_emptyString();
    void testStrIdFrom_multipleUuids();
    void testRmR_file();
    void testRmR_directory();
    void testRmR_nonExistent();
    void testCpRf_file();
    void testCpRf_directory();
    void testCpRf_nonExistent();
    void testGetSceneFileNames_empty();
    void testGetSceneFileNames_withPages();
};

#endif // TST_UBFOREIGNOBJECTSHELPER_H
