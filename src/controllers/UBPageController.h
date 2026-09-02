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

public:
    explicit UBPageController(QObject* parent = nullptr);

    int currentPage() const;
    int pageCount() const;
    bool canGoBack() const;
    bool canGoForward() const;

public slots:
    void nextPage();
    void previousPage();
    void addPage();
    void duplicatePage();
    void deletePage();
    void importPage();
    void goToPage(int index);

signals:
    void currentPageChanged();
    void pageCountChanged();

private slots:
    void onActiveSceneChanged();
    void onDocumentChanged();
};

#endif // UBPAGECONTROLLER_H
