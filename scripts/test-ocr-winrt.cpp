// test-ocr-winrt.cpp — WinRT Ink OCR tester
// Reads ocr_strokes_dump.txt and tests recognition with various parameters.
// Results are printed AND copied to clipboard for easy sharing.
//
// Compile (from Developer Command Prompt for VS 2022):
//   cl /EHsc /std:c++17 /bigobj test-ocr-winrt.cpp WindowsApp.lib /Fe:test-ocr-winrt.exe
//
// Run:
//   test-ocr-winrt.exe [path_to_dump]
//   (defaults to ocr_strokes_dump.txt in current directory)

// Silence deprecation of experimental coroutines (WinRT headers use them in C++17 mode)
#define _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Input.Inking.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI::Input::Inking;

struct Point2D { double x, y; };
struct Stroke { std::vector<Point2D> points; };

std::vector<Stroke> readDumpFile(const char* path)
{
    std::vector<Stroke> strokes;
    FILE* f = fopen(path, "r");
    if (!f) { printf("Cannot open %s\n", path); return strokes; }

    char line[256];
    Stroke current;

    while (fgets(line, sizeof(line), f))
    {
        int n;
        if (sscanf(line, "STROKE %*d POINTS %d", &n) == 1)
        {
            if (!current.points.empty())
                strokes.push_back(current);
            current.points.clear();
        }
        else
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

void copyToClipboard(const std::string& text)
{
    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem) {
        memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
        GlobalUnlock(hMem);
        SetClipboardData(CF_TEXT, hMem);
    }
    CloseClipboard();
}

std::string testRecognition(const std::vector<Stroke>& strokes, double targetHeight, int maxPointsPerStroke, bool reverseOrder)
{
    // Compute bounding box
    double minX = strokes[0].points[0].x, minY = strokes[0].points[0].y;
    double maxX = minX, maxY = minY;
    for (auto& s : strokes) {
        for (auto& p : s.points) {
            if (p.x < minX) minX = p.x; if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y; if (p.y > maxY) maxY = p.y;
        }
    }
    double sceneH = maxY - minY;
    if (sceneH < 1) sceneH = 1;
    double scale = targetHeight / sceneH;

    try {
        InkStrokeBuilder builder;
        InkStrokeContainer container;
        int added = 0;

        // Optionally reverse stroke order
        std::vector<size_t> order;
        for (size_t i = 0; i < strokes.size(); i++) order.push_back(i);
        if (reverseOrder) std::reverse(order.begin(), order.end());

        for (size_t idx : order)
        {
            auto& stroke = strokes[idx];
            if (stroke.points.size() < 2) continue;

            // Subsample
            std::vector<Point2D> simplified;
            int n = (int)stroke.points.size();
            if (n <= maxPointsPerStroke) {
                simplified = stroke.points;
            } else {
                simplified.push_back(stroke.points.front());
                double step = (double)(n - 1) / (double)(maxPointsPerStroke - 1);
                for (int i = 1; i < maxPointsPerStroke - 1; i++) {
                    int idx2 = (int)(i * step + 0.5);
                    if (idx2 < n) simplified.push_back(stroke.points[idx2]);
                }
                simplified.push_back(stroke.points.back());
            }

            // Create InkPoints
            std::vector<InkPoint> inkPoints;
            for (auto& p : simplified) {
                float x = (float)((p.x - minX) * scale);
                float y = (float)((p.y - minY) * scale);
                inkPoints.push_back(InkPoint(Windows::Foundation::Point(x, y), 0.5f));
            }

            auto inkStroke = builder.CreateStrokeFromInkPoints(
                winrt::single_threaded_vector<InkPoint>(std::move(inkPoints)),
                Windows::Foundation::Numerics::float3x2::identity());
            container.AddStroke(inkStroke);
            added++;
        }

        // Recognize
        InkRecognizerContainer recoContainer;
        auto results = recoContainer.RecognizeAsync(container, InkRecognitionTarget::All).get();

        std::string text;
        for (uint32_t i = 0; i < results.Size(); i++) {
            auto candidates = results.GetAt(i).GetTextCandidates();
            if (candidates.Size() > 0) {
                if (!text.empty()) text += " ";
                text += winrt::to_string(candidates.GetAt(0));
            }
        }

        char buf[512];
        snprintf(buf, sizeof(buf), "height=%.0f maxPts=%d %s -> \"%s\" (%d strokes, scale=%.1f)",
            targetHeight, maxPointsPerStroke, reverseOrder ? "reversed" : "chronological",
            text.c_str(), added, scale);
        return std::string(buf);
    }
    catch (const winrt::hresult_error& ex) {
        return "ERROR: " + winrt::to_string(ex.message());
    }
}

int main(int argc, char* argv[])
{
    const char* dumpPath = argc > 1 ? argv[1] : "ocr_strokes_dump.txt";

    printf("=== WinRT OCR Test ===\n");
    printf("Reading: %s\n", dumpPath);

    auto strokes = readDumpFile(dumpPath);
    printf("Loaded %zu strokes\n\n", strokes.size());

    if (strokes.empty()) { printf("No strokes!\n"); return 1; }

    // Show stroke summary
    double gMinX = strokes[0].points[0].x, gMaxX = gMinX;
    double gMinY = strokes[0].points[0].y, gMaxY = gMinY;
    for (auto& s : strokes) {
        for (auto& p : s.points) {
            if (p.x < gMinX) gMinX = p.x; if (p.x > gMaxX) gMaxX = p.x;
            if (p.y < gMinY) gMinY = p.y; if (p.y > gMaxY) gMaxY = p.y;
        }
    }
    printf("Bounding box: X[%.1f, %.1f] Y[%.1f, %.1f] = %.0f x %.0f\n\n",
        gMinX, gMinY, gMaxX, gMaxY, gMaxX-gMinX, gMaxY-gMinY);

    // Initialize WinRT — MUST be multi_threaded to avoid RecognizeAsync deadlock
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    } catch (...) {}

    // List recognizers
    try {
        InkRecognizerContainer rc;
        auto recos = rc.GetRecognizers();
        printf("Available recognizers: %u\n", recos.Size());
        for (uint32_t i = 0; i < recos.Size(); i++) {
            printf("  [%u] %s\n", i, winrt::to_string(recos.GetAt(i).Name()).c_str());
        }
        printf("\n");
    } catch (const winrt::hresult_error& ex) {
        printf("ERROR listing recognizers: %s\n", winrt::to_string(ex.message()).c_str());
    }

    // Test matrix
    double heights[] = {20, 30, 40, 60, 100, 200};
    int maxPts[] = {20, 40, 100, 999};

    std::ostringstream log;
    log << "=== WinRT OCR Results ===\n";
    log << "File: " << dumpPath << "\n";
    log << "Strokes: " << strokes.size() << "\n";
    log << "Bounds: " << (gMaxX-gMinX) << " x " << (gMaxY-gMinY) << "\n\n";

    printf("--- Testing parameters ---\n");

    for (double h : heights) {
        for (int mp : maxPts) {
            // Chronological order
            std::string r1 = testRecognition(strokes, h, mp, false);
            printf("  %s\n", r1.c_str());
            log << r1 << "\n";

            // Reversed order
            std::string r2 = testRecognition(strokes, h, mp, true);
            printf("  %s\n", r2.c_str());
            log << r2 << "\n";
        }
    }

    // Copy to clipboard
    std::string logStr = log.str();
    copyToClipboard(logStr);
    printf("\n=== Results copied to clipboard ===\n");

    return 0;
}
