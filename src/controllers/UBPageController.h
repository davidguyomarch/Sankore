/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBPAGECONTROLLER_H
#define UBPAGECONTROLLER_H

#include <QObject>

/**
 * PageController — exposes page navigation state to QML.
 *
 * Wraps UBBoardController page operations with simple Q_PROPERTY bindings.
 */
class UBPageController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY currentPageChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY currentPageChanged)
    // #321: a monotonically increasing counter bumped on every page mutation
    // (add/delete/duplicate/reorder). The QML thumbnail ListView folds it into
    // its model so the delegates rebuild even when pageCount is unchanged —
    // otherwise a reorder (same count) is invisible. See #328 for the proper
    // thumbnail-model fix.
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)

public:
    explicit UBPageController(QObject* parent = nullptr);

    int currentPage() const;
    int pageCount() const;
    bool canGoBack() const;
    bool canGoForward() const;
    int revision() const { return m_revision; }

public slots:
    void nextPage();
    void previousPage();
    void addPage();
    void duplicatePage();
    void duplicatePageAt(int index);
    void deletePage();
    void deletePageAt(int index);
    void importPage();
    void goToPage(int index);
    // Reorder pages: move the page at `source` to `target` (0-based indices).
    void moveSceneToIndex(int source, int target);

signals:
    void currentPageChanged();
    void pageCountChanged();
    void revisionChanged();

private slots:
    void onActiveSceneChanged();
    void onDocumentChanged();

private:
    void bumpRevision();

    int m_revision = 0;
};

#endif // UBPAGECONTROLLER_H
