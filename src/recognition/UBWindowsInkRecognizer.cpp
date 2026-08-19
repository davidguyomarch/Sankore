/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBWindowsInkRecognizer.h"

#ifdef Q_OS_WIN

#include <QDebug>
#include <comdef.h>
#include <msinkaut.h>
#include <msinkaut_i.c>
#include <limits>
#include <winnls.h>

#include "core/UBSettings.h"

#pragma comment(lib, "ole32.lib")

UBWindowsInkRecognizer::UBWindowsInkRecognizer()
    : mAvailable(false)
    , mComInitialized(false)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE)
    {
        mComInitialized = (hr != RPC_E_CHANGED_MODE);

        // Test if ink recognizer is available
        IInkRecognizers* recognizers = nullptr;
        hr = CoCreateInstance(CLSID_InkRecognizers, NULL, CLSCTX_INPROC_SERVER,
                             IID_IInkRecognizers, (void**)&recognizers);
        if (SUCCEEDED(hr) && recognizers)
        {
            long count = 0;
            recognizers->get_Count(&count);
            mAvailable = (count > 0);
            qDebug() << "Windows Ink: found" << count << "recognizer(s)";

            // List all available recognizers for diagnostic
            for (long i = 0; i < count; i++)
            {
                IInkRecognizer* reco = nullptr;
                hr = recognizers->Item(i, &reco);
                if (SUCCEEDED(hr) && reco)
                {
                    BSTR bstrName = nullptr;
                    reco->get_Name(&bstrName);
                    short langCount = 0;
                    VARIANT varLangs;
                    VariantInit(&varLangs);
                    reco->get_Languages(&varLangs);

                    QString name = bstrName ? QString::fromWCharArray(bstrName) : "unnamed";
                    mAvailableRecognizers.append(name);
                    qDebug() << "  Recognizer" << i << ":" << name;

                    if (bstrName) SysFreeString(bstrName);
                    VariantClear(&varLangs);
                    reco->Release();
                }
            }
            recognizers->Release();
        }
        else
        {
            qDebug() << "Windows Ink: CoCreateInstance failed, hr=" << QString::number((unsigned long)hr, 16);
        }
    }
    else
    {
        qDebug() << "Windows Ink: CoInitializeEx failed, hr=" << QString::number((unsigned long)hr, 16);
    }

    if (mAvailable)
        qDebug() << "Windows Ink recognizer available";
    else
        qDebug() << "Windows Ink recognizer NOT available";
}

UBWindowsInkRecognizer::~UBWindowsInkRecognizer()
{
    if (mComInitialized)
        CoUninitialize();
}

bool UBWindowsInkRecognizer::isAvailable() const
{
    return mAvailable;
}

