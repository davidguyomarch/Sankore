/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: minimal frameworks/UBPlatformUtils.h for test builds (issue #244).
// UBDisplayManager only calls the two static fade helpers; everything else in
// the real header (keyboard layout tables, tr functions) is irrelevant to the
// screen-count logic under test. Shadows the heavy real header via the tests/
// include-path priority in tests.pro.
#ifndef UBPLATFORMUTILS_H_
#define UBPLATFORMUTILS_H_

class UBPlatformUtils
{
public:
    static void fadeDisplayOut() {}
    static void fadeDisplayIn() {}
};

#endif /* UBPLATFORMUTILS_H_ */
