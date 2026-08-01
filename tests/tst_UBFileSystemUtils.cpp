#include "tst_UBFileSystemUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

void TestUBFileSystemUtils::initTestCase()
{
    mTempDir = QDir::tempPath() + "/ub_test_" + QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(mTempDir);
}

void TestUBFileSystemUtils::cleanupTestCase()
{
    UBFileSystemUtils::deleteDir(mTempDir);
}

// --- removeLocalFilePrefix ---

void TestUBFileSystemUtils::testRemoveLocalFilePrefix_unix()
{
    QString input = "file:///home/user/doc.pdf";
    QString result = UBFileSystemUtils::removeLocalFilePrefix(input);
    QCOMPARE(result, QString("/home/user/doc.pdf"));
}

void TestUBFileSystemUtils::testRemoveLocalFilePrefix_noPrefix()
{
    QString input = "/home/user/doc.pdf";
    QString result = UBFileSystemUtils::removeLocalFilePrefix(input);
    QCOMPARE(result, QString("/home/user/doc.pdf"));
}

void TestUBFileSystemUtils::testRemoveLocalFilePrefix_empty()
{
    QString input = "";
    QString result = UBFileSystemUtils::removeLocalFilePrefix(input);
    QCOMPARE(result, QString(""));
}

// --- cleanName ---

void TestUBFileSystemUtils::testCleanName_removesSlash()
{
    QCOMPARE(UBFileSystemUtils::cleanName("my/file"), QString("myfile"));
}

void TestUBFileSystemUtils::testCleanName_removesColon()
{
    QCOMPARE(UBFileSystemUtils::cleanName("file:name"), QString("filename"));
}

void TestUBFileSystemUtils::testCleanName_removesMultipleChars()
{
    QString input = "file<name>with|bad*chars?";
    QString result = UBFileSystemUtils::cleanName(input);
    QVERIFY(!result.contains("<"));
    QVERIFY(!result.contains(">"));
    QVERIFY(!result.contains("|"));
    QVERIFY(!result.contains("*"));
    QVERIFY(!result.contains("?"));
    QCOMPARE(result, QString("filenamewithbadchars"));
}

void TestUBFileSystemUtils::testCleanName_preservesNormal()
{
    QString input = "My Document 2013";
    QCOMPARE(UBFileSystemUtils::cleanName(input), input);
}

// --- normalizeFilePath ---

void TestUBFileSystemUtils::testNormalizeFilePath_backslashes()
{
    QString input = "C:\\Users\\test\\file.txt";
    QString result = UBFileSystemUtils::normalizeFilePath(input);
    QCOMPARE(result, QString("C:/Users/test/file.txt"));
}

void TestUBFileSystemUtils::testNormalizeFilePath_alreadyNormalized()
{
    QString input = "/home/user/file.txt";
    QCOMPARE(UBFileSystemUtils::normalizeFilePath(input), input);
}

// --- extension ---

void TestUBFileSystemUtils::testExtension_simple()
{
    QCOMPARE(UBFileSystemUtils::extension("document.pdf"), QString("pdf"));
}

void TestUBFileSystemUtils::testExtension_multiple_dots()
{
    QCOMPARE(UBFileSystemUtils::extension("archive.tar.gz"), QString("gz"));
}

void TestUBFileSystemUtils::testExtension_noExtension()
{
    QCOMPARE(UBFileSystemUtils::extension("Makefile"), QString(""));
}

void TestUBFileSystemUtils::testExtension_trailingSlash()
{
    // Extension with trailing slash should be cleaned
    QString result = UBFileSystemUtils::extension("file.txt/");
    QCOMPARE(result, QString("txt"));
}

// --- lastPathComponent ---

void TestUBFileSystemUtils::testLastPathComponent_file()
{
    QCOMPARE(UBFileSystemUtils::lastPathComponent("/home/user/file.txt"), QString("file.txt"));
}

