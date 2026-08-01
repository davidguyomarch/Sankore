/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBZinniaRecognizer.h"

#ifndef Q_OS_WIN

#include <QDebug>
#include <QFile>
#include <QDir>
#include <algorithm>
#include <cmath>

// Zinnia C API
#include <zinnia.h>

UBZinniaRecognizer::UBZinniaRecognizer()
    : mAvailable(false)
    , mRecognizer(nullptr)
{
    // Search for model file in common locations
    QStringList modelPaths = {
        "/usr/share/zinnia/model/tomoe/handwriting-en.model",
        "/usr/share/tegaki/models/zinnia/handwriting-en.model",
        "/usr/local/share/zinnia/model/tomoe/handwriting-en.model",
        "/usr/share/zinnia/model/tomoe/handwriting-ja.model", // fallback to Japanese
        QDir::homePath() + "/.local/share/zinnia/handwriting-en.model"
    };

    for (const QString& path : modelPaths)
    {
        if (QFile::exists(path))
        {
            mModelPath = path;
            break;
        }
    }

    if (mModelPath.isEmpty())
    {
        qDebug() << "Zinnia: no model file found";
        return;
    }

    // Create recognizer
    zinnia_recognizer_t* reco = zinnia_recognizer_new();
    if (!reco)
    {
        qDebug() << "Zinnia: failed to create recognizer";
        return;
    }

    if (!zinnia_recognizer_open(reco, mModelPath.toUtf8().constData()))
    {
        qDebug() << "Zinnia: failed to open model:" << zinnia_recognizer_strerror(reco);
        zinnia_recognizer_destroy(reco);
        return;
    }

    mRecognizer = reco;
    mAvailable = true;
    qDebug() << "Zinnia recognizer initialized with model:" << mModelPath;
}

UBZinniaRecognizer::~UBZinniaRecognizer()
{
    if (mRecognizer)
        zinnia_recognizer_destroy((zinnia_recognizer_t*)mRecognizer);
}

bool UBZinniaRecognizer::isAvailable() const
{
    return mAvailable;
}

QString UBZinniaRecognizer::engineName() const
{
    return "Zinnia";
}

UBRecognitionResult UBZinniaRecognizer::recognize(const QVector<UBRecognitionStroke>& strokes)
{
    UBRecognitionResult result;

    if (!mAvailable || strokes.isEmpty())
    {
        result.success = false;
        result.errorMessage = "Zinnia recognizer not available or no strokes";
        return result;
    }

    // Segment strokes into individual characters
    QVector<QVector<UBRecognitionStroke>> characters = segmentIntoCharacters(strokes);

    if (characters.isEmpty())
    {
        result.success = false;
        result.errorMessage = "Could not segment strokes into characters";
        return result;
    }

    // Recognize each character
    QString fullText;
    for (const auto& charStrokes : characters)
    {
        QString ch = recognizeCharacter(charStrokes);
        if (!ch.isEmpty())
            fullText += ch;
    }

    if (fullText.isEmpty())
    {
        result.success = false;
        result.errorMessage = "Recognition returned empty result";
        return result;
    }

    result.success = true;
    result.text = fullText;
    return result;
}

QVector<QVector<UBRecognitionStroke>> UBZinniaRecognizer::segmentIntoCharacters(const QVector<UBRecognitionStroke>& strokes)
{
    QVector<QVector<UBRecognitionStroke>> characters;

    if (strokes.isEmpty())
        return characters;

    // Simple segmentation: group strokes that overlap horizontally
    // A new character starts when there's a significant horizontal gap

    struct StrokeBounds {
        qreal minX, maxX;
        int strokeIndex;
    };

    QVector<StrokeBounds> bounds;
    for (int i = 0; i < strokes.size(); i++)
    {
        if (strokes[i].points.isEmpty())
            continue;
        qreal minX = strokes[i].points[0].x();
        qreal maxX = minX;
        for (const QPointF& p : strokes[i].points)
        {
            minX = std::min(minX, p.x());
            maxX = std::max(maxX, p.x());
        }
        bounds.append({minX, maxX, i});
    }

    if (bounds.isEmpty())
        return characters;

    // Sort by minX
    std::sort(bounds.begin(), bounds.end(), [](const StrokeBounds& a, const StrokeBounds& b) {
        return a.minX < b.minX;
    });

    // Calculate average stroke width for gap threshold
    qreal totalWidth = 0;
    for (const auto& b : bounds)
        totalWidth += (b.maxX - b.minX);
    qreal avgWidth = totalWidth / bounds.size();
    qreal gapThreshold = avgWidth * 0.8; // 80% of average width = gap between chars

    // Group strokes
    QVector<UBRecognitionStroke> currentGroup;
    qreal currentMaxX = bounds[0].maxX;
    currentGroup.append(strokes[bounds[0].strokeIndex]);

    for (int i = 1; i < bounds.size(); i++)
    {
        qreal gap = bounds[i].minX - currentMaxX;
        if (gap > gapThreshold)
        {
            // New character
            characters.append(currentGroup);
            currentGroup.clear();
        }
        currentGroup.append(strokes[bounds[i].strokeIndex]);
        currentMaxX = std::max(currentMaxX, bounds[i].maxX);
    }
    if (!currentGroup.isEmpty())
        characters.append(currentGroup);

    return characters;
}

QString UBZinniaRecognizer::recognizeCharacter(const QVector<UBRecognitionStroke>& charStrokes)
{
    if (!mRecognizer || charStrokes.isEmpty())
        return QString();

    zinnia_recognizer_t* reco = (zinnia_recognizer_t*)mRecognizer;

    // Create a character (canvas) for Zinnia
    zinnia_character_t* character = zinnia_character_new();
    if (!character)
        return QString();

    // Determine bounding box to normalize coordinates to 0-300 range
    qreal minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& stroke : charStrokes)
    {
        for (const QPointF& p : stroke.points)
        {
            minX = std::min(minX, p.x());
            maxX = std::max(maxX, p.x());
            minY = std::min(minY, p.y());
            maxY = std::max(maxY, p.y());
        }
    }

    qreal width = maxX - minX;
    qreal height = maxY - minY;
    if (width < 1.0) width = 1.0;
    if (height < 1.0) height = 1.0;

    // Zinnia expects a square canvas (typically 300x300)
    const int canvasSize = 300;
    zinnia_character_set_width(character, canvasSize);
    zinnia_character_set_height(character, canvasSize);

    // Scale factor to fit in canvas with margin
    qreal scale = (canvasSize - 20) / std::max(width, height);
    qreal offsetX = (canvasSize - width * scale) / 2.0 - minX * scale;
    qreal offsetY = (canvasSize - height * scale) / 2.0 - minY * scale;

    // Add strokes
    for (int s = 0; s < charStrokes.size(); s++)
    {
        for (int i = 0; i < charStrokes[s].points.size(); i++)
        {
            int x = (int)(charStrokes[s].points[i].x() * scale + offsetX);
            int y = (int)(charStrokes[s].points[i].y() * scale + offsetY);
            zinnia_character_add(character, s, x, y);
        }
    }

    // Classify
    zinnia_result_t* res = zinnia_recognizer_classify(reco, character, 5);
    QString bestResult;

    if (res)
    {
        if (zinnia_result_size(res) > 0)
            bestResult = QString::fromUtf8(zinnia_result_value(res, 0));
        zinnia_result_destroy(res);
    }

    zinnia_character_destroy(character);
    return bestResult;
}

#endif // !Q_OS_WIN
