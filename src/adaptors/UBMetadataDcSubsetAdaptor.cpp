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



#include "UBMetadataDcSubsetAdaptor.h"
#include "UBMetadataLoader.h"

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QtXml>
#include <QScreen>
#include <QGuiApplication>

#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "document/UBDocumentProxy.h"


const QString UBMetadataDcSubsetAdaptor::nsRdf = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const QString UBMetadataDcSubsetAdaptor::nsDc = "http://purl.org/dc/elements/1.1/";
const QString UBMetadataDcSubsetAdaptor::metadataFilename = "metadata.rdf";


UBMetadataDcSubsetAdaptor::UBMetadataDcSubsetAdaptor()
{
    /*
     *
     * sample dublin core metadata
     *
     *
     *
        <?xml version="1.0"?>

        <rdf:RDF
        xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
        xmlns:dc= "http://purl.org/dc/elements/1.1/">

        <rdf:Description rdf:about="http://www.w3schools.com">
          <dc:title>D-Lib Program</dc:title>
          <dc:description>W3Schools - Free tutorials</dc:description>
          <dc:publisher>Refsnes Data as</dc:publisher>
          <dc:date>1999-09-01</dc:date>
          <dc:type>Web Development</dc:type>
          <dc:format>text/html</dc:format>
          <dc:language>en</dc:language>
        </rdf:Description>

        </rdf:RDF>
    */
}


UBMetadataDcSubsetAdaptor::~UBMetadataDcSubsetAdaptor()
{
    // NOOP
}


void UBMetadataDcSubsetAdaptor::persist(UBDocumentProxy* proxy)
{
    if(!QDir(proxy->persistencePath()).exists()){
        //In this case the a document is an empty document so we do not persist it
        return;
    }
    QString fileName = proxy->persistencePath() + "/" + metadataFilename;

    qDebug() << "persisting metadata to" << fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "cannot open " << fileName << " for writing ...";
        qCritical() << "error : "  << file.errorString();
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    xmlWriter.writeDefaultNamespace(nsRdf);
    xmlWriter.writeNamespace(nsDc, "dc");
    xmlWriter.writeNamespace(UBSettings::uniboardDocumentNamespaceUri, "ub");

    xmlWriter.writeStartElement("RDF");

    xmlWriter.writeStartElement("Description");
    xmlWriter.writeAttribute("about", proxy->metaData(UBSettings::documentIdentifer).toString());

    xmlWriter.writeTextElement(nsDc, "title", proxy->metaData(UBSettings::documentName).toString());
    xmlWriter.writeTextElement(nsDc, "type", proxy->metaData(UBSettings::documentGroupName).toString());
    xmlWriter.writeTextElement(nsDc, "date", proxy->metaData(UBSettings::documentDate).toString());
    xmlWriter.writeTextElement(nsDc, "format", "image/svg+xml");

    // introduced in UB 4.2
    xmlWriter.writeTextElement(nsDc, "identifier", proxy->metaData(UBSettings::documentIdentifer).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "version", UBSettings::currentFileVersion);
    QString width = QString::number(proxy->defaultDocumentSize().width());
    QString height = QString::number(proxy->defaultDocumentSize().height());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "size", QString("%1x%2").arg(width).arg(height));

    // introduced in UB 4.4
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "updated-at", UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTimeUtc()));
    // introduced in OpenSankore 1.40.00
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionTitle,proxy->metaData(UBSettings::sessionTitle).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionAuthors,proxy->metaData(UBSettings::sessionAuthors).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionObjectives,proxy->metaData(UBSettings::sessionObjectives).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionKeywords,proxy->metaData(UBSettings::sessionKeywords).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionGradeLevel,proxy->metaData(UBSettings::sessionGradeLevel).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionSubjects,proxy->metaData(UBSettings::sessionSubjects).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionType,proxy->metaData(UBSettings::sessionType).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionLicence,proxy->metaData(UBSettings::sessionLicence).toString());
    // Issue 1684 - ALTI/AOU - 20131210
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::documentDefaultBackgroundImage, proxy->metaData(UBSettings::documentDefaultBackgroundImage).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::documentDefaultBackgroundImageDisposition, proxy->metaData(UBSettings::documentDefaultBackgroundImageDisposition).toString());
    // Fin Issue 1684 - ALTI/AOU - 20131210

    //Issue N/C - NNE - 20140526
    if(proxy->metaData(UBSettings::documentTagVersion).toString().isEmpty() == false)
        xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, UBSettings::documentTagVersion, proxy->metaData(UBSettings::documentTagVersion).toString());
    //Issue N/C - NNE - 20140526 : END

    xmlWriter.writeEndElement(); //dc:Description
    xmlWriter.writeEndElement(); //RDF

    xmlWriter.writeEndDocument();

    file.flush();
    file.close();
}


QMap<QString, QVariant> UBMetadataDcSubsetAdaptor::load(QString pPath, UBSettings* settings)
{
    return UBMetadataLoader::load(pPath, settings);
}

