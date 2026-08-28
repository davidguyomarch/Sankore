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



#include "UBDocumentProxy.h"

#include <QLocale>
#include "frameworks/UBStringUtils.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBSettingsData.h"
#include "core/UBDocumentManager.h"


UBDocumentProxy::UBDocumentProxy()
    : mPageCount(0)
    , mHasDefaultImageBackground(false)
{
    init();
}

UBDocumentProxy::UBDocumentProxy(const UBDocumentProxy &rValue) :
    QObject()
{
    mPersistencePath = rValue.mPersistencePath;
    mMetaDatas = rValue.mMetaDatas;
    mIsModified = rValue.mIsModified;
    mPageCount = rValue.mPageCount;
    mHasDefaultImageBackground = false;
}

UBDocumentProxy::UBDocumentProxy(const QString& pPersistancePath)
    : mPageCount(0)
    , mHasDefaultImageBackground(false)
{
    init();
    setPersistencePath(pPersistancePath);
}

bool UBDocumentProxy::theSameDocument(UBDocumentProxy *proxy)
{
    return  proxy && mPersistencePath == proxy->mPersistencePath;
}

void UBDocumentProxy::init()
{
    mSettings = UBSettings::settings();

    setMetaData(UBSettingsData::documentGroupName, "");

    QDateTime now = QDateTime::currentDateTime();
    setMetaData(UBSettingsData::documentName, QLocale().toString(now, QLocale::ShortFormat));

    setUuid(QUuid::createUuid());

    setDefaultDocumentSize(mSettings->pageSize->get().toSize());

    //teacherGuide metadata
    setMetaData(UBSettingsData::sessionTitle,"");
    setMetaData(UBSettingsData::sessionAuthors,"");
    setMetaData(UBSettingsData::sessionObjectives,"");
    setMetaData(UBSettingsData::sessionKeywords,"");
    setMetaData(UBSettingsData::sessionGradeLevel,"");
    setMetaData(UBSettingsData::sessionSubjects,"");
    setMetaData(UBSettingsData::sessionType,"");
    setMetaData(UBSettingsData::sessionLicence,"");
    // Issue 1684 - ALTI/AOU - 20131210
    setMetaData(UBSettingsData::documentDefaultBackgroundImage,"");
    setMetaData(UBSettingsData::documentDefaultBackgroundImageDisposition, "");
    // Fin Issue 1684 - ALTI/AOU - 20131210
}




UBDocumentProxy::~UBDocumentProxy()
{
    externalFilesClear(); // Issue 1683 - ALTI/AOU - 20131212
}


int UBDocumentProxy::pageCount()
{
    return mPageCount;
}


void UBDocumentProxy::setPageCount(int pPageCount)
{
    mPageCount = pPageCount;
}


int UBDocumentProxy::incPageCount()
{
    if (mPageCount <= 0)
    {
        mPageCount = 1;
    }
    else
    {
        mPageCount++;
    }

    return mPageCount;

}


int UBDocumentProxy::decPageCount()
{
    mPageCount --;

    if (mPageCount < 0)
    {
        mPageCount = 0;
    }

    return mPageCount;
}

QString UBDocumentProxy::persistencePath() const
{
    return mPersistencePath;
}

void UBDocumentProxy::setPersistencePath(const QString& pPersistencePath)
{
    if (pPersistencePath != mPersistencePath)
    {
        mIsModified = true;
        mPersistencePath = pPersistencePath;
    }
}

void UBDocumentProxy::setMetaData(const QString& pKey, const QVariant& pValue)
{
    if (mMetaDatas.contains(pKey) && mMetaDatas.value(pKey) == pValue)
        return;
    else
    {
        mIsModified = true;
        mMetaDatas.insert(pKey, pValue);
        if (pKey == UBSettingsData::documentUpdatedAt)
        {
            UBDocumentManager *documentManager = UBDocumentManager::documentManager();
            if (documentManager)
                documentManager->emitDocumentUpdated(this);
        }
    }
}

