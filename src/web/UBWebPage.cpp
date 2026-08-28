/*
 * Copyright (C) 2026 David Guyomarch
 */

#include "UBWebPage.h"

#ifdef SANKORE_WEBENGINE

UBWebPage::UBWebPage(QObject *parent)
    : QWebEnginePage(parent)
    , mPluginFactory(nullptr)
{
}

UBWebPage::~UBWebPage()
{
}

void UBWebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_UNUSED(level);
    qDebug("JavaScript> %s (%s:%d)", qPrintable(message), qPrintable(sourceID), lineNumber);
}

#endif
