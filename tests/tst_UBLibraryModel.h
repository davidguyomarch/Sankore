/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBLIBRARYMODEL_H
#define TST_UBLIBRARYMODEL_H

#include <QObject>

/**
 * Unit tests for the QML V2 Library model + controller (#258, PR 1).
 *
 * Covers the headless-testable logic: UBLibraryItemModel roles/rowCount and
 * UBLibraryController virtual-folder navigation (children filtering, enter
 * folder, go up, parentPath). No disk scan, no board controller.
 */
class TestUBLibraryModel : public QObject
{
    Q_OBJECT

private slots:
    // UBLibraryController static helpers (pure)
    void testParentPathClampsAtRoot();
    void testParentPathOneLevel();
    void testChildrenOfFiltersByVirtualDir();

    // UBLibraryItemModel
    void testModelEmptyByDefault();
    void testModelRolesForImage();
    void testModelIconFallbackForNonImage();
    void testModelFolderFlags();

    // UBLibraryController navigation
    void testControllerStartsAtRoot();
    void testSetFeaturesPopulatesRootChildren();
    void testEnterFolderNavigates();
    void testEnterNonFolderIsNoOp();
    void testGoUpReturnsToParent();
    void testGoUpAtRootIsNoOp();
};

#endif // TST_UBLIBRARYMODEL_H
