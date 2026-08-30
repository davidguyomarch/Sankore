/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBBACKGROUNDRENDERER_H
#define UBBACKGROUNDRENDERER_H

#include <QObject>
#include <QRectF>
#include <functional>

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
    bool isCrossedBackground() const { return mCrossedBackground; }
    bool isDesktopMode()       const { return mIsDesktopMode; }

    // --- Mutators (called by scene facades) ---

    /**
     * Toggle dark/light and crossed background.
     * Recolors eraser, recolors all stroke items, invalidates views.
     * Returns true if a repaint was triggered.
     */
    bool setBackground(bool isDark, bool isCrossed);

    void setZoomFactor(qreal zoom);
    void setDesktopMode(bool desktopMode);

    // --- Painting (called from scene's drawBackground override) ---

    void paintBackground(QPainter* painter, const QRectF& rect);

private:
    void recolorAllItems();

    QGraphicsScene* mScene;
    std::function<void(bool)> mOnEraserRecolor;

    bool  mDarkBackground    = false;
    bool  mCrossedBackground = false;
    bool  mIsDesktopMode     = false;
    qreal mZoomFactor        = 1.0;
};

#endif // UBBACKGROUNDRENDERER_H
