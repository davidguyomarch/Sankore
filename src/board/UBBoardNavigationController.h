/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBBOARDNAVIGATIONCONTROLLER_H
#define UBBOARDNAVIGATIONCONTROLLER_H

#include <QObject>
#include "IUBBoardContext.h"

class IUBBoardContext;
class UBDocumentProxy;
class UBGraphicsScene;

/**
 * @brief Handles page navigation within a document.
 *
 * Extracted from UBBoardController to reduce its size and improve testability.
 * Manages: previous/next/first/last page, add/delete/duplicate/move pages.
 */
class UBBoardNavigationController : public QObject
{
    Q_OBJECT

public:
    explicit UBBoardNavigationController(IUBBoardContext* boardController, QObject* parent = nullptr);

public slots:
    void previousScene();
    void nextScene();
    void firstScene();
    void lastScene();

    void addScene();
    void addScene(UBDocumentProxy* proxy, int sceneIndex, bool replaceActiveIfEmpty = false);
    void addScene(UBGraphicsScene* scene, bool replaceActiveIfEmpty = false);

    void duplicateScene();
    void duplicateScene(int index);

    void deleteScene(int index);
    void moveSceneToIndex(int source, int target);
    void importPage();

private:
    IUBBoardContext* mBoardController;
};

#endif // UBBOARDNAVIGATIONCONTROLLER_H
