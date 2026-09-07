/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBAPPCONTROLLER_H
#define UBAPPCONTROLLER_H

#include <QObject>

/**
 * UBAppController — exposes app-level state to QML top bar.
 *
 * Mode switching, backgrounds, undo/redo availability.
 */
class UBAppController : public QObject
{
    Q_OBJECT

    // Active mode (0=Board, 1=Documents, 2=Desktop)
    Q_PROPERTY(int activeMode READ activeMode WRITE setActiveMode NOTIFY activeModeChanged)

    // Background state
    Q_PROPERTY(bool isDarkBackground READ isDarkBackground NOTIFY backgroundChanged)
    Q_PROPERTY(bool isCrossedBackground READ isCrossedBackground NOTIFY backgroundChanged)
    // #289: full ruling type (0=Plain,1=Grid,2=Seyes,3=SeyesLarge,4=Double3mm)
    Q_PROPERTY(int gridType READ gridType NOTIFY backgroundChanged)

    // Undo/Redo
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoStateChanged)

public:
    enum Mode { Board = 0, Documents, Desktop };
    Q_ENUM(Mode)

    explicit UBAppController(QObject* parent = nullptr);

    int activeMode() const;
    void setActiveMode(int mode);

    // Update mode state without triggering actions (used when mode changes
    // externally, e.g. returning from Desktop via legacy path)
    void syncMode(int mode);

    bool isDarkBackground() const;
    bool isCrossedBackground() const;
    int gridType() const;

    bool canUndo() const;
    bool canRedo() const;

public slots:
    void undo();
    void redo();

    void openPreferences();
    void quit();

    void setBackgroundLight();
    void setBackgroundDark();
    void setBackgroundCrossedLight();
    void setBackgroundCrossedDark();
    void setBackgroundPlainLight();
    void setBackgroundPlainDark();
    void toggleGrid();
    /// #289: set the ruling type (keeps the current dark/light).
    void setGridType(int gridType);

signals:
    void activeModeChanged();
    void backgroundChanged();
    void undoStateChanged();

private slots:
    void onActiveSceneChanged();
    void onUndoChanged(bool canUndo);

private:
    int m_mode;
};

#endif // UBAPPCONTROLLER_H
