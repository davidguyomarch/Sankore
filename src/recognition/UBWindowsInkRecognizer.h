/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBWINDOWSINKRECOGNIZER_H
#define UBWINDOWSINKRECOGNIZER_H

#include "IHandwritingRecognizer.h"
#include <QStringList>

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

    /// Returns a diagnostic string listing available recognizers
    QString diagnosticInfo() const;

private:
    bool mAvailable;
    bool mComInitialized;
    QStringList mAvailableRecognizers;
};

#endif // Q_OS_WIN

#endif // UBWINDOWSINKRECOGNIZER_H
