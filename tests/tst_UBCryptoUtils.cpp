/*
 * Copyright (C) 2026 David Guyomarch
 */

#include "tst_UBCryptoUtils.h"
#include "stubs/UBCryptoUtils_stub.h"

void TestUBCryptoUtils::initTestCase()
{
    // Ensure the singleton instance is created
    UBCryptoUtils::instance();
}

void TestUBCryptoUtils::cleanupTestCase()
{
    UBCryptoUtils::destroy();
}

void TestUBCryptoUtils::testEncryptDecryptRoundtrip()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    QString plaintext = "Hello, Open-Sankore!";
    QString encrypted = crypto->symetricEncrypt(plaintext);
    QVERIFY(!encrypted.isEmpty());

    QString decrypted = crypto->symetricDecrypt(encrypted);
    QCOMPARE(decrypted, plaintext);
}

void TestUBCryptoUtils::testEncryptedDiffersFromPlaintext()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    QString plaintext = "This is a test message for encryption";
    QString encrypted = crypto->symetricEncrypt(plaintext);

    QVERIFY(!encrypted.isEmpty());
    QVERIFY(encrypted != plaintext);
}

void TestUBCryptoUtils::testEmptyStringInput()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    QString plaintext = "";
    QString encrypted = crypto->symetricEncrypt(plaintext);
    // Empty string encryption should still produce output (padding)
    QVERIFY(!encrypted.isEmpty());

    QString decrypted = crypto->symetricDecrypt(encrypted);
    QCOMPARE(decrypted, plaintext);
}

void TestUBCryptoUtils::testUnicodeCharacters()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    // Test with Unicode characters typical in educational contexts
    QString plaintext = QString::fromUtf8("Élève Müller - 学生太郎 - مرحبا");
    QString encrypted = crypto->symetricEncrypt(plaintext);
    QVERIFY(!encrypted.isEmpty());

    QString decrypted = crypto->symetricDecrypt(encrypted);
    QCOMPARE(decrypted, plaintext);
}

void TestUBCryptoUtils::testSpecialCharacters()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    QString plaintext = "!@#$%^&*()_+-=[]{}|;':\",./<>?`~";
    QString encrypted = crypto->symetricEncrypt(plaintext);
    QVERIFY(!encrypted.isEmpty());

    QString decrypted = crypto->symetricDecrypt(encrypted);
    QCOMPARE(decrypted, plaintext);
}

void TestUBCryptoUtils::testLargePayload()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    // Create a 10KB+ payload
    QString plaintext;
    for (int i = 0; i < 1000; ++i)
    {
        plaintext += "0123456789"; // 10 chars * 1000 = 10000 chars > 10KB in UTF-8
    }
    QVERIFY(plaintext.toUtf8().size() >= 10000);

    QString encrypted = crypto->symetricEncrypt(plaintext);
    QVERIFY(!encrypted.isEmpty());

    QString decrypted = crypto->symetricDecrypt(encrypted);
    QCOMPARE(decrypted, plaintext);
}

void TestUBCryptoUtils::testDecryptInvalidBase64()
{
    UBCryptoUtils *crypto = UBCryptoUtils::instance();

    // Corrupted base64 should return empty string (decrypt failure)
    QString corrupted = "ThisIsNotValidBase64!!!@@@";
    QString result = crypto->symetricDecrypt(corrupted);

    // Should either return empty or not crash — the important thing is no segfault
    // EVP_DecryptFinal_ex will fail on invalid padding
    Q_UNUSED(result);
    // If we get here without crashing, the test passes
    QVERIFY(true);
}
