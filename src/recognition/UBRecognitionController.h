/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBRECOGNITIONCONTROLLER_H
#define UBRECOGNITIONCONTROLLER_H

#include <QObject>

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

private:
    IHandwritingRecognizer* mRecognizer;
};

#endif // UBRECOGNITIONCONTROLLER_H