QVariant UBDocumentProxy::metaData(const QString& pKey) const
{
    if (mMetaDatas.contains(pKey))
    {
        return mMetaDatas.value(pKey);
    }
    else
    {
        qDebug() << "Unknown metadata key" << pKey;
        return QString(""); // failsafe
    }
}

QHash<QString, QVariant> UBDocumentProxy::metaDatas() const
{
    return mMetaDatas;
}

QString UBDocumentProxy::name() const
{
    return metaData(UBSettingsData::documentName).toString();
}

QString UBDocumentProxy::groupName() const
{
    return metaData(UBSettingsData::documentGroupName).toString();
}

QSize UBDocumentProxy::defaultDocumentSize() const
{
    if (mMetaDatas.contains(UBSettingsData::documentSize))
        return metaData(UBSettingsData::documentSize).toSize();
    else
        return mSettings->pageSize->get().toSize();
}

void UBDocumentProxy::setDefaultDocumentSize(QSize pSize)
{
    if (defaultDocumentSize() != pSize)
    {
        setMetaData(UBSettingsData::documentSize, QVariant(pSize));
        emit defaultDocumentSizeChanged();

        mIsModified = true;
    }
}

void UBDocumentProxy::setDefaultDocumentSize(int pWidth, int pHeight)
{
    setDefaultDocumentSize(QSize(pWidth, pHeight));
}


QUuid UBDocumentProxy::uuid() const
{
    QString id = metaData(UBSettingsData::documentIdentifer).toString();
    QString sUuid = id.replace(UBSettings::uniboardDocumentNamespaceUri + "/", "");

    return QUuid(sUuid);
}

void UBDocumentProxy::setUuid(const QUuid& uuid)
{
    setMetaData(UBSettingsData::documentIdentifer,
            UBSettings::uniboardDocumentNamespaceUri + "/" + UBStringUtils::toCanonicalUuid(uuid));
}


QDateTime UBDocumentProxy::documentDate()
{
    if(mMetaDatas.contains(UBSettingsData::documentDate))
        return UBStringUtils::fromUtcIsoDate(metaData(UBSettingsData::documentDate).toString());
    return QDateTime::currentDateTime();
}

QDateTime UBDocumentProxy::lastUpdate()
{
    if(mMetaDatas.contains(UBSettingsData::documentUpdatedAt))
        return UBStringUtils::fromUtcIsoDate(metaData(UBSettingsData::documentUpdatedAt).toString());
    return QDateTime().currentDateTime();
}

bool UBDocumentProxy::isModified() const
{
    return mIsModified;
}


void UBDocumentProxy::setHasDefaultImageBackground(const bool hasDefault)
{
   mHasDefaultImageBackground = hasDefault;
   // Issue 1684 - ALTI/AOU - 20131210
   if (hasDefault == false){
        setMetaData(UBSettingsData::documentDefaultBackgroundImage, "");
        setMetaData(UBSettingsData::documentDefaultBackgroundImageDisposition, "");
   // Fin Issue 1684 - ALTI/AOU - 20131210
   }
}

bool UBDocumentProxy::hasDefaultImageBackground() const
{
    return mHasDefaultImageBackground;
}

void UBDocumentProxy::setDefaultImageBackground(const UBFeature& item)
{
   mDefaultImageBackground = item;
}

UBFeature& UBDocumentProxy::defaultImageBackground()
{
    return mDefaultImageBackground;
}

const UBFeature& UBDocumentProxy::defaultImageBackground() const
{
    return mDefaultImageBackground;
}

// Issue 1683 - ALTI/AOU - 20131212
void UBDocumentProxy::externalFilesAdd(UBDocumentExternalFile *file)
{
    mExternalFiles.append(file);
}

const QList<UBDocumentExternalFile *> *UBDocumentProxy::externalFiles() const
{
    return &mExternalFiles;
}

void UBDocumentProxy::externalFilesClear(){
    for (UBDocumentExternalFile* ef : mExternalFiles)
    {
        delete ef;
        ef = nullptr;
    }
    mExternalFiles.clear();
}
// Fin Issue 1683 - ALTI/AOU - 20131212

