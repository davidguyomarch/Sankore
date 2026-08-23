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



#include "UBBoardPaletteManager.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDisplayManager.h"

#include "gui/UBMainWindow.h"
#include "gui/UBStylusPalette.h"
#include "gui/UBKeyboardPalette.h"
#include "gui/UBToolWidget.h"
#include "gui/UBZoomPalette.h"
#include "gui/UBWebToolsPalette.h"
#include "gui/UBActionPalette.h"
#include "gui/UBFavoriteToolPalette.h"
#include "gui/UBDockTeacherGuideWidget.h"
#include "gui/UBStartupHintsPalette.h"
#include "gui/UBCreateLinkPalette.h"

#include "web/UBWebPage.h"
#include "web/UBWebController.h"
#ifdef SANKORE_WEBENGINE
#include "web/browser/WBBrowserWindow.h"
#include "web/browser/WBTabWidget.h"
#include "web/browser/WBWebView.h"
#endif

#include "desktop/UBDesktopAnnotationController.h"

#include <QQuickWidget>
#include <QQmlContext>
#include "qml/UBThemeManager.h"
#include "qml/UBStylusController.h"
#include "qml/UBDrawingPropertiesController.h"
#include "qml/UBShapesController.h"


#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPixmapItem.h"

#include "document/UBDocumentProxy.h"
#include "podcast/UBPodcastController.h"
#include "board/UBDrawingController.h"

#include "tools/UBToolsManager.h"

#include "UBBoardController.h"

#include "document/UBDocumentController.h"


UBBoardPaletteManager::UBBoardPaletteManager(QWidget* container, UBBoardController* pBoardController)
    : QObject(container)
    , mKeyboardPalette(0)
    , mWebToolsCurrentPalette(0)
    , mContainer(container)
    , mBoardControler(pBoardController)
    , mStylusPalette(0)
    , mStylusPaletteQml(nullptr)
    , mStylusController(nullptr)
    , mDrawingPropsQml(nullptr)
    , mDrawingPropsController(nullptr)
    , mShapesPaletteQml(nullptr)
    , mShapesController(nullptr)
    , mDrawingPalette(nullptr)
    , mZoomPalette(0)
    , mTipPalette(0)
    , mLinkPalette(0)
    , mLeftPalette(nullptr)
    , mRightPalette(nullptr)
    , mBackgroundsPalette(0)
    , mToolsPalette(0)
    , mAddItemPalette(0)
    , mErasePalette(nullptr)
    , mPagePalette(nullptr)
    , mImageBackgroundPalette(nullptr)
    , mEllipseActionPaletteButton(nullptr)
    , mPendingPageButtonPressed(false)
    , mPendingZoomButtonPressed(false)
    , mPendingPanButtonPressed(false)
    , mPendingEraseButtonPressed(false)
    , mpPageNavigWidget(nullptr)
    , mpCachePropWidget(nullptr)
    , mpDownloadWidget(nullptr)
    , mpTeacherGuideWidget(nullptr)
    , mDownloadInProgress(false)
{
    mSettings = UBSettings::settings();
    mTeacherResources = nullptr;
    setupPalettes();
    connectPalettes();
}


UBBoardPaletteManager::~UBBoardPaletteManager()
{

// mAddedItemPalette is delete automatically because of is parent
// that changes depending on the mode

// mMainWindow->centralWidget is the parent of mStylusPalette
// do not delete this here.
}

void UBBoardPaletteManager::initPalettesPosAtStartup()
{
    mStylusPalette->initPosition();
    mDrawingPalette->initPosition();
}

void UBBoardPaletteManager::setupLayout()
{

}

/**
 * \brief Set up the dock palette widgets
 */
void UBBoardPaletteManager::setupDockPaletteWidgets()
{

    //------------------------------------------------//
    // Create the widgets for the dock palettes

    mpCachePropWidget = new UBCachePropertiesWidget();
    mpDownloadWidget = new UBDockDownloadWidget();


    // Add the dock palettes
    mLeftPalette = new UBLeftPalette(mContainer);

    // LEFT palette widgets
    mpPageNavigWidget = new UBPageNavigationWidget();
    mLeftPalette->registerWidget(mpPageNavigWidget);
    mLeftPalette->addTab(mpPageNavigWidget);


    // Teacher Guide disabled - UBDockTeacherGuideWidget crashes (uses QWebEngineView stubs)
    // if(mSettings->teacherGuidePageZeroActivated->get().toBool() || mSettings->teacherGuideLessonPagesActivated->get().toBool()){
    //     mpTeacherGuideWidget = new UBDockTeacherGuideWidget();
    //     mLeftPalette->registerWidget(mpTeacherGuideWidget);
    //     mLeftPalette->addTab(mpTeacherGuideWidget);
    // }


    //issue 1682 - NNE - 20131218
    mTeacherResources = new UBDockResourcesWidget;
    mLeftPalette->registerWidget(mTeacherResources);
    mLeftPalette->addTab(mTeacherResources);
    //issue 1682 - NNE - 20131218 : END

    mLeftPalette->connectSignals();


    mRightPalette = new UBRightPalette(mContainer);
    // RIGHT palette widgets
    mpFeaturesWidget = new UBFeaturesWidget();
    mRightPalette->registerWidget(mpFeaturesWidget);
    mRightPalette->addTab(mpFeaturesWidget);


    // The cache widget will be visible only if a cache is put on the page
    mRightPalette->registerWidget(mpCachePropWidget);

    //  The download widget will be part of the right palette but
    //  will become visible only when the first download starts
    mRightPalette->registerWidget(mpDownloadWidget);
    mRightPalette->connectSignals();
    changeMode(eUBDockPaletteWidget_BOARD, true);

    // Hide the tabs that must be hidden
    mRightPalette->removeTab(mpDownloadWidget);
    mRightPalette->removeTab(mpCachePropWidget);

}

