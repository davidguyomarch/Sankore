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
    double scales[] = {1.0, 10.0, 50.0, 100.0};

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { printf("COM init failed\n"); return 1; }

    IInkRecognizers* recognizers = nullptr;
    CoCreateInstance(CLSID_InkRecognizers, NULL, CLSCTX_INPROC_SERVER, IID_IInkRecognizers, (void**)&recognizers);
    IInkRecognizer* defaultReco = nullptr;
    recognizers->GetDefaultRecognizer(0, &defaultReco);

    for (double scale : scales)
    {
        printf("\n--- Scale factor: %.1f ---\n", scale);

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
                pData[i*2]   = (LONG)(stroke.points[i].x * scale);
                pData[i*2+1] = (LONG)(stroke.points[i].y * scale);
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
