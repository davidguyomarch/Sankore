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

#include "UBApplicationController.h"
#include <QJSEngine>
#include <QJSValue>
#include <QDesktopServices>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QElapsedTimer>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBVersion.h"

// --- Temporary diagnostics for #259 (brief display flash when opening Documents).
//     Appends to startup.log next to the executable. Remove once the bug is fixed.
static void ubDocFlashDiag(const QString &line)
{
    QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
    if (logFile.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&logFile);
        out << "[DOCFLASH] " << line << "\n";
        logFile.close();
    }
}

#include "core/UBApplication.h"
#include "core/UBPresentationController.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDocumentManager.h"
#include "core/UBDisplayManager.h"

#include "board/UBBoardView.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "controllers/UBToolController.h"


#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "domain/UBGraphicsWidgetItem.h"

#include "desktop/UBDesktopAnnotationController.h"

#include "web/UBWebController.h"

#include "gui/UBScreenMirror.h"
#include "gui/UBMainWindow.h"

#include "domain/UBGraphicsPixmapItem.h"


#include "network/UBNetworkAccessManager.h"

#include "ui_mainWindow.h"

#ifdef Q_OS_MACOS
#include <Carbon/Carbon.h>
#endif


UBApplicationController::UBApplicationController(UBBoardView *pControlView,
                                                 UBBoardView *pDisplayView,
                                                 UBMainWindow* pMainWindow,
                                                 QObject* parent)
    : QObject(parent)
    , mMainWindow(pMainWindow)
    , mControlView(pControlView)
    , mDisplayView(pDisplayView)
    , mMirror(0)
    , mMainMode(Board)
    , mDisplayManager(0)
    , mAutomaticCheckForUpdates(false)
    , mCheckingForUpdates(false)
    , mIsShowingDesktop(false)
    
{
    mSettings = UBSettings::settings();
    mDisplayManager = new UBDisplayManager(this);

    mUninoteController = new UBDesktopAnnotationController(this);

    // #399 (ADR-0008 D2) brick 2: shadow presentation-state controller.
    mPresentationController = new UBPresentationController(this);

    connect(mDisplayManager, &UBDisplayManager::screenLayoutChanged, this, &UBApplicationController::screenLayoutChanged);
    connect(mDisplayManager, &UBDisplayManager::screenLayoutChanged, mUninoteController, &UBDesktopAnnotationController::screenLayoutChanged);
    connect(mDisplayManager, &UBDisplayManager::screenLayoutChanged, UBApplication::webController, &UBWebController::screenLayoutChanged);

    connect(mUninoteController, &UBDesktopAnnotationController::imageCaptured, this,
            [this](const QPixmap &pPixmap, bool pageMode) { addCapturedPixmap(pPixmap, pageMode); });
    connect(mUninoteController, &UBDesktopAnnotationController::restoreUniboard, this, &UBApplicationController::hideDesktop);

    for(int i = 0; i < mDisplayManager->numPreviousViews(); i++)
    {
        UBBoardView *previousView = new UBBoardView(UBApplication::boardController, UBItemLayerType::FixedBackground, UBItemLayerType::Tool, 0);
        previousView->setInteractive(false);
        mPreviousViews.append(previousView);
    }

    mBlackScene = new UBGraphicsScene(0); // deleted by UBApplicationController::destructor
    mBlackScene->setBackground(true, false);

    if (mDisplayManager->numScreens() >= 2)
    {
        mMirror = new UBScreenMirror();
    }

    connect(UBApplication::webController, &UBWebController::imageCaptured,
            this, &UBApplicationController::addCapturedPixmap);

    networkAccessManager = new QNetworkAccessManager (this);

#ifdef Q_OS_LINUX
    mMainWindow->setStyleSheet("QToolButton { font-size: 11px}");
#endif

}


UBApplicationController::~UBApplicationController()
{
    for (UBBoardView* view : mPreviousViews)
    {
        delete view;
    }

    delete mBlackScene;
    delete mMirror;
    
}


void UBApplicationController::initViewState(int horizontalPosition, int verticalPostition)
{
    mInitialHScroll = horizontalPosition;
    mInitialVScroll = verticalPostition;
}


