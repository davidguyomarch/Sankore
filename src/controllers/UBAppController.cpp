/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBAppController.h"

#include <QTimer>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

#include <QUndoStack>
#include <QFile>
#include <QTextStream>
#include <QApplication>

UBAppController::UBAppController(QObject* parent)
    : QObject(parent)
    , m_mode(Board)
{
    // Track scene changes for background state
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged,
            this, &UBAppController::onActiveSceneChanged);

    // Track undo/redo state
    if (UBApplication::undoStack)
    {
        connect(UBApplication::undoStack, &QUndoStack::canUndoChanged,
                this, &UBAppController::onUndoChanged);
        connect(UBApplication::undoStack, &QUndoStack::canRedoChanged,
                this, [this]() { emit undoStateChanged(); });
    }
}

// --- Mode ---

int UBAppController::activeMode() const
{
    return m_mode;
}

void UBAppController::setActiveMode(int mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;

    // Log mode change for diagnostics
    {
        QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "\n[MODE CHANGE] mode=" << mode
                << (mode == Board ? " (Board)" : mode == Desktop ? " (Desktop)" : " (Documents)")
                << "\n";
            logFile.close();
        }
    }

    switch (mode)
    {
    case Board:
        UBApplication::app()->showBoard();
        break;
    case Documents:
        UBApplication::app()->showDocument();
        break;
    case Desktop:
        UBApplication::applicationController->showDesktop();
        break;
    }

    emit activeModeChanged();
}

void UBAppController::syncMode(int mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    emit activeModeChanged();
}

// --- Background ---

bool UBAppController::isDarkBackground() const
{
    if (UBApplication::isClosing() || !UBApplication::boardController)
        return false;
    auto* scene = UBApplication::boardController->activeScene();
    return scene ? scene->isDarkBackground() : false;
}

bool UBAppController::isCrossedBackground() const
{
    if (UBApplication::isClosing() || !UBApplication::boardController)
        return false;
    auto* scene = UBApplication::boardController->activeScene();
    return scene ? scene->isCrossedBackground() : false;
}

void UBAppController::setBackgroundLight()
{
    UBApplication::boardController->changeBackground(false, false);
    emit backgroundChanged();
}

void UBAppController::setBackgroundDark()
{
    UBApplication::boardController->changeBackground(true, false);
    emit backgroundChanged();
}

void UBAppController::setBackgroundCrossedLight()
{
    UBApplication::boardController->changeBackground(false, true);
    emit backgroundChanged();
}

void UBAppController::setBackgroundCrossedDark()
{
    UBApplication::boardController->changeBackground(true, true);
    emit backgroundChanged();
}

void UBAppController::setBackgroundPlainLight()
{
    UBApplication::boardController->changeBackground(false, false);
    emit backgroundChanged();
}

void UBAppController::setBackgroundPlainDark()
{
    UBApplication::boardController->changeBackground(true, false);
    emit backgroundChanged();
}

void UBAppController::toggleGrid()
{
    if (UBApplication::isClosing() || !UBApplication::boardController)
        return;
    auto* scene = UBApplication::boardController->activeScene();
    if (scene)
    {
        bool dark = scene->isDarkBackground();
        bool crossed = !scene->isCrossedBackground();
        UBApplication::boardController->changeBackground(dark, crossed);
        emit backgroundChanged();
    }
}

// --- Undo/Redo ---

bool UBAppController::canUndo() const
{
    if (UBApplication::isClosing())
        return false;
    return UBApplication::undoStack ? UBApplication::undoStack->canUndo() : false;
}

bool UBAppController::canRedo() const
{
    if (UBApplication::isClosing())
        return false;
    return UBApplication::undoStack ? UBApplication::undoStack->canRedo() : false;
}

void UBAppController::undo()
{
    if (UBApplication::undoStack)
        UBApplication::undoStack->undo();
}

void UBAppController::redo()
{
    if (UBApplication::undoStack)
        UBApplication::undoStack->redo();
}

void UBAppController::openPreferences()
{
    if (UBApplication::mainWindow && UBApplication::mainWindow->actionPreferences)
        UBApplication::mainWindow->actionPreferences->trigger();
}

void UBAppController::quit()
{
    // Log the quit attempt
    {
        QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "\n[QUIT] quit() called\n";
            logFile.close();
        }
    }
    // UBMainWindow::closeEvent ignores QCloseEvent, so QApplication::quit() alone
    // won't close the window. closing() saves state and defers the real quit via
    // QTimer::singleShot(0, qApp, &QApplication::quit), which unwinds app.exec()
    // and lets UBApplication::cleanup() destroy controllers/QML in the right order.
    UBApplication::app()->closing();

    // Watchdog only: if the deferred quit somehow does not unwind the event loop,
    // force-exit as a last resort. Kept long so it never races the normal
    // shutdown path (which previously used ::exit(0) at 500ms and killed the
    // process mid QML-binding-evaluation — see #293).
    QTimer::singleShot(5000, []() { ::exit(0); });
}

// --- Private slots ---

void UBAppController::onActiveSceneChanged()
{
    // During shutdown the board/scene state is being torn down; re-emitting
    // would make QML bindings re-evaluate against dangling objects (#293).
    if (UBApplication::isClosing())
        return;
    emit backgroundChanged();
}

void UBAppController::onUndoChanged(bool)
{
    // boardController->closing() calls ClearUndoStack(), which fires
    // canUndo/canRedoChanged. Swallow it during shutdown so QML does not
    // re-evaluate appController.canUndo/canRedo bindings mid-teardown (#293).
    if (UBApplication::isClosing())
        return;
    emit undoStateChanged();
}
