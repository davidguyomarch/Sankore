/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "IHandwritingRecognizer.h"

#ifdef Q_OS_WIN
#include "UBWindowsInkRecognizer.h"
#else
#include "UBZinniaRecognizer.h"
#endif

#include "UBStubRecognizer.h"

IHandwritingRecognizer* IHandwritingRecognizer::createDefault()
{
#ifdef Q_OS_WIN
    auto* recognizer = new UBWindowsInkRecognizer();
    if (recognizer->isAvailable())
        return recognizer;
    delete recognizer;
#else
    auto* recognizer = new UBZinniaRecognizer();
    if (recognizer->isAvailable())
        return recognizer;
    delete recognizer;
#endif

    return new UBStubRecognizer();
}
