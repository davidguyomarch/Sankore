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



#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>
#ifdef SANKORE_WEBENGINE
#include <QWebEnginePage>
#endif
#include "UBStartupHintsPalette.h"

#include "globals/UBGlobals.h"
#include "core/UBSettings.h"





UBStartupHintsPalette::UBStartupHintsPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopRightCorner,parent)
{
    mSettings = UBSettings::settings();
    setObjectName("UBStartupHintsPalette");
    if(mSettings->appStartupHintsEnabled->get().toBool()){
        setFixedSize(700,450);
        mLayout = new QVBoxLayout();
        mLayout->setContentsMargins(10,28,10,10);
        setLayout(mLayout);
        QString url = mSettings->applicationStartupHintsDirectory() + "/index.html";
#ifdef SANKORE_WEBENGINE
        mpWebView = new QWebEngineView(this);
        mpSankoreAPI = new UBWidgetUniboardAPI(0);
        mpWebView->setUrl(QUrl::fromLocalFile(url));
        mpWebView->setAcceptDrops(false);
#else
        mpWebView = nullptr;
        mpSankoreAPI = nullptr;
#endif
        mLayout->addWidget(mpWebView);
        mButtonLayout = new QHBoxLayout();
        mLayout->addLayout(mButtonLayout);
        mShowNextTime = new QCheckBox(tr("Visible next time"),this);
        mShowNextTime->setCheckState(Qt::Checked);
        connect(mShowNextTime, &QCheckBox::stateChanged, this, &UBStartupHintsPalette::onShowNextTimeStateChanged);
        mButtonLayout->addStretch();
        mButtonLayout->addWidget(mShowNextTime);
    }
    else
        hide();
}

UBStartupHintsPalette::~UBStartupHintsPalette()
{
//    DELETEPTR(mButtonLayout);
//    DELETEPTR(mLayout);
}

void UBStartupHintsPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, QPixmap(":/images/close.svg"));
}


void UBStartupHintsPalette::close()
{
    hide();
}


void UBStartupHintsPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        close();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBStartupHintsPalette::onShowNextTimeStateChanged(int state)
{
    mSettings->appStartupHintsEnabled->setBool(state == Qt::Checked);
}

void UBStartupHintsPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustSizeAndPosition();
    move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 5);
}


int UBStartupHintsPalette::border()
{
    return 40;
}

void UBStartupHintsPalette::javaScriptWindowObjectCleared()
{
    // TODO: Use QWebChannel: //"sankore", mpSankoreAPI);
}
