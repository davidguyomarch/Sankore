/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBIniFileParser.h"
#include "adaptors/UBIniFileParser.h"
#include <QTemporaryFile>
#include <QTextStream>

static QString writeIniFile(const QString& content)
{
    QTemporaryFile *tmp = new QTemporaryFile();
    tmp->setAutoRemove(false);
    tmp->open();
    QTextStream out(tmp);
    out << content;
    tmp->close();
    QString path = tmp->fileName();
    delete tmp;
    return path;
}

void TestUBIniFileParser::testParseSimpleIni()
{
    QString path = writeIniFile("[General]\nname=Open-Sankore\nversion=3.1\n");
    UBIniFileParser parser(path);
    QCOMPARE(parser.getStringValue("General", "name"), QString("Open-Sankore"));
    QCOMPARE(parser.getStringValue("General", "version"), QString("3.1"));
    QFile::remove(path);
}

void TestUBIniFileParser::testParseMissingSectionReturnsEmpty()
{
    QString path = writeIniFile("[General]\nname=Test\n");
    UBIniFileParser parser(path);
    QCOMPARE(parser.getStringValue("NonExistent", "name"), QString(""));
    QFile::remove(path);
}

void TestUBIniFileParser::testParseMissingKeyReturnsEmpty()
{
    QString path = writeIniFile("[General]\nname=Test\n");
    UBIniFileParser parser(path);
    QCOMPARE(parser.getStringValue("General", "missing"), QString(""));
    QFile::remove(path);
}

void TestUBIniFileParser::testParseMultipleSections()
{
    QString content = "[Database]\nhost=localhost\nport=5432\n[App]\nname=Sankore\ndebug=true\n";
    QString path = writeIniFile(content);
    UBIniFileParser parser(path);
    QCOMPARE(parser.getStringValue("Database", "host"), QString("localhost"));
    QCOMPARE(parser.getStringValue("Database", "port"), QString("5432"));
    QCOMPARE(parser.getStringValue("App", "name"), QString("Sankore"));
    QCOMPARE(parser.getStringValue("App", "debug"), QString("true"));
    QFile::remove(path);
}

void TestUBIniFileParser::testParseNonExistentFile()
{
    UBIniFileParser parser("/nonexistent/path/file.ini");
    QCOMPARE(parser.getStringValue("General", "key"), QString(""));
}

void TestUBIniFileParser::testParseEmptyFile()
{
    QString path = writeIniFile("");
    UBIniFileParser parser(path);
    QCOMPARE(parser.getStringValue("General", "key"), QString(""));
    QFile::remove(path);
}

void TestUBIniFileParser::testParseValuesWithEquals()
{
    // Value contains = sign
    QString path = writeIniFile("[General]\nformula=a=b+c\n");
    UBIniFileParser parser(path);
    // Should capture everything after first =
    QCOMPARE(parser.getStringValue("General", "formula"), QString("a=b+c"));
    QFile::remove(path);
}
