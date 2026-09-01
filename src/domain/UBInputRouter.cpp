/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBInputRouter.h"

#include <QLineF>
#include <QtMath>

#include "UBGraphicsScene.h"
#include "UBDrawingHandler.h"
#include "UBToolOverlay.h"
#include "UBSceneContext.h"
#include "UBSmoothStrokeItem.h"
#include "UBGraphicsStroke.h"
#include "UBGraphicsPolygonItem.h"
#include "UBGraphicsStrokesGroup.h"
#include "UBGraphicsItemUndoCommand.h"

#include "core/UB.h"
#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "controllers/UBToolController.h"
#include "tools/UBAbstractDrawRuler.h"
#include "frameworks/UBCoreGraphicsScene.h"

UBInputRouter::UBInputRouter(UBGraphicsScene* scene)
    : mScene(scene)
{
}

bool UBInputRouter::inputDevicePress(const QPointF& scenePos, qreal pressure)
{
    bool accepted = false;

    const UBSceneContext& ctx = mScene->sceneContext();

    // Guard: drawingController may be null on scenes without a live board context.  (#135)
    if (!ctx.drawingController)
        return false;

    if (mInputDeviceIsPressed)
    {
        qWarning() << "scene received input device pressed, without input device release, muting event as input device move";
        accepted = inputDeviceMove(scenePos, pressure);
    }
    else
    {
        mInputDeviceIsPressed = true;

        UBDrawingHandler* drawing = mScene->drawingHandler();
        UBStylusTool::Enum currentTool = (UBStylusTool::Enum)ctx.drawingController->stylusTool();

        if (ctx.drawingController->isDrawingTool())
        {
            // -----------------------------------------------------------------
            // We fall here if we are using the Pen, the Marker or the Line tool
            // -----------------------------------------------------------------
            qreal width = 0;

            if (currentTool != UBStylusTool::Line){
                width = ctx.drawingController->currentToolWidth() * pressure;
            }else{
                width = ctx.drawingController->currentToolWidth();
            }

            width /= ctx.systemScaleFactor();
            width /= ctx.currentZoom();

            mScene->addedItemsRef().clear();
            mScene->removedItemsRef().clear();

            // -----------------------------------------------------------------
            // New smooth stroke pipeline (single QPainterPath item)
            // Used for Pen and Marker freehand. Line tool uses legacy pipeline.
            // -----------------------------------------------------------------
            bool useSmoothStroke = mScene->settings()->appSmoothStrokeItem->get().toBool()
                                   && !ctx.drawingController->mActiveRuler;

            if (useSmoothStroke)
            {
                mCurrentSmoothStroke = new UBSmoothStrokeItem();
                mCurrentSmoothStroke->setStrokeWidth(width);

                QColor colorOnDarkBG, colorOnLightBG;
                if (currentTool == UBStylusTool::Marker)
                {
                    colorOnDarkBG = ctx.markerColorOnDarkBackground();
                    colorOnLightBG = ctx.markerColorOnLightBackground();
                }
                else
                {
                    colorOnDarkBG = ctx.penColorOnDarkBackground();
                    colorOnLightBG = ctx.penColorOnLightBackground();
                }

                mCurrentSmoothStroke->setColorOnDarkBackground(colorOnDarkBG);
                mCurrentSmoothStroke->setColorOnLightBackground(colorOnLightBG);

                if (mScene->isDarkBackground())
                    mCurrentSmoothStroke->setStrokeColor(colorOnDarkBG);
                else
                    mCurrentSmoothStroke->setStrokeColor(colorOnLightBG);

                mCurrentSmoothStroke->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));

                mScene->addItem(mCurrentSmoothStroke);
                mCurrentSmoothStroke->addPoint(scenePos, pressure);

                drawing->setCurrentStroke(nullptr);
            }
            else
            {
                // Legacy polygon-based pipeline (Line tool, ruler-guided, or setting disabled)
                if (drawing->currentStroke() && drawing->currentStroke()->polygons().empty()){
                    delete drawing->currentStroke();
                    drawing->setCurrentStroke(nullptr);
                }

                drawing->setCurrentStroke(new UBGraphicsStroke());
                mCurrentSmoothStroke = nullptr;

                if (ctx.drawingController->mActiveRuler)
                {
                    ctx.drawingController->mActiveRuler->StartLine(scenePos, width);
                }
                else
                {
                    mScene->moveTo(scenePos);
                    mScene->drawLineTo(scenePos, width, ctx.drawingController->stylusTool() == UBStylusTool::Line);
                }
            }
            accepted = true;
        }
        else if (currentTool == UBStylusTool::Eraser)
        {
            mScene->addedItemsRef().clear();
            mScene->removedItemsRef().clear();
            mScene->moveTo(scenePos);

            qreal eraserWidth = ctx.currentEraserWidth();
            eraserWidth /= ctx.systemScaleFactor();
            eraserWidth /= ctx.currentZoom();

            mScene->eraseLineTo(scenePos, eraserWidth);
            mScene->toolOverlay()->drawEraser(scenePos, ctx, true);

            accepted = true;
        }
        else if (currentTool == UBStylusTool::Pointer)
        {
            mScene->toolOverlay()->drawPointer(scenePos, ctx, true);
            accepted = true;
        }
    }

    UBDrawingHandler* drawing = mScene->drawingHandler();
    if (drawing->currentStroke() && drawing->currentStroke()->polygons().empty()){
        delete drawing->currentStroke();
        drawing->setCurrentStroke(nullptr);
    }

    return accepted;
}

