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



#include <QWidget>
#include <QMenu>
#include <QApplication>
#include <QPainter>
#include <QtGlobal>

#include "UBGraphicsWidgetItemDelegate.h"
#include "UBGraphicsScene.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsDelegateFrame.h"


UBGraphicsWidgetItemDelegate::UBGraphicsWidgetItemDelegate(UBGraphicsWidgetItem* pDelegated, int widgetType)
    : UBGraphicsItemDelegate(pDelegated, 0, true, false, false, true)
    , freezeAction(0)
    , setAsToolAction(0)
{
    mWidgetType = widgetType;
}


UBGraphicsWidgetItemDelegate::~UBGraphicsWidgetItemDelegate()
{
    // NOOP
}


void UBGraphicsWidgetItemDelegate::pin()
{
    UBApplication::boardController->moveGraphicsWidgetToControlView(delegated());
}


void UBGraphicsWidgetItemDelegate::updateMenuActionState()
{
    UBGraphicsItemDelegate::updateMenuActionState();

    if (freezeAction)
        freezeAction->setChecked(delegated()->isFrozen());
}

void UBGraphicsWidgetItemDelegate::decorateMenu(QMenu* menu)
{
    UBGraphicsItemDelegate::decorateMenu(menu);

    freezeAction = menu->addAction(tr("Frozen"), this, &UBGraphicsWidgetItemDelegate::freeze);

    QIcon freezeIcon;
    freezeIcon.addPixmap(QPixmap(":/images/frozen.svg"), QIcon::Normal, QIcon::On);
    freezeIcon.addPixmap(QPixmap(":/images/unfrozen.svg"), QIcon::Normal, QIcon::Off);
    freezeAction->setIcon(freezeIcon);

    freezeAction->setCheckable(true);

    if (delegated()->canBeTool())
    {
        setAsToolAction = mMenu->addAction(tr("Transform as Tool "), this, [this]() { pin(); });
        QIcon pinIcon;
        pinIcon.addPixmap(QPixmap(":/images/unpin.svg"), QIcon::Normal, QIcon::On);
        pinIcon.addPixmap(QPixmap(":/images/pin.svg"), QIcon::Normal, QIcon::Off);
        setAsToolAction->setIcon(pinIcon);
    }
}


void UBGraphicsWidgetItemDelegate::freeze(bool frozen)
{
    if(frozen)
    {
       delegated()->freeze();
    }
    else
    {
       delegated()->unFreeze();
    }
}


UBGraphicsWidgetItem* UBGraphicsWidgetItemDelegate::delegated()
{
    return static_cast<UBGraphicsWidgetItem*>(mDelegated);
}


void UBGraphicsWidgetItemDelegate::remove(bool canundo)
{
    delegated()->removeScript();
    UBGraphicsItemDelegate::remove(canundo);
}
