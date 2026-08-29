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

#include "UBBoardNavigationController.h"
#include "UBBoardController.h"
#include "UBBoardView.h"

#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSettingsData.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"
#include "core/UBDownloadManager.h"
#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "frameworks/UBStringUtils.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"
#include "board/UBFeaturesController.h"

UBBoardNavigationController::UBBoardNavigationController(IUBBoardContext* boardController, QObject* parent)
    : QObject(parent)
    , mBoardController(boardController)
{
}

void UBBoardNavigationController::previousScene()
{
    if (mBoardController->activeSceneIndex() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        mBoardController->persistViewPositionOnCurrentScene();
        mBoardController->persistCurrentScene();
        mBoardController->setActiveDocumentScene(mBoardController->activeSceneIndex() - 1);
        mBoardController->controlView()->centerOn(mBoardController->activeScene()->lastCenter());
        QApplication::restoreOverrideCursor();
    }

    mBoardController->updateActionStates();
    mBoardController->emitPageChanged();
}

void UBBoardNavigationController::nextScene()
{
    if (mBoardController->activeSceneIndex() < mBoardController->selectedDocument()->pageCount() - 1)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        mBoardController->persistViewPositionOnCurrentScene();
        mBoardController->persistCurrentScene();
        mBoardController->setActiveDocumentScene(mBoardController->activeSceneIndex() + 1);
        mBoardController->controlView()->centerOn(mBoardController->activeScene()->lastCenter());
        QApplication::restoreOverrideCursor();
    }

    mBoardController->updateActionStates();
    mBoardController->emitPageChanged();
}

void UBBoardNavigationController::firstScene()
{
    if (mBoardController->activeSceneIndex() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        mBoardController->persistViewPositionOnCurrentScene();
        mBoardController->persistCurrentScene();
        mBoardController->setActiveDocumentScene(0);
        mBoardController->controlView()->centerOn(mBoardController->activeScene()->lastCenter());
        QApplication::restoreOverrideCursor();
    }

    mBoardController->updateActionStates();
    mBoardController->emitPageChanged();
}

void UBBoardNavigationController::lastScene()
{
    if (mBoardController->activeSceneIndex() < mBoardController->selectedDocument()->pageCount() - 1)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        mBoardController->persistViewPositionOnCurrentScene();
        mBoardController->persistCurrentScene();
        mBoardController->setActiveDocumentScene(mBoardController->selectedDocument()->pageCount() - 1);
        mBoardController->controlView()->centerOn(mBoardController->activeScene()->lastCenter());
        QApplication::restoreOverrideCursor();
    }

    mBoardController->updateActionStates();
    mBoardController->emitPageChanged();
}

void UBBoardNavigationController::addScene()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    mBoardController->persistViewPositionOnCurrentScene();
    mBoardController->persistCurrentScene();

    mBoardController->addPage(mBoardController->activeSceneIndex() + 1);

    mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    mBoardController->reloadThumbnails();
    mBoardController->setActiveDocumentScene(mBoardController->activeSceneIndex() + 1);

    // Handle default background image
    QString backgroundImage = mBoardController->selectedDocument()->metaData(UBSettingsData::documentDefaultBackgroundImage).toString();
    UBFeatureBackgroundDisposition backgroundImageDisposition = static_cast<UBFeatureBackgroundDisposition>(mBoardController->selectedDocument()->metaData(UBSettingsData::documentDefaultBackgroundImageDisposition).toInt());
    if (!backgroundImage.isEmpty())
    {
        QString sUrl = "file:///" + mBoardController->selectedDocument()->persistencePath() + "/" + UBPersistenceManager::imageDirectory + "/" + backgroundImage;
        QUrl urlImage(sUrl);
        mBoardController->downloadURL(urlImage, QString(), QPointF(), QSize(), true, false, backgroundImageDisposition);
    }

    QApplication::restoreOverrideCursor();
}

void UBBoardNavigationController::addScene(UBGraphicsScene* scene, bool replaceActiveIfEmpty)
{
    if (scene)
    {
        if (scene->document() && (scene->document() != mBoardController->selectedDocument()))
        {
            for (const QUrl& relativeFile : scene->relativeDependencies())
            {
                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                QString target = mBoardController->selectedDocument()->persistencePath() + "/" + relativeFile.toString();

                QFileInfo fi(target);
                QDir d = fi.dir();
                d.mkpath(d.absolutePath());
                QFile::copy(source, target);
            }
        }

        if (replaceActiveIfEmpty && mBoardController->activeScene()->isEmpty())
        {
            mBoardController->setActiveDocumentScene(mBoardController->activeSceneIndex());
        }
        else
        {
            mBoardController->persistCurrentScene();
            UBPersistenceManager::persistenceManager()->insertDocumentSceneAt(mBoardController->selectedDocument(), scene, mBoardController->activeSceneIndex() + 1);
            mBoardController->setActiveDocumentScene(mBoardController->activeSceneIndex() + 1);
        }

        mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    }
}

void UBBoardNavigationController::addScene(UBDocumentProxy* proxy, int sceneIndex, bool replaceActiveIfEmpty)
{
    UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(proxy, sceneIndex);
    if (scene)
    {
        addScene(scene, replaceActiveIfEmpty);
    }
}

void UBBoardNavigationController::duplicateScene(int nIndex)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    mBoardController->persistCurrentScene();

    QList<int> scIndexes;
    scIndexes << nIndex;
    mBoardController->duplicatePages(scIndexes);
    mBoardController->insertThumbPage(nIndex);
    mBoardController->emitDocumentThumbnailsUpdated(mBoardController);
    mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    mBoardController->setActiveDocumentScene(nIndex + 1);
    QApplication::restoreOverrideCursor();

    mBoardController->emitPageChanged();
    mBoardController->reloadThumbnails();
}

void UBBoardNavigationController::duplicateScene()
{
    if (UBApplication::applicationController->displayMode() != UBApplicationController::Board)
        return;
    duplicateScene(mBoardController->activeSceneIndex());
}

void UBBoardNavigationController::deleteScene(int nIndex)
{
    if (mBoardController->selectedDocument()->pageCount() >= 2)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        mBoardController->persistCurrentScene();
        mBoardController->showMessage(tr("Delete page %1 from document").arg(nIndex + 1), true);

        QList<int> scIndexes;
        scIndexes << nIndex;
        mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

        if (nIndex >= mBoardController->pageCount())
            nIndex = mBoardController->pageCount() - 1;
        mBoardController->setActiveDocumentScene(nIndex - 1);
        mBoardController->deletePages(scIndexes);
        mBoardController->reloadThumbnails();
        mBoardController->emitPageChanged();
        mBoardController->showMessage(tr("Page %1 deleted").arg(nIndex));
        QApplication::restoreOverrideCursor();
    }
}

void UBBoardNavigationController::moveSceneToIndex(int source, int target)
{
    if (mBoardController->selectedDocument())
    {
        mBoardController->persistCurrentScene();

        mBoardController->movePageToIndex(source, target);

        mBoardController->selectedDocument()->setMetaData(UBSettingsData::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(mBoardController->selectedDocument());
        mBoardController->setActiveDocumentScene(target);

        mBoardController->emitActiveSceneChanged();
    }
}

void UBBoardNavigationController::importPage()
{
    int pageCount = mBoardController->selectedDocument()->pageCount();
    if (UBApplication::documentController->addFileToDocument(mBoardController->selectedDocument()))
    {
        mBoardController->setActiveDocumentScene(mBoardController->selectedDocument(), pageCount, true);
    }
}
