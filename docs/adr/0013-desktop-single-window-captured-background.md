# ADR-0013: Desktop mode is a single-window see-through page backed by a refreshable desktop capture (supersedes the live-overlay path of ADR-0007)

- **Status:** Accepted <!-- Proposed | Accepted | Superseded by ADR-XXXX -->
- **Date:** 2026-10-17
- **Deciders:** maintainer (David Guyomarch)
- **Related:** supersedes the live-see-through / windowing decision of [ADR-0007](0007-unify-board-desktop-modes.md) (R3 and the overlay implementation it ended on); builds on [ADR-0009](0009-background-kind-axis.md) (background kind axis — `Opaque | SeeThrough | Image`) and [ADR-0011](0011-custom-background-image.md) (image embedded + cover fit + `drawBackground` path); keeps [ADR-0012](0012-keep-active-tool-across-board-desktop.md) (tool kept across the transition); `notes/408-seethrough-board-spike-decision.md` (the #413 follow-up — live main-window see-through hit the #397 presentation wall and was abandoned); #393/#414 (current overlay-renders-shared-scene state); #433/#503/#514/#515 (desktop-toolbar cost/bugs this removes); #397 (paint-but-not-present wall on the software VM)

## Context

The maintainer's goal is an architecture **as simple and as homogeneous as
possible between Board and Desktop**, because the app — and Desktop mode in
particular — is too complex and feels laggy. They are willing to recode a large
part to get there.

### Where we actually are today (measured, not assumed)

A code audit (not steering) established the current shape:

- **Two parallel UI stacks.** The drawing canvas is QWidget/`QGraphicsScene`/
  `QGraphicsView` (`UBBoardView`); the chrome (TopBar, StylusPaletteV2,
  DrawingPropsBar, …) is QML `QQuickWidget` overlaid on top. They talk only
  through context-property controllers. This split is structural and is **not**
  what this ADR changes.
- **Desktop mode duplicates almost the whole board.** Since #414 (ADR-0007 brick
  3) the board and desktop share the **scene**, the **tools** (`UBToolController`,
  ADR-0012) and the **input router** — but desktop still runs a **second
  `QGraphicsView`** (`mTransparentDrawingView`, a full-screen always-on-top
  `WA_TranslucentBackground` top-level window) rendering that shared scene, plus a
  **second QML hosting model** (top-level translucent transient-parented
  `DesktopToolbar` + a second `DrawingPropsBar`), a per-view `drawBackground`
  split, and a full-screen click **mask** recomputed on tool changes / resizes /
  keyboard moves.
