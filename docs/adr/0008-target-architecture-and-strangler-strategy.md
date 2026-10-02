# ADR-0008: Target architecture and incremental (strangler) migration strategy

- **Status:** Accepted <!-- Proposed | Accepted | Superseded by ADR-XXXX -->
- **Date:** 2026-09-28 (accepted 2026-10-02)
- **Deciders:** maintainer (David Guyomarch)
- **Related:** ADR-0001 (QML V2 UI migration); ADR-0007 (unify Board and Desktop modes); #393 (background-type model — the work that surfaced this); #397 (first-stroke-invisible-on-return — fixed as brick 1); #399 (PresentationState refactoring — delivers D2); #364, #381, #387 (transition-bug class); drawing-model / desktop-mode / qml-ui steering

## Context

Open-Sankoré works, ships (Windows/Linux, `.exe`/`.deb`/`.rpm`), and has users.
It was migrated from Qt 4.8 to Qt 6 and given a QML V2 UI (ADR-0001). But the
core is still organised the way the original 2010–2013 codebase was, and that
organisation is now the main source of expensive, recurring bugs.

The concrete trigger is issue #393 / ADR-0007. Trying to make "Desktop mode" just
a *background type of the board page* exposed the underlying problem: **the
application "mode" is an implicit global state scattered across controllers that
know about each other and push each other around.** `UBApplicationController`
hides the main window, `UBBoardController` points a scene, and
`UBDesktopAnnotationController` owns a *second* scene and a *second* view — and
the three must be kept in sync by hand across every transition. Every new
interaction becomes a minefield of re-entrancy and half-updated state:

- #364 — first stroke invisible after returning from Desktop (z-order inherited
  from the transition), root-caused after ~10 VM cycles;
- #397 — first stroke invisible after returning from Desktop (the main window
  left in `WindowFullScreen` by the transition's display-manager call, so a
  GPU-less VM never presented the freshly painted stroke); eliminated 9 wrong
  hypotheses before measurement found the real cause;
- tool desync on Desktop entry (toolbar shows one tool, another is effective);
- #381 — global UI slowdown after a Desktop→Document round-trip (a palette left
  parented to the always-on-top overlay);
- #387 — ghost `OpenSankore.exe` surviving quit (a top-level overlay window not
  torn down).

These are not independent bugs. They are all instances of one defect: **business
state and presentation state are entangled, and the domain model is expressed
directly as Qt objects (`QGraphicsScene`/`QGraphicsItem`), so there is no
Qt-free core that can be reasoned about or unit-tested.** The consequence is that
most behaviour can only be validated on the Windows VM (~25 min per cycle),
because there is no seam below the UI to test.

We need a written target architecture — a compass — so that each future issue can
pull one brick toward it, instead of each contributor re-deciding the structure
ad hoc. This ADR **decides** that architecture and the migration strategy. It does
**not** authorise a rewrite.

## Decision

We adopt a **layered architecture with a Qt-free domain core and an explicit
presentation state machine**, and we migrate toward it **incrementally (strangler
fig)** — never by big-bang rewrite. Concretely, we commit to the following
decisions.

### D1 — Four layers, dependencies point downward only

```
Presentation (QML + views)      widgets/QML, "dumb", replaceable
        │  depends on
Application (use cases)          orchestration: commands + SessionController
        │  depends on
Domain (pure C++, no Qt UI)      Document, Page, Layer, Item, Tool, Background
        │  depends on
Infrastructure (adapters)        persistence (.ubz), PDF, screen capture, network
```

The **domain** layer must not depend on `QtWidgets`, `QtQuick`, `QGraphicsScene`,
`QGraphicsItem`, or any windowing type. It may use `QtCore` value types
(`QString`, `QPointF`, `QColor`, containers) for pragmatism — a full STL-only core
is not worth the churn — but nothing that pulls in a UI or an event loop. This is
the seam that makes the core unit-testable off the VM.

### D2 — A single, explicit `PresentationState` machine replaces scattered mode flags

