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


#include <QScreen>
#include <QGuiApplication>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QPainterPath>
#include <QWindow>

#include "UBDesktopAnnotationController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"

#include "web/UBWebController.h"

#include "gui/UBMainWindow.h"

#include "board/UBBoardView.h"
#include "controllers/UBToolController.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBSceneContext.h"

#include "qml/UBThemeManager.h"

#include "UBCustomCaptureWindow.h"
#include "UBWindowCapture.h"

#include "gui/UBKeyboardPalette.h"
#include "gui/UBResources.h"


UBDesktopAnnotationController::UBDesktopAnnotationController(QObject *parent)
        : QObject(parent)
        , mTransparentDrawingView(0)
        , mTransparentDrawingScene(0)
        , mToolbarQml(nullptr)
        , mIsFullyTransparent(false)
        , mBoardStylusTool(UBToolController::toolController()->stylusTool())
        , mDesktopStylusTool(UBToolController::toolController()->stylusTool())
{
    mSettings = UBSettings::settings();

    mTransparentDrawingView = new UBBoardView(UBApplication::boardController, static_cast<QWidget*>(0), false, true); // deleted in UBDesktopAnnotationController::destructor
    mTransparentDrawingView->setAttribute(Qt::WA_TranslucentBackground, true);
#ifdef Q_OS_MACOS
    mTransparentDrawingView->setAttribute(Qt::WA_MacNoShadow, true);
#endif
    mTransparentDrawingView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window);
    mTransparentDrawingView->setCacheMode(QGraphicsView::CacheNone);
    mTransparentDrawingView->resize(QGuiApplication::primaryScreen()->geometry().width(), QGuiApplication::primaryScreen()->geometry().height());

    mTransparentDrawingView->setMouseTracking(true);

    mTransparentDrawingView->setAcceptDrops(true);

    QString backgroundStyle = "QWidget {background-color: rgba(127, 127, 127, 0)}";
    mTransparentDrawingView->setStyleSheet(backgroundStyle);

    mTransparentDrawingScene = new UBGraphicsScene(0, false);
    // Initialize the scene context with the live drawing controller so that
    // inputDevicePress / inputDeviceMove / updateGroupButtonState do not
    // dereference a null drawingController.  (#135)
    {
        UBSceneContext ctx;
        ctx.drawingController = UBToolController::toolController();
        ctx.boardController = UBApplication::boardController;
        mTransparentDrawingScene->setSceneContext(ctx);
    }
    mTransparentDrawingView->setScene(mTransparentDrawingScene);
    mTransparentDrawingScene->setDrawingMode(true);

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        connect(UBApplication::boardController->paletteManager()->mKeyboardPalette, &UBKeyboardPalette::keyboardActivated,
                 mTransparentDrawingView, &UBBoardView::virtualKeyboardActivated);

#ifdef Q_OS_LINUX
        connect(UBApplication::boardController->paletteManager()->mKeyboardPalette, &UBKeyboardPalette::moved, this, [this]() { refreshMask(); });
        connect(UBApplication::mainWindow->actionVirtualKeyboard, &QAction::triggered, this, [this]() { refreshMask(); });
#endif
    }

    connect(mTransparentDrawingView, &UBBoardView::resized, this, [this]() { onTransparentWidgetResized(); });

    connect(UBToolController::toolController(), &UBToolController::stylusToolChanged, this, &UBDesktopAnnotationController::stylusToolChanged);

    connect(UBApplication::mainWindow->actionEraseDesktopAnnotations, &QAction::triggered, this, [this]() {
        if (mTransparentDrawingScene)
            mTransparentDrawingScene->clearContent(UBGraphicsScene::clearAnnotations);
    });

    // --- V2 QML Desktop Toolbar (issue #336) ---
    setupToolbar();
    // --- V2 QML Drawing Props Bar (color/width) in desktop mode (issue #351) ---
    setupPropsBar();
    // Show/hide + resize the props bar when the active tool changes.
    connect(UBToolController::toolController(), &UBToolController::activeToolChanged,
            this, [this]() { updatePropsBarVisibility(); });
}

UBDesktopAnnotationController::~UBDesktopAnnotationController()
{
    // Top-level windows with no QObject parent → delete explicitly.
    delete mToolbarQml;
    delete mPropsBarQml;
    delete mTransparentDrawingScene;
    delete mTransparentDrawingView;
}


