/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBTOOLOVERLAY_H
#define UBTOOLOVERLAY_H

#include <QObject>
#include <QPointF>
#include <QSet>

class QGraphicsEllipseItem;
class QGraphicsItem;
class QGraphicsScene;
struct UBSceneContext;

/**
 * @brief Owns the eraser and pointer visual overlay items.
 *
 * Extracted as phase 3 of the UBGraphicsScene decomposition (#111).
 *
 * Creates, positions, shows and hides the two QGraphicsEllipseItem
 * overlays (eraser circle + red pointer dot) that follow the stylus.
 */
class UBToolOverlay : public QObject
{
    Q_OBJECT

public:
    /**
     * @param scene  The QGraphicsScene that owns the overlay items.
     * @param tools  The scene's tool set — overlay items are registered here.
     */
    explicit UBToolOverlay(QGraphicsScene* scene, QSet<QGraphicsItem*>& tools,
                           QObject* parent = nullptr);

    /** Create the eraser ellipse item and add it to the scene. */
    void createEraser();

    /** Create the pointer ellipse item and add it to the scene. */
    void createPointer();

    /** Position (and optionally show) the eraser circle. */
    void drawEraser(const QPointF& pos, const UBSceneContext& ctx, bool isFirstDraw = false);

    /** Position (and optionally show) the pointer circle. */
    void drawPointer(const QPointF& pos, const UBSceneContext& ctx, bool isFirstDraw = false);

    void hideEraser();
    void hidePointer();

    /** Raw access to the eraser item (needed by background recolor callback). */
    QGraphicsEllipseItem* eraserItem() const { return mEraser; }

    /** Raw access to the pointer item (needed by inputDeviceRelease). */
    QGraphicsEllipseItem* pointerItem() const { return mPointer; }

private:
    QGraphicsScene* mScene;
    QSet<QGraphicsItem*>& mTools;

    QGraphicsEllipseItem* mEraser  = nullptr;
    QGraphicsEllipseItem* mPointer = nullptr;
};

#endif // UBTOOLOVERLAY_H