- **That duplication is the cost.** The lag and the complexity come overwhelmingly
  from the translucent full-screen overlay: per-frame alpha compositing of the
  whole screen on the GPU-less VM, `CacheMode::CacheNone`, two views rendering one
  scene at once, and the `O(items)` mask rebuild. The code itself records this —
  `hideWindow()` drops the always-on-top hint "because a hidden translucent
  full-screen window still slowed the whole UI". A string of recent issues
  (#433 desktop TopBar, #503 ColorOverlay→MultiEffect, #514 dark icon tile, #515)
  exist **only** because of the translucent-overlay hosting.

### Why ADR-0007's live-see-through path is no longer the plan

ADR-0007 R3 chose a **genuinely transparent window showing the live desktop**
(no capture, no repaint). The #408 spike green-lit that on the *dedicated overlay*
window — but the #413 follow-up (recorded in the #408 note) then measured that
making the **real presenting surface** translucent hits the **#397 wall**: on the
software VM backend a complex, already-shown translucent window paints the stroke
but **does not present it** without a native event. The project therefore stopped
at **brick 3 (#414)**: keep the overlay as the presenting surface, but render the
shared scene. That is precisely the duplicated, laggy state we now want gone.

So the live-see-through requirement is the thing standing between us and a single
window. The maintainer has decided to **trade live see-through for simplicity**:
a frozen capture of the desktop, refreshed at the right moments, is good enough
for the teaching use case (annotate an app, go manipulate it, come back → refresh)
and lets us delete the whole overlay.

## Decision

We will make **Desktop mode a single-window feature with no overlay**: the Board
window is the only drawing surface, and "Bureau" becomes an ordinary board page
whose **background is a captured image of the desktop**, refreshed on events.

Concretely:

1. **One window, one view, no overlay.** Remove `mTransparentDrawingView`, the
   top-level translucent `DesktopToolbar`/`DrawingPropsBar` hosting, the
   transient-parent dance, the per-tool alpha-0/alpha-1 click-through brush, the
   full-screen `updateMask`/`desktopPalettePath`, and the `bIsDesktop`
   `drawBackground` split. Board and Desktop are rendered by the **same**
   `mControlView` in the **same** main window.

2. **Bureau = a `SeeThrough` page whose surface is a desktop capture.** We do
   **not** make the window translucent (that is the #397 wall). Instead, entering
   Bureau grabs the desktop of Sankoré's current screen once and paints it as the
   page **background**, reusing the `BackgroundKind::Image` paint path and cover
   fit from ADR-0011. The capture is a **transient, non-persisted** background:
   it is the page surface, painted in `drawBackground`, never a scene item, never
   written into the `.ubz`.

3. **The capture is refreshed on events, not continuously.** The desktop
   background is re-grabbed:
   - on **entry** into Bureau — and specifically **just before the window
     transition** so the grab sees the real desktop, not Sankoré's own window
     (the exact sequencing is VM-only to settle, see Open questions);
   - when **Sankoré regains focus** (`QApplication::applicationStateChanged ==
     ApplicationActive` / window-activate) while a Bureau page is shown — this is
     the alt-tab case: the teacher went to another app and came back, so the
     desktop may have changed.
   Between refreshes the background is frozen. This is an accepted, documented
   limitation (no live video behind the annotations). An explicit **"refresh
   desktop"** button is **deferred**: we ship with auto-refresh only (entry +
   focus-return) and add a manual refresh later only if that proves insufficient
   (it may well not be needed).

4. **Shared UI components, no desktop-specific chrome.** The same TopBar,
   StylusPaletteV2 and DrawingPropsBar serve both. `DesktopToolbar.qml` is
   retired. The desktop-only actions (region/screen capture, "refresh desktop",
   leave Bureau) appear **contextually** on the shared toolbar when the current
   page is a Bureau page — driven by the page background, not by a separate mode
   widget.

5. **Persistence — store the Bureau *flag*, never the captured pixels.** A Bureau
   page serializes as a see-through/desktop page (the ADR-0009 kind), **without**
   the captured image. Reopening a Bureau page re-grabs the *current* desktop
   (same contract ADR-0007 R5 intended: store the mode, replay live-ish). The
   pages sidebar thumbnail may show the capture that was current when the page was
   last shown (preview only).

6. **The capture is a runtime-only source, not a new background kind.** The
   persisted model stays ADR-0009's `Opaque | SeeThrough | Image`. A Bureau page
   is a **`SeeThrough` page** (that flag, and only that flag, is serialized). The
   desktop capture is a **runtime-only "desktop capture" source** handed to the
   renderer — **not** a 4th `BackgroundKind` and **not** serialized as `Image`
   (an embedded user image). We reuse ADR-0011's image **paint/cover-fit** path to
   *draw* the capture, but the two must never be confused: `Image` is a persisted
   user picture embedded in the `.ubz`; the desktop capture is transient pixels
   the renderer is given at runtime and discards. From the user's point of view
   this is simply the **"Bureau" entry in the Fond (background) menu**, alongside
   the rulings and the image — choosing it shows the desktop captured at the
   moments above. Invariant: **persisted = SeeThrough flag only; runtime = a
   refreshable capture painted via the image path.**

This **supersedes ADR-0007** on the windowing/see-through mechanism (R3) and on
the overlay implementation it ended at (#414): the drawing surface is the main
window, see-through is a *captured* background (not a live compositor hole), and
there is no second view. It **keeps** ADR-0007's unification intent (one scene,
shared tools/UI — R1/R2) and ADR-0012 (the active tool is kept across the
transition). ADR-0009 (kind axis) and ADR-0011 (image paint path) are reused, not
changed.

## Consequences

**Easier / better**
- The overlay — the single largest source of lag and of recurring bugs — is
  **deleted**, not patched. #433, #503, #514, #515 (all translucent-overlay
  artifacts) become moot or trivial; the #397 presentation wall is sidestepped
  entirely because nothing is translucent.
- Board and Desktop become genuinely homogeneous: one window, one view, one set of
  QML chrome. A Bureau page is "a page with a desktop-capture background", nothing
  more. Transitions are trivial (set/clear the capture background), removing the
  view enable/disable and cross-world event routing.
- Capture cost is a few milliseconds **on an event**, versus continuous full-screen
  alpha compositing today — a net performance win on the GPU-less VM.

**Harder / given up / to watch**
- **No live desktop.** Between refreshes the background is a still image; a video
  or animation behind Sankoré is frozen until the next refresh. Explicit product
  tradeoff (simplicity + perf over live-through). Documented in release notes.
- **Capture timing on focus-return.** When Sankoré regains focus its own window is
  frontmost; the grab must capture the desktop *behind* Sankoré, not Sankoré
  itself. The safe sequencing (grab before re-presenting, or briefly exclude our
  window) is VM-only to validate — instrument with `[DESKTOP]` logs, do not guess.
- **Multi-monitor — capture the screen Sankoré's window is on.** Grab only the
  screen where the main window is displayed (as the overlay did via
  `mTransparentDrawingView->screen()` → now `mainWindow->screen()`); the teacher's
  other screen(s) are **not** captured and stay private. Settled (was Q5).
- **Large removal + churn.** `UBDesktopAnnotationController` shrinks to a small
  "desktop-capture background service"; `UBApplicationController::showDesktop/
  hideDesktop`, `UBBoardPaletteManager::changeMode`, and the `bIsDesktop` paths in
  `UBBoardView` are gutted. Land incrementally behind the current behavior; keep
  the overlay path until the captured-background path is validated on the VM.
- **Not unit-testable.** Window presentation, screen grab, focus events and real
  rendering are VM-only. The testable slivers (the cover-fit rect — already tested
  in `tst_UBBackgroundGrid`; the "persist SeeThrough flag, never the capture"
  round-trip; the refresh-trigger decision as a pure predicate) should be carved
  out and unit-tested; the rest is validated on the VM with `[DESKTOP]`
  diagnostics.

**Migration bricks (proposed, to refine at implementation time)**
1. **Desktop-capture background service.** A small component that grabs the
   current screen and feeds it to the renderer as the transient background of a
   see-through page, via ADR-0011's cover-fit image paint path. No window change
   yet — exercise it on a normal board page first (set capture → see it painted →
   clear).
2. **Refresh triggers.** Wire entry + `applicationStateChanged(Active)` +
   explicit "refresh desktop" action to re-grab. Validate the alt-tab sequencing
   on the VM.
3. **Switch Bureau entry to single-window.** "Enter Bureau" = main window stays
   put (Maximized, never fullscreen on mono-screen — the #397 rule), set the
   current page's background to the desktop capture, show the shared toolbar's
   contextual capture/refresh/leave actions. Keep the overlay path alive behind a
   flag until this is VM-validated.
3b. **Remove the overlay.** Delete `mTransparentDrawingView`, `DesktopToolbar.qml`,
   the second props-bar hosting, the mask, the alpha brush, the `bIsDesktop`
   `drawBackground` split, and the dead transitions. Retire `DesktopToolbar`.
4. **Persistence + thumbnail.** Serialize the Bureau page as SeeThrough (flag
   only); re-grab on reopen; thumbnail = last capture (preview only).

**Follow-up / cleanup enabled by this ADR**
- Close/repurpose #433 (desktop TopBar — now the shared TopBar is always the one),
  #503 (ColorOverlay→MultiEffect prep — no translucent surface to composite on
  anymore), #514 (dark icon tile — overlay artifact gone). Re-scope #515-style
  frame fixes stay (they are page-dark, not overlay).
- Consider, separately, retiring the stubbed Web mode and folding `mMainMode`/
  `mIsShowingDesktop` into `UBPresentationController` (noise reduction; not part
  of this ADR).

**Separate concern — UI-technology homogeneity (NOT decided here)**
This ADR removes the desktop-only duplicated chrome (the translucent
`DesktopToolbar`), which is one source of "two technologies for the clickable
surface". It does **not** address the broader heterogeneity of the *object*
controls: the selection-frame buttons are `QGraphicsView` scene items and the
object context menu ("…") is a native `QMenu`, while the rest of the chrome is
QML. That is tracked as a separate cluster of issues, best tackled **after** this
unification (which removes the translucent-overlay case that complicates
QML-over-scene rendering, cf. #514/#515): object menu → QML (**#517**, bounded,
reuses ADR-0010's model), frame controls → QML (**#518**, gated by scene→view
coordinate tracking), and the canvas itself → QML/RHI (**#519**, epic/long-term,
the structural horizon of the other two). The QWidget-canvas / QML-chrome split
(ADR-0001) is intentional and is not what this ADR changes.

## Alternatives considered

- **Live see-through via a translucent main window (ADR-0007 R3).** Rejected now,
  on measurement: the #413 follow-up showed the main presenting window hits the
  #397 paint-but-not-present wall on the software VM. It only ever worked on a
  dedicated simple overlay — i.e. the duplicated surface we are removing.
- **Keep the overlay but make it lightweight** (re-enable a cache, stop rendering
  two views, drop the `O(items)` mask). Rejected as the primary plan: it recovers
  some performance but **keeps the duplication and complexity** the maintainer
  explicitly wants gone. (Still a reasonable fallback if single-window captured
  background proves unacceptable on the VM.)
- **Keep live-through by capturing and repainting the desktop every frame.**
  Rejected: that is continuous full-screen work — the exact cost we are removing —
  and risks leaking another screen's pixels. Event-driven capture gives most of
  the benefit for almost none of the cost.
- **Make the captured desktop a locked `QGraphicsItem` on the scene.** Rejected
  for the same reason ADR-0011 rejected it for the image: it is the page surface,
  not content; a scene item would need excluding from selection/z-order/erase-all/
  export. Paint it in `drawBackground` like the image and opaque fill.
- **Persist the captured desktop pixels in the `.ubz`.** Rejected: it bloats the
  document and, worse, saves a snapshot of the teacher's desktop into a shared
  file. Store the SeeThrough flag only; re-grab on reopen (ADR-0007 R5 intent).

## Settled during review (2026-10-17)

These were open when the ADR was Proposed; the maintainer resolved them before
acceptance:

- **Transient capture representation** → a **runtime-only "desktop capture"
  source** handed to the renderer; no 4th `BackgroundKind`, never serialized
  (Decision point 6). User-facing: the **"Bureau" entry in the Fond menu**.
- **"Refresh desktop" button** → **deferred**. Ship with auto-refresh only (entry
  + focus-return); add a manual refresh later only if needed (Decision point 3).
- **Multi-monitor** → capture **only the screen Sankoré's window is on**; other
  screens stay private (Consequences / Q5).
- **Captures (region/screen)** → not a decision, just implementation: re-wire them
  to the single view and remove the overlay's `desktopPalettePath` mask (listed in
  the migration bricks).

## Open questions (to resolve during implementation — VM-only)

1. **Entry grab sequencing.** The entry capture must be taken **just before the
   window transition** so it sees the real desktop, not Sankoré's own window
   (grab-before-present, or momentarily exclude the window). This is runtime
   compositor behavior on the software VM — to validate on the VM with
   `[DESKTOP]` diagnostics, not reproducible headless.
