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



#ifndef UBAPPLICATIONCONTROLLER_H_
#define UBAPPLICATIONCONTROLLER_H_

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QNetworkAccessManager>

class UBBoardView;
class UBDocumentProxy;
class UBGraphicsScene;
class UBDesktopAnnotationController;
class UBPresentationController;
class UBScreenMirror;
class UBMainWindow;
class UBDisplayManager;
class UBVersion;
class UBSoftwareUpdate;
class QNetworkAccessManager;
class QNetworkReply;
// class QHttp; -- removed in Qt6
class UBSettings;


class UBApplicationController : public QObject
{
    Q_OBJECT;

    public:

        UBApplicationController(UBBoardView *pControlView, UBBoardView *pDisplayView, UBMainWindow *pMainWindow, QObject* parent);
        virtual ~UBApplicationController();

        void setSettings(UBSettings* settings) { mSettings = settings; }

        int initialHScroll() { return mInitialHScroll; }
        int initialVScroll() { return mInitialVScroll; }

        void adaptToolBar();
        void adjustDisplayView();
        void adjustPreviousViews(int pActiveSceneIndex, UBDocumentProxy *pActiveDocument);

        void blackout();

        void initScreenLayout(bool useMultiscreen);

        void closing();

        void setMirrorSourceWidget(QWidget*);

        void mirroringEnabled(bool);

        void initViewState(int horizontalPosition, int verticalPosition);

        // #399 (brick 3): public transition entry points. Each is now a thin
        // guarded wrapper around a private do*() body (see goToState / the
        // re-entrancy guard). Signatures and semantics are unchanged for callers.
        void showBoard();

        void showInternet();

        void showDocument();

        void showMessage(const QString& message, bool showSpinningWheel);

        void importFile(const QString& pFilePath);

        UBDisplayManager* displayManager()
        {
            return mDisplayManager;
        }

        UBDesktopAnnotationController* uninotesController()
        {
            return mUninoteController;
        }

        /// #399 (ADR-0008 D2): single source of truth for the presentation state.
        /// Introduced in shadow mode (brick 2) — updated in mirror of the
        /// existing transitions; no consumer of its stateChanged yet.
        UBPresentationController* presentationController()
        {
            return mPresentationController;
        }

        enum MainMode
        {
            Board = 0, Internet, Document, WebDocument
        };

        MainMode displayMode()
        {
            return mMainMode;
        }

        bool isCheckingForSoftwareUpdate() const;

        bool isShowingDesktop()
        {
            return mIsShowingDesktop;
        }

        QStringList widgetInlineJavaScripts();

    signals:
        // #399 (brick 4c): legacy mainModeChanged / desktopMode signals removed.
        // Mode transitions are now observed via UBPresentationController::
        // stateChanged (see presentationController()).

    public slots:

        /**
         * Add the pPixmap to the current scene and reactivate the board.
         * This Slot is connected with uninotes to manage the transition between board and uninotes.
         */
        void addCapturedPixmap(const QPixmap &pPixmap, bool pageMode, const QUrl& sourceUrl = QUrl());

        void addCapturedEmbedCode(const QString& embedCode);

        void screenLayoutChanged();

        // defaulting to false to match QAction triggered(bool checked = false)
        void showDesktop(bool dontSwitchFrontProcess = false);

        void hideDesktop();

        void useMultiScreen(bool use);

        void actionCut();
        void actionCopy();
        void actionPaste();

    private slots:

    private:
        // #399 (ADR-0008 D2, brick 3): the actual, UNGUARDED transition bodies.
        // The public showBoard()/showInternet()/showDocument()/showDesktop()/
        // hideDesktop() are thin re-entrancy-guarded wrappers around these.
        // Internal transition chains (e.g. hideDesktop -> board) call these do*()
        // directly so a legitimate synchronous nested transition is not blocked
        // by the guard.
        void doShowBoard();
        void doShowInternet();
        void doShowDocument();
        void doShowDesktop(bool dontSwitchFrontProcess);
        void doHideDesktop();

    protected:

        UBDesktopAnnotationController *mUninoteController;

        UBPresentationController *mPresentationController;  // #399 (shadow)

        UBMainWindow *mMainWindow;

        UBBoardView *mControlView;
        UBBoardView *mDisplayView;
        QList<UBBoardView*> mPreviousViews;

        UBGraphicsScene *mBlackScene;

        UBScreenMirror* mMirror;

        int mInitialHScroll, mInitialVScroll;

    private:

        UBSettings* mSettings;

        MainMode mMainMode;

        UBDisplayManager *mDisplayManager;

        bool mAutomaticCheckForUpdates;
        bool mCheckingForUpdates;

        void setCheckingForUpdates(bool value);

        bool mIsShowingDesktop;

        // #399 (ADR-0008 D2, brick 3): re-entrancy guard for mode transitions.
        // The transition methods (showBoard/showDesktop/hideDesktop/showInternet/
        // showDocument) drive palette slots via PresentationController::
        // stateChanged, which can trigger another transition — and hideDesktop() calls
        // showBoard() which calls hideWindow() again (the measured triple
        // hideWindow). This flag serializes them: a transition requested while
        // one is already running is ignored (logged), so each user action yields
        // exactly one transition.
        bool mInModeTransition = false;

        QNetworkAccessManager *networkAccessManager;

        // QHttp removed in Qt6 - TODO: use QNetworkAccessManager
};

#endif /* UBAPPLICATIONCONTROLLER_H_ */
