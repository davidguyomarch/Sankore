/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBBackgroundRenderer.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "UBGraphicsStrokesGroup.h"
#include "core/UBSettings.h"

UBBackgroundRenderer::UBBackgroundRenderer(QGraphicsScene* scene,
                                           std::function<void(bool isDark)> onEraserRecolor,
                                           QObject* parent)
    : QObject(parent)
    , mScene(scene)
    , mOnEraserRecolor(std::move(onEraserRecolor))
{
}

bool UBBackgroundRenderer::setBackground(bool isDark, bool isCrossed)
{
    bool needRepaint = false;

    if (mDarkBackground != isDark)
    {
        mDarkBackground = isDark;

        if (mOnEraserRecolor)
            mOnEraserRecolor(mDarkBackground);

        recolorAllItems();

        needRepaint = true;
    }

    if (mCrossedBackground != isCrossed)
    {
        mCrossedBackground = isCrossed;
        needRepaint = true;
    }

    if (needRepaint)
    {
        for (QGraphicsView* view : mScene->views())
        {
            view->resetCachedContent();
        }
    }

    return needRepaint;
}

void UBBackgroundRenderer::setZoomFactor(qreal zoom)
{
    mZoomFactor = zoom;
}

void UBBackgroundRenderer::setDesktopMode(bool desktopMode)
{
    mIsDesktopMode = desktopMode;
}

void UBBackgroundRenderer::recolorAllItems()
{
    QMap<QGraphicsView*, QGraphicsView::ViewportUpdateMode> previousUpdateModes;
    for (QGraphicsView* view : mScene->views())
    {
        previousUpdateModes.insert(view, view->viewportUpdateMode());
        view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    }

    bool currentIsLight = isLightBackground();
    for (QGraphicsItem* item : mScene->items())
    {
        if (item->type() == UBGraphicsStrokesGroup::Type)
        {
            UBGraphicsStrokesGroup* curGroup = static_cast<UBGraphicsStrokesGroup*>(item);
            UBGraphicsStrokesGroup::colorType reqCol =
                currentIsLight ? UBGraphicsStrokesGroup::colorOnLightBackground
                               : UBGraphicsStrokesGroup::colorOnDarkBackground;
            curGroup->setColor(curGroup->color(reqCol));
        }
    }

    for (QGraphicsView* view : mScene->views())
    {
        view->setViewportUpdateMode(previousUpdateModes.value(view));
    }
}

void UBBackgroundRenderer::paintBackground(QPainter* painter, const QRectF& rect)
{
    if (mIsDesktopMode)
        return;  // caller falls through to QGraphicsScene::drawBackground

    if (mDarkBackground)
        painter->fillRect(rect, QBrush(QColor(Qt::black)));
    else
        painter->fillRect(rect, QBrush(QColor(Qt::white)));

    if (mZoomFactor > 0.5)
    {
        QColor bgCrossColor;

        if (mDarkBackground)
            bgCrossColor = UBSettings::crossDarkBackground;
        else
            bgCrossColor = UBSettings::crossLightBackground;

        if (mZoomFactor < 1.0)
        {
            int alpha = 255 * mZoomFactor / 2;
            bgCrossColor.setAlpha(alpha);
        }

        painter->setPen(bgCrossColor);

        if (mCrossedBackground)
        {
            qreal firstY = ((int)(rect.y() / UBSettings::crossSize)) * UBSettings::crossSize;
            for (qreal yPos = firstY; yPos < rect.y() + rect.height(); yPos += UBSettings::crossSize)
            {
                painter->drawLine(rect.x(), yPos, rect.x() + rect.width(), yPos);
            }

            qreal firstX = ((int)(rect.x() / UBSettings::crossSize)) * UBSettings::crossSize;
            for (qreal xPos = firstX; xPos < rect.x() + rect.width(); xPos += UBSettings::crossSize)
            {
                painter->drawLine(xPos, rect.y(), xPos, rect.y() + rect.height());
            }
        }
    }
}
