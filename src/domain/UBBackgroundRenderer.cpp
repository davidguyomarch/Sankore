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
#include "UBSmoothStrokeItem.h"
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
    // Legacy entry point: crossed maps to the uniform square Grid, otherwise
    // Plain. Ruling types go through setBackgroundType() (#289). To avoid
    // clobbering a finer ruling with a plain "crossed=true" toggle, keep the
    // current ruling when it is already ruled and the caller asks for crossed.
    UBBackgroundGrid::Type target;
    if (isCrossed)
        target = UBBackgroundGrid::isRuled(mGridType) ? mGridType
                                                      : UBBackgroundGrid::Type::Grid;
    else
        target = UBBackgroundGrid::Type::Plain;

    return setBackgroundType(isDark, target);
}

bool UBBackgroundRenderer::setBackgroundType(bool isDark, UBBackgroundGrid::Type gridType)
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

    if (mGridType != gridType)
    {
        mGridType = gridType;
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
        else if (item->type() == UBSmoothStrokeItem::Type)
        {
            // #307: modern freehand pen/marker strokes are standalone
            // UBSmoothStrokeItem (never wrapped in a UBGraphicsStrokesGroup),
            // so they were skipped by the recolor loop and stayed black on a
            // dark background (invisible). Repaint them from their stored
            // light/dark color pair, like grouped strokes.
            UBSmoothStrokeItem* stroke = static_cast<UBSmoothStrokeItem*>(item);
            stroke->applyBackgroundColor(!currentIsLight);
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

        if (UBBackgroundGrid::isRuled(mGridType))
        {
            // Minor (interline) colour: same hue as the major grid, fainter.
            QColor minorColor = bgCrossColor;
            minorColor.setAlpha(minorColor.alpha() * 0.5);

            // Red margin line (Séyès), kept readable on both backgrounds.
            QColor marginColor = mDarkBackground ? QColor(255, 110, 110)
                                                 : QColor(210, 40, 40);
            if (mZoomFactor < 1.0)
                marginColor.setAlpha(255 * mZoomFactor / 2);

            const auto lines = UBBackgroundGrid::generateLines(mGridType, rect);
            for (const auto& line : lines)
            {
                switch (line.weight)
                {
                case UBBackgroundGrid::Weight::Major:  painter->setPen(bgCrossColor); break;
                case UBBackgroundGrid::Weight::Minor:  painter->setPen(minorColor);   break;
                case UBBackgroundGrid::Weight::Margin: painter->setPen(marginColor);  break;
                }

                if (line.orientation == UBBackgroundGrid::Orientation::Horizontal)
                    painter->drawLine(QPointF(rect.x(), line.pos),
                                      QPointF(rect.x() + rect.width(), line.pos));
                else
                    painter->drawLine(QPointF(line.pos, rect.y()),
                                      QPointF(line.pos, rect.y() + rect.height()));
            }
        }
    }
}