void TestUBFileSystemUtils::testLastPathComponent_dir()
{
    QCOMPARE(UBFileSystemUtils::lastPathComponent("/home/user/documents"), QString("documents"));
}

void TestUBFileSystemUtils::testLastPathComponent_trailingSlash()
{
    QCOMPARE(UBFileSystemUtils::lastPathComponent("/home/user/documents/"), QString("documents"));
}

// --- mimeTypeFromFileName ---

void TestUBFileSystemUtils::testMimeTypeFromFileName_png()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromFileName("image.png"), QString("image/png"));
}

void TestUBFileSystemUtils::testMimeTypeFromFileName_pdf()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromFileName("doc.pdf"), QString("application/pdf"));
}

void TestUBFileSystemUtils::testMimeTypeFromFileName_svg()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromFileName("drawing.svg"), QString("image/svg+xml"));
}

void TestUBFileSystemUtils::testMimeTypeFromFileName_mp4()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromFileName("video.mp4"), QString("video/mp4"));
}

void TestUBFileSystemUtils::testMimeTypeFromFileName_unknown()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromFileName("file.xyz"), QString(""));
}

// --- fileExtensionFromMimeType ---

void TestUBFileSystemUtils::testFileExtensionFromMimeType_image()
{
    QCOMPARE(UBFileSystemUtils::fileExtensionFromMimeType("image/png"), QString("png"));
    QCOMPARE(UBFileSystemUtils::fileExtensionFromMimeType("image/jpeg"), QString("jpeg"));
}

void TestUBFileSystemUtils::testFileExtensionFromMimeType_video()
{
    QCOMPARE(UBFileSystemUtils::fileExtensionFromMimeType("video/mp4"), QString("mp4"));
}

void TestUBFileSystemUtils::testFileExtensionFromMimeType_unknown()
{
    QCOMPARE(UBFileSystemUtils::fileExtensionFromMimeType("application/x-unknown"), QString(""));
}

// --- mimeTypeFromString ---

void TestUBFileSystemUtils::testMimeTypeFromString_rasterImage()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("image/png"), UBMimeType::RasterImage);
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("image/jpeg"), UBMimeType::RasterImage);
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("image/gif"), UBMimeType::RasterImage);
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("image/bmp"), UBMimeType::RasterImage);
}

void TestUBFileSystemUtils::testMimeTypeFromString_vectorImage()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("image/svg+xml"), UBMimeType::VectorImage);
}

void TestUBFileSystemUtils::testMimeTypeFromString_video()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("video/mp4"), UBMimeType::Video);
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("video/quicktime"), UBMimeType::Video);
}

void TestUBFileSystemUtils::testMimeTypeFromString_audio()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("audio/mpeg"), UBMimeType::Audio);
}

void TestUBFileSystemUtils::testMimeTypeFromString_pdf()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("application/pdf"), UBMimeType::PDF);
}

void TestUBFileSystemUtils::testMimeTypeFromString_unknown()
{
    QCOMPARE(UBFileSystemUtils::mimeTypeFromString("application/x-unknown"), UBMimeType::UNKNOWN);
}

// --- isPathMatch ---

void TestUBFileSystemUtils::testIsPathMatch_exact()
{
    QVERIFY(UBFileSystemUtils::isPathMatch("/root/folder", "/root/folder"));
}

void TestUBFileSystemUtils::testIsPathMatch_subpath()
{
    QVERIFY(UBFileSystemUtils::isPathMatch("/root/folder/sub", "/root/folder"));
}

void TestUBFileSystemUtils::testIsPathMatch_noMatch()
{
    QVERIFY(!UBFileSystemUtils::isPathMatch("/root/other", "/root/folder"));
}

void TestUBFileSystemUtils::testIsPathMatch_similarPrefix()
{
    // This is the key test: /root/folder1 should NOT match /root/folder
    QVERIFY(!UBFileSystemUtils::isPathMatch("/root/folder1", "/root/folder"));
}

