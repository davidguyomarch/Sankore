# ADR-0010: Homogeneous capability model for object context menus

- **Status:** Proposed
- **Date:** 2026-10-05
- **Deciders:** David Guyomarch (maintainer)
- **Related:** #450, #367 (shape fill colour via menu), #251 (add image via context menu), ADR-0005 (shared pen/shape palette)

## Context

Every board item (shape, pen/marker stroke, text, image, SVG, PDF, widget,
media, group) owns a `UBGraphicsItemDelegate`. When selected, the item shows a
`UBGraphicsDelegateFrame` with floating buttons; a "…" (`dots-three`) button
opens a `QMenu` built by `UBGraphicsItemDelegate::decorateMenu()` (and a few
overrides). There is no native right-click menu on the board (the only
`contextMenuEvent` override is the text item's trivial deferral to Qt's default).

The product goal is a **consistent experience across all object types**. The
current state does not meet it. An inventory of the delegate code shows:

- **Frame buttons are already homogeneous**: Delete / Duplicate / "…" / Z-order
  up / Z-order down on every type.
- **The "…" menu diverges sharply**, driven by boolean flags set inconsistently
  in each item/delegate constructor:
  - **Flip** menu entries ("Flip on horizontal/vertical axis") exist **only on
    shapes** (`UBAbstractGraphicsItem` sets `setHorizontalMirror/VerticalMirror`).
    Image, SVG and smooth strokes are `setFlippable(true)` (resize-handle flip)
    but expose **no** flip menu entry — two unrelated axes (`mFlippable` vs the
    mirror flags) applied inconsistently.
  - **"Add an action"** is present on shapes, strokes, text, image, SVG, group,
    but **absent** on PDF, widget and media (`canTrigAnAction` left false) with
    no stated rationale.
  - **PDF** is the most degenerate: Locked + Visible only.
  - **Duplicate** is universal-by-accident: `mCanDuplicate` defaults to `true`
    and nothing ever disables it (even a PDF background page shows Duplicate).
  - **Two mechanisms coexist**: most types use the "…" menu, but **text** and
    **media** use a `buildButtons()` toolbar. Image and widget request
    `useToolBar=true` but provide no `buildButtons()` — a latent empty toolbar.
  - **`UBGraphicsGroupContainerItemDelegate` duplicates base code** (four
    `//TODO claudio ... duplicated code` blocks): its `decorateMenu` does not
    call the base and re-implements Locked/Visible; `onAddActionClicked` /
    `saveAction` / `onRemoveActionClicked` are verbatim copies.
  - **Shapes have no fill/stroke colour entry** in the menu (the item-level
    `setFillColor`/`setStrokeColor` exist but are not wired) — the gap behind
    #367.

The divergence is not "menus are missing" — they exist everywhere. It is that
the menu contents are governed by scattered, inconsistently-set flags plus some
copy-paste debt.

## Decision

We will make the object context menu **capability-driven from a single explicit
matrix**, instead of per-constructor ad-hoc flags, and align the inconsistent
behaviours to that matrix.

1. **Define one capability matrix** (below) as the source of truth for which menu
   entries / frame buttons each object type exposes. Each item type declares its
   capabilities in one place; `decorateMenu()` and `init()` read that declaration
   rather than a scattered set of setters.
2. **Make the group delegate reuse the base** `decorateMenu()` and the base
   action handlers — delete the duplicated `//TODO claudio` blocks.
3. **Couple flip to flippability**: a "Flip" menu affordance appears whenever the
   object is flippable, so flip is consistent with the resize-handle flip
   behaviour (no more "flippable but no flip entry").
4. **Rationalise "Add an action"**: enable it for every content object for which
   an action is meaningful, or document per type why not.
5. **Remove latent empty toolbars**: an item either provides `buildButtons()`
   content or does not request `useToolBar`.
6. **Treat #367 (shape fill/stroke colour) as a cell of this matrix**, not a
   one-off: shapes with `hasFillingProperty()`/`hasStrokeProperty()` expose a
   colour affordance wired to the existing `setFillColor`/`setStrokeColor`.

### Target capability matrix (to ratify)

Legend: ✅ = exposed, ✖ = intentionally not exposed, (tbd) = decision pending.

| Type | Delete | Duplicate | Z-order | Lock | Visible-ext | Flip | Add action | Colour (fill/stroke) | Go-to-source | Type-specific |
|------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|---|
| Shape (rect/ellipse/line/poly…) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (#367) | ✖ | Return-to-creation (polygon) |
| Freehand shape | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | (tbd) | ✖ | — |
| Pen/marker stroke | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✖ | ✖ | — |
| Text | ✅ | ✅ | ✅ | ✅ | ✅ | ✖ | ✅ | ✖ | ✖ | Editable + rich toolbar |
| Image | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✖ | ✅ | — |
| SVG | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✖ | ✅ | — |
| PDF | ✅ | (tbd) | ✅ | ✅ | ✅ | ✖ | (tbd) | ✖ | ✖ | — |
| Widget/app | ✅ | ✅ | ✅ | ✅ | ✅ | ✖ | (tbd) | ✖ | ✅ | Frozen, Transform-as-Tool |
| Media (video/audio) | ✅ | ✅ | ✅ | ✅ | ✅ | ✖ | (tbd) | ✖ | ✅ | Transport toolbar |
| Group | ✅ | ✅ | ✅ | ✅ | ✅ | (tbd) | ✅ | ✖ | ✖ | (Ungroup — tbd) |

The `(tbd)` cells are the ones to arbitrate before implementation; the ✅/✖ cells
record the intended homogeneous baseline.

## Consequences

Easier:
- One place to read/change what an object type offers; new object types declare
  capabilities explicitly instead of copying constructor incantations.
- The group delegate stops drifting from the base (duplicated code removed).
- #367 and #251 become well-scoped cells of a known matrix rather than ad-hoc
  additions.

Harder / to watch:
- Touching the shared base `decorateMenu` affects all types — changes need the VM
  visual check (menu rendering is not verifiable headless) across several object
  types.
- Behaviour changes are user-visible (e.g. Duplicate possibly removed from PDF,
  flip appearing on more types); each change should land as its own small PR with
  a VM check, not a big-bang.
- The matrix must be ratified first: the `(tbd)` cells (PDF duplicate, Add-action
  on media/widget/PDF, group flip/ungroup, colour on freehand) are product calls.

Follow-up work (separate issues/PRs, after the matrix is Accepted):
- Deduplicate `UBGraphicsGroupContainerItemDelegate` onto the base.
- Introduce the capability declaration and route `decorateMenu`/`init` through it.
- Couple flip menu entries to flippability.
- Wire shape colour entry (#367).
- Decide and implement the `(tbd)` cells.
- Remove empty-toolbar requests (image/widget).

## Alternatives considered

- **Keep patching per type as issues arise** — rejected: it is what produced the
  current scattered flags and copy-paste debt; it will keep diverging.
- **Add a native right-click `contextMenuEvent` menu on every item** — deferred:
  it is a separate interaction question (and needed anyway for #251 empty-board
  right-click), orthogonal to making the existing "…" menu consistent. Can be a
  later ADR once the capability model exists.
- **Collapse everything into a single uniform menu with no per-type entries** —
  rejected: text (rich formatting), media (transport) and widget (freeze/pin)
  have legitimately type-specific needs; the goal is a consistent *baseline* plus
  declared type-specific extensions, not forced uniformity.