void UBApplicationController::initScreenLayout(bool useMultiscreen)
{
    mDisplayManager->setControlWidget(mMainWindow);
    mDisplayManager->setDisplayWidget(mDisplayView);

    mDisplayManager->setPreviousDisplaysWidgets(mPreviousViews);

    mDisplayManager->setDesktopWidget(mUninoteController->drawingView());

    mDisplayManager->setUseMultiScreen(useMultiscreen);
    mDisplayManager->adjustScreens(-1);
}


void UBApplicationController::screenLayoutChanged()
{
    initViewState(mControlView->horizontalScrollBar()->value(),
            mControlView->verticalScrollBar()->value());

    adaptToolBar();

    adjustDisplayView();

    if (mDisplayManager->hasDisplay())
    {
        UBApplication::boardController->setBoxing(mDisplayView->geometry());
    }
    else
    {
       UBApplication::boardController->setBoxing(QRect());
    }

    adjustPreviousViews(0, 0);
}


void UBApplicationController::adaptToolBar()
{
    bool highResolution = mMainWindow->width() > 1024;

    mMainWindow->actionClearPage->setVisible(Board == mMainMode && highResolution);
    mMainWindow->actionBoard->setVisible(Board != mMainMode || highResolution);
    mMainWindow->actionDocument->setVisible(Document != mMainMode || highResolution);
    mMainWindow->actionWeb->setVisible(Internet != mMainMode || highResolution);
    mMainWindow->boardToolBar->setIconSize(QSize(highResolution ? 48 : 42, mMainWindow->boardToolBar->iconSize().height()));

    mMainWindow->actionBoard->setEnabled(mMainMode != Board);
    mMainWindow->actionWeb->setEnabled(mMainMode != Internet);
    mMainWindow->actionDocument->setEnabled(mMainMode != Document);

    if (Document == mMainMode)
    {
        connect(qApp, &QApplication::focusChanged, UBApplication::documentController, &UBDocumentController::focusChanged);
    }
    else
    {
        disconnect(qApp, &QApplication::focusChanged, UBApplication::documentController, &UBDocumentController::focusChanged);
        if (Board == mMainMode)
            mMainWindow->actionDuplicate->setEnabled(true);
    }

    UBApplication::boardController->setToolbarTexts();

    UBApplication::webController->adaptToolBar();

}


void UBApplicationController::adjustDisplayView()
{
    if (mDisplayView)
    {
        qreal systemDisplayViewScaleFactor = 1.0;

        QSize pageSize = UBApplication::boardController->activeScene()->nominalSize();
        QSize displaySize = mDisplayView->size();

        qreal hFactor = ((qreal)displaySize.width()) / ((qreal)pageSize.width());
        qreal vFactor = ((qreal)displaySize.height()) / ((qreal)pageSize.height());

        systemDisplayViewScaleFactor = qMin(hFactor, vFactor);

        QTransform tr;
        qreal scaleFactor = systemDisplayViewScaleFactor * UBApplication::boardController->currentZoom();

        tr.scale(scaleFactor, scaleFactor);

        QRect rect = mControlView->rect();
        QPoint center(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

        QTransform recentTransform = mDisplayView->transform();

        if (recentTransform != tr)
        {
            mDisplayView->setTransform(tr);
        }

        mDisplayView->centerOn(UBApplication::boardController->activeScene()->lastCenter());// Issue 1598/1605 - CFA - 20131028
    }
}


void UBApplicationController::adjustPreviousViews(int pActiveSceneIndex, UBDocumentProxy *pActiveDocument)
{
    int viewIndex = pActiveSceneIndex;

    for (UBBoardView* previousView : mPreviousViews)
    {
        if (viewIndex > 0)
        {
            viewIndex--;

            UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pActiveDocument, viewIndex);

            if (scene)
            {
                previousView->setScene(scene);

                qreal ratio = ((qreal)previousView->geometry().width()) / ((qreal)previousView->geometry().height());
                QRectF sceneRect = scene->normalizedSceneRect(ratio);
                qreal scaleRatio = previousView->geometry().width() / sceneRect.width();

                previousView->resetTransform();

                previousView->scale(scaleRatio, scaleRatio);

                previousView->centerOn(sceneRect.center());
            }
        }
        else
        {
            previousView->setScene(mBlackScene);
        }
    }
}


void UBApplicationController::blackout()
{
    mDisplayManager->blackout();
}


