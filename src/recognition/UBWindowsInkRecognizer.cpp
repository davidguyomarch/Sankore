/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBWindowsInkRecognizer.h"

#ifdef Q_OS_WIN

// C++/WinRT headers
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Input.Inking.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>

// IMemoryBufferByteAccess for direct SoftwareBitmap pixel access
#include <robuffer.h>

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QtMath>
#include <optional>
#include <algorithm>

#include "core/UBSettings.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Input::Inking;

UBWindowsInkRecognizer::UBWindowsInkRecognizer()
    : mAvailable(false)
{
    try
    {
        // Try to initialize WinRT apartment. If already initialized (by Qt/COM),
        // this may throw — that's OK, we can still use the APIs.
        try {
            winrt::init_apartment(winrt::apartment_type::single_threaded);
        } catch (const winrt::hresult_error&) {
            // Already initialized — fine
        }

        // Check if recognizers are available
        InkRecognizerContainer container;
        auto recognizers = container.GetRecognizers();

        mAvailable = (recognizers.Size() > 0);
        qDebug() << "WinRT Ink: found" << recognizers.Size() << "recognizer(s)";

        for (uint32_t i = 0; i < recognizers.Size(); i++)
        {
            auto reco = recognizers.GetAt(i);
            QString name = QString::fromStdString(winrt::to_string(reco.Name()));
            mAvailableRecognizers.append(name);
            qDebug() << "  Recognizer" << i << ":" << name;
        }
    }
    catch (const winrt::hresult_error& ex)
    {
        qDebug() << "WinRT Ink: initialization failed -" << QString::fromStdString(winrt::to_string(ex.message()));
        mAvailable = false;
    }
    catch (...)
    {
        qDebug() << "WinRT Ink: initialization failed (unknown exception)";
        mAvailable = false;
    }

    if (mAvailable)
        qDebug() << "WinRT Ink recognizer available";
    else
        qDebug() << "WinRT Ink recognizer NOT available";
}

UBWindowsInkRecognizer::~UBWindowsInkRecognizer()
{
    // winrt::uninit_apartment() handled automatically
}

bool UBWindowsInkRecognizer::isAvailable() const
{
    return mAvailable;
}

QString UBWindowsInkRecognizer::engineName() const
{
    return "Windows Ink (WinRT)";
}

QString UBWindowsInkRecognizer::diagnosticInfo() const
{
    QString info;
    if (!mAvailable)
    {
        info = "WinRT Ink: NO recognizers installed.\n"
               "To fix: Settings > Time & Language > Language & Region > "
               "your language > Options > Handwriting > Download/Install.";
    }
    else
    {
        info = QString("WinRT Ink: %1 recognizer(s) available:\n").arg(mAvailableRecognizers.size());
        for (const QString& name : mAvailableRecognizers)
            info += "  - " + name + "\n";
    }
    return info;
}

/**
 * @brief Resample a polyline to have at most maxPoints evenly-spaced along arc length.
 * This ensures the recognizer gets a consistent point density regardless of drawing speed.
 */
static QVector<QPointF> resampleEquidistant(const QVector<QPointF>& points, int maxPoints)
{
    if (points.size() <= 2 || points.size() <= maxPoints)
        return points;

    // Compute cumulative arc lengths
    QVector<qreal> arcLengths;
    arcLengths.reserve(points.size());
    arcLengths.append(0.0);
    for (int i = 1; i < points.size(); ++i)
    {
        qreal dx = points[i].x() - points[i-1].x();
        qreal dy = points[i].y() - points[i-1].y();
        arcLengths.append(arcLengths.last() + qSqrt(dx*dx + dy*dy));
    }

    qreal totalLength = arcLengths.last();
    if (totalLength < 1.0)
        return points; // degenerate stroke

    // Resample at uniform intervals
    qreal interval = totalLength / (maxPoints - 1);
    QVector<QPointF> result;
    result.reserve(maxPoints);
    result.append(points.first());

    int srcIdx = 1;
    for (int i = 1; i < maxPoints - 1; ++i)
    {
        qreal targetDist = i * interval;

        // Advance srcIdx to the segment containing targetDist
        while (srcIdx < points.size() - 1 && arcLengths[srcIdx] < targetDist)
            ++srcIdx;

        // Linear interpolation within the segment [srcIdx-1, srcIdx]
        qreal segStart = arcLengths[srcIdx - 1];
        qreal segEnd = arcLengths[srcIdx];
        qreal segLen = segEnd - segStart;
        qreal t = (segLen > 0.0) ? (targetDist - segStart) / segLen : 0.0;

        QPointF interpolated = points[srcIdx - 1] * (1.0 - t) + points[srcIdx] * t;
        result.append(interpolated);
    }

    result.append(points.last());
    return result;
}

