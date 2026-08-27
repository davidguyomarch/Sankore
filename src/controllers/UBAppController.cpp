/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBAppController.h"

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

    // Documents mode temporarily disabled — it shows the legacy UBDocumentController
    // which conflicts with QML V2 palettes. Will be reimplemented as a QML view.
    if (mode == Documents)
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
    auto* scene = UBApplication::boardController->activeScene();
    return scene ? scene->isDarkBackground() : false;
}

bool UBAppController::isCrossedBackground() const
{
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
    return UBApplication::undoStack ? UBApplication::undoStack->canUndo() : false;
}

bool UBAppController::canRedo() const
{
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
    // won't close the window. We must call closing() to save state, then exit directly.
    UBApplication::app()->closing();
    // closing() defers quit via singleShot, but the static guard blocks subsequent calls.
    // Force exit in case closing() was already called once.
    QTimer::singleShot(500, []() { ::exit(0); });
}

// --- Private slots ---

void UBAppController::onActiveSceneChanged()
{
    emit backgroundChanged();
}

void UBAppController::onUndoChanged(bool)
{
    emit undoStateChanged();
}
