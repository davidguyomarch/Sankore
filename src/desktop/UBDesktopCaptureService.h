/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBDESKTOPCAPTURESERVICE_H
#define UBDESKTOPCAPTURESERVICE_H

#include <QObject>
#include <QPixmap>

class QWidget;
class QScreen;
class UBGraphicsScene;

/**
 * UBDesktopCaptureService — #521 / ADR-0013 (brick 1).
 *
 * Grabs the desktop of the screen a given window is on and feeds it to a
 * UBGraphicsScene as the page's TRANSIENT background capture (painted cover-fit
 * on a see-through page, via UBGraphicsScene::setDesktopCapturePixmap). The
 * capture is runtime-only and NEVER serialized (ADR-0013: a Bureau page persists
 * only its SeeThrough flag; the desktop is re-grabbed on reopen / focus-return).
 *
 * This is the single-window path: there is NO overlay. The service only grabs a
 * screen (reusing the proven `QScreen::grabWindow(0, …)` mechanism) and hands the
 * pixmap to the scene; the board view paints it in drawBackground.
 *
 * Brick 1 scope: the grab + hand-off only. Refresh triggers (entry /
 * focus-return) and the single-window Bureau entry are later bricks.
 *
 * Not unit-testable: a real screen grab + rendering are VM-only. The pure
 * cover-fit geometry (UBBackgroundGrid::coverRect) and the "never serialized"
 * contract are covered by unit tests separately.
 */
class UBDesktopCaptureService : public QObject
{
    Q_OBJECT

public:
    explicit UBDesktopCaptureService(QObject* parent = nullptr);

    /**
     * Grab the desktop of the screen @p onWindow is displayed on (fallback: the
     * primary screen) and return it. May be null if the grab fails. Does NOT
     * touch any scene — see refreshInto() for the hand-off.
     *
     * NB: never call processEvents() around the grab (re-entrancy crash, #135).
     */
    QPixmap grabDesktop(const QWidget* onWindow) const;

    /**
     * Grab the desktop (via grabDesktop) and set it as @p scene's transient
     * capture. A null/failed grab clears the capture (passes a null pixmap).
     * Returns true if a non-null capture was applied.
     */
    bool refreshInto(UBGraphicsScene* scene, const QWidget* onWindow) const;

    /**
     * Clear @p scene's transient desktop capture (null pixmap). Used when leaving
     * Bureau so the page paints normally again. Never serialized either way.
     */
    void clear(UBGraphicsScene* scene) const;

private:
    QScreen* screenFor(const QWidget* onWindow) const;
};

#endif // UBDESKTOPCAPTURESERVICE_H
