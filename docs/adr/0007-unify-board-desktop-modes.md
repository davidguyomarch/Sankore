# ADR-0007: Unify Board and Desktop annotation modes on a single scene/view

- **Status:** Proposed <!-- Proposed | Accepted | Superseded by ADR-XXXX -->
- **Date:** 2026-09-25
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #350 (4.5.0 review); #364 (first-stroke-on-return, root-caused); #387 (ghost process on quit); #389 (future custom background image — same page-background model); ADR-0006 (dedicated desktop toolbar); desktop-mode / drawing-model steering; `notes/364-desktop-return-firststroke-diagnosis.md`

## Context

Desktop annotation mode is currently a **second, parallel world** alongside the
board:

- a dedicated full-screen translucent overlay **view** (`mTransparentDrawingView`,
  a second `UBBoardView`) with its own **scene** (`mTransparentDrawingScene`),
  separate from the board's `mActiveScene`/`controlView`;
- its own QML toolbar (`DesktopToolbar.qml`) and a re-hosted props bar, wired as
  top-level windows transient-parented to the overlay (ADR-0006);
- entry/exit transitions that hide/disable one world and show the other
  (`UBApplicationController::showDesktop`/`hideDesktop`,
  `UBDesktopAnnotationController::showWindow`/`hideWindow`,
  `UBBoardPaletteManager::changeMode`).

This dual-world design has been the **single largest source of hard, recurring
bugs**, all born from the *transition* between the two scenes/views rather than
from either mode in isolation. Documented cases:

- **#364 — first stroke invisible on return.** Root-caused after ~10 VM cycles: a
  stroke drawn right after returning to the board received `z = errorNumber`
  (≈ −2e7), i.e. *below the page background*, and stayed hidden until a tool
  change re-sorted z. Fixed narrowly (the smooth-stroke item now carries the
  z-layer key), but the *class* of "state inherited from the other world / lost
  in the transition" remains.
- **Tool desync on entry** (toolbar highlights one tool, another is effective),
  which needed a deterministic-Pen-on-entry workaround.
- **#381 — global UI slowdown** after a Desktop→Document round-trip (a palette
  left parented to the always-on-top translucent overlay).
- **#387 — ghost `OpenSankore.exe`** surviving quit (likely a top-level overlay /
  QQuickWidget window not torn down), blocking relaunch via the single-instance
  lock.
- Repeated "the strokes I drew in one mode aren't there in the other" confusion,
  because the two scenes genuinely hold different items.

The maintainer's product intent is simpler than the current model: **entering
Desktop mode should only change the *background* (show the real desktop through a
transparent page) — the drawing surface, tools, and existing annotations should
stay the same.** In other words, Desktop is a *background variant of the board*,
not a separate application mode.

## Requirements (maintainer)

The unified design must satisfy these four product requirements — they resolve
the previously-open windowing and persistence questions:

1. **Shared drawing area.** Board and Desktop share the *same* drawing surface: a
   stroke drawn in Board is visible in Desktop and vice-versa (same scene, same
   items).
2. **Shared interface.** Board and Desktop use the *same* UI: the same toolbar and
   controls, not a parallel toolbar.
3. **Live desktop via a genuinely transparent window (no redraw/capture).** In
   Desktop view the page background is truly transparent so the compositor shows
   the *real* desktop **of the screen Sankoré is displayed on**, live, through the
   page. Sankoré does **not** capture or repaint the desktop — it just lets it
   show through. Consequence (intended): only the desktop of Sankoré's own screen
   is visible; a teacher's *other* screen stays private/separate. This also keeps
   multi-monitor simple (nothing to mirror — the see-through is a compositor hole,
   not pixels Sankoré owns).
4. **Desktop mode is purely "annotate over the see-through desktop".** The
   fullscreen window captures input for drawing; clicking *through* to the apps
   behind is **not** supported (this drops the old overlay's click-through mask —
   an intentional behavior change).
