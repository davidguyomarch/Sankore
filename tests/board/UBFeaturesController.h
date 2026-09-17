/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: UBDocumentProxy only needs the UBFeature value type, not the heavy
// UBFeaturesController. Since #258 UBFeature lives in its own light header
// (board/UBFeature.h), so we include the REAL one here. Using a divergent stub
// caused an ODR violation (two incompatible UBFeature layouts in one binary →
// SIGSEGV in ~UBFeature). See moc-premoc / dev-workflow steering.
#ifndef UBFEATURESCONTROLLER_STUB_SHIM_H
#define UBFEATURESCONTROLLER_STUB_SHIM_H
#include "board/UBFeature.h"
#endif
