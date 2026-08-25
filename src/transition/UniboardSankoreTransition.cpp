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



#include "UniboardSankoreTransition.h"
#include <QStandardPaths>
#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "gui/UBMainWindow.h"


UniboardSankoreTransition::UniboardSankoreTransition(QObject *parent) :
    QObject(parent)
    , mTransitionDlg(nullptr)
    , mThread(nullptr)
{
    mOldSankoreDirectory = UBFileSystemUtils::normalizeFilePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    mUniboardSourceDirectory = UBFileSystemUtils::normalizeFilePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#if defined(Q_OS_MACOSX)
    mOldSankoreDirectory.replace("Sankore/Sankore 3.1", "Sankore");
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Uniboard");
#else
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Mnemis/Uniboard");
#endif
    connect(this, &UniboardSankoreTransition::docAdded, UBPersistenceManager::persistenceManager(), &UBPersistenceManager::documentCreated);
}

UniboardSankoreTransition::~UniboardSankoreTransition()
{
    if(nullptr != mTransitionDlg)
    {
        delete mTransitionDlg;
        mTransitionDlg = nullptr;
    }

    if(mThread){
        delete mThread;
        mThread = nullptr;
    }
}

void UniboardSankoreTransition::rollbackDocumentsTransition(QFileInfoList& fileInfoList)
{
    QFileInfoList::iterator fileInfo;
    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo += 1) {
        if (fileInfo->isDir() && fileInfo->fileName().startsWith("Uniboard Document ")){
            QString sankoreDocumentName = fileInfo->fileName();
            sankoreDocumentName.replace("Uniboard","Sankore");
            QString sankoreDocumentDirectoryPath = UBSettings::userDocumentDirectory() + "/" + sankoreDocumentName;
            if (QFileInfo(sankoreDocumentDirectoryPath).exists()){
                UBFileSystemUtils::deleteDir(sankoreDocumentDirectoryPath);
            }
        }
    }
}

bool UniboardSankoreTransition::checkDocumentDirectory(QString& documentDirectoryPath)
{
    bool result = true;
    result = updateSankoreHRef(documentDirectoryPath);
    QString sankoreWidgetPath = documentDirectoryPath + "/widgets";
    result &= updateIndexWidget(sankoreWidgetPath);
    return result;
}

void UniboardSankoreTransition::documentTransition()
{
    if (QFileInfo(mUniboardSourceDirectory).exists() || QFileInfo(mOldSankoreDirectory).exists()){
        QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";

        QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);
        fileInfoList << UBFileSystemUtils::allElementsInDirectory(mOldSankoreDirectory + "/document");

        QString backupDirectoryPath = UBFileSystemUtils::normalizeFilePath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

        if (fileInfoList.count() != 0){
            mTransitionDlg = new UBUpdateDlg(nullptr, fileInfoList.count(), backupDirectoryPath);
            connect(mTransitionDlg, &UBUpdateDlg::updateFiles, this, &UniboardSankoreTransition::startDocumentTransition);
            connect(this, &UniboardSankoreTransition::transitionFinished, mTransitionDlg, &UBUpdateDlg::onFilesUpdated);
            mTransitionDlg->show();
        }
    }
    else{
        UBApplication::mainWindow->information(tr("Import old Uniboard/Sankore documents"), tr("There are no documents that should be imported"));
    }
}

void UniboardSankoreTransition::startDocumentTransition()
{
    mThread = new UniboardSankoreThread(this);
    mThread->start();
    connect(this, &UniboardSankoreTransition::transitioningFile, mTransitionDlg, &UBUpdateDlg::transitioningFile);
}


