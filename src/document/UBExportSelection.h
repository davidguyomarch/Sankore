/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBEXPORTSELECTION_H_
#define UBEXPORTSELECTION_H_

#include <QList>

class UBExportAdaptor;

/**
 * Pure selection logic for the Documents-view export menu.
 *
 * Extracted so it can be unit-tested without instantiating the heavy
 * UBDocumentController (main window, board controller, persistence, QML).
 *
 * The export menu connects each QAction to a lambda that captures its index,
 * so the chosen adaptor must be resolved from that index — never from
 * QObject::sender() (which, when triggered through a lambda, is not the
 * QAction and yields a null pointer, see #262).
 */
namespace UBExportSelection
{
    /**
     * Returns the export adaptor at @p index, or nullptr when @p index is out
     * of range (negative or >= size). Never dereferences an invalid index.
     */
    UBExportAdaptor* adaptorForIndex(const QList<UBExportAdaptor*>& adaptors, int index);
}

#endif // UBEXPORTSELECTION_H_