UBRecognitionResult UBWindowsInkRecognizer::recognize(const QVector<UBRecognitionStroke>& strokes)
{
    UBRecognitionResult result;

    if (!mAvailable || strokes.isEmpty())
    {
        result.success = false;
        result.errorMessage = "Recognizer not available or no strokes provided";
        return result;
    }

    try
    {
        // Filter out very short strokes (artifacts, accidental taps)
        QVector<UBRecognitionStroke> validStrokes;
        for (const auto& stroke : strokes)
        {
            if (stroke.points.size() >= 3)
                validStrokes.append(stroke);
        }

        if (validStrokes.isEmpty())
        {
            result.success = false;
            result.errorMessage = "No valid strokes (all too short)";
            return result;
        }

        // Compute bounding box for normalization
        qreal minX = validStrokes[0].points[0].x(), minY = validStrokes[0].points[0].y();
        qreal maxX = minX, maxY = minY;
        for (const auto& stroke : validStrokes)
        {
            for (const QPointF& p : stroke.points)
            {
                if (p.x() < minX) minX = p.x();
                if (p.y() < minY) minY = p.y();
                if (p.x() > maxX) maxX = p.x();
                if (p.y() > maxY) maxY = p.y();
            }
        }

        qreal sceneWidth = maxX - minX;
        qreal sceneHeight = maxY - minY;
        if (sceneWidth < 1.0) sceneWidth = 1.0;
        if (sceneHeight < 1.0) sceneHeight = 1.0;

        // Scale based on HEIGHT to ensure letters are tall enough for recognition.
        // WinRT recognizer works best when character height is ~40-100 units.
        // For horizontal text, width >> height, so scaling by max would squash letters.
        const qreal targetHeight = 80.0;
        qreal scale = targetHeight / sceneHeight;

        // Sort strokes left-to-right by their centroid X coordinate.
        // The recognizer expects chronological/reading order. If strokes arrive
        // in reverse Z-order (last drawn first), recognition fails.
        QVector<UBRecognitionStroke> sortedStrokes = validStrokes;
        std::sort(sortedStrokes.begin(), sortedStrokes.end(),
            [](const UBRecognitionStroke& a, const UBRecognitionStroke& b) {
                // Compute centroid X for each stroke
                qreal axSum = 0, bxSum = 0;
                for (const QPointF& p : a.points) axSum += p.x();
                for (const QPointF& p : b.points) bxSum += p.x();
                return (axSum / a.points.size()) < (bxSum / b.points.size());
            });

        // Run recognition on a worker thread to avoid blocking the UI message pump.
        // WinRT async operations deadlock if .get() is called on the STA UI thread.
        QString recognizedText;
        QStringList candidates;
        QString recoName;
        int totalStrokesAdded = 0;
        QString errorMsg;

        QThread* thread = QThread::create([&]() {
            try {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);

                InkStrokeBuilder builder;
                InkStrokeContainer container;

                for (const auto& stroke : sortedStrokes)
                {
                    // Equidistant resampling: resample the stroke to have evenly-spaced
                    // points along the arc length. This gives the recognizer a consistent
                    // point density regardless of drawing speed.
                    const int maxPoints = 100;
                    QVector<QPointF> resampled = resampleEquidistant(stroke.points, maxPoints);

                    if (resampled.size() < 2)
                        continue;

                    // Create InkPoints with normalized coordinates
                    std::vector<InkPoint> inkPoints;
                    for (const QPointF& p : resampled)
                    {
                        float x = (float)((p.x() - minX) * scale);
                        float y = (float)((p.y() - minY) * scale);
                        inkPoints.push_back(InkPoint(Windows::Foundation::Point(x, y), 0.5f));
                    }

                    auto inkStroke = builder.CreateStrokeFromInkPoints(
                        winrt::single_threaded_vector<InkPoint>(std::move(inkPoints)),
                        Windows::Foundation::Numerics::float3x2::identity());

                    container.AddStroke(inkStroke);
                    totalStrokesAdded++;
                }

                if (totalStrokesAdded == 0)
                {
                    errorMsg = "No valid strokes to recognize";
                    return;
                }

                // Recognize
                InkRecognizerContainer recoContainer;
                auto recognizers = recoContainer.GetRecognizers();

                // Find recognizer matching app language
                QString appLang = UBSettings::settings()->appPreferredLanguage->get().toString();
                recoName = (recognizers.Size() > 0) ?
                    QString::fromStdString(winrt::to_string(recognizers.GetAt(0).Name())) : "none";

                auto recoResults = recoContainer.RecognizeAsync(container, InkRecognitionTarget::All).get();

                // Collect results
                for (uint32_t i = 0; i < recoResults.Size(); i++)
                {
                    auto recoResult = recoResults.GetAt(i);
                    auto cands = recoResult.GetTextCandidates();
                    if (cands.Size() > 0)
                    {
                        if (!recognizedText.isEmpty()) recognizedText += " ";
                        recognizedText += QString::fromStdString(winrt::to_string(cands.GetAt(0)));

                        if (i == 0)
                        {
                            for (uint32_t j = 0; j < cands.Size() && j < 5; j++)
                                candidates.append(QString::fromStdString(winrt::to_string(cands.GetAt(j))));
                        }
                    }
                }

                winrt::uninit_apartment();
            }
            catch (const winrt::hresult_error& ex) {
                errorMsg = "WinRT error: " + QString::fromStdString(winrt::to_string(ex.message()));
            }
            catch (const std::exception& ex) {
                errorMsg = QString("Exception: ") + ex.what();
            }
        });

        thread->start();
        thread->wait(10000); // 10 second timeout
        delete thread;

        // Write diagnostic file
        {
            QString diagPath = QCoreApplication::applicationDirPath() + "/ocr_diagnostic.txt";
            QFile diagFile(diagPath);
            if (diagFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&diagFile);
                out << "=== OCR Diagnostic (WinRT) ===\n";
                out << "Input strokes: " << strokes.size() << " (valid: " << validStrokes.size() << ")\n";
                out << "Strokes sent to recognizer: " << totalStrokesAdded << "\n";
                out << "Scene bounds: X[" << minX << " .. " << maxX << "] Y[" << minY << " .. " << maxY << "]\n";
                out << "Scene size: " << sceneWidth << " x " << sceneHeight << " (scene units)\n";
                out << "Target height: " << targetHeight << " | Scale factor: " << scale << "\n";
                out << "Normalized size: " << (sceneWidth * scale) << " x " << (sceneHeight * scale) << "\n";
                out << "Max points per stroke: 100 (equidistant resampling)\n";
                out << "Stroke order: sorted left-to-right by centroid X\n";
                out << "Recognizer: " << recoName << "\n";
                out << "Result: \"" << recognizedText << "\"\n";
                if (!candidates.isEmpty())
                    out << "Candidates: " << candidates.join(", ") << "\n";
                if (!errorMsg.isEmpty())
                    out << "Error: " << errorMsg << "\n";

                // Append stroke dump data for single-file diagnostics
                out << "\n=== STROKE DUMP (sorted order) ===\n";
                out << "STROKES " << sortedStrokes.size() << "\n";
                for (int i = 0; i < sortedStrokes.size(); i++)
                {
                    out << "STROKE " << i << " POINTS " << sortedStrokes[i].points.size() << "\n";
                    for (const QPointF& p : sortedStrokes[i].points)
                        out << p.x() << " " << p.y() << "\n";
                }
                diagFile.close();
            }
        }

        if (!errorMsg.isEmpty())
        {
            result.success = false;
            result.errorMessage = errorMsg;
            qDebug() << "OCR error:" << errorMsg;
        }
        else if (recognizedText.isEmpty())
        {
            result.success = false;
            result.errorMessage = "Recognition produced no text";
        }
        else
        {
            result.success = true;
            result.text = recognizedText;
            result.candidates = candidates;
        }
    }
    catch (const std::exception& ex)
    {
        result.success = false;
        result.errorMessage = QString("Exception: ") + ex.what();
        qDebug() << "OCR exception:" << result.errorMessage;
    }

    return result;
}

