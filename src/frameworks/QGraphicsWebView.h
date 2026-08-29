/*
 * Copyright (C) 2026 David Guyomarch
 */

/**
 * @file QGraphicsWebView.h
 * @brief QGraphicsWebView for Qt6 — real WebEngine or stub depending on SANKORE_WEBENGINE.
 */
#ifndef QGRAPHICSWEBVIEW_H
#define QGRAPHICSWEBVIEW_H

#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QUrl>
#include <QPainter>

#ifdef SANKORE_WEBENGINE
#include <QEventLoop>
#include <QTimer>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebChannel>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#endif

// Forward declarations for non-WebEngine mode
#ifndef SANKORE_WEBENGINE
class QWebEnginePage;
class QWebEngineSettings;
class QWebEngineView;
#endif

// QWebFrame compatibility shim
class QWebFrame : public QObject
{
    Q_OBJECT
public:
    QWebFrame(QObject *parent = nullptr) : QObject(parent), m_page(nullptr)
#ifdef SANKORE_WEBENGINE
        , m_channel(nullptr)
#endif
    {}

#ifdef SANKORE_WEBENGINE
    void setPage(QWebEnginePage *page) {
        if (m_page) disconnect(m_page, nullptr, this, nullptr);
        m_page = page;
        if (m_page) {
            if (!m_channel) m_channel = new QWebChannel(this);
            m_page->setWebChannel(m_channel);

            QWebEngineScript channelScript;
            channelScript.setName("qwebchannel");
            channelScript.setSourceCode(
                "new QWebChannel(qt.webChannelTransport, function(channel) {"
                "  window.__qt_webchannel_objects = channel.objects;"
                "  for (var name in channel.objects) { window[name] = channel.objects[name]; }"
                "});"
            );
            channelScript.setInjectionPoint(QWebEngineScript::DocumentReady);
            channelScript.setWorldId(QWebEngineScript::MainWorld);
            channelScript.setRunsOnSubFrames(false);
            m_page->scripts().insert(channelScript);

            connect(m_page, &QWebEnginePage::loadFinished, this, [this](bool ok) {
                if (ok) {
                    emit javaScriptWindowObjectCleared();
                    emit initialLayoutCompleted();
                }
            });
        }
    }
    QVariant evaluateJavaScript(const QString &script) {
        if (m_page) m_page->runJavaScript(script);
        return QVariant();
    }
    QVariant evaluateJavaScriptSync(const QString &script) {
        if (!m_page) return QVariant();
        QVariant result;
        bool finished = false;
        m_page->runJavaScript(script, [&result, &finished](const QVariant &v) {
            result = v; finished = true;
        });
        if (!finished) {
            QEventLoop loop;
            QTimer timer;
            timer.setSingleShot(true);
            connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            timer.start(1000);
            while (!finished && timer.isActive()) {
                loop.processEvents(QEventLoop::AllEvents, 50);
            }
        }
        return result;
    }
    void addToJavaScriptWindowObject(const QString &name, QObject *object) {
        if (!m_channel) {
            m_channel = new QWebChannel(this);
            if (m_page) m_page->setWebChannel(m_channel);
        }
        m_channel->registerObject(name, object);
    }
#else
    void setPage(QWebEnginePage *) {}
    QVariant evaluateJavaScript(const QString &) { return QVariant(); }
    QVariant evaluateJavaScriptSync(const QString &) { return QVariant(); }
    void addToJavaScriptWindowObject(const QString &, QObject *) {}
#endif

    void setScrollBarPolicy(Qt::Orientation, Qt::ScrollBarPolicy) {}
    QUrl url() const { return m_url; }
    void setUrl(const QUrl &url) { m_url = url; }
    QString toHtml() const { return QString(); }
    QSize contentsSize() const { return QSize(800, 600); }

signals:
    void javaScriptWindowObjectCleared();
    void initialLayoutCompleted();

private:
    QWebEnginePage *m_page;
    QUrl m_url;
#ifdef SANKORE_WEBENGINE
    QWebChannel *m_channel;
#endif
};

class QGraphicsWebView : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit QGraphicsWebView(QGraphicsItem *parent = nullptr);
    virtual ~QGraphicsWebView();

    QWebEnginePage* page() const;
    void setPage(QWebEnginePage *page);
    QWebFrame* mainFrame() const { return m_mainFrame; }
    QWebEngineSettings* settings() const;

    QUrl url() const;
    void setUrl(const QUrl &url);
    void load(const QUrl &url);
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    QString title() const;
    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void resize(const QSizeF &size);
    void resize(qreal w, qreal h);
    QSizeF size() const { return QGraphicsWidget::size(); }
    void setMaximumSize(const QSizeF &size) { QGraphicsWidget::setMaximumSize(size); }
    void setMaximumSize(qreal w, qreal h) { QGraphicsWidget::setMaximumSize(w, h); }

    enum { Type = UserType + 100 };
    int type() const override { return Type; }

signals:
    void loadStarted();
    void loadFinished(bool ok);
    void loadProgress(int progress);
    void urlChanged(const QUrl &url);
    void titleChanged(const QString &title);
    void iconChanged();
    void linkClicked(const QUrl &url);

private:
#ifdef SANKORE_WEBENGINE
    QWebEngineView *m_webView;
    QGraphicsProxyWidget *m_proxy;
#endif
    QWebFrame *m_mainFrame;
    QUrl m_url;
    qreal m_zoomFactor;
};

#endif // QGRAPHICSWEBVIEW_H
