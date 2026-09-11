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

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QTimer>

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
    if (UBApplication::isClosing())
        return false;
    auto* dc = UBApplication::documentController;
    return dc && dc->firstSelectedTreeProxy() != nullptr;
}

QString UBDocumentActionController::documentTitle() const
{
    if (UBApplication::isClosing())
        return QString();
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
    // The Documents top-bar quit button previously called closeAllWindows(),
    // but UBMainWindow::closeEvent() calls event->ignore(), so the window
    // never closed and nothing happened (#281). Use the same shutdown path as
    // the board top bar (UBAppController::quit): closing() saves state and
    // defers the real quit via QApplication::quit(), letting cleanup() tear
    // down controllers/QML in order.
    {
        QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "\n[QUIT] quit() called (Documents top bar)\n";
            logFile.close();
        }
    }

    UBApplication::app()->closing();

    // Watchdog: force-exit only if the deferred quit does not unwind the event
    // loop. 1.5s bounds the worst-case perceived delay (#309) without racing the
    // normal ordered shutdown, and without returning to the old 500ms ::exit(0)
    // that caused #293. Reaching it is abnormal — log it.
    QTimer::singleShot(1500, []() {
        QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "[QUIT] WATCHDOG fired at 1500ms (Documents) — forcing exit(0)\n";
            logFile.close();
        }
        ::exit(0);
    });
}
