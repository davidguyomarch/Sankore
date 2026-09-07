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



#include "UBDocumentToolsPalette.h"

#include <QWidget>
#include <QApplication>
#include <QPainter>


UBDocumentToolsPalette::UBDocumentToolsPalette(QWidget *parent)
    : UBActionPalette(Qt::TopRightCorner, parent)
{
    // #284: the virtual keyboard was removed from the Documents view. It relies
    // on a legacy X11 key-injection backend that is unreliable under Qt6, and a
    // virtual keyboard has no clear use in a document-management view. It is kept
    // on the board and desktop modes (UBBoardController / UBDesktopPalette).
    // This palette now holds no actions, so isEmpty() is true and the
    // "Document Tools" button is hidden by UBDocumentController::setupPalettes().
    QList<QAction*> actions;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    adjustSizeAndPosition();
}


UBDocumentToolsPalette::~UBDocumentToolsPalette()
{
    // NOOP
}

