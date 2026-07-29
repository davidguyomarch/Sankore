/**
 * @file UBBoardSubControllers_stub.cpp
 * @brief Thin wrappers around UBPure:: for test linking.
 *
 * No duplication — calls UBPureFunctions.h directly.
 */

#include "frameworks/UBPureFunctions.h"

namespace UBBoardZoomController
{
    QPair<qreal, qreal> computeZoomRatio(qreal requestedRatio, qreal currentViewScale, qreal systemScaleFactor, qreal maxZoom)
    {
        return UBPure::computeZoomRatio(requestedRatio, currentViewScale, systemScaleFactor, maxZoom);
    }
}

namespace UBBoardToolbarController
{
    QString truncate(const QString& text, int maxWidth, const QFont& font)
    {
        return UBPure::truncateText(text, maxWidth, font);
    }
}
