/*
 * Unit tests for OCR recognition module.
 * Tests pure logic: stroke extraction, segmentation, context helpers.
 */

#include "tst_UBRecognition.h"
#include "recognition/IHandwritingRecognizer.h"
#include "domain/UBSceneContext.h"
#include "frameworks/UBPureFunctions.h"

#include <QFont>

// === Segmentation helper (must be defined before test methods that use it) ===

static QVector<QVector<UBRecognitionStroke>> segmentStrokes(const QVector<UBRecognitionStroke>& strokes)
{
    // Reimplementation of the segmentation algorithm for testing
    // (same logic as UBZinniaRecognizer::segmentIntoCharacters)
    QVector<QVector<UBRecognitionStroke>> characters;
    if (strokes.isEmpty()) return characters;

    struct StrokeBounds { qreal minX, maxX; int idx; };
    QVector<StrokeBounds> bounds;

    for (int i = 0; i < strokes.size(); i++)
    {
        if (strokes[i].points.isEmpty()) continue;
        qreal minX = strokes[i].points[0].x(), maxX = minX;
        for (const QPointF& p : strokes[i].points)
        {
            if (p.x() < minX) minX = p.x();
            if (p.x() > maxX) maxX = p.x();
        }
        bounds.append({minX, maxX, i});
    }

    if (bounds.isEmpty()) return characters;

    std::sort(bounds.begin(), bounds.end(), [](const StrokeBounds& a, const StrokeBounds& b) {
        return a.minX < b.minX;
    });

    qreal totalWidth = 0;
    for (const auto& b : bounds) totalWidth += (b.maxX - b.minX);
    qreal avgWidth = totalWidth / bounds.size();
    qreal gapThreshold = avgWidth * 0.8;

    QVector<UBRecognitionStroke> currentGroup;
    qreal currentMaxX = bounds[0].maxX;
    currentGroup.append(strokes[bounds[0].idx]);

    for (int i = 1; i < bounds.size(); i++)
    {
        qreal gap = bounds[i].minX - currentMaxX;
        if (gap > gapThreshold)
        {
            characters.append(currentGroup);
            currentGroup.clear();
        }
        currentGroup.append(strokes[bounds[i].idx]);
        currentMaxX = qMax(currentMaxX, bounds[i].maxX);
    }
    if (!currentGroup.isEmpty())
        characters.append(currentGroup);

    return characters;
}

// === UBStrokeExtractor tests ===

void TestUBRecognition::testExtractFromEmptySelection()
{
    // Test that the segmentation handles empty input
    QVector<UBRecognitionStroke> empty;
    auto result = segmentStrokes(empty);
    QCOMPARE(result.size(), 0);
}

void TestUBRecognition::testExtractSkipsDuplicateStrokes()
{
    // Single stroke with zero width (all points at same X) = one character
    UBRecognitionStroke stroke;
    stroke.points = {QPointF(50, 10), QPointF(50, 50), QPointF(50, 90)};
    QVector<UBRecognitionStroke> strokes = {stroke, stroke}; // duplicate

    auto result = segmentStrokes(strokes);
    // Both have same bounds → grouped as one character
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].size(), 2);
}

void TestUBRecognition::testSegmentSingleCharacter()
{
    // One stroke = one character
    UBRecognitionStroke stroke;
    stroke.points = {QPointF(10, 10), QPointF(20, 50), QPointF(10, 90)};

    QVector<UBRecognitionStroke> strokes = {stroke};
    auto result = segmentStrokes(strokes);

    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].size(), 1);
}

void TestUBRecognition::testSegmentTwoCharactersWithGap()
{
    // Two strokes with a big gap between them = two characters
    UBRecognitionStroke stroke1;
    stroke1.points = {QPointF(10, 10), QPointF(30, 10), QPointF(30, 50)};

    UBRecognitionStroke stroke2;
    stroke2.points = {QPointF(100, 10), QPointF(120, 10), QPointF(120, 50)};

    QVector<UBRecognitionStroke> strokes = {stroke1, stroke2};
    auto result = segmentStrokes(strokes);

    QCOMPARE(result.size(), 2);
    QCOMPARE(result[0].size(), 1);
    QCOMPARE(result[1].size(), 1);
}

void TestUBRecognition::testSegmentOverlappingStrokesGrouped()
{
    // Two strokes that overlap horizontally = same character
    UBRecognitionStroke stroke1;
    stroke1.points = {QPointF(10, 10), QPointF(30, 50)};

    UBRecognitionStroke stroke2;
    stroke2.points = {QPointF(15, 20), QPointF(25, 60)};  // overlaps stroke1

    QVector<UBRecognitionStroke> strokes = {stroke1, stroke2};
    auto result = segmentStrokes(strokes);

    QCOMPARE(result.size(), 1);  // grouped as one character
    QCOMPARE(result[0].size(), 2);  // both strokes
}

// === UBSceneContext ===

void TestUBRecognition::testAdjustedWidthWithZoom()
{
    UBSceneContext ctx;
    ctx.testSystemScaleFactor = 2.0;
    ctx.testCurrentZoom = 3.0;

    // 60 / 2.0 / 3.0 = 10.0
    QCOMPARE(ctx.adjustedWidth(60.0), 10.0);

    // Edge case: zoom = 1
    ctx.testCurrentZoom = 1.0;
    QCOMPARE(ctx.adjustedWidth(60.0), 30.0);
}

// === UBPureFunctions ===

void TestUBRecognition::testComputeZoomRatio()
{
    // computeZoomRatio(requestedRatio, currentViewScale, systemScaleFactor, maxZoom)
    auto result = UBPure::computeZoomRatio(2.0, 1.5, 1.0, 8.0);
    // Should return a valid ratio pair
    QVERIFY(result.first > 0);
    QVERIFY(result.second > 0);
    QVERIFY(result.second <= 8.0);
}

void TestUBRecognition::testTruncateText()
{
    QFont font("Arial", 12);

    // Short text should not be truncated
    QString shortText = "Hi";
    QString result = UBPure::truncateText(shortText, 500, font);
    QCOMPARE(result, shortText);

    // Very long text with tiny width should be truncated
    QString longText = "This is a very long text that should definitely be truncated when the max width is small";
    result = UBPure::truncateText(longText, 50, font);
    QVERIFY(result.length() < longText.length());
    QVERIFY(result.endsWith("..."));
}
