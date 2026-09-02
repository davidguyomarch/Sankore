/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "document/UBExportSelection.h"

namespace UBExportSelection
{

UBExportAdaptor* adaptorForIndex(const QList<UBExportAdaptor*>& adaptors, int index)
{
    if (index < 0 || index >= adaptors.size())
        return nullptr;

    return adaptors.at(index);
}

}
