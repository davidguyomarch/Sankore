/*
 * Copyright (C) 2026 David Guyomarch
 */

/*
 * Stub for UBSettings - provides static string constants and a minimal
 * settings() accessor for testing UBDocumentProxy in isolation.
 */
#ifndef UBSETTINGS_STUB_H
#define UBSETTINGS_STUB_H

#include <QtCore>
#include <QColor>
#include <QVariant>
#include <QSize>

class UBSettings;

class UBSetting : public QObject
{
    Q_OBJECT

public:
    UBSetting(UBSettings* parent = nullptr) : QObject(), mOwner(nullptr), mDefaultValue(QVariant()) { Q_UNUSED(parent); }
    UBSetting(UBSettings* owner, const QString& pDomain, const QString& pKey,
              const QVariant& pDefaultValue)
        : QObject(), mOwner(owner), mDomain(pDomain), mKey(pKey), mDefaultValue(pDefaultValue)
    {
        mPath = pDomain + "/" + pKey;
    }

    virtual ~UBSetting() {}

    virtual void set(const QVariant& pValue) { mValue = pValue; emit changed(pValue); }
    virtual QVariant get() { return mValue.isValid() ? mValue : mDefaultValue; }
    virtual QVariant reset() { mValue = mDefaultValue; return mValue; }

    virtual QString domain() const { return mDomain; }
    virtual QString key() const { return mKey; }
    virtual QString path() const { return mPath; }

public slots:
    void setBool(bool pValue) { set(QVariant(pValue)); }
    void setString(const QString& pValue) { set(QVariant(pValue)); }
    void setInt(int pValue) { set(QVariant(pValue)); }

signals:
    void changed(QVariant newValue);

protected:
    UBSettings* mOwner;
    QString mDomain;
    QString mKey;
    QString mPath;
    QVariant mDefaultValue;
    QVariant mValue;
};

class UBColorListSetting : public UBSetting
{
    Q_OBJECT
public:
    UBColorListSetting(UBSettings* parent = 0) : UBSetting(parent) {}
    QList<QColor> colors() const { return mColors; }
    void setColor(int pIndex, const QColor& color) {
        while (mColors.size() <= pIndex) mColors.append(Qt::black);
        mColors[pIndex] = color;
    }
private:
    QList<QColor> mColors;
};

class UBSettings : public QObject
{
    Q_OBJECT

public:
    static UBSettings* settings();
    static void destroy();

    UBSettings(QObject *parent = nullptr);
    virtual ~UBSettings();

    // Static string constants used by UBDocumentProxy



    static QString uniboardDocumentNamespaceUri;

    static QString trashedDocumentGroupNamePrefix;
    static QString currentFileVersion;

    static int pointerDiameter;
    static int boardMargin;

    // UBSetting members used by UBDocumentProxy
    UBSetting* pageSize;

    // Directory path methods
    static QString userDataDirectory();
    static QString userDocumentDirectory();
    static QString userImageDirectory();

private:
    static UBSettings* sInstance;
};

#endif // UBSETTINGS_STUB_H