void UBBoardPaletteManager::slot_changeMainMode(UBApplicationController::MainMode mainMode)
{
//    Board = 0, Internet, Document, WebDocument

    switch( mainMode )
    {
        case UBApplicationController::Board:
            {
                // call changeMode only when switch NOT from desktop mode
                if(!UBApplication::applicationController->isShowingDesktop())
                    changeMode(eUBDockPaletteWidget_BOARD);
            }
            break;

        case UBApplicationController::Internet:
            changeMode(eUBDockPaletteWidget_WEB);
            break;

        case UBApplicationController::Document:
            changeMode(eUBDockPaletteWidget_DOCUMENT);
            break;

        default:
            {
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != nullptr)
                    mKeyboardPalette->hide();
            }
            break;
    }
}

void UBBoardPaletteManager::slot_changeDesktopMode(bool isDesktop)
{
    UBApplicationController::MainMode currMode = UBApplication::applicationController->displayMode();
    if(!isDesktop)
    {
        switch( currMode )
        {
            case UBApplicationController::Board:
                changeMode(eUBDockPaletteWidget_BOARD);
                break;

            default:
                break;
        }
    }
    else
        changeMode(eUBDockPaletteWidget_DESKTOP);
}

void UBBoardPaletteManager::setupPalettes()
{

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        mKeyboardPalette = new UBKeyboardPalette(0);
#ifndef Q_OS_WIN
        connect(mKeyboardPalette, SIGNAL(closed()), mKeyboardPalette, SLOT(onDeactivated()));
#endif
    }


    setupDockPaletteWidgets();



    // Add the other palettes
    // Keep old C++ stylus palette hidden — it still owns the QActions and button group logic
    mStylusPalette = new UBStylusPalette(mContainer, mSettings->appToolBarOrientationVertical->get().toBool() ? Qt::Vertical : Qt::Horizontal);
    connect(mStylusPalette, SIGNAL(stylusToolDoubleClicked(int)), UBApplication::boardController, SLOT(stylusToolDoubleClicked(int)));
    mStylusPalette->hide(); // replaced by QML palette

    mDrawingPalette = new UBDrawingPalette(mContainer, mSettings->appDrawingPaletteOrientationHorizontal->get().toBool() ? Qt::Horizontal : Qt::Vertical);
    mDrawingPalette->hide();

    mZoomPalette = new UBZoomPalette(mContainer);
    mDrawingPalette->stackUnder(mZoomPalette);

    // --- QML Stylus Palette (Issue #110 Step 2) ---
    bool isVertical = mSettings->appToolBarOrientationVertical->get().toBool();

    mStylusController = new UBStylusController(this);
    mStylusController->setVertical(isVertical);

    // Register tool buttons (same order as old UBStylusPalette)
    UBMainWindow* mw = UBApplication::mainWindow;
    mStylusController->addTool(tr("Drawing Palette"), "qrc:/images/stylusPalette/svg/drawing.svg", mw->actionDrawing);
    mStylusController->addTool(tr("Pen"), "qrc:/images/stylusPalette/svg/pen.svg", mw->actionPen);
    mStylusController->addTool(tr("Eraser"), "qrc:/images/stylusPalette/svg/eraser.svg", mw->actionEraser);
    mStylusController->addTool(tr("Marker"), "qrc:/images/stylusPalette/svg/marker.svg", mw->actionMarker);
    mStylusController->addTool(tr("Selector"), "qrc:/images/stylusPalette/svg/selector.svg", mw->actionSelector);
    mStylusController->addTool(tr("Play"), "qrc:/images/stylusPalette/svg/play.svg", mw->actionPlay);
    mStylusController->addTool(tr("Hand"), "qrc:/images/stylusPalette/svg/hand.svg", mw->actionHand);
    mStylusController->addTool(tr("Zoom In"), "qrc:/images/stylusPalette/svg/zoomIn.svg", mw->actionZoomIn);
    mStylusController->addTool(tr("Zoom Out"), "qrc:/images/stylusPalette/svg/zoomOut.svg", mw->actionZoomOut);
    mStylusController->addTool(tr("Pointer"), "qrc:/images/stylusPalette/svg/pointer.svg", mw->actionPointer);
    mStylusController->addTool(tr("Line"), "qrc:/images/stylusPalette/svg/line.svg", mw->actionLine);
    mStylusController->addTool(tr("Text"), "qrc:/images/stylusPalette/svg/text.svg", mw->actionText);
    mStylusController->addTool(tr("Capture"), "qrc:/images/stylusPalette/svg/capture.svg", mw->actionCapture);
    mStylusController->addTool(tr("OCR"), "qrc:/images/stylusPalette/svg/ocr.svg", mw->actionOcr);
    if (mw->actionAutoOcr)
        mStylusController->addTool(tr("Auto OCR"), "qrc:/images/stylusPalette/svg/ocr-auto-off.svg", mw->actionAutoOcr, true);
    mStylusController->finalize();

    // Create the QQuickWidget for the stylus palette
    mStylusPaletteQml = new QQuickWidget(mContainer);
    mStylusPaletteQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mStylusPaletteQml->setClearColor(Qt::transparent);
    mStylusPaletteQml->setAttribute(Qt::WA_TranslucentBackground);
    mStylusPaletteQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mStylusPaletteQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mStylusPaletteQml->rootContext()->setContextProperty("stylusController", mStylusController);
    mStylusPaletteQml->setSource(QUrl("qrc:/qml/StylusPalette.qml"));

    // Size the widget based on orientation and tool count
    int toolCount = mStylusController->tools().size();
    int btnSize = 44;
    int spacing = 2;
    int padding = 6;
    int contentLen = toolCount * btnSize + (toolCount - 1) * spacing + padding * 2;
    int thickness = btnSize + padding * 2;

    if (isVertical) {
        mStylusPaletteQml->setFixedSize(thickness, contentLen);
    } else {
        mStylusPaletteQml->setFixedSize(contentLen, thickness);
    }

    // Position: bottom-center for horizontal, right-center for vertical
    if (isVertical) {
        int posX = mContainer->width() - thickness - 20;
        int posY = (mContainer->height() - contentLen) / 2;
        mStylusPaletteQml->move(posX, posY);
    } else {
        int posX = (mContainer->width() - contentLen) / 2;
        int posY = mContainer->height() - thickness - 20;
        mStylusPaletteQml->move(posX, posY);
    }

    mStylusPaletteQml->show();
    mStylusPaletteQml->raise();

    // --- QML Drawing Properties Panel (Issue #110 Step 3) ---
    mDrawingPropsController = new UBDrawingPropertiesController(this);

    mDrawingPropsQml = new QQuickWidget(mContainer);
    mDrawingPropsQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mDrawingPropsQml->setClearColor(Qt::transparent);
    mDrawingPropsQml->setAttribute(Qt::WA_TranslucentBackground);
    mDrawingPropsQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mDrawingPropsQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mDrawingPropsQml->rootContext()->setContextProperty("drawingProps", mDrawingPropsController);
    mDrawingPropsQml->setSource(QUrl("qrc:/qml/DrawingProperties.qml"));

    // Max size for the widget (7 buttons + separator: 4 colors + sep + 3 widths)
    int dpBtnSize = 36;
    int dpSpacing = 6;
    int dpPadding = 8;
    int dpMaxButtons = 7; // 4 colors + 3 widths
    int dpWidth = dpMaxButtons * dpBtnSize + (dpMaxButtons - 1) * dpSpacing
                  + dpSpacing + 1 + dpSpacing // separator
                  + dpPadding * 2;
    int dpHeight = dpBtnSize + dpPadding * 2;
    mDrawingPropsQml->setFixedSize(dpWidth, dpHeight);

    // Position just above/below the stylus palette
    if (isVertical) {
        int posX = mStylusPaletteQml->x() - dpWidth - 10;
        int posY = mStylusPaletteQml->y();
        mDrawingPropsQml->move(posX, posY);
    } else {
        int posX = (mContainer->width() - dpWidth) / 2;
        int posY = mStylusPaletteQml->y() - dpHeight - 10;
        mDrawingPropsQml->move(posX, posY);
    }

    mDrawingPropsQml->show();
    mDrawingPropsQml->raise();

    // --- QML Shapes Palette (Issue #110 Step 5) ---
    mShapesController = new UBShapesController(this);

    mShapesPaletteQml = new QQuickWidget(mContainer);
    mShapesPaletteQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mShapesPaletteQml->setClearColor(Qt::transparent);
    mShapesPaletteQml->setAttribute(Qt::WA_TranslucentBackground);
    mShapesPaletteQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mShapesPaletteQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mShapesPaletteQml->rootContext()->setContextProperty("shapesController", mShapesController);
    mShapesPaletteQml->setSource(QUrl("qrc:/qml/ShapesPalette.qml"));
    mShapesPaletteQml->setFixedSize(160, 380);

    // Position: to the left of the stylus palette
    if (isVertical) {
        mShapesPaletteQml->move(mStylusPaletteQml->x() - 170, mStylusPaletteQml->y());
    } else {
        mShapesPaletteQml->move(mStylusPaletteQml->x(), mStylusPaletteQml->y() - 390);
    }
    mShapesPaletteQml->show();
    mShapesPaletteQml->raise();

    // Connect the Drawing action to toggle the shapes palette
    connect(UBApplication::mainWindow->actionDrawing, &QAction::toggled, mShapesController, [this](bool checked) {
        mShapesController->setVisible(checked);
    });

    // UBStartupHintsPalette disabled - contains QWebEngineView that crashes on paint
    // mTipPalette = new UBStartupHintsPalette(mContainer);
    mTipPalette = nullptr;

    QList<QAction*> backgroundsActions;

    backgroundsActions << UBApplication::mainWindow->actionPlainLightBackground;
    backgroundsActions << UBApplication::mainWindow->actionCrossedLightBackground;
    backgroundsActions << UBApplication::mainWindow->actionPlainDarkBackground;
    backgroundsActions << UBApplication::mainWindow->actionCrossedDarkBackground;

    mBackgroundsPalette = new UBActionPalette(backgroundsActions, Qt::Horizontal , mContainer);
    mBackgroundsPalette->setButtonIconSize(QSize(128, 128));
    mBackgroundsPalette->groupActions();
    mBackgroundsPalette->setClosable(true);
    mBackgroundsPalette->setAutoClose(true);
    mBackgroundsPalette->adjustSizeAndPosition();
    mBackgroundsPalette->hide();

    QList<QAction*> addItemActions;

    addItemActions << UBApplication::mainWindow->actionAddItemToCurrentPage;
    addItemActions << UBApplication::mainWindow->actionAddItemToNewPage;
    addItemActions << UBApplication::mainWindow->actionAddItemToLibrary;

    mAddItemPalette = new UBActionPalette(addItemActions, Qt::Horizontal, mContainer);
    mAddItemPalette->setButtonIconSize(QSize(128, 128));
    mAddItemPalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mAddItemPalette->groupActions();
    mAddItemPalette->setClosable(true);
    mAddItemPalette->setAutoClose(true);
    mAddItemPalette->adjustSizeAndPosition();
    mAddItemPalette->hide();

    QList<QAction*> eraseActions;

    eraseActions << UBApplication::mainWindow->actionEraseAnnotations;
    eraseActions << UBApplication::mainWindow->actionEraseItems;
    eraseActions << UBApplication::mainWindow->actionClearPage;
    eraseActions << UBApplication::mainWindow->actionEraseBackground;

    mErasePalette = new UBActionPalette(eraseActions, Qt::Horizontal , mContainer);
    mErasePalette->setButtonIconSize(QSize(128, 128));
    mErasePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mErasePalette->groupActions();
    mErasePalette->setClosable(true);
    mErasePalette->setAutoClose(true);
    mErasePalette->adjustSizeAndPosition();
    mErasePalette->hide();

    QList<QAction*> pageActions;

    pageActions << UBApplication::mainWindow->actionNewPage;
    pageActions << UBApplication::mainWindow->actionDuplicatePage;
    pageActions << UBApplication::mainWindow->actionImportPage;

    mPagePalette = new UBActionPalette(pageActions, Qt::Horizontal , mContainer);
    mPagePalette->setButtonIconSize(QSize(128, 128));
    mPagePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mPagePalette->groupActions();
    mPagePalette->setClosable(true);
    mPagePalette->setAutoClose(true);
    mPagePalette->adjustSizeAndPosition();
    mPagePalette->hide();

    // Issue 1684 - CFA - 20131120
    QList<QAction*> imageBackgroundActions;

    imageBackgroundActions << UBApplication::mainWindow->actionCenterImageBackground;
    UBApplication::mainWindow->actionCenterImageBackground->setIcon(QIcon(":/images/imageBackgroundPalette/centerBackground.png"));
    imageBackgroundActions << UBApplication::mainWindow->actionAdjustImageBackground;
    UBApplication::mainWindow->actionAdjustImageBackground->setIcon(QIcon(":/images/imageBackgroundPalette/adjustBackground.png"));
    imageBackgroundActions << UBApplication::mainWindow->actionMosaicImageBackground;
    UBApplication::mainWindow->actionMosaicImageBackground->setIcon(QIcon(":/images/imageBackgroundPalette/mosaicBackground.png"));
    imageBackgroundActions << UBApplication::mainWindow->actionFillImageBackground;
    UBApplication::mainWindow->actionFillImageBackground->setIcon(QIcon(":/images/imageBackgroundPalette/fillBackground.png"));
    imageBackgroundActions << UBApplication::mainWindow->actionExtendImageBackground;
    UBApplication::mainWindow->actionExtendImageBackground->setIcon(QIcon(":/images/imageBackgroundPalette/extendBackground.png"));

    mImageBackgroundPalette = new UBActionPalette(imageBackgroundActions, Qt::Horizontal , mContainer);
    mImageBackgroundPalette->setButtonIconSize(QSize(128, 128));
    mImageBackgroundPalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mImageBackgroundPalette->groupActions();
    mImageBackgroundPalette->setClosable(true);
    mImageBackgroundPalette->setAutoClose(true);
    mImageBackgroundPalette->adjustSizeAndPosition();
    mImageBackgroundPalette->hide();
    // Fin Issue 1684 - CFA - 20131120

    connect(mSettings->appToolBarOrientationVertical, SIGNAL(changed(QVariant)), this, SLOT(changeStylusPaletteOrientation(QVariant)));
}

