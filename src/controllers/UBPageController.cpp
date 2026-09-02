/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBPageController.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "document/UBDocumentProxy.h"

UBPageController::UBPageController(QObject* parent)
    : QObject(parent)
{
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged,
            this, &UBPageController::onActiveSceneChanged);
    connect(UBApplication::boardController, &UBBoardController::pageChanged,
            this, &UBPageController::onActiveSceneChanged);
}

int UBPageController::currentPage() const
{
    return UBApplication::boardController->activeSceneIndex() + 1;
}

int UBPageController::pageCount() const
{
    auto* doc = UBApplication::boardController->selectedDocument();
    return doc ? doc->pageCount() : 0;
}

bool UBPageController::canGoBack() const
{
    return UBApplication::boardController->activeSceneIndex() > 0;
}

bool UBPageController::canGoForward() const
{
    auto* doc = UBApplication::boardController->selectedDocument();
    return doc && UBApplication::boardController->activeSceneIndex() < doc->pageCount() - 1;
}

void UBPageController::nextPage()
{
    UBApplication::boardController->nextScene();
}

void UBPageController::previousPage()
{
    UBApplication::boardController->previousScene();
}

void UBPageController::addPage()
{
    UBApplication::boardController->addScene();
}

void UBPageController::duplicatePage()
{
    UBApplication::boardController->duplicateScene();
}

void UBPageController::duplicatePageAt(int index)
{
    // Clone a specific page (from its thumbnail context menu). The copy is
    // inserted right after `index` and becomes the active scene.
    UBApplication::boardController->duplicateScene(index);
}

void UBPageController::deletePage()
{
    UBApplication::boardController->deleteScene(
        UBApplication::boardController->activeSceneIndex());
}

void UBPageController::deletePageAt(int index)
{
    // Delete a specific page (from its thumbnail context menu), not just the
    // active one — otherwise right-clicking a non-active thumbnail would delete
    // the wrong page.
    UBApplication::boardController->deleteScene(index);
}

void UBPageController::importPage()
{
    // Adds pages from a file (image/PDF/document) into the current document
    // and navigates to the first imported page. UBBoardController::importPage()
    // already opens the file dialog via UBDocumentController::addFileToDocument.
    UBApplication::boardController->importPage();
}

void UBPageController::goToPage(int index)
{
    UBApplication::boardController->setActiveDocumentScene(index);
}

void UBPageController::onActiveSceneChanged()
{
    emit currentPageChanged();
    emit pageCountChanged();
}

void UBPageController::onDocumentChanged()
{
    emit currentPageChanged();
    emit pageCountChanged();
}
