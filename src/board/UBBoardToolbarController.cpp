/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#include "UBBoardToolbarController.h"
#include "UBBoardController.h"
#include "UBDrawingController.h"

#include <QAction>
#include <QFontMetricsF>
#include <QToolBar>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"
#include "domain/UBGraphicsScene.h"
#include "frameworks/UBPureFunctions.h"

UBBoardToolbarController::UBBoardToolbarController(IUBBoardContext* boardController,
                                                   UBMainWindow* mainWindow,
                                                   UBSettings* settings,
                                                   QObject* parent)
    : QObject(parent)
    , mBoardController(boardController)
    , mMainWindow(mainWindow)
    , mSettings(settings)
{
}

void UBBoardToolbarController::setupToolbar()
{
    // Setup color choice widget
    QList<QAction *> colorActions;
    colorActions.append(mMainWindow->actionColor0);
    colorActions.append(mMainWindow->actionColor1);
    colorActions.append(mMainWindow->actionColor2);
    colorActions.append(mMainWindow->actionColor3);

    UBToolbarButtonGroup *colorChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, colorActions);

    QAction* colorChoiceAction = mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, colorChoice);

    connect(mSettings->appToolBarDisplayText, SIGNAL(changed(QVariant)), colorChoice, SLOT(displayText(QVariant)));
    connect(colorChoice, SIGNAL(activated(int)), mBoardController->asQObject(), SLOT(setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));
    // Route colorPaletteChanged to boardController::penColorChanged for tools (e.g. compass)
    connect(UBDrawingController::drawingController(), &UBDrawingController::colorPaletteChanged,
            UBApplication::boardController, &UBBoardController::penColorChanged);

    colorChoice->displayText(QVariant(mSettings->appToolBarDisplayText->get().toBool()));
    colorChoice->colorPaletteChanged();

    // Hidden: replaced by QML DrawingProperties panel (Issue #110 Step 3)
    colorChoiceAction->setVisible(false);

    // Setup line width choice widget
    QList<QAction *> lineWidthActions;
    lineWidthActions.append(mMainWindow->actionLineSmall);
    lineWidthActions.append(mMainWindow->actionLineMedium);
    lineWidthActions.append(mMainWindow->actionLineLarge);

    UBToolbarButtonGroup *lineWidthChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, lineWidthActions);

    connect(mSettings->appToolBarDisplayText, SIGNAL(changed(QVariant)), lineWidthChoice, SLOT(displayText(QVariant)));

    connect(lineWidthChoice, SIGNAL(activated(int))
            , UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));

    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int))
            , lineWidthChoice, SLOT(setCurrentIndex(int)));

    lineWidthChoice->displayText(QVariant(mSettings->appToolBarDisplayText->get().toBool()));

    QAction* lineWidthChoiceAction = mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, lineWidthChoice);

    // Hidden: replaced by QML DrawingProperties panel (Issue #110 Step 3)
    lineWidthChoiceAction->setVisible(false);

    // Setup eraser width choice widget
    QList<QAction *> eraserWidthActions;
    eraserWidthActions.append(mMainWindow->actionEraserSmall);
    eraserWidthActions.append(mMainWindow->actionEraserMedium);
    eraserWidthActions.append(mMainWindow->actionEraserLarge);

    UBToolbarButtonGroup *eraserWidthChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, eraserWidthActions);

    QAction* eraserWidthChoiceAction = mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, eraserWidthChoice);

    connect(mSettings->appToolBarDisplayText, SIGNAL(changed(QVariant)), eraserWidthChoice, SLOT(displayText(QVariant)));
    connect(eraserWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setEraserWidthIndex(int)));

    eraserWidthChoice->displayText(QVariant(mSettings->appToolBarDisplayText->get().toBool()));
    eraserWidthChoice->setCurrentIndex(mSettings->eraserWidthIndex());

    // Hidden: replaced by QML DrawingProperties panel (Issue #110 Step 3)
    eraserWidthChoiceAction->setVisible(false);

    mMainWindow->boardToolBar->insertSeparator(mMainWindow->actionBackgrounds);

    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->boardToolBar, mMainWindow->actionBoard);

    UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

    mMainWindow->actionBoard->setVisible(false);

    mMainWindow->webToolBar->hide();
    mMainWindow->documentToolBar->hide();
    mMainWindow->boardToolBar->hide(); // Hidden: replaced by QML TopBar

    connectToolbar();
    initToolbarTexts();
}

