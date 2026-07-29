/**
 * @file UBBoardSubControllers_stub.cpp
 * @brief Standalone implementations of static methods for testing.
 *
 * These are copies of the pure static methods from UBBoardZoomController
 * and UBBoardToolbarController, compiled without the full app dependency chain.
 */

#include <QPair>
#include <QString>
#include <QFont>
#include <QFontMetricsF>

namespace UBBoardZoomController
{
    QPair<qreal, qreal> computeZoomRatio(qreal requestedRatio, qreal currentViewScale, qreal systemScaleFactor, qreal maxZoom)
    {
        qreal currentZoom = requestedRatio * currentViewScale / systemScaleFactor;
        qreal usedRatio = requestedRatio;

        if (currentZoom > maxZoom)
        {
            currentZoom = maxZoom;
            usedRatio = currentZoom * systemScaleFactor / currentViewScale;
        }

        return QPair<qreal, qreal>(currentZoom, usedRatio);
    }
}

namespace UBBoardToolbarController
{
    QString truncate(const QString& text, int maxWidth, const QFont& font)
    {
        QFontMetricsF fontMetrics(font);
        return fontMetrics.elidedText(text, Qt::ElideRight, maxWidth);
    }
}
