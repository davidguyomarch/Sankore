/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBFILESYSTEMUTILS_H
#define TST_UBFILESYSTEMUTILS_H

#include <QtTest>
#include <QObject>

class TestUBFileSystemUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // removeLocalFilePrefix
    void testRemoveLocalFilePrefix_unix();
    void testRemoveLocalFilePrefix_noPrefix();
    void testRemoveLocalFilePrefix_empty();

    // cleanName
    void testCleanName_removesSlash();
    void testCleanName_removesColon();
    void testCleanName_removesMultipleChars();
    void testCleanName_preservesNormal();

    // normalizeFilePath
    void testNormalizeFilePath_backslashes();
    void testNormalizeFilePath_alreadyNormalized();

    // extension
    void testExtension_simple();
    void testExtension_multiple_dots();
    void testExtension_noExtension();
    void testExtension_trailingSlash();

    // lastPathComponent
    void testLastPathComponent_file();
    void testLastPathComponent_dir();
    void testLastPathComponent_trailingSlash();

    // mimeTypeFromFileName
    void testMimeTypeFromFileName_png();
    void testMimeTypeFromFileName_pdf();
    void testMimeTypeFromFileName_svg();
    void testMimeTypeFromFileName_mp4();
    void testMimeTypeFromFileName_unknown();

    // fileExtensionFromMimeType
    void testFileExtensionFromMimeType_image();
    void testFileExtensionFromMimeType_video();
    void testFileExtensionFromMimeType_unknown();

    // mimeTypeFromString
    void testMimeTypeFromString_rasterImage();
    void testMimeTypeFromString_vectorImage();
    void testMimeTypeFromString_video();
    void testMimeTypeFromString_audio();
    void testMimeTypeFromString_pdf();
    void testMimeTypeFromString_unknown();

    // isPathMatch
    void testIsPathMatch_exact();
    void testIsPathMatch_subpath();
    void testIsPathMatch_noMatch();
    void testIsPathMatch_similarPrefix();

    // File operations (using temp dirs)
    void testCopyFile();
    void testCopyFile_nonExistent();
    void testDeleteFile();
    void testDeleteDir();
    void testCopyDir();
    void testMoveDir();
    void testAllFiles();
    void testNextAvailableFileName();
    void testNextAvailableDirName();
    void testReadTextFile();

    // thumbnailPath
    void testThumbnailPath();

    // deletePath
    void testDeletePath_file();
    void testDeletePath_dir();

    // mimeTypeFromUrl
    void testMimeTypeFromUrl();

private:
    QString mTempDir;
};

#endif // TST_UBFILESYSTEMUTILS_H
