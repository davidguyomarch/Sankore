/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBMagnifierHandler.h"

#include <QApplication>
#include <QGraphicsView>
#include <QWidget>

#include "frameworks/UBCoreGraphicsScene.h"
#include "gui/UBMagnifer.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"

UBMagnifierHandler::UBMagnifierHandler(UBCoreGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , mScene(scene)
{
}

UBMagnifierHandler::~UBMagnifierHandler()
{
    disposeMagnifierWidgets();
}

void UBMagnifierHandler::addMagnifier(UBMagnifierParams params)
{
    // Can have only one magnifier at one time
    if (mControlWidget)
        return;

    QWidget* cContainer = static_cast<QWidget*>(UBApplication::boardController->controlContainer());
    QGraphicsView* cView = static_cast<QGraphicsView*>(UBApplication::boardController->controlView());
    QGraphicsView* dView = static_cast<QGraphicsView*>(UBApplication::boardController->displayView());

    QPoint dvZeroPoint = dView->mapToGlobal(QPoint(0, 0));

    int cvW = cView->width();
    int dvW = dView->width();
    qreal wCoeff = static_cast<qreal>(dvW) / static_cast<qreal>(cvW);

    int cvH = cView->height();
    int dvH = dView->height();
    qreal hCoeff = static_cast<qreal>(dvH) / static_cast<qreal>(cvH);

    QPoint ccPoint(params.x, params.y);
    QPoint globalPoint = cContainer->mapToGlobal(ccPoint);
    QPoint cvPoint = cView->mapFromGlobal(globalPoint);
    QPoint dvPoint(cvPoint.x() * wCoeff + dvZeroPoint.x(),
                   cvPoint.y() * hCoeff + dvZeroPoint.y());

    mControlWidget = new UBMagnifier(static_cast<QWidget*>(UBApplication::boardController->controlContainer()), true);
    mControlWidget->setGrabView(static_cast<QGraphicsView*>(UBApplication::boardController->controlView()));
    mControlWidget->setMoveView(static_cast<QWidget*>(UBApplication::boardController->controlContainer()));
    mControlWidget->setSize(params.sizePercentFromScene);
    mControlWidget->setZoom(params.zoom);

    mDisplayWidget = new UBMagnifier(static_cast<QWidget*>(UBApplication::boardController->displayView()), false);
    mDisplayWidget->setGrabView(static_cast<QGraphicsView*>(UBApplication::boardController->controlView()));
    mDisplayWidget->setMoveView(static_cast<QGraphicsView*>(UBApplication::boardController->displayView()));
    mDisplayWidget->setSize(params.sizePercentFromScene);
    mDisplayWidget->setZoom(params.zoom);

    mControlWidget->grabNMove(globalPoint, globalPoint, true);
    mDisplayWidget->grabNMove(globalPoint, dvPoint, true);
    mControlWidget->show();
    mDisplayWidget->show();

    connect(mControlWidget, &UBMagnifier::magnifierMoved_Signal,
            this, [this](QPoint pos) { moveMagnifier(pos); });
    connect(mControlWidget, &UBMagnifier::magnifierClose_Signal,
            this, &UBMagnifierHandler::closeMagnifier);
    connect(mControlWidget, &UBMagnifier::magnifierZoomIn_Signal,
            this, &UBMagnifierHandler::zoomInMagnifier);
    connect(mControlWidget, &UBMagnifier::magnifierZoomOut_Signal,
            this, &UBMagnifierHandler::zoomOutMagnifier);
    connect(mControlWidget, &UBMagnifier::magnifierDrawingModeChange_Signal,
            this, &UBMagnifierHandler::changeMagnifierMode);
    connect(mControlWidget, &UBMagnifier::magnifierResized_Signal,
            this, &UBMagnifierHandler::resizedMagnifier);

    mScene->setModified(true);
}

void UBMagnifierHandler::moveMagnifier()
{
    if (mControlWidget)
    {
        QPoint magnifierPos = QPoint(
            mControlWidget->pos().x() + mControlWidget->size().width() / 2,
            mControlWidget->pos().y() + mControlWidget->size().height() / 2);
        moveMagnifier(magnifierPos, true);
        mScene->setModified(true);
    }
}

void UBMagnifierHandler::moveMagnifier(QPoint newPos, bool forceGrab)
{
    QWidget* cContainer = static_cast<QWidget*>(UBApplication::boardController->controlContainer());
    QGraphicsView* cView = static_cast<QGraphicsView*>(UBApplication::boardController->controlView());
    QGraphicsView* dView = static_cast<QGraphicsView*>(UBApplication::boardController->displayView());

    QPoint dvZeroPoint = dView->mapToGlobal(QPoint(0, 0));

    int cvW = cView->width();
    int dvW = dView->width();
    qreal wCoeff = static_cast<qreal>(dvW) / static_cast<qreal>(cvW);

    int cvH = cView->height();
    int dvH = dView->height();
    qreal hCoeff = static_cast<qreal>(dvH) / static_cast<qreal>(cvH);

    QPoint globalPoint = cContainer->mapToGlobal(newPos);
    QPoint cvPoint = cView->mapFromGlobal(globalPoint);
    QPoint dvPoint(cvPoint.x() * wCoeff + dvZeroPoint.x(),
                   cvPoint.y() * hCoeff + dvZeroPoint.y());

    mControlWidget->grabNMove(globalPoint, globalPoint, forceGrab, false);
    mDisplayWidget->grabNMove(globalPoint, dvPoint, forceGrab, true);

    mScene->setModified(true);
}

void UBMagnifierHandler::closeMagnifier()
{
    disposeMagnifierWidgets();
    mScene->setModified(true);
}

void UBMagnifierHandler::zoomInMagnifier()
{
    if (mControlWidget->params.zoom < 8)
    {
        mControlWidget->setZoom(mControlWidget->params.zoom + 0.5);
        mDisplayWidget->setZoom(mDisplayWidget->params.zoom + 0.5);
    }
}

void UBMagnifierHandler::zoomOutMagnifier()
{
    if (mControlWidget->params.zoom > 1)
    {
        mControlWidget->setZoom(mControlWidget->params.zoom - 0.5);
        mDisplayWidget->setZoom(mDisplayWidget->params.zoom - 0.5);
        mScene->setModified(true);
    }
}

void UBMagnifierHandler::changeMagnifierMode(int mode)
{
    if (mControlWidget)
        mControlWidget->setDrawingMode(mode);
    if (mDisplayWidget)
        mDisplayWidget->setDrawingMode(mode);
}

void UBMagnifierHandler::resizedMagnifier(qreal newPercent)
{
    if (newPercent > 18 && newPercent < 50)
    {
        mControlWidget->setSize(newPercent);
        mControlWidget->grabPoint();
        mDisplayWidget->setSize(newPercent);
        mDisplayWidget->grabPoint();
        mScene->setModified(true);
    }
}

void UBMagnifierHandler::disposeMagnifierWidgets()
{
    if (mControlWidget)
    {
        mControlWidget->hide();
        mControlWidget->setParent(nullptr);
        delete mControlWidget;
        mControlWidget = nullptr;
    }

    if (mDisplayWidget)
    {
        mDisplayWidget->hide();
        mDisplayWidget->setParent(nullptr);
        delete mDisplayWidget;
        mDisplayWidget = nullptr;
    }

    // Sometimes crashes on access to app (when called from destructor when closing the app)
    try
    {
        UBApplication::app()->restoreOverrideCursor();
    }
    catch (...)
    {
    }
}
