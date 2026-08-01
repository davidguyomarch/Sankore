/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBZINNIARECOGNIZER_H
#define UBZINNIARECOGNIZER_H

#include "IHandwritingRecognizer.h"

#ifndef Q_OS_WIN

/**
 * @brief Linux handwriting recognizer using Zinnia (stroke-based).
 *
 * Zinnia is a lightweight open-source handwriting recognition engine
 * that works with stroke data (x,y coordinates per stroke).
 * It recognizes individual characters and returns ranked candidates.
 *
 * Requires: libzinnia-dev package + a trained model file.
 * Model location: /usr/share/zinnia/model/tomoe/ or bundled in resources.
 */
class UBZinniaRecognizer : public IHandwritingRecognizer
{
public:
    UBZinniaRecognizer();
    ~UBZinniaRecognizer() override;

    bool isAvailable() const override;
    QString engineName() const override;
    UBRecognitionResult recognize(const QVector<UBRecognitionStroke>& strokes) override;

private:
    bool mAvailable;
    QString mModelPath;
    void* mRecognizer; // zinnia::Recognizer* (opaque to avoid header dep)

    /// Segment strokes into character groups based on spatial gaps
    QVector<QVector<UBRecognitionStroke>> segmentIntoCharacters(const QVector<UBRecognitionStroke>& strokes);

    /// Recognize a single character from its strokes
    QString recognizeCharacter(const QVector<UBRecognitionStroke>& charStrokes);
};

#endif // !Q_OS_WIN

#endif // UBZINNIARECOGNIZER_H
