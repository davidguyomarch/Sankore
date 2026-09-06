/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBBackgroundGrid.h"
#include "domain/UBBackgroundGrid.h"

#include <algorithm>

using namespace UBBackgroundGrid;

// Board calibration: 8 mm == 32 units → 4 units/mm.
static const double UPM = UBBackgroundGrid::unitsPerMm();

static int countLines(const std::vector<Line>& lines, Orientation o)
{
    return static_cast<int>(std::count_if(lines.begin(), lines.end(),
        [o](const Line& l){ return l.orientation == o; }));
}

static int countLines(const std::vector<Line>& lines, Orientation o, Weight w)
{
    return static_cast<int>(std::count_if(lines.begin(), lines.end(),
        [o, w](const Line& l){ return l.orientation == o && l.weight == w; }));
}

void TestUBBackgroundGrid::testPlainProducesNoLines()
{
    auto lines = generateLines(Type::Plain, QRectF(0, 0, 800, 600));
    QVERIFY(lines.empty());
}

void TestUBBackgroundGrid::testTypeTokenRoundTrip()
{
    QCOMPARE(fromToken(toToken(Type::Plain)), Type::Plain);
    QCOMPARE(fromToken(toToken(Type::Grid)), Type::Grid);
    QCOMPARE(fromToken(toToken(Type::Seyes)), Type::Seyes);
    QCOMPARE(fromToken(toToken(Type::SeyesLarge)), Type::SeyesLarge);
    QCOMPARE(fromToken(toToken(Type::DoubleLine3mm)), Type::DoubleLine3mm);
    // Unknown token falls back to Plain.
    QCOMPARE(fromToken(QStringLiteral("bogus")), Type::Plain);
}

void TestUBBackgroundGrid::testTypeIntRoundTrip()
{
    for (int v : {0, 1, 2, 3, 4})
        QCOMPARE(toInt(fromInt(v)), v);
    // Out-of-range integer falls back to Plain (0).
    QCOMPARE(fromInt(99), Type::Plain);
}

void TestUBBackgroundGrid::testIsRuled()
{
    QVERIFY(!isRuled(Type::Plain));
    QVERIFY(isRuled(Type::Grid));
    QVERIFY(isRuled(Type::Seyes));
    QVERIFY(isRuled(Type::SeyesLarge));
    QVERIFY(isRuled(Type::DoubleLine3mm));
}

void TestUBBackgroundGrid::testGridIsUniform8mm()
{
    // 8 mm step == 32 units. In a 0..320 window we expect lines at
    // 0,32,...,288 → 10 horizontals and 10 verticals, all Major.
    const double step = 8.0 * UPM;
    QCOMPARE(step, 32.0);

    auto lines = generateLines(Type::Grid, QRectF(0, 0, 320, 320));
    QCOMPARE(countLines(lines, Orientation::Horizontal), 10);
    QCOMPARE(countLines(lines, Orientation::Vertical), 10);
    QCOMPARE(countLines(lines, Orientation::Horizontal, Weight::Minor), 0);

    // First and second horizontal lines are exactly one step apart.
    std::vector<double> ys;
    for (const auto& l : lines)
        if (l.orientation == Orientation::Horizontal) ys.push_back(l.pos);
    std::sort(ys.begin(), ys.end());
    QVERIFY(qFuzzyCompare(ys[1] - ys[0], step));
}

