/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

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

#include "UBTextDelegateDialogHandler.h"

#include <QFontDialog>
#include <QColorDialog>
#include <QListView>
#include <QComboBox>
#include <QStringListModel>

#include "core/UBSettings.h"
#include "UBResources.h"
#include "domain/UBGraphicsTextItemDelegate.h"

UBTextDelegateDialogHandler::UBTextDelegateDialogHandler(QWidget* parentWidget, UBSettings* settings, QObject* parent)
    : QObject(parent)
    , mParentWidget(parentWidget)
    , mSettings(settings)
{
}

void UBTextDelegateDialogHandler::connectToDelegate(UBGraphicsTextItemDelegate* delegate)
{
    connect(delegate, &UBGraphicsTextItemDelegate::fontChangeRequested,
            this, &UBTextDelegateDialogHandler::onFontChangeRequested);
    connect(delegate, &UBGraphicsTextItemDelegate::textColorChangeRequested,
            this, &UBTextDelegateDialogHandler::onTextColorChangeRequested);
    connect(delegate, &UBGraphicsTextItemDelegate::backgroundColorChangeRequested,
            this, &UBTextDelegateDialogHandler::onBackgroundColorChangeRequested);
}

void UBTextDelegateDialogHandler::disconnectFromDelegate(UBGraphicsTextItemDelegate* delegate)
{
    disconnect(delegate, &UBGraphicsTextItemDelegate::fontChangeRequested,
               this, &UBTextDelegateDialogHandler::onFontChangeRequested);
    disconnect(delegate, &UBGraphicsTextItemDelegate::textColorChangeRequested,
               this, &UBTextDelegateDialogHandler::onTextColorChangeRequested);
    disconnect(delegate, &UBGraphicsTextItemDelegate::backgroundColorChangeRequested,
               this, &UBTextDelegateDialogHandler::onBackgroundColorChangeRequested);
}

void UBTextDelegateDialogHandler::onFontChangeRequested(const QFont& currentFont)
{
    UBGraphicsTextItemDelegate* delegate = qobject_cast<UBGraphicsTextItemDelegate*>(sender());
    if (!delegate)
        return;

    QFontDialog fontDialog(currentFont, mParentWidget);
    customizeFontDialog(fontDialog);

    if (fontDialog.exec())
    {
        delegate->applyFont(fontDialog.selectedFont());
    }
}

void UBTextDelegateDialogHandler::onTextColorChangeRequested(const QColor& currentColor)
{
    UBGraphicsTextItemDelegate* delegate = qobject_cast<UBGraphicsTextItemDelegate*>(sender());
    if (!delegate)
        return;

    QColorDialog colorDialog(currentColor, mParentWidget);
    colorDialog.setWindowTitle(tr("Text Color"));
    if (mSettings->isDarkBackground())
    {
        colorDialog.setStyleSheet("background-color: white;");
    }

    if (colorDialog.exec())
    {
        delegate->applyTextColor(colorDialog.selectedColor());
    }
}

void UBTextDelegateDialogHandler::onBackgroundColorChangeRequested(const QColor& currentColor)
{
    UBGraphicsTextItemDelegate* delegate = qobject_cast<UBGraphicsTextItemDelegate*>(sender());
    if (!delegate)
        return;

    QColorDialog colorDialog(currentColor, mParentWidget);
    colorDialog.setWindowTitle(tr("Background Color"));
    if (mSettings->isDarkBackground())
    {
        colorDialog.setStyleSheet("background-color: white;");
    }

    if (colorDialog.exec())
    {
        delegate->applyBackgroundColor(colorDialog.selectedColor());
    }
}

void UBTextDelegateDialogHandler::customizeFontDialog(QFontDialog& fontDialog)
{
    fontDialog.setOption(QFontDialog::DontUseNativeDialog);

    if (mSettings->isDarkBackground()) {
        fontDialog.setStyleSheet("background-color: white;");
    }

    QListView* fontNameListView = nullptr;
    QList<QListView*> listViews = fontDialog.findChildren<QListView*>();
    if (listViews.count() > 0)
    {
        fontNameListView = listViews.at(0);
        for (QListView* listView : listViews)
        {
            if (listView->pos().x() < fontNameListView->pos().x())
                fontNameListView = listView;
        }
    }

    if (fontNameListView)
    {
        QStringListModel* stringListModel = dynamic_cast<QStringListModel*>(fontNameListView->model());
        if (stringListModel)
        {
            QStringList dialogFontNames = stringListModel->stringList();
            QStringList safeWebFontNames;
            safeWebFontNames.append("Arial");
            safeWebFontNames.append("Arial Black");
            safeWebFontNames.append("Comic Sans MS");
            safeWebFontNames.append("Courier New");
            safeWebFontNames.append("Georgia");
            safeWebFontNames.append("Impact");
            safeWebFontNames.append("Times New Roman");
            safeWebFontNames.append("Trebuchet MS");
            safeWebFontNames.append("Verdana");

            QStringList customFontList = UBResources::resources()->customFontList();
            int index = 0;
            for (const QString& dialogFontName : dialogFontNames) {
                if (safeWebFontNames.contains(dialogFontName, Qt::CaseInsensitive)
                    || customFontList.contains(dialogFontName, Qt::CaseSensitive))
                    index++;
                else
                    stringListModel->removeRow(index);
            }
        }
    }

    QList<QComboBox*> comboBoxes = fontDialog.findChildren<QComboBox*>();
    if (comboBoxes.count() > 0)
        comboBoxes.at(0)->setEnabled(false);
}