void UBBoardPaletteManager::pagePaletteButtonPressed()
{
    mPageButtonPressedTime = QTime::currentTime();

    mPendingPageButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(pagePaletteButtonReleased()));
}

void UBBoardPaletteManager::pagePaletteButtonReleased()
{
    if (mPendingPageButtonPressed)
    {
        if( mPageButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            // The palette is reinstanciated because the duplication depends on the current scene
            delete(mPagePalette);
            mPagePalette = 0;
            QList<QAction*>pageActions;
            pageActions << UBApplication::mainWindow->actionNewPage;
            UBBoardController* boardController = UBApplication::boardController;
            if(UBApplication::documentController->pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(boardController->activeSceneIndex()))){
                pageActions << UBApplication::mainWindow->actionDuplicatePage;
            }
            pageActions << UBApplication::mainWindow->actionImportPage;

            mPagePalette = new UBActionPalette(pageActions, Qt::Horizontal , mContainer);
            mPagePalette->setButtonIconSize(QSize(128, 128));
            mPagePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            mPagePalette->groupActions();
            mPagePalette->setClosable(true);
            mPagePalette->setAutoClose(true);

            // As we recreate the pagePalette every time, we must reconnect the slots
            connect(UBApplication::mainWindow->actionNewPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
            connect(UBApplication::mainWindow->actionDuplicatePage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
            connect(UBApplication::mainWindow->actionImportPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
            connect(mPagePalette, SIGNAL(closed()), this, SLOT(pagePaletteClosed()));

            togglePagePalette(true);
        }
        else
        {
            UBApplication::mainWindow->actionNewPage->trigger();
        }

        mPendingPageButtonPressed = false;
    }
}

void UBBoardPaletteManager::erasePaletteButtonPressed()
{
    mEraseButtonPressedTime = QTime::currentTime();

    mPendingEraseButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(erasePaletteButtonReleased()));
}


void UBBoardPaletteManager::erasePaletteButtonReleased()
{
    if (mPendingEraseButtonPressed)
    {
        if( mEraseButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            toggleErasePalette(true);
        }
        else
        {
            UBApplication::mainWindow->actionClearPage->trigger();
        }

        mPendingEraseButtonPressed = false;
    }
}

void UBBoardPaletteManager::linkClicked(const QUrl& url)
{
      UBApplication::applicationController->showInternet();
      UBApplication::webController->loadUrl(url);
}


void UBBoardPaletteManager::purchaseLinkActivated(const QString& link)
{
    UBApplication::applicationController->showInternet();
    UBApplication::webController->loadUrl(QUrl(link));
}

void UBBoardPaletteManager::connectPalettes()
{
    connect(UBApplication::mainWindow->actionDrawing, SIGNAL(toggled(bool)), this, SLOT(toggleDrawingPalette(bool)));
    connect(UBApplication::mainWindow->actionStylus, SIGNAL(toggled(bool)), this, SLOT(toggleStylusPalette(bool)));

    // Close all popup palettes when any toolbar action is triggered
    connect(UBApplication::mainWindow->boardToolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(closeAllPopupPalettes()));

    // Also close popup palettes when stylus palette buttons are clicked
    if (mStylusPalette)
        connect(mStylusPalette, SIGNAL(buttonGroupClicked(int)), this, SLOT(closeAllPopupPalettes()));
    if (mDrawingPalette)
        connect(mDrawingPalette, SIGNAL(buttonGroupClicked(int)), this, SLOT(closeAllPopupPalettes()));

    // Close popup palettes on any stylus tool change
    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int)), this, SLOT(closeAllPopupPalettes()));

    for (QWidget *widget : UBApplication::mainWindow->actionZoomIn->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(zoomButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(zoomButtonReleased()));
        }
    }

    for (QWidget *widget : UBApplication::mainWindow->actionZoomOut->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(zoomButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(zoomButtonReleased()));
        }
    }

    for (QWidget *widget : UBApplication::mainWindow->actionHand->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(panButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(panButtonReleased()));
        }
    }

    connect(UBApplication::mainWindow->actionBackgrounds, SIGNAL(toggled(bool)), this, SLOT(toggleBackgroundPalette(bool)));
    connect(mBackgroundsPalette, SIGNAL(closed()), this, SLOT(backgroundPaletteClosed()));

    connect(UBApplication::mainWindow->actionPlainLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionCrossedLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionPlainDarkBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionCrossedDarkBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionPodcast, SIGNAL(triggered(bool)), this, SLOT(tooglePodcastPalette(bool)));

    connect(UBApplication::mainWindow->actionAddItemToCurrentPage, SIGNAL(triggered()), this, SLOT(addItemToCurrentPage()));
    connect(UBApplication::mainWindow->actionAddItemToNewPage, SIGNAL(triggered()), this, SLOT(addItemToNewPage()));
    connect(UBApplication::mainWindow->actionAddItemToLibrary, SIGNAL(triggered()), this, SLOT(addItemToLibrary()));

    // Issue 1684 - CFA - 20131119
    connect(UBApplication::mainWindow->actionEraseItems, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionEraseAnnotations, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionClearPage, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionEraseBackground,SIGNAL(triggered()),mErasePalette,SLOT(close()));
    connect(mErasePalette, SIGNAL(closed()), this, SLOT(erasePaletteClosed()));

    connect(UBApplication::mainWindow->actionCenterImageBackground, SIGNAL(triggered()), mImageBackgroundPalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionAdjustImageBackground, SIGNAL(triggered()),mImageBackgroundPalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionExtendImageBackground, SIGNAL(triggered()), mImageBackgroundPalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionFillImageBackground,SIGNAL(triggered()),mImageBackgroundPalette,SLOT(close()));
    connect(UBApplication::mainWindow->actionMosaicImageBackground,SIGNAL(triggered()),mImageBackgroundPalette,SLOT(close()));

    for (QWidget *widget : UBApplication::mainWindow->actionErase->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(erasePaletteButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(erasePaletteButtonReleased()));
        }
    }

    connect(UBApplication::mainWindow->actionNewPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionDuplicatePage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionImportPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(mPagePalette, SIGNAL(closed()), this, SLOT(pagePaletteClosed()));

    for (QWidget *widget : UBApplication::mainWindow->actionPages->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(pagePaletteButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(pagePaletteButtonReleased()));
        }
    }
}


