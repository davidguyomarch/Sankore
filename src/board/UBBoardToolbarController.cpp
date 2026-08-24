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
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), mBoardController->asQObject(), SLOT(colorPaletteChanged()));

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

    connectToolbar();
    initToolbarTexts();
}

void UBBoardToolbarController::connectToolbar()
{
    auto* bc = static_cast<UBBoardController*>(mBoardController->asQObject());

    connect(mMainWindow->actionAdd, &QAction::triggered, bc, &UBBoardController::addItem);
    connect(mMainWindow->actionNewPage, &QAction::triggered, bc, &UBBoardController::addScene);
    connect(mMainWindow->actionDuplicatePage, &QAction::triggered, bc, &UBBoardController::duplicateScene);

    connect(mMainWindow->actionClearPage, &QAction::triggered, bc, &UBBoardController::clearScene);
    connect(mMainWindow->actionEraseItems, &QAction::triggered, bc, &UBBoardController::clearSceneItems);
    connect(mMainWindow->actionEraseAnnotations, &QAction::triggered, bc, &UBBoardController::clearSceneAnnotation);
    connect(mMainWindow->actionEraseBackground, &QAction::triggered, bc, &UBBoardController::clearSceneBackground);

    connect(mMainWindow->actionCenterImageBackground, &QAction::triggered, bc, &UBBoardController::centerImageBackground);
    connect(mMainWindow->actionAdjustImageBackground, &QAction::triggered, bc, &UBBoardController::adjustImageBackground);
    connect(mMainWindow->actionMosaicImageBackground, &QAction::triggered, bc, &UBBoardController::mosaicImageBackground);
    connect(mMainWindow->actionFillImageBackground, &QAction::triggered, bc, &UBBoardController::fillImageBackground);
    connect(mMainWindow->actionExtendImageBackground, &QAction::triggered, bc, &UBBoardController::extendImageBackground);

    connect(mMainWindow->actionUndo, &QAction::triggered, UBApplication::undoStack, &QUndoStack::undo);
    connect(mMainWindow->actionRedo, &QAction::triggered, UBApplication::undoStack, &QUndoStack::redo);
    connect(mMainWindow->actionRedo, &QAction::triggered, bc, &UBBoardController::startScript);
    connect(mMainWindow->actionBack, &QAction::triggered, bc, &UBBoardController::previousScene);
    connect(mMainWindow->actionForward, &QAction::triggered, bc, &UBBoardController::nextScene);
    connect(mMainWindow->actionSleep, &QAction::triggered, bc, &UBBoardController::stopScript);
    connect(mMainWindow->actionSleep, &QAction::triggered, bc, &UBBoardController::blackout);
    connect(mMainWindow->actionVirtualKeyboard, &QAction::triggered, bc, &UBBoardController::showKeyboard);
    connect(mMainWindow->actionImportPage, &QAction::triggered, bc, &UBBoardController::importPage);
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
