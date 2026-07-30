/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef IHANDWRITINGRECOGNIZER_H
#define IHANDWRITINGRECOGNIZER_H

#include <QString>
#include <QPointF>
#include <QVector>

/**
 * @brief A single stroke: ordered list of points captured from the stylus.
 */
struct UBRecognitionStroke
{
    QVector<QPointF> points;
};

/**
 * @brief Result of a handwriting recognition attempt.
 */
struct UBRecognitionResult
{
    bool success = false;
    QString text;           // best recognized text
    QStringList candidates; // alternative interpretations (if available)
    QString errorMessage;   // human-readable error if !success
};

/**
 * @brief Abstract interface for handwriting recognition engines.
 *
 * Platform-specific implementations:
 *   - UBWindowsInkRecognizer (Windows 10+ via COM IInkRecognizerContext)
 *   - UBStubRecognizer (Linux/Mac fallback)
 */
class IHandwritingRecognizer
{
public:
    virtual ~IHandwritingRecognizer() {}

    /// Returns true if the recognizer is available on this platform.
    virtual bool isAvailable() const = 0;

    /// Returns the display name of the engine (e.g. "Windows Ink").
    virtual QString engineName() const = 0;

    /// Recognize handwriting from a list of strokes.
    virtual UBRecognitionResult recognize(const QVector<UBRecognitionStroke>& strokes) = 0;

    /// Factory: create the best available recognizer for this platform.
    static IHandwritingRecognizer* createDefault();
};

#endif // IHANDWRITINGRECOGNIZER_H
