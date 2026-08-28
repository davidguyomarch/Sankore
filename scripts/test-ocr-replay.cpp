/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// test-ocr-replay.cpp — Reads ocr_strokes_dump.txt and replays through Windows Ink
// Compile: cl /EHsc /std:c++17 test-ocr-replay.cpp ole32.lib oleaut32.lib
// Run: test-ocr-replay.exe C:\Sankore\ocr_strokes_dump.txt

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <comdef.h>
#include <msinkaut.h>
#include <msinkaut_i.c>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

struct Point { double x, y; };
struct Stroke { std::vector<Point> points; };

std::vector<Stroke> readDumpFile(const char* path)
{
    std::vector<Stroke> strokes;
    FILE* f = fopen(path, "r");
    if (!f) { printf("Cannot open %s\n", path); return strokes; }

    char line[256];
    Stroke current;
    int readingPoints = 0;

    while (fgets(line, sizeof(line), f))
    {
        int n;
        if (sscanf(line, "STROKE %*d POINTS %d", &n) == 1)
        {
            if (!current.points.empty())
                strokes.push_back(current);
            current.points.clear();
            readingPoints = n;
        }
        else if (readingPoints > 0)
        {
            double x, y;
            if (sscanf(line, "%lf %lf", &x, &y) == 2)
                current.points.push_back({x, y});
        }
    }
    if (!current.points.empty())
        strokes.push_back(current);

    fclose(f);
    return strokes;
}

