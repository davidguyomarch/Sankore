/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBDocumentActionController.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "document/UBDocumentController.h"
#include "gui/UBMainWindow.h"

UBDocumentActionController::UBDocumentActionController(QObject* parent)
    : QObject(parent)
{
}

int UBDocumentActionController::activeMode() const
{
    return 1; // Always Documents when this controller is active
}

void UBDocumentActionController::setActiveMode(int mode)
{
    if (mode == 0) // Board
        UBApplication::applicationController->showBoard();
    else if (mode == 2) // Desktop
        UBApplication::applicationController->showDesktop();
    // mode == 1 is already Documents — no-op
    emit activeModeChanged();
}

bool UBDocumentActionController::hasSelection() const
{
    auto* dc = UBApplication::documentController;
    return dc && dc->firstSelectedTreeProxy() != nullptr;
}

QString UBDocumentActionController::documentTitle() const
{
    auto* dc = UBApplication::documentController;
    if (dc && dc->firstSelectedTreeProxy())
        return dc->firstSelectedTreeProxy()->metaData(UBSettings::documentName).toString();
    return QString();
}

void UBDocumentActionController::newDocument()
{
    if (UBApplication::mainWindow->actionNewDocument)
        UBApplication::mainWindow->actionNewDocument->trigger();
}

void UBDocumentActionController::newFolder()
{
    if (UBApplication::mainWindow->actionNewFolder)
        UBApplication::mainWindow->actionNewFolder->trigger();
}

void UBDocumentActionController::importFile()
{
    if (UBApplication::mainWindow->actionImport)
        UBApplication::mainWindow->actionImport->trigger();
}

void UBDocumentActionController::exportDocument()
{
    if (UBApplication::mainWindow->actionExport)
        UBApplication::mainWindow->actionExport->trigger();
}

void UBDocumentActionController::renameItem()
{
    if (UBApplication::mainWindow->actionRename)
        UBApplication::mainWindow->actionRename->trigger();
}

void UBDocumentActionController::duplicateItem()
{
    if (UBApplication::mainWindow->actionDuplicate)
        UBApplication::mainWindow->actionDuplicate->trigger();
}

void UBDocumentActionController::deleteItem()
{
    if (UBApplication::mainWindow->actionDelete)
        UBApplication::mainWindow->actionDelete->trigger();
}

void UBDocumentActionController::openInBoard()
{
    if (UBApplication::mainWindow->actionOpen)
        UBApplication::mainWindow->actionOpen->trigger();
}

void UBDocumentActionController::quit()
{
    UBApplication::app()->closeAllWindows();
}
