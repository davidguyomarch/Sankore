/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBSceneContext.h"

// Forward-declared in header — we only need these two methods:
//   UBBoardController::systemScaleFactor()
//   UBBoardController::currentZoom()
// Include the full header only in the app build (not tests)
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
#include "board/UBBoardController.h"
#include "controllers/UBToolController.h"
#include "core/UBSettings.h"
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

QColor UBSceneContext::penColorOnDarkBackground() const
{
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
    if (boardController)
        return UBSettings::settings()->penColor(true);
#endif
    return testPenColorOnDarkBackground;
}

QColor UBSceneContext::penColorOnLightBackground() const
{
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
    if (boardController)
        return UBSettings::settings()->penColor(false);
#endif
    return testPenColorOnLightBackground;
}

QColor UBSceneContext::markerColorOnDarkBackground() const
{
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
    if (boardController)
        return UBSettings::settings()->markerColor(true);
#endif
    return testMarkerColorOnDarkBackground;
}

QColor UBSceneContext::markerColorOnLightBackground() const
{
#if defined(QT_SVG_LIB) && !defined(QT_TESTLIB_LIB)
    if (boardController)
        return UBSettings::settings()->markerColor(false);
#endif
    return testMarkerColorOnLightBackground;
}

qreal UBSceneContext::currentEraserWidth() const
{
    return eraserMediumWidth;
}
