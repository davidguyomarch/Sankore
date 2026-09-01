/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBTextTools.h"
#include "core/UBTextTools.h"

// --- cleanHtmlCData: removes embedded NUL characters ---

void TestUBTextTools::testCleanHtmlCData_stripsNul()
{
    QString input = QStringLiteral("ab");
    input.insert(1, QChar(QChar::Null)); // "a\0b"

    QString result = UBTextTools::cleanHtmlCData(input);

    QCOMPARE(result, QStringLiteral("ab"));
    QVERIFY(!result.contains(QChar(QChar::Null)));
}

void TestUBTextTools::testCleanHtmlCData_noNul()
{
    QString input = QStringLiteral("hello world");
    QCOMPARE(UBTextTools::cleanHtmlCData(input), input);
}

void TestUBTextTools::testCleanHtmlCData_empty()
{
    QCOMPARE(UBTextTools::cleanHtmlCData(QString()), QString());
}

// --- cleanHtml: extracts the <body ...> slice ---

void TestUBTextTools::testCleanHtml_extractsBody()
{
    QString input = QStringLiteral("<html><head></head><body>content</body></html>");
    QString result = UBTextTools::cleanHtml(input);

    // The extracted slice must start at the opening <body tag.
    QVERIFY(result.startsWith(QStringLiteral("<body")));
    QVERIFY(result.contains(QStringLiteral("content")));

    // The slice must stop at the </body boundary and must NOT overshoot into
    // the trailing markup (e.g. "</html>"). This is the regression guard for
    // issue #228 where mid() was called with an absolute index as its length,
    // which ran start+end characters into the string.
    QCOMPARE(result, QStringLiteral("<body>content</body"));
    QVERIFY(!result.contains(QStringLiteral("</html>")));
}

void TestUBTextTools::testCleanHtml_noBody()
{
    // Without a <body tag, the original html is returned unchanged.
    QString input = QStringLiteral("<div>no body here</div>");
    QCOMPARE(UBTextTools::cleanHtml(input), input);
}

void TestUBTextTools::testCleanHtml_bodyWithAttributes()
{
    QString input = QStringLiteral("<html><body style=\"margin:0\">x</body></html>");
    QString result = UBTextTools::cleanHtml(input);
    QVERIFY(result.startsWith(QStringLiteral("<body")));
}

void TestUBTextTools::testCleanHtml_caseInsensitive()
{
    // Tag matching is done on a lowercased copy, so uppercase tags are found.
    QString input = QStringLiteral("<HTML><BODY>upper</BODY></HTML>");
    QString result = UBTextTools::cleanHtml(input);
    QVERIFY(result.startsWith(QStringLiteral("<BODY")));
    QCOMPARE(result, QStringLiteral("<BODY>upper</BODY"));
}

void TestUBTextTools::testCleanHtml_openTagButNoCloseTag()
{
    // Opening <body but no closing </body. The guard must treat this as "no
    // valid body slice" and return the original html unchanged, rather than
    // producing a garbage slice from a negative end index (issue #228 note).
    QString input = QStringLiteral("<html><body>content only");
    QCOMPARE(UBTextTools::cleanHtml(input), input);
}
