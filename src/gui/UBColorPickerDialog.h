/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBCOLORPICKERDIALOG_H
#define UBCOLORPICKERDIALOG_H

#include <QColor>
#include <QColorDialog>
#include <QWidget>
#include <QString>

#include "core/UBSettings.h"

/**
 * #475: single reusable modal colour picker for the whole app.
 *
 * Centralises the boilerplate that was duplicated across every QColorDialog
 * call site (shape fill, text colour/background, legacy stroke/fill palettes,
 * and now the DrawingPropsBar custom-colour button): parent to the given
 * widget, optional alpha channel, dark-theme legibility guard, and the macOS
 * non-native dialog opt-out.
 *
 * Returns the chosen colour, or an **invalid** QColor (\c QColor()) if the user
 * cancels — callers test \c result.isValid().
 *
 * Header-only (like UBIconUtils) so it is callable from both QML-facing
 * controllers and the C++ delegates without any moc/premoc wiring.
 */
namespace UBColorPickerDialog
{
    /**
     * Seed the dialog with a colour whose HSV *value* is never so low that the
     * colour wheel traps the user.
     *
     * QColorDialog's wheel/square picks hue + saturation but NOT the value (V);
     * V is the separate vertical slider. If the seed is black / very dark, V
     * starts at (or near) 0, so clicking a vivid hue in the wheel still yields a
     * (near-)black colour because V was never raised — the reported #475 pitfall.
     *
     * We raise V to a readable floor for the OPENING seed only, preserving hue
     * and saturation. This does not constrain the user afterwards (they can
     * still drag V down deliberately); it only ensures the wheel opens at a
     * usable brightness. For a pure grey/black seed (saturation 0) this opens on
     * a light grey, which is a sane neutral start.
     */
    inline QColor seedColor(const QColor& initial)
    {
        // Default seed when the caller passes nothing valid.
        QColor c = initial.isValid() ? initial : QColor(Qt::black);   // ui-color-allow
        // Qt 6 getHsvF/fromHsvF take float* / float (not qreal).
        float h, s, v, a;
        c.getHsvF(&h, &s, &v, &a);
        // Achromatic colours (grey/black/white) report hue == -1; keep hue valid
        // for fromHsvF (-1 → 0 is fine for a zero-saturation seed).
        constexpr float kMinValue = 0.5f;   // readable brightness floor
        if (v < kMinValue)
            c = QColor::fromHsvF(h < 0.0f ? 0.0f : h, s, kMinValue, a);
        return c;
    }

    inline QColor pick(const QColor& initial,
                       QWidget* parent = nullptr,
                       bool withAlpha = true,
                       const QString& title = QString())
    {
        QColorDialog dialog(seedColor(initial), parent);
        dialog.setOption(QColorDialog::ShowAlphaChannel, withAlpha);
#ifdef Q_OS_MACOS
        // The native macOS panel ignores our styling and parenting; use Qt's.
        dialog.setOption(QColorDialog::DontUseNativeDialog, true);
#endif
        if (!title.isEmpty())
            dialog.setWindowTitle(title);

        // Keep the dialog legible on the dark theme (the native/default palette
        // can render dark-on-dark). Same guard the delegates used inline.
        if (UBSettings::settings() && UBSettings::settings()->isDarkBackground())
            dialog.setStyleSheet("background-color: white;");

        if (dialog.exec() == QDialog::Accepted)
            return dialog.selectedColor();
        return QColor();   // invalid → cancelled
    }
}

#endif // UBCOLORPICKERDIALOG_H