void UBApplicationController::addCapturedPixmap(const QPixmap &pPixmap, bool pageMode, const QUrl& sourceUrl)
{
    if (!pPixmap.isNull())
    {
        qreal sf = UBApplication::boardController->systemScaleFactor();
        qreal scaledWidth = ((qreal)pPixmap.width()) / sf;
        qreal scaledHeight = ((qreal)pPixmap.height()) / sf;

        QSize pageNominalSize = UBApplication::boardController->activeScene()->nominalSize();

        int newWidth = qMin((int)scaledWidth, pageNominalSize.width());
        int newHeight = qMin((int)scaledHeight, pageNominalSize.height());

        if (pageMode)
        {
            newHeight = pPixmap.height();
        }

        QSizeF scaledSize(scaledWidth, scaledHeight);
        scaledSize.scale(newWidth, newHeight, Qt::KeepAspectRatio);

        qreal scaleFactor = qMin(scaledSize.width() / (qreal)pPixmap.width(), scaledSize.height() / (qreal)pPixmap.height());

        QPointF pos(0.0, 0.0);

        if (pageMode)
        {
            pos.setY(pageNominalSize.height() / -2  + scaledSize.height() / 2);
        }

        UBApplication::boardController->paletteManager()->addItem(pPixmap, pos, scaleFactor, sourceUrl);
    }
}


void UBApplicationController::addCapturedEmbedCode(const QString& embedCode)
{
    if (!embedCode.isEmpty())
    {
        showBoard();

        const QString userWidgetPath = mSettings->userInteractiveDirectory() + "/" + tr("Web"); // TODO UB 4.x synch with w3cWidget
        QDir userWidgetDir(userWidgetPath);

        int width = 300;
        int height = 150;

        QString widgetPath = UBGraphicsW3CWidgetItem::createHtmlWrapperInDir(embedCode, userWidgetDir,
                QSize(width, height), UBStringUtils::toCanonicalUuid(QUuid::createUuid()));

        if (widgetPath.length() > 0)
            UBApplication::boardController->downloadURL(QUrl::fromLocalFile(widgetPath));
    }
}


// ===========================================================================
// #399 (ADR-0008 D2, brick 3): single guarded entry point for mode transitions.
//
// Every public transition (showBoard / showInternet / showDocument /
// showDesktop / hideDesktop) is a thin wrapper that runs under a re-entrancy
// guard, then dispatches to the real (unguarded) do*() body. The transitions
// drive the palette slots via UBPresentationController::stateChanged, whose
// handlers could request ANOTHER transition mid-flight; the guard makes each
// user action yield
// exactly one transition (killing the measured triple hideWindow on return,
// the double props bar, and the background-menu desync). Internal continuations
// (hideDesktop -> board/web/doc, internet -> desktop) call do*() directly and
// are therefore NOT blocked.
// ===========================================================================

void UBApplicationController::showBoard()
{
    if (mInModeTransition)
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[STATE] (reentrant showBoard ignored)\n";
        return;
    }
    mInModeTransition = true;
    doShowBoard();
    mInModeTransition = false;
}

void UBApplicationController::showInternet()
{
    if (mInModeTransition)
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[STATE] (reentrant showInternet ignored)\n";
        return;
    }
    mInModeTransition = true;
    doShowInternet();
    mInModeTransition = false;
}

void UBApplicationController::showDocument()
{
    if (mInModeTransition)
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[STATE] (reentrant showDocument ignored)\n";
        return;
    }
    mInModeTransition = true;
    doShowDocument();
    mInModeTransition = false;
}

void UBApplicationController::showDesktop(bool dontSwitchFrontProcess)
{
    if (mInModeTransition)
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[STATE] (reentrant showDesktop ignored)\n";
        return;
    }
    mInModeTransition = true;
    if (useSingleWindowDesktop())
        doShowDesktopSingleWindow();
    else
        doShowDesktop(dontSwitchFrontProcess);
    mInModeTransition = false;
}

void UBApplicationController::hideDesktop()
{
    if (mInModeTransition)
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[STATE] (reentrant hideDesktop ignored)\n";
        return;
    }
    mInModeTransition = true;
    if (useSingleWindowDesktop())
        doHideDesktopSingleWindow();
    else
        doHideDesktop();
    mInModeTransition = false;
}

bool UBApplicationController::useSingleWindowDesktop()
{
    // #523 / ADR-0013 (bricks 2+3): the new single-window Bureau is the default
    // path. SANKORE_SINGLE_WINDOW_DESKTOP=0 falls back to the legacy translucent
    // overlay (kept until the single-window path is VM-validated; removed in
    // brick 3b). Read once.
    static const bool single = (qgetenv("SANKORE_SINGLE_WINDOW_DESKTOP") != "0");
    return single;
}

