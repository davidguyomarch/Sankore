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

#include "UBMetadataLoader.h"
#include "core/UBSettings.h"
#include "core/UBSettingsData.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

namespace UBMetadataLoader
{

const QString metadataFilename = "metadata.rdf";

QMap<QString, QVariant> load(const QString& pPath, UBSettings* settings)
{
    if (!settings)
        settings = UBSettings::settings();

    QMap<QString, QVariant> metadata;

    QString fileName = pPath + "/" + metadataFilename;

    QFile file(fileName);

    bool sizeFound = false;
    bool updatedAtFound = false;
    QString date;

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << fileName << " for reading ...";
            return metadata;
        }

        QString docVersion = "4.1"; // untagged doc version 4.1
        metadata.insert(UBSettingsData::documentVersion, docVersion);

        QXmlStreamReader xml(&file);

        while (!xml.atEnd())
        {
            xml.readNext();

            if (xml.isStartElement())
            {
                if (xml.name() == QLatin1String("title"))
                {
                    metadata.insert(UBSettingsData::documentName, xml.readElementText());
                }
                else if (xml.name() == QLatin1String("type"))
                {
                    metadata.insert(UBSettingsData::documentGroupName, xml.readElementText());
                }
                else if (xml.name() == QLatin1String("date"))
                {
                    date = xml.readElementText();
                }
                else if (xml.name() == QLatin1String("identifier"))
                {
                    metadata.insert(UBSettingsData::documentIdentifer, xml.readElementText());
                }
                else if (xml.name() == QLatin1String("version")
                        && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    docVersion = xml.readElementText();
                    metadata.insert(UBSettingsData::documentVersion, docVersion);
                }
                else if (xml.name() == QLatin1String("size")
                        && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    QString size = xml.readElementText();
                    QStringList sizeParts = size.split("x");
                    bool ok = false;
                    if (sizeParts.count() >= 2)
                    {
                        bool widthOK, heightOK;
                        int width = sizeParts.at(0).toInt(&widthOK);
                        int height = sizeParts.at(1).toInt(&heightOK);
                        ok = widthOK && heightOK;

                        QSize docSize(width, height);

                        if (width == 1024 && height == 768) // migrate from 1024/768 to pageSize
                        {
                            docSize = settings->pageSize->get().toSize();
                        }

                        metadata.insert(UBSettingsData::documentSize, QVariant(docSize));
                    }
                    if (!ok)
                    {
                        qWarning() << "Invalid document size:" << size;
                    }

                    sizeFound = true;
                }
                else if (xml.name() == QLatin1String("updated-at")
                        && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::documentUpdatedAt, xml.readElementText());
                    updatedAtFound = true;
                }
                else if (xml.name() == UBSettingsData::sessionTitle
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionTitle, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionAuthors
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionAuthors, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionObjectives
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionObjectives, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionKeywords
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionKeywords, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionGradeLevel
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionGradeLevel, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionSubjects
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionSubjects, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionType
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionType, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::sessionLicence
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::sessionLicence, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::documentDefaultBackgroundImage
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::documentDefaultBackgroundImage, xml.readElementText());
                }
                else if (xml.name() == UBSettingsData::documentDefaultBackgroundImageDisposition
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettingsData::documentDefaultBackgroundImageDisposition, xml.readElementText());
                }
            }

            if (xml.hasError())
            {
                qWarning() << "error parsing sankore metadata.rdf file " << xml.errorString();
            }
        }

        file.close();
    }

    if (!sizeFound)
    {
        QSize docSize(1280, 960); // sensible default
        QScreen* primaryScreen = QGuiApplication::primaryScreen();
        if (primaryScreen) {
            docSize = primaryScreen->geometry().size();
            docSize.setHeight(docSize.height() - 70); // 70 = toolbar height
        }
        metadata.insert(UBSettingsData::documentSize, QVariant(docSize));
    }

    // Update old files date format
    QString dateString = metadata.value(UBSettingsData::documentDate).toString();
    if (dateString.length() < 10) {
        metadata.remove(UBSettingsData::documentDate);
        metadata.insert(UBSettingsData::documentDate, dateString + "T00:00:00Z");
    }

    if (!updatedAtFound) {
        metadata.insert(UBSettingsData::documentUpdatedAt, dateString);
    }

    metadata.insert(UBSettingsData::documentDate, QVariant(date));

    return metadata;
}

} // namespace UBMetadataLoader