/**
 * \brief Create the V2 QML desktop toolbar (DesktopToolbar.qml).
 *
 * TOP-LEVEL window (renders on Windows) tied to the overlay as a TRANSIENT
 * PARENT (reliable z-order above it + receives clicks).
 *
 * #336 saga, all diagnosed on the Windows VM via startup.log:
 *  - child of the translucent overlay (translucent OR opaque backing) → the QML
 *    content is NOT painted on Windows: a QQuickWidget child of a top-level
 *    *translucent* window does not composite its RHI backing. status=Ready,
 *    rootObject present, visible=1, correctly sized/positioned — yet blank.
 *  - plain separate top-level window → it paints, but the fullscreen overlay
 *    (another WindowStaysOnTopHint window, shown last) sits above it and eats the
 *    clicks; event->ignore() cannot hand a click to a different top-level window.
 *
 * This version: top-level (so it paints) + set the overlay window as the
 * toolbar's transient parent, which ties the toolbar's stacking to the overlay
 * so it stays above it and receives input. Positioned in global coordinates.
 * A rounded mask gives the rounded corners.
 */
void UBDesktopAnnotationController::setupToolbar()
{
    mToolbarQml = new QQuickWidget(nullptr);
    mToolbarQml->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    mToolbarQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mToolbarQml->setClearColor(Qt::transparent);
    mToolbarQml->setAttribute(Qt::WA_TranslucentBackground);
    mToolbarQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mToolbarQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mToolbarQml->rootContext()->setContextProperty("toolController", UBToolController::toolController());
    // The toolbar's capture / return-to-board buttons call slots on this controller.
    mToolbarQml->rootContext()->setContextProperty("desktopController", this);
    mToolbarQml->setSource(QUrl("qrc:/qml/DesktopToolbar.qml"));
    if (mToolbarQml->status() == QQuickWidget::Error)
        for (const auto& e : mToolbarQml->errors())
            qWarning() << "DesktopToolbar QML error:" << e.toString();

    // Size: 8 buttons + 2 separators (see DesktopToolbar.qml buttons array).
    const int btnSize = 40;
    const int numButtons = 8;
    const int numSeps = 2;
    const int sepWidth = 1 + 12; // separator + spacing margins
    const int spacing = 2;
    const int padding = 6;
    const int contentLen = numButtons * btnSize + (numButtons - 1) * spacing + numSeps * sepWidth + padding * 2;
    const int thickness = btnSize + padding * 2;

    mToolbarQml->setFixedSize(contentLen, thickness);

    // Rounded mask: clips corners and lets clicks through outside the rounded shape.
    {
        QPainterPath path;
        path.addRoundedRect(0, 0, contentLen, thickness, 12, 12);
        mToolbarQml->setMask(QRegion(path.toFillPolygon().toPolygon()));
    }

    positionToolbar();
}

/**
 * \brief Place the toolbar at the bottom-center of the screen (top-level → global coords).
 */
void UBDesktopAnnotationController::positionToolbar()
{
    if (!mToolbarQml || !mTransparentDrawingView)
        return;

    QScreen* screen = mTransparentDrawingView->screen();
    const QRect g = screen ? screen->geometry()
                           : QGuiApplication::primaryScreen()->geometry();
    const int posX = g.x() + (g.width() - mToolbarQml->width()) / 2;
    const int posY = g.y() + g.height() - mToolbarQml->height() - 40;
    mToolbarQml->move(qMax(g.x(), posX), posY);
}

/**
 * \brief Create the drawing props bar (color/width) for desktop mode (#351).
 *
 * Reuses the board's DrawingPropsBar.qml, hosted exactly like the toolbar
 * (top-level, transient-parented to the overlay). Shown above the toolbar when a
 * drawing tool is active (toolController.showDrawingProps).
 */
void UBDesktopAnnotationController::setupPropsBar()
{
    mPropsBarQml = new QQuickWidget(nullptr);
    mPropsBarQml->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    mPropsBarQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mPropsBarQml->setClearColor(Qt::transparent);
    mPropsBarQml->setAttribute(Qt::WA_TranslucentBackground);
    mPropsBarQml->setAttribute(Qt::WA_AlwaysStackOnTop);
    mPropsBarQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
    mPropsBarQml->rootContext()->setContextProperty("toolController", UBToolController::toolController());
    mPropsBarQml->setSource(QUrl("qrc:/qml/DrawingPropsBar.qml"));
    if (mPropsBarQml->status() == QQuickWidget::Error)
        for (const auto& e : mPropsBarQml->errors())
            qWarning() << "DesktopToolbar DrawingPropsBar QML error:" << e.toString();
    mPropsBarQml->hide();
}

