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



#include <QFileDialog>

#include "UBUpdateDlg.h"
#include "core/UBApplication.h"
#include "UBMainWindow.h"


UBUpdateDlg::UBUpdateDlg(QWidget *parent, int nbFiles, const QString& bkpPath)
    : QDialog(parent)
    , mMainLayout(nullptr)
    , mNbFilesLabel(nullptr)
    , mBkpLabel(nullptr)
    , mBkpPath(nullptr)
    , mBrowseBttn(nullptr)
    , mpDlgBttn(nullptr)
    , mLayout(nullptr)
    , mHLayout(nullptr)
    , mStackedWidget(nullptr)
    , mDialogWidget(nullptr)
    , mProgressWidget(nullptr)
    , mProgressLayout(nullptr)
    , mProgressLabel(nullptr)

{
    mDialogWidget = new QWidget(this);
    mProgressWidget = new QWidget(this);

    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->addWidget(mDialogWidget);
    mStackedWidget->addWidget(mProgressWidget);

    setFixedSize(450, 110);
    setModal(true);
    setWindowTitle(tr("Document updater"));
    mLayout = new QVBoxLayout();
    setLayout(mLayout);

    QString str = QString::number(nbFiles);
    str.append(tr(" files require an update."));
    mNbFilesLabel = new QLabel(mDialogWidget);
    mNbFilesLabel->setText(str);

    mLayout->addWidget(mNbFilesLabel);

    mBkpLabel = new QLabel(mDialogWidget);
    mBkpLabel->setText(tr("Backup path: "));

    mBkpPath = new QLineEdit(mDialogWidget);
    mBkpPath->setText(bkpPath);

    mBrowseBttn = new QPushButton(mDialogWidget);
    mBrowseBttn->setText(tr("Browse"));

    mHLayout = new QHBoxLayout();
    mHLayout->addWidget(mBkpLabel);
    mHLayout->addWidget(mBkpPath, 1);
    mHLayout->addWidget(mBrowseBttn);

    mLayout->addLayout(mHLayout);

    mpDlgBttn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, mDialogWidget);
    mLayout->addWidget(mpDlgBttn);

    mpDlgBttn->button(QDialogButtonBox::Ok)->setText(tr("Update"));
    mpDlgBttn->button(QDialogButtonBox::Cancel)->setText(tr("Remind me later"));

    QObject::connect(mBrowseBttn, &QPushButton::clicked, this, [this]() { onBrowse(); });
    QObject::connect(mpDlgBttn, &QDialogButtonBox::accepted, this, &UBUpdateDlg::onUpdate);
    QObject::connect(mpDlgBttn, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mDialogWidget->setLayout(mLayout);
    mStackedWidget->setCurrentWidget(mDialogWidget);

    mMainLayout = new QVBoxLayout();
    this->setLayout(mMainLayout);
    mMainLayout->addWidget(mStackedWidget);
}

UBUpdateDlg::~UBUpdateDlg()
{
    if (nullptr != mpDlgBttn)
    {
        delete mpDlgBttn;
        mpDlgBttn = nullptr;
    }

    if (mNbFilesLabel) {
        delete mNbFilesLabel;
        mNbFilesLabel = nullptr;
    }

    if (mBkpLabel) {
        delete mBkpLabel;
        mBkpLabel = nullptr;
    }

    if (mBkpPath) {
        delete mBkpPath;
        mBkpPath = nullptr;
    }

    if (mBrowseBttn) {
        delete mBrowseBttn;
        mBrowseBttn = nullptr;
    }

    if (mProgressLabel) {
        delete mProgressLabel;
        mProgressLabel = nullptr;
    }

    if (mHLayout) {
        delete mHLayout;
        mHLayout = nullptr;
    }

    if (mLayout) {
        delete mLayout;
        mLayout = nullptr;
    }

    if (mProgressLayout) {
        delete mProgressLayout;
        mProgressLayout = nullptr;
    }

    if (mDialogWidget) {
        delete mDialogWidget;
        mDialogWidget = nullptr;
    }

    if (mProgressWidget) {
        delete mProgressWidget;
        mProgressWidget = nullptr;
    }

    if (mStackedWidget) {
        delete mStackedWidget;
        mStackedWidget = nullptr;
    }

    if (mMainLayout) {
        delete mMainLayout;
        mMainLayout = nullptr;
    }
}

void UBUpdateDlg::onBrowse()
{
    QString qsSelectedDir;
    qsSelectedDir = QFileDialog::getExistingDirectory(this, tr("Select a backup folder"), mBkpPath->text());
    mBkpPath->setText(qsSelectedDir);
}

void UBUpdateDlg::onUpdate()
{
    mProgressLabel = new QLabel(mProgressWidget);
    mProgressLabel->setText(tr("Please wait the import process will start soon..."));
    mProgressLayout = new QHBoxLayout();
    mProgressLayout->addWidget(mProgressLabel);
    mProgressWidget->setLayout(mProgressLayout);
    mStackedWidget->setCurrentWidget(mProgressWidget);
    emit updateFiles();
}

void UBUpdateDlg::onFilesUpdated(bool bResult)
{
    this->hide();
    QString qsMsg;

    if (bResult) {
        qsMsg = tr("Files update successful!\nPlease reboot the application to access the updated documents.");
    }
    else {
        qsMsg = tr("An error occured during the update. The files have not been affected.");
    }
    UBApplication::mainWindow->information(tr("Files update results"), qsMsg);
}

QString UBUpdateDlg::backupPath()
{
    return mBkpPath->text();
}

void UBUpdateDlg::transitioningFile(QString fileName)
{
    mProgressLabel->setText(tr("Updating file ") + fileName);
}

