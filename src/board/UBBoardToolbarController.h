/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBBOARDTOOLBARCONTROLLER_H
#define UBBOARDTOOLBARCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QPair>
#include <QString>

class QAction;
class UBBoardController;
class UBMainWindow;
class UBSettings;

/**
 * @brief Manages toolbar setup, action connections, and text display.
 *
 * Extracted from UBBoardController to reduce its size and improve testability.
 * Handles: setupToolbar, connectToolbar, initToolbarTexts, setToolbarTexts,
 * updateActionStates, updatePageSizeState.
 */
class UBBoardToolbarController : public QObject
{
    Q_OBJECT

public:
    explicit UBBoardToolbarController(UBBoardController* boardController,
                                      UBMainWindow* mainWindow,
                                      UBSettings* settings,
                                      QObject* parent = nullptr);

    void setupToolbar();
    void connectToolbar();
    void initToolbarTexts();
    void setToolbarTexts();
    void updateActionStates();
    void updatePageSizeState();

    /**
     * @brief Elide text to fit a given pixel width.
     * Static so it can be unit-tested without instantiating the controller.
     */
    static QString truncate(const QString& text, int maxWidth, const QFont& font);

private:

    UBBoardController* mBoardController;
    UBMainWindow* mMainWindow;
    UBSettings* mSettings;
    QMap<QAction*, QPair<QString, QString>> mActionTexts;
};

#endif // UBBOARDTOOLBARCONTROLLER_H
