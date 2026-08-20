/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBWINDOWSINKRECOGNIZER_H
#define UBWINDOWSINKRECOGNIZER_H

#include "IHandwritingRecognizer.h"
#include <QStringList>

#ifdef Q_OS_WIN

/**
 * @brief Windows Ink handwriting recognizer using WinRT API.
 *
 * Uses Windows.UI.Input.Inking (InkRecognizerContainer + InkStrokeBuilder)
 * from the modern WinRT API. This replaces the legacy COM API (msinkaut)
 * which is deprecated and broken under x64 emulation on ARM64.
 *
 * Requires Windows 10+ and C++/WinRT headers (included in Windows SDK).
 */
class UBWindowsInkRecognizer : public IHandwritingRecognizer
{
public:
    UBWindowsInkRecognizer();
    ~UBWindowsInkRecognizer() override;

    bool isAvailable() const override;
    QString engineName() const override;
    UBRecognitionResult recognize(const QVector<UBRecognitionStroke>& strokes) override;
    QString diagnosticInfo() const override;

private:
    bool mAvailable;
    QStringList mAvailableRecognizers;
};

#endif // Q_OS_WIN

#endif // UBWINDOWSINKRECOGNIZER_H
