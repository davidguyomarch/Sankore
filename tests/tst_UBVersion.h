/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBVERSION_H
#define TST_UBVERSION_H

#include <QObject>
#include <QtTest>

class TestUBVersion : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultConstructor();
    void testValidVersionTwoParts();
    void testValidVersionThreeParts();
    void testValidVersionAlpha();
    void testValidVersionBeta();
    void testValidVersionRC();
    void testInvalidVersion();
    void testComparisonLessThan();
    void testComparisonGreaterThan();
    void testComparisonEqual();
    void testComparisonDifferentPlatform();
    void testComparisonDifferentMajor();
    void testComparisonDifferentStage();
    void testToString();
    void testToStringInvalid();
};

#endif // TST_UBVERSION_H