bool isFirstResized = true;
void UBBoardPaletteManager::containerResized()
{
    int innerMargin = UBSettings::boardMargin;

    int userLeft = innerMargin;
    int userWidth = mContainer->width() - (2 * innerMargin);
    int userTop = innerMargin;
    int userHeight = mContainer->height() - (2 * innerMargin);

    if(mStylusPalette)
    {
        //mStylusPalette->move(userLeft, userTop);
        mStylusPalette->adjustSizeAndPosition(true,false);
        mStylusPalette->initPosition();
    }

    // Reposition QML stylus palette on resize
    if (mStylusPaletteQml)
    {
        bool isVertical = mStylusController->vertical();
        int w = mStylusPaletteQml->width();
        int h = mStylusPaletteQml->height();

        if (isVertical) {
            int posX = mContainer->width() - w - 20;
            int posY = (mContainer->height() - h) / 2;
            mStylusPaletteQml->move(posX, posY);
        } else {
            int posX = (mContainer->width() - w) / 2;
            int posY = mContainer->height() - h - 20;
            mStylusPaletteQml->move(posX, posY);
        }

        // Reposition drawing properties panel relative to stylus palette
        if (mDrawingPropsQml)
        {
            int dpW = mDrawingPropsQml->width();
            int dpH = mDrawingPropsQml->height();
            if (isVertical) {
                mDrawingPropsQml->move(mStylusPaletteQml->x() - dpW - 10,
                                       mStylusPaletteQml->y());
            } else {
                int posX = (mContainer->width() - dpW) / 2;
                mDrawingPropsQml->move(posX, mStylusPaletteQml->y() - dpH - 10);
            }
        }
    }

    if (mDrawingPalette)
    {
        mDrawingPalette->adjustSizeAndPosition(true,false);
        mDrawingPalette->initPosition();
    }

    if(mZoomPalette)
    {
        mZoomPalette->move(userLeft + userWidth - mZoomPalette->width()
                , userTop + userHeight /*- mPageNumberPalette->height()*/ - innerMargin - mZoomPalette->height());
        mZoomPalette->adjustSizeAndPosition(true,false);
    }

    if (isFirstResized && mKeyboardPalette && mKeyboardPalette->parent() == UBApplication::boardController->controlContainer())
    {
        isFirstResized = false;
        mKeyboardPalette->move(userLeft + (userWidth - mKeyboardPalette->width())/2,
                               userTop + (userHeight - mKeyboardPalette->height())/2);
        mKeyboardPalette->adjustSizeAndPosition();
    }

    if(mLeftPalette)
    {
        mLeftPalette->resize(mLeftPalette->width(), mContainer->height());
    }

    if(mRightPalette)
    {
        mRightPalette->resize(mRightPalette->width(), mContainer->height());
    }
}


