/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBSceneContext.h"

// Forward-declared in header — we only need these two methods:
//   UBBoardController::systemScaleFactor()
//   UBBoardController::currentZoom()
// Include the full header only in the app build (not tests)
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#endif

qreal UBSceneContext::systemScaleFactor() const
{
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
    if (boardController)
        return boardController->systemScaleFactor();
#endif
    return testSystemScaleFactor;
}

qreal UBSceneContext::currentZoom() const
{
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
    if (boardController)
        return boardController->currentZoom();
#endif
    return testCurrentZoom;
}

qreal UBSceneContext::currentEraserWidth() const
{
    return eraserMediumWidth;
}