QString UBWindowsInkRecognizer::engineName() const
{
    return "Windows Ink";
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

    // Create InkCollector and add strokes
    IInkDisp* inkDisp = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_InkDisp, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IInkDisp, (void**)&inkDisp);
    if (FAILED(hr) || !inkDisp)
    {
        result.success = false;
        result.errorMessage = "Failed to create InkDisp object";
        return result;
    }

    IInkStrokes* inkStrokes = nullptr;

    // Add each stroke as an array of points
    // First, compute bounding box of ALL strokes to normalize coordinates
    qreal minX = (std::numeric_limits<qreal>::max)();
    qreal minY = (std::numeric_limits<qreal>::max)();
    qreal maxX = (std::numeric_limits<qreal>::lowest)();
    qreal maxY = (std::numeric_limits<qreal>::lowest)();

    for (const UBRecognitionStroke& stroke : strokes)
    {
        for (const QPointF& p : stroke.points)
        {
            minX = qMin(minX, p.x());
            minY = qMin(minY, p.y());
            maxX = qMax(maxX, p.x());
            maxY = qMax(maxY, p.y());
        }
    }

    qreal sceneWidth = maxX - minX;
    qreal sceneHeight = maxY - minY;
    if (sceneWidth < 1.0) sceneWidth = 1.0;
    if (sceneHeight < 1.0) sceneHeight = 1.0;

    // Windows Ink HIMETRIC: typical handwriting character height is ~2000-4000 units.
    // Scale based on the HEIGHT of the writing (not width) to normalize character size.
    // A single line of handwriting should be ~3000 HIMETRIC tall.
    const qreal targetHeight = 3000.0;
    qreal scale = targetHeight / sceneHeight;

    for (const UBRecognitionStroke& stroke : strokes)
    {
        if (stroke.points.size() < 2)
            continue;

        int numPoints = stroke.points.size();

        // CreateStroke expects a 1D SAFEARRAY of LONG: x1,y1,x2,y2,...
        VARIANT varPoints;
        VariantInit(&varPoints);
        varPoints.vt = VT_ARRAY | VT_I4;

        SAFEARRAYBOUND bound;
        bound.lLbound = 0;
        bound.cElements = numPoints * 2;

        varPoints.parray = SafeArrayCreate(VT_I4, 1, &bound);

        LONG* pData = nullptr;
        SafeArrayAccessData(varPoints.parray, (void**)&pData);
        for (int i = 0; i < numPoints; i++)
        {
            // Normalize: shift to origin (0,0) then scale to HIMETRIC
            pData[i * 2]     = (LONG)((stroke.points[i].x() - minX) * scale);
            pData[i * 2 + 1] = (LONG)((stroke.points[i].y() - minY) * scale);
        }
        SafeArrayUnaccessData(varPoints.parray);

        // Add stroke to ink
        IInkStrokeDisp* newStroke = nullptr;
        VARIANT varPacketDesc;
        VariantInit(&varPacketDesc); // empty = default (x, y only)

        hr = inkDisp->CreateStroke(varPoints, varPacketDesc, &newStroke);
        if (SUCCEEDED(hr) && newStroke)
            newStroke->Release();

        VariantClear(&varPoints);
    }

    // Get default recognizer
    IInkRecognizers* recognizers = nullptr;
    hr = CoCreateInstance(CLSID_InkRecognizers, NULL, CLSCTX_INPROC_SERVER,
                         IID_IInkRecognizers, (void**)&recognizers);
    if (FAILED(hr) || !recognizers)
    {
        if (inkStrokes) inkStrokes->Release();
        inkDisp->Release();
        result.success = false;
        result.errorMessage = "Failed to get recognizers";
        return result;
    }

    IInkRecognizer* defaultRecognizer = nullptr;

    // Use the language set in Sankoré preferences for recognition
    LCID recognizerLcid = 0; // 0 = system default
    QString appLang = UBSettings::settings()->appPreferredLanguage->get().toString();
    if (!appLang.isEmpty())
    {
        // Convert ISO language code (e.g. "fr", "en", "de") to Windows LCID
        // LocaleNameToLCID expects BCP 47 tags like "fr", "en-US", etc.
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        appLang.toWCharArray(localeName);
        localeName[appLang.length()] = 0;
        LCID lcid = LocaleNameToLCID(localeName, 0);
        if (lcid != 0)
            recognizerLcid = lcid;

        qDebug() << "OCR: app language =" << appLang << "-> LCID =" << recognizerLcid;
    }

    hr = recognizers->GetDefaultRecognizer(recognizerLcid, &defaultRecognizer);
    if (FAILED(hr) || !defaultRecognizer)
    {
        // Fallback: try system default if app language recognizer not found
        if (recognizerLcid != 0)
        {
            qDebug() << "OCR: no recognizer for LCID" << recognizerLcid << ", trying system default";
            hr = recognizers->GetDefaultRecognizer(0, &defaultRecognizer);
        }
    }
    if (FAILED(hr) || !defaultRecognizer)
    {
        recognizers->Release();
        if (inkStrokes) inkStrokes->Release();
        inkDisp->Release();
        result.success = false;
        result.errorMessage = "No default recognizer found";
        return result;
    }

    // Create recognizer context
    IInkRecognizerContext* context = nullptr;
    hr = defaultRecognizer->CreateRecognizerContext(&context);

    // Log which recognizer was selected
    {
        BSTR bstrName = nullptr;
        defaultRecognizer->get_Name(&bstrName);
        if (bstrName) {
            qDebug() << "OCR: using recognizer:" << QString::fromWCharArray(bstrName);
            SysFreeString(bstrName);
        }
    }

    if (FAILED(hr) || !context)
    {
        defaultRecognizer->Release();
        recognizers->Release();
        if (inkStrokes) inkStrokes->Release();
        inkDisp->Release();
        result.success = false;
        result.errorMessage = "Failed to create recognizer context";
        return result;
    }

    // Assign strokes to context
    inkDisp->get_Strokes(&inkStrokes);
    context->putref_Strokes(inkStrokes);

    // Recognize
    IInkRecognitionResult* recoResult = nullptr;
    InkRecognitionStatus status;
    hr = context->Recognize(&status, &recoResult);

    if (SUCCEEDED(hr) && recoResult && status == IRS_NoError)
    {
        BSTR bstrResult = nullptr;
        recoResult->get_TopString(&bstrResult);
        if (bstrResult)
        {
            result.success = true;
            result.text = QString::fromWCharArray(bstrResult);
            SysFreeString(bstrResult);
        }

        // Get alternates
        IInkRecognitionAlternates* alternates = nullptr;
        hr = recoResult->AlternatesFromSelection(0, -1, 5, &alternates);
        if (SUCCEEDED(hr) && alternates)
        {
            long altCount = 0;
            alternates->get_Count(&altCount);
            for (long i = 0; i < altCount && i < 5; i++)
            {
                IInkRecognitionAlternate* alt = nullptr;
                alternates->Item(i, &alt);
                if (alt)
                {
                    BSTR bstrAlt = nullptr;
                    alt->get_String(&bstrAlt);
                    if (bstrAlt)
                    {
                        result.candidates.append(QString::fromWCharArray(bstrAlt));
                        SysFreeString(bstrAlt);
                    }
                    alt->Release();
                }
            }
            alternates->Release();
        }

        recoResult->Release();
    }
    else
    {
        result.success = false;
        result.errorMessage = "Recognition failed (status: " + QString::number((int)status) + ")";
    }

    // Cleanup
    context->Release();
    defaultRecognizer->Release();
    recognizers->Release();
    inkStrokes->Release();
    inkDisp->Release();

    return result;
}

QString UBWindowsInkRecognizer::diagnosticInfo() const
{
    QString info;
    if (!mAvailable)
    {
        info = "Windows Ink: NO recognizers installed.\n"
               "To fix: Settings > Time & Language > Language & Region > "
               "your language > Options > Handwriting > Download/Install.";
    }
    else
    {
        info = QString("Windows Ink: %1 recognizer(s) available:\n").arg(mAvailableRecognizers.size());
        for (const QString& name : mAvailableRecognizers)
            info += "  - " + name + "\n";
    }
    return info;
}

#endif // Q_OS_WIN
