/*
 * Copyright (C) 2026 David Guyomarch
 */

#ifndef TST_UBCRYPTOUTILS_H
#define TST_UBCRYPTOUTILS_H

#include <QtTest>
#include <QObject>

class TestUBCryptoUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testEncryptDecryptRoundtrip();
    void testEncryptedDiffersFromPlaintext();
    void testEmptyStringInput();
    void testUnicodeCharacters();
    void testSpecialCharacters();
    void testLargePayload();
    void testDecryptInvalidBase64();
};

#endif // TST_UBCRYPTOUTILS_H
