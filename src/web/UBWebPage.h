/*
 * Copyright (C) 2026 David Guyomarch
 */

#ifndef UBWEBPAGE_H_
#define UBWEBPAGE_H_

#include <QObject>
#include <QString>
#ifdef SANKORE_WEBENGINE
#include <QWebEnginePage>

class UBWebPage : public QWebEnginePage
{
    Q_OBJECT;
public:
    UBWebPage(QObject *parent = 0);
    virtual ~UBWebPage();
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) override;
private:
    void *mPluginFactory;
};

#else // Stub when no WebEngine

class UBWebPage : public QObject
{
    Q_OBJECT;
public:
    UBWebPage(QObject *parent = 0) : QObject(parent) {}
    virtual ~UBWebPage() {}
};

#endif

#endif /* UBWEBPAGE_H_ */
