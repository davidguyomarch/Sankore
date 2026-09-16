/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBLibraryModel.h"

#include <QtTest/QtTest>
#include <QImage>
#include <QUrl>

#include "controllers/UBLibraryController.h"
#include "controllers/UBLibraryItemModel.h"
#include "board/UBFeature.h"

// Helper: build a UBFeature at a given virtual folder with a name and type.
static UBFeature makeFeature(const QString& parentVirtualDir,
                             const QString& name,
                             UBFeatureElementType type,
                             const QUrl& realPath = QUrl())
{
    // UBFeature's ctor takes the FULL virtual path and splits it into
    // (virtualDir, name); pass "<parent>/<name>".
    return UBFeature(parentVirtualDir + "/" + name, QImage(), name, realPath, type);
}

// --- UBLibraryController static helpers ---

void TestUBLibraryModel::testParentPathClampsAtRoot()
{
    QCOMPARE(UBLibraryController::parentPath("/root"), QStringLiteral("/root"));
    QCOMPARE(UBLibraryController::parentPath(""), QStringLiteral("/root"));
    QCOMPARE(UBLibraryController::parentPath("/root/Pictures"), QStringLiteral("/root"));
}

void TestUBLibraryModel::testParentPathOneLevel()
{
    QCOMPARE(UBLibraryController::parentPath("/root/Pictures/Sub"),
             QStringLiteral("/root/Pictures"));
}

void TestUBLibraryModel::testChildrenOfFiltersByVirtualDir()
{
    QList<UBFeature> all;
    all << makeFeature("/root", "Pictures", FEATURE_CATEGORY)
        << makeFeature("/root", "Audios", FEATURE_CATEGORY)
        << makeFeature("/root/Pictures", "cat.png", FEATURE_IMAGE)
        << makeFeature("/root/Pictures", "dog.png", FEATURE_IMAGE)
        << makeFeature("/root/Audios", "bell.mp3", FEATURE_AUDIO);

    const QList<UBFeature> rootChildren =
        UBLibraryController::childrenOf(all, "/root");
    QCOMPARE(rootChildren.size(), 2);

    const QList<UBFeature> picChildren =
        UBLibraryController::childrenOf(all, "/root/Pictures");
    QCOMPARE(picChildren.size(), 2);

    const QList<UBFeature> none =
        UBLibraryController::childrenOf(all, "/root/DoesNotExist");
    QCOMPARE(none.size(), 0);
}

// --- UBLibraryItemModel ---

void TestUBLibraryModel::testModelEmptyByDefault()
{
    UBLibraryItemModel model;
    QCOMPARE(model.rowCount(), 0);
    // roleNames must expose the QML-facing names.
    const auto roles = model.roleNames();
    QVERIFY(roles.values().contains(QByteArray("name")));
    QVERIFY(roles.values().contains(QByteArray("thumbnailUrl")));
    QVERIFY(roles.values().contains(QByteArray("iconName")));
    QVERIFY(roles.values().contains(QByteArray("isFolder")));
}

void TestUBLibraryModel::testModelRolesForImage()
{
    UBLibraryItemModel model;
    const QUrl real = QUrl::fromLocalFile("/tmp/cat.png");
    model.setItems({ makeFeature("/root/Pictures", "cat.png", FEATURE_IMAGE, real) });

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex idx = model.index(0, 0);
    QCOMPARE(model.data(idx, UBLibraryItemModel::NameRole).toString(),
             QStringLiteral("cat.png"));
    // An image exposes a non-empty file:// thumbnail URL.
    QCOMPARE(model.data(idx, UBLibraryItemModel::ThumbnailUrlRole).toString(),
             real.toString());
    QCOMPARE(model.data(idx, UBLibraryItemModel::IsFolderRole).toBool(), false);
    QCOMPARE(model.data(idx, UBLibraryItemModel::TypeRole).toInt(),
             static_cast<int>(FEATURE_IMAGE));
}