void UBApplicationController::doShowDesktopSingleWindow()
{
    // #523 / ADR-0013 (bricks 2+3): enter Bureau WITHOUT an overlay. The main
    // window and the board view stay the presenting surface (never translucent,
    // #397/#413); we just make the current page see-through and paint a captured
    // desktop as its background. Contrast with doShowDesktop(), which hides the
    // main window and shows mTransparentDrawingView.
    //
    // #523 / ADR-0013 open question: grab the desktop BEHIND Sankoré (the service
    // hides the main window for the grab) so the captured background does not
    // contain Sankoré's own UI (VM-reported). Done BEFORE flipping the page
    // see-through so the capture is ready when the board view repaints.
    UBGraphicsScene* scene = UBApplication::boardController
                                 ? UBApplication::boardController->activeScene() : nullptr;
    {
        static int sEnterCount = 0;
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[DESKTOP] doShowDesktopSingleWindow BEGIN #"
                            << (++sEnterCount) << " scene=" << (scene ? 1 : 0)
                            << " grabInFlight=" << mDesktopGrabInFlight << "\n";
    }
    // #523: the grab hides/re-shows Sankoré's windows, which emits activation
    // events; guard so the ApplicationActivate hook does not re-enter the grab
    // (the feedback loop that flickered the screen and crashed the VM).
    mDesktopGrabInFlight = true;
    if (scene && mUninoteController)
        mUninoteController->installDesktopCapture(scene, mMainWindow);
    mDesktopGrabInFlight = false;

    mIsShowingDesktop = true;
    // Drive the presentation state (Fond menu highlight, guards). The palette
    // manager keeps the board chrome visible in this state when single-window is
    // on (see UBBoardPaletteManager::changeMode).
    mPresentationController->setState(UBPresentationController::State::DesktopAnnotation);

    // Make the current page see-through (transient, not persisted) so the board
    // view paints the capture instead of an opaque fill — same renderer flip the
    // overlay used, but here it is the MAIN board view that presents it.
    if (scene)
        scene->setDrawingMode(true);

    UBPlatformUtils::setDesktopMode(true);

    // Keep the board view the presenting surface: do NOT hide the main window,
    // do NOT disable the control view, do NOT show the overlay.
    if (UBApplication::boardController && UBApplication::boardController->controlView())
        UBApplication::boardController->controlView()->viewport()->update();
}

void UBApplicationController::doHideDesktopSingleWindow()
{
    // #523 / ADR-0013 (bricks 2+3): leave single-window Bureau. Clear the
    // transient capture, restore the page's opaque look, and go back to the
    // Board presentation state. No overlay to tear down.
    UBGraphicsScene* scene = UBApplication::boardController
                                 ? UBApplication::boardController->activeScene() : nullptr;
    if (scene)
    {
        scene->setDrawingMode(false);
        if (mUninoteController)
            mUninoteController->clearDesktopCapture(scene);
    }

    UBPlatformUtils::setDesktopMode(false);
    mIsShowingDesktop = false;

    // Back to the board presentation state (restores the TopBar highlight etc.).
    mPresentationController->setState(UBPresentationController::State::Board);

    if (UBApplication::boardController && UBApplication::boardController->controlView())
        UBApplication::boardController->controlView()->viewport()->update();
}

void UBApplicationController::refreshDesktopCaptureIfActive()
{
    // #523 / ADR-0013 (brick 2): on focus-return (alt-tab), re-grab the desktop
    // into the current see-through page so the frozen background follows what the
    // teacher changed behind Sankoré. Only in single-window Bureau, only while a
    // see-through page is shown.
    //
    // #523 FEEDBACK-LOOP GUARD: the grab itself hides and re-shows Sankoré's
    // windows, each of which raises a QEvent::ApplicationActivate — which calls
    // THIS method again. Bail out when a grab is already in flight (or a mode
    // transition is running), otherwise the hide/show cycles indefinitely
    // (observed as heavy flicker + a VM crash).
    if (mDesktopGrabInFlight || mInModeTransition)
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[DESKTOP] refresh ignored (grabInFlight="
                            << mDesktopGrabInFlight << " inTransition="
                            << mInModeTransition << ")\n";
        return;
    }
    if (!useSingleWindowDesktop() || !mIsShowingDesktop)
        return;
    UBGraphicsScene* scene = UBApplication::boardController
                                 ? UBApplication::boardController->activeScene() : nullptr;
    if (scene && scene->isDrawingMode() && mUninoteController)
    {
        {
            QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
            if (f.open(QIODevice::Append | QIODevice::Text))
                QTextStream(&f) << "[DESKTOP] refresh (focus-return) grabbing\n";
        }
        mDesktopGrabInFlight = true;
        mUninoteController->installDesktopCapture(scene, mMainWindow);
        mDesktopGrabInFlight = false;
    }
}