void UBBoardPaletteManager::changeBackground()
{
    if (UBApplication::mainWindow->actionCrossedLightBackground->isChecked())
        UBApplication::boardController->changeBackground(false, true);
    else if (UBApplication::mainWindow->actionPlainDarkBackground->isChecked())
        UBApplication::boardController->changeBackground(true, false);
    else if (UBApplication::mainWindow->actionCrossedDarkBackground->isChecked())
        UBApplication::boardController->changeBackground(true, true);
    else
        UBApplication::boardController->changeBackground(false, false);

    UBApplication::mainWindow->actionBackgrounds->setChecked(false);
}


void UBBoardPaletteManager::activeSceneChanged()
{
    UBGraphicsScene *activeScene =  UBApplication::boardController->activeScene();
    int pageIndex = UBApplication::boardController->activeSceneIndex();

    if (mStylusPalette)
        connect(mStylusPalette, SIGNAL(mouseEntered()), activeScene, SLOT(hideEraser()));

    if (mpPageNavigWidget)
    {
        mpPageNavigWidget->setPageNumber(UBDocumentContainer::pageFromSceneIndex(pageIndex), activeScene->document()->pageCount());
    }

    //issue 1682 - NNE - 20140113
    if(pageIndex > 0){
        int currentTabIndex = mLeftPalette->currentTabIndex();
        mLeftPalette->onShowTabWidget(mTeacherResources); // ALTI/AOU - 20140217 : instead of addTab(), we use onShowTabWidget() because it calls moveTabs().
        mLeftPalette->showTabWidget(currentTabIndex); // Stay on same tab. Don't go to the added tab.
    }else{
        mLeftPalette->onHideTabWidget(mTeacherResources); // ALTI/AOU - 20140217 : instead of removeTab(), we use onHideTabWidget() because it calls moveTabs().
        mLeftPalette->showTabWidget(mLeftPalette->currentTabIndex());
    }
    //issue 1682 - NNE - 20140113 : END

    if (mZoomPalette)
        connect(mZoomPalette, SIGNAL(mouseEntered()), activeScene, SLOT(hideEraser()));

    if (mBackgroundsPalette)
        connect(mBackgroundsPalette, SIGNAL(mouseEntered()), activeScene, SLOT(hideEraser()));
}