// --- File operations ---

void TestUBFileSystemUtils::testCopyFile()
{
    QString srcPath = mTempDir + "/source.txt";
    QString dstPath = mTempDir + "/dest.txt";

    QFile src(srcPath);
    src.open(QIODevice::WriteOnly);
    src.write("Hello World");
    src.close();

    bool result = UBFileSystemUtils::copyFile(srcPath, dstPath);
    QVERIFY(result);
    QVERIFY(QFile::exists(dstPath));

    QFile::remove(srcPath);
    QFile::remove(dstPath);
}

void TestUBFileSystemUtils::testCopyFile_nonExistent()
{
    bool result = UBFileSystemUtils::copyFile("/nonexistent/file.txt", mTempDir + "/out.txt");
    QVERIFY(!result);
}

void TestUBFileSystemUtils::testDeleteFile()
{
    QString filePath = mTempDir + "/to_delete.txt";
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.write("delete me");
    f.close();

    QVERIFY(QFile::exists(filePath));
    bool result = UBFileSystemUtils::deleteFile(filePath);
    QVERIFY(result);
    QVERIFY(!QFile::exists(filePath));
}

void TestUBFileSystemUtils::testDeleteDir()
{
    QString dirPath = mTempDir + "/dir_to_delete";
    QDir().mkpath(dirPath + "/subdir");

    QFile f(dirPath + "/file.txt");
    f.open(QIODevice::WriteOnly);
    f.write("content");
    f.close();

    QFile f2(dirPath + "/subdir/nested.txt");
    f2.open(QIODevice::WriteOnly);
    f2.write("nested");
    f2.close();

    QVERIFY(QDir(dirPath).exists());
    bool result = UBFileSystemUtils::deleteDir(dirPath);
    QVERIFY(result);
    QVERIFY(!QDir(dirPath).exists());
}

void TestUBFileSystemUtils::testCopyDir()
{
    QString srcDir = mTempDir + "/copy_src";
    QString dstDir = mTempDir + "/copy_dst";
    QDir().mkpath(srcDir);

    QFile f(srcDir + "/file.txt");
    f.open(QIODevice::WriteOnly);
    f.write("copy me");
    f.close();

    bool result = UBFileSystemUtils::copyDir(srcDir, dstDir);
    QVERIFY(result);
    QVERIFY(QFile::exists(dstDir + "/file.txt"));

    UBFileSystemUtils::deleteDir(srcDir);
    UBFileSystemUtils::deleteDir(dstDir);
}

void TestUBFileSystemUtils::testMoveDir()
{
    QString srcDir = mTempDir + "/move_src";
    QString dstDir = mTempDir + "/move_dst";
    QDir().mkpath(srcDir);

    QFile f(srcDir + "/file.txt");
    f.open(QIODevice::WriteOnly);
    f.write("move me");
    f.close();

    bool result = UBFileSystemUtils::moveDir(srcDir, dstDir);
    QVERIFY(result);
    QVERIFY(QFile::exists(dstDir + "/file.txt"));
    QVERIFY(!QDir(srcDir).exists());

    UBFileSystemUtils::deleteDir(dstDir);
}

void TestUBFileSystemUtils::testAllFiles()
{
    QString dir = mTempDir + "/allfiles";
    QDir().mkpath(dir + "/sub");

    QFile f1(dir + "/a.txt");
    f1.open(QIODevice::WriteOnly);
    f1.write("a");
    f1.close();

    QFile f2(dir + "/sub/b.txt");
    f2.open(QIODevice::WriteOnly);
    f2.write("b");
    f2.close();

    QStringList files = UBFileSystemUtils::allFiles(dir, true);
    QCOMPARE(files.size(), 2);

    QStringList filesNonRecursive = UBFileSystemUtils::allFiles(dir, false);
    // Non-recursive includes subdirs as entries too (QDir::Dirs flag)
    QCOMPARE(filesNonRecursive.size(), 2);

    UBFileSystemUtils::deleteDir(dir);
}

