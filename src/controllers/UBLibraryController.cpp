/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBLibraryController.h"
#include "UBLibraryItemModel.h"

UBLibraryController::UBLibraryController(QObject* parent)
    : QObject(parent)
    , m_itemModel(new UBLibraryItemModel(this))
    , m_currentPath(rootPath())
{
}

QObject* UBLibraryController::itemModel() const
{
    return m_itemModel;
}

void UBLibraryController::setFeatures(const QList<UBFeature>& features)
{
    m_features = features;
    refreshModel();
}

void UBLibraryController::setCurrentPath(const QString& path)
{
    const QString target = path.isEmpty() ? rootPath() : path;
    if (target == m_currentPath)
        return;
    m_currentPath = target;
    refreshModel();
    emit currentPathChanged();
}

QList<UBFeature> UBLibraryController::childrenOf(const QList<UBFeature>& all, const QString& path)
{
    QList<UBFeature> result;
    for (const UBFeature& f : all) {
        // A direct child has its parent virtual folder (virtualDir) equal to path.
        if (f.getVirtualPath() == path)
            result.append(f);
    }
    return result;
}

QString UBLibraryController::parentPath(const QString& path)
{
    if (path == rootPath() || path.isEmpty())
        return rootPath();

    const int slash = path.lastIndexOf('/');
    if (slash <= 0)
        return rootPath();

    const QString parent = path.left(slash);
    return parent.isEmpty() ? rootPath() : parent;
}

void UBLibraryController::enterFolder(int row)
{
    const UBFeature f = m_itemModel->featureAt(row);
    if (!f.isFolder())
        return;
    setCurrentPath(f.getFullVirtualPath());
}

void UBLibraryController::goUp()
{
    if (!canGoUp())
        return;
    setCurrentPath(parentPath(m_currentPath));
}

void UBLibraryController::refreshModel()
{
    m_itemModel->setItems(childrenOf(m_features, m_currentPath));
}
