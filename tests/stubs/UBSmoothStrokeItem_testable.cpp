/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Wrapper to compile UBSmoothStrokeItem.cpp in the test context.
// Provides a minimal UBGraphicsScene stub (the real one has too many deps).

// Pre-define the include guard to prevent the real UBGraphicsScene.h from loading
#define UBGRAPHICSSCENE_H_

// Prevent the real UBGraphicsItemDelegate.h (huge dependency tree) from loading;
// UBSmoothStrokeItem.cpp only needs a minimal delegate for #243 (own a delegate
// so Delegate() is non-null). We provide a lightweight stand-in below.
#define UBGRAPHICSITEMDELEGATE_H_

#include <QGraphicsScene>
#include "core/UB.h"

// Minimal UBGraphicsItemDelegate stub — just the methods the UBSmoothStrokeItem
// constructor calls (#243). No frame, no scene wiring: enough to verify the item
// owns a non-null delegate after construction. Must be a complete type BEFORE
// UBItem.h is included, since UBItem.h holds a UBGraphicsItemDelegate* member.
class UBGraphicsItemDelegate
{
public:
    UBGraphicsItemDelegate(QGraphicsItem*, QObject* = nullptr, bool = true,
                           bool = false, bool = true, bool = false) {}
    void init() {}
    void setFlippable(bool) {}
    void setRotatable(bool) {}
    void setCanTrigAnAction(bool) {}
};

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

void UBGraphicsItem::setDelegate(UBGraphicsItemDelegate* delegate)
{
    mDelegate = delegate;
}

UBGraphicsItem::~UBGraphicsItem()
{
    delete mDelegate;
}

void UBGraphicsItem::assignZValue(QGraphicsItem *item, qreal value)
{
    item->setZValue(value);
    item->setData(UBGraphicsItemData::ItemOwnZValue, value);
}

// Now include the real implementation
#include "domain/UBSmoothStrokeItem.cpp"
