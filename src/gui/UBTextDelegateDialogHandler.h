/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBTEXTDELEGATEDIALOGHANDLER_H
#define UBTEXTDELEGATEDIALOGHANDLER_H

#include <QObject>
#include <QFont>
#include <QColor>

class QWidget;
class QFontDialog;
class UBGraphicsTextItemDelegate;
class UBSettings;

/**
 * @brief GUI-layer mediator that opens font/color dialogs in response to
 *        domain-layer signals from UBGraphicsTextItemDelegate.
 *
 * This class bridges the domain (model) and the view (dialogs) without
 * the domain needing to know about QFontDialog or QColorDialog.
 */
class UBTextDelegateDialogHandler : public QObject
{
    Q_OBJECT

public:
    explicit UBTextDelegateDialogHandler(QWidget* parentWidget, UBSettings* settings, QObject* parent = nullptr);

    /**
     * Connect this handler to a text delegate's signals.
     * Call this whenever a new text delegate is created or becomes active.
     */
    void connectToDelegate(UBGraphicsTextItemDelegate* delegate);

    /**
     * Disconnect from a previously connected delegate.
     */
    void disconnectFromDelegate(UBGraphicsTextItemDelegate* delegate);

private slots:
    void onFontChangeRequested(const QFont& currentFont);
    void onTextColorChangeRequested(const QColor& currentColor);
    void onBackgroundColorChangeRequested(const QColor& currentColor);

private:
    void customizeFontDialog(QFontDialog& fontDialog);

    QWidget* mParentWidget;
    UBSettings* mSettings;
};

#endif // UBTEXTDELEGATEDIALOGHANDLER_H
