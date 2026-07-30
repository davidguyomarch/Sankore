/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "IHandwritingRecognizer.h"

#ifdef Q_OS_WIN
#include "UBWindowsInkRecognizer.h"
#endif

#include "UBStubRecognizer.h"

IHandwritingRecognizer* IHandwritingRecognizer::createDefault()
{
#ifdef Q_OS_WIN
    auto* recognizer = new UBWindowsInkRecognizer();
    if (recognizer->isAvailable())
        return recognizer;
    delete recognizer;
#endif

    return new UBStubRecognizer();
}
