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



#include "UBMessagesDialog.h"
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFileDialog>

UBMessagesDialog::UBMessagesDialog(QString windowTitle, QWidget *parent)
: QWidget(parent)
{
    resize(400, 0);

    setWindowTitle(windowTitle);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

}

void UBMessagesDialog::setMessages(const QList<QString> messages)
{
    mMessages = messages;

    if (mMessages.count())
    {        
        QVBoxLayout *messagesLayout = new QVBoxLayout(this);
        for (const QString& message : mMessages)
        {
            QTextEdit *messageBox = new QTextEdit(this);
            messageBox->setMinimumHeight(55);
            messageBox->setReadOnly(true);
            messageBox->setFocusPolicy(Qt::NoFocus);
            messageBox->setText(message);
            messagesLayout->addWidget(messageBox);
        }
        QPushButton *closeButton = new QPushButton(tr("Close"), this);
        connect(closeButton, &QPushButton::clicked, this, [this]() { dispose(); });

        messagesLayout->addWidget(closeButton);

        setLayout(messagesLayout);
    }
}

void UBMessagesDialog::dispose()
{
    delete this;
}