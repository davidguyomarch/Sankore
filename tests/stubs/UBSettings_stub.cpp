/*
 * Stub implementation for UBSettings - provides minimal singleton
 * and non-string-key members needed by test code.
 *
 * String constants (documentName, sessionTitle, etc.) are now in
 * UBSettingsData.h (header-only) — no stub needed for those.
 */
#include "UBSettings_stub.h"
#include <QDir>

UBSettings* UBSettings::sInstance = 0;

// Non-string statics still needed by some test code paths
QString UBSettings::uniboardDocumentNamespaceUri = "http://uniboard.mnemis.com/document";
QString UBSettings::trashedDocumentGroupNamePrefix = "Trash:";
QString UBSettings::currentFileVersion = "4.7.0";
int UBSettings::pointerDiameter = 40;
int UBSettings::boardMargin = 10;

UBSettings* UBSettings::settings()
{
    if (!sInstance)
        sInstance = new UBSettings(0);
    return sInstance;
}

void UBSettings::destroy()
{
    if (sInstance) {
        delete sInstance;
        sInstance = 0;
    }
}

UBSettings::UBSettings(QObject *parent)
    : QObject(parent)
{
    pageSize = new UBSetting(this, "Board", "PageSize", QVariant(QSize(1380, 1080)));
}

UBSettings::~UBSettings()
{
    delete pageSize;
}

QString UBSettings::userDataDirectory()
{
    return QDir::tempPath() + "/OpenSankoreTests/data";
}

QString UBSettings::userDocumentDirectory()
{
    return QDir::tempPath() + "/OpenSankoreTests/documents";
}

QString UBSettings::userImageDirectory()
{
    return QDir::tempPath() + "/OpenSankoreTests/images";
}
