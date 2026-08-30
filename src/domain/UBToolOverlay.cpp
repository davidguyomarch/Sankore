/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBToolOverlay.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>

#include "UBSceneContext.h"
#include "core/UB.h"

UBToolOverlay::UBToolOverlay(QGraphicsScene* scene, QSet<QGraphicsItem*>& tools,
                             QObject* parent)
    : QObject(parent)
    , mScene(scene)
    , mTools(tools)
{
}

void UBToolOverlay::createEraser()
{
    mEraser = new QGraphicsEllipseItem();
    mEraser->setRect(QRect(0, 0, 0, 0));
    mEraser->setVisible(false);

    mEraser->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    mEraser->setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Eraiser));

    mTools << mEraser;
    mScene->addItem(mEraser);
}

void UBToolOverlay::createPointer()
{
    mPointer = new QGraphicsEllipseItem();
    mPointer->setRect(QRect(0, 0, 20, 20));
    mPointer->setVisible(false);

    mPointer->setPen(Qt::NoPen);
    mPointer->setBrush(QBrush(QColor(255, 0, 0, 186)));

    mPointer->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));
    mPointer->setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Pointer));

    mTools << mPointer;
    mScene->addItem(mPointer);
}

void UBToolOverlay::drawEraser(const QPointF& pos, const UBSceneContext& ctx, bool isFirstDraw)
{
    qreal eraserWidth = ctx.currentEraserWidth();
    eraserWidth /= ctx.systemScaleFactor();
    eraserWidth /= ctx.currentZoom();

    qreal eraserRadius = eraserWidth / 2;

    if (mEraser)
    {
        mEraser->setRect(QRectF(pos.x() - eraserRadius, pos.y() - eraserRadius,
                                eraserWidth, eraserWidth));
        if (isFirstDraw)
            mEraser->show();
    }
}

void UBToolOverlay::drawPointer(const QPointF& pos, const UBSceneContext& ctx, bool isFirstDraw)
{
    qreal pointerDiameter = ctx.pointerDiameter / ctx.currentZoom();
    qreal pointerRadius = pointerDiameter / 2;

    if (mPointer)
    {
        mPointer->setRect(QRectF(pos.x() - pointerRadius, pos.y() - pointerRadius,
                                 pointerDiameter, pointerDiameter));
        if (isFirstDraw)
            mPointer->show();
    }
}

void UBToolOverlay::hideEraser()
{
    if (mEraser)
        mEraser->hide();
}

void UBToolOverlay::hidePointer()
{
    if (mPointer)
        mPointer->hide();
}
