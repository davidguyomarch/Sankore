/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef IUBBOARDCONTEXT_H
#define IUBBOARDCONTEXT_H

#include <QSize>
#include <QPointF>
#include <QUrl>
#include <QObject>

#include "core/UBTypes.h"

class UBGraphicsScene;
class UBDocumentProxy;
class UBBoardView;
class UBBoardPaletteManager;
class UBItem;
class UBGraphicsItem;
class UBGraphicsWidgetItem;

typedef enum{
    eItemActionType_Default,
    eItemActionType_Duplicate,
    eItemActionType_Paste
}eItemActionType;

/**
 * @brief Abstract interface exposing UBBoardController accessors.
 *
 * Sub-controllers depend on this interface instead of the concrete
 * UBBoardController class. This allows unit testing with a mock.
 *
 * Only read-only accessors and lightweight actions are here.
 * Heavy mutations (persistCurrentScene, setActiveDocumentScene) remain
 * on the concrete class for now.
 */
class IUBBoardContext
{
public:
    virtual ~IUBBoardContext() = default;

    // --- QObject access for signal/slot connections ---
    virtual QObject* asQObject() = 0;

    // --- Read-only state accessors ---
    virtual UBGraphicsScene* activeScene() = 0;
    virtual int activeSceneIndex() const = 0;
    virtual UBDocumentProxy* selectedDocument() = 0;
    virtual UBBoardView* controlView() = 0;
    virtual QWidget* controlContainer() = 0;
    virtual QSize controlViewport() = 0;
    virtual qreal systemScaleFactor() = 0;
    virtual qreal zoomFactor() = 0;
    virtual UBBoardPaletteManager* paletteManager() = 0;
    virtual int pageCount() = 0;

    // --- Lightweight actions (no heavy side effects) ---
    virtual void updateActionStates() = 0;
    virtual void showMessage(const QString& message, bool showSpinningWheel = false) = 0;
    virtual void setSystemScaleFactor(qreal factor) = 0;

    // --- Navigation actions (delegated from sub-controllers) ---
    virtual void persistCurrentScene(UBDocumentProxy* pProxy = nullptr) = 0;
    virtual void persistViewPositionOnCurrentScene() = 0;
    virtual void setActiveDocumentScene(int pSceneIndex) = 0;
    virtual void setActiveDocumentScene(UBDocumentProxy* pDocumentProxy, int pSceneIndex = 0, bool forceReload = false, const bool onImport = false) = 0;
    virtual void reloadThumbnails() = 0;

    // --- Page operations ---
    virtual void addPage(int index) = 0;
    virtual void duplicatePages(QList<int> indexes) = 0;
    virtual void deletePages(QList<int> indexes) = 0;
    virtual void movePageToIndex(int source, int target) = 0;
    virtual void insertThumbPage(int index) = 0;

    // --- Item operations ---
    virtual void downloadURL(const QUrl& url, QString contentSourceUrl = QString(), const QPointF& pPos = QPointF(0.0, 0.0), const QSize& pSize = QSize(), bool isBackground = false, bool internalData = false, UBFeatureBackgroundDisposition disposition = (UBFeatureBackgroundDisposition)0) = 0;
    virtual UBGraphicsItem* duplicateItem(UBItem* item, bool bAsync = true, eItemActionType actionType = (eItemActionType)0) = 0;

    // --- Scene operations ---
    virtual void addScene(UBDocumentProxy* proxy, int sceneIndex, bool replaceActiveIfEmpty = false) = 0;

    // --- Signal emission (implemented by concrete QObject) ---
    virtual void emitPageChanged() = 0;
    virtual void emitActiveSceneChanged() = 0;
    virtual void emitZoomChanged(qreal zoom) = 0;
    virtual void emitControlViewportChanged() = 0;
    virtual void emitSystemScaleFactorChanged(qreal factor) = 0;
    virtual void emitDocumentThumbnailsUpdated(void* sender) = 0;
};

#endif // IUBBOARDCONTEXT_H
