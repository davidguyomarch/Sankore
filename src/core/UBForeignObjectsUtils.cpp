/*
 * Utility functions extracted from UBForeignObjectsHandler for testability.
 */

#include "UBForeignObjectsUtils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

namespace UBForeignObjectsUtils {

QString strIdFrom(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return QString();
    }

    QRegularExpression rx("\\{.(?!.*\\{).*\\}");
    QRegularExpressionMatch match = rx.match(filePath);
    if (!match.hasMatch()) {
        return QString();
    }

    return match.captured();
}

bool rm_r(const QString &rmPath)
{
    QFileInfo fi(rmPath);
    if (!fi.exists()) {
        qDebug() << rmPath << "does not exist";
        return false;
    } else if (fi.isFile()) {
        if (!QFile::remove(rmPath)) {
            qDebug() << "can't remove file" << rmPath;
            return false;
        }
        return true;
    } else if (fi.isDir()) {
        QFileInfoList fList = QDir(rmPath).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (const QFileInfo& sub : fList) {
            rm_r(sub.absoluteFilePath());
        }
        if (!QDir().rmdir(rmPath)) {
            qDebug() << "can't remove dir" << rmPath;
            return false;
        }
        return true;
    }
    return false;
}

bool cp_rf(const QString &what, const QString &where)
{
    QFileInfo whatFi(what);
    QFileInfo whereFi(where);

    if (!whatFi.exists()) {
        qDebug() << what << "does not exist";
        return false;
    } else if (whatFi.isFile()) {
        QString whereDir = where.section("/", 0, -2, QString::SectionSkipEmpty | QString::SectionIncludeLeadingSep);
        QString newFilePath = where;
        if (!whereFi.exists()) {
            QDir().mkpath(whereDir);
        } else if (whereFi.isDir()) {
            newFilePath = whereDir + "/" + whatFi.fileName();
        }
        if (QFile::exists(newFilePath)) {
            QFile::remove(newFilePath);
        }
        if (!QFile::copy(what, newFilePath)) {
            qDebug() << "can't copy" << what << "to" << where;
            return false;
        }
        return true;
    } else if (whatFi.isDir()) {
        if (whereFi.isFile() && whereFi.fileName().toLower() == whatFi.fileName().toLower()) {
            qDebug() << "can't copy dir" << what << "to file" << where;
            return false;
        } else if (whereFi.isDir()) {
            rm_r(where);
        }

        QDir().mkpath(where);

        QFileInfoList fList = QDir(what).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (const QFileInfo& sub : fList) {
            if (!cp_rf(sub.absoluteFilePath(), where + "/" + sub.fileName()))
                return false;
        }
        return true;
    }
    return true;
}

QStringList getSceneFileNames(const QString &folder)
{
    QDir dir(folder, "page???.svg", QDir::Name, QDir::Files);
    return dir.entryList();
}

} // namespace UBForeignObjectsUtils