void UBApplicationController::doShowBoard()
{
    mMainWindow->webToolBar->hide();
    mMainWindow->documentToolBar->hide();
    mMainWindow->boardToolBar->hide(); // Hidden: replaced by QML TopBar

//    if (mMainMode == Document)
//    {
//        int selectedSceneIndex = UBApplication::documentController->getSelectedItemIndex();
//        if (selectedSceneIndex != -1)
//        {
//            UBApplication::boardController->setActiveDocumentScene(UBApplication::documentController->selectedDocument(), selectedSceneIndex, true);
//        }
//    }

    mMainMode = Board;
    mPresentationController->setState(UBPresentationController::State::Board); // #399 shadow

    adaptToolBar();

    mirroringEnabled(false);

    mMainWindow->switchToBoardWidget();

    if (UBApplication::boardController)
    {
        UBApplication::boardController->activeScene()->setRenderingContext(UBGraphicsScene::Screen);
        UBApplication::boardController->show();
    }

    mIsShowingDesktop = false;
    UBPlatformUtils::setDesktopMode(false);

    mUninoteController->hideWindow();

    mMainWindow->show();

    // #399 (brick 4c): legacy mainModeChanged/desktopMode emits removed — the
    // palette manager is driven by UBPresentationController::stateChanged
    // (setState above). No subscriber remains for the old signals.

    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBToolController::toolController()->stylusTool();
    // Legacy code forced Selector here — disabled for QML V2 UI which sets Pen at startup
    // if (UBStylusTool::Selector != currentTool)
    //     UBToolController::toolController()->setStylusTool(UBStylusTool::Selector);

    UBApplication::boardController->freezeW3CWidgets(false);
    UBApplication::boardController->activeScene()->updateGroupButtonState();
}


void UBApplicationController::doShowInternet()
{

    if (UBApplication::boardController)
    {
        UBApplication::boardController->activeScene()->setRenderingContext(UBGraphicsScene::NonScreen);
        UBApplication::boardController->persistCurrentScene();
        UBApplication::boardController->hide();
    }

    if (mSettings->webUseExternalBrowser->get().toBool())
    {
        doShowDesktop(true); // #399: internal chain, bypass the re-entrancy guard
        UBApplication::webController->show(UBWebController::WebBrowser);
        // really no have emit mainModeChanged here ? potential problem with virtual keyboard ?
    }
    else
    {
        mMainWindow->boardToolBar->hide();
        mMainWindow->documentToolBar->hide();
        mMainWindow->webToolBar->show();

        mMainMode = Internet;
        mPresentationController->setState(UBPresentationController::State::Web); // #399 shadow

        adaptToolBar();

        mMainWindow->show();
        mUninoteController->hideWindow();

        UBApplication::webController->show(UBWebController::WebBrowser);
        // #399 (brick 4c): legacy mainModeChanged emit removed (see setState(Web)).
    }
}


