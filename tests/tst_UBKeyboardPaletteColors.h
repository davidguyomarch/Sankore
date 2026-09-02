/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef TST_UBKEYBOARDPALETTECOLORS_H
#define TST_UBKEYBOARDPALETTECOLORS_H

#include <QObject>
#include <QtTest>

/**
 * @brief Regression tests for the virtual keyboard key contrast (issue #263).
 *
 * The key faces are fixed light-grey images. Before the fix, the key glyphs
 * were drawn with the widget's default pen, which the app-wide dark stylesheet
 * turned white — making the labels unreadable (white glyph on light-grey key).
 *
 * These tests pin the color contract (dark label, sufficient WCAG contrast) and
 * reproduce the buggy rendering path to prove the failure/fix.
 */
class TestUBKeyboardPaletteColors : public QObject
{
    Q_OBJECT

private slots:
    // Contract: the label color must contrast strongly with both key faces.
    void testKeyLabelContrastWithFaces();

    // Rendering: a glyph drawn with the OLD (white) pen is unreadable on the
    // light key face, while the fixed dark pen renders a readable glyph.
    void testRenderedGlyphReadability();

private:
    // Peak WCAG contrast ratio of any rendered glyph pixel against the key face.
    // Font-metric independent, so it is portable across platforms.
    double glyphVisibility(const QColor& penColor);
};

#endif // TST_UBKEYBOARDPALETTECOLORS_H
