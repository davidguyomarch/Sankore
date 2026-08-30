/*
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBMAGNIFIERHANDLER_H
#define UBMAGNIFIERHANDLER_H

#include <QObject>
#include <QPoint>

class UBMagnifier;
class UBMagnifierParams;
class UBCoreGraphicsScene;

/**
 * @brief Owns the two UBMagnifier widgets (control + display) and all
 *        magnifier-related logic previously embedded in UBGraphicsScene.
 *
 * Extracted as phase 1 of the UBGraphicsScene decomposition (#111).
 *
 * The handler does NOT know about UBGraphicsScene directly — it only
 * needs a UBCoreGraphicsScene* to call setModified().
 */
class UBMagnifierHandler : public QObject
{
    Q_OBJECT

public:
    explicit UBMagnifierHandler(UBCoreGraphicsScene* scene, QObject* parent = nullptr);
    ~UBMagnifierHandler();

    /** Create and show the magnifier widgets. No-op if already active. */
    void addMagnifier(UBMagnifierParams params);

    /** Reposition both widgets based on the control widget's current center. */
    void moveMagnifier();

    /** Reposition both widgets to a specific position. */
    void moveMagnifier(QPoint newPos, bool forceGrab = false);

    /** Close and destroy both widgets. */
    void closeMagnifier();

    void zoomInMagnifier();
    void zoomOutMagnifier();
    void changeMagnifierMode(int mode);
    void resizedMagnifier(qreal newPercent);

    /** Destroy widgets without marking scene as modified (used by scene destructor). */
    void disposeMagnifierWidgets();

    /** Returns true if the magnifier is currently active. */
    bool isActive() const { return mControlWidget != nullptr; }

private:
    UBCoreGraphicsScene* mScene;

    UBMagnifier* mControlWidget = nullptr;
    UBMagnifier* mDisplayWidget = nullptr;
};

#endif // UBMAGNIFIERHANDLER_H
