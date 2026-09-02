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



#include "UBDisplayManager.h"
#include <QAbstractButton>
#include <QGuiApplication>
#include <QScreen>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"

#include "board/UBBoardView.h"

#include "gui/UBBlackoutWidget.h"

#include "ui_blackoutWidget.h"


UBDisplayManager::UBDisplayManager(QObject *parent)
    : QObject(parent)
    , mControlScreenIndex(-1)
    , mDisplayScreenIndex(-1)
    , mControlWidget(0)
    , mDisplayWidget(0)
    , mDesktopWidget(0)
{
    mSettings = UBSettings::settings();

    // Issue #244: honour the user preference instead of always spanning every
    // display. The setting defaults to single-screen (false), so on a
    // multi-display setup the app no longer spreads across all screens at
    // startup — multi-screen is opt-in.
    mUseMultiScreen = mSettings->appUseMultiscreen->get().toBool();

    initScreenIndexes();

        // TODO: connect to QGuiApplication::screenAdded/screenRemoved
    
}


void UBDisplayManager::initScreenIndexes()
{
    int screenCount = numScreens();

    mScreenIndexesRoles.clear();

    if (screenCount > 0)
    {
        mControlScreenIndex = 0;
        if (mSettings->swapControlAndDisplayScreens->get().toBool())
        {
           mControlScreenIndex = mControlScreenIndex^1;
        }

        mScreenIndexesRoles << Control;
    }
    else
    {
        mControlScreenIndex = -1;
    }

    if (screenCount > 1)
    {
        mDisplayScreenIndex = mControlScreenIndex != 0 ? 0 : 1;
        mScreenIndexesRoles << Display;
    }
    else
    {
        mDisplayScreenIndex = -1;
    }

    mPreviousScreenIndexes.clear();

    if (screenCount > 2)
    {
       for(int i = 2; i < screenCount; i++)
       {
           if(mControlScreenIndex == i)
               mPreviousScreenIndexes.append(1);
           else
               mPreviousScreenIndexes.append(i);
       }
    }
}


UBDisplayManager::~UBDisplayManager()
{
    // NOOP
}


int UBDisplayManager::numScreens()
{
    if (mUseMultiScreen)
    {
        return QGuiApplication::screens().count();
    }
    else
    {
        return 1;
    }
}


int UBDisplayManager::numPreviousViews()
{
    return mPreviousScreenIndexes.size();
}


void UBDisplayManager::setControlWidget(QWidget* pControlWidget)
{
    if(hasControl() && pControlWidget && (pControlWidget != mControlWidget))
        mControlWidget = pControlWidget;
}

void UBDisplayManager::setDesktopWidget(QWidget* pControlWidget )
{
    if(pControlWidget && (pControlWidget != mControlWidget))
        mDesktopWidget = pControlWidget;
}

void UBDisplayManager::setDisplayWidget(QWidget* pDisplayWidget)
{
    if(pDisplayWidget && (pDisplayWidget != mDisplayWidget))
    {
        if (mDisplayWidget)
        {
            mDisplayWidget->hide();
            pDisplayWidget->setGeometry(mDisplayWidget->geometry());
            pDisplayWidget->setWindowFlags(mDisplayWidget->windowFlags());
        }
        mDisplayWidget = pDisplayWidget;
        if (mDisplayScreenIndex >= 0 && mDisplayScreenIndex < QGuiApplication::screens().size())
        {
            mDisplayWidget->setGeometry(QGuiApplication::screens().at(mDisplayScreenIndex)->geometry());
            if (mSettings->appUseMultiscreen->get().toBool())
                mDisplayWidget->showFullScreen();
        }
    }
}


void UBDisplayManager::setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews)
{
    mPreviousDisplayWidgets = pPreviousViews;
}


QRect UBDisplayManager::controlGeometry()
{
    return QGuiApplication::screens().at(mControlScreenIndex)->geometry();
}

QRect UBDisplayManager::displayGeometry()
{
    return QGuiApplication::screens().at(mDisplayScreenIndex)->geometry();
}

