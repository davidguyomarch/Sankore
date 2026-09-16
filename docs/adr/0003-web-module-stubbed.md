# ADR-0003: Stub the web module instead of porting QtWebEngine

- **Status:** Accepted
- **Date:** 2026-09-16
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #155 (embedded browser), THIRD_PARTY.md

> Recorded retroactively.

## Context

The Qt4 application embedded web content (browser, web widgets, oEmbed) via the
old Qt WebKit / later QtWebEngine. QtWebEngine is heavy to build and deploy
(Chromium), was not available/practical in the CI and static-link setup targeted
for Windows, and is not required for the core whiteboard use cases. Yet a lot of
code (`UBWebController`, `UBGraphicsWidgetItem`, delegates) still references a web
API surface.

## Decision

We will **stub the web module** rather than port QtWebEngine. `src/web/` provides
a compile-and-link stub (`UBWebController` methods return empty/no-op values;
`loadUrl` delegates to the system browser via `QDesktopServices::openUrl`), and
`src/frameworks/` holds stub headers for `QWebEngineView/Page/Profile/Settings`
and `QGraphicsWebView` (renders "Web view disabled"). Builds pass
`CONFIG+=no_webengine`.

## Consequences

- The app builds and links without Chromium/QtWebEngine, keeping Windows and CI
  builds tractable.
- Web widgets / embedded browser are non-functional ("Web view disabled");
  external links open in the system browser.
- The stub API must satisfy existing callers, so it carries empty methods that
  may look like dead code but are needed for linkage (see the cleanup notes in
  the dead-code inventory).
- Restoring a real embedded browser (#155) would be a new decision, likely
  superseding this ADR.

## Alternatives considered

- **Port QtWebEngine** — rejected for now: heavy Chromium dependency,
  build/deploy cost (esp. Windows static setup + CI), not needed for core use.
- **Remove all web code outright** — rejected: too many callers across items and
  delegates; a stub keeps the surface compilable and the door open for #155.