void UBBoardPaletteManager::toggleBackgroundPalette(bool checked)
{
    mBackgroundsPalette->setVisible(checked);

    if (checked)
    {
        UBApplication::mainWindow->actionErase->setChecked(false);
        UBApplication::mainWindow->actionNewPage->setChecked(false);

        mBackgroundsPalette->adjustSizeAndPosition();
    }
}


void UBBoardPaletteManager::backgroundPaletteClosed()
{
    UBApplication::mainWindow->actionBackgrounds->setChecked(false);
}

void UBBoardPaletteManager::toggleStylusPalette(bool checked)
{
    mStylusPalette->setVisible(checked);
    if (mStylusPaletteQml)
        mStylusPaletteQml->setVisible(checked);
}

void UBBoardPaletteManager::toggleDrawingPalette(bool checked)
{
    // Old C++ palette hidden — replaced by QML ShapesPalette
    // mDrawingPalette->setVisible(checked);
    Q_UNUSED(checked);
}


void UBBoardPaletteManager::toggleErasePalette(bool checked)
{
    mErasePalette->setVisible(checked);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionNewPage->setChecked(false);

        mErasePalette->adjustSizeAndPosition();
    }
}


void UBBoardPaletteManager::erasePaletteClosed()
{
    UBApplication::mainWindow->actionErase->setChecked(false);
}


// Issue 1684 - CFA - 20131120
void UBBoardPaletteManager::toggleImageBackgroundPalette(bool checked, bool isDefault)
{
    mImageBackgroundPalette->setVisible(checked);
    UBApplication::boardController->selectedDocument()->setHasDefaultImageBackground(isDefault);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionErase->setChecked(false);

        mImageBackgroundPalette->adjustSizeAndPosition();
    }
}

void UBBoardPaletteManager::togglePagePalette(bool checked)
{
    mPagePalette->setVisible(checked);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionErase->setChecked(false);

        mPagePalette->adjustSizeAndPosition();
    }
}


void UBBoardPaletteManager::pagePaletteClosed()
{
    UBApplication::mainWindow->actionPages->setChecked(false);
}


void UBBoardPaletteManager::tooglePodcastPalette(bool checked)
{
    UBPodcastController::instance()->toggleRecordingPalette(checked);
}


void UBBoardPaletteManager::addItem(const QUrl& pUrl)
{
    mItemUrl = pUrl;
    mPixmap = QPixmap();
    mPos = QPointF(0, 0);
    mScaleFactor = 1.;

    mAddItemPalette->show();
    mAddItemPalette->adjustSizeAndPosition();
}

