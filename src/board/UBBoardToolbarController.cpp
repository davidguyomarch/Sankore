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

UBBoardToolbarController::UBBoardToolbarController(UBBoardController* boardController,
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

    mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, colorChoice);

    connect(mSettings->appToolBarDisplayText, SIGNAL(changed(QVariant)), colorChoice, SLOT(displayText(QVariant)));
    connect(colorChoice, SIGNAL(activated(int)), mBoardController, SLOT(setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), mBoardController, SLOT(colorPaletteChanged()));

    colorChoice->displayText(QVariant(mSettings->appToolBarDisplayText->get().toBool()));
    colorChoice->colorPaletteChanged();

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

    mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, lineWidthChoice);

    // Setup eraser width choice widget
    QList<QAction *> eraserWidthActions;
    eraserWidthActions.append(mMainWindow->actionEraserSmall);
    eraserWidthActions.append(mMainWindow->actionEraserMedium);
    eraserWidthActions.append(mMainWindow->actionEraserLarge);

    UBToolbarButtonGroup *eraserWidthChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, eraserWidthActions);

    mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, eraserWidthChoice);

    connect(mSettings->appToolBarDisplayText, SIGNAL(changed(QVariant)), eraserWidthChoice, SLOT(displayText(QVariant)));
    connect(eraserWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setEraserWidthIndex(int)));

    eraserWidthChoice->displayText(QVariant(mSettings->appToolBarDisplayText->get().toBool()));
    eraserWidthChoice->setCurrentIndex(mSettings->eraserWidthIndex());

    mMainWindow->boardToolBar->insertSeparator(mMainWindow->actionBackgrounds);

    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->boardToolBar, mMainWindow->actionBoard);

    UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

    mMainWindow->actionBoard->setVisible(false);

    mMainWindow->webToolBar->hide();
    mMainWindow->documentToolBar->hide();

    connectToolbar();
    initToolbarTexts();
}

void UBBoardToolbarController::connectToolbar()
{
    connect(mMainWindow->actionAdd, SIGNAL(triggered()), mBoardController, SLOT(addItem()));
    connect(mMainWindow->actionNewPage, SIGNAL(triggered()), mBoardController, SLOT(addScene()));
    connect(mMainWindow->actionDuplicatePage, SIGNAL(triggered()), mBoardController, SLOT(duplicateScene()));

    connect(mMainWindow->actionClearPage, SIGNAL(triggered()), mBoardController, SLOT(clearScene()));
    connect(mMainWindow->actionEraseItems, SIGNAL(triggered()), mBoardController, SLOT(clearSceneItems()));
    connect(mMainWindow->actionEraseAnnotations, SIGNAL(triggered()), mBoardController, SLOT(clearSceneAnnotation()));
    connect(mMainWindow->actionEraseBackground, SIGNAL(triggered()), mBoardController, SLOT(clearSceneBackground()));

    connect(mMainWindow->actionCenterImageBackground, SIGNAL(triggered()), mBoardController, SLOT(centerImageBackground()));
    connect(mMainWindow->actionAdjustImageBackground, SIGNAL(triggered()), mBoardController, SLOT(adjustImageBackground()));
    connect(mMainWindow->actionMosaicImageBackground, SIGNAL(triggered()), mBoardController, SLOT(mosaicImageBackground()));
    connect(mMainWindow->actionFillImageBackground, SIGNAL(triggered()), mBoardController, SLOT(fillImageBackground()));
    connect(mMainWindow->actionExtendImageBackground, SIGNAL(triggered()), mBoardController, SLOT(extendImageBackground()));

    connect(mMainWindow->actionUndo, SIGNAL(triggered()), UBApplication::undoStack, SLOT(undo()));
    connect(mMainWindow->actionRedo, SIGNAL(triggered()), UBApplication::undoStack, SLOT(redo()));
    connect(mMainWindow->actionRedo, SIGNAL(triggered()), mBoardController, SLOT(startScript()));
    connect(mMainWindow->actionBack, SIGNAL(triggered()), mBoardController, SLOT(previousScene()));
    connect(mMainWindow->actionForward, SIGNAL(triggered()), mBoardController, SLOT(nextScene()));
    connect(mMainWindow->actionSleep, SIGNAL(triggered()), mBoardController, SLOT(stopScript()));
    connect(mMainWindow->actionSleep, SIGNAL(triggered()), mBoardController, SLOT(blackout()));
    connect(mMainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), mBoardController, SLOT(showKeyboard(bool)));
    connect(mMainWindow->actionImportPage, SIGNAL(triggered()), mBoardController, SLOT(importPage()));
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
        QString shortText = truncate(nominalText, 48);
        QPair<QString, QString> texts(nominalText, shortText);

        mActionTexts.insert(action, texts);
    }
}

void UBBoardToolbarController::setToolbarTexts()
{
    bool highResolution = mMainWindow->width() > 1024;
    QSize iconSize;

    if (highResolution)
        iconSize = QSize(48, 32);
    else
        iconSize = QSize(32, 32);

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

QString UBBoardToolbarController::truncate(QString text, int maxWidth)
{
    QFontMetricsF fontMetrics(mMainWindow->font());
    return fontMetrics.elidedText(text, Qt::ElideRight, maxWidth);
}
