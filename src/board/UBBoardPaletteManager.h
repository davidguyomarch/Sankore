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



#ifndef UBBOARDPALETTEMANAGER_H_
#define UBBOARDPALETTEMANAGER_H_

#include <QWidget>
#include <QApplication>
#include <QPainter>
#ifdef SANKORE_WEBENGINE
#include <QWebEngineView>
#endif

#include "gui/UBLeftPalette.h"
#include "gui/UBRightPalette.h"
#include "gui/UBFeaturesWidget.h"
#include "gui/UBDockResourcesWidget.h"
#include "gui/UBDrawingPalette.h"
#include "gui/UBStylusPalette.h"
#include "core/UBApplicationController.h"

class UBSettings;

class UBWebToolsPalette;
class UBStylusPalette;
class UBZoomPalette;
class UBActionPalette;
class UBBoardController;
class UBKeyboardPalette;
class UBMainWindow;
class UBApplicationController;
class UBDockTeacherGuideWidget;
class UBCreateLinkPalette;
class QQuickWidget;
class UBToolController;
class UBPageController;
class UBAppController;

class UBBoardPaletteManager : public QObject
{
    Q_OBJECT

    public:
        UBBoardPaletteManager(QWidget* container, UBBoardController* controller);
        virtual ~UBBoardPaletteManager();

        void setupLayout();
        UBLeftPalette* leftPalette(){return mLeftPalette;}
        UBRightPalette* rightPalette(){return mRightPalette;}
        UBFeaturesWidget *featuresWidget(){return mpFeaturesWidget;}
        UBStylusPalette* stylusPalette(){return mStylusPalette;}
        UBDrawingPalette* drawingPalette() { return mDrawingPalette; }
        UBToolController* toolController() { return mToolController; }
        UBActionPalette *addItemPalette() {return mAddItemPalette;}
        void showVirtualKeyboard(bool show = true);
        void initPalettesPosAtStartup();
        void connectToDocumentController();
        void refreshPalettes();

        UBKeyboardPalette *mKeyboardPalette;

        UBCreateLinkPalette* linkPalette();

        void setCurrentWebToolsPalette(UBWebToolsPalette *palette) {mWebToolsCurrentPalette = palette;}
        UBWebToolsPalette* mWebToolsCurrentPalette;

        UBDockTeacherGuideWidget* teacherGuideDockWidget() { return nullptr; }

        //issue 1682 - NNE - 20140110
        UBDockResourcesWidget* teacherResourcesDockWidget(){ return mTeacherResources; }

        void processPalettersWidget(UBDockPalette *paletter, eUBDockPaletteWidgetMode mode);
        void changeMode(eUBDockPaletteWidgetMode newMode, bool isInit = false);
        eUBDockPaletteWidgetMode currentMode() const { return mPaletteMode; }
        void startDownloads();
        void stopDownloads();

    signals:
        void connectToDocController();

    public slots:

        void activeSceneChanged();
        void containerResized();
        void addItem(const QUrl& pUrl);
        void addItem(const QPixmap& pPixmap, const QPointF& p = QPointF(0.0, 0.0), qreal scale = 1.0, const QUrl& sourceUrl = QUrl());

        void slot_changeMainMode(UBApplicationController::MainMode);
        void slot_changeDesktopMode(bool);

        void toggleErasePalette(bool ckecked);
        void toggleImageBackgroundPalette(bool ckecked, bool isDefault);
        void closeAllPopupPalettes();

    private:
        UBSettings* mSettings;

        void setupPalettes();
        void connectPalettes();
        void positionFreeDisplayPalette();
        void setupDockPaletteWidgets();

        QWidget* mContainer;
        UBBoardController *mBoardControler;

        // Old palettes (still referenced by UBBoardController/UBDrawingController)
        UBDrawingPalette *mDrawingPalette;
        UBStylusPalette *mStylusPalette;

        // QML V2 palettes
        QQuickWidget *mStylusPaletteQml;
        UBToolController *mToolController;
        UBPageController *mPageController;
        UBAppController *mAppController;
        QQuickWidget *mTopBarQml;
        QQuickWidget *mPageNavQml;
        QQuickWidget *mDrawingPropsBarQml;
        QQuickWidget *mShapesPaletteV2Qml;

        UBZoomPalette *mZoomPalette;
        UBCreateLinkPalette* mLinkPalette;

        /** The left dock palette */
        UBLeftPalette* mLeftPalette;
        /** The right dock palette */
        UBRightPalette* mRightPalette;

        //issue 1682 - NNE - 20131218
        UBDockResourcesWidget *mTeacherResources;

        UBActionPalette *mBackgroundsPalette;
        UBActionPalette *mToolsPalette;
        UBActionPalette* mAddItemPalette;
        UBActionPalette* mErasePalette;
        UBActionPalette* mPagePalette;
        UBActionPalette* mImageBackgroundPalette;

        // EV-7 - CFA - 20140102
        UBActionPaletteButton* mEllipseActionPaletteButton;

        QUrl mItemUrl;
        QPixmap mPixmap;
        QPointF mPos;
        qreal mScaleFactor;

        QTime mPageButtonPressedTime;
        bool mPendingPageButtonPressed;

        QTime mZoomButtonPressedTime;
        bool mPendingZoomButtonPressed;

        QTime mPanButtonPressedTime;
        bool mPendingPanButtonPressed;

        QTime mEraseButtonPressedTime;
        bool mPendingEraseButtonPressed;

        UBFeaturesWidget *mpFeaturesWidget;

        eUBDockPaletteWidgetMode mPaletteMode;

        bool mDownloadInProgress;

    private slots:

        void changeBackground();

        void toggleBackgroundPalette(bool checked);
        void backgroundPaletteClosed();

        void toggleStylusPalette(bool checked);
        void toggleDrawingPalette(bool checked);
        void tooglePodcastPalette(bool checked);

        void erasePaletteButtonPressed();
        void erasePaletteButtonReleased();

        void erasePaletteClosed();

        void togglePagePalette(bool ckecked);
        void pagePaletteClosed();

        void pagePaletteButtonPressed();
        void pagePaletteButtonReleased();

        void addItemToCurrentPage();
        void addItemToNewPage();
        void addItemToLibrary();

        void purchaseLinkActivated(const QString&);

        void linkClicked(const QUrl& url);

        void zoomButtonPressed();
        void zoomButtonReleased();
        void panButtonPressed();
        void panButtonReleased();

        void changeStylusPaletteOrientation(QVariant var);
};

#endif /* UBBOARDPALETTEMANAGER_H_ */
