/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#include "UBBoardItemFactory.h"
#include "UBBoardController.h"
#include "UBBoardView.h"
#include "UBDrawingController.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QBuffer>
#include <QPainter>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSettingsData.h"
#include "core/UBMimeData.h"
#include "core/UBPersistenceManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBItem.h"

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"

#include "gui/UBFeaturesWidget.h"
#include "board/UBBoardPaletteManager.h"

UBBoardItemFactory::UBBoardItemFactory(IUBBoardContext* boardController, QObject* parent)
    : QObject(parent)
    , mBoardController(boardController)
{
}

UBGraphicsMediaItem* UBBoardItemFactory::addVideo(const QUrl& pSourceUrl, bool startPlay, const QPointF& pos, bool bUseSource)
{
    QUuid uuid = QUuid::createUuid();
    QUrl concreteUrl = pSourceUrl;

    if (bUseSource)
    {
        QString destFile;
        bool b = UBPersistenceManager::persistenceManager()->addFileToDocument(mBoardController->selectedDocument(),
                    pSourceUrl.toLocalFile(),
                    UBPersistenceManager::videoDirectory,
                    uuid,
                    destFile);
        if (!b)
        {
            mBoardController->showMessage(tr("Add file operation failed: file copying error"));
            return nullptr;
        }
        concreteUrl = QUrl::fromLocalFile(destFile);
    }

    UBGraphicsMediaItem* vi = mBoardController->activeScene()->addMedia(concreteUrl, startPlay, pos);
    mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    if (vi) {
        vi->setUuid(uuid);
        vi->setSourceUrl(pSourceUrl);
    }

    return vi;
}

UBGraphicsMediaItem* UBBoardItemFactory::addAudio(const QUrl& pSourceUrl, bool startPlay, const QPointF& pos, bool bUseSource)
{
    QUuid uuid = QUuid::createUuid();
    QUrl concreteUrl = pSourceUrl;

    if (bUseSource)
    {
        QString destFile;
        bool b = UBPersistenceManager::persistenceManager()->addFileToDocument(mBoardController->selectedDocument(),
            pSourceUrl.toLocalFile(),
            UBPersistenceManager::audioDirectory,
            uuid,
            destFile);
        if (!b)
        {
            mBoardController->showMessage(tr("Add file operation failed: file copying error"));
            return nullptr;
        }
        concreteUrl = QUrl::fromLocalFile(destFile);
    }

    UBGraphicsMediaItem* ai = mBoardController->activeScene()->addMedia(concreteUrl, startPlay, pos);
    mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    if (ai){
        ai->setUuid(uuid);
        ai->setSourceUrl(pSourceUrl);
    }

    return ai;
}

UBGraphicsWidgetItem* UBBoardItemFactory::addW3cWidget(const QUrl& pUrl, const QPointF& pos)
{
    UBGraphicsWidgetItem* w3cWidgetItem = 0;

    QUuid uuid = QUuid::createUuid();

    QString destPath;
    if (!UBPersistenceManager::persistenceManager()->addGraphicsWidgetToDocument(mBoardController->selectedDocument(), pUrl.toLocalFile(), uuid, destPath))
        return nullptr;
    QUrl newUrl = QUrl::fromLocalFile(destPath);

    w3cWidgetItem = mBoardController->activeScene()->addW3CWidget(newUrl, pos);

    if (w3cWidgetItem) {
        w3cWidgetItem->setUuid(uuid);
        w3cWidgetItem->setOwnFolder(newUrl);
        w3cWidgetItem->setSourceUrl(pUrl);

        QString struuid = UBStringUtils::toCanonicalUuid(uuid);
        QString snapshotPath = mBoardController->selectedDocument()->persistencePath() + "/" + UBPersistenceManager::widgetDirectory + "/" + struuid + ".png";
        w3cWidgetItem->setSnapshotPath(QUrl::fromLocalFile(snapshotPath));
    }

    return w3cWidgetItem;
}

void UBBoardItemFactory::addItem()
{
    UBSettings* settings = UBSettings::settings();
    QString defaultPath = settings->lastImportToLibraryPath->get().toString();

    QString extensions;
    for (const QString& ext : UBSettings::imageFileExtensions)
    {
        extensions += " *.";
        extensions += ext;
    }

    QString filename = QFileDialog::getOpenFileName(mBoardController->controlContainer(), tr("Add Item"),
                                                    defaultPath,
                                                    tr("All Supported (%1)").arg(extensions), nullptr, QFileDialog::DontUseNativeDialog);

    if (filename.length() > 0)
    {
        mBoardController->paletteManager()->addItem(QUrl::fromLocalFile(filename));
        QFileInfo source(filename);
        settings->lastImportToLibraryPath->set(QVariant(source.absolutePath()));
    }
}

void UBBoardItemFactory::cut()
{
    copy();

    QList<UBItem*> selected;
    for (QGraphicsItem* gi : mBoardController->activeScene()->selectedItems())
    {
        gi->setSelected(false);

        UBItem* ubItem = dynamic_cast<UBItem*>(gi);
        UBGraphicsItem *ubGi = dynamic_cast<UBGraphicsItem*>(gi);

        if (ubItem && ubGi && !mBoardController->activeScene()->tools().contains(gi))
        {
            selected << ubItem->deepCopy();
            ubGi->remove();
        }
    }
}

