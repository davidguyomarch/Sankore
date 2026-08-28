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



#include "UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "document/UBDocumentContainer.h"
#include "globals/UBGlobals.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"
#include <QDir>

bool UBFileSystemUtils::isAZipFile(QString &filePath)
{
   if(QFileInfo(filePath).isDir()) return false;
   QFile file(filePath);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
       return false;

   bool result = false;
   QByteArray responseArray = file.readLine(10);
   QString responseString(responseArray);

   result = responseString.startsWith("pk", Qt::CaseInsensitive);

   file.close();
   return result;
}

QString UBFileSystemUtils::digitFileFormat(const QString& s, int digit)
{
    int pageDigit = UBDocumentContainer::pageFromSceneIndex(digit);
    return s.arg(pageDigit, 3, 10, QLatin1Char('0'));
}


bool UBFileSystemUtils::compressDirInZip(const QDir& pDir, const QString& pDestPath,
                QuaZipFile *pOutZipFile, bool pRootDocumentFolder, UBProcessingProgressListener* progressListener)
{
    QFileInfoList files = pDir.entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    QStringList filters;
    filters << "*.svg";
    QFileInfoList pageFiles = pDir.entryInfoList(filters);

    for (const QFileInfo& file : files)
    {
        if (file.isDir())
        {
            QDir dir(file.absoluteFilePath());
            if (!compressDirInZip(dir, pDestPath + dir.dirName() + "/" , pOutZipFile, false))
            {
                return false;
            }
        }

        if (file.isFile())
        {
            QString objectType;
            if (pRootDocumentFolder)
            {
                objectType = "Page";
            }
            else
            {
                objectType = pDir.dirName();
            }

            if (!pRootDocumentFolder)
            {
                if (progressListener)
                    progressListener->processing(objectType, files.indexOf(file), files.size());
            }
            // we ignore thumbnails message because it is very fast.
            else if (progressListener && file.suffix() == "svg")
            {
                progressListener->processing(objectType, pageFiles.indexOf(file), pageFiles.size());
            }

            QFile inFile(file.absoluteFilePath());
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: inFile.open(): " << inFile.errorString();
                return false;
            }

            qDebug() << "will open" << pDestPath << file.fileName() << inFile.fileName();

            if(!pOutZipFile->open(QIODevice::WriteOnly, QuaZipNewInfo(pDestPath + file.fileName(), inFile.fileName())))
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: outFile.open(): " << pOutZipFile->getZipError();
                inFile.close();
                return false;
            }

            pOutZipFile->write(inFile.readAll());
            if(pOutZipFile->getZipError() != UNZ_OK)
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: outFile.write(): " << pOutZipFile->getZipError();

                inFile.close();
                pOutZipFile->close();
                return false;
            }

            pOutZipFile->close();
            if(pOutZipFile->getZipError() != UNZ_OK)
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: outFile.close(): " << pOutZipFile->getZipError();

                inFile.close();
                pOutZipFile->close();
                return false;
            }

            pOutZipFile->close();
            inFile.close();
        }
    }

    return true;
}



bool UBFileSystemUtils::expandZipToDir(const QFile& pZipFile, const QDir& pTargetDir)
{
    QuaZip zip(pZipFile.fileName());

    if(!zip.open(QuaZip::mdUnzip))
    {
        qWarning() << "ZIP expand failed. Cause zip.open(): " << zip.getZipError();
        return false;
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    QString documentRootFolder = pTargetDir.absolutePath();

    if(!pTargetDir.exists())
        pTargetDir.mkpath(documentRootFolder);

    QFile out;
    char c;
    for(bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if(!zip.getCurrentFileInfo(&info))
        {
            //TOD UB 4.3 O display error to user or use crash reporter
            qWarning() << "ZIP expand failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            return false;
        }

        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "ZIP expand failed. Cause: file.open(): " << zip.getZipError();
            return false;
        }

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "ZIP expand failed. Cause: file.getFileName(): " << zip.getZipError();
            return false;
        }

        QString newFileName = documentRootFolder + "/" + file.getActualFileName();
        QFileInfo newFileInfo(newFileName);
        QDir root(documentRootFolder);
        root.mkpath(newFileInfo.absolutePath());

        out.setFileName(newFileName);
        out.open(QIODevice::WriteOnly);

        // Slow like hell (on GNU/Linux at least), but it is not my fault.
        // Not ZIP/UNZIP package's fault either.
        // The slowest thing here is out.putChar(c).
        QByteArray outFileContent = file.readAll();
        if (out.write(outFileContent) == -1)
        {
            // qWarning() << "ZIP expand failed. Cause: Unable to write file";
            // this may happen if we are decompressing a directory
        }

        while(file.getChar(&c))
            out.putChar(c);

        out.close();

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "ZIP expand failed. Cause: " << zip.getZipError();
            return false;
        }

        if(!file.atEnd())
        {
            qWarning() << "ZIP expand failed. Cause: read all but not EOF";
            return false;
        }

        file.close();

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "ZIP expand failed. Cause: file.close(): " <<  file.getZipError();
            return false;
        }

    }

    zip.close();

    if(zip.getZipError()!= UNZ_OK)
    {
      qWarning() << "ZIP expand failed. Cause: zip.close(): " << zip.getZipError();
      return false;
    }

    return true;
}