/**
 * \brief Size + place the props bar centered above the toolbar (global coords).
 */
void UBDesktopAnnotationController::positionPropsBar()
{
    if (!mPropsBarQml || !mTransparentDrawingView || !mToolbarQml)
        return;

    // Same sizes as the board (UBBoardPaletteManager): 210 for the eraser
    // (widths + actions), 280 otherwise (colors + widths).
    const bool isEraser =
        (UBToolController::toolController()->activeTool() == UBStylusTool::Eraser);
    const int barW = isEraser ? 210 : 280;
    const int barH = 48;
    mPropsBarQml->setFixedSize(barW, barH);
    {
        QPainterPath path;
        path.addRoundedRect(0, 0, barW, barH, 12, 12);
        mPropsBarQml->setMask(QRegion(path.toFillPolygon().toPolygon()));
    }

    QScreen* screen = mTransparentDrawingView->screen();
    const QRect g = screen ? screen->geometry()
                           : QGuiApplication::primaryScreen()->geometry();
    const int posX = g.x() + (g.width() - barW) / 2;
    const int posY = mToolbarQml->y() - barH - 8; // just above the toolbar
    mPropsBarQml->move(qMax(g.x(), posX), posY);
}

/**
 * \brief Show/hide + reposition the props bar based on the active tool.
 */
void UBDesktopAnnotationController::updatePropsBarVisibility()
{
    if (!mPropsBarQml)
        return;
    // Only while the desktop overlay is up.
    const bool overlayUp = mTransparentDrawingView && mTransparentDrawingView->isVisible();
    if (overlayUp && UBToolController::toolController()->showDrawingProps()) {
        if (mTransparentDrawingView->windowHandle()) {
            mPropsBarQml->winId();
            if (mPropsBarQml->windowHandle())
                mPropsBarQml->windowHandle()->setTransientParent(mTransparentDrawingView->windowHandle());
        }
        positionPropsBar();
        mPropsBarQml->show();
        mPropsBarQml->raise();
    } else {
        mPropsBarQml->hide();
    }
}

void UBDesktopAnnotationController::showToolbar()
{
    if (!mToolbarQml)
        return;
    // Tie the toolbar's stacking to the overlay: as a transient parent, the
    // toolbar stays above the overlay and receives input, while remaining its own
    // native (rendering) window. Must be done once both window handles exist.
    if (mTransparentDrawingView && mTransparentDrawingView->windowHandle()) {
        mToolbarQml->winId(); // force native window creation
        if (mToolbarQml->windowHandle())
            mToolbarQml->windowHandle()->setTransientParent(mTransparentDrawingView->windowHandle());
    }
    positionToolbar();
    mToolbarQml->show();
    mToolbarQml->raise();
    updatePropsBarVisibility();
}

void UBDesktopAnnotationController::hideToolbarForCapture()
{
    if (mToolbarQml)
        mToolbarQml->hide();
    if (mPropsBarQml)
        mPropsBarQml->hide();
    qApp->processEvents();
}

void UBDesktopAnnotationController::restoreToolbarAfterCapture()
{
    if (mToolbarQml) {
        mToolbarQml->show();
        mToolbarQml->raise();
    }
    updatePropsBarVisibility();
}


QPainterPath UBDesktopAnnotationController::desktopPalettePath() const
{
    QPainterPath result;
    if (mToolbarQml && mToolbarQml->isVisible())
        result.addRect(mToolbarQml->geometry());
    return result;
}

UBBoardView* UBDesktopAnnotationController::drawingView()
{
    return mTransparentDrawingView;
}