void TestUBFileSystemUtils::testNextAvailableFileName()
{
    QString filePath = mTempDir + "/unique.txt";

    // File doesn't exist, should return same name
    QString result = UBFileSystemUtils::nextAvailableFileName(filePath);
    QCOMPARE(result, filePath);

    // Create the file, should return incremented name
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.write("x");
    f.close();

    result = UBFileSystemUtils::nextAvailableFileName(filePath);
    QVERIFY(result != filePath);
    QVERIFY(result.contains("1"));

    QFile::remove(filePath);
}

void TestUBFileSystemUtils::testNextAvailableDirName()
{
    QString dirPath = mTempDir + "/uniquedir";

    // Dir doesn't exist, should return same name
    QString result = UBFileSystemUtils::nextAvailableDirName(dirPath);
    QCOMPARE(result, dirPath);

    // Create the dir, should return incremented name
    QDir().mkpath(dirPath);
    result = UBFileSystemUtils::nextAvailableDirName(dirPath);
    QVERIFY(result != dirPath);
    QVERIFY(result.contains("1"));

    QDir().rmdir(dirPath);
}

void TestUBFileSystemUtils::testReadTextFile()
{
    QString filePath = mTempDir + "/read_test.txt";
    QString content = "Hello\nWorld\nTest";

    QFile f(filePath);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&f);
    out << content;
    f.close();

    QString result = UBFileSystemUtils::readTextFile(filePath);
    QCOMPARE(result, content);

    // Non-existent file should return empty string
    QCOMPARE(UBFileSystemUtils::readTextFile("/nonexistent.txt"), QString(""));

    QFile::remove(filePath);
}

void TestUBFileSystemUtils::testThumbnailPath()
{
    QString path = "/documents/page001.svg";
    QString result = UBFileSystemUtils::thumbnailPath(path);
    QCOMPARE(result, QString("/documents/page001.thumbnail.png"));
}

void TestUBFileSystemUtils::testDeletePath_file()
{
    QString filePath = mTempDir + "/deletepath_file.txt";
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.write("to delete");
    f.close();

    QVERIFY(QFile::exists(filePath));
    bool result = UBFileSystemUtils::deletePath(filePath);
    QVERIFY(result);
    QVERIFY(!QFile::exists(filePath));
}

void TestUBFileSystemUtils::testDeletePath_dir()
{
    QString dirPath = mTempDir + "/deletepath_dir";
    QDir().mkpath(dirPath);

    QFile f(dirPath + "/inner.txt");
    f.open(QIODevice::WriteOnly);
    f.write("inner");
    f.close();

    QVERIFY(QDir(dirPath).exists());
    bool result = UBFileSystemUtils::deletePath(dirPath);
    QVERIFY(result);
    QVERIFY(!QDir(dirPath).exists());
}

void TestUBFileSystemUtils::testMimeTypeFromUrl()
{
    QUrl pngUrl("file:///home/user/image.png");
    QCOMPARE(UBFileSystemUtils::mimeTypeFromUrl(pngUrl), UBMimeType::RasterImage);

    QUrl svgUrl("http://example.com/drawing.svg");
    QCOMPARE(UBFileSystemUtils::mimeTypeFromUrl(svgUrl), UBMimeType::VectorImage);

    QUrl mp4Url("file:///videos/lesson.mp4");
    QCOMPARE(UBFileSystemUtils::mimeTypeFromUrl(mp4Url), UBMimeType::Video);

    QUrl pdfUrl("file:///docs/manual.pdf");
    QCOMPARE(UBFileSystemUtils::mimeTypeFromUrl(pdfUrl), UBMimeType::PDF);

    QUrl unknownUrl("file:///data/file.xyz");
    QCOMPARE(UBFileSystemUtils::mimeTypeFromUrl(unknownUrl), UBMimeType::UNKNOWN);
}
