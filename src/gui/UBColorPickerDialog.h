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
    inline QColor pick(const QColor& initial,
                       QWidget* parent = nullptr,
                       bool withAlpha = true,
                       const QString& title = QString())
    {
        // A seed value for the picker when the caller passes an invalid colour
        // (the dialog needs some initial swatch) — not UI chrome.
        QColorDialog dialog(initial.isValid() ? initial : QColor(Qt::black), parent);   // ui-color-allow
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
