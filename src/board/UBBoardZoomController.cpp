/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#include "UBBoardZoomController.h"
#include "UBBoardController.h"
#include "UBBoardView.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QScrollBar>
#include <QTransform>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsScene.h"
#include "frameworks/UBPureFunctions.h"

#define UB_MAX_ZOOM 9.0

UBBoardZoomController::UBBoardZoomController(UBBoardController* boardController, QObject* parent)
    : QObject(parent)
    , mBoardController(boardController)
{
}

void UBBoardZoomController::zoomIn(QPointF scenePoint)
{
    if (mBoardController->controlView()->transform().m11() > UB_MAX_ZOOM)
    {
        qApp->beep();
        return;
    }
    zoom(mBoardController->zoomFactor(), scenePoint);
}

void UBBoardZoomController::zoomOut(QPointF scenePoint)
{
    if ((mBoardController->controlView()->horizontalScrollBar()->maximum() == 0) &&
        (mBoardController->controlView()->verticalScrollBar()->maximum() == 0))
    {
        qApp->beep();
        return;
    }

    qreal newZoomFactor = 1.0 / mBoardController->zoomFactor();
    zoom(newZoomFactor, scenePoint);
}

void UBBoardZoomController::zoomRestore()
{
    QTransform tr;
    tr.scale(mBoardController->systemScaleFactor(), mBoardController->systemScaleFactor());
    mBoardController->controlView()->setTransform(tr);

    centerRestore();

    for (QGraphicsItem *gi : mBoardController->activeScene()->selectedItems())
    {
        gi->setSelected(false);
        gi->setSelected(true);
    }

    emit mBoardController->zoomChanged(1.0);
}

void UBBoardZoomController::centerRestore()
{
    centerOn(QPointF(0, 0));
}

void UBBoardZoomController::centerOn(QPointF scenePoint)
{
    mBoardController->controlView()->centerOn(scenePoint);
    UBApplication::applicationController->adjustDisplayView();
}

/**
 * @brief Compute the effective zoom ratio, clamped to UB_MAX_ZOOM.
 *
 * Pure function — no side effects, testable in isolation.
 * @param requestedRatio The zoom ratio requested
 * @param currentViewScale The current m11() of the view transform
 * @param systemScaleFactor The system scale factor
 * @param maxZoom Maximum allowed zoom level
 * @return QPair<qreal, qreal> (clampedZoom, usedRatio)
 */
QPair<qreal, qreal> UBBoardZoomController::computeZoomRatio(qreal requestedRatio, qreal currentViewScale, qreal systemScaleFactor, qreal maxZoom)
{
    return UBPure::computeZoomRatio(requestedRatio, currentViewScale, systemScaleFactor, maxZoom);
}

void UBBoardZoomController::zoom(const qreal ratio, QPointF scenePoint)
{
    UBBoardView* controlView = mBoardController->controlView();

    QPointF viewCenter = controlView->mapToScene(QRect(0, 0, controlView->width(), controlView->height()).center());
    QPointF offset = scenePoint - viewCenter;
    QPointF scalledOffset = offset / ratio;

    qreal currentZoom = ratio * controlView->viewportTransform().m11() / mBoardController->systemScaleFactor();

    qreal usedRatio = ratio;
    if (currentZoom > UB_MAX_ZOOM)
    {
        currentZoom = UB_MAX_ZOOM;
        usedRatio = currentZoom * mBoardController->systemScaleFactor() / controlView->viewportTransform().m11();
    }

    controlView->scale(usedRatio, usedRatio);

    QPointF newCenter = scenePoint - scalledOffset;
    controlView->centerOn(newCenter);

    emit mBoardController->zoomChanged(currentZoom);
    UBApplication::applicationController->adjustDisplayView();

    emit mBoardController->controlViewportChanged();
    mBoardController->activeScene()->setBackgroundZoomFactor(controlView->transform().m11());
}

void UBBoardZoomController::handScroll(qreal dx, qreal dy)
{
    mBoardController->controlView()->translate(dx, dy);
    UBApplication::applicationController->adjustDisplayView();
    emit mBoardController->controlViewportChanged();
}

void UBBoardZoomController::persistViewPositionOnCurrentScene()
{
    UBBoardView* controlView = mBoardController->controlView();
    QRect rect = controlView->rect();
    QPoint center(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    QPointF viewRelativeCenter = controlView->mapToScene(center);
    mBoardController->activeScene()->setLastCenter(viewRelativeCenter);
}

void UBBoardZoomController::updateSystemScaleFactor()
{
    qreal newScaleFactor = 1.0;

    if (mBoardController->activeScene())
    {
        QSize pageNominalSize = mBoardController->activeScene()->nominalSize();
        QSize controlSize = mBoardController->controlViewport();

        qreal hFactor = ((qreal)controlSize.width()) / ((qreal)pageNominalSize.width());
        qreal vFactor = ((qreal)controlSize.height()) / ((qreal)pageNominalSize.height());

        newScaleFactor = qMin(hFactor, vFactor);
    }

    if (mBoardController->systemScaleFactor() != newScaleFactor)
    {
        mBoardController->setSystemScaleFactor(newScaleFactor);
        emit mBoardController->systemScaleFactorChanged(newScaleFactor);
    }

    UBGraphicsScene::SceneViewState viewState = mBoardController->activeScene()->viewState();

    QTransform scalingTransform;
    qreal scaleFactor = viewState.zoomFactor * mBoardController->systemScaleFactor();
    scalingTransform.scale(scaleFactor, scaleFactor);

    mBoardController->controlView()->setTransform(scalingTransform);
    mBoardController->controlView()->horizontalScrollBar()->setValue(viewState.horizontalPosition);
    mBoardController->controlView()->verticalScrollBar()->setValue(viewState.verticalPostition);
    mBoardController->activeScene()->setBackgroundZoomFactor(mBoardController->controlView()->transform().m11());
}
