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



#include "UBBoardPaletteManager.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDisplayManager.h"

#include "gui/UBMainWindow.h"
#include "gui/UBKeyboardPalette.h"
#include "gui/UBToolWidget.h"
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
#include <QTimer>
#include <QToolBar>
#include <QFile>
#include <QTextStream>
#include <QPainterPath>
#include "qml/UBThemeManager.h"


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
#include "controllers/UBToolController.h"
#include "controllers/UBPageController.h"
#include "controllers/UBAppController.h"


UBBoardPaletteManager::UBBoardPaletteManager(QWidget* container, UBBoardController* pBoardController)
    : QObject(container)
    , mKeyboardPalette(0)
    , mWebToolsCurrentPalette(0)
    , mContainer(container)
    , mBoardControler(pBoardController)
    , mStylusPaletteQml(nullptr)
    , mToolController(nullptr)
    , mPageController(nullptr)
    , mAppController(nullptr)
    , mTopBarQml(nullptr)
    , mPageNavQml(nullptr)
    , mDrawingPropsBarQml(nullptr)
    , mShapesPaletteV2Qml(nullptr)
    , mLinkPalette(0)
    , mAddItemPalette(0)
    , mImageBackgroundPalette(nullptr)
    , mDownloadInProgress(false)
    , mPaletteMode(eUBDockPaletteWidget_BOARD)
{
    mSettings = UBSettings::settings();
    mTeacherResources = nullptr;
    setupPalettes();
    connectPalettes();
}


UBBoardPaletteManager::~UBBoardPaletteManager()
{
    // Destroy QML widgets BEFORE their controllers are deleted.
    delete mShapesPaletteV2Qml;
    mShapesPaletteV2Qml = nullptr;
    delete mDrawingPropsBarQml;
    mDrawingPropsBarQml = nullptr;
    delete mStylusPaletteQml;
    mStylusPaletteQml = nullptr;
    delete mTopBarQml;
    mTopBarQml = nullptr;
    delete mPageNavQml;
    mPageNavQml = nullptr;

// mAddedItemPalette is delete automatically because of is parent
// that changes depending on the mode

// mMainWindow->centralWidget is the parent of mStylusPalette
// do not delete this here.
}

void UBBoardPaletteManager::initPalettesPosAtStartup()
{
    // Legacy palettes removed — QML V2 palettes are positioned in containerResized()
}

void UBBoardPaletteManager::setupLayout()
{

}

/**
 * \brief Set up the dock palette widgets
 */
