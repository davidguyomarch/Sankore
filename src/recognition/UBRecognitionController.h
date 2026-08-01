/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBRECOGNITIONCONTROLLER_H
#define UBRECOGNITIONCONTROLLER_H

#include <QObject>
#include <QRectF>
#include <QTimer>

class IHandwritingRecognizer;
class UBGraphicsScene;

/**
 * @brief Controller that manages handwriting recognition on the board.
 *
 * Provides a slot to recognize the currently selected strokes and replace
 * them with a text item.
 */
class UBRecognitionController : public QObject
{
    Q_OBJECT

public:
    explicit UBRecognitionController(QObject* parent = nullptr);
    ~UBRecognitionController();

    bool isAvailable() const;

public slots:
    /// Recognize currently selected strokes on the active scene.
    void recognizeSelection();

    /// Recognize strokes within a rectangular zone on the active scene.
    void recognizeZone(const QRectF& sceneRect);

    /// Toggle auto-recognition mode (recognize after pause in writing)
    void setAutoMode(bool enabled);

    /// Called when a stroke is finished on the active scene
    void onStrokeFinished();

private slots:
    void onAutoTimerExpired();

private:
    IHandwritingRecognizer* mRecognizer;
    bool mAutoMode;
    QTimer* mAutoTimer;
};

#endif // UBRECOGNITIONCONTROLLER_H
