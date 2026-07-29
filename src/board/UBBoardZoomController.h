/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBBOARDZOOMCONTROLLER_H
#define UBBOARDZOOMCONTROLLER_H

#include <QObject>
#include "IUBBoardContext.h"
#include <QPointF>
#include <QPair>

class IUBBoardContext;

/**
 * @brief Manages zoom, scroll, and view transformations.
 *
 * Extracted from UBBoardController to reduce its size and improve testability.
 * Handles: zoomIn, zoomOut, zoomRestore, centerRestore, centerOn, zoom, handScroll,
 * persistViewPositionOnCurrentScene, updateSystemScaleFactor.
 */
class UBBoardZoomController : public QObject
{
    Q_OBJECT

public:
    explicit UBBoardZoomController(IUBBoardContext* boardController, QObject* parent = nullptr);

public slots:
    void zoomIn(QPointF scenePoint = QPointF(0, 0));
    void zoomOut(QPointF scenePoint = QPointF(0, 0));
    void zoomRestore();
    void centerRestore();
    void centerOn(QPointF scenePoint = QPointF(0, 0));
    void zoom(const qreal ratio, QPointF scenePoint);
    void handScroll(qreal dx, qreal dy);
    void persistViewPositionOnCurrentScene();
    void updateSystemScaleFactor();

    /**
     * @brief Pure computation: clamp zoom ratio to max.
     * Static so it can be unit-tested without instantiating the controller.
     */
    static QPair<qreal, qreal> computeZoomRatio(qreal requestedRatio, qreal currentViewScale, qreal systemScaleFactor, qreal maxZoom = 9.0);

private:
    IUBBoardContext* mBoardController;
};

#endif // UBBOARDZOOMCONTROLLER_H
