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

/**
 * @file UBWebController.cpp
 * @brief Stub - web browser disabled in Qt6 build (no QtWebEngine).
 */

#include "UBWebController.h"

#include <QDesktopServices>
#include <QGuiApplication>
#include <QScreen>

class UBMainWindow;

UBWebController::UBWebController(UBMainWindow* mainWindow)
    : QObject(reinterpret_cast<QObject*>(mainWindow))
    , mMainWindow(mainWindow)
    , mCurrentWebBrowser(0)
    , mBrowserWidget(0)
    , mTrapContentController(0)
    , mToolsCurrentPalette(0)
    , mToolsPalettePositionned(false)
    , mDownloadViewIsVisible(false)
{
    mStackedWidget = nullptr;
    for (int i = 0; i < TotalNumberOfWebInstances; i++) {
        mWebBrowserList[i] = 0;
        mToolsPaletteList[i] = 0;
        mToolsPalettePositionnedList[i] = false;
    }
}

UBWebController::~UBWebController() {}
void UBWebController::closing() {}
void UBWebController::adaptToolBar() {}
QPixmap UBWebController::captureCurrentPage() { return QPixmap(); }
void UBWebController::showTabAtTop(bool) {}

void UBWebController::loadUrl(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

#ifdef SANKORE_WEBENGINE
QWebEngineView* UBWebController::createNewTab() { return nullptr; }
#else
void* UBWebController::createNewTab() { return nullptr; }
#endif
QUrl UBWebController::currentPageUrl() const { return QUrl(); }
void UBWebController::show(WebInstance) {}
void UBWebController::setupPalettes() {}

QPixmap UBWebController::getScreenPixmap()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen ? screen->grabWindow(0) : QPixmap();
}

void UBWebController::screenLayoutChanged() {}
void UBWebController::setSourceWidget(QWidget*) {}
void UBWebController::toggleWebToolsPalette(bool) {}
void UBWebController::captureWindow() {}
void UBWebController::customCapture() {}
void UBWebController::toogleMirroring(bool) {}
void UBWebController::captureoEmbed(QUrl) {}
void UBWebController::captureoEmbed() {}
void UBWebController::captureEduMedia() {}
bool UBWebController::isOEmbedable(const QUrl&) { return false; }
bool UBWebController::hasEmbeddedContent() { return false; }
void UBWebController::getEmbeddableContent() {}
bool UBWebController::isEduMedia(const QUrl&) { return false; }
void UBWebController::copy() {}
void UBWebController::paste() {}
void UBWebController::cut() {}
void UBWebController::activePageChanged() {}
void UBWebController::toggleWebTrap(bool) {}
void UBWebController::webTrapContent() {}
void UBWebController::onOEmbedParsed(QVector<sOEmbedContent>) {}
void UBWebController::onActionBookmark() {}
void UBWebController::initialiazemOEmbedProviders() {}
void UBWebController::webBrowserInstance() {}
void UBWebController::lookForEmbedContent(QString*, QString, QString, QList<QUrl>*) {}
void UBWebController::checkForOEmbed(QString*) {}
