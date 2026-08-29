/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBMETADATADCSUBSETADAPTOR_H
#define TST_UBMETADATADCSUBSETADAPTOR_H

#include <QObject>
#include <QtTest>

class TestUBMetadataDcSubsetAdaptor : public QObject
{
    Q_OBJECT

private slots:
    void testLoadNonExistentPath();
    void testLoadBasicMetadata();
    void testLoadDocumentSize();
    void testLoadSizeMigration1024x768();
    void testLoadVersion();
    void testLoadUpdatedAt();
    void testLoadSessionFields();
    void testLoadDateFormatShort();
    void testLoadMissingSize();
    void testLoadIdentifier();
    void testLoadBackgroundImage();
    void testLoadSizeMigrationWithCustomSettings();
    void testLoadWithNullSettings();
};

#endif // TST_UBMETADATADCSUBSETADAPTOR_H
