/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBItemCapabilities.h"

#include "domain/UBItemCapabilities.h"

using UBItemMenu::Capabilities;
using UBItemMenu::Entry;
using UBItemMenu::baseMenuEntries;

void TestUBItemCapabilities::testLockAndVisibleAlwaysPresent()
{
    // Everything off: Locked + Visible are still there (unconditional).
    Capabilities caps;
    caps.goToSource = caps.linkAction = caps.returnToCreation = false;
    caps.horizontalMirror = caps.verticalMirror = false;

    const QList<Entry> e = baseMenuEntries(caps);
    QCOMPARE(e.size(), 2);
    QCOMPARE(e.at(0), Entry::Locked);
    QCOMPARE(e.at(1), Entry::VisibleOnDisplay);
}

void TestUBItemCapabilities::testOrderIsStable()
{
    // All flags on: the order must match decorateMenu()'s build order exactly.
    Capabilities caps;
    caps.goToSource = true;
    caps.linkAction = true;
    caps.returnToCreation = true;
    caps.horizontalMirror = true;
    caps.verticalMirror = true;

    const QList<Entry> e = baseMenuEntries(caps);
    const QList<Entry> expected = {
        Entry::Locked,
        Entry::VisibleOnDisplay,
        Entry::GoToContentSource,
        Entry::LinkAction,
        Entry::ReturnToCreation,
        Entry::FlipHorizontal,
        Entry::FlipVertical
    };
    QCOMPARE(e, expected);
}

void TestUBItemCapabilities::testGatedEntriesHiddenByDefault()
{
    // A default-constructed Capabilities exposes no optional entry.
    const QList<Entry> e = baseMenuEntries(Capabilities{});
    QVERIFY(!e.contains(Entry::GoToContentSource));
    QVERIFY(!e.contains(Entry::LinkAction));
    QVERIFY(!e.contains(Entry::ReturnToCreation));
    QVERIFY(!e.contains(Entry::FlipHorizontal));
    QVERIFY(!e.contains(Entry::FlipVertical));
}

void TestUBItemCapabilities::testShapeProfile()
{
    // Shape (UBAbstractGraphicsItem): canTrigAnAction + H/V mirror, no go-to-source.
    Capabilities caps;
    caps.linkAction = true;
    caps.horizontalMirror = true;
    caps.verticalMirror = true;

    const QList<Entry> e = baseMenuEntries(caps);
    const QList<Entry> expected = {
        Entry::Locked,
        Entry::VisibleOnDisplay,
        Entry::LinkAction,
        Entry::FlipHorizontal,
        Entry::FlipVertical
    };
    QCOMPARE(e, expected);
}

void TestUBItemCapabilities::testPdfProfile()
{
    // PDF: bare — only Locked + Visible (no go-to-source, no link, no flip).
    const QList<Entry> e = baseMenuEntries(Capabilities{});
    const QList<Entry> expected = { Entry::Locked, Entry::VisibleOnDisplay };
    QCOMPARE(e, expected);
}

void TestUBItemCapabilities::testImageProfile()
{
    // Image: go-to-source + link action + flippable (showGoContent +
    // canTrigAnAction + setFlippable(true)). #456: being flippable now yields
    // both Flip entries.
    Capabilities caps;
    caps.goToSource = true;
    caps.linkAction = true;
    caps.flippable = true;

    const QList<Entry> e = baseMenuEntries(caps);
    const QList<Entry> expected = {
        Entry::Locked,
        Entry::VisibleOnDisplay,
        Entry::GoToContentSource,
        Entry::LinkAction,
        Entry::FlipHorizontal,
        Entry::FlipVertical
    };
    QCOMPARE(e, expected);
}

void TestUBItemCapabilities::testFlippableYieldsBothFlips()
{
    // #456: an item that is only flippable (strokes/SVG — no mirror flags) still
    // gets both Flip entries.
    Capabilities caps;
    caps.flippable = true;

    const QList<Entry> e = baseMenuEntries(caps);
    QVERIFY(e.contains(Entry::FlipHorizontal));
    QVERIFY(e.contains(Entry::FlipVertical));
}

void TestUBItemCapabilities::testShapeMirrorStillFlips()
{
    // #456 regression guard: shapes set the mirror flags (not flippable) and
    // must keep both Flip entries.
    Capabilities caps;
    caps.horizontalMirror = true;
    caps.verticalMirror = true;
    caps.flippable = false;

    const QList<Entry> e = baseMenuEntries(caps);
    QVERIFY(e.contains(Entry::FlipHorizontal));
    QVERIFY(e.contains(Entry::FlipVertical));
}

void TestUBItemCapabilities::testWidgetProfile()
{
    // Widget: go-to-source only in the BASE menu (Frozen / Transform-as-Tool are
    // added by the subclass override, not modelled here). canTrigAnAction stays
    // false for widgets, so no LinkAction in the base.
    Capabilities caps;
    caps.goToSource = true;

    const QList<Entry> e = baseMenuEntries(caps);
    const QList<Entry> expected = {
        Entry::Locked,
        Entry::VisibleOnDisplay,
        Entry::GoToContentSource
    };
    QCOMPARE(e, expected);
}