void UBBoardPaletteManager::setupDockPaletteWidgets()
{
    // Dock palettes removed (QML V2 replaces them).
    // Only keep mpFeaturesWidget (used by addItemToLibrary).
    mTeacherResources = new UBDockResourcesWidget;
    mpFeaturesWidget = new UBFeaturesWidget();
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
                // Sync QML controller back to Board when returning from Documents mode
                if (mAppController && mAppController->activeMode() != UBAppController::Board)
                    mAppController->syncMode(UBAppController::Board);
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
        // Sync QML controller back to Board when leaving Desktop mode
        if (mAppController)
            mAppController->syncMode(UBAppController::Board);

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
        connect(mKeyboardPalette, &UBActionPalette::closed, mKeyboardPalette, &UBKeyboardPalette::onDeactivated);
#endif
    }


    setupDockPaletteWidgets();



    // --- QML V2 Stylus Palette ---
    bool isVertical = mSettings->appToolBarOrientationVertical->get().toBool();

    // Create the QQuickWidget for the stylus palette
    mStylusPaletteQml = new QQuickWidget(mContainer);
    mStylusPaletteQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mStylusPaletteQml->setClearColor(Qt::transparent);
    mStylusPaletteQml->setAttribute(Qt::WA_TranslucentBackground);
    mStylusPaletteQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mStylusPaletteQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());

    // New V2 controller — direct binding, no QAction
    mToolController = new UBToolController(this);
    // Force Pen as the active tool at startup (the old UBStylusController/QActionGroup
    // may have changed UBDrawingController to Selector during finalize())
    mToolController->setActiveTool(UBToolController::Pen);
    mPageController = new UBPageController(this);
    mAppController = new UBAppController(this);
    mStylusPaletteQml->rootContext()->setContextProperty("toolController", mToolController);
    mStylusPaletteQml->rootContext()->setContextProperty("pageController", mPageController);
    mStylusPaletteQml->rootContext()->setContextProperty("appController", mAppController);

    mStylusPaletteQml->setSource(QUrl("qrc:/qml/StylusPaletteV2.qml"));
    if (mStylusPaletteQml->status() == QQuickWidget::Error)
        for (const auto& e : mStylusPaletteQml->errors())
            qWarning() << "StylusPaletteV2 QML error:" << e.toString();

    // Size the widget — let QML compute, use a generous max
    int btnSize = 40;
    int numTools = 14; // excluding separators
    int numSeps = 2;
    int sepWidth = 1 + 12; // sep + margins
    int spacing = 2;
    int padding = 6;
    int contentLen = numTools * btnSize + (numTools - 1) * spacing + numSeps * sepWidth + padding * 2;
    int thickness = btnSize + padding * 2;

    if (isVertical) {
        mStylusPaletteQml->setFixedSize(thickness, contentLen);
    } else {
        mStylusPaletteQml->setFixedSize(contentLen, thickness);
    }
    // Rounded mask: clips corners and lets clicks through outside the rounded shape
    {
        int w = mStylusPaletteQml->width();
        int h = mStylusPaletteQml->height();
        QPainterPath path;
        path.addRoundedRect(0, 0, w, h, 12, 12);
        mStylusPaletteQml->setMask(QRegion(path.toFillPolygon().toPolygon()));
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

    // --- QML Top Bar (Issue #121 Step 3) ---
    mTopBarQml = new QQuickWidget(mContainer);
    mTopBarQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mTopBarQml->setClearColor(Qt::transparent);
    mTopBarQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mTopBarQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mTopBarQml->rootContext()->setContextProperty("appController", mAppController);
    mTopBarQml->rootContext()->setContextProperty("pageController", mPageController);
    mTopBarQml->rootContext()->setContextProperty("toolController", mToolController);
    mTopBarQml->setSource(QUrl("qrc:/qml/TopBar.qml"));
    if (mTopBarQml->status() == QQuickWidget::Error)
        for (const auto& e : mTopBarQml->errors())
            qWarning() << "TopBar QML error:" << e.toString();
    mTopBarQml->setFixedSize(mContainer->width(), 48);
    mTopBarQml->move(0, 0);
    mTopBarQml->show();
    mTopBarQml->raise();

    // --- QML Page Navigator Sidebar (Issue #121 Step 4) ---
    mPageNavQml = new QQuickWidget(mContainer);
    mPageNavQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mPageNavQml->setClearColor(Qt::transparent);
    mPageNavQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mPageNavQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mPageNavQml->rootContext()->setContextProperty("pageController", mPageController);
    mPageNavQml->setSource(QUrl("qrc:/qml/PageNavigator.qml"));
    if (mPageNavQml->status() == QQuickWidget::Error)
        for (const auto& e : mPageNavQml->errors())
            qWarning() << "PageNavigator QML error:" << e.toString();
    int sidebarWidth = 180;
    mPageNavQml->setFixedSize(sidebarWidth, mContainer->height() - 48 - 52); // between top bar and bottom bar
    mPageNavQml->move(0, 48);
    mPageNavQml->show();
    mPageNavQml->raise();

    // --- QML Drawing Props Bar (Issue #121 Step 5) ---
    mDrawingPropsBarQml = new QQuickWidget(mContainer);
    mDrawingPropsBarQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mDrawingPropsBarQml->setClearColor(Qt::transparent);
    mDrawingPropsBarQml->setAttribute(Qt::WA_TranslucentBackground);
    mDrawingPropsBarQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mDrawingPropsBarQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mDrawingPropsBarQml->rootContext()->setContextProperty("toolController", mToolController);
    mDrawingPropsBarQml->setSource(QUrl("qrc:/qml/DrawingPropsBar.qml"));
    if (mDrawingPropsBarQml->status() == QQuickWidget::Error)
        for (const auto& e : mDrawingPropsBarQml->errors())
            qWarning() << "DrawingPropsBar QML error:" << e.toString();

    // Size: 280 for pen/marker (colors + widths), 120 for eraser (widths only)
    // Also hides/shows the QQuickWidget at C++ level to avoid ghost black bar (#129)
    auto updatePropsBarSize = [this]() {
        if (!mDrawingPropsBarQml || !mContainer || !mStylusPaletteQml)
            return;
        if (!mToolController->showDrawingProps()) {
            mDrawingPropsBarQml->hide();
            return;
        }
        bool isEraser = (mToolController->activeTool() == UBStylusTool::Eraser);
        int barW = isEraser ? 120 : 280;
        mDrawingPropsBarQml->setFixedSize(barW, 48);
        // Apply rounded mask so clicks outside the rounded shape pass through
        QPainterPath path;
        path.addRoundedRect(0, 0, barW, 48, 12, 12);
        mDrawingPropsBarQml->setMask(QRegion(path.toFillPolygon().toPolygon()));
        int posX = (mContainer->width() - barW) / 2;
        int posY = mStylusPaletteQml->y() - 48 - 8;
        mDrawingPropsBarQml->move(posX, posY);
        mDrawingPropsBarQml->show();
        mDrawingPropsBarQml->raise();
    };
    mDrawingPropsBarQml->setFixedSize(280, 48);
    {
        QPainterPath path;
        path.addRoundedRect(0, 0, 280, 48, 12, 12);
        mDrawingPropsBarQml->setMask(QRegion(path.toFillPolygon().toPolygon()));
    }
    int propsX = (mContainer->width() - 280) / 2;
    int propsY = mContainer->height() - 52 - 70; // above bottom bar
    mDrawingPropsBarQml->move(propsX, propsY);
    mDrawingPropsBarQml->show();
    mDrawingPropsBarQml->raise();

    // Re-size and re-center when tool changes (eraser is narrower than pen)
    connect(mToolController, &UBToolController::activeToolChanged, this, [updatePropsBarSize]() {
        QTimer::singleShot(0, updatePropsBarSize);
    });

    // --- QML Shapes Palette V2 (Issue #121 Step 5) ---
    mShapesPaletteV2Qml = new QQuickWidget(mContainer);
    mShapesPaletteV2Qml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mShapesPaletteV2Qml->setClearColor(Qt::transparent);
    mShapesPaletteV2Qml->setAttribute(Qt::WA_TranslucentBackground);
    mShapesPaletteV2Qml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mShapesPaletteV2Qml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mShapesPaletteV2Qml->rootContext()->setContextProperty("toolController", mToolController);
    mShapesPaletteV2Qml->setSource(QUrl("qrc:/qml/ShapesPaletteV2.qml"));
    mShapesPaletteV2Qml->setFixedSize(160, 320);
    {
        QPainterPath path;
        path.addRoundedRect(0, 0, 160, 320, 12, 12);
        mShapesPaletteV2Qml->setMask(QRegion(path.toFillPolygon().toPolygon()));
    }
    // Positioned to the left of center, above bottom bar
    mShapesPaletteV2Qml->move(sidebarWidth + 20, mContainer->height() - 52 - 330);
    mShapesPaletteV2Qml->hide(); // starts hidden, controlled by toolController.shapesVisible
    // Show/hide based on controller
    connect(mToolController, &UBToolController::shapesVisibleChanged, this, [this]() {
        if (mToolController->shapesVisible())
        {
            // Position above bottom bar, to the right of sidebar
            int y = mContainer->height() - mStylusPaletteQml->height() - mShapesPaletteV2Qml->height() - 28;
            mShapesPaletteV2Qml->move(190, qMax(50, y));
            mShapesPaletteV2Qml->show();
            mShapesPaletteV2Qml->raise();
        }
        else
        {
            mShapesPaletteV2Qml->hide();
        }
    });

    // Debug: log QML widget positions
    // Diagnostic: write widget positions to startup.log
    {
        QString exePath = QCoreApplication::applicationDirPath();
        QFile logFile(exePath + "/startup.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&logFile);
            out << "\n=== QML UI V2 Widget Positions ===\n";
            out << "Container: " << mContainer->width() << "x" << mContainer->height() << "\n";
            out << "StylusPalette: pos=" << mStylusPaletteQml->x() << "," << mStylusPaletteQml->y()
                << " size=" << mStylusPaletteQml->width() << "x" << mStylusPaletteQml->height()
                << " visible=" << mStylusPaletteQml->isVisible() << "\n";
            out << "TopBar: pos=" << mTopBarQml->x() << "," << mTopBarQml->y()
                << " size=" << mTopBarQml->width() << "x" << mTopBarQml->height()
                << " visible=" << mTopBarQml->isVisible()
                << " status=" << mTopBarQml->status() << "\n";
            out << "PageNav: pos=" << mPageNavQml->x() << "," << mPageNavQml->y()
                << " size=" << mPageNavQml->width() << "x" << mPageNavQml->height()
                << " visible=" << mPageNavQml->isVisible()
                << " status=" << mPageNavQml->status() << "\n";
            out << "DrawingPropsBar: pos=" << mDrawingPropsBarQml->x() << "," << mDrawingPropsBarQml->y()
                << " size=" << mDrawingPropsBarQml->width() << "x" << mDrawingPropsBarQml->height()
                << " visible=" << mDrawingPropsBarQml->isVisible()
                << " status=" << mDrawingPropsBarQml->status() << "\n";
            // Log QML errors
            for (const auto& e : mStylusPaletteQml->errors())
                out << "StylusPalette ERROR: " << e.toString() << "\n";
            for (const auto& e : mTopBarQml->errors())
                out << "TopBar ERROR: " << e.toString() << "\n";
            for (const auto& e : mPageNavQml->errors())
                out << "PageNav ERROR: " << e.toString() << "\n";
            for (const auto& e : mDrawingPropsBarQml->errors())
                out << "DrawingPropsBar ERROR: " << e.toString() << "\n";
            for (const auto& e : mShapesPaletteV2Qml->errors())
                out << "ShapesPaletteV2 ERROR: " << e.toString() << "\n";
            out << "ShapesPaletteV2: status=" << mShapesPaletteV2Qml->status() << "\n";
            out << "ToolController activeTool=" << mToolController->activeTool() << "\n";
            out << "===================================\n";
            logFile.close();
        }
    }

    // Old DrawingProperties and ShapesPalette V1 removed — superseded by
    // DrawingPropsBar.qml and ShapesPaletteV2.qml (controlled by UBToolController)

    // UBStartupHintsPalette disabled - contains QWebEngineView that crashes on paint

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

    connect(mSettings->appToolBarOrientationVertical, &UBSetting::changed, this, &UBBoardPaletteManager::changeStylusPaletteOrientation);
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
    // Close all popup palettes when any toolbar action is triggered
    connect(UBApplication::mainWindow->boardToolBar, &QToolBar::actionTriggered, this, [this]() { closeAllPopupPalettes(); });

    // Close popup palettes on any stylus tool change
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, this, [this]() { closeAllPopupPalettes(); });

    connect(UBApplication::mainWindow->actionPlainLightBackground, &QAction::triggered, this, [this]() { changeBackground(); });
    connect(UBApplication::mainWindow->actionCrossedLightBackground, &QAction::triggered, this, [this]() { changeBackground(); });
    connect(UBApplication::mainWindow->actionPlainDarkBackground, &QAction::triggered, this, [this]() { changeBackground(); });
    connect(UBApplication::mainWindow->actionCrossedDarkBackground, &QAction::triggered, this, [this]() { changeBackground(); });
    connect(UBApplication::mainWindow->actionPodcast, &QAction::triggered, this, &UBBoardPaletteManager::tooglePodcastPalette);

    connect(UBApplication::mainWindow->actionAddItemToCurrentPage, &QAction::triggered, this, [this]() { addItemToCurrentPage(); });
    connect(UBApplication::mainWindow->actionAddItemToNewPage, &QAction::triggered, this, [this]() { addItemToNewPage(); });
    connect(UBApplication::mainWindow->actionAddItemToLibrary, &QAction::triggered, this, [this]() { addItemToLibrary(); });

    connect(UBApplication::mainWindow->actionCenterImageBackground, &QAction::triggered, mImageBackgroundPalette, [this]() { mImageBackgroundPalette->close(); });
    connect(UBApplication::mainWindow->actionAdjustImageBackground, &QAction::triggered, mImageBackgroundPalette, [this]() { mImageBackgroundPalette->close(); });
    connect(UBApplication::mainWindow->actionExtendImageBackground, &QAction::triggered, mImageBackgroundPalette, [this]() { mImageBackgroundPalette->close(); });
    connect(UBApplication::mainWindow->actionFillImageBackground, &QAction::triggered, mImageBackgroundPalette, [this]() { mImageBackgroundPalette->close(); });
    connect(UBApplication::mainWindow->actionMosaicImageBackground, &QAction::triggered, mImageBackgroundPalette, [this]() { mImageBackgroundPalette->close(); });
}


