#include "tst_UBForeignObjectsHelper.h"
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QRegularExpression>

// Standalone implementations (same as UBForeignObjectsHandler.cpp static functions)

static QString strIdFrom(const QString &filePath)
{
    if (filePath.isEmpty())
        return QString();

    QRegularExpression rx("\\{.(?!.*\\{).*\\}");
    QRegularExpressionMatch match = rx.match(filePath);
    if (!match.hasMatch())
        return QString();

    return match.captured();
}

static bool rm_r(const QString &rmPath)
{
    QFileInfo fi(rmPath);
    if (!fi.exists()) return false;
    if (fi.isFile()) return QFile::remove(rmPath);
    if (fi.isDir()) {
        QFileInfoList fList = QDir(rmPath).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (const QFileInfo& sub : fList)
            rm_r(sub.absoluteFilePath());
        return QDir().rmdir(rmPath);
    }
    return false;
}

static bool cp_rf(const QString &what, const QString &where)
{
    QFileInfo whatFi(what);
    QFileInfo whereFi(where);

    if (!whatFi.exists()) return false;
    if (whatFi.isFile()) {
        QString whereDir = where.section("/", 0, -2, QString::SectionSkipEmpty | QString::SectionIncludeLeadingSep);
        QString newFilePath = where;
        if (!whereFi.exists())
            QDir().mkpath(whereDir);
        else if (whereFi.isDir())
            newFilePath = whereDir + "/" + whatFi.fileName();
        if (QFile::exists(newFilePath))
            QFile::remove(newFilePath);
        return QFile::copy(what, newFilePath);
    }
    if (whatFi.isDir()) {
        if (whereFi.isFile()) return false;
        if (whereFi.isDir()) rm_r(where);
        QDir().mkpath(where);
        QFileInfoList fList = QDir(what).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (const QFileInfo& sub : fList) {
            if (!cp_rf(sub.absoluteFilePath(), where + "/" + sub.fileName()))
                return false;
        }
        return true;
    }
    return true;
}

static QStringList getSceneFileNames(const QString& folder)
{
    QDir dir(folder, "page???.svg", QDir::Name, QDir::Files);
    return dir.entryList();
}

// --- Tests ---

void TestUBForeignObjectsHelper::testStrIdFrom_validUuid()
{
    QString path = "/documents/images/{12345678-abcd-1234-abcd-123456789abc}.png";
    QString result = strIdFrom(path);
    QVERIFY(result.startsWith("{"));
    QVERIFY(result.endsWith("}"));
    QVERIFY(result.contains("12345678"));
}

void TestUBForeignObjectsHelper::testStrIdFrom_noUuid()
{
    QString path = "/documents/images/photo.png";
    QString result = strIdFrom(path);
    QVERIFY(result.isEmpty());
}

void TestUBForeignObjectsHelper::testStrIdFrom_emptyString()
{
    QVERIFY(strIdFrom("").isEmpty());
}

void TestUBForeignObjectsHelper::testStrIdFrom_multipleUuids()
{
    // Should capture the LAST UUID (regex uses negative lookahead for {)
    QString path = "/docs/{aaaa-bbbb}/{cccc-dddd}.svg";
    QString result = strIdFrom(path);
    QVERIFY(result.contains("cccc-dddd"));
}

void TestUBForeignObjectsHelper::testRmR_file()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString filePath = dir.path() + "/test.txt";
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.write("test");
    f.close();

    QVERIFY(QFile::exists(filePath));
    QVERIFY(rm_r(filePath));
    QVERIFY(!QFile::exists(filePath));
}

void TestUBForeignObjectsHelper::testRmR_directory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString subDir = dir.path() + "/sub";
    QDir().mkpath(subDir);
    QFile f(subDir + "/inner.txt");
    f.open(QIODevice::WriteOnly);
    f.write("data");
    f.close();

    QVERIFY(QDir(subDir).exists());
    QVERIFY(rm_r(subDir));
    QVERIFY(!QDir(subDir).exists());
}

void TestUBForeignObjectsHelper::testRmR_nonExistent()
{
    QVERIFY(!rm_r("/nonexistent/path/that/does/not/exist"));
}

void TestUBForeignObjectsHelper::testCpRf_file()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString src = dir.path() + "/source.txt";
    QString dst = dir.path() + "/dest.txt";

    QFile f(src);
    f.open(QIODevice::WriteOnly);
    f.write("hello");
    f.close();

    QVERIFY(cp_rf(src, dst));
    QVERIFY(QFile::exists(dst));
}

void TestUBForeignObjectsHelper::testCpRf_directory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString srcDir = dir.path() + "/srcdir";
    QString dstDir = dir.path() + "/dstdir";
    QDir().mkpath(srcDir);

    QFile f(srcDir + "/file.txt");
    f.open(QIODevice::WriteOnly);
    f.write("content");
    f.close();

    QVERIFY(cp_rf(srcDir, dstDir));
    QVERIFY(QFile::exists(dstDir + "/file.txt"));
}

void TestUBForeignObjectsHelper::testCpRf_nonExistent()
{
    QVERIFY(!cp_rf("/nonexistent/file.txt", "/tmp/dest.txt"));
}

void TestUBForeignObjectsHelper::testGetSceneFileNames_empty()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QStringList files = getSceneFileNames(dir.path());
    QCOMPARE(files.size(), 0);
}

void TestUBForeignObjectsHelper::testGetSceneFileNames_withPages()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Create page files matching the pattern page???.svg
    for (int i = 0; i < 5; i++) {
        QFile f(dir.path() + QString("/page%1.svg").arg(i, 3, 10, QChar('0')));
        f.open(QIODevice::WriteOnly);
        f.write("<svg/>");
        f.close();
    }
    // Create a non-matching file
    QFile other(dir.path() + "/thumbnail.png");
    other.open(QIODevice::WriteOnly);
    other.write("png");
    other.close();

    QStringList files = getSceneFileNames(dir.path());
    QCOMPARE(files.size(), 5);
    QCOMPARE(files.at(0), QString("page000.svg"));
    QCOMPARE(files.at(4), QString("page004.svg"));
}
