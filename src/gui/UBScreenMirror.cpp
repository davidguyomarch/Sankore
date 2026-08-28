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



#include "UBScreenMirror.h"
#include <QScreen>
#include <QGuiApplication>

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#if defined(Q_OS_MACOS)
#include <ApplicationServices/ApplicationServices.h>
#endif



UBScreenMirror::UBScreenMirror(QWidget* parent)
    : QWidget(parent)
    , mScreenIndex(0)
    , mSourceWidget(0)
    , mTimerID(0)
{
    mSettings = UBSettings::settings();
    // NOOP
}


UBScreenMirror::~UBScreenMirror()
{
    // NOOP
}


void UBScreenMirror::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.fillRect(0, 0, width(), height(), QBrush(Qt::black));

    if (!mLastPixmap.isNull())
    {
        int x = (width() - mLastPixmap.width()) / 2;
        int y = (height() - mLastPixmap.height()) / 2;

        painter.drawPixmap(x, y, mLastPixmap);
    }
}


void UBScreenMirror::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    grabPixmap();

    update();
}

void UBScreenMirror::grabPixmap()
{
    if (mSourceWidget)
    {
        QPoint topLeft = mSourceWidget->mapToGlobal(mSourceWidget->geometry().topLeft());
        QPoint bottomRight = mSourceWidget->mapToGlobal(mSourceWidget->geometry().bottomRight());

        mRect.setTopLeft(topLeft);
        mRect.setBottomRight(bottomRight);
        mLastPixmap = mSourceWidget->grab();
    }
    else{
        // WHY HERE?
        // this is the case we are showing the desktop but the is no widget and we use the last widget rectagle to know
        // what we have to grab. Not very good way of doing
        // In Qt6, use QScreen::grabWindow()
        mLastPixmap = QGuiApplication::screens().at(mScreenIndex)->grabWindow(0, mRect.x(), mRect.y(), mRect.width(), mRect.height());
    }

    if (!mLastPixmap.isNull())
        mLastPixmap = mLastPixmap.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


void UBScreenMirror::setSourceWidget(QWidget *sourceWidget)
{
    mSourceWidget = sourceWidget;

    mScreenIndex = 0; // Qt6: use sourceWidget->screen() to find index

    grabPixmap();

    update();
}


void UBScreenMirror::start()
{
    qDebug() << "mirroring START";
    UBApplication::boardController->freezeW3CWidgets(true);
    if (mTimerID == 0)
    {
        int ms = 125;

        bool success;
        int fps = mSettings->mirroringRefreshRateInFps->get().toInt(&success);

        if (success && fps > 0)
        {
            ms = 1000 / fps;
        }

        mTimerID = startTimer(ms);
    }
    else
    {
        qDebug() << "UBScreenMirror::start() : Timer already running ...";
    }
}


void UBScreenMirror::stop()
{
    qDebug() << "mirroring STOP";
    UBApplication::boardController->freezeW3CWidgets(false);
    if (mTimerID != 0)
    {
        killTimer(mTimerID);
        mTimerID = 0;
    }
}
