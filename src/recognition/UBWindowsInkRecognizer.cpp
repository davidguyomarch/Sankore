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

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

#include "core/UBSettings.h"

using namespace winrt;
using namespace Windows::Foundation;
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
        // Compute bounding box for normalization
        qreal minX = strokes[0].points[0].x(), minY = strokes[0].points[0].y();
        qreal maxX = minX, maxY = minY;
        for (const auto& stroke : strokes)
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

        // WinRT Ink uses DIP (device-independent pixels, 96 DPI).
        // Typical handwriting on screen: letters ~30-50 DIP tall.
        // Scale so that writing height maps to ~40 DIP.
        const qreal targetHeight = 40.0;
        qreal scale = targetHeight / sceneHeight;

        // Build ink strokes using InkStrokeBuilder
        InkStrokeBuilder builder;
        InkStrokeContainer container;

        int totalStrokesAdded = 0;

        for (const auto& stroke : strokes)
        {
            if (stroke.points.size() < 2)
                continue;

            // Subsample to max 40 points per stroke
            QVector<QPointF> simplified;
            int numOriginal = stroke.points.size();
            int maxPoints = 40;

            if (numOriginal <= maxPoints)
            {
                simplified = stroke.points;
            }
            else
            {
                simplified.append(stroke.points.first());
                double step = (double)(numOriginal - 1) / (double)(maxPoints - 1);
                for (int i = 1; i < maxPoints - 1; i++)
                {
                    int idx = (int)(i * step + 0.5);
                    if (idx < numOriginal)
                        simplified.append(stroke.points[idx]);
                }
                simplified.append(stroke.points.last());
            }

            // Create InkPoints
            std::vector<InkPoint> inkPoints;
            for (const QPointF& p : simplified)
            {
                float x = (float)((p.x() - minX) * scale);
                float y = (float)((p.y() - minY) * scale);
                inkPoints.push_back(InkPoint(Windows::Foundation::Point(x, y), 0.5f));
            }

            // Create the stroke
            auto inkStroke = builder.CreateStrokeFromInkPoints(
                winrt::single_threaded_vector<InkPoint>(std::move(inkPoints)),
                Windows::Foundation::Numerics::float3x2::identity());

            container.AddStroke(inkStroke);
            totalStrokesAdded++;
        }

        if (totalStrokesAdded == 0)
        {
            result.success = false;
            result.errorMessage = "No valid strokes to recognize";
            return result;
        }

        // Select recognizer based on app language
        InkRecognizerContainer recoContainer;
        auto recognizers = recoContainer.GetRecognizers();

        // Find recognizer matching app language
        QString appLang = UBSettings::settings()->appPreferredLanguage->get().toString();
        InkRecognizer selectedRecognizer{nullptr};

        if (!appLang.isEmpty())
        {
            for (uint32_t i = 0; i < recognizers.Size(); i++)
            {
                auto reco = recognizers.GetAt(i);
                QString name = QString::fromStdString(winrt::to_string(reco.Name()));
                if (name.contains(appLang, Qt::CaseInsensitive) ||
                    name.contains(appLang.left(2), Qt::CaseInsensitive))
                {
                    selectedRecognizer = reco;
                    break;
                }
            }
        }

        // Fallback to first recognizer if no language match
        if (!selectedRecognizer && recognizers.Size() > 0)
            selectedRecognizer = recognizers.GetAt(0);

        QString recoName = selectedRecognizer ?
            QString::fromStdString(winrt::to_string(selectedRecognizer.Name())) : "none";
        qDebug() << "OCR: using recognizer:" << recoName << "(app lang:" << appLang << ")";

        // Recognize
        auto recoResults = recoContainer.RecognizeAsync(container, InkRecognitionTarget::All).get();

        // Collect results
        QString fullText;
        for (uint32_t i = 0; i < recoResults.Size(); i++)
        {
            auto recoResult = recoResults.GetAt(i);
            auto candidates = recoResult.GetTextCandidates();
            if (candidates.Size() > 0)
            {
                if (!fullText.isEmpty()) fullText += " ";
                fullText += QString::fromStdString(winrt::to_string(candidates.GetAt(0)));

                // Store alternatives from first result only
                if (i == 0)
                {
                    for (uint32_t j = 0; j < candidates.Size() && j < 5; j++)
                        result.candidates.append(QString::fromStdString(winrt::to_string(candidates.GetAt(j))));
                }
            }
        }

        // Write diagnostic file
        {
            QString diagPath = QCoreApplication::applicationDirPath() + "/ocr_diagnostic.txt";
            QFile diagFile(diagPath);
            if (diagFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&diagFile);
                out << "OCR Diagnostic (WinRT)\n";
                out << "Strokes added: " << totalStrokesAdded << "\n";
                out << "Scale used: " << scale << "\n";
                out << "Scene bounds: X[" << minX << "," << maxX << "] Y[" << minY << "," << maxY << "]\n";
                out << "Scene size: " << sceneWidth << " x " << sceneHeight << "\n";
                out << "Recognizer: " << recoName << "\n";
                out << "Recognition results count: " << recoResults.Size() << "\n";
                out << "Full text: \"" << fullText << "\"\n";
                diagFile.close();
            }
        }

        if (fullText.isEmpty())
        {
            result.success = false;
            result.errorMessage = "Recognition produced no text";
        }
        else
        {
            result.success = true;
            result.text = fullText;
        }
    }
    catch (const winrt::hresult_error& ex)
    {
        result.success = false;
        result.errorMessage = "WinRT error: " + QString::fromStdString(winrt::to_string(ex.message()));
        qDebug() << "OCR WinRT error:" << result.errorMessage;
    }
    catch (const std::exception& ex)
    {
        result.success = false;
        result.errorMessage = QString("Exception: ") + ex.what();
        qDebug() << "OCR exception:" << result.errorMessage;
    }

    return result;
}

#endif // Q_OS_WIN
