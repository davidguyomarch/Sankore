/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/**
 * @file qtsingleapplication.h
 * @brief Minimal stub replacing the ThirdParty QtSingleApplication.
 *
 * Provides a simple single-instance mechanism using QLockFile.
 * This replaces the full QtSingleApplication dependency.
 */
#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <QApplication>
#include <QLockFile>
#include <QDir>

class QtSingleApplication : public QApplication
{
    Q_OBJECT

public:
    QtSingleApplication(const QString &id, int &argc, char **argv)
        : QApplication(argc, argv)
        , mId(id)
        , mLockFile(QDir::tempPath() + "/" + id + ".lock")
    {
        mLockFile.setStaleLockTime(0);
    }

    virtual ~QtSingleApplication() {}

    bool isRunning()
    {
        if (!mLockFile.tryLock(100)) {
            return true; // Another instance holds the lock
        }
        return false;
    }

    QString id() const { return mId; }

    bool sendMessage(const QString &message, int timeout = 5000)
    {
        Q_UNUSED(message);
        Q_UNUSED(timeout);
        return false; // Stub: no IPC in this minimal implementation
    }

    void setActivationWindow(QWidget *aw, bool activateOnMessage = true)
    {
        Q_UNUSED(aw);
        Q_UNUSED(activateOnMessage);
    }

signals:
    void messageReceived(const QString &message);

private:
    QString mId;
    QLockFile mLockFile;
};

#endif // QTSINGLEAPPLICATION_H