Today the "mode" lives in `mMainMode`, `mIsShowingDesktop`,
`UBToolController::setInDesktopMode`, `UBMainWindow` window flags, and implicit
show/hide ordering across three controllers. We replace all of it with **one**
owner holding one state:

```
PresentationState = Board | DesktopAnnotation | Documents | Web
```

Transitions are explicit, guarded against re-entrancy, and each state declares
which views are visible and which single view receives input. No controller
reaches into another to hide a window or disable a view as a side effect; they
subscribe to state changes.

### D3 — Presentation state is orthogonal to page/document content

The **background type of a page** (plain, grid, Séyès, **see-through/desktop**,
image — the model from ADR-0007) is *document content*. The **presentation
state** (are we showing the fullscreen see-through window right now?) is
*session state*. They are **decoupled**: a page may be of type "desktop
see-through" while the session is in `Board` (it simply renders transparent), and
"return to board" is a `DesktopAnnotation → Board` **presentation** transition
that does **not** rewrite the page's background type. This is what makes the #393
"return-to-board loops because the page is still Desktop-type" problem impossible
by construction, and it is the principled version of ADR-0007's Option 2.

### D4 — One logical scene per page, potentially several views onto it

A page has one logical drawing surface (its scene/items). Multiple **views** may
present it: the board view, the fullscreen see-through overlay view, the student
/ second-screen view. Views are subscribers; they never own a divergent copy of
the content. This removes the "two scenes to keep in sync" defect that caused
#364, and it generalises the existing `mControlView`/`mDisplayView` precedent
(two views, one scene) instead of the desktop overlay's separate scene.

### D5 — Application logic runs through commands operating on the domain

User actions become **commands** (`AddStroke`, `ChangePageBackground`,
`AddPage`, …) applied to the domain and pushed on an undo stack, rather than
mutations of `QGraphicsItem`s. `QUndoStack` may remain the mechanism, but commands
target the domain model, not view items. This gives deterministic, testable
undo/redo and a natural place for persistence to hook in.

### D6 — Migrate by strangler fig, one issue at a time; no rewrite

Every step must keep the app shippable. We pull one brick per issue toward D1–D5,
behind existing behaviour where possible, validated on the VM when it touches
presentation. A big-bang rewrite is **explicitly rejected** (second-system trap;
it would strand real users).

### What we keep (not rejected)

- **QML V2** (TopBar, palettes, DrawingPropsBar, `UBThemeManager`, shared
  `UBToolbar`): this is already a clean presentation layer and the intended
  target for D1's top layer. ADR-0001 stands.
- **The background model** (`UBBackgroundGrid` enum + `fromInt`/`toToken`): the
  embryo of the D1 domain.
- **The VM transparency know-how** (#241/#336/#390): feeds the presentation-layer
  `DesktopOverlayView` directly.
- **`startup.log` diagnostics**: the pragmatic answer to "not unit-testable
  headless"; kept until the domain seam (D1) makes more of the logic testable.
- **Qt 6** as the framework: correct multiplatform choice; no change.

## Implementation status (as of acceptance)

D2 is **delivered** via issue #399 (and the #397 fix), as five strangler bricks,
each a separate VM-validated PR on `master`:

1. **#397 fix** — `UBDisplayManager::positionScreens()` no longer forces the main
   window to fullscreen in single-screen mode (it stays Maximized), which fixed
   the invisible-first-stroke-on-return bug. Groundwork for not mutating window
   state across transitions.
2. **`UBPresentationController` (shadow)** — a QObject in `src/core/` holding the
   `State` enum + `state()`/`setState()`/`stateChanged`, updated in mirror of the
   existing transitions; a `[STATE] from -> to` diagnostic. No behaviour change.
3. **Single guarded entry point** — the transition methods
   (`showBoard`/`showInternet`/`showDocument`/`showDesktop`/`hideDesktop`) became
   thin wrappers over private `do*()` bodies, serialized by a `mInModeTransition`
   re-entrancy guard. One user action = one transition.
4. **Subscribers react to `stateChanged`** — `UBBoardPaletteManager` is driven by
   `UBPresentationController::stateChanged` (slot `slot_changePresentationState`)
   instead of the legacy `desktopMode`/`mainModeChanged` signals, which were then
   removed along with their dead slots.

What is **not** yet done (future strangler bricks, each its own issue/PR; order
indicative, not a commitment):

- **D1** — a Qt-free `Document`/`Page`/`Background` domain behind the current
  `UBGraphicsScene`, with unit tests; persistence as an infrastructure adapter.
- **D4** — reduce views to subscribers of one scene per page; retire the desktop
  overlay's *separate* scene. NB: on `master` the Desktop overlay still draws on
  its **own** scene, so board and desktop annotations are **not** shared — this is
  the "two surfaces" limitation. ADR-0007's shared-scene goal (its "Option 2" /
  cardinality-2 target) remains **open**; it was prototyped on the
  `feat/393-background-model` branch but not merged, because the GPU-less VM could
  not composite a translucent fullscreen *board* window (black background). D3 +
  D4 are the principled path to it once that windowing constraint is solved.
