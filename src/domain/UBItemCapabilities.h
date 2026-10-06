/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBITEMCAPABILITIES_H
#define UBITEMCAPABILITIES_H

#include <QList>

/**
 * ADR-0010 (#454): single declarative description of what a board item exposes
 * in its selection delegate — the "…" context menu and the frame buttons.
 *
 * Historically each item type set a scattered collection of boolean flags in its
 * constructor (setRotatable / setHorizontalMirror / setCanTrigAnAction / …) and
 * `UBGraphicsItemDelegate::decorateMenu()` / `init()` read those flags directly.
 * This header turns that into one explicit value type plus a PURE function that
 * maps it to the ordered list of base menu entries. The pure logic is unit-
 * tested (tst_UBItemCapabilities); the actual QMenu/QAction rendering stays in
 * the delegate (not headless-testable).
 *
 * Scope of this header: the BASE entries common to all types and governed by the
 * old flags. Type-specific extras (text "Editable", widget "Frozen" /
 * "Transform as Tool", …) are still added by the delegate subclass overrides,
 * which call the base first — so they are intentionally NOT modelled here.
 *
 * Dependency-free (Qt core QList only) so the production delegate and the tests
 * share the exact same logic.
 */
namespace UBItemMenu
{
    /**
     * The base menu entries, in the exact order `decorateMenu()` adds them.
     * Keep this enum in sync with the delegate's build order.
     */
    enum class Entry
    {
        Locked,            // always (checkable)
        VisibleOnDisplay,  // always (checkable) — "Visible on Extended Screen"
        FillColour,        // if fillColour — "Fill colour…" (shapes, #458)
        GoToContentSource, // if goToSource
        LinkAction,        // if linkAction — "Link an action…"
        ReturnToCreation,  // if returnToCreation
        FlipHorizontal,    // if horizontalMirror
        FlipVertical       // if verticalMirror
    };

    /**
     * Capability declaration for an item type. Fields mirror the ADR-0010 matrix
     * cells that are driven by the legacy flags. `duplicate` governs the frame
     * Duplicate button (not a menu entry); the other fields govern menu entries.
     */
    struct Capabilities
    {
        // Frame buttons (Delete / Menu / Z-order are unconditional and are not
        // modelled here; only Duplicate is optional today).
        bool duplicate = true;      // mCanDuplicate (default true in the base ctor)

        // Base menu entries.
        bool fillColour = false;       // #458: shapes with a fill property
        bool goToSource = false;       // mShowGoContentButton
        bool linkAction = false;       // mCanTrigAnAction
        bool returnToCreation = false; // mCanReturnInCreationMode
        bool horizontalMirror = false; // mHorizontalMirror
        bool verticalMirror = false;   // mVerticalMirror

        // #456/ADR-0010: an item that is flippable (resize-handle flip) also
        // gets the Flip menu entries, so flip is consistent across types.
        // Shapes historically set the mirror flags (not flippable); image / SVG /
        // strokes set flippable (not the mirror flags) — both must show Flip.
        bool flippable = false;        // mFlippable
    };

    /**
     * Pure mapping: capability struct -> ordered list of base menu entries.
     * Locked and VisibleOnDisplay are always present; the rest are gated by the
     * matching capability flag. This is the single source of truth for the base
     * menu composition and is unit-tested.
     */
    inline QList<Entry> baseMenuEntries(const Capabilities& caps)
    {
        QList<Entry> entries;
        entries << Entry::Locked;
        entries << Entry::VisibleOnDisplay;
        if (caps.fillColour)
            entries << Entry::FillColour;
        if (caps.goToSource)
            entries << Entry::GoToContentSource;
        if (caps.linkAction)
            entries << Entry::LinkAction;
        if (caps.returnToCreation)
            entries << Entry::ReturnToCreation;
        // #456: Flip entries follow flippability OR an explicit mirror flag, so
        // flippable items (image/SVG/strokes) get Flip too, not just shapes.
        if (caps.horizontalMirror || caps.flippable)
            entries << Entry::FlipHorizontal;
        if (caps.verticalMirror || caps.flippable)
            entries << Entry::FlipVertical;
        return entries;
    }

    // ---------------------------------------------------------------------
    // #461/ADR-0010 (option ii): per-type capability PROFILES.
    //
    // Each board item type declares its menu capabilities in ONE place, via one
    // of these pure factory functions, instead of a scattered sequence of
    // setXxx() calls in its constructor. The item passes the result to
    // UBGraphicsItemDelegate::applyMenuCapabilities() right after init().
    //
    // These functions are dependency-free and unit-tested (tst_UBItemCapabilities):
    // they freeze the ADR-0010 matrix per type. They describe the STATIC starting
    // profile only — a few flags are mutated at runtime by the delegate itself
    // (e.g. a group recomputes flippable/rotatable as children change; attaching
    // an action forces linkAction) and stay the delegate's responsibility.
    //
    // `fillColour` is intentionally NOT set here: it is decided dynamically by
    // the delegate from the item's hasFillingProperty() (a filled shape vs an
    // outline-only one), not statically per type.
    // ---------------------------------------------------------------------

    // Shapes (rect/square/ellipse/circle/regular/freehand): rotatable + link
    // action + explicit H/V mirror (shapes flip via the mirror flags, not the
    // flippable resize handle).
    inline Capabilities forShape()
    {
        Capabilities c;
        c.linkAction = true;
        c.horizontalMirror = true;
        c.verticalMirror = true;
        return c;
    }

    // Free polygon: a shape that can also return to creation mode.
    inline Capabilities forPolygon()
    {
        Capabilities c = forShape();
        c.returnToCreation = true;
        return c;
    }

    // Line: a shape, explicitly WITHOUT return-to-creation.
    inline Capabilities forLine()
    {
        Capabilities c = forShape();
        c.returnToCreation = false;
        return c;
    }

    // Pen/marker strokes (UBSmoothStrokeItem, UBGraphicsStrokesGroup): flippable
    // + link action, no mirror flags.
    inline Capabilities forStroke()
    {
        Capabilities c;
        c.flippable = true;
        c.linkAction = true;
        return c;
    }

    // Text: link action, not flippable, no go-to-source.
    inline Capabilities forText()
    {
        Capabilities c;
        c.linkAction = true;
        return c;
    }

    // Image (pixmap): flippable + link action + go-to-source.
    inline Capabilities forImage()
    {
        Capabilities c;
        c.flippable = true;
        c.linkAction = true;
        c.goToSource = true;
        return c;
    }

    // SVG: same profile as image.
    inline Capabilities forSvg()
    {
        return forImage();
    }

    // PDF: background content — minimal menu, and NOT duplicable.
    inline Capabilities forPdf()
    {
        Capabilities c;
        c.duplicate = false;
        return c;
    }

    // Widget/app: go-to-source only (Frozen / Transform-as-Tool are added by the
    // widget delegate override, not part of the base profile).
    inline Capabilities forWidget()
    {
        Capabilities c;
        c.goToSource = true;
        return c;
    }

    // Media (video/audio): go-to-source only (transport controls are a toolbar,
    // not menu entries).
    inline Capabilities forMedia()
    {
        Capabilities c;
        c.goToSource = true;
        return c;
    }

    // Group: link action; flippable/rotatable are recomputed at runtime from the
    // children by the group item, so they start false here.
    inline Capabilities forGroup()
    {
        Capabilities c;
        c.linkAction = true;
        return c;
    }
}

#endif // UBITEMCAPABILITIES_H
