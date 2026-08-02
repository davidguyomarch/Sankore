/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBSceneContext.h"

// Forward-declared in header — we only need these two methods:
//   UBBoardController::systemScaleFactor()
//   UBBoardController::currentZoom()
// Include the full header only in the app build (not tests)
#ifdef QT_SVG_LIB
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#endif

qreal UBSceneContext::systemScaleFactor() const
{
#ifdef QT_SVG_LIB
    if (boardController)
        return boardController->systemScaleFactor();
#endif
    return testSystemScaleFactor;
}

qreal UBSceneContext::currentZoom() const
{
#ifdef QT_SVG_LIB
    if (boardController)
        return boardController->currentZoom();
#endif
    return testCurrentZoom;
}

qreal UBSceneContext::currentEraserWidth() const
{
    return eraserMediumWidth;
}