5. **Mode is persisted; reopening replays live.** Saving records the *mode* of the
   page (board vs desktop/see-through). Reopening a page saved in Desktop mode
   shows the **current** live desktop through it (the page stores a see-through
   *flag*, never the live desktop pixels). A **static thumbnail** capture of the
   background for the pages sidebar is acceptable (it is only a preview, not the
   page content).

## Decision

We will **unify Board and Desktop annotation into one scene and one view.**
Desktop mode becomes a **property of the current board page** — "transparent
background so the live desktop shows through" — instead of a separate overlay
world.

Concretely, the target model is:

1. **One scene, one interactive view.** Annotations live in the board's
   `UBGraphicsScene`/`controlView`. There is no second scene
   (`mTransparentDrawingScene`) and no second interactive view
   (`mTransparentDrawingView`) for drawing. Entering/leaving Desktop mode does
   **not** move items between scenes and does **not** swap the interactive view.

2. **Desktop mode = a persisted see-through background + a transparent window.**
   A new page **background type** ("desktop / see-through") makes the page
   background transparent; the fullscreen window is genuinely translucent
   (`WA_TranslucentBackground`, as already proven for the #241 overlay) so the
   compositor shows the live desktop behind. Sankoré does **not** repaint or
   capture the desktop — transparency is a compositor property, cost-free
   (Requirement 3). The same drawing pipeline, the same `UBToolController`, the
   same items, and the same z-ordering apply. The see-through flag is **saved
   with the page** (Requirement 5): the document stores only the *mode*, never
   desktop pixels, so reopening a desktop page shows the *current* live desktop.

   This new background type sits alongside the existing background types (plain,
   grid, Séyès…). It is designed as **one more entry in the page-background model**
   so that a future **user-chosen background image** (see Related) fits the same
   mechanism rather than a special case: background = { none | ruling(kind) |
   see-through | image(ref) }, all selected the same way and serialized the same
   way.

3. **One toolbar surface.** The board's toolbar (`StylusPaletteV2`, already a thin
   instance of the shared `UBToolbar` since #384) is the single toolbar. Desktop
   mode may hide/show board-only affordances (e.g. page navigator) and add its
   capture/return actions, but it does not instantiate a parallel toolbar world.
   `DesktopToolbar.qml` is retired or reduced to a board-toolbar configuration.

4. **Transitions become trivial.** "Enter desktop" = make the top-level window
   cover the screen and set the page background to see-through; "leave desktop" =
   restore the normal window and background. No view disable/re-enable, no
   cross-scene tool save/restore, no cross-world event routing — which is exactly
   what removes the #364 / tool-desync / #381 / #387 class of bugs by
   construction.

This supersedes the **hosting/parallel-toolbar** parts of ADR-0006 for the
drawing surface (see Consequences); ADR-0006's "V2 look, no legacy
`UBDesktopPalette`" intent is preserved.

### Settled by the requirements (previously open)

- **See-through via compositor transparency, not capture or repaint.**
  Requirement 3 is satisfied by a genuinely transparent window (the desktop shows
  through), showing only the screen Sankoré is on. No screenshot, no per-frame
  repaint. The teacher's other screen stays private, and multi-monitor needs no
  special mirroring of desktop pixels.
- **Desktop pages persist as normal pages, storing a mode flag.** Requirement 5
  makes a desktop page a first-class saved page whose background *mode* is
  serialized; no desktop pixels are stored. A static thumbnail capture for the
  pages sidebar is acceptable (preview only).
- **No click-through.** Requirement 4: the mode is purely annotation over the
  see-through desktop; the old click-through mask is removed.

## Consequences

**Easier / fixed by construction**

- The whole "state lost/inherited across the transition" bug class disappears:
  #364 (z-order), tool desync on entry/return, and most likely #387 (no separate
  top-level overlay/QQuickWidget to leak) and #381 (no palette reparenting onto an
  always-on-top overlay).
- One drawing code path to reason about, test, and theme. Strokes drawn "on the
  desktop" are simply strokes on the current page.
- Matches the product mental model (Desktop = see-through board), less surprising
  for teachers.

**Harder / given up / to watch**