bool UniboardSankoreTransition::checkPage(QString& sankorePagePath)
{
    QFile file(sankorePagePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray documentByteArray = file.readAll();
    file.close();

    QString sankoreDirectory = sankorePagePath.left(sankorePagePath.indexOf("/page"));

    sankoreDirectory = QUrl::fromLocalFile(sankoreDirectory).toString();
    QString documentString(documentByteArray);

    QRegularExpression videoRegExp("<video(.*?)xlink:href=\"(.*?)videos/(.*?)/>");

    documentString.replace(videoRegExp,"<video\\1xlink:href=\"videos/\\3/>");


    QRegularExpression audioRegExp("<audio(.*?)xlink:href=\"(.*?)audios/(.*?)/>");

    documentString.replace(audioRegExp,"<audio\\1xlink:href=\"audios/\\3/>");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(documentString.toLatin1());
    file.close();

    return true;
}


bool UniboardSankoreTransition::checkWidget(QString& sankoreWidgetIndexPath)
{
    QFile file(sankoreWidgetIndexPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray documentByteArray = file.readAll();
    file.close();

    QString documentString(documentByteArray);

    QRegularExpression swfOriginFilePathRegExp("<param name=\"movie\" value=\"(.*?)\">");
    QRegularExpressionMatch originMatch = swfOriginFilePathRegExp.match(documentString);
    QString origin = originMatch.captured(1);
    if(origin.contains("http://")){
        // an url is the source of the swf. The source is kept as is.
        return true;
    }

    //changing the path
    QRegularExpression swfDataPathRegExp("<object(.*?)data=\"(.*?)interactive content/Web/(.*?)\"(.*?)>");
    documentString.replace(swfDataPathRegExp,"<object\\1data=\"\\3\">");

    QRegularExpression swfMoviePathRegExp("<param name=\"movie\" value=\"(.*?)interactive content/Web/(.*?)\">");
    documentString.replace(swfMoviePathRegExp,"<param name=\"movie\" value=\"\\2\">");

    //copy the swf on the right place
    QRegularExpression swfFileNameRegExp("<param name=\"movie\" value=\"(.*?)\">");
    QRegularExpressionMatch swfMatch = swfFileNameRegExp.match(documentString);
    QString swfFileName = swfMatch.captured(1);
    int lastDirectoryLevel = sankoreWidgetIndexPath.lastIndexOf("/");
    if (lastDirectoryLevel == -1)
        lastDirectoryLevel = sankoreWidgetIndexPath.lastIndexOf("\\");


    QString destination(sankoreWidgetIndexPath.left(lastDirectoryLevel) + "/" + swfFileName);
    QFile(origin).copy(destination);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(documentString.toLatin1());
    file.close();

    return true;
}


bool UniboardSankoreTransition::updateIndexWidget(QString& sankoreWidgetPath)
{
    bool result = true;
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(sankoreWidgetPath);

    QFileInfoList::iterator fileInfo;
    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->fileName().endsWith("wgt")){
            QString path = fileInfo->absolutePath() + "/" + fileInfo->fileName() + "/index.html";
            if (QFile(path).exists())
                result = checkWidget(path);

            path = fileInfo->absolutePath() + "/" + fileInfo->fileName() + "/index.htm";
            if (QFile(path).exists())
                result &= checkWidget(path);
        }
    }

    return result;
}

bool UniboardSankoreTransition::updateSankoreHRef(QString& sankoreDocumentPath)
{
    bool result = true;
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(sankoreDocumentPath);

    QFileInfoList::iterator fileInfo;

    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->fileName().endsWith("svg")){
            QString path = fileInfo->absolutePath() + "/" + fileInfo->fileName();
            result = checkPage(path);
        }
    }

    return result;
}

void UniboardSankoreTransition::executeTransition()
{
    bool result = false;
    QString backupDestinationPath = mTransitionDlg->backupPath() + "/OldSankoreAndUniboardVersionsBackup";
    result = UBFileSystemUtils::copyDir(mUniboardSourceDirectory + "/document", backupDestinationPath);
    result &= UBFileSystemUtils::copyDir(mOldSankoreDirectory + "/document", backupDestinationPath);

    QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);
    fileInfoList.append(UBFileSystemUtils::allElementsInDirectory(mOldSankoreDirectory + "/document"));

    QFileInfoList::iterator fileInfo;
    QString sankoreDocumentDirectory = UBSettings::userDocumentDirectory();

    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->isDir() && (fileInfo->fileName().startsWith("Uniboard Document ") || fileInfo->fileName().startsWith("Sankore Document "))){
            QString sankoreDocumentName = fileInfo->fileName();
            emit transitioningFile(sankoreDocumentName);
            sankoreDocumentName.replace("Uniboard","Sankore");
            QString sankoreDocumentPath = sankoreDocumentDirectory + "/" + sankoreDocumentName;
            result = UBFileSystemUtils::copyDir(fileInfo->filePath(),sankoreDocumentPath);
            result &= updateSankoreHRef(sankoreDocumentPath);
            QString sankoreWidgetPath = sankoreDocumentDirectory +  "/" + sankoreDocumentName + "/widgets";
            result &= updateIndexWidget(sankoreWidgetPath);
        }
    }

    if (!result){
        qWarning() << "The transition has failed";
        rollbackDocumentsTransition(fileInfoList);
        UBFileSystemUtils::deleteDir(backupDestinationPath);
    }
    else{
        UBFileSystemUtils::deleteDir(mOldSankoreDirectory);
        UBFileSystemUtils::deleteDir(mUniboardSourceDirectory);
    }

    emit transitionFinished(result);
}


UniboardSankoreThread::UniboardSankoreThread(QObject* parent):QThread(parent)
{

}

UniboardSankoreThread::~UniboardSankoreThread()
{

}

void UniboardSankoreThread::run()
{
    UniboardSankoreTransition* pTransition = dynamic_cast<UniboardSankoreTransition*>(parent());

    pTransition->executeTransition();
}

