/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBBOARDITEMFACTORY_H
#define UBBOARDITEMFACTORY_H

#include <QObject>
#include <QPointF>
#include <QUrl>
#include <QSize>
#include <QMimeData>

#include "UBBoardController.h"

class UBGraphicsMediaItem;
class UBGraphicsWidgetItem;
class UBItem;
class UBGraphicsItem;

/**
 * @brief Creates and manages board items (media, widgets, clipboard).
 *
 * Extracted from UBBoardController to reduce its size.
 * Handles: addVideo, addAudio, addW3cWidget, addItem, cut, copy, paste,
 * processMimeData, grabScene.
 */
class UBBoardItemFactory : public QObject
{
    Q_OBJECT

public:
    explicit UBBoardItemFactory(UBBoardController* boardController, QObject* parent = nullptr);

public slots:
    UBGraphicsMediaItem* addVideo(const QUrl& pUrl, bool startPlay, const QPointF& pos, bool bUseSource = false);
    UBGraphicsMediaItem* addAudio(const QUrl& pUrl, bool startPlay, const QPointF& pos, bool bUseSource = false);
    UBGraphicsWidgetItem* addW3cWidget(const QUrl& pUrl, const QPointF& pos);
    void addItem();
    void cut();
    void copy();
    void paste();
    void processMimeData(const QMimeData* pMimeData, const QPointF& pPos, eItemActionType actionType = eItemActionType_Default);
    void grabScene(const QRectF& pSceneRect);

private:
    UBBoardController* mBoardController;
};

#endif // UBBOARDITEMFACTORY_H