bool UBInputRouter::inputDeviceMove(const QPointF& scenePos, qreal pressure)
{
    bool accepted = false;

    const UBSceneContext& ctx = mScene->sceneContext();

    // Guard: drawingController may be null on scenes without a live board context.  (#135)
    if (!ctx.drawingController)
        return false;

    UBDrawingHandler* drawing = mScene->drawingHandler();
    UBToolController* dc = ctx.drawingController;
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)dc->stylusTool();

    QPointF position = QPointF(scenePos);

    if (currentTool == UBStylusTool::Eraser)
    {
        mScene->toolOverlay()->drawEraser(position, ctx);
        accepted = true;
    }

    if (mInputDeviceIsPressed)
    {
        if (dc->isDrawingTool())
        {
            // --- New smooth stroke pipeline: just accumulate points ---
            if (mCurrentSmoothStroke)
            {
                if (currentTool == UBStylusTool::Line)
                {
                    // Line tool: rubber-band from start to snapped endpoint
                    QPointF startPos = mCurrentSmoothStroke->rawPoints().first();
                    QPointF sceneStart = mCurrentSmoothStroke->mapToScene(startPos);

                    QLineF radius(sceneStart, position);
                    qreal angle = radius.angle();
                    angle = qRound(angle / 45) * 45;
                    qreal radiusLength = radius.length();
                    QPointF snappedPos(
                        sceneStart.x() + radiusLength * cos((angle * PI) / 180),
                        sceneStart.y() - radiusLength * sin((angle * PI) / 180));
                    QLineF chord(position, snappedPos);
                    if (chord.length() < qMin((int)16, (int)(radiusLength / 20)))
                        position = snappedPos;

                    mCurrentSmoothStroke->setLastPoint(position, pressure);
                }
                else
                {
                    mCurrentSmoothStroke->addPoint(position, pressure);
                }
            }
            else
            {
            qreal width = 0;

            if (currentTool != UBStylusTool::Line){
                width = dc->currentToolWidth() * pressure;
            }else{
                width = dc->currentToolWidth();
            }

            width /= ctx.systemScaleFactor();
            width /= ctx.currentZoom();

            if (currentTool == UBStylusTool::Line || dc->mActiveRuler)
            {
                if (dc->stylusTool() != UBStylusTool::Marker)
                if(nullptr != drawing->lastPolygon() && nullptr != drawing->currentStroke() && mScene->addedItemsRef().size() > 0){
                    mScene->removeItemFromDeletion(drawing->lastPolygon());
                    mScene->addedItemsRef().remove(drawing->lastPolygon());
                    drawing->currentStroke()->remove(drawing->lastPolygon());
                    if (drawing->currentStroke()->polygons().empty()){
                        delete drawing->currentStroke();
                        drawing->setCurrentStroke(nullptr);
                    }
                    mScene->removeItem(drawing->lastPolygon());
                    drawing->previousPolygonItems().removeAll(drawing->lastPolygon());
                }

                // ------------------------------------------------------------------------
                // Here we wanna make sure that the Line will 'grip' at i*45, i*90 degrees
                // ------------------------------------------------------------------------

                QLineF radius(drawing->previousPoint(), position);
                qreal angle = radius.angle();
                angle = qRound(angle / 45) * 45;
                qreal radiusLength = radius.length();
                QPointF newPosition(
                    drawing->previousPoint().x() + radiusLength * cos((angle * PI) / 180),
                    drawing->previousPoint().y() - radiusLength * sin((angle * PI) / 180));
                QLineF chord(position, newPosition);
                if (chord.length() < qMin((int)16, (int)(radiusLength / 20)))
                    position = newPosition;
            }

            if(dc->mActiveRuler){
                dc->mActiveRuler->DrawLine(position, width);
            }else{
                bool bLineStyle = dc->stylusTool() == UBStylusTool::Line;
                if (!bLineStyle && currentTool != UBStylusTool::Line
                    && currentTool != UBStylusTool::Marker)
                {
                    // Freehand Pen stroke: route through smoothing buffer if enabled
                    if (mScene->settings()->appStrokeSmoothing->get().toBool())
                    {
                        drawing->smoothBuffer().append(position);
                        drawing->smoothWidths().append(width);

                        if (drawing->smoothBuffer().size() >= 4)
                        {
                            int n = drawing->smoothBuffer().size();
                            drawing->drawSmoothedSegment(mScene, ctx,
                                drawing->smoothBuffer()[n-4], drawing->smoothBuffer()[n-3],
                                drawing->smoothBuffer()[n-2], drawing->smoothBuffer()[n-1],
                                drawing->smoothWidths()[n-3], drawing->smoothWidths()[n-2],
                                false);
                        }
                        else if (drawing->smoothBuffer().size() == 2)
                        {
                            mScene->drawLineTo(position, width, false);
                        }
                    }
                    else
                    {
                        mScene->drawLineTo(position, width, false);
                    }
                }
                else
                {
                    mScene->drawLineTo(position, width, bLineStyle);
                }
            }
            } // end legacy else branch
        }
        else if (currentTool == UBStylusTool::Eraser)
        {
            qreal eraserWidth = ctx.currentEraserWidth();
            eraserWidth /= ctx.systemScaleFactor();
            eraserWidth /= ctx.currentZoom();

            mScene->eraseLineTo(position, eraserWidth);
        }
        else if (currentTool == UBStylusTool::Pointer)
        {
            mScene->toolOverlay()->drawPointer(position, ctx);
        }

        accepted = true;
    }

    return accepted;
}

