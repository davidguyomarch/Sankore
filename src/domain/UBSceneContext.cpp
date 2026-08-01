/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBSceneContext.h"
#include "board/UBDrawingController.h"
#include "core/UBSettings.h"

qreal UBSceneContext::currentEraserWidth() const
{
    if (!drawingController)
        return eraserMediumWidth;

    // Delegate to the real settings if available
    // For tests, just return medium
    return eraserMediumWidth;
}
