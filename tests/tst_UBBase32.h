/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBBASE32_H
#define TST_UBBASE32_H

#include <QObject>
#include <QtTest>

class TestUBBase32 : public QObject
{
    Q_OBJECT

private slots:
    void testDecodeEmpty();
    void testDecodeRFC4648Vectors();
    void testDecodeCaseInsensitive();
    void testDecodeInvalidChars();
    void testDecodeHelloWorld();
};

#endif // TST_UBBASE32_H
