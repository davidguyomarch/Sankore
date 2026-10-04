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
    // #407 (ADR-0007/0009): the page's background is the see-through "Bureau"
    // kind (BackgroundKind::SeeThrough). Picking it in the Fond d'écran menu
    // enters desktop mode; picking any other background leaves it.
    Q_PROPERTY(bool isSeeThrough READ isSeeThrough NOTIFY backgroundChanged)

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
    bool isSeeThrough() const;  // #407

    bool canUndo() const;
    bool canRedo() const;

public slots:
    void undo();
    void redo();

    void openPreferences();
    void quit();

    void setBackgroundLight();
    void setBackgroundDark();
    /// #289: set the ruling type (keeps the current dark/light).
    void setGridType(int gridType);
    /// #407 (ADR-0007): make the current page's background the see-through
    /// "Bureau" kind (persisted) and enter desktop mode via the guarded
    /// transition. Choosing any other background (light/dark/grid) leaves it.
    void setBackgroundSeeThrough();

signals:
    void activeModeChanged();
    void backgroundChanged();
    void undoStateChanged();

private slots:
    void onActiveSceneChanged();
    void onUndoChanged(bool canUndo);

private:
    // #407: if the current page is see-through (we are in desktop mode because
    // the user picked the Bureau background), set the kind back to Opaque and
    // LEAVE desktop via the guarded hideDesktop(). Called by the normal
    // background setters so choosing light/dark/grid exits Bureau. Returns true
    // if it left desktop (so the caller can skip conflicting work).
    bool leaveSeeThroughIfActive();

    int m_mode;
};

#endif // UBAPPCONTROLLER_H
