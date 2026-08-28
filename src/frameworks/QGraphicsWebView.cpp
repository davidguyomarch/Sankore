/*
 * Copyright (C) 2026 David Guyomarch
 */

/**
 * @file QGraphicsWebView.cpp
 * @brief QGraphicsWebView — real WebEngine or stub depending on SANKORE_WEBENGINE.
 */
#include "QGraphicsWebView.h"

#ifdef SANKORE_WEBENGINE

QGraphicsWebView::QGraphicsWebView(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
    , m_webView(new QWebEngineView())
    , m_proxy(nullptr)
    , m_mainFrame(new QWebFrame(this))
    , m_zoomFactor(1.0)
{
    m_proxy = new QGraphicsProxyWidget(this);
    m_proxy->setWidget(m_webView);
    m_mainFrame->setPage(m_webView->page());

    connect(m_webView, &QWebEngineView::loadStarted, this, &QGraphicsWebView::loadStarted);
    connect(m_webView, &QWebEngineView::loadFinished, this, &QGraphicsWebView::loadFinished);
    connect(m_webView, &QWebEngineView::loadProgress, this, &QGraphicsWebView::loadProgress);
    connect(m_webView, &QWebEngineView::urlChanged, this, &QGraphicsWebView::urlChanged);
    connect(m_webView, &QWebEngineView::titleChanged, this, &QGraphicsWebView::titleChanged);
    connect(m_webView, &QWebEngineView::iconChanged, this, &QGraphicsWebView::iconChanged);
}

QGraphicsWebView::~QGraphicsWebView() {}

QWebEnginePage* QGraphicsWebView::page() const { return m_webView->page(); }
void QGraphicsWebView::setPage(QWebEnginePage *page) {
    m_webView->setPage(page);
    m_mainFrame->setPage(page);
}
QWebEngineSettings* QGraphicsWebView::settings() const { return m_webView->page()->settings(); }
QUrl QGraphicsWebView::url() const { return m_webView->url(); }
void QGraphicsWebView::setUrl(const QUrl &url) { m_webView->setUrl(url); }
void QGraphicsWebView::load(const QUrl &url) { m_webView->load(url); }
void QGraphicsWebView::setHtml(const QString &html, const QUrl &baseUrl) { m_webView->setHtml(html, baseUrl); }
QString QGraphicsWebView::title() const { return m_webView->title(); }
qreal QGraphicsWebView::zoomFactor() const { return m_webView->zoomFactor(); }
void QGraphicsWebView::setZoomFactor(qreal factor) { m_webView->setZoomFactor(factor); }

void QGraphicsWebView::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {
    // Proxy widget handles painting
}

void QGraphicsWebView::resize(const QSizeF &size) {
    QGraphicsWidget::resize(size);
    if (m_proxy) m_proxy->resize(size);
}
void QGraphicsWebView::resize(qreal w, qreal h) { resize(QSizeF(w, h)); }

#else // STUB MODE — no WebEngine

QGraphicsWebView::QGraphicsWebView(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
    , m_mainFrame(new QWebFrame(this))
    , m_zoomFactor(1.0)
{}

QGraphicsWebView::~QGraphicsWebView() {}

QWebEnginePage* QGraphicsWebView::page() const { return nullptr; }
void QGraphicsWebView::setPage(QWebEnginePage *) {}
QWebEngineSettings* QGraphicsWebView::settings() const { return nullptr; }
QUrl QGraphicsWebView::url() const { return m_url; }
void QGraphicsWebView::setUrl(const QUrl &url) { m_url = url; emit urlChanged(url); }
void QGraphicsWebView::load(const QUrl &url) { setUrl(url); emit loadFinished(true); }
void QGraphicsWebView::setHtml(const QString &, const QUrl &) { emit loadFinished(true); }
QString QGraphicsWebView::title() const { return QString(); }
qreal QGraphicsWebView::zoomFactor() const { return m_zoomFactor; }
void QGraphicsWebView::setZoomFactor(qreal f) { m_zoomFactor = f; }

void QGraphicsWebView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->fillRect(boundingRect(), Qt::white);
    painter->drawText(boundingRect(), Qt::AlignCenter, "Web view disabled");
}

void QGraphicsWebView::resize(const QSizeF &size) { QGraphicsWidget::resize(size); }
void QGraphicsWebView::resize(qreal w, qreal h) { resize(QSizeF(w, h)); }

#endif
