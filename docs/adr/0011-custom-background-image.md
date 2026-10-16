# ADR-0011: Custom page background image — embedded in the .ubz, rendered via BackgroundKind::Image

- **Status:** Accepted
- **Date:** 2026-10-12
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #389 (custom background image); ADR-0009 (background kind axis — reserves `BackgroundKind::Image`); ADR-0007 (unify board & desktop — `background = { ruling(kind) | see-through | image(ref) }`); #412 (where the kind axis landed)

## Context

Teachers want to set a **background image** for a page (a map, a worksheet scan, a
photo) to annotate on top of it. ADR-0009 already settled the *model*: a page
background has two orthogonal axes — a **ruling** (`UBBackgroundGrid::Type`) and a
**kind** (`UBBackgroundGrid::BackgroundKind { Opaque | SeeThrough | Image }`). It
deliberately **reserved `Image` in the enum and in the `.ubz` serialization**
(token `"image"`, with the "unknown/absent token → Opaque" degradation rule) but
left the **storage and rendering of the actual image out of scope**. This ADR
settles those remaining open points so #389 can be implemented without a second
format decision later.

What is already in place (ADR-0009 / #412), and NOT re-decided here:
- `BackgroundKind::Image = 2` and its stable tokens (`kindToToken` /
  `kindFromToken`) in the pure, unit-tested `UBBackgroundGrid.h`.
- `UBGraphicsScene::backgroundKind()/setBackgroundKind()`,
  `UBBackgroundRenderer::mBackgroundKind`.
- The `.ubz` SVG carries a `background-kind` attribute (written only when
  non-default; absent → `Opaque`).
- The two axes are independent: a page can have both a ruling and a kind.

What is still open, and what this ADR decides: **where the image lives**, **how it
is referenced in the document**, and **how it is painted**.

## Decision

We will add a custom page **background image** as the rendering of
`BackgroundKind::Image`, with the following choices:

1. **Storage — embed the image in the `.ubz`.** The chosen image is copied into
   the document package (alongside the page SVG and other page media), not stored
   as an external path reference. This keeps documents portable (the image
   travels with the `.ubz`) and consistent with how other page media are stored.
   An external-path reference is explicitly rejected (breaks when the file moves).

2. **Reference — a `background-image` attribute in the page SVG.** The SVG element
   that already carries `background-kind` also carries a `background-image`
   attribute holding the **relative file name** of the image inside the package.
   Round-trip rule:
   - write `background-image="<name>"` only when `kind == Image` and an image is
     set;
   - on read, `kind == Image` **with** a resolvable `background-image` → render
     the image; `kind == Image` **without** a usable image (missing attribute or
     missing file) → **degrade to Opaque** (extends ADR-0009's degradation rule).

3. **Rendering — in the existing background paint path, not a scene item.** The
   image is painted by the background renderer
   (`UBBackgroundRenderer::paintBackground` / `UBBoardView::drawBackground`) when
   `kind == Image`, exactly where the opaque fill / see-through / ruling are
   handled. The background image is **not** a `QGraphicsItem` on the scene: it is
   not selectable, movable, or part of the z-order — it is the page surface, like
   the opaque fill it replaces.

4. **Fit — one mode first: cover.** The image is scaled to **cover** the page
   while preserving its aspect ratio (centered, cropped as needed), so the page is
   fully painted with no letterboxing. Other fit modes (stretch / fit-inside /
   center / tile) are **deferred**; the serialization may gain an optional
   `background-image-fit` attribute later without breaking documents written now
   (absent → cover).

5. **Orthogonality preserved — ruling may sit on top of the image.** Because
   ruling and kind are independent axes (ADR-0009), a ruling (grid/Séyès…) is
   still painted on top when set. Day/night only affects the **opaque** fill; it
   does **not** recolor or alter the background image.

6. **Scope — per page.** The background image is a per-page property (like the
   kind and ruling it rides with). A per-document "apply to all pages" convenience
   is out of scope for this ADR.

## Consequences

**Easier / better**
- #389 slots into the existing, serialized `BackgroundKind` model with no new
  background concept — one more rendered kind, not a parallel path.
- Documents stay portable (image embedded) and backward/forward compatible: older
  readers (and this app before the renderer lands) see `background-kind=image`
  with no renderable image and fall back to Opaque per the ADR-0009 rule.
- The pure, testable surface grows cleanly: the `background-image` round-trip
  (write-when-Image / read-back / degrade-when-missing) and the cover-fit
  destination-rect geometry can be unit-tested (`tst_UBBackgroundGrid` already
  exists) without any UI.

**Harder / to watch**
- `.ubz` size grows with the embedded image; acceptable (same tradeoff as other
  embedded media) and the price of portability.
- The renderer now has an image-decoding/scaling path in `drawBackground`; it must
  cache the scaled pixmap (not rescale every paint) to stay cheap on the software
  backend (VM).
- Reading a document whose image fails to load must degrade to Opaque silently,
  never crash — same contract as a missing see-through.
- Cover-only fit may crop images with a very different aspect ratio than the page;
  mitigated by deferring the richer fit modes behind an optional attribute.

**Follow-up**
- Implement the renderer path, the embed-on-save / load-from-package wiring, and
  the UI entry (an "Image…" choice in the TopBar background menu → file picker →
  `setBackgroundKind(Image)` + embed). The page thumbnail picks up the image
  automatically since it renders through the same path.
- Later: additional fit modes via `background-image-fit`; a per-document apply.

## Alternatives considered

- **Store an external path reference instead of embedding.** Rejected: smaller
  documents but non-portable — the background breaks as soon as the image file is
  moved, renamed, or the document is opened on another machine. Embedding matches
  the other page media and ADR-0009/#389's intent.
- **Model the background image as a locked `QGraphicsItem` on the scene.**
  Rejected: it is the page surface, not an object. A scene item would need to be
  excluded from selection, z-order, duplication, erase-all, and export-as-object,
  reintroducing the "is this chrome or content" special-casing ADR-0009 removed.
  Painting it in `drawBackground` (like opaque/see-through) is simpler and correct.
- **Ship multiple fit modes now (stretch/center/tile).** Rejected as premature:
  cover handles the common "annotate a full-page scan/photo" case; the other modes
  add UI and serialization surface before there is a demonstrated need. The
  optional `background-image-fit` attribute keeps the door open compatibly.
- **A new top-level background concept separate from `BackgroundKind`.** Rejected:
  ADR-0009 already reserved `Image` precisely so this would not need a new axis.
