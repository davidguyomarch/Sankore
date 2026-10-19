/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBDesktopCaptureService.h"

#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QRect>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

#include "domain/UBGraphicsScene.h"

namespace {
// #523 / ADR-0013: temporary diagnostic — trace the entry grab sequencing so we
// can confirm on the VM that the capture excludes Sankoré's own window. Remove
// once the behaviour is validated.
void ubDesktopLog(const QString& line)
{
    QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
    if (f.open(QIODevice::Append | QIODevice::Text))
        QTextStream(&f) << "[DESKTOP] " << line << "\n";
}
}

UBDesktopCaptureService::UBDesktopCaptureService(QObject* parent)
    : QObject(parent)
{
}

QScreen* UBDesktopCaptureService::screenFor(const QWidget* onWindow) const
{
    // ADR-0013 Q5: capture ONLY the screen Sankoré's window is on; the teacher's
    // other screen(s) stay private. Fall back to the primary screen if the
    // window has no screen yet (e.g. grabbed before it is shown).
    if (onWindow && onWindow->screen())
        return onWindow->screen();
    return QGuiApplication::primaryScreen();
}

QPixmap UBDesktopCaptureService::grabDesktop(const QWidget* onWindow) const
{
    QScreen* screen = screenFor(onWindow);
    if (!screen)
        return QPixmap();

    // Reuse the proven grab mechanism (same as the old overlay's getScreenPixmap):
    // grab the whole screen geometry. NO processEvents() around it (#135).
    const QRect g = screen->geometry();
    return screen->grabWindow(0, g.x(), g.y(), g.width(), g.height());
}

QPixmap UBDesktopCaptureService::grabDesktopBehind(QWidget* window) const
{
    // #523 / ADR-0013 (open question): to capture what is BEHIND Sankoré, the
    // window must not be frontmost at grab time. Hiding it and grabbing
    // immediately races the compositor (the window is still on screen), so we
    // hide, flush to the platform, let the compositor remove the window with a
    // short NATIVE sleep (NOT processEvents — that re-enters the event loop and
    // crashed during the transition, #135), grab, then re-show.
    // Option B (#523, VM-confirmed): the main window is not the only Sankoré
    // surface on screen — the Fond menu is a SEPARATE top-level popup window, so
    // hiding mMainWindow alone leaves it in the capture. Hide EVERY visible
    // top-level window of this app for the grab, then restore them. This is
    // defensive against any popup/palette (not just the one menu we know about).
    const QWindowList tops = QGuiApplication::topLevelWindows();
    QList<QWindow*> hidden;
    hidden.reserve(tops.size());
    for (QWindow* w : tops)
    {
        if (w && w->isVisible())
        {
            w->setVisible(false);
            hidden.append(w);
        }
    }

    ubDesktopLog(QString("grabDesktopBehind BEGIN mainWindow=%1 mainVisible=%2 topLevelsHidden=%3")
                     .arg((quintptr)window, 0, 16)
                     .arg(window ? window->isVisible() : -1)
                     .arg(hidden.size()));

    // Also hide the main QWidget window itself (its QWindow is normally in the
    // list above, but hide the widget too so its backing store is released).
    const bool widgetWasVisible = window && window->isVisible();
    if (widgetWasVisible)
        window->hide();

    // Push the hides to the window system WITHOUT pumping our own event loop
    // (processEvents re-enters and crashed during the transition, #135), then
    // let the compositor drop the windows with a short native sleep.
    QGuiApplication::sync();
    QThread::msleep(120);
    QGuiApplication::sync();

    const QPixmap shot = grabDesktop(window);

    // Restore everything we hid, in the same order.
    if (widgetWasVisible && window)
        window->show();
    for (QWindow* w : hidden)
        w->setVisible(true);

    ubDesktopLog(QString("grabDesktopBehind END pixmap=%1x%2 null=%3 restored=%4")
                     .arg(shot.width()).arg(shot.height())
                     .arg(shot.isNull())
                     .arg(hidden.size()));
    return shot;
}

bool UBDesktopCaptureService::refreshInto(UBGraphicsScene* scene, const QWidget* onWindow) const
{
    if (!scene)
        return false;

    const QPixmap shot = grabDesktop(onWindow);
    // A null grab clears the capture (the see-through page paints normally).
    scene->setDesktopCapturePixmap(shot);
    return !shot.isNull();
}

bool UBDesktopCaptureService::refreshBehindInto(UBGraphicsScene* scene, QWidget* window) const
{
    // #523: entry grab — capture the desktop BEHIND Sankoré (window hidden for
    // the grab), so the captured background does not contain Sankoré's own UI.
    if (!scene)
        return false;
    const QPixmap shot = grabDesktopBehind(window);
    scene->setDesktopCapturePixmap(shot);
    return !shot.isNull();
}

void UBDesktopCaptureService::clear(UBGraphicsScene* scene) const
{
    if (scene)
        scene->setDesktopCapturePixmap(QPixmap());
}
