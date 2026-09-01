/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Wrapper to compile the real UBGraphicsStroke.cpp in the test context.
//
// UBGraphicsStroke.cpp does #include "UBGraphicsPolygonItem.h", which — because
// quoted includes resolve relative to the source directory first — would pull in
// the real (very heavy) src/domain/UBGraphicsPolygonItem.h and fail to link.
//
// By including the lightweight stub header FIRST, its include guard
// (UBGRAPHICSPOLYGONITEM_H) is already defined by the time the real source tries
// to include the polygon header, so the real header is skipped and the stub is
// used instead. The stub provides exactly the accessors UBGraphicsStroke needs.

#include "UBGraphicsPolygonItem.h" // tests/stubs stub, defines UBGRAPHICSPOLYGONITEM_H

#include "../../src/domain/UBGraphicsStroke.cpp"
