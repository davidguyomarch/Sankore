/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBPAGETHUMBNAILMODEL_H
#define UBPAGETHUMBNAILMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>

/**
 * UBPageThumbnailModel — a QAbstractListModel of page thumbnails for the QML
 * PageNavigator (#328).
 *
 * One row per page of the active document. Each row exposes a file:// URL to
 * the page's `pageN.thumbnail.jpg` (see UBThumbnailAdaptor), so a QML `Image`
 * can render the real page preview — replacing the old integer model
 * (`pageCount`) where the delegate only showed the page number.
 *
 * Data source: UBApplication::boardController (a UBDocumentContainer). The model
 * resets on documentThumbnailsUpdated (structural changes: add/delete/move/
 * reload — the only signal available is global) and refreshes a single row on
 * documentPageUpdated(index) (page content regenerated).
 *
 * Cache busting: QML `Image` caches by URL, and a regenerated thumbnail keeps
 * the same file path, so a per-row version counter is appended as `?v=N` to
 * force a reload when a page's content changes.
 */
class UBPageThumbnailModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        ThumbnailUrlRole = Qt::UserRole + 1,  // file:// URL (with ?v=N cache buster)
    };

    explicit UBPageThumbnailModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private slots:
    void onThumbnailsUpdated();       // structural change → full reset
    void onPageUpdated(int index);    // single page content change → dataChanged

private:
    int pageCount() const;

    // Per-row version, bumped when a page's thumbnail is regenerated, appended to
    // the URL so QML reloads the Image instead of serving the cached one.
    QList<int> m_versions;
};

#endif // UBPAGETHUMBNAILMODEL_H
