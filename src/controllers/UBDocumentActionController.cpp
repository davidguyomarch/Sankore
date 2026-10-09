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
#include "core/UBDocumentManager.h"
#include "document/UBDocumentController.h"
#include "adaptors/UBExportAdaptor.h"
#include "gui/UBMainWindow.h"

#include <QCoreApplication>
#include <QCursor>
#include <QFile>
#include <QMenu>
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
    // mode == 1 is already Documents — no-op.
    // #407: Desktop (mode 2) removed as a mode tab — it is now a page background
    // kind selected from the board's Fond d'écran menu, not reachable here.
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
    // #472: the QML "Exporter" button used to call actionExport->trigger(), but
    // that QAction has no slot — the real export menu was pinned to a QToolButton
    // in the legacy documentToolBar, which is hidden under the QML V2 UI, so
    // export was unreachable. Build the adaptor menu here and run the chosen one
    // directly via UBDocumentController::exportDocumentAt().
    UBDocumentController* dc = UBApplication::documentController;
    if (!dc || !dc->firstSelectedTreeProxy())
        return;  // nothing to export (no document selected)

    const QList<UBExportAdaptor*> adaptors =
        UBDocumentManager::documentManager()->supportedExportAdaptors();
    if (adaptors.isEmpty())
        return;

    QMenu menu;
    for (int i = 0; i < adaptors.length(); ++i)
    {
        UBExportAdaptor* adaptor = adaptors.at(i);
        QAction* act = menu.addAction(adaptor->exportName());
        QObject::connect(act, &QAction::triggered, dc, [dc, i]() { dc->exportDocumentAt(i); });
    }

    menu.exec(QCursor::pos());
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
