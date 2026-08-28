/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Unit tests for the OCR/recognition module.
 * Tests stroke extraction, character segmentation, and pure functions.
 */

#ifndef TST_UBRECOGNITION_H
#define TST_UBRECOGNITION_H

#include <QObject>
#include <QTest>

class TestUBRecognition : public QObject
{
    Q_OBJECT

private slots:
    // UBStrokeExtractor
    void testExtractFromEmptySelection();
    void testExtractSkipsDuplicateStrokes();

    // Segmentation (Zinnia logic — testable as pure function)
    void testSegmentSingleCharacter();
    void testSegmentTwoCharactersWithGap();
    void testSegmentOverlappingStrokesGrouped();

    // UBSceneContext adjustedWidth (already tested but more cases)
    void testAdjustedWidthWithZoom();

    // UBPureFunctions
    void testComputeZoomRatio();
    void testTruncateText();
};

#endif // TST_UBRECOGNITION_H