void UBDesktopAnnotationController::showWindow()
{
    showToolbar();

    updateBackground();

    mBoardStylusTool = UBToolController::toolController()->stylusTool();

    UBToolController::toolController()->setStylusTool(mDesktopStylusTool);

#ifdef Q_OS_WIN
    // #241: try REAL transparency (show the live desktop through the overlay)
    // instead of painting a frozen desktop screenshot. The old capture approach
    // pasted a static QPixmap as the scene backgroundBrush — but a tiled texture
    // brush in scene coordinates never lined up, and (more importantly)
    // UBBoardView::drawBackground filled the viewport opaque white before it,
    // so the user only saw white. Now UBBoardView::drawBackground defers to
    // QGraphicsView::drawBackground for the desktop overlay, and a transparent
    // scene brush + WA_TranslucentBackground lets the real desktop show through.
    mTransparentDrawingView->setStyleSheet(QString());
    mTransparentDrawingView->setAttribute(Qt::WA_TranslucentBackground, true);
    mTransparentDrawingView->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    mTransparentDrawingView->viewport()->setAutoFillBackground(false);
    mTransparentDrawingScene->setBackgroundBrush(QBrush(Qt::transparent));
    mTransparentDrawingView->showFullScreen();
#elif defined(Q_OS_LINUX)
    // this is necessary to avoid unity to hide the panels
    mTransparentDrawingView->show();
#else
    mTransparentDrawingView->showFullScreen();
#endif
    UBPlatformUtils::setDesktopMode(true);

    // Keep the toolbar on top of the overlay after the view is shown.
    showToolbar();

#ifdef Q_OS_LINUX
    updateMask(true);
#endif
}


void UBDesktopAnnotationController::close()
{
    // NOOP
}


void UBDesktopAnnotationController::stylusToolChanged(int tool)
{
    Q_UNUSED(tool);
    updateBackground();
}


void UBDesktopAnnotationController::updateBackground()
{
    QBrush newBrush;

    if (mIsFullyTransparent
            || UBToolController::toolController()->stylusTool() == UBStylusTool::Selector)
    {
        newBrush = QBrush(Qt::transparent);
#ifdef Q_OS_LINUX
        updateMask(true);
#endif
    }
    else
    {
#if defined(Q_OS_MACOS)
        newBrush = QBrush(QColor(127, 127, 127, 15));
#else
        newBrush = QBrush(QColor(127, 127, 127, 1));
#endif
#ifdef Q_OS_LINUX
        updateMask(false);
#endif
    }

    if (mTransparentDrawingScene && mTransparentDrawingScene->backgroundBrush() != newBrush)
        mTransparentDrawingScene->setBackgroundBrush(newBrush);
}


void UBDesktopAnnotationController::hideWindow()
{
    if (mToolbarQml)
        mToolbarQml->hide();
    if (mPropsBarQml)
        mPropsBarQml->hide();

    if (mTransparentDrawingView)
        mTransparentDrawingView->hide();

    mDesktopStylusTool = UBToolController::toolController()->stylusTool();
    UBToolController::toolController()->setStylusTool(mBoardStylusTool);
}


void UBDesktopAnnotationController::goToUniboard()
{
    hideWindow();

    UBPlatformUtils::setDesktopMode(false);
    UBToolController::toolController()->setInDesktopMode(false);

    emit restoreUniboard();
}


void UBDesktopAnnotationController::customCapture()
{
    mIsFullyTransparent = true;
    updateBackground();

    hideToolbarForCapture();
    UBCustomCaptureWindow customCaptureWindow(mTransparentDrawingView);
    // need to show the window before execute it to avoid some glitch on windows.

#ifndef Q_OS_WIN // Working only without this call on win32 desktop mode
    customCaptureWindow.show();
#endif

    if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
    {
        QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
        emit imageCaptured(selectedPixmap, false);
    }

    restoreToolbarAfterCapture();

    mIsFullyTransparent = false;
    updateBackground();
}


void UBDesktopAnnotationController::windowCapture()
{
    mIsFullyTransparent = true;
    updateBackground();

    hideToolbarForCapture();

    UBWindowCapture util(this);

    if (util.execute() == QDialog::Accepted)
    {
        QPixmap windowPixmap = util.getCapturedWindow();

        // on Mac OS X we can only know that user cancel the operatiion by checking is the image is null
        // because the screencapture utility always return code 0 event if user cancel the application
        if (!windowPixmap.isNull())
        {
            emit imageCaptured(windowPixmap, false);
        }
    }

    restoreToolbarAfterCapture();

    mIsFullyTransparent = false;

    updateBackground();
}


void UBDesktopAnnotationController::screenCapture()
{
    mIsFullyTransparent = true;
    updateBackground();

    hideToolbarForCapture();

    QPixmap originalPixmap = getScreenPixmap();

    restoreToolbarAfterCapture();

    emit imageCaptured(originalPixmap, false);

    mIsFullyTransparent = false;

    updateBackground();
}