void UBBoardItemFactory::copy()
{
    QList<UBItem*> selected;

    for (QGraphicsItem* gi : mBoardController->activeScene()->selectedItems())
    {
        UBItem* ubItem = dynamic_cast<UBItem*>(gi);
        if (ubItem && !mBoardController->activeScene()->tools().contains(gi))
            selected << ubItem;
    }

    if (selected.size() > 0)
    {
        QClipboard *clipboard = QApplication::clipboard();

        UBMimeDataGraphicsItem* mimeGi = new UBMimeDataGraphicsItem(selected);
        mimeGi->setData(UBApplication::mimeTypeUniboardPageItem, QByteArray());
        clipboard->setMimeData(mimeGi);
    }
}

void UBBoardItemFactory::paste()
{
    QClipboard *clipboard = QApplication::clipboard();
    qreal xPosition = ((qreal)QRandomGenerator::global()->generate() / (qreal)RAND_MAX) * 400;
    qreal yPosition = ((qreal)QRandomGenerator::global()->generate() / (qreal)RAND_MAX) * 200;
    QPointF pos(xPosition - 200, yPosition - 100);
    processMimeData(clipboard->mimeData(), pos, eItemActionType_Paste);

    mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
}

void UBBoardItemFactory::processMimeData(const QMimeData* pMimeData, const QPointF& pPos, eItemActionType actionType)
{
    if (pMimeData->hasFormat(UBApplication::mimeTypeUniboardPage))
    {
        const UBMimeData* mimeData = qobject_cast<const UBMimeData*>(pMimeData);
        if (mimeData)
        {
            int previousActiveSceneIndex = mBoardController->activeSceneIndex();
            int previousPageCount = mBoardController->selectedDocument()->pageCount();

            for (const UBMimeDataItem& sourceItem : mimeData->items())
                mBoardController->addScene(sourceItem.documentProxy(), sourceItem.sceneIndex(), true);

            if (mBoardController->selectedDocument()->pageCount() < previousPageCount + mimeData->items().count())
                mBoardController->setActiveDocumentScene(previousActiveSceneIndex);
            else
                mBoardController->setActiveDocumentScene(previousActiveSceneIndex + 1);

            return;
        }
    }

    if (pMimeData->hasFormat(UBApplication::mimeTypeUniboardPageItem))
    {
        const UBMimeDataGraphicsItem* mimeData = qobject_cast<const UBMimeDataGraphicsItem*>(pMimeData);
        if (mimeData)
        {
            for (UBItem* item : mimeData->items())
            {
                QGraphicsItem* pItem = dynamic_cast<QGraphicsItem*>(item);
                if (nullptr != pItem)
                    mBoardController->duplicateItem(item, true, actionType);
            }
            return;
        }
    }

    if (pMimeData->hasHtml())
    {
        QString qsHtml = pMimeData->html();
        QString url = UBApplication::urlFromHtml(qsHtml);
        if ("" != url)
        {
            mBoardController->downloadURL(url, QString(), pPos);
            return;
        }
    }

    if (pMimeData->hasUrls())
    {
        QList<QUrl> urls = pMimeData->urls();
        int index = 0;

        const UBFeaturesMimeData *internalMimeData = qobject_cast<const UBFeaturesMimeData*>(pMimeData);
        bool internalData = (internalMimeData != nullptr);

        for (const QUrl& url : urls)
        {
            QPointF pos(pPos + QPointF(index * 15, index * 15));
            mBoardController->downloadURL(url, QString(), pos, QSize(), false, internalData);
            index++;
        }
        return;
    }

    if (pMimeData->hasImage())
    {
        QImage img = qvariant_cast<QImage>(pMimeData->imageData());
        QPixmap pix = QPixmap::fromImage(img);
        if (pix.width() != 0 && pix.height() != 0)
        {
            mBoardController->activeScene()->addPixmap(pix, nullptr, pPos, 1.);
            return;
        }
    }

    if (pMimeData->hasText())
    {
        if (pMimeData->text().length())
        {
            QString qsTmp = pMimeData->text().remove(QRegularExpression("[\\0]"));
            if (qsTmp.startsWith("http"))
            {
                mBoardController->downloadURL(QUrl(qsTmp), QString(), pPos);
            }
            else
            {
                if (actionType == eItemActionType_Paste && (mBoardController->activeScene()->selectedItems().size() > 0) &&
                    mBoardController->activeScene()->selectedItems().at(0)->type() == UBGraphicsItemType::TextItemType)
                {
                    dynamic_cast<UBGraphicsTextItem*>(mBoardController->activeScene()->selectedItems().at(0))->setHtml(pMimeData->text());
                }
                else
                {
                    mBoardController->activeScene()->addTextHtml(pMimeData->text(), pPos);
                }
            }
        }
        else
        {
#ifdef Q_OS_MACOSX
            QString qsUrl = UBPlatformUtils::urlFromClipboard();
            if ("" != qsUrl)
            {
                mBoardController->downloadURL(qsUrl, qsUrl, pPos);
                return;
            }
#endif
        }
    }
}

void UBBoardItemFactory::grabScene(const QRectF& pSceneRect)
{
    if (mBoardController->activeScene())
    {
        QImage image(pSceneRect.width(), pSceneRect.height(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QRectF targetRect(0, 0, pSceneRect.width(), pSceneRect.height());
        QPainter painter(&image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);

        mBoardController->activeScene()->setRenderingQuality(UBItem::RenderingQualityHigh);
        mBoardController->activeScene()->render(&painter, targetRect, pSceneRect);
        mBoardController->activeScene()->setRenderingQuality(UBItem::RenderingQualityNormal);

        mBoardController->paletteManager()->addItem(QPixmap::fromImage(image));
        mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    }
}
