HEADERS += src/desktop/UBDesktopAnnotationController.h \
           src/desktop/UBCustomCaptureWindow.h \
           src/desktop/UBWindowCapture.h

SOURCES += src/desktop/UBDesktopAnnotationController.cpp \
           src/desktop/UBCustomCaptureWindow.cpp

win32 { 
    HEADERS += src/desktop/UBWindowCaptureDelegate_win.h
    SOURCES += src/desktop/UBWindowCapture_win.cpp \
               src/desktop/UBWindowCaptureDelegate_win.cpp
}

macx:OBJECTIVE_SOURCES += src/desktop/UBWindowCapture_mac.mm

linux-*:SOURCES += src/desktop/UBWindowCapture_linux.cpp
