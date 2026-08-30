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



#include <QWidget>
#include <QApplication>
#include <QPainter>
#include "UBToolWidget.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "document/UBDocumentController.h"


QPixmap* UBToolWidget::sClosePixmap = 0;
QPixmap* UBToolWidget::sUnpinPixmap = 0;


UBToolWidget::UBToolWidget(const QUrl& pUrl, QWidget *pParent)
    : QWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mWebView(0)
    , mToolWidget(0)
    , mShouldMoveWidget(false)
    , mContentMargin(0)
    , mFrameWidth(0)

{
    mSettings = UBSettings::settings();
    int widgetType = UBGraphicsWidgetItem::widgetType(pUrl);
    if (widgetType == UBWidgetType::Apple)
        mToolWidget = new UBGraphicsAppleWidgetItem(pUrl);
    else if (widgetType == UBWidgetType::W3C)
        mToolWidget = new UBGraphicsW3CWidgetItem(pUrl);
    else
        qDebug() << "UBToolWidget::UBToolWidget: Unknown widget Type";

    initialize();
}

UBToolWidget::UBToolWidget(UBGraphicsWidgetItem *pWidget, QWidget *pParent)
    : QWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mWebView(0)
    , mToolWidget(pWidget)
    , mShouldMoveWidget(false)
    , mContentMargin(0)
    , mFrameWidth(0)
    , mScene(0)

{
    initialize();
    javaScriptWindowObjectCleared();
}

UBToolWidget::~UBToolWidget()
{
    // NOOP
}

void UBToolWidget::initialize()
{
    if (!sClosePixmap)
        sClosePixmap = new QPixmap(":/images/close.svg");

    if(!sUnpinPixmap)
        sUnpinPixmap = new QPixmap(":/images/unpin.svg");

    UBGraphicsScene *wscene = dynamic_cast<UBGraphicsScene *>(mToolWidget->scene());
    if (wscene)
    {
        wscene->removeItemFromDeletion(mToolWidget);
        wscene->removeItem(mToolWidget);
        mScene = wscene;
    }


    // Create the embedded web view for HTML widgets
#ifdef SANKORE_WEBENGINE
    mWebView = new QWebEngineView(this);
#else
    mWebView = nullptr;
#endif

    mFrameWidth = mSettings->objectFrameWidth;
    mContentMargin = sClosePixmap->width() / 2 + mFrameWidth;
    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(mContentMargin, mContentMargin, mContentMargin, mContentMargin);

    setFixedSize(mToolWidget->boundingRect().width() + mContentMargin * 2, mToolWidget->boundingRect().height() + mContentMargin * 2);

#ifdef SANKORE_WEBENGINE
    connect(mWebView->page(), &QWebEnginePage::loadFinished, this, &UBToolWidget::javaScriptWindowObjectCleared);
    mWebView->load(mToolWidget->mainHtml());
    mWebView->setAcceptDrops(false);
    mWebView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    mWebView->setAttribute(Qt::WA_OpaquePaintEvent, false);
#endif

    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &UBToolWidget::javaScriptWindowObjectCleared);
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &UBToolWidget::reactOnBoardChanged);
}


bool UBToolWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (mShouldMoveWidget && obj == mWebView && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);

        if (mouseMoveEvent->buttons() & Qt::LeftButton)
        {
            move(pos() - mMousePressPos + mWebView->mapTo(this, mouseMoveEvent->pos()));

            event->accept();
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void UBToolWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    /* did webkit consume the mouse press ? */
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);
    mMousePressPos = event->pos();
    event->accept();
    update();
}

void UBToolWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(mShouldMoveWidget && (event->buttons() & Qt::LeftButton)) {
        move(pos() - mMousePressPos + event->pos());
        event->accept();
    }
    QWidget::mouseMoveEvent(event);
}

void UBToolWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mShouldMoveWidget = false;

    if (event->pos().x() >= 0 && event->pos().x() < sClosePixmap->width() && event->pos().y() >= 0 && event->pos().y() < sClosePixmap->height()) {
        UBApplication::boardController->removeTool(this);
        event->accept();
    }
    else if (mToolWidget->canBeContent() && event->pos().x() >= mContentMargin && event->pos().x() < mContentMargin + sUnpinPixmap->width() && event->pos().y() >= 0 && event->pos().y() < sUnpinPixmap->height()) {
        UBApplication::boardController->moveToolWidgetToScene(this);
        event->accept();
    }
    else
        QWidget::mouseReleaseEvent(event); /* don't propgate to parent, the widget is deleted in UBApplication */

}

void UBToolWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    //if (isActiveWindow())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(127, 127, 127, 127));

        painter.drawRoundedRect(QRectF(sClosePixmap->width() / 2
                                     , sClosePixmap->height() / 2
                                     , width() - sClosePixmap->width()
                                     , mFrameWidth)
                                     , mFrameWidth / 2
                                     , mFrameWidth / 2);

        painter.drawPixmap(0, 0, *sClosePixmap);

        if (mToolWidget->canBeContent())
            painter.drawPixmap(mContentMargin, 0, *sUnpinPixmap);
    }
}

void UBToolWidget::javaScriptWindowObjectCleared()
{
    // Widget JS APIs removed (UBWidgetUniboardAPI, UBW3CWidgetAPI) — WebEngine is stubbed
}

void UBToolWidget::reactOnBoardChanged()
{
    if (!mScene) {
        return;
    }

    UBApplication::boardController->selectedDocument() == mScene->document() ? show() : hide();
}

UBGraphicsWidgetItem* UBToolWidget::toolWidget() const
{
    return mToolWidget;
}

QPoint UBToolWidget::naturalCenter() const
{
    if (mWebView)
        return mWebView->geometry().center();
    else
        return QPoint(0, 0);
}

void UBToolWidget::remove()
{
    mToolWidget = nullptr;
    hide();
    deleteLater();
}

void UBToolWidget::centerOn(const QPoint& pos)
{
    QWidget::move(pos - QPoint(width() / 2, height() / 2));
}
