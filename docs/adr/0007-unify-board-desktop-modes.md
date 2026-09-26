# ADR-0007: Unify Board and Desktop annotation modes on a single scene/view

- **Status:** Proposed <!-- Proposed | Accepted | Superseded by ADR-XXXX -->
- **Date:** 2026-09-25
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #350 (4.5.0 review); #364 (first-stroke-on-return, root-caused); #387 (ghost process on quit); ADR-0006 (dedicated desktop toolbar); desktop-mode / drawing-model steering; `notes/364-desktop-return-firststroke-diagnosis.md`

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
3. **Live desktop, in real time.** In Desktop view the *real* desktop is seen
   through the transparent page **live** — if the desktop changes (a window moves,
   a video plays), it updates in real time behind the annotations. This rules out
   a static desktop screenshot painted as the background.
4. **Mode is persisted; reopening replays live.** Saving records the *mode* of the
   page (board vs desktop/see-through). Reopening a page saved in Desktop mode
   shows the **current** live desktop through it (again: the page stores a
   see-through *flag*, never a captured desktop image).

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

2. **Desktop mode = a persisted see-through background + a live window
   presentation.** A new background type/flag ("desktop / see-through") on the
   page makes its background transparent so the **live** real desktop is visible
   behind the annotations, updating in real time (Requirement 3). The same
   drawing pipeline, the same `UBToolController`, the same items, and the same
   z-ordering apply. The see-through flag is **saved with the page** (Requirement
   4): the document stores only the *mode*, never a desktop screenshot, so
   reopening a desktop page shows the *current* live desktop through it.

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

- **Live transparency, not a capture.** Requirement 3 mandates a real see-through
  window showing the live desktop, so the "captured desktop image as background"
  fallback is **excluded**. Implementation must achieve real-time transparency
  (and prove it on the GPU-less test VM — the main technical risk, see
  Consequences).
- **Desktop pages persist as normal pages, storing a mode flag.** Requirement 4
  makes a desktop page a first-class saved page whose background mode is
  serialized; no desktop pixels are stored. Reopening replays the live desktop.

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

- **Live translucent windowing on Windows is the main risk, and now mandatory.**
  Requirement 3 forbids the static-capture fallback, so the implementation must
  reveal the *live* desktop through a real translucent full-screen window and
  update in real time. This is fragile on the GPU-less test VM, where Qt Quick
  falls back to the software backend (see the qml-ui / desktop-mode steering).
  This must be prototyped on the VM **first**, before committing to the unified
  window path — if live transparency proves unachievable on target hardware, this
  ADR must be revisited (a follow-up ADR), not silently downgraded to a capture.
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
- **Capture the desktop into a static background image on entry** — **rejected**
  by Requirement 3 (the desktop must be seen live, updating in real time). Kept
  here only as the documented fallback *iff* live transparency turns out to be
  impossible on target hardware, in which case a follow-up ADR would revisit the
  requirement rather than silently substitute a static capture.

## Open questions (to resolve during implementation, may spawn follow-up ADRs)

1. **How to serialize the see-through mode** in the `.ubz`/document model
   (Requirement 4) — a new page background type/attribute, backward-compatible
   with existing documents (older readers should degrade gracefully).
2. **Toolbar/affordances in desktop presentation.** Which board affordances hide
   (page navigator, top bar?) and how the capture/return actions attach to the
   single shared toolbar (Requirement 2).
3. **Multi-monitor** presentation when the board window goes fullscreen over one
   screen while a second screen mirrors/extends (`UBDisplayManager`).
4. **Thumbnails** of a desktop page: what a saved desktop page shows in the pages
   sidebar (annotations over a neutral/transparent background, since no desktop
   pixels are stored).
