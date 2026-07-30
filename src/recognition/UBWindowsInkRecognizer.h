/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBWINDOWSINKRECOGNIZER_H
#define UBWINDOWSINKRECOGNIZER_H

#include "IHandwritingRecognizer.h"

#ifdef Q_OS_WIN

#include <windows.h>

/**
 * @brief Windows Ink handwriting recognizer using the COM Tablet PC API.
 *
 * Uses IInkRecognizerContext from msinkaut.h (InkObj.dll) to convert
 * stylus strokes into text. Supports multiple languages depending on
 * installed recognizer packs.
 */
class UBWindowsInkRecognizer : public IHandwritingRecognizer
{
public:
    UBWindowsInkRecognizer();
    ~UBWindowsInkRecognizer() override;

    bool isAvailable() const override;
    QString engineName() const override;
    UBRecognitionResult recognize(const QVector<UBRecognitionStroke>& strokes) override;

private:
    bool mAvailable;
    bool mComInitialized;
};

#endif // Q_OS_WIN

#endif // UBWINDOWSINKRECOGNIZER_H