void UBBoardPaletteManager::changeMode(eUBDockPaletteWidgetMode newMode, bool isInit)
{
    bool rightPaletteVisible = mRightPalette->switchMode(newMode);
    bool leftPaletteVisible = mLeftPalette->switchMode(newMode);

    if (newMode != eUBDockPaletteWidget_BOARD)
    {
        if (mBackgroundsPalette)
            mBackgroundsPalette->savePos();
        if (mKeyboardPalette)
            mKeyboardPalette->savePos();
        if (mZoomPalette)
            mZoomPalette->savePos();
        if (mPagePalette)
            mPagePalette->savePos();
        if (mErasePalette)
            mErasePalette->savePos();
        if (mAddItemPalette)
            mAddItemPalette->savePos();
    }
    else
    {
        if (mBackgroundsPalette)
            mBackgroundsPalette->restorePos();
        if (mKeyboardPalette)
            mKeyboardPalette->restorePos();
        if (mZoomPalette)
            mZoomPalette->restorePos();
        if (mPagePalette)
            mPagePalette->restorePos();
        if (mErasePalette)
            mErasePalette->restorePos();
        if (mAddItemPalette)
            mAddItemPalette->restorePos();
    }


    switch( newMode )
    {
        case eUBDockPaletteWidget_BOARD:
            {
                // On Application start up the mAddItemPalette isn't initialized yet
                if(mAddItemPalette){
                    mAddItemPalette->setParent(UBApplication::boardController->controlContainer());
                }
                mLeftPalette->assignParent(mContainer);
                mRightPalette->assignParent(mContainer);

                // Restore QML palettes when returning from desktop mode
                if (mStylusPaletteQml)
                    mStylusPaletteQml->show();
                if (mDrawingPropsQml)
                    mDrawingPropsQml->show();
                if (mShapesPaletteQml)
                    mShapesPaletteQml->show();

                if (mDrawingPalette)
                    mDrawingPalette->stackUnder(mStylusPalette);

                // Dock palettes must be above the board view but below floating palettes
                // Don't stackUnder — let raise() in setVisible handle z-order

                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != nullptr)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::boardController->controlContainer());
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::boardController->controlContainer());
                }

                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
#ifdef Q_OS_WIN
                if (rightPaletteVisible)
                    mRightPalette->setAdditionalVOffset(0);
#endif

                if( !isInit )
                    containerResized();
                else
                {
                    // At startup, ensure palettes are sized to container
                    if(mLeftPalette)
                        mLeftPalette->resize(mLeftPalette->width(), mContainer->height());
                    if(mRightPalette)
                        mRightPalette->resize(mRightPalette->width(), mContainer->height());
                }
                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        case eUBDockPaletteWidget_DESKTOP:
            {
                // Hide QML palettes — they are parented to mContainer (board view) which
                // gets hidden in desktop mode. Calling raise() on them would crash.
                if (mStylusPaletteQml)
                    mStylusPaletteQml->hide();
                if (mDrawingPropsQml)
                    mDrawingPropsQml->hide();
                if (mShapesPaletteQml)
                    mShapesPaletteQml->hide();

                mAddItemPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mLeftPalette->assignParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mRightPalette->assignParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mStylusPalette->raise();
                mDrawingPalette->raise();

                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != nullptr)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
#ifndef Q_OS_LINUX
                        mKeyboardPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
#else
                        mKeyboardPalette->setParent(0);
#endif
#ifdef Q_OS_MACOS
                        mKeyboardPalette->setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint);
#endif
                        mKeyboardPalette->show();
                    }
                    else
// In linux keyboard in desktop mode have to allways be with null parent
#ifdef Q_OS_LINUX
                        mKeyboardPalette->setParent(0);
#else
                        mKeyboardPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
#endif //Q_OS_LINUX
#ifdef Q_OS_MACOS
                        mKeyboardPalette->setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint);
#endif

                }

                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
#ifdef Q_OS_WIN
                if (rightPaletteVisible)
                {
                    if (mSettings->appToolBarPositionedAtTop->get().toBool())
                        mRightPalette->setAdditionalVOffset(30);
                    else
                    {
                        QScreen *screen = mRightPalette->screen();
                        int taskBarOffset = screen->geometry().height() - screen->availableGeometry().height();
                        mRightPalette->setAdditionalVOffset(-taskBarOffset);
                    }
                }
#endif

                if(!isInit)
                    UBApplication::applicationController->uninotesController()->TransparentWidgetResized();

                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        case eUBDockPaletteWidget_WEB:
            {
                mAddItemPalette->setParent(UBApplication::mainWindow);

#ifdef SANKORE_WEBENGINE
                mRightPalette->assignParent(UBApplication::webController->GetCurrentWebBrowser());
#endif
                mRightPalette->setVisible(rightPaletteVisible);

                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != nullptr)
                {
                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::mainWindow);
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::mainWindow);
                }

            }
            break;

        case eUBDockPaletteWidget_DOCUMENT:
            {
                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
                mLeftPalette->assignParent(UBApplication::documentController->controlView());
                mRightPalette->assignParent(UBApplication::documentController->controlView());
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != nullptr)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::documentController->controlView());
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::documentController->controlView());
                }
                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        default:
            {
                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
                mLeftPalette->assignParent(0);
                mRightPalette->assignParent(0);
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != nullptr)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(0);
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(0);
                }
            }
            break;
    }

    if( !isInit )
        UBApplication::boardController->notifyPageChanged();
}

void UBBoardPaletteManager::addItem(const QPixmap& pPixmap, const QPointF& pos,  qreal scaleFactor, const QUrl& sourceUrl)
{
    mItemUrl = sourceUrl;
    mPixmap = pPixmap;
    mPos = pos;
    mScaleFactor = scaleFactor;

    mAddItemPalette->show();
    mAddItemPalette->adjustSizeAndPosition();
}


void UBBoardPaletteManager::addItemToCurrentPage()
{
    //Issue NC - CFA - 20140331 : retour au mode board si mode desktop (en mode desktop, displayMode() renvoie Board...)
    if (UBApplication::applicationController->displayMode() != UBApplicationController::Board || UBApplication::applicationController->isShowingDesktop())
        UBApplication::applicationController->showBoard();

    mAddItemPalette->hide();
    if(mPixmap.isNull())
        UBApplication::boardController->downloadURL(mItemUrl);
    else
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);

        UBGraphicsPixmapItem* item = UBApplication::boardController->activeScene()->addPixmap(mPixmap, nullptr, mPos, mScaleFactor);

        item->setSourceUrl(mItemUrl);
        item->setSelected(true);       
    }
}


void UBBoardPaletteManager::addItemToNewPage()
{
    UBApplication::boardController->addScene();
    addItemToCurrentPage();
}