void UBBoardToolbarController::connectToolbar()
{
    connect(mMainWindow->actionAdd, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(addItem()));
    connect(mMainWindow->actionNewPage, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(addScene()));
    connect(mMainWindow->actionDuplicatePage, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(duplicateScene()));

    connect(mMainWindow->actionClearPage, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(clearScene()));
    connect(mMainWindow->actionEraseItems, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(clearSceneItems()));
    connect(mMainWindow->actionEraseAnnotations, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(clearSceneAnnotation()));
    connect(mMainWindow->actionEraseBackground, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(clearSceneBackground()));

    connect(mMainWindow->actionCenterImageBackground, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(centerImageBackground()));
    connect(mMainWindow->actionAdjustImageBackground, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(adjustImageBackground()));
    connect(mMainWindow->actionMosaicImageBackground, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(mosaicImageBackground()));
    connect(mMainWindow->actionFillImageBackground, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(fillImageBackground()));
    connect(mMainWindow->actionExtendImageBackground, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(extendImageBackground()));

    connect(mMainWindow->actionUndo, SIGNAL(triggered()), UBApplication::undoStack, SLOT(undo()));
    connect(mMainWindow->actionRedo, SIGNAL(triggered()), UBApplication::undoStack, SLOT(redo()));
    connect(mMainWindow->actionRedo, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(startScript()));
    connect(mMainWindow->actionBack, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(previousScene()));
    connect(mMainWindow->actionForward, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(nextScene()));
    connect(mMainWindow->actionSleep, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(stopScript()));
    connect(mMainWindow->actionSleep, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(blackout()));
    connect(mMainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), mBoardController->asQObject(), SLOT(showKeyboard(bool)));
    connect(mMainWindow->actionImportPage, SIGNAL(triggered()), mBoardController->asQObject(), SLOT(importPage()));
}

void UBBoardToolbarController::initToolbarTexts()
{
    QList<QAction*> allToolbarActions;

    allToolbarActions << mMainWindow->boardToolBar->actions();
    allToolbarActions << mMainWindow->webToolBar->actions();
    allToolbarActions << mMainWindow->documentToolBar->actions();

    for (QAction* action : allToolbarActions)
    {
        QString nominalText = action->text();
        QString shortText = truncate(nominalText, 48, mMainWindow->font());
        QPair<QString, QString> texts(nominalText, shortText);

        mActionTexts.insert(action, texts);
    }
}

void UBBoardToolbarController::setToolbarTexts()
{
    bool highResolution = mMainWindow->width() > 1024;
    QSize iconSize;

    if (highResolution)
        iconSize = QSize(44, 44);
    else
        iconSize = QSize(36, 36);

    mMainWindow->boardToolBar->setIconSize(iconSize);
    mMainWindow->webToolBar->setIconSize(iconSize);
    mMainWindow->documentToolBar->setIconSize(iconSize);

    for (QAction* action : mActionTexts.keys())
    {
        QPair<QString, QString> texts = mActionTexts.value(action);

        if (highResolution)
            action->setText(texts.first);
        else
            action->setText(texts.second);

        action->setToolTip(texts.first);
    }
}

void UBBoardToolbarController::updateActionStates()
{
    mMainWindow->actionBack->setEnabled(mBoardController->selectedDocument() && (mBoardController->activeSceneIndex() > 0));
    mMainWindow->actionForward->setEnabled(mBoardController->selectedDocument() && (mBoardController->activeSceneIndex() < mBoardController->selectedDocument()->pageCount() - 1));
    mMainWindow->actionErase->setEnabled(mBoardController->activeScene() && !mBoardController->activeScene()->isEmpty());
}

void UBBoardToolbarController::updatePageSizeState()
{
    if (mBoardController->activeScene()->nominalSize() == mSettings->documentSizes.value(DocumentSizeRatio::Ratio16_9))
    {
        mMainWindow->actionWidePageSize->setChecked(true);
    }
    else if (mBoardController->activeScene()->nominalSize() == mSettings->documentSizes.value(DocumentSizeRatio::Ratio4_3))
    {
        mMainWindow->actionRegularPageSize->setChecked(true);
    }
    else if (mBoardController->activeScene()->nominalSize() == mSettings->documentSizes.value(DocumentSizeRatio::Ratio16_10))
    {
        mMainWindow->actionWidePageSize_16_10->setChecked(true);
    }
    else
    {
        mMainWindow->actionCustomPageSize->setChecked(true);
    }
}

QString UBBoardToolbarController::truncate(const QString& text, int maxWidth, const QFont& font)
{
    return UBPure::truncateText(text, maxWidth, font);
}
