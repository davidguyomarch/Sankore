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



#include <QRegularExpression>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
// QScriptValue/QScriptEngine removed in Qt6 - use QJsonDocument instead
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "UBOEmbedParser.h"
#include "UBOEmbedUtils.h"


UBOEmbedParser::UBOEmbedParser(QObject *parent, const char* name)
{
    Q_UNUSED(parent);
    setObjectName(name);
    mParsedTitles.clear();
    connect(this, &UBOEmbedParser::parseContent, this, &UBOEmbedParser::onParseContent);
}

UBOEmbedParser::~UBOEmbedParser()
{

}

void UBOEmbedParser::setNetworkAccessManager(QNetworkAccessManager *nam)
{
    mpNam = nam;
    connect(mpNam, &QNetworkAccessManager::finished, this, &UBOEmbedParser::onFinished);
}

void UBOEmbedParser::parse(const QString& html)
{
    mContents.clear();
    QString query = "<link([^>]*)>";
    QRegularExpression exp(query);
    QStringList results;
    int count = 0;
    QRegularExpressionMatchIterator it = exp.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ++count;
        if("" != match.captured(1)){
            results << match.captured(1);
        }
    }

    QVector<QString> oembedUrls;

    if(2 <= results.size()){
        for(int i=1; i<results.size(); i++){
            if("" != results.at(i)){
                QString qsNode = QString("<link%0>").arg(results.at(i));
                QDomDocument domDoc;
                domDoc.setContent(qsNode);
                QDomNode linkNode = domDoc.documentElement();

                //  At this point, we have a node that is the <link> element. Now we have to parse its attributes
                //  in order to check if it is a oEmbed node or not
                QDomAttr typeAttribute = linkNode.toElement().attributeNode("type");
                if(typeAttribute.value().contains("oembed")){
                    // The node is an oembed one! We have to get the url and the type of oembed encoding
                    QDomAttr hrefAttribute = linkNode.toElement().attributeNode("href");
                    QString url = hrefAttribute.value();
                    oembedUrls.append(url);
                }
            }
        }
    }

    mPending = oembedUrls.size();

    if(0 == mPending){
        emit oembedParsed(mContents);
    }else{
        // Here we start the parsing (finally...)!
        for(int i=0; i<oembedUrls.size(); i++){
            emit parseContent(oembedUrls.at(i));
        }
    }
}

/**
  /brief Extract the oembed infos from the JSON
  @param jsonUrl as the url of the JSON file
  */
sOEmbedContent UBOEmbedParser::getJSONInfos(const QString &json)
{
    return UBOEmbedUtils::parseJSON(json);
}

/**
  /brief Extract the oembed infos from the XML
  @param xmlUrl as the url of the XML file
  */
sOEmbedContent UBOEmbedParser::getXMLInfos(const QString &xml)
{
    return UBOEmbedUtils::parseXML(xml);
}

void UBOEmbedParser::onParseContent(QString url)
{
    QUrl qurl = QUrl::fromEncoded(url.toLatin1());

    QNetworkRequest req;
    req.setUrl(qurl);
    if(nullptr != mpNam){
        mpNam->get(req);
    }
}

void UBOEmbedParser::onFinished(QNetworkReply *reply)
{
    if(QNetworkReply::NoError == reply->error()){
        QString receivedDatas = reply->readAll().constData();
        sOEmbedContent crntContent;
        // The received datas can be in two different formats: JSON or XML
        if(receivedDatas.contains("<oembed>")){
            // XML !
            crntContent = getXMLInfos(receivedDatas);
        }else if(receivedDatas.contains("{\"provider_url")){
            // JSON !
            crntContent = getJSONInfos(receivedDatas);
        }

        //  As we don't want duplicates, we have to check if the content title has already
        //  been parsed.
        if("" != crntContent.title && !mParsedTitles.contains(crntContent.title)){
            mParsedTitles << crntContent.title;
            mContents << crntContent;
        }

    }else{
        //  We decided to not handle the error case here. If there is a problem with
        //  getting the oembed content information, we just don't handle it: the content
        //  will not be available for importation.
    }

    // Decrement the number of content to analyze
    mPending--;
    if(0 == mPending){
        //  All the oembed contents have been parsed. We notify it!
        emit oembedParsed(mContents);
    }
}