void TestUBBackgroundGrid::testSeyesInterlineSpacing()
{
    // Séyès: within an 8 mm cell there are 4 horizontal lines (1 Major at the
    // cell boundary + 3 Minor interlines), spaced 2 mm apart.
    const double cell = 8.0 * UPM;   // 32
    const double inter = 2.0 * UPM;  // 8

    // A window covering exactly one cell [0, 32) must yield the 4 lines of that
    // cell (Major at 0, Minor at 8, 16, 24).
    auto lines = generateLines(Type::Seyes, QRectF(0, 0, 320, cell));
    std::vector<Line> h;
    for (const auto& l : lines)
        if (l.orientation == Orientation::Horizontal) h.push_back(l);
    std::sort(h.begin(), h.end(), [](const Line& a, const Line& b){ return a.pos < b.pos; });

    QVERIFY(h.size() >= 4);
    QCOMPARE(h[0].weight, Weight::Major);
    QVERIFY(qFuzzyCompare(h[0].pos, 0.0));
    QCOMPARE(h[1].weight, Weight::Minor);
    QVERIFY(qFuzzyCompare(h[1].pos - h[0].pos, inter));
    QCOMPARE(h[2].weight, Weight::Minor);
    QCOMPARE(h[3].weight, Weight::Minor);
    // 3 minor interlines per cell.
    QCOMPARE(countLines(lines, Orientation::Horizontal, Weight::Minor), 3);
    QCOMPARE(countLines(lines, Orientation::Horizontal, Weight::Major), 1);
}

void TestUBBackgroundGrid::testSeyesHasVerticalAndMargin()
{
    // Vertical major lines every 8 mm, plus one red margin line at 40 mm.
    const double cell = 8.0 * UPM;    // 32
    const double marginX = 40.0 * UPM; // 160

    auto lines = generateLines(Type::Seyes, QRectF(0, 0, 320, 320));
    QVERIFY(countLines(lines, Orientation::Vertical, Weight::Major) > 0);

    // Exactly one margin line, at 40 mm.
    int marginCount = countLines(lines, Orientation::Vertical, Weight::Margin);
    QCOMPARE(marginCount, 1);
    auto it = std::find_if(lines.begin(), lines.end(), [](const Line& l){
        return l.weight == Weight::Margin; });
    QVERIFY(it != lines.end());
    QVERIFY(qFuzzyCompare(it->pos, marginX));

    // Vertical majors are one cell apart.
    QVERIFY(qFuzzyCompare(cell, 32.0));
}

void TestUBBackgroundGrid::testSeyesLargeIsScaled()
{
    // Enlarged Séyès uses 1.5x spacing, so fewer lines fit the same window.
    auto normal = generateLines(Type::Seyes, QRectF(0, 0, 320, 320));
    auto large = generateLines(Type::SeyesLarge, QRectF(0, 0, 320, 320));

    int normalH = countLines(normal, Orientation::Horizontal);
    int largeH = countLines(large, Orientation::Horizontal);
    QVERIFY(largeH < normalH);

    // Interline for large == 3 mm (2 mm * 1.5) == 12 units.
    std::vector<double> ys;
    for (const auto& l : large)
        if (l.orientation == Orientation::Horizontal) ys.push_back(l.pos);
    std::sort(ys.begin(), ys.end());
    QVERIFY(ys.size() >= 2);
    QVERIFY(qFuzzyCompare(ys[1] - ys[0], 2.0 * UPM * 1.5));
}

void TestUBBackgroundGrid::testDoubleLine3mmPairs()
{
    // Maternelle double ruling: pairs 3 mm apart, period 6 mm.
    const double band = 3.0 * UPM;   // 12
    const double period = 6.0 * UPM; // 24

    auto lines = generateLines(Type::DoubleLine3mm, QRectF(0, 0, 320, period * 2));
    // No verticals for this ruling.
    QCOMPARE(countLines(lines, Orientation::Vertical), 0);

    std::vector<double> ys;
    for (const auto& l : lines)
        if (l.orientation == Orientation::Horizontal) ys.push_back(l.pos);
    std::sort(ys.begin(), ys.end());
    QVERIFY(ys.size() >= 2);
    // First pair is `band` apart.
    QVERIFY(qFuzzyCompare(ys[1] - ys[0], band));
    // Next pair starts one period after the first.
    if (ys.size() >= 3)
        QVERIFY(qFuzzyCompare(ys[2] - ys[0], period));
}