int main(int argc, char* argv[])
{
    const char* dumpPath = argc > 1 ? argv[1] : "C:\\Sankore\\ocr_strokes_dump.txt";

    printf("=== OCR Replay Test ===\n");
    printf("Reading: %s\n\n", dumpPath);

    auto strokes = readDumpFile(dumpPath);
    printf("Loaded %zu strokes\n", strokes.size());
    for (size_t i = 0; i < strokes.size(); i++)
    {
        printf("  Stroke %zu: %zu points", i, strokes[i].points.size());
        if (!strokes[i].points.empty())
        {
            auto& pts = strokes[i].points;
            double minX=pts[0].x, maxX=pts[0].x, minY=pts[0].y, maxY=pts[0].y;
            for (auto& p : pts) {
                if (p.x < minX) minX = p.x; if (p.x > maxX) maxX = p.x;
                if (p.y < minY) minY = p.y; if (p.y > maxY) maxY = p.y;
            }
            printf(" [%.0f,%.0f - %.0f,%.0f] size=%.0fx%.0f", minX, minY, maxX, maxY, maxX-minX, maxY-minY);
        }
        printf("\n");
    }

    if (strokes.empty()) { printf("No strokes to process\n"); return 1; }

    // Try different scale factors
    // First compute bounding box for normalization
    double gMinX = strokes[0].points[0].x, gMaxX = gMinX;
    double gMinY = strokes[0].points[0].y, gMaxY = gMinY;
    for (auto& s : strokes) {
        for (auto& p : s.points) {
            if (p.x < gMinX) gMinX = p.x; if (p.x > gMaxX) gMaxX = p.x;
            if (p.y < gMinY) gMinY = p.y; if (p.y > gMaxY) gMaxY = p.y;
        }
    }
    double sceneW = gMaxX - gMinX; if (sceneW < 1) sceneW = 1;
    double sceneH = gMaxY - gMinY; if (sceneH < 1) sceneH = 1;
    printf("\nBounding box: X[%.1f, %.1f] Y[%.1f, %.1f] size=%.1f x %.1f\n", gMinX, gMinY, gMaxX, gMaxY, sceneW, sceneH);

    // Scale by height to 3000 HIMETRIC (like app does)
    double normalizedScale = 3000.0 / sceneH;
    double scales[] = {1.0, 10.0, 50.0, 100.0, normalizedScale};
    const char* scaleNames[] = {"1.0", "10.0", "50.0", "100.0", "normalized(height=3000)"};

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { printf("COM init failed\n"); return 1; }

    IInkRecognizers* recognizers = nullptr;
    CoCreateInstance(CLSID_InkRecognizers, NULL, CLSCTX_INPROC_SERVER, IID_IInkRecognizers, (void**)&recognizers);

    // List recognizers
    long recoCount = 0;
    recognizers->get_Count(&recoCount);
    printf("\nAvailable recognizers: %ld\n", recoCount);
    for (long i = 0; i < recoCount; i++) {
        IInkRecognizer* r = nullptr;
        recognizers->Item(i, &r);
        if (r) {
            BSTR name = nullptr;
            r->get_Name(&name);
            if (name) { wprintf(L"  [%ld] %s\n", i, name); SysFreeString(name); }
            r->Release();
        }
    }

    IInkRecognizer* defaultReco = nullptr;
    recognizers->GetDefaultRecognizer(0, &defaultReco);
    {
        BSTR name = nullptr;
        defaultReco->get_Name(&name);
        if (name) { wprintf(L"\nUsing recognizer: %s\n", name); SysFreeString(name); }
    }

    for (int si = 0; si < 5; si++)
    {
        double scale = scales[si];
        printf("\n--- Scale: %s (%.2f) --- [shifted to 0,0]\n", scaleNames[si], scale);

        IInkDisp* inkDisp = nullptr;
        CoCreateInstance(CLSID_InkDisp, NULL, CLSCTX_INPROC_SERVER, IID_IInkDisp, (void**)&inkDisp);

        for (auto& stroke : strokes)
        {
            if (stroke.points.size() < 2) continue;

            int n = (int)stroke.points.size();
            VARIANT varPoints;
            VariantInit(&varPoints);
            varPoints.vt = VT_ARRAY | VT_I4;

            SAFEARRAYBOUND bound = {(ULONG)(n * 2), 0};
            varPoints.parray = SafeArrayCreate(VT_I4, 1, &bound);

            LONG* pData = nullptr;
            SafeArrayAccessData(varPoints.parray, (void**)&pData);
            for (int i = 0; i < n; i++)
            {
                // Shift to origin (0,0) then scale
                pData[i*2]   = (LONG)((stroke.points[i].x - gMinX) * scale);
                pData[i*2+1] = (LONG)((stroke.points[i].y - gMinY) * scale);
            }
            SafeArrayUnaccessData(varPoints.parray);

            VARIANT varPD; VariantInit(&varPD);
            IInkStrokeDisp* newStroke = nullptr;
            hr = inkDisp->CreateStroke(varPoints, varPD, &newStroke);
            if (newStroke) newStroke->Release();
            VariantClear(&varPoints);
        }

        IInkStrokes* inkStrokes = nullptr;
        inkDisp->get_Strokes(&inkStrokes);
        long cnt = 0;
        if (inkStrokes) inkStrokes->get_Count(&cnt);
        printf("  Strokes in ink: %ld\n", cnt);

        IInkRecognizerContext* ctx = nullptr;
        defaultReco->CreateRecognizerContext(&ctx);
        if (inkStrokes) ctx->putref_Strokes(inkStrokes);

        IInkRecognitionResult* res = nullptr;
        InkRecognitionStatus status;
        hr = ctx->Recognize(&status, &res);
        printf("  Recognize: hr=0x%08lX status=%d\n", hr, (int)status);

        if (SUCCEEDED(hr) && res)
        {
            BSTR bstr = nullptr;
            res->get_TopString(&bstr);
            if (bstr) {
                wprintf(L"  Result: \"%s\"\n", bstr);
                SysFreeString(bstr);
            } else {
                printf("  Result: (null)\n");
            }
            res->Release();
        }

        if (inkStrokes) inkStrokes->Release();
        ctx->Release();
        inkDisp->Release();
    }

    defaultReco->Release();
    recognizers->Release();
    CoUninitialize();
    printf("\n=== Done ===\n");
    return 0;
}
