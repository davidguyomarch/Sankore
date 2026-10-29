/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBDesktopCaptureService.h"

#include <QWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QRect>

#include "domain/UBGraphicsScene.h"

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

bool UBDesktopCaptureService::refreshInto(UBGraphicsScene* scene, const QWidget* onWindow) const
{
    if (!scene)
        return false;

    const QPixmap shot = grabDesktop(onWindow);
    // A null grab clears the capture (the see-through page paints normally).
    scene->setDesktopCapturePixmap(shot);
    return !shot.isNull();
}

void UBDesktopCaptureService::clear(UBGraphicsScene* scene) const
{
    if (scene)
        scene->setDesktopCapturePixmap(QPixmap());
}