bool isFirstResized = true;
void UBBoardPaletteManager::containerResized()
{
    int innerMargin = UBSettings::boardMargin;

    int userLeft = innerMargin;
    int userWidth = mContainer->width() - (2 * innerMargin);
    int userTop = innerMargin;
    int userHeight = mContainer->height() - (2 * innerMargin);

    // Reposition QML stylus palette on resize
    if (mStylusPaletteQml)
    {
        bool isVertical = mSettings->appToolBarOrientationVertical->get().toBool();
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

    }

    // Only show/raise QML palettes in Board mode — in Desktop/Document mode they must
    // stay hidden. Showing QQuickWidgets on a hidden parent container causes access
    // violations in QGraphicsView::viewportEvent.
    bool isBoardMode = (mPaletteMode == eUBDockPaletteWidget_BOARD);

    // Reposition QML V2 widgets on container resize
    if (mTopBarQml)
    {
        mTopBarQml->setFixedSize(mContainer->width(), 48);
        mTopBarQml->move(0, 0);
        if (isBoardMode) { mTopBarQml->show(); mTopBarQml->raise(); }
    }
    if (mPageNavQml)
    {
        int sidebarHeight = mContainer->height() - 48 - 52; // between top bar and bottom bar
        mPageNavQml->setFixedSize(180, qMax(100, sidebarHeight));
        mPageNavQml->move(0, 48);
        if (isBoardMode) { mPageNavQml->show(); mPageNavQml->raise(); }
    }
    if (mDrawingPropsBarQml)
    {
        int posX = (mContainer->width() - mDrawingPropsBarQml->width()) / 2;
        int posY = mStylusPaletteQml->y() - mDrawingPropsBarQml->height() - 8;
        mDrawingPropsBarQml->move(posX, posY);
        if (isBoardMode && mToolController && mToolController->showDrawingProps()) {
            mDrawingPropsBarQml->show();
            mDrawingPropsBarQml->raise();
        }
    }
    if (mShapesPaletteV2Qml && mToolController && mToolController->shapesVisible())
    {
        mShapesPaletteV2Qml->move(190, mContainer->height() - 52 - 330);
        if (isBoardMode) mShapesPaletteV2Qml->raise();
    }

    // Hide old palettes (replaced by QML V2)

    // Log final positions once container is big enough
    static bool loggedResize = false;
    if (!loggedResize && mContainer->width() > 200) {
        loggedResize = true;
        QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "\n=== containerResized FINAL ===\n";
            out << "Container: " << mContainer->width() << "x" << mContainer->height() << "\n";
            out << "StylusPalette: " << mStylusPaletteQml->x() << "," << mStylusPaletteQml->y()
                << " " << mStylusPaletteQml->width() << "x" << mStylusPaletteQml->height()
                << " vis=" << mStylusPaletteQml->isVisible() << "\n";
            out << "TopBar: " << mTopBarQml->x() << "," << mTopBarQml->y()
                << " " << mTopBarQml->width() << "x" << mTopBarQml->height()
                << " vis=" << mTopBarQml->isVisible() << "\n";
            out << "PageNav: " << mPageNavQml->x() << "," << mPageNavQml->y()
                << " " << mPageNavQml->width() << "x" << mPageNavQml->height()
                << " vis=" << mPageNavQml->isVisible() << "\n";
            out << "PropsBar: " << mDrawingPropsBarQml->x() << "," << mDrawingPropsBarQml->y()
                << " " << mDrawingPropsBarQml->width() << "x" << mDrawingPropsBarQml->height()
                << " vis=" << mDrawingPropsBarQml->isVisible() << "\n";
            out << "================================\n";
            logFile.close();
        }
    }

    if (isFirstResized && mKeyboardPalette && mKeyboardPalette->parent() == UBApplication::boardController->controlContainer())
    {
        isFirstResized = false;
        mKeyboardPalette->move(userLeft + (userWidth - mKeyboardPalette->width())/2,
                               userTop + (userHeight - mKeyboardPalette->height())/2);
        mKeyboardPalette->adjustSizeAndPosition();
    }

    {
    }

    {
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

    if (activeScene)
    {
        // Hide eraser when hovering QML palettes — covered by QML mouse areas
    }
}


