// test-ocr.cpp — Standalone Windows Ink recognition test
// Compile: cl /EHsc /std:c++17 test-ocr.cpp ole32.lib oleaut32.lib
// Run: test-ocr.exe

#include <windows.h>
#include <stdio.h>
#include <comdef.h>
#include <msinkaut.h>
#include <msinkaut_i.c>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

int main()
{
    printf("=== Windows Ink OCR Test ===\n\n");

    // Init COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        printf("ERROR: CoInitializeEx failed: 0x%08lX\n", hr);
        return 1;
    }
    printf("[OK] COM initialized\n");

    // Check recognizers
    IInkRecognizers* recognizers = nullptr;
    hr = CoCreateInstance(CLSID_InkRecognizers, NULL, CLSCTX_INPROC_SERVER,
                         IID_IInkRecognizers, (void**)&recognizers);
    if (FAILED(hr) || !recognizers) {
        printf("ERROR: Cannot create InkRecognizers: 0x%08lX\n", hr);
        CoUninitialize();
        return 1;
    }

    long count = 0;
    recognizers->get_Count(&count);
    printf("[OK] Found %ld recognizers\n", count);

    if (count == 0) {
        printf("ERROR: No recognizers installed\n");
        recognizers->Release();
        CoUninitialize();
        return 1;
    }

    // Get default recognizer
    IInkRecognizer* defaultReco = nullptr;
    hr = recognizers->GetDefaultRecognizer(0, &defaultReco);
    if (FAILED(hr) || !defaultReco) {
        printf("ERROR: GetDefaultRecognizer failed: 0x%08lX\n", hr);
        recognizers->Release();
        CoUninitialize();
        return 1;
    }

    BSTR recoName = nullptr;
    defaultReco->get_Name(&recoName);
    if (recoName) {
        wprintf(L"[OK] Default recognizer: %s\n", recoName);
        SysFreeString(recoName);
    }

    // Create InkDisp
    IInkDisp* inkDisp = nullptr;
    hr = CoCreateInstance(CLSID_InkDisp, NULL, CLSCTX_INPROC_SERVER,
                         IID_IInkDisp, (void**)&inkDisp);
    if (FAILED(hr) || !inkDisp) {
        printf("ERROR: Cannot create InkDisp: 0x%08lX\n", hr);
        defaultReco->Release();
        recognizers->Release();
        CoUninitialize();
        return 1;
    }
    printf("[OK] InkDisp created\n");

    // Create a simple stroke: the letter "L"
    // Vertical line from (1000,1000) to (1000,3000)
    // Horizontal line from (1000,3000) to (2500,3000)
    LONG strokeL[] = {
        1000, 1000,
        1000, 1500,
        1000, 2000,
        1000, 2500,
        1000, 3000,
        1500, 3000,
        2000, 3000,
        2500, 3000
    };
    int numPointsL = 8;

    VARIANT varPoints;
    VariantInit(&varPoints);
    varPoints.vt = VT_ARRAY | VT_I4;

    SAFEARRAYBOUND bound;
    bound.lLbound = 0;
    bound.cElements = numPointsL * 2;
    varPoints.parray = SafeArrayCreate(VT_I4, 1, &bound);

    LONG* pData = nullptr;
    SafeArrayAccessData(varPoints.parray, (void**)&pData);
    memcpy(pData, strokeL, sizeof(strokeL));
    SafeArrayUnaccessData(varPoints.parray);

    VARIANT varPacketDesc;
    VariantInit(&varPacketDesc);

    IInkStrokeDisp* newStroke = nullptr;
    hr = inkDisp->CreateStroke(varPoints, varPacketDesc, &newStroke);
    printf("CreateStroke hr=0x%08lX, stroke=%p\n", hr, newStroke);
    if (newStroke) newStroke->Release();
    VariantClear(&varPoints);

    // Create recognizer context
    IInkRecognizerContext* context = nullptr;
    hr = defaultReco->CreateRecognizerContext(&context);
    if (FAILED(hr) || !context) {
        printf("ERROR: CreateRecognizerContext failed: 0x%08lX\n", hr);
        inkDisp->Release();
        defaultReco->Release();
        recognizers->Release();
        CoUninitialize();
        return 1;
    }
    printf("[OK] RecognizerContext created\n");

    // Assign strokes
    IInkStrokes* inkStrokes = nullptr;
    hr = inkDisp->get_Strokes(&inkStrokes);
    printf("get_Strokes hr=0x%08lX, strokes=%p\n", hr, inkStrokes);

    if (inkStrokes) {
        long strokeCount = 0;
        inkStrokes->get_Count(&strokeCount);
        printf("Stroke count in collection: %ld\n", strokeCount);

        hr = context->putref_Strokes(inkStrokes);
        printf("putref_Strokes hr=0x%08lX\n", hr);
    }

    // Recognize
    IInkRecognitionResult* recoResult = nullptr;
    InkRecognitionStatus status;
    hr = context->Recognize(&status, &recoResult);
    printf("Recognize hr=0x%08lX, status=%d, result=%p\n", hr, (int)status, recoResult);

    if (SUCCEEDED(hr) && recoResult) {
        BSTR bstrResult = nullptr;
        hr = recoResult->get_TopString(&bstrResult);
        printf("get_TopString hr=0x%08lX\n", hr);
        if (bstrResult) {
            wprintf(L"\n*** RECOGNIZED: \"%s\" ***\n\n", bstrResult);
            SysFreeString(bstrResult);
        } else {
            printf("TopString is NULL\n");
        }
        recoResult->Release();
    } else {
        printf("Recognition failed or no result\n");
    }

    // Cleanup
    if (inkStrokes) inkStrokes->Release();
    context->Release();
    inkDisp->Release();
    defaultReco->Release();
    recognizers->Release();
    CoUninitialize();

    printf("\n=== Done ===\n");
    return 0;
}