void UBDisplayManager::reinitScreens(bool swap)
{
    Q_UNUSED(swap);
    adjustScreens(-1);
}

void UBDisplayManager::adjustScreens(int screen)
{
    Q_UNUSED(screen);

    initScreenIndexes();

    positionScreens();

    emit screenLayoutChanged();
}


void UBDisplayManager::positionScreens()
{

    if(mDesktopWidget && mControlScreenIndex > -1)
    {
        mDesktopWidget->hide();
        mDesktopWidget->setGeometry(QGuiApplication::screens().at(mControlScreenIndex)->geometry());
    }
    if (mControlWidget && mControlScreenIndex > -1)
    {
        mControlWidget->hide();
        mControlWidget->setGeometry(QGuiApplication::screens().at(mControlScreenIndex)->geometry());
        mControlWidget->showFullScreen();
    }

    if (mDisplayWidget && mDisplayScreenIndex > -1)
    {
        mDisplayWidget->hide();
        mDisplayWidget->setGeometry(QGuiApplication::screens().at(mDisplayScreenIndex)->geometry());
        mDisplayWidget->showFullScreen();
    }
    else if(mDisplayWidget)
    {
        mDisplayWidget->hide();
    }

    for (int wi = mPreviousScreenIndexes.size(); wi < mPreviousDisplayWidgets.size(); wi++)
    {
        mPreviousDisplayWidgets.at(wi)->hide();
    }

    for (int psi = 0; psi < mPreviousScreenIndexes.size(); psi++)
    {
        if (mPreviousDisplayWidgets.size() > psi)
        {
            QWidget* previous = mPreviousDisplayWidgets.at(psi);
            previous->setGeometry(QGuiApplication::screens().at(mPreviousScreenIndexes.at(psi))->geometry());
            previous->showFullScreen();
        }
    }

    if (mControlWidget && mControlScreenIndex > -1)
        mControlWidget->activateWindow();

}


void UBDisplayManager::blackout()
{
    QList<int> screenIndexes;

    if (mControlScreenIndex > -1)
        screenIndexes << mControlScreenIndex;

    if (mDisplayScreenIndex > -1)
        screenIndexes << mDisplayScreenIndex;

    screenIndexes << mPreviousScreenIndexes;

    for (int i = 0; i < screenIndexes.size(); i++)
    {
        int screenIndex = screenIndexes.at(i);

        UBBlackoutWidget *blackoutWidget = new UBBlackoutWidget(); //deleted in UBDisplayManager::unBlackout
        Ui::BlackoutWidget *blackoutUi = new Ui::BlackoutWidget();
        blackoutUi->setupUi(blackoutWidget);

        connect(blackoutUi->iconButton, &QAbstractButton::pressed, blackoutWidget, &UBBlackoutWidget::doActivity);
        connect(blackoutWidget, &UBBlackoutWidget::activity, this, &UBDisplayManager::unBlackout);

        // display Uniboard logo on main screen
        blackoutUi->iconButton->setVisible(screenIndex == mControlScreenIndex);
        blackoutUi->labelClickToReturn->setVisible(screenIndex == mControlScreenIndex);

        blackoutWidget->setGeometry(QGuiApplication::screens().at(screenIndex)->geometry());

        mBlackoutWidgets << blackoutWidget;
    }

    UBPlatformUtils::fadeDisplayOut();

    for (UBBlackoutWidget *blackoutWidget : mBlackoutWidgets)
    {
        blackoutWidget->showFullScreen();
    }
}


void UBDisplayManager::unBlackout()
{
    while (!mBlackoutWidgets.isEmpty())
    {
        // the widget is also destroyed thanks to its Qt::WA_DeleteOnClose attribute
        mBlackoutWidgets.takeFirst()->close();
    }

    UBPlatformUtils::fadeDisplayIn();
}


void UBDisplayManager::setRoleToScreen(DisplayRole role, int screenIndex)
{
    Q_UNUSED(role);
    Q_UNUSED(screenIndex);
}


void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
}

