/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 */

#ifndef UBSETTINGSDATA_H
#define UBSETTINGSDATA_H

#include <QString>
#include <QStringList>
#include <QColor>

/**
 * @brief Static data constants from UBSettings that have NO dependency on
 * QSettings, QObject, or any singleton.
 *
 * This header can be included in test binaries without pulling in
 * UBApplication, UBPersistenceManager, or the full UBSettings class.
 *
 * It provides:
 *   - Metadata key names (document properties)
 *   - Default sizes and dimensions
 *   - File extension lists
 *   - Static directory path functions that only depend on QStandardPaths
 */
namespace UBSettingsData
{
    // --- Document metadata keys ---
    inline const QString documentGroupName = "Subject";
    inline const QString documentName = "Lesson";
    inline const QString documentSize = "Size";
    inline const QString documentIdentifer = "ID";
    inline const QString documentVersion = "Version";
    inline const QString documentUpdatedAt = "UpdatedAt";
    inline const QString documentDefaultBackgroundImage = "defaultBackgroundImage";
    inline const QString documentDefaultBackgroundImageDisposition = "defaultBackgroundImageDisposition";

    // --- Session metadata keys ---
    inline const QString sessionTitle = "sessionTitle";
    inline const QString sessionAuthors = "sessionAuthors";
    inline const QString sessionObjectives = "sessionObjectives";
    inline const QString sessionKeywords = "sessionKeywords";
    inline const QString sessionGradeLevel = "sessionGradeLevel";
    inline const QString sessionSubjects = "sessionSubjects";
    inline const QString sessionType = "sessionType";
    inline const QString sessionLicence = "sessionLicence";

    // --- Document date ---
    inline const QString documentDate = "date";
    inline const QString documentTagVersion = "versionCreation";

    // --- Default dimensions ---
    inline constexpr int maxThumbnailWidth = 400;
    inline constexpr int defaultThumbnailWidth = 150;
    inline constexpr int defaultLibraryIconSize = 80;
    inline constexpr int defaultGipWidth = 150;
    inline constexpr int defaultSoundWidth = 50;
    inline constexpr int defaultImageWidth = 150;
    inline constexpr int defaultShapeWidth = 50;
    inline constexpr int defaultWidgetIconWidth = 110;
    inline constexpr int defaultVideoWidth = 80;
    inline constexpr int thumbnailSpacing = 20;
    inline constexpr int longClickInterval = 1200;
    inline constexpr qreal minScreenRatio = 1.33;
    inline constexpr int objectInControlViewMargin = 100;

    // --- File extensions ---
    inline QStringList bitmapFileExtensions()
    {
        return {"jpg", "jpeg", "png", "gif", "bmp", "tif", "tiff"};
    }

    inline QStringList vectorFileExtensions()
    {
        return {"svg", "svgz"};
    }

    inline QStringList imageFileExtensions()
    {
        return bitmapFileExtensions() + vectorFileExtensions();
    }

    inline QStringList widgetFileExtensions()
    {
        return {"wgt", "wdgt"};
    }

    inline QStringList interactiveContentFileExtensions()
    {
        return widgetFileExtensions() + QStringList{"swf"};
    }
}

#endif // UBSETTINGSDATA_H
