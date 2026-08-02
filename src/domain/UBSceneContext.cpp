/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBSceneContext.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UBSettings.h"

qreal UBSceneContext::systemScaleFactor() const
{
    if (boardController)
        return boardController->systemScaleFactor();
    return testSystemScaleFactor;
}

qreal UBSceneContext::currentZoom() const
{
    if (boardController)
        return boardController->currentZoom();
    return testCurrentZoom;
}

qreal UBSceneContext::currentEraserWidth() const
{
    if (!drawingController)
        return eraserMediumWidth;
    return eraserMediumWidth;
}
