/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 * Copyright (C) 2026 David Guyomarch
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBGraphicsGroupContainerItemDelegate.h"
#include <QMenu>

#include <QWidget>
#include <QApplication>
#include <QPainter>

#include "UBGraphicsScene.h"

#include "core/UBApplication.h"

#include "board/UBBoardPaletteManager.h"


#include "gui/UBResources.h"
#include "gui/UBCreateLinkPalette.h"

#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsGroupContainerItem.h"

#include "board/UBBoardController.h"
#include "controllers/UBToolController.h"


#include "customWidgets/UBGraphicsItemAction.h"


UBGraphicsGroupContainerItemDelegate::UBGraphicsGroupContainerItemDelegate(QGraphicsItem *pDelegated, QObject *parent) :
    UBGraphicsItemDelegate(pDelegated, parent, true, false, false), mDestroyGroupButton(0)

{
    //Wrapper function. Use it to set correct data() to QGraphicsItem as well
    setFlippable(false);
    setRotatable(false);
    setCanDuplicate(true);
}

UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItemDelegate::delegated()
{
    return dynamic_cast<UBGraphicsGroupContainerItem*>(mDelegated);
}

// #455/ADR-0010: decorateMenu(), onAddActionClicked(), saveAction() and
// onRemoveActionClicked() were verbatim copies of the base delegate (the old
// //TODO claudio "duplicated code"). They are removed: the group now inherits
// the base decorateMenu(), which — because the constructor sets
// setCanTrigAnAction(true) — produces the same Locked / Visible / Link-action
// menu, and the base's link-action handlers (which also fix an audio source
// leak on delete). The buildButtons() override that merely forwarded to the
// base is likewise dropped. The group keeps only its genuinely specific mouse
// handling below (deselect current + play the action in Play mode).

bool UBGraphicsGroupContainerItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //TODO claudio
    // another chunk of duplicated code
    delegated()->deselectCurrentItem();
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBToolController::toolController()->stylusTool();
    if(currentTool == UBStylusTool::Play){
        if(mAction)
            mAction->play();
        return true;
    }
    return false;
}

bool UBGraphicsGroupContainerItemDelegate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    return false;
}

bool UBGraphicsGroupContainerItemDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    return false;
}

void UBGraphicsGroupContainerItemDelegate::setAction(UBGraphicsItemAction* action)
{
    UBGraphicsItemDelegate::setAction(action);
}