void UBApplicationController::doShowDocument()
{
    mMainWindow->webToolBar->hide();
    mMainWindow->boardToolBar->hide();
    mMainWindow->documentToolBar->hide(); // Hidden: replaced by QML DocumentsTopBar

    mMainMode = Document;
    mPresentationController->setState(UBPresentationController::State::Documents); // #399 shadow

    adaptToolBar();

    mirroringEnabled(false);

    // --- Diagnostics #259: time each transition step so we can tell which one
    //     causes the brief flash (double switch / late hide / re-show). Elapsed
    //     ms are cumulative from the start of showDocument().
    QElapsedTimer docFlashTimer;
    docFlashTimer.start();
    ubDocFlashDiag(QString("showDocument BEGIN (t=0ms)"));

    // #259: avoid the brief flash of a half-built Documents view.
    // Populate the Documents view *before* switching the stacked layout to it,
    // so the switch happens only once the tree/thumbnails/top bar are ready.
    // While we build it, the board is still the visible page (no intermediate
    // paint of a partially-constructed Documents view).
    if (UBApplication::boardController
        && UBApplication::boardController->activeScene()
        && UBApplication::boardController->activeScene()->isModified())
    {
        UBApplication::boardController->persistCurrentScene();
        ubDocFlashDiag(QString("after persistCurrentScene t=%1ms").arg(docFlashTimer.elapsed()));
    }

    UBDocumentController *docCtrl = UBApplication::documentController;
    if (docCtrl) {
        docCtrl->show();
        ubDocFlashDiag(QString("after docCtrl->show t=%1ms").arg(docFlashTimer.elapsed()));
        if (docCtrl->firstSelectedTreeProxy()) {
            docCtrl->setDocument(docCtrl->firstSelectedTreeProxy(), true);
            ubDocFlashDiag(QString("after setDocument(reload) t=%1ms").arg(docFlashTimer.elapsed()));
        }
    }

    // Now the Documents view is fully populated: switch to it and hide the
    // board in one go, producing a single visible transition.
    mMainWindow->switchToDocumentsWidget();
    ubDocFlashDiag(QString("after switchToDocumentsWidget t=%1ms").arg(docFlashTimer.elapsed()));

    if (UBApplication::boardController)
        UBApplication::boardController->hide();

    mMainWindow->show();
    ubDocFlashDiag(QString("after mainWindow->show END t=%1ms").arg(docFlashTimer.elapsed()));

    mUninoteController->hideWindow();
    // #399 (brick 4c): legacy mainModeChanged emit removed (see setState(Documents)).
}

void UBApplicationController::doShowDesktop(bool dontSwitchFrontProcess)
{
    // Use the screen where the main window is currently displayed
    QScreen *currentScreen = mMainWindow->screen();
    if (!currentScreen)
        currentScreen = QGuiApplication::primaryScreen();

    int desktopWidgetIndex = QGuiApplication::screens().indexOf(currentScreen);
    if (desktopWidgetIndex < 0)
        desktopWidgetIndex = 0;

    if (UBApplication::boardController)
        UBApplication::boardController->hide();

    // Disable the board view so that stale mouse events queued before the
    // transition cannot be delivered to its QGraphicsView::viewportEvent
    // while the UI is in an inconsistent state.  (#135)
    if (UBApplication::boardController && UBApplication::boardController->controlView())
        UBApplication::boardController->controlView()->setEnabled(false);

    // Set the presentation state to DesktopAnnotation BEFORE hiding the main
    // window and showing the overlay. This drives changeMode(DESKTOP) via
    // UBPresentationController::stateChanged, hiding the QML palettes before they
    // could receive stale mouse events during the transition. (#135, #399)
    mIsShowingDesktop = true;
    mPresentationController->setState(UBPresentationController::State::DesktopAnnotation);

    mMainWindow->hide();
    mUninoteController->showWindow();

    if (mMirror)
    {
        QRect rect = QGuiApplication::screens().at(desktopWidgetIndex)->geometry();
        mMirror->setSourceRect(rect);
    }

    if (!dontSwitchFrontProcess) {
        UBPlatformUtils::bringPreviousProcessToFront();
    }

    // #390: do NOT force Selector here. showWindow() sets a deterministic Pen on
    // entry; forcing Selector afterwards is what left the Selector tool active in
    // desktop mode instead of Pen.
}


void UBApplicationController::doHideDesktop()
{
    // Re-enable the board view that was disabled in showDesktop() to prevent
    // stale events during the Desktop mode transition.  (#135)
    if (UBApplication::boardController && UBApplication::boardController->controlView())
        UBApplication::boardController->controlView()->setEnabled(true);

    // #399: internal continuation of the SAME transition — call the unguarded
    // do*() bodies directly so the re-entrancy guard does not block the return.
    if (mMainMode == Board)
    {
        doShowBoard();
    }
    else if (mMainMode == Internet)
    {
        doShowInternet();
    }
    else if (mMainMode == Document)
    {
        doShowDocument();
    }

    mIsShowingDesktop = false;

    mDisplayManager->adjustScreens(-1);

    // #399 (brick 4c): legacy desktopMode(false) emit removed. The palette
    // restore is driven by the inner doShow*() above, whose setState() emits
    // stateChanged (DesktopAnnotation -> Board/Web/Documents).
}

