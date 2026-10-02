# ADR-0009: Page background is two orthogonal axes — ruling type and background kind

- **Status:** Accepted
- **Date:** 2026-10-02
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #393 (board/desktop single-surface unification) + its brick #412 (where this landed); ADR-0007 (unify board & desktop — defines `background = { ruling(kind) | see-through | image(ref) }`); #389 (future custom background image); #289 (French school rulings / `UBBackgroundGrid::Type`); drawing-model steering

## Context

A page background used to be described by a single ruling notion: the
`UBBackgroundGrid::Type` enum (`Plain`, `Grid`, `Seyes`, `SeyesLarge`,
`DoubleLine3mm`) plus a `dark`/`light` flag. "Desktop annotation" (the live
desktop showing through) was **not** part of this model at all — it was a
separate scene-wide flag on the renderer (`setDesktopMode(bool)` →
`mIsDesktopMode`), living entirely in the desktop-overlay world.

ADR-0007 unifies board and desktop onto a single surface, where "desktop" is just
a **see-through page background**. That requires see-through to become a
first-class, per-page, serialized background property. The future custom
background **image** (#389) has the same need. The question this ADR settles:
**how do see-through (and image) fit the background model?**

Two shapes were possible:
- fold `SeeThrough`/`Image` as extra values **into** the existing
  `UBBackgroundGrid::Type` enum, or
- model the background as **two orthogonal axes**: the *ruling* (how the page is
  ruled) and the *kind* (what the page surface is).

ADR-0007 R2 already names the target as `background = { ruling(kind) |
see-through | image(ref) }`, i.e. see-through and image sit beside "ruling", not
inside it.

## Decision

We model a page background as **two orthogonal axes**:

1. **Ruling** — `UBBackgroundGrid::Type` (unchanged): `Plain | Grid | Seyes |
   SeyesLarge | DoubleLine3mm`. Describes *how the page is ruled*.
2. **Kind** — a new `UBBackgroundGrid::BackgroundKind`: `Opaque | SeeThrough |
   Image`. Describes *what the page surface is*:
   - `Opaque` — a normal filled page (white/black per dark mode); the default.
   - `SeeThrough` — no opaque fill, the window is translucent so the live desktop
     shows through. This is the unified "Desktop" page (ADR-0007), replacing the
     old scene-wide `desktopMode` flag.
   - `Image` — a user-chosen background image (future, #389). Reserved in the
     enum + serialization now; rendering is out of scope here.

The two axes are **independent**: a page has both a ruling and a kind. We will
**not** fold kind values into the ruling enum.

Concretely (landed in #412):
- `BackgroundKind` + stable tokens (`kindToToken`/`kindFromToken`) live in the
  pure, unit-tested `UBBackgroundGrid.h`.
- `UBBackgroundRenderer` stores `mBackgroundKind`; the old `mIsDesktopMode` member
  is removed and `setDesktopMode()`/`isDesktopMode()` become thin
  backward-compatible shims over the kind (so the existing overlay keeps working
  during the strangler migration).
- `UBGraphicsScene` exposes `backgroundKind()` / `isSeeThrough()` /
  `setBackgroundKind()`.
- The `.ubz` SVG serializes a `background-kind` attribute, written only when
  non-default; an absent attribute reads back as `Opaque`.

## Consequences

**Easier / better**
- See-through and (future) image are first-class, per-page, **serialized**
  background properties — exactly what ADR-0007 and #389 need — without special
  cases scattered across the code.
- The ruling logic (`isRuled`, `generateLines`, the renderer's line painting) is
  untouched and keeps its clean, unit-tested geometry; kind is a separate concern.
- A see-through page *could* also carry a ruling later (the axes are independent)
  without a model change.
- The old scene-wide "desktop mode" concept collapses into a page property,
  removing a whole class of "which world am I in" coupling (continued by the
  later #393 bricks).

**Harder / to watch**
- Two axes mean two things to set/read/serialize; callers that only thought in
  terms of "ruling" must now also consider kind. Mitigated by `Opaque` being the
  default and the legacy `setDesktopMode` shim.
- `Image` is reserved but not rendered yet; a reader seeing `background-kind=image`
  before #389 lands must degrade gracefully (treated as a background with no image
  → effectively opaque). Backward/forward compatibility relies on the
  "unknown/absent token → Opaque" rule.
- Dark/light remains a third sub-property of the opaque page; it is intentionally
  not merged into the kind axis.

## Alternatives considered

- **Fold `SeeThrough`/`Image` into `UBBackgroundGrid::Type`.** Rejected: it
  conflates "how the page is ruled" with "what the surface is". `SeeThrough` is
  not a ruling (it draws no lines), so every ruling consumer (`isRuled`,
  `generateLines`, the renderer loop) would need a special case, and a
  see-through-page-with-a-ruling would be inexpressible. Simpler to write for one
  brick, worse structurally — and contrary to ADR-0007 R2.
- **Keep the scene-wide `desktopMode` flag, don't model it as a background.**
  Rejected: it cannot be persisted per page (ADR-0007 R5) and keeps desktop as a
  separate world, which is exactly what #393 removes.
- **A single free-form background descriptor object now.** Rejected as premature:
  two enums + a token cover the known cases (ruling, see-through, image) with
  trivial, backward-compatible serialization; a richer object can come if a real
  need appears.
