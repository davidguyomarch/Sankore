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

#ifndef UBFEATURE_H
#define UBFEATURE_H

// UBFeature — a single entry of the media Library (image, sound, folder,
// category, favourite, ...). Extracted from UBFeaturesController.h (#258) so
// the Library value object is a standalone, lightweight translation unit,
// decoupled from the heavy UBFeaturesController (scan thread, QListView, board
// controller). This lets the QML V2 Library controller/model (and their unit
// tests) depend only on UBFeature.

#include <QMetaType>
#include <QString>
#include <QImage>
#include <QUrl>
#include <QMap>

#include "core/UBTypes.h"

enum UBFeatureElementType
{
    FEATURE_CATEGORY,
    FEATURE_VIRTUALFOLDER,
    FEATURE_FOLDER,
    FEATURE_INTERACTIVE,
    FEATURE_INTERNAL,
    FEATURE_ITEM,
    FEATURE_AUDIO,
    FEATURE_VIDEO,
    FEATURE_IMAGE,
    FEATURE_FLASH,
    FEATURE_TRASH,
    FEATURE_FAVORITE,
    FEATURE_SEARCH,
    FEATURE_BOOKMARK,
    FEATURE_LINK,
    FEATURE_INVALID,
    FEATURE_RTE
};

class UBFeature
{
public:
    enum Permission {
        NO_P      = 0x0  //0000
        ,DELETE_P = 0x1  //0001
        , WRITE_P = 0x2  //0010
        , ALL_P   = 0xF  //1111
    };
    Q_DECLARE_FLAGS(Permissions, Permission)

    UBFeature() {;}
    UBFeature(const QString &url
              , const QImage &icon
              , const QString &name
              , const QUrl &realPath
              , UBFeatureElementType type = FEATURE_CATEGORY
              , Permissions pOwnPermissions = ALL_P
              , QString pSortKey = QString());
    // Defined inline (= default) on purpose: UBFeature is a value type held by
    // value in several classes (e.g. UBDocumentProxy::mDefaultImageBackground).
    // With a virtual destructor defined out-of-line in UBFeature.cpp, MSVC emits
    // it in both UBFeature.obj and UBDocumentProxy.obj, causing LNK2005 when both
    // TUs are linked together (the unit-test target does exactly that). An inline
    // defaulted dtor is a COMDAT/weak symbol that the linker de-duplicates. (#258)
    virtual ~UBFeature() = default;
    QString getName() const { return mName; }

    QString getDisplayName() const {return mDisplayName;}
    QImage getThumbnail() const {return mThumbnail;}
    QString getVirtualPath() const { return virtualDir; }
    QUrl getFullPath() const { return mPath; }
    QString getFullVirtualPath() const { return  virtualDir + "/" + mName; }
    QString getUrl() const;
    QString getSortKey() const {return mSortKey;}
    void setFullPath(const QUrl &newPath) {mPath = newPath;}
    void setFullVirtualPath(const QString &newVirtualPath) {virtualDir = newVirtualPath;}

    /**
      * Change the current name of the feature.
      * @arg newName The new name of the feature.
      */
    void setName(const QString &newName);

    UBFeatureElementType getType() const { return elementType; }
    UBFeature &markedWithSortKey(const QString &str);

    const UBFeatureBackgroundDisposition& backgroundDisposition() const;
    void setBackgroundDisposition(UBFeatureBackgroundDisposition disposition);

    bool isFolder() const;
    bool allowedCopy() const;
    bool isDeletable() const;
    bool inTrash() const;
    Permissions getPermissions() const {return mOwnPermissions;}
    bool testPermissions(Permissions pPermissions) const {return mOwnPermissions & pPermissions;}
    void setPermissions(Permissions pPermissions) {mOwnPermissions = pPermissions;}
    void addPermissions(Permissions pPermissions) {mOwnPermissions |= pPermissions;}
    void unsetPermissions(Permissions pPermissions) {mOwnPermissions &= ~pPermissions;}
    bool operator ==( const UBFeature &f )const;
    bool operator !=( const UBFeature &f )const;
    const QMap<QString,QString> & getMetadata() const { return metadata; }
    void setMetadata( const QMap<QString,QString> &data ) { metadata = data; }

private:
    QString getNameFromVirtualPath(const QString &pVirtPath);
    QString getVirtualDirFromVirtualPath(const QString &pVirtPath);

private:
    QString virtualDir;
    QString virtualPath;
    QImage mThumbnail;
    QString mName;
    QString mDisplayName;
    QUrl mPath;
    UBFeatureElementType elementType;
    QMap<QString,QString> metadata;
    Permissions mOwnPermissions;
    QString mSortKey;
    UBFeatureBackgroundDisposition mDisposition;
};
Q_DECLARE_METATYPE( UBFeature )
Q_DECLARE_OPERATORS_FOR_FLAGS(UBFeature::Permissions)

#endif // UBFEATURE_H
