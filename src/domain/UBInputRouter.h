/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBINPUTROUTER_H
#define UBINPUTROUTER_H

#include <QPointF>

class UBGraphicsScene;
class UBSmoothStrokeItem;

/**
 * @brief Central dispatcher for stylus/mouse input events.
 *
 * Phase 6 (final) of the UBGraphicsScene decomposition (#111).
 *
 * Routes device press/move/release events to the drawing handler,
 * eraser, and pointer overlay based on the active tool. Owns the
 * "pressed" state and the current smooth-stroke pipeline item, and
 * wires completed strokes into the undo stack.
 *
 * Operates on the scene through its public interface — it does not
 * hold any scene-private state beyond its own two fields.
 */
class UBInputRouter
{
public:
    explicit UBInputRouter(UBGraphicsScene* scene);

    bool inputDevicePress(const QPointF& scenePos, qreal pressure);
    bool inputDeviceMove(const QPointF& scenePos, qreal pressure);
    bool inputDeviceRelease();

private:
    UBGraphicsScene* mScene;

    bool mInputDeviceIsPressed = false;
    UBSmoothStrokeItem* mCurrentSmoothStroke = nullptr;
};

#endif // UBINPUTROUTER_H
