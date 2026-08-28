/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBBase32.h"
#include "frameworks/UBBase32.h"

void TestUBBase32::testDecodeEmpty()
{
    QByteArray result = UBBase32::decode("");
    QVERIFY(result.isEmpty());
}

void TestUBBase32::testDecodeRFC4648Vectors()
{
    // RFC 4648 test vectors (without padding)
    QCOMPARE(UBBase32::decode("MY"), QByteArray("f"));
    QCOMPARE(UBBase32::decode("MZXQ"), QByteArray("fo"));
    QCOMPARE(UBBase32::decode("MZXW6"), QByteArray("foo"));
    QCOMPARE(UBBase32::decode("MZXW6YQ"), QByteArray("foob"));
    QCOMPARE(UBBase32::decode("MZXW6YTB"), QByteArray("fooba"));
    QCOMPARE(UBBase32::decode("MZXW6YTBOI"), QByteArray("foobar"));
}

void TestUBBase32::testDecodeCaseInsensitive()
{
    // Lowercase should decode same as uppercase
    QByteArray upper = UBBase32::decode("MFRGGZDFMY");
    QByteArray lower = UBBase32::decode("mfrggzdfmy");
    QCOMPARE(lower, upper);
}

void TestUBBase32::testDecodeInvalidChars()
{
    // Characters outside base32 alphabet should be skipped
    QByteArray result = UBBase32::decode("MY======");  // padding chars (=) are outside lookup
    // Should still decode "f" (MY part) — padding is skipped
    QCOMPARE(result.left(1), QByteArray("f"));
}

void TestUBBase32::testDecodeHelloWorld()
{
    // "Hello" in base32 is JBSWY3DP
    QCOMPARE(UBBase32::decode("JBSWY3DP"), QByteArray("Hello"));
}
