/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBLIBRARYITEMMODEL_H
#define UBLIBRARYITEMMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>
#include <QList>

#include "board/UBFeature.h"

/**
 * UBLibraryItemModel — a QAbstractListModel of library items for the QML V2
 * Library panel (#258).
 *
 * One row per entry of the currently displayed virtual folder. Modeled on
 * UBPageThumbnailModel (#328): a plain QAbstractListModel with text roleNames()
 * so a QML GridView delegate can bind directly.
 *
 * This model is deliberately decoupled from UBFeaturesController and from any
 * QWidget/QListView: it holds a flat QList<UBFeature> set via setItems(), which
 * makes it unit-testable headless (no disk scan, no board controller). The
 * controller (UBLibraryController) feeds it the direct children of the current
 * folder; the model only renders them.
 *
 * Roles exposed to QML:
 *   name         (QString)  display name
 *   thumbnailUrl (QString)  file:// URL for image files, empty otherwise
 *   iconName     (QString)  Phosphor icon name for the entry type (fallback art)
 *   type         (int)      UBFeatureElementType
 *   virtualPath  (QString)  full virtual path (for navigation into folders)
 *   isFolder     (bool)
 *   deletable    (bool)
 */
class UBLibraryItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ThumbnailUrlRole,
        IconNameRole,
        TypeRole,
        VirtualPathRole,
        IsFolderRole,
        DeletableRole,
    };

    explicit UBLibraryItemModel(QObject* parent = nullptr);

    // Replace the displayed items (the direct children of the current folder).
    void setItems(const QList<UBFeature>& items);
    const QList<UBFeature>& items() const { return m_items; }

    // Access a single feature (for the controller to act on a clicked row).
    UBFeature featureAt(int row) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Map a UBFeatureElementType to a Phosphor icon name (pure, static, testable).
    static QString iconNameForType(UBFeatureElementType type);

private:
    QList<UBFeature> m_items;
};

#endif // UBLIBRARYITEMMODEL_H
