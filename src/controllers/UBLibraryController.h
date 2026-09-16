/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBLIBRARYCONTROLLER_H
#define UBLIBRARYCONTROLLER_H

#include <QObject>
#include <QString>
#include <QList>

#include "board/UBFeature.h"

class UBLibraryItemModel;

/**
 * UBLibraryController — exposes the media Library (Features) to the QML V2 UI (#258).
 *
 * Replaces the QWidget dock (UBFeaturesWidget). Modeled on UBPageController:
 * a QObject with Q_PROPERTY bindings and a QAbstractListModel exposed to QML.
 *
 * Responsibilities in this first step (PR 1):
 *  - hold the current virtual folder path (breadcrumb state),
 *  - compute the direct children of that folder from a flat feature list,
 *  - feed them to UBLibraryItemModel (exposed as `itemModel`),
 *  - navigation: enter a folder, go up one level.
 *
 * The feature list is *injected* via setFeatures(): in production it comes from
 * UBFeaturesController::getFeatures() (wired in a later PR), in unit tests it is
 * a fabricated list — so this controller's navigation logic is fully testable
 * headless, with no disk scan and no board controller.
 *
 * The virtual filesystem uses paths like "/root", "/root/Pictures",
 * "/root/Pictures/subfolder". A feature's parent folder is getVirtualPath()
 * (alias virtualDir); its own full path is getFullVirtualPath().
 */
class UBLibraryController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(bool canGoUp READ canGoUp NOTIFY currentPathChanged)

public:
    explicit UBLibraryController(QObject* parent = nullptr);

    static QString rootPath() { return QStringLiteral("/root"); }

    QObject* itemModel() const;
    QString currentPath() const { return m_currentPath; }
    bool canGoUp() const { return m_currentPath != rootPath(); }

    // Inject the full flat feature list (all scanned entries). Recomputes the
    // current folder's children.
    void setFeatures(const QList<UBFeature>& features);
    const QList<UBFeature>& features() const { return m_features; }

    // Navigate to an absolute virtual folder path (e.g. "/root/Pictures").
    void setCurrentPath(const QString& path);

    // Direct children of a virtual folder path, computed from the flat list.
    // Pure and static so it is unit-testable in isolation.
    static QList<UBFeature> childrenOf(const QList<UBFeature>& all, const QString& path);

    // Parent folder path of a virtual path ("/root/A/B" -> "/root/A"),
    // clamped at rootPath(). Pure and static.
    static QString parentPath(const QString& path);

public slots:
    // Enter the folder at model row `row` (no-op if it is not a folder).
    void enterFolder(int row);
    // Go up one level (no-op at root).
    void goUp();

signals:
    void currentPathChanged();

private:
    void refreshModel();

    UBLibraryItemModel* m_itemModel;
    QList<UBFeature> m_features;
    QString m_currentPath;
};

#endif // UBLIBRARYCONTROLLER_H
