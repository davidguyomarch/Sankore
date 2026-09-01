/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBOEMBEDPARSER_H
#define TST_UBOEMBEDPARSER_H

#include <QObject>
#include <QtTest>

class TestUBOEmbedParser : public QObject
{
    Q_OBJECT

private slots:
    void testGetJSONInfos_video();
    void testGetJSONInfos_photo();
    void testGetJSONInfos_emptyJson();
    void testGetXMLInfos_video();
    void testGetXMLInfos_photo();
    void testGetXMLInfos_emptyXml();
    void testGetJSONInfos_partialData();
    void testGetXMLInfos_allFields();

    // Regression test for #229: parse() must not crash when an oembed link is
    // found before setNetworkAccessManager() has been called. The constructor
    // must initialise mpNam (nullptr) and mPending (0).
    void testParseWithoutNamDoesNotCrash();
    void testConstructorInitialisesMembers();
};

#endif // TST_UBOEMBEDPARSER_H
