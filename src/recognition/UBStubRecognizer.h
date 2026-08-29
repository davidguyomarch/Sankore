/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBSTUBRECOGNIZER_H
#define UBSTUBRECOGNIZER_H

#include "IHandwritingRecognizer.h"

/**
 * @brief Stub recognizer for platforms without handwriting recognition.
 * Always returns an error indicating the feature is unavailable.
 */
class UBStubRecognizer : public IHandwritingRecognizer
{
public:
    bool isAvailable() const override { return false; }
    QString engineName() const override { return "None (not available)"; }

    UBRecognitionResult recognize(const QVector<UBRecognitionStroke>& strokes) override
    {
        Q_UNUSED(strokes);
        UBRecognitionResult result;
        result.success = false;
        result.errorMessage = "Handwriting recognition is not available on this platform.";
        return result;
    }
};

#endif // UBSTUBRECOGNIZER_H