UBRecognitionResult UBWindowsInkRecognizer::recognizeImage(const QImage& image)
{
    UBRecognitionResult result;

    if (image.isNull())
    {
        result.success = false;
        result.errorMessage = "Empty image provided";
        return result;
    }

    try
    {
        QString recognizedText;
        QString errorMsg;

        QThread* thread = QThread::create([&]() {
            try {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);

                // Use fully qualified names to avoid ambiguity with ::Windows from MemoryBuffer.h
                auto engine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
                if (!engine)
                {
                    errorMsg = "OcrEngine not available (no language pack installed)";
                    return;
                }

                // Convert QImage to WinRT SoftwareBitmap
                QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);
                int w = rgbaImage.width();
                int h = rgbaImage.height();

                winrt::Windows::Graphics::Imaging::SoftwareBitmap bitmap(
                    winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Rgba8, w, h,
                    winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied);
                {
                    auto buffer = bitmap.LockBuffer(
                        winrt::Windows::Graphics::Imaging::BitmapBufferAccessMode::Write);
                    auto ref = buffer.CreateReference();

                    // Get raw pixel pointer via IMemoryBufferByteAccess
                    auto byteAccess = ref.as<::Windows::Foundation::IMemoryBufferByteAccess>();
                    uint8_t* dstData = nullptr;
                    uint32_t dstCapacity = 0;
                    winrt::check_hresult(byteAccess->GetBuffer(&dstData, &dstCapacity));

                    // Copy pixel data row by row
                    int srcStride = rgbaImage.bytesPerLine();
                    int dstStride = w * 4;
                    for (int y = 0; y < h; ++y)
                    {
                        memcpy(dstData + y * dstStride,
                               rgbaImage.constScanLine(y),
                               qMin(srcStride, dstStride));
                    }
                }

                // Recognize
                auto ocrResult = engine.RecognizeAsync(bitmap).get();
                recognizedText = QString::fromStdString(winrt::to_string(ocrResult.Text()));

                winrt::uninit_apartment();
            }
            catch (const winrt::hresult_error& ex) {
                errorMsg = "WinRT OCR image error: " + QString::fromStdString(winrt::to_string(ex.message()));
            }
            catch (const std::exception& ex) {
                errorMsg = QString("Exception: ") + ex.what();
            }
        });

        thread->start();
        thread->wait(15000); // 15 second timeout for image OCR
        delete thread;

        if (!errorMsg.isEmpty())
        {
            result.success = false;
            result.errorMessage = errorMsg;
        }
        else if (recognizedText.isEmpty())
        {
            result.success = false;
            result.errorMessage = "Image OCR produced no text";
        }
        else
        {
            result.success = true;
            result.text = recognizedText.trimmed();
        }
    }
    catch (const std::exception& ex)
    {
        result.success = false;
        result.errorMessage = QString("Exception: ") + ex.what();
    }

    return result;
}

#endif // Q_OS_WIN