- **Windowing risk is low** now that transparency is a plain compositor property
  (Requirement 3): the same `WA_TranslucentBackground` full-screen transparency
  already works for the #241 overlay, and Sankoré neither captures nor repaints
  the desktop, so there is no per-frame cost and no dependency on GPU effects on
  the software-backend VM. The remaining care point is the board view's
  **background cache**: `UBBoardView` uses `CacheBackground`, which must be
  disabled (or the background drawn transparent) for a see-through page so the
  cached opaque page fill does not hide the desktop. Prototype the fullscreen
  transparent presentation on the VM before removing the old overlay path.
- **Click-through is dropped.** The old overlay let clicks pass through to the
  apps behind via a mask (`desktopPalettePath`); the unified fullscreen window
  captures input for drawing instead (Requirement 4). Intentional behavior change
  to document in release notes.
- **Multi-monitor / mirroring** behavior (`UBDisplayManager`) must be re-derived
  for the "board window goes fullscreen over the desktop" case.
- **Captures** (`customCapture`/`screenCapture`) must be re-wired to the unified
  view; the click-mask logic (`desktopPalettePath`) tied to the overlay geometry
  goes away.
- **Large removal + churn** in `UBDesktopAnnotationController`,
  `UBApplicationController::showDesktop/hideDesktop`, and
  `UBBoardPaletteManager::changeMode`. Landed incrementally behind the current
  behavior where possible.
- ADR-0006's dedicated `DesktopToolbar` hosting rationale becomes largely moot for
  the drawing surface; that ADR stays as the historical record and will be marked
  `Superseded by ADR-0007` for the parts this changes, once this ADR is Accepted
  and implemented.
- Behavior is **not** unit-testable headless (overlay, fullscreen, real
  transparency, mouse); validation stays on the Windows VM with `[TAG]`
  diagnostics (see dev-workflow / desktop-mode steering).

**Migration approach (proposed, to refine at implementation time)**

- Introduce the see-through background mode on the board scene first (no window
  change), validate drawing/z-order/tools on it.
- Switch Desktop entry to "board window fullscreen + see-through background",
  keeping the existing overlay path until the new path is validated on the VM.
- Remove `mTransparentDrawingView`/`mTransparentDrawingScene` and the parallel
  toolbar/props-bar hosting once the unified path is confirmed.
- Fold the abandoned #364 return-side experiments (tool cycle, FullViewportUpdate)
  — never merged — permanently; keep the merged z-layer-key fix.

## Alternatives considered

- **Keep two scenes/views, keep patching the transition** — rejected. This is the
  status quo that produced #364, the tool desync, #381 and #387; the #364
  investigation showed each fix only moved the symptom. The coupling between two
  worlds is the defect, not any single line.
- **One scene but keep a separate overlay *view* onto it** (share the scene,
  duplicate the view) — rejected. It removes the cross-scene item problem but
  keeps two interactive views, two input routers, and the view enable/disable
  dance that caused the dirty-region / event-ordering confusion during the #364
  hunt. Most of the transition fragility would remain.
- **Capture the desktop into a static/repainted background image** — **rejected**.
  Requirement 3 is met more cheaply by plain window transparency (compositor
  hole); capturing/repainting would add cost and, worse, could leak another
  screen's content. A static capture is used *only* for the page thumbnail.
- **Share the scene but keep a separate overlay view** — rejected (see above): it
  keeps two interactive views and the transition fragility.

## Open questions (to resolve during implementation, may spawn follow-up ADRs)

1. **How to serialize the background type** in the `.ubz`/document model
   (Requirement 5) — extend the page background model to
   { none | ruling(kind) | see-through | image(ref) } (the last for the future
   custom background image, see Related), backward-compatible so older readers
   degrade gracefully (unknown type → plain).
2. **Toolbar/affordances in desktop presentation.** Which board affordances hide
   (page navigator, top bar?) and how the capture/return actions attach to the
   single shared toolbar (Requirement 2).
3. **Background cache.** Confirm the exact way to make `UBBoardView`'s
   `CacheBackground` not hide the see-through desktop (disable cache for the mode,
   or paint the background truly transparent) — this is the mechanism that, when
   mishandled, produced the #364 z/paint confusion.
