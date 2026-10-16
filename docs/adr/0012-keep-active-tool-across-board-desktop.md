# ADR-0012: Keep the active tool across the Board ↔ Desktop transition

- **Status:** Accepted
- **Date:** 2026-10-13
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #506 (bug: tool not preserved across modes); amends ADR-0007 (unify Board & Desktop on one scene) and the `#390` decision it carried (force a deterministic Pen on desktop entry); ADR-0009 (background kind axis)

## Context

ADR-0007 unified the board and the desktop annotation mode onto a single shared
scene/surface. During that migration, desktop **entry** forced a deterministic
**Pen** tool (`#390`: "a teacher expects to draw immediately") and desktop
**exit** restored the tool the board had before entry (`mBoardStylusTool`). That
made the active tool *asymmetric* across the transition: whatever you had, you
got Pen on the desktop, and your pre-desktop tool back on return.

Now that the two modes are a single unified surface (ADR-0007 completed, #414),
testers found this surprising: the toolbar is the same, the scene is the same,
but the selected tool jumps on each transition. A ruling/background change could
also appear to disturb the tool because of the surrounding churn. The expectation
for a unified surface is simple: **the active tool does not change just because
you cross between board and desktop.**

## Decision

We will **keep the active tool unchanged across the Board ↔ Desktop
transition**, in both directions. Concretely:

- Desktop **entry** (`UBDesktopAnnotationController::showWindow`) no longer forces
  the Pen tool and no longer saves a board tool.
- Desktop **exit** (`hideWindow`) no longer restores a saved board tool.
- The active tool is a shared singleton (`UBToolController`), so it simply
  remains what the user last selected — the Pen stays Pen, the Selector stays
  Selector, both ways.
- Changing the background kind (Fond) or ruling (Lignage) must never change the
  active tool either.

This **amends** the `#390` behaviour that ADR-0007 carried. ADR-0007 stays
Accepted for everything else (single scene, overlay as the presenting surface,
see-through as a transient flip, etc.); only its "force Pen on desktop entry /
restore on exit" consequence is superseded by this ADR.

## Consequences

**Easier / better**
- The tool is stable and predictable on a unified surface — no more jump on each
  transition, matching the "one surface" mental model.
- Less state: the `mBoardStylusTool` save/restore and the entry Pen-bounce are
  removed, so there is no tool/toolbar desync window to reason about.
- Symmetry: board→desktop and desktop→board behave identically.

**Harder / to watch**
- A teacher who dropped to the desktop with a non-drawing tool (e.g. Selector)
  will not auto-get the Pen; they pick a drawing tool as they would on the board.
  This is the intended, consistent behaviour, not a regression.
- The entry Pen-bounce also used to re-assert the toolbar highlight; we must
  ensure the toolbar highlight still reflects the shared tool after the
  transition (the shared `UBToolController` signals already drive it).

## Alternatives considered

- **Keep forcing Pen on entry (status quo, ADR-0007 / #390).** Rejected: it is
  the exact surprise #506 reports; it contradicts the unified-surface model now
  that board and desktop are one scene.
- **Remember a separate "last desktop tool" and restore it per side.** Rejected:
  that per-side memory was already removed once (it caused toolbar/effective-tool
  desyncs, noted in the old #390 comment). Two remembered tools is more state and
  still asymmetric; a single shared tool is simpler and is what users expect.