- **D5** — route user actions through domain commands; migrate undo to the domain.

## Consequences

**Easier / gained**

- A Qt-free domain (D1) is unit-testable without the VM, shrinking the
  ~25-min-per-cycle feedback loop for core logic.
- The transition-bug class (#364, #397, tool desync, #381, #387) is removed *by
  construction* as D2–D4 land: there is no second scene to desync, no implicit
  cross-controller show/hide ordering, and presentation ≠ content. (#397 is
  already fixed; the single guarded entry point and the single `stateChanged`
  subscriber replace the previous double-driven, re-entrant transitions.)
- The #393 UX decision ("return to board" on a desktop-type page) becomes trivial
  and non-looping under D3.
- New features (student view, custom background image #389, presenter mode) slot
  into existing seams instead of adding another parallel world.

**Harder / to watch / given up**

- **Rendering-stack tension.** The mix of `QWidget` + `QGraphicsView` +
  top-level `QQuickWidget` is the root of the transparency pain. This ADR does
  **not** decide to rewrite the rendering onto Quick Scene Graph — that would be a
  large, separate decision (a future ADR) if ever taken. Until then, D1–D5 are
  achievable while keeping `QGraphicsView` as the page renderer; the domain simply
  stops *being* the graphics items.
- **Sustained discipline over a long horizon.** Strangler migration only works if
  each PR genuinely moves toward D1–D5 and none regresses. Without that, we get a
  half-migrated hybrid that is worse than either end state. This risk is real and
  accepted; the ADR is the guardrail.
- **No headless test for presentation.** D2's state logic is unit-tested
  (`tst_UBPresentationController`), but the actual windowing/transparency stays
  VM-only (`[STATE]`/`[TAG]` diagnostics).
- **Temporary duplication.** During migration a concept may briefly exist in both
  the old and new shape (e.g. mode flag *and* `PresentationState`). Acceptable if
  short-lived and tracked in the relevant issue. (The legacy mode signals were
  removed once their single subscriber moved to `stateChanged`.)

## Alternatives considered

- **Big-bang rewrite on a clean architecture** — rejected. The app ships and has
  users; a from-scratch rewrite is the classic second-system trap and would strand
  the existing Windows/Linux distributions. The value is in the *direction*, not
  in throwing away working, distributed code.
- **Keep the current controller-centric design and keep patching transitions** —
  rejected. This is the status quo that produced #364/#381/#387/#397; each
  investigation showed a point fix only moved the symptom. The coupling is the
  defect.
- **Rewrite the rendering onto QML/Quick Scene Graph now** (to solve the
  transparency/window-hosting pain at the source) — deferred, not adopted here.
  It is a large, independent decision with its own risk; it would deserve its own
  ADR. D1–D5 deliver most of the benefit (testability, no transition bugs) without
  betting the project on a full view rewrite.
- **Introduce the state machine but leave the domain as `QGraphicsItem`s** —
  this is the *current* delivered state (D2 done, D1 not yet). Accepted as an
  intermediate strangler step, not as the end state (the domain is still
  UI-coupled until D1).