void UBBoardPaletteManager::addItemToLibrary()
{
    if(mPixmap.isNull())
    {
       mPixmap = QPixmap(mItemUrl.toLocalFile());
    }

    if(!mPixmap.isNull())
    {
        if(mScaleFactor != 1.)
        {
             mPixmap = mPixmap.scaled(mScaleFactor * mPixmap.width(), mScaleFactor* mPixmap.height()
                     , Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        QImage image = mPixmap.toImage();

        QDateTime now = QDateTime::currentDateTime();
        QString capturedName  = tr("CapturedImage") + "-" + now.toString("dd-MM-yyyy hh-mm-ss") + ".png";
        mpFeaturesWidget->importImage(image, capturedName);
    }
    else
    {
        UBApplication::showMessage(tr("Error Adding Image to Library"));
    }

    mAddItemPalette->hide();
}

void UBBoardPaletteManager::zoomButtonPressed()
{
    mZoomButtonPressedTime = QTime::currentTime();

    mPendingZoomButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(zoomButtonReleased()));
}

void UBBoardPaletteManager::zoomButtonReleased()
{
    if (mPendingZoomButtonPressed)
    {
        if(mZoomButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            mBoardControler->zoomRestore();
        }

        mPendingZoomButtonPressed = false;
    }
}

void UBBoardPaletteManager::panButtonPressed()
{
    mPanButtonPressedTime = QTime::currentTime();

    mPendingPanButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(panButtonReleased()));
}


void UBBoardPaletteManager::panButtonReleased()
{
    if (mPendingPanButtonPressed)
    {
        if(mPanButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            mBoardControler->centerRestore();
        }

        mPendingPanButtonPressed = false;
    }
}

void UBBoardPaletteManager::showVirtualKeyboard(bool show)
{
    if (mKeyboardPalette)
        mKeyboardPalette->setVisible(show);
}

void UBBoardPaletteManager::changeStylusPaletteOrientation(QVariant var)
{
    bool bVertical = var.toBool();
    bool bVisible = mStylusPalette->isVisible();

    // Clean the old palette
    if(nullptr != mStylusPalette)
    {
        delete mStylusPalette;
        mStylusPalette = nullptr;
    }

    // Create the new palette
    if(bVertical)
    {
        mStylusPalette = new UBStylusPalette(mContainer, Qt::Vertical);
    }
    else
    {
        mStylusPalette = new UBStylusPalette(mContainer, Qt::Horizontal);
    }

    connect(mStylusPalette, SIGNAL(stylusToolDoubleClicked(int)), UBApplication::boardController, SLOT(stylusToolDoubleClicked(int)));
    mStylusPalette->setVisible(bVisible); // always show stylus palette at startup
    mDrawingPalette->initPosition(); // move de drawing Palette

    // Update QML palette orientation
    if (mStylusController)
    {
        mStylusController->setVertical(bVertical);

        // Resize the QML widget
        int toolCount = mStylusController->tools().size();
        int btnSize = 44;
        int spacing = 2;
        int padding = 6;
        int contentLen = toolCount * btnSize + (toolCount - 1) * spacing + padding * 2;
        int thickness = btnSize + padding * 2;

        if (mStylusPaletteQml)
        {
            if (bVertical)
                mStylusPaletteQml->setFixedSize(thickness, contentLen);
            else
                mStylusPaletteQml->setFixedSize(contentLen, thickness);

            // Reposition
            if (bVertical) {
                int posX = mContainer->width() - thickness - 20;
                int posY = (mContainer->height() - contentLen) / 2;
                mStylusPaletteQml->move(posX, posY);
            } else {
                int posX = (mContainer->width() - contentLen) / 2;
                int posY = mContainer->height() - thickness - 20;
                mStylusPaletteQml->move(posX, posY);
            }

            mStylusPaletteQml->setVisible(bVisible);
        }
    }
}


void UBBoardPaletteManager::connectToDocumentController()
{
    emit connectToDocController();
}

void UBBoardPaletteManager::refreshPalettes()
{
    mRightPalette->update();
    mLeftPalette->update();
}

void UBBoardPaletteManager::startDownloads()
{
    if(!mDownloadInProgress)
    {
        mDownloadInProgress = true;
        mpDownloadWidget->setVisibleState(true);
        mRightPalette->addTab(mpDownloadWidget);
    }
}

void UBBoardPaletteManager::stopDownloads()
{
    if(mDownloadInProgress)
    {
        mDownloadInProgress = false;
        mpDownloadWidget->setVisibleState(false);
        mRightPalette->removeTab(mpDownloadWidget);
    }
}


UBCreateLinkPalette* UBBoardPaletteManager::linkPalette()
{
    if(mLinkPalette)
        delete mLinkPalette;
    mLinkPalette = new UBCreateLinkPalette(mContainer);
    return mLinkPalette;
}


void UBBoardPaletteManager::closeAllPopupPalettes()
{
    if (mAddItemPalette && mAddItemPalette->isVisible())
        mAddItemPalette->close();
    if (mErasePalette && mErasePalette->isVisible())
        mErasePalette->close();
    if (mPagePalette && mPagePalette->isVisible())
        mPagePalette->close();
    if (mImageBackgroundPalette && mImageBackgroundPalette->isVisible())
        mImageBackgroundPalette->close();
    if (mBackgroundsPalette && mBackgroundsPalette->isVisible())
        mBackgroundsPalette->close();
    if (mDrawingPalette && mDrawingPalette->isVisible())
    {
        // Only hide drawing palette if the current tool is NOT the Drawing tool
        int currentTool = UBDrawingController::drawingController()->stylusTool();
        if (currentTool != UBStylusTool::Drawing)
        {
            mDrawingPalette->hideSubPalettes();
            mDrawingPalette->hide();
            UBApplication::mainWindow->actionDrawing->setChecked(false);
        }
    }
}