QPixmap UBDesktopAnnotationController::getScreenPixmap()
{
    // Capture the screen where the transparent drawing view is displayed
    QScreen *screen = nullptr;
    if (mTransparentDrawingView && mTransparentDrawingView->screen())
        screen = mTransparentDrawingView->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    const QRect screenRect = screen->geometry();
    // processEvents() removed: it caused re-entrancy during Desktop mode transition,
    // dispatching stale mouse events to the board view and crashing in viewportEvent (#135)
    QPixmap grabbed = screen->grabWindow(0, screenRect.x(), screenRect.y(), screenRect.width(), screenRect.height());

    return grabbed;
}


void UBDesktopAnnotationController::updateShowHideState(bool pEnabled)
{
    Q_UNUSED(pEnabled);
    // Mirroring on/off is now driven from UBApplicationController::mirroringEnabled
    // directly; the legacy show/hide eye toggle was removed with UBDesktopPalette (#336).
}


void UBDesktopAnnotationController::screenLayoutChanged()
{
    // The legacy show/hide-on-secondary-screen button lived on UBDesktopPalette,
    // removed in #336. Nothing to update on the toolbar here.
}


void UBDesktopAnnotationController::TransparentWidgetResized()
{
    onTransparentWidgetResized();
}

/**
 * \brief Re-center the QML toolbar when the transparent overlay is resized.
 */
void UBDesktopAnnotationController::onTransparentWidgetResized()
{
    positionToolbar();
    updatePropsBarVisibility(); // repositions the props bar above the toolbar
}

void UBDesktopAnnotationController::updateMask(bool bTransparent)
{
    if(bTransparent)
    {
        // Here we have to generate a new mask This method is certainly resource
        // consuming but for the moment this is the only solution that I found.
        mMask = QPixmap(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mMask.fill(Qt::transparent);

        QPainter p;

        p.begin(&mMask);

        p.setPen(Qt::red);
        p.setBrush(QBrush(Qt::red));

        // Toolbar region (so it stays clickable through the mask)
        if(mToolbarQml && mToolbarQml->isVisible())
        {
            p.drawRect(mToolbarQml->geometry());
        }
        if(UBApplication::boardController->paletteManager()->mKeyboardPalette->isVisible())
        {
            p.drawRect(UBApplication::boardController->paletteManager()->mKeyboardPalette->geometry().x(), UBApplication::boardController->paletteManager()->mKeyboardPalette->geometry().y(),
                       UBApplication::boardController->paletteManager()->mKeyboardPalette->width(), UBApplication::boardController->paletteManager()->mKeyboardPalette->height());
        }

#ifdef Q_OS_LINUX
        //Rquiered only for compiz wm
        //TODO. Window manager detection screen

        if (UBApplication::boardController->paletteManager()->addItemPalette()->isVisible()) {
            p.drawRect(UBApplication::boardController->paletteManager()->addItemPalette()->geometry());
        }

#endif

        p.end();

        // Then we add the annotations. We create another painter because we need to
        // apply transformations on it for coordinates matching
        QPainter annotationPainter;

        QTransform trans;
        trans.translate(mTransparentDrawingView->width()/2, mTransparentDrawingView->height()/2);

        annotationPainter.begin(&mMask);
        annotationPainter.setPen(Qt::red);
        annotationPainter.setBrush(Qt::red);

        annotationPainter.setTransform(trans);

        QList<QGraphicsItem*> allItems = mTransparentDrawingScene->items();

        for(int i = 0; i < allItems.size(); i++)
        {
            QGraphicsItem* pCrntItem = allItems.at(i);

            if(pCrntItem->isVisible() && pCrntItem->type() == UBGraphicsPolygonItem::Type)
            {
                QPainterPath crntPath = pCrntItem->shape();
                QRectF rect = crntPath.boundingRect();

                annotationPainter.drawRect(rect);
            }
        }

        annotationPainter.end();

        mTransparentDrawingView->setMask(mMask.mask());
    }
    else
    {
        // Remove the mask
        QPixmap noMask(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mTransparentDrawingView->setMask(noMask.mask());
    }
}

void UBDesktopAnnotationController::refreshMask()
{
    if (mTransparentDrawingScene && mTransparentDrawingView->isVisible()) {
        if(mIsFullyTransparent
                || UBToolController::toolController()->stylusTool() == UBStylusTool::Selector
                //Needed to work correctly when another actions on stylus are checked
                || UBToolController::toolController()->stylusTool() == UBStylusTool::Eraser
                || UBToolController::toolController()->stylusTool() == UBStylusTool::Pointer
                || UBToolController::toolController()->stylusTool() == UBStylusTool::Pen
                || UBToolController::toolController()->stylusTool() == UBStylusTool::Marker)
        {
            updateMask(true);
        }
    }
}
