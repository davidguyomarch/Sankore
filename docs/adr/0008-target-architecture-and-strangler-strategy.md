# ADR-0008: Target architecture and incremental (strangler) migration strategy

- **Status:** Proposed <!-- Proposed | Accepted | Superseded by ADR-XXXX -->
- **Date:** 2026-09-28
- **Deciders:** maintainer (David Guyomarch)
- **Related:** ADR-0001 (QML V2 UI migration); ADR-0007 (unify Board and Desktop modes); #393 (unify board/desktop — the work that surfaced this); #364, #381, #387 (transition-bug class); drawing-model / desktop-mode / qml-ui steering

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
owner (a `SessionController` in the application layer) holding one state:

```
PresentationState = Board | DesktopAnnotation | Documents | Presenter
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
it would strand real users). ADR-0007/#393 is already the first brick (background
becomes a typed value in the model). The next natural bricks are listed under
Consequences → Roadmap.

### What we keep (not rejected)

- **QML V2** (TopBar, palettes, DrawingPropsBar, `UBThemeManager`, shared
  `UBToolbar`): this is already a clean presentation layer and the intended
  target for D1's top layer. ADR-0001 stands.
- **The background model** (`UBBackgroundGrid` enum + `fromInt`/`toToken`): the
  embryo of the D1 domain; extended by ADR-0007.
- **The VM transparency know-how** (#241/#336/#390): feeds the presentation-layer
  `DesktopOverlayView` directly.
- **`startup.log` diagnostics**: the pragmatic answer to "not unit-testable
  headless"; kept until the domain seam (D1) makes more of the logic testable.
- **Qt 6** as the framework: correct multiplatform choice; no change.

## Consequences

**Easier / gained**

- A Qt-free domain (D1) is unit-testable without the VM, shrinking the
  ~25-min-per-cycle feedback loop for core logic.
- The transition-bug class (#364, tool desync, #381, #387) is removed *by
  construction* once D2–D4 land: there is no second scene to desync, no implicit
  cross-controller show/hide ordering, and presentation ≠ content.
- The current #393 UX decision ("return to board" on a desktop-type page) becomes
  trivial and non-looping under D3.
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
- **No headless test for presentation.** D2's state machine is testable, but the
  actual windowing/transparency stays VM-only (`[TAG]` diagnostics).
- **Temporary duplication.** During migration a concept may briefly exist in both
  the old and new shape (e.g. mode flag *and* `PresentationState`). Acceptable if
  short-lived and tracked in the relevant issue.

**Roadmap (indicative, each its own issue/PR; not a commitment to order)**

1. (#393, in progress) Background becomes a typed value; desktop = a background
   type. Presentation kept as-is for now.
2. Extract `SessionController` + `PresentationState` (D2), replacing `mMainMode`
   / `mIsShowingDesktop` / `setInDesktopMode`. This is what makes #393's
   return-to-board clean (D3).
3. Introduce a Qt-free `Document`/`Page`/`Background` domain (D1) behind the
   current `UBGraphicsScene`, with unit tests; adapt persistence as an
   infrastructure adapter.
4. Route user actions through domain commands (D5); migrate undo to target the
   domain.
5. Reduce views to subscribers of one scene per page (D4); retire the second
   desktop scene.

## Alternatives considered

- **Big-bang rewrite on a clean architecture** — rejected. The app ships and has
  users; a from-scratch rewrite is the classic second-system trap and would strand
  the existing Windows/Linux distributions. The value is in the *direction*, not
  in throwing away working, distributed code.
- **Keep the current controller-centric design and keep patching transitions** —
  rejected. This is the status quo that produced #364/#381/#387; the #364
  investigation showed each fix only moved the symptom. The coupling is the
  defect.
- **Rewrite the rendering onto QML/Quick Scene Graph now** (to solve the
  transparency/window-hosting pain at the source) — deferred, not adopted here.
  It is a large, independent decision with its own risk; it would deserve its own
  ADR. D1–D5 deliver most of the benefit (testability, no transition bugs) without
  betting the project on a full view rewrite.
- **Introduce the state machine but leave the domain as `QGraphicsItem`s** —
  rejected as the end state (keeps the core untestable and UI-coupled), but
  accepted as an *intermediate* strangler step (roadmap item 2 before item 3).
