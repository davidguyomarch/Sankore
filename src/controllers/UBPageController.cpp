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

void UBPageController::deletePage()
{
    UBApplication::boardController->deleteScene(
        UBApplication::boardController->activeSceneIndex());
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
