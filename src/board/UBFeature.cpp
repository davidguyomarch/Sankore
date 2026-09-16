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

#include "UBFeature.h"

#include <QDebug>

#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"

UBFeature::UBFeature(const QString &url
                     , const QImage &icon
                     , const QString &name
                     , const QUrl &realPath
                     , UBFeatureElementType type
                     , Permissions pOwnPermissions
                     , QString pSortKey)
    : mThumbnail(icon)
    , mDisplayName(name)
    , mPath(realPath)
    , elementType(type)
    , mOwnPermissions(pOwnPermissions)
    , mSortKey(pSortKey)
    , mDisposition(Center)
{
    mName = getNameFromVirtualPath(url);
    virtualDir = getVirtualDirFromVirtualPath(url);
    if (mSortKey.isNull()) {
        mSortKey = name;
    }
}

UBFeature::~UBFeature()
{
}

QString UBFeature::getNameFromVirtualPath(const QString &pVirtPath)
{
    QString result;
    int slashPos = pVirtPath.lastIndexOf("/");
    if (slashPos != -1) {
        result = pVirtPath.right(pVirtPath.count() - slashPos - 1);
    } else {
        qDebug() << "UBFeature: incorrect virtual path parameter specified";
    }

    return result;
}

QString UBFeature::getVirtualDirFromVirtualPath(const QString &pVirtPath)
{
    QString result;
    int slashPos = pVirtPath.lastIndexOf("/");
    if (slashPos != -1) {
        result = pVirtPath.left(slashPos);
    } else {
        qDebug() << "UBFeature: incorrect virtual path parameter specified";
    }

    return result;
}

QString UBFeature::getUrl() const
{
    if ( elementType == FEATURE_INTERNAL )
        return getFullPath().toString();

    return getFullPath().toLocalFile();
}

UBFeature &UBFeature::markedWithSortKey(const QString &str)
{
    mSortKey = str;
    return *this;
}

bool UBFeature::operator ==( const UBFeature &f )const
{
    return virtualDir == f.getVirtualPath() && mName == f.getName() && mPath == f.getFullPath() && elementType == f.getType();
}

bool UBFeature::operator !=( const UBFeature &f )const
{
    return !(*this == f);
}

bool UBFeature::isFolder() const
{
    return elementType == FEATURE_CATEGORY || elementType == FEATURE_TRASH || elementType == FEATURE_FAVORITE
        || elementType == FEATURE_FOLDER || elementType == FEATURE_SEARCH;
}

bool UBFeature::allowedCopy() const
{
    return isFolder()
            && elementType != FEATURE_CATEGORY
            && elementType != FEATURE_SEARCH;
}

bool UBFeature::isDeletable() const
{
    return elementType == FEATURE_ITEM
            || elementType == FEATURE_AUDIO
            || elementType == FEATURE_VIDEO
            || elementType == FEATURE_IMAGE
            || elementType == FEATURE_FLASH
            || elementType == FEATURE_FOLDER
            || elementType == FEATURE_BOOKMARK
            || elementType == FEATURE_LINK
    //Ilia. Just a hotfix. Permission mechanism for UBFeatures should be reworked
            || getVirtualPath().startsWith("/root/Interactivities/Web");// Issue 1627 - CFA - 20131024 : Interactivities, not Applications
}

bool UBFeature::inTrash() const
{
    return getFullPath().toLocalFile().startsWith(QUrl::fromLocalFile(UBSettings::userTrashDirPath()).toLocalFile() );
}

void UBFeature::setName(const QString &newName)
{
    QString name = newName;
    if(!this->isFolder()){
        name += '.' + UBFileSystemUtils::extension(this->mName);
    }

    this->mDisplayName = name;
    this->mName = name;
    this->mSortKey = name;

    QString fullPath = this->getFullPath().toString();
    int slashPos = fullPath.lastIndexOf("/");

    QString newUrl = fullPath.mid(0, slashPos+1) + name;
    this->mPath = QUrl(newUrl);
}

const UBFeatureBackgroundDisposition& UBFeature::backgroundDisposition() const
{
    return mDisposition;
}

void UBFeature::setBackgroundDisposition(UBFeatureBackgroundDisposition disposition)
{
    mDisposition = disposition;
}
