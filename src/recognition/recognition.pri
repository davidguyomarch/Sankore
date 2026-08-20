
HEADERS += \
    src/recognition/IHandwritingRecognizer.h \
    src/recognition/UBStubRecognizer.h \
    src/recognition/UBStrokeExtractor.h \
    src/recognition/UBRecognitionController.h

SOURCES += \
    src/recognition/UBHandwritingRecognizerFactory.cpp \
    src/recognition/UBStrokeExtractor.cpp \
    src/recognition/UBRecognitionController.cpp

win32 {
    HEADERS += src/recognition/UBWindowsInkRecognizer.h
    SOURCES += src/recognition/UBWindowsInkRecognizer.cpp
    # WinRT C++/WinRT — WindowsApp.lib is linked in OpenSankore.pro
}

!win32 {
    HEADERS += src/recognition/UBZinniaRecognizer.h
    SOURCES += src/recognition/UBZinniaRecognizer.cpp
    # Zinnia library (install: apt install libzinnia-dev)
    LIBS += -lzinnia
}
