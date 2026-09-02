/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Wrapper to compile the real src/core/UBDisplayManager.cpp in the test context
// (issue #244 — multi-screen must default to single-screen at startup).
//
// All of UBDisplayManager.cpp's heavy application includes are shadowed by
// lightweight shims placed earlier on the tests/ include path (see tests.pro):
//   core/UBSettings.h            -> tests/core/UBSettings.h -> stubs/UBSettings_stub.h
//   core/UBApplication.h         -> tests/core/UBApplication.h (empty shim)
//   core/UBApplicationController.h-> tests/core/UBApplicationController.h (empty)
//   board/UBBoardView.h          -> tests/board/UBBoardView.h
//   gui/UBBlackoutWidget.h       -> tests/gui/UBBlackoutWidget.h (moc pre-generated)
//   frameworks/UBPlatformUtils.h -> tests/frameworks/UBPlatformUtils.h
//   ui_blackoutWidget.h          -> tests/ui_blackoutWidget.h
//
// UBDisplayManager itself is a QObject; its moc is pre-generated into
// tests/premoc/moc_UBDisplayManager.cpp (see scripts/docker-build.sh).

#include "../../src/core/UBDisplayManager.cpp"
