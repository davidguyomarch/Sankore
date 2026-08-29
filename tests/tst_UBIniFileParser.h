/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBINIFILEPARSER_H
#define TST_UBINIFILEPARSER_H

#include <QObject>
#include <QtTest>

class TestUBIniFileParser : public QObject
{
    Q_OBJECT

private slots:
    void testParseSimpleIni();
    void testParseMissingSectionReturnsEmpty();
    void testParseMissingKeyReturnsEmpty();
    void testParseMultipleSections();
    void testParseNonExistentFile();
    void testParseEmptyFile();
    void testParseValuesWithEquals();
};

#endif // TST_UBINIFILEPARSER_H
