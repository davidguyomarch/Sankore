/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 *
 * Image-based OCR using Windows.Media.Ocr (WinRT).
 * This is a SEPARATE compilation unit from UBWindowsInkRecognizer.cpp
 * because the Windows.Graphics.Imaging headers require IMemoryBufferByteAccess
 * which conflicts with the C++/WinRT IUnknown when both are in the same TU.
 */

#include "IHandwritingRecognizer.h"

#ifdef Q_OS_WIN

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

#include <QDebug>
#include <QImage>
#include <QTemporaryFile>
#include <QThread>
#include <QDir>
#include <QFile>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Ocr;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

UBRecognitionResult ubWindowsImageOcrRecognize(const QImage& image)
{
    UBRecognitionResult result;

    if (image.isNull())
    {
        result.success = false;
        result.errorMessage = "Empty image provided";
        return result;
    }

    // Save QImage to a temporary BMP file, then load via WinRT file APIs.
    // This avoids the IMemoryBufferByteAccess COM interop nightmare.
    QString tempPath = QDir::tempPath() + "/sankore_ocr_temp.bmp";
    if (!image.save(tempPath, "BMP"))
    {
        result.success = false;
        result.errorMessage = "Failed to save temp image for OCR";
        return result;
    }

    QString recognizedText;
    QString errorMsg;

    QThread* thread = QThread::create([&]() {
        try {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);

            // Create OcrEngine
            auto engine = OcrEngine::TryCreateFromUserProfileLanguages();
            if (!engine)
            {
                errorMsg = "OcrEngine not available (no language pack installed)";
                return;
            }

            // Open temp file via WinRT StorageFile
            std::wstring wpath = tempPath.toStdWString();
            auto file = StorageFile::GetFileFromPathAsync(wpath).get();
            auto stream = file.OpenAsync(FileAccessMode::Read).get();

            // Decode to SoftwareBitmap
            auto decoder = BitmapDecoder::CreateAsync(stream).get();
            auto bitmap = decoder.GetSoftwareBitmapAsync().get();

            // OcrEngine requires Bgra8 + Premultiplied
            auto convertedBitmap = SoftwareBitmap::Convert(
                bitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);

            // Recognize
            auto ocrResult = engine.RecognizeAsync(convertedBitmap).get();
            recognizedText = QString::fromStdWString(std::wstring(ocrResult.Text()));

            stream.Close();
            winrt::uninit_apartment();
        }
        catch (const winrt::hresult_error& ex) {
            errorMsg = "WinRT image OCR error: " + QString::fromStdWString(std::wstring(ex.message()));
        }
        catch (const std::exception& ex) {
            errorMsg = QString("Exception: ") + ex.what();
        }
    });

    thread->start();
    thread->wait(15000);
    delete thread;

    // Clean up temp file
    QFile::remove(tempPath);

    if (!errorMsg.isEmpty())
    {
        result.success = false;
        result.errorMessage = errorMsg;
        qDebug() << "Image OCR error:" << errorMsg;
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
        qDebug() << "Image OCR success:" << result.text;
    }

    return result;
}

#else // !Q_OS_WIN

UBRecognitionResult ubWindowsImageOcrRecognize(const QImage& image)
{
    Q_UNUSED(image);
    UBRecognitionResult result;
    result.success = false;
    result.errorMessage = "Image OCR not available on this platform";
    return result;
}

#endif // Q_OS_WIN
