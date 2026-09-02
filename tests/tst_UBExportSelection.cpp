/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBExportSelection.h"

#include "document/UBExportSelection.h"

// UBExportAdaptor is a heavy QObject; adaptorForIndex() only stores/returns the
// pointers, it never dereferences them. So we use opaque non-null sentinel
// pointers as stand-in adaptors — this keeps the test free of app dependencies.
static UBExportAdaptor* sentinel(quintptr id)
{
    return reinterpret_cast<UBExportAdaptor*>(id);
}

void TestUBExportSelection::testAdaptorForIndex_validIndex()
{
    QList<UBExportAdaptor*> adaptors;
    adaptors << sentinel(0x10) << sentinel(0x20) << sentinel(0x30);

    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, 0), sentinel(0x10));
    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, 1), sentinel(0x20));
    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, 2), sentinel(0x30));
}

void TestUBExportSelection::testAdaptorForIndex_negativeIndex()
{
    QList<UBExportAdaptor*> adaptors;
    adaptors << sentinel(0x10) << sentinel(0x20);

    // This is the #262 scenario: a bad/absent index (previously produced by a
    // null sender()) must return nullptr, never crash.
    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, -1), nullptr);
}

void TestUBExportSelection::testAdaptorForIndex_indexTooLarge()
{
    QList<UBExportAdaptor*> adaptors;
    adaptors << sentinel(0x10) << sentinel(0x20);

    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, 2), nullptr);
    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, 999), nullptr);
}

void TestUBExportSelection::testAdaptorForIndex_emptyList()
{
    QList<UBExportAdaptor*> adaptors;

    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, 0), nullptr);
    QCOMPARE(UBExportSelection::adaptorForIndex(adaptors, -1), nullptr);
}
