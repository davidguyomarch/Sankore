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
    bool isDesktopMode()       const { return mIsDesktopMode; }

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
    void setDesktopMode(bool desktopMode);

    // --- Painting (called from scene's drawBackground override) ---

    void paintBackground(QPainter* painter, const QRectF& rect);

private:
    void recolorAllItems();

    QGraphicsScene* mScene;
    std::function<void(bool)> mOnEraserRecolor;

    bool  mDarkBackground    = false;
    UBBackgroundGrid::Type mGridType = UBBackgroundGrid::Type::Plain;
    bool  mIsDesktopMode     = false;
    qreal mZoomFactor        = 1.0;
};

#endif // UBBACKGROUNDRENDERER_H
