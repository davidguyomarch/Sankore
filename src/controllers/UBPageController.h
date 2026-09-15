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
    // #328: real thumbnail model (QAbstractListModel) for the QML PageNavigator,
    // exposing a file:// URL per page so the delegate renders the actual page
    // preview. Supersedes the revision/pageCount integer-model workaround (#321).
    Q_PROPERTY(QObject* thumbnailModel READ thumbnailModel CONSTANT)

public:
    explicit UBPageController(QObject* parent = nullptr);

    int currentPage() const;
    int pageCount() const;
    bool canGoBack() const;
    bool canGoForward() const;
    QObject* thumbnailModel() const;

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

private slots:
    void onActiveSceneChanged();
    void onDocumentChanged();

private:
    class UBPageThumbnailModel* m_thumbnailModel = nullptr;
};

#endif // UBPAGECONTROLLER_H
