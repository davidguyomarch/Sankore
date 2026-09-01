/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBTEXTTOOLS_H
#define TST_UBTEXTTOOLS_H

#include <QtTest>
#include <QObject>

class TestUBTextTools : public QObject
{
    Q_OBJECT

private slots:
    void testCleanHtmlCData_stripsNul();
    void testCleanHtmlCData_noNul();
    void testCleanHtmlCData_empty();
    void testCleanHtml_extractsBody();
    void testCleanHtml_noBody();
    void testCleanHtml_bodyWithAttributes();
    void testCleanHtml_caseInsensitive();
};

#endif // TST_UBTEXTTOOLS_H