bool UBInputRouter::inputDeviceRelease()
{
    const UBSceneContext& ctx = mScene->sceneContext();

    // Guard: drawingController may be null on scenes without a live board context.  (#135)
    if (!ctx.drawingController)
        return false;

    bool accepted = false;

    UBDrawingHandler* drawing = mScene->drawingHandler();

    if (mScene->toolOverlay()->pointerItem())
    {
        mScene->toolOverlay()->hidePointer();
        accepted = true;
    }

    UBToolController* dc = ctx.drawingController;

    if (dc->isDrawingTool() || drawing->drawWithCompass())
    {
        // --- New smooth stroke pipeline: finalize and wire undo ---
        if (mCurrentSmoothStroke)
        {
            mCurrentSmoothStroke->finalize();

            // The item was already added to the scene in inputDevicePress.
            // Wire it into the undo system.
            mScene->addedItemsRef().clear();
            mScene->addedItemsRef() << mCurrentSmoothStroke;

            mCurrentSmoothStroke = nullptr;
        }
        else
        {
        // --- Legacy polygon pipeline ---
        // Flush any remaining smoothing buffer points
        drawing->flushSmoothBuffer(mScene, ctx, dc->stylusTool() == UBStylusTool::Line);

        if(drawing->arcPolygonItem()){

                UBGraphicsStrokesGroup* pStrokes = new UBGraphicsStrokesGroup();

                // Add the arc
                mScene->addedItemsRef().remove(drawing->arcPolygonItem());
                mScene->removeItem(drawing->arcPolygonItem());
                mScene->removeItemFromDeletion(drawing->arcPolygonItem());
                drawing->arcPolygonItem()->setStrokesGroup(pStrokes);
                pStrokes->addToGroup(drawing->arcPolygonItem());

                // Add the center cross
                for (QGraphicsItem* item : mScene->addedItemsRef()){
                    mScene->addedItemsRef().remove(item);
                    mScene->removeItem(item);
                    mScene->removeItemFromDeletion(item);

                    UBGraphicsPolygonItem *crossLine = qgraphicsitem_cast<UBGraphicsPolygonItem *>(item);
                    if (crossLine)
                        crossLine->setStrokesGroup(pStrokes);

                    pStrokes->addToGroup(item);
                }

                mScene->addedItemsRef().clear();
                mScene->addedItemsRef() << pStrokes;
                mScene->addItem(pStrokes);
                drawing->setDrawWithCompass(false);

        }
        else if (drawing->currentStroke()){
                UBGraphicsStrokesGroup* pStrokes = new UBGraphicsStrokesGroup();

                // Remove the strokes that were just drawn here and replace them by a stroke item
                for (UBGraphicsPolygonItem* poly : drawing->currentStroke()->polygons()){
                    drawing->previousPolygonItems().removeAll(poly);
                    mScene->removeItem(poly);
                    mScene->removeItemFromDeletion(poly);
                    poly->setStrokesGroup(pStrokes);
                    pStrokes->addToGroup(poly);
                }

                // TODO LATER : Generate well pressure-interpolated polygons and create the line group with them

                mScene->addedItemsRef().clear();
                mScene->addedItemsRef() << pStrokes;
                mScene->addItem(pStrokes);

            if (drawing->currentStroke()->polygons().empty()){
                delete drawing->currentStroke();
                drawing->setCurrentStroke(0);
            }
        }
        } // end legacy else
    }

    if (mScene->removedItemsRef().size() > 0 || mScene->addedItemsRef().size() > 0)
    {
        if (mScene->isURStackIsEnabled()) { //should be deleted after scene own undo stack implemented
            UBGraphicsItemUndoCommand* udcmd = new UBGraphicsItemUndoCommand(mScene, mScene->removedItemsRef(), mScene->addedItemsRef()); //deleted by the undoStack

            if(UBApplication::undoStack)
                UBApplication::undoStack->push(udcmd);
        }

        mScene->removedItemsRef().clear();
        mScene->addedItemsRef().clear();
        accepted = true;
    }

    mInputDeviceIsPressed = false;

    mScene->setDocumentUpdated();

    if (drawing->currentStroke() && drawing->currentStroke()->polygons().empty()){
        delete drawing->currentStroke();
    }

    drawing->setCurrentStroke(nullptr);

    return accepted;
}