// Issue 1684 - CFA - 20131120
void UBBoardPaletteManager::toggleImageBackgroundPalette(bool checked, bool isDefault)
{
    mImageBackgroundPalette->setVisible(checked);
    UBApplication::boardController->selectedDocument()->setHasDefaultImageBackground(isDefault);
    if (checked)
    {
        mImageBackgroundPalette->adjustSizeAndPosition();
    }
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
    mPaletteMode = newMode;

    // Dock palettes are disabled (QML V2 replaces them) — skip switchMode to prevent
    // re-adding tabs and re-showing the tab palette.

    if (newMode != eUBDockPaletteWidget_BOARD)
    {
        if (mKeyboardPalette)
            mKeyboardPalette->savePos();
        if (mAddItemPalette)
            mAddItemPalette->savePos();
    }
    else
    {
        if (mKeyboardPalette)
            mKeyboardPalette->restorePos();
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
                // Dock palettes permanently hidden — QML V2 replaces them

                // Restore QML palettes when returning from desktop/document mode
                if (mStylusPaletteQml)
                    mStylusPaletteQml->show();
                if (mTopBarQml)
                    mTopBarQml->show();
                if (mPageNavQml)
                    mPageNavQml->show();
                if (mDrawingPropsBarQml && mToolController && mToolController->showDrawingProps())
                    mDrawingPropsBarQml->show();

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

#ifdef Q_OS_WIN
                // Disabled: dock palettes replaced by QML V2
#endif

                if( !isInit )
                    containerResized();
                else
                {
                    // At startup, ensure palettes are sized to container
                }
                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        case eUBDockPaletteWidget_DESKTOP:
            {
                // Hide all QML V2 palettes — they are parented to mContainer (board view)
                // which gets hidden in desktop mode. The desktop overlay has its own
                // UBDesktopPalette for tool control.
                if (mStylusPaletteQml)
                    mStylusPaletteQml->hide();
                if (mTopBarQml)
                    mTopBarQml->hide();
                if (mPageNavQml)
                    mPageNavQml->hide();
                if (mDrawingPropsBarQml)
                    mDrawingPropsBarQml->hide();
                if (mShapesPaletteV2Qml)
                    mShapesPaletteV2Qml->hide();

                mAddItemPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                // Dock palettes permanently hidden — QML V2 replaces them

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

#ifdef Q_OS_WIN
                // Disabled: dock palettes replaced by QML V2
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
                // Dock palettes permanently hidden — QML V2 replaces them
#endif

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
                // Hide QML V2 palettes — document view has its own toolbar
                if (mStylusPaletteQml)
                    mStylusPaletteQml->hide();
                if (mTopBarQml)
                    mTopBarQml->hide();
                if (mPageNavQml)
                    mPageNavQml->hide();
                if (mDrawingPropsBarQml)
                    mDrawingPropsBarQml->hide();
                if (mShapesPaletteV2Qml)
                    mShapesPaletteV2Qml->hide();

                // Dock palettes permanently hidden — QML V2 replaces them
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
                // Dock palettes permanently hidden — QML V2 replaces them
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

void UBBoardPaletteManager::showVirtualKeyboard(bool show)
{
    if (mKeyboardPalette)
        mKeyboardPalette->setVisible(show);
}

void UBBoardPaletteManager::changeStylusPaletteOrientation(QVariant var)
{
    bool bVertical = var.toBool();
    bool bVisible = mStylusPaletteQml ? mStylusPaletteQml->isVisible() : true;

    // Legacy palette removed — only update QML palette orientation

    // Update QML palette orientation
    {
        int btnSize = 40;
        int numTools = 14;
        int numSeps = 2;
        int sepWidth = 1 + 12;
        int spacing = 2;
        int padding = 6;
        int contentLen = numTools * btnSize + (numTools - 1) * spacing + numSeps * sepWidth + padding * 2;
        int thickness = btnSize + padding * 2;

        if (mStylusPaletteQml)
        {
            if (bVertical)
                mStylusPaletteQml->setFixedSize(thickness, contentLen);
            else
                mStylusPaletteQml->setFixedSize(contentLen, thickness);

            // Update rounded mask after size change
            {
                int w = mStylusPaletteQml->width();
                int h = mStylusPaletteQml->height();
                QPainterPath maskPath;
                maskPath.addRoundedRect(0, 0, w, h, 12, 12);
                mStylusPaletteQml->setMask(QRegion(maskPath.toFillPolygon().toPolygon()));
            }

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
}

void UBBoardPaletteManager::startDownloads()
{
    // Download widget removed — dock palettes are permanently hidden (QML V2)
    mDownloadInProgress = true;
}

void UBBoardPaletteManager::stopDownloads()
{
    mDownloadInProgress = false;
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
    if (mImageBackgroundPalette && mImageBackgroundPalette->isVisible())
        mImageBackgroundPalette->close();
}