void UBApplicationController::setMirrorSourceWidget(QWidget* pWidget)
{
    if (mMirror)
    {
        mMirror->setSourceWidget(pWidget);
    }
}


void UBApplicationController::mirroringEnabled(bool enabled)
{
    if (mMirror)
    {
        if (enabled)
        {
            mMirror->start();
            mDisplayManager->setDisplayWidget(mMirror);

        }
        else
        {
            mDisplayManager->setDisplayWidget(mDisplayView);
            mMirror->stop();
        }

        mMirror->setVisible(enabled && mDisplayManager->numScreens() > 1);
        mUninoteController->updateShowHideState(enabled);
        UBApplication::mainWindow->actionWebShowHideOnDisplay->setChecked(enabled);
    }
    else
    {
        mDisplayManager->setDisplayWidget(mDisplayView);
    }
}


void UBApplicationController::closing()
{
    if (mMirror)
        mMirror->stop();

    if (mUninoteController)
    {
        mUninoteController->hideWindow();
        mUninoteController->close();
    }

    /*

    if (UBApplication::documentController)
        UBApplication::documentController->closing();

    */

    UBPersistenceManager::persistenceManager()->closing(); // ALTI/AOU - 20140616 : to update the file "documents/folders.xml"
}


void UBApplicationController::showMessage(const QString& message, bool showSpinningWheel)
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Document)
        {
            UBApplication::boardController->hideMessage();
            UBApplication::documentController->showMessage(message, showSpinningWheel);
        }
        else
        {
            UBApplication::documentController->hideMessage();
            UBApplication::boardController->showMessage(message, showSpinningWheel);
        }
    }
}


void UBApplicationController::importFile(const QString& pFilePath)
{
    const QFile fileToOpen(pFilePath);

    if (!fileToOpen.exists())
        return;

    UBDocumentProxy* document = 0;

    bool success = false;

    document = UBDocumentManager::documentManager()->importFile(fileToOpen, "");

    success = (document != 0);

    if (success && document)
    {
        if (mMainMode == Board || mMainMode == Internet)
        {
            if (UBApplication::boardController)
            {
                UBApplication::boardController->setActiveDocumentScene(document, 0);
                showBoard();
            }
        }
        else if (mMainMode == Document)
        {
            if (UBApplication::documentController)
                UBApplication::documentController->selectDocument(document);
        }
    }
}

void UBApplicationController::useMultiScreen(bool use)
{
    mDisplayManager->setUseMultiScreen(use);
    mDisplayManager->adjustScreens(0);
    mSettings->appUseMultiscreen->set(use);

}


QStringList UBApplicationController::widgetInlineJavaScripts()
{
    QString scriptDirPath = UBPlatformUtils::applicationResourcesDirectory() + "/widget-inline-js";
    QDir scriptDir(scriptDirPath);

    QStringList scripts;

    if (scriptDir.exists())
    {
        QStringList files = scriptDir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

        for (const QString& file : files)
        {
            QFile scriptFile(scriptDirPath + "/" + file);
            if (file.endsWith(".js") && scriptFile.open(QIODevice::ReadOnly))
            {
                QString s = QString::fromUtf8(scriptFile.readAll());

                if (s.length() > 0)
                    scripts << s;

            }
        }
    }

    std::sort(scripts.begin(), scripts.end());

    return scripts;
}



void UBApplicationController::actionCut()
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Board)
        {
            UBApplication::boardController->cut();
        }
        else if(mMainMode == Document)
        {
            UBApplication::documentController->cut();
        }
        else if(mMainMode == Internet)
        {
            UBApplication::webController->cut();
        }
    }
}


void UBApplicationController::actionCopy()
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Board)
        {
            UBApplication::boardController->copy();
        }
        else if(mMainMode == Document)
        {
            UBApplication::documentController->copy();
        }
        else if(mMainMode == Internet)
        {
            UBApplication::webController->copy();
        }
    }
}


void UBApplicationController::actionPaste()
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Board)
        {
            UBApplication::boardController->paste();
        }
        else if (mMainMode == Document)
        {
            UBApplication::documentController->paste();
        }
        else if(mMainMode == Internet)
        {
            UBApplication::webController->paste();
        }
    }
}