void TestUBLibraryModel::testModelIconFallbackForNonImage()
{
    UBLibraryItemModel model;
    model.setItems({ makeFeature("/root/Audios", "bell.mp3", FEATURE_AUDIO,
                                 QUrl::fromLocalFile("/tmp/bell.mp3")) });
    const QModelIndex idx = model.index(0, 0);
    // Non-image: no file preview, a Phosphor icon name instead.
    QVERIFY(model.data(idx, UBLibraryItemModel::ThumbnailUrlRole).toString().isEmpty());
    QVERIFY(!model.data(idx, UBLibraryItemModel::IconNameRole).toString().isEmpty());
    QCOMPARE(UBLibraryItemModel::iconNameForType(FEATURE_FOLDER),
             QStringLiteral("folder"));
}

void TestUBLibraryModel::testModelFolderFlags()
{
    UBLibraryItemModel model;
    model.setItems({ makeFeature("/root", "Pictures", FEATURE_CATEGORY) });
    const QModelIndex idx = model.index(0, 0);
    QCOMPARE(model.data(idx, UBLibraryItemModel::IsFolderRole).toBool(), true);
    QCOMPARE(model.featureAt(0).getFullVirtualPath(),
             QStringLiteral("/root/Pictures"));
}

// --- UBLibraryController navigation ---

void TestUBLibraryModel::testControllerStartsAtRoot()
{
    UBLibraryController ctrl;
    QCOMPARE(ctrl.currentPath(), QStringLiteral("/root"));
    QCOMPARE(ctrl.canGoUp(), false);
    QVERIFY(ctrl.itemModel() != nullptr);
}

void TestUBLibraryModel::testSetFeaturesPopulatesRootChildren()
{
    UBLibraryController ctrl;
    QList<UBFeature> all;
    all << makeFeature("/root", "Pictures", FEATURE_CATEGORY)
        << makeFeature("/root", "Audios", FEATURE_CATEGORY)
        << makeFeature("/root/Pictures", "cat.png", FEATURE_IMAGE);
    ctrl.setFeatures(all);

    auto* model = qobject_cast<UBLibraryItemModel*>(ctrl.itemModel());
    QVERIFY(model);
    // At root, only the two categories are direct children.
    QCOMPARE(model->rowCount(), 2);
}

void TestUBLibraryModel::testEnterFolderNavigates()
{
    UBLibraryController ctrl;
    QList<UBFeature> all;
    all << makeFeature("/root", "Pictures", FEATURE_CATEGORY)
        << makeFeature("/root/Pictures", "cat.png", FEATURE_IMAGE)
        << makeFeature("/root/Pictures", "dog.png", FEATURE_IMAGE);
    ctrl.setFeatures(all);

    QSignalSpy spy(&ctrl, &UBLibraryController::currentPathChanged);
    ctrl.enterFolder(0); // "Pictures" category at row 0

    QCOMPARE(ctrl.currentPath(), QStringLiteral("/root/Pictures"));
    QCOMPARE(ctrl.canGoUp(), true);
    QCOMPARE(spy.count(), 1);

    auto* model = qobject_cast<UBLibraryItemModel*>(ctrl.itemModel());
    QCOMPARE(model->rowCount(), 2); // cat.png + dog.png
}

void TestUBLibraryModel::testEnterNonFolderIsNoOp()
{
    UBLibraryController ctrl;
    ctrl.setFeatures({ makeFeature("/root", "cat.png", FEATURE_IMAGE) });
    ctrl.enterFolder(0); // an image is not a folder
    QCOMPARE(ctrl.currentPath(), QStringLiteral("/root"));
}

void TestUBLibraryModel::testGoUpReturnsToParent()
{
    UBLibraryController ctrl;
    QList<UBFeature> all;
    all << makeFeature("/root", "Pictures", FEATURE_CATEGORY)
        << makeFeature("/root/Pictures", "cat.png", FEATURE_IMAGE);
    ctrl.setFeatures(all);

    ctrl.enterFolder(0);
    QCOMPARE(ctrl.currentPath(), QStringLiteral("/root/Pictures"));
    ctrl.goUp();
    QCOMPARE(ctrl.currentPath(), QStringLiteral("/root"));
    QCOMPARE(ctrl.canGoUp(), false);
}

void TestUBLibraryModel::testGoUpAtRootIsNoOp()
{
    UBLibraryController ctrl;
    QSignalSpy spy(&ctrl, &UBLibraryController::currentPathChanged);
    ctrl.goUp();
    QCOMPARE(ctrl.currentPath(), QStringLiteral("/root"));
    QCOMPARE(spy.count(), 0);
}
