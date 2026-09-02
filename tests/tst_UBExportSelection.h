/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBEXPORTSELECTION_H
#define TST_UBEXPORTSELECTION_H

#include <QtTest>
#include <QObject>

class TestUBExportSelection : public QObject
{
    Q_OBJECT

private slots:
    // #262 — Documents view "Export" crashed because the chosen adaptor was
    // resolved from QObject::sender() while the menu action was connected via a
    // lambda (sender() is then null → null deref). The fix resolves the adaptor
    // from the captured index through UBExportSelection::adaptorForIndex().
    void testAdaptorForIndex_validIndex();
    void testAdaptorForIndex_negativeIndex();
    void testAdaptorForIndex_indexTooLarge();
    void testAdaptorForIndex_emptyList();
};

#endif // TST_UBEXPORTSELECTION_H
