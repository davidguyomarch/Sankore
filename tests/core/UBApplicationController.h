/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: stub out core/UBApplicationController.h for test builds (issue #244).
// UBDisplayManager.cpp includes this but uses no symbols from it.
#ifndef UBAPPLICATIONCONTROLLER_STUB_SHIM_H
#define UBAPPLICATIONCONTROLLER_STUB_SHIM_H
// Empty — no symbols needed from UBApplicationController in UBDisplayManager.cpp
#endif
