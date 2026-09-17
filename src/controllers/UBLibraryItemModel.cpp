/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBLibraryItemModel.h"

#include <QUrl>

UBLibraryItemModel::UBLibraryItemModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void UBLibraryItemModel::setItems(const QList<UBFeature>& items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

UBFeature UBLibraryItemModel::featureAt(int row) const
{
    if (row < 0 || row >= m_items.size())
        return UBFeature();
    return m_items.at(row);
}

int UBLibraryItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant UBLibraryItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    if (row < 0 || row >= m_items.size())
        return QVariant();

    const UBFeature& f = m_items.at(row);

    switch (role) {
    case NameRole:
        return f.getDisplayName();
    case ThumbnailUrlRole: {
        // Only image files get a file:// preview; other types fall back to an
        // icon (IconNameRole). A folder never shows a file preview.
        if (f.getType() == FEATURE_IMAGE) {
            const QUrl path = f.getFullPath();
            if (!path.isEmpty())
                return path.toString();
        }
        return QString();
    }
    case IconNameRole:
        return iconNameForType(f.getType());
    case TypeRole:
        return static_cast<int>(f.getType());
    case VirtualPathRole:
        return f.getFullVirtualPath();
    case IsFolderRole:
        return f.isFolder();
    case DeletableRole:
        return f.isDeletable();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> UBLibraryItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ThumbnailUrlRole] = "thumbnailUrl";
    roles[IconNameRole] = "iconName";
    roles[TypeRole] = "type";
    roles[VirtualPathRole] = "virtualPath";
    roles[IsFolderRole] = "isFolder";
    roles[DeletableRole] = "deletable";
    return roles;
}

QString UBLibraryItemModel::iconNameForType(UBFeatureElementType type)
{
    // Phosphor icon names (resources/icons/phosphor/). Fallback art for entries
    // without a file preview.
    switch (type) {
    case FEATURE_FOLDER:
    case FEATURE_CATEGORY:
    case FEATURE_VIRTUALFOLDER:
        return QStringLiteral("folder");
    case FEATURE_AUDIO:
        return QStringLiteral("play");
    case FEATURE_VIDEO:
        return QStringLiteral("play");
    case FEATURE_IMAGE:
        return QStringLiteral("image");
    case FEATURE_INTERACTIVE:
    case FEATURE_INTERNAL:
        return QStringLiteral("shapes");
    case FEATURE_TRASH:
        return QStringLiteral("trash");
    case FEATURE_FAVORITE:
        return QStringLiteral("star");
    case FEATURE_BOOKMARK:
    case FEATURE_LINK:
        return QStringLiteral("link");
    default:
        return QStringLiteral("file-text");
    }
}
