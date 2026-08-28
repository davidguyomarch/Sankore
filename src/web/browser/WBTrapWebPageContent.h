/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 * Copyright (C) 2026 David Guyomarch
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


#ifndef WB_TRAP_WEB_PAGE_CONTENT_H
#define WB_TRAP_WEB_PAGE_CONTENT_H

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QWebEngineView>

#include "WBWebView.h"

class WBTrapWebPageContentWindow : public QDialog
{
    Q_OBJECT

    public:
        WBTrapWebPageContentWindow(QObject *controller, QWidget *parent = nullptr);
        virtual ~WBTrapWebPageContentWindow();

        void setUrl(const QUrl &url);
        void setReadyForTrap(bool bReady);
        QWebEngineView *webView() const {return mTrapContentPreview;}
        QComboBox *itemsComboBox() const {return mSelectContentCombobox;}
        QLineEdit *applicationNameLineEdit() const {return mApplicationNameEdit;}

    private:

        QObject *mController;

        QHBoxLayout *mTrapApplicationHLayout;
        QVBoxLayout *mTrapApplicationVLayout;
        QHBoxLayout *mSelectContentLayout;
        QLabel *mSelectContentLabel;
        QComboBox *mSelectContentCombobox;

        WBWebView *mTrapContentPreview;
        UBWebTrapMouseEventMask *mTrapingWidgetMask;

        QHBoxLayout *mApplicationNameLayout;
        QLabel *mApplicationNameLabel;
        QLineEdit *mApplicationNameEdit;
        QToolButton *mShowDisclaimerToolButton;
        QList<QToolButton *> mTrapButtons;
        QVariant mLastHitTestResult;

};

#endif //WB_TRAP_WEB_PAGE_CONTENT_H
