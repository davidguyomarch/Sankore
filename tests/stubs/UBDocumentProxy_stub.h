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

/**
 * @file UBDocumentProxy_stub.h
 * @brief Stub header for UBDocumentProxy that uses test stubs instead of
 *        UBApplication/UBPersistenceManager/UBSettings singletons.
 *
 * This re-declares UBDocumentProxy with the same interface but includes
 * from stubs/ instead of core/.
 */
#ifndef UBDOCUMENTPROXY_STUB_H
#define UBDOCUMENTPROXY_STUB_H

#include <QtGui>

#include "../src/frameworks/UBStringUtils.h"
#include "stubs/UBSettings_stub.h"
#include "stubs/UBFeature_stub.h"

class UBGraphicsScene;

class UBDocumentExternalFile
{
private:
    QString mTitle;
    QString mPath;
public:
    inline QString title() const {return mTitle;}
    inline void setTitle(QString title){mTitle = title;}

    inline QString path() const {return mPath;}
    inline void setPath(QString path){mPath = path;}
};

class UBDocumentProxy : public QObject
{
    Q_OBJECT

    public:
        UBDocumentProxy();
        UBDocumentProxy(const UBDocumentProxy &rValue);
        UBDocumentProxy(const QString& pPersistencePath);

        virtual ~UBDocumentProxy();

        QString persistencePath() const;
        void setPersistencePath(const QString& pPersistencePath);

        void setMetaData(const QString& pKey , const QVariant& pValue);
        QVariant metaData(const QString& pKey) const;
        QHash<QString, QVariant> metaDatas() const;

        QString name() const;
        QString groupName() const;
        QDateTime documentDate();
        QDateTime lastUpdate();

        void setHasDefaultImageBackground(const bool hasDefault);
        bool hasDefaultImageBackground() const;
        void setDefaultImageBackground(const UBFeature& item);
        UBFeature& defaultImageBackground();
        const UBFeature& defaultImageBackground() const;

        QSize defaultDocumentSize() const;
        void setDefaultDocumentSize(QSize pSize);
        void setDefaultDocumentSize(int pWidth, int pHeight);

        QUuid uuid() const;
        void setUuid(const QUuid& uuid);

        bool isModified() const;

        int pageCount();

        bool theSameDocument(UBDocumentProxy *proxy);

        void externalFilesAdd(UBDocumentExternalFile* file);
        const QList<UBDocumentExternalFile*> * externalFiles() const;
        void externalFilesClear();

        // Expose protected methods for testing
        void setPageCount(int pPageCount);
        int incPageCount();
        int decPageCount();

        // Dependency injection (mirrors real UBDocumentProxy)
        void setSettings(UBSettings* settings) { mSettings = settings; }
        UBSettings* settings() const { return mSettings; }

    signals:
        void defaultDocumentSizeChanged();

    private:
        void init();

        UBSettings* mSettings;
        QString mPersistencePath;
        QHash<QString, QVariant> mMetaDatas;
        bool mIsModified;
        int mPageCount;
        bool mHasDefaultImageBackground;
        UBFeature mDefaultImageBackground;
        QList<UBDocumentExternalFile*> mExternalFiles;
};

inline bool operator==(const UBDocumentProxy &proxy1, const UBDocumentProxy &proxy2)
{
    return proxy1.persistencePath() == proxy2.persistencePath();
}

inline size_t qHash(const UBDocumentProxy &key)
{
    return qHash(key.persistencePath());
}

#endif // UBDOCUMENTPROXY_STUB_H
