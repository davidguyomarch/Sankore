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

#include "pdf/UBWebPluginPDFWidget.h"
#include "frameworks/UBFileSystemUtils.h"


UBWebPluginWidget::UBWebPluginWidget(const QUrl &url, QWidget *parent)
    : QWidget(parent)
    , mLoadingProgressBar(this)
{
    UBHttpGet* httpGet = new UBHttpGet(this);
    
    connect(httpGet, &UBHttpGet::downloadFinished, this, [this](bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF, QSize, bool) {
        downloadFinished(pSuccess, sourceUrl, pContentTypeHeader, pData);
    });
    connect(httpGet, &UBHttpGet::downloadProgress, this, &UBWebPluginWidget::downloadProgress);
    
    httpGet->get(url);
}

UBWebPluginWidget::~UBWebPluginWidget()
{
    // NOOP
}

void UBWebPluginWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    mLoadingProgressBar.move(geometry().center() - mLoadingProgressBar.geometry().center());
}

void UBWebPluginWidget::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal == -1)
    {
        mLoadingProgressBar.setMinimum(0);
        mLoadingProgressBar.setMaximum(0);
    } 
    else
    {
        mLoadingProgressBar.setMaximum(bytesTotal);
        mLoadingProgressBar.setValue(bytesReceived);
    }
}

void UBWebPluginWidget::downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData)
{
    Q_UNUSED(pSuccess);
    Q_UNUSED(pContentTypeHeader);

    QString tempFile = UBFileSystemUtils::createTempDir("UBWebPluginTemplate") + "/" + QFileInfo(sourceUrl.path()).fileName();
    QFile pdfFile(tempFile);
    pdfFile.open(QIODevice::WriteOnly);
    pdfFile.write(pData);
    pdfFile.close();
    handleFile(tempFile);
    mLoadingProgressBar.hide();
    update();
}

QString UBWebPluginWidget::title() const
{
    return QString(tr("Loading..."));
}
