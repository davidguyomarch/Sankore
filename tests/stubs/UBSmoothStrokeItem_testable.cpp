// Wrapper to compile UBSmoothStrokeItem.cpp in the test context.
// Provides a minimal UBGraphicsScene stub (the real one has too many deps).

// Pre-define the include guard to prevent the real UBGraphicsScene.h from loading
#define UBGRAPHICSSCENE_H_

#include <QGraphicsScene>
#include "core/UB.h"
#include "domain/UBItem.h"

// Minimal UBGraphicsScene stub — just enough for UBSmoothStrokeItem::scene()
class UBGraphicsScene : public QGraphicsScene
{
public:
    using QGraphicsScene::QGraphicsScene;
};

// Stubs for UBItem and UBGraphicsItem base class functions
// (avoids compiling UBItem.cpp which has massive dependencies)
UBItem::UBItem() : mUuid(QUuid()), mRenderingQuality(UBItem::RenderingQualityNormal) {}
UBItem::~UBItem() {}

UBGraphicsItem::~UBGraphicsItem()
{
    // No delegate in test context
}

void UBGraphicsItem::assignZValue(QGraphicsItem *item, qreal value)
{
    item->setZValue(value);
    item->setData(UBGraphicsItemData::ItemOwnZValue, value);
}

// Now include the real implementation
#include "domain/UBSmoothStrokeItem.cpp"
