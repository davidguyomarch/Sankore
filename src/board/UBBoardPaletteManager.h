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



#ifndef UBBOARDPALETTEMANAGER_H_
#define UBBOARDPALETTEMANAGER_H_

#include <QWidget>
#include <QApplication>
#include <QPainter>
#ifdef SANKORE_WEBENGINE
#include <QWebEngineView>
#endif

#include "gui/UBFeaturesWidget.h"
#include "gui/UBDockPalette.h"
#include "core/UBApplicationController.h"

class UBSettings;

class UBActionPalette;
class UBBoardController;
class UBKeyboardPalette;
class UBMainWindow;
class UBApplicationController;
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
        UBToolController* toolController() { return mToolController; }
        UBFeaturesWidget *featuresWidget(){return mpFeaturesWidget;}
        UBActionPalette *addItemPalette() {return mAddItemPalette;}
        void showVirtualKeyboard(bool show = true);
        void initPalettesPosAtStartup();
        void connectToDocumentController();
        void refreshPalettes();

        UBKeyboardPalette *mKeyboardPalette;

        UBCreateLinkPalette* linkPalette();

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

        // QML V2 palettes
        QQuickWidget *mStylusPaletteQml;
        UBToolController *mToolController;
        UBPageController *mPageController;
        UBAppController *mAppController;
        QQuickWidget *mTopBarQml;
        QQuickWidget *mPageNavQml;
        QQuickWidget *mDrawingPropsBarQml;
        QQuickWidget *mShapesPaletteV2Qml;

        UBCreateLinkPalette* mLinkPalette;

        UBActionPalette* mAddItemPalette;
        UBActionPalette* mImageBackgroundPalette;

        QUrl mItemUrl;
        QPixmap mPixmap;
        QPointF mPos;
        qreal mScaleFactor;

        UBFeaturesWidget *mpFeaturesWidget;

        eUBDockPaletteWidgetMode mPaletteMode;

        bool mDownloadInProgress;

    private slots:

        void changeBackground();

        void addItemToCurrentPage();
        void addItemToNewPage();
        void addItemToLibrary();

        void purchaseLinkActivated(const QString&);

        void linkClicked(const QUrl& url);

        void changeStylusPaletteOrientation(QVariant var);
};

#endif /* UBBOARDPALETTEMANAGER_H_ */
