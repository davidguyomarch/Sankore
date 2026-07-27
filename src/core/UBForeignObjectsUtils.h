/*
 * Utility functions extracted from UBForeignObjectsHandler for testability.
 * These were originally static (file-local) functions.
 */

#ifndef UBFOREIGNOBJECTSUTILS_H
#define UBFOREIGNOBJECTSUTILS_H

#include <QString>
#include <QStringList>

namespace UBForeignObjectsUtils {

    /**
     * Extract the last UUID ({...}) from a file path.
     */
    QString strIdFrom(const QString &filePath);

    /**
     * Recursively remove a file or directory.
     */
    bool rm_r(const QString &rmPath);

    /**
     * Recursively copy a file or directory.
     */
    bool cp_rf(const QString &what, const QString &where);

    /**
     * List scene file names matching page???.svg pattern.
     */
    QStringList getSceneFileNames(const QString &folder);

}

#endif // UBFOREIGNOBJECTSUTILS_H
