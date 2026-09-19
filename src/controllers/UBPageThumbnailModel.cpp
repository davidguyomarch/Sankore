/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBPageThumbnailModel.h"

#include <QUrl>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "document/UBDocumentProxy.h"

UBPageThumbnailModel::UBPageThumbnailModel(QObject* parent)
    : QAbstractListModel(parent)
{
    // Structural changes (add/delete/move/reload) only emit the global
    // documentThumbnailsUpdated — resync with a full reset.
    connect(UBApplication::boardController, &UBBoardController::documentThumbnailsUpdated,
            this, &UBPageThumbnailModel::onThumbnailsUpdated);
    // A single page's content changed (re-rendered thumbnail).
    connect(UBApplication::boardController, &UBBoardController::documentPageUpdated,
            this, &UBPageThumbnailModel::onPageUpdated);

    m_versions = QList<int>(pageCount(), 0);
}

int UBPageThumbnailModel::pageCount() const
{
    auto* doc = UBApplication::boardController
                    ? UBApplication::boardController->selectedDocument() : nullptr;
    return doc ? doc->pageCount() : 0;
}

int UBPageThumbnailModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return pageCount();
}

QVariant UBPageThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    if (row < 0 || row >= pageCount())
        return QVariant();

    if (role == ThumbnailUrlRole)
    {
        auto* doc = UBApplication::boardController->selectedDocument();
        if (!doc)
            return QString();

        // #359: guarantee the thumbnail file exists before handing out its URL.
        // Otherwise a page whose thumbnail has not been generated yet (never
        // persisted/selected) would resolve to a missing file, the QML Image
        // would go to status=Error and — because the URL never changes until
        // the page is selected (which bumps ?v=N) — stay blank. Generating it
        // on demand here makes the preview appear without selecting the page.
        UBThumbnailAdaptor::ensureThumbnail(doc, row);

        QUrl url = UBThumbnailAdaptor::thumbnailUrl(doc, row);
        // Append a per-row version so QML's Image cache reloads the file after a
        // regeneration (same path would otherwise serve the cached image).
        const int v = (row < m_versions.size()) ? m_versions.at(row) : 0;
        return url.toString() + QStringLiteral("?v=") + QString::number(v);
    }

    return QVariant();
}

QHash<int, QByteArray> UBPageThumbnailModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ThumbnailUrlRole] = "thumbnailUrl";
    return roles;
}

void UBPageThumbnailModel::onThumbnailsUpdated()
{
    // Structural change with no index info: reset. Keep per-row versions across
    // the reset by resizing (a moved/removed page keeps a non-stale URL because
    // the file path is index-based and the reset re-reads every row anyway).
    beginResetModel();
    const int count = pageCount();
    if (m_versions.size() != count)
        m_versions.resize(count);  // new entries default-init to 0
    // Bump every version so any regenerated file (same path) is reloaded.
    for (int& v : m_versions)
        ++v;
    endResetModel();
}

void UBPageThumbnailModel::onPageUpdated(int index)
{
    if (index < 0 || index >= pageCount())
        return;
    if (index >= m_versions.size())
        m_versions.resize(pageCount());
    ++m_versions[index];
    const QModelIndex mi = createIndex(index, 0);
    emit dataChanged(mi, mi, { ThumbnailUrlRole });
}
