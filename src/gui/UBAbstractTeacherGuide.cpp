/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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

#include "UBAbstractTeacherGuide.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

UBAbstractTeacherGuide::UBAbstractTeacherGuide(QWidget *parent):
    QStackedWidget(parent),
    mKeyboardActionFired(false)
{
    connect(UBApplication::boardController->controlView(), &UBBoardView::clickOnBoard, this, &UBAbstractTeacherGuide::showPresentationMode);
    connectToStylusPalette();
}

void UBAbstractTeacherGuide::connectToStylusPalette()
{
    connect(UBApplication::mainWindow->actionPen, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionEraser, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionMarker, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionPointer, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionPlay, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionZoomIn, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionZoomOut, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionCapture, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionHand, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionLine, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionText, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionSelector, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredAction);
    connect(UBApplication::mainWindow->actionVirtualKeyboard, &QAction::triggered, this, &UBAbstractTeacherGuide::onTriggeredKeyboardAction);
}

void UBAbstractTeacherGuide::onTriggeredAction(bool checked)
{
    Q_UNUSED(checked);
    if(!mKeyboardActionFired)
        showPresentationMode();
    mKeyboardActionFired=false;
}

void UBAbstractTeacherGuide::onTriggeredKeyboardAction(bool checked)
{
    Q_UNUSED(checked);
    mKeyboardActionFired = true;
}
