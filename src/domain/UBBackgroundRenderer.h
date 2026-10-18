/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBBACKGROUNDRENDERER_H
#define UBBACKGROUNDRENDERER_H

#include <QObject>
#include <QRectF>
#include <QColor>
#include <QString>
#include <QPixmap>
#include <functional>

#include "UBBackgroundGrid.h"

class QPainter;
class QGraphicsScene;
class QGraphicsView;

/**
 * @brief Owns background state and rendering for UBGraphicsScene.
 *
 * Extracted as phase 2 of the UBGraphicsScene decomposition (#111).
 *
 * Manages dark/light toggle, crossed grid, desktop mode, zoom factor,
 * and the recolorAllItems() pass that switches stroke colors on
 * background change.
 */
class UBBackgroundRenderer : public QObject
{
    Q_OBJECT

public:
    /**
     * @param scene  The QGraphicsScene to query items()/views() from.
     * @param onEraserRecolor  Callback invoked when the eraser visual
     *        needs recoloring after a dark/light toggle.  The bool
     *        argument is true when the new background is dark.
     */
    explicit UBBackgroundRenderer(QGraphicsScene* scene,
                                  std::function<void(bool isDark)> onEraserRecolor = nullptr,
                                  QObject* parent = nullptr);

    // --- State accessors ---

    bool isDarkBackground()    const { return mDarkBackground; }
    bool isLightBackground()   const { return !mDarkBackground; }
    /// Legacy compatibility shim: "crossed" == any ruled background (#289).
    bool isCrossedBackground() const { return UBBackgroundGrid::isRuled(mGridType); }
    /// The full ruling type (#289).
    UBBackgroundGrid::Type gridType() const { return mGridType; }
    // #393 (ADR-0007): a see-through page has no opaque fill (the desktop shows
    // through). This supersedes the old scene-wide "desktop mode" flag, which is
    // now a backward-compatible shim over the background kind (see .cpp).
    UBBackgroundGrid::BackgroundKind backgroundKind() const { return mBackgroundKind; }
    bool isSeeThrough()        const { return mBackgroundKind == UBBackgroundGrid::BackgroundKind::SeeThrough; }
    bool isImageBackground()   const { return mBackgroundKind == UBBackgroundGrid::BackgroundKind::Image; }
    bool isDesktopMode()       const { return isSeeThrough(); }

    // #389 / ADR-0011: custom background image. The RELATIVE file name is what is
    // serialized in the .ubz (background-image attribute); the pixmap is the
    // decoded, cached image used at paint time. An empty name / null pixmap with
    // kind==Image degrades to Opaque at paint (ADR-0011).
    QString backgroundImageFile() const { return mBackgroundImageFile; }
    const QPixmap& backgroundImagePixmap() const { return mBackgroundImagePixmap; }

    // #521 / ADR-0013 (brick 1): the TRANSIENT desktop capture painted as the
    // surface of a see-through page. Distinct from the (serialized) image above:
    // this pixmap is runtime-only, NEVER written to the .ubz — a Bureau page
    // persists only its SeeThrough flag and re-grabs the current desktop on
    // reopen. Null = no capture (the see-through page paints as before).
    const QPixmap& desktopCapturePixmap() const { return mDesktopCapturePixmap; }
    bool hasDesktopCapture() const { return !mDesktopCapturePixmap.isNull(); }

    // --- Mutators (called by scene facades) ---

    /**
     * Toggle dark/light and crossed background.
     * `isCrossed` maps to Grid (true) or Plain (false) for backward
     * compatibility; use setBackgroundType() for the finer rulings (#289).
     * Recolors eraser, recolors all stroke items, invalidates views.
     * Returns true if a repaint was triggered.
     */
    bool setBackground(bool isDark, bool isCrossed);

    /// Set dark/light plus the full ruling type (#289). Returns true if a
    /// repaint was triggered.
    bool setBackgroundType(bool isDark, UBBackgroundGrid::Type gridType);

    void setZoomFactor(qreal zoom);

    /// Set the page background kind (#393). Returns true if a repaint is needed.
    bool setBackgroundKind(UBBackgroundGrid::BackgroundKind kind);
    /// Backward-compatible shim: desktopMode==true → SeeThrough, else Opaque.
    void setDesktopMode(bool desktopMode);

    // #389 / ADR-0011: set the background image. @p fileName is the relative name
    // stored in the .ubz; @p pixmap is the decoded image (may be null if not yet
    // loaded). Does NOT change the kind — the caller sets kind==Image separately.
    void setBackgroundImage(const QString& fileName, const QPixmap& pixmap);

    // #521 / ADR-0013 (brick 1): set (or clear, with a null pixmap) the transient
    // desktop capture painted on a see-through page. Non-persisting by design —
    // it invalidates the view cache but NEVER marks the scene modified, so the
    // capture is never serialized (modeled on setDesktopMode, not
    // setBackgroundImage). Does NOT change the kind.
    void setDesktopCapturePixmap(const QPixmap& pixmap);

    // --- Painting (called from scene's drawBackground override) ---

    void paintBackground(QPainter* painter, const QRectF& rect);

private:
    void recolorAllItems();

    QGraphicsScene* mScene;
    std::function<void(bool)> mOnEraserRecolor;

    bool  mDarkBackground    = false;
    UBBackgroundGrid::Type mGridType = UBBackgroundGrid::Type::Plain;
    UBBackgroundGrid::BackgroundKind mBackgroundKind = UBBackgroundGrid::BackgroundKind::Opaque;
    qreal mZoomFactor        = 1.0;

    // #389 / ADR-0011: background image (file name serialized, pixmap cached).
    QString mBackgroundImageFile;
    QPixmap mBackgroundImagePixmap;

    // #521 / ADR-0013 (brick 1): transient desktop capture (runtime-only, never
    // serialized). Painted cover-fit on a see-through page when non-null.
    QPixmap mDesktopCapturePixmap;
};

#endif // UBBACKGROUNDRENDERER_H
