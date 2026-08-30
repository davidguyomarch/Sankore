# Open-Sankoré — Project Context

## Overview

Open-Sankoré is a C++/Qt interactive whiteboard application originally built with Qt 4.8.
It has been migrated to Qt 6 and features a new QML V2 interface (v4.1.0+).

## Current Status (v4.1.0)

- **Windows x64 build**: ✅ Open-Sankore.exe + installer (.exe) via GitHub Actions CI
- **Linux x64 build**: ✅ binary + .deb + .rpm via GitHub Actions CI
- **Linux ARM64 build**: ✅ binary + .deb + .rpm via GitHub Actions CI
- **QML V2 UI**: ✅ StylusPaletteV2, TopBar, PageNavigator, DrawingPropsBar, ShapesPaletteV2
- **Unit tests**: ✅ 17+ suites QTest (83%+ line coverage)
- **Documents mode**: ✅ Réactivé (ancienne vue UBDocumentController)
- **Desktop mode**: ⚠️ Réactivé mais crash au clic souris (#135)

## Architecture

- **Build system**: qmake (`.pro` / `.pri` files)
- **Language**: C++17
- **Framework**: Qt 6.8.2 (Windows CI), Qt 6.8.3 (Linux / Docker dev)
- **Target platforms**: Windows x64 (MSVC 2022), Linux x64 (GCC 14), Linux ARM64
- **CI**: GitHub Actions (`build-windows.yml`, `build-linux.yml`, `build-linux-arm64.yml`, `release.yml`)
- **Docker dev**: `sankore-dev` image (Ubuntu 25.04, Qt 6.8.3, aarch64)
- **Issue tracking**: https://github.com/davidguyomarch/Sankore/issues
- **Git workflow**: voir `.kiro/steering/dev-workflow.md`
- **CI/CD doc**: voir `.kiro/steering/ci-cd.md`

## Key Directories

```
src/
  adaptors/           # Import/export (PDF, SVG, CFF, documents)
  board/              # Whiteboard controller, view, palette manager, drawing controller
  controllers/        # QML V2 controllers (UBToolController, UBPageController, UBAppController)
  core/               # Application lifecycle, settings, main.cpp
  document/           # Document model, document controller
  domain/             # Graphics scene, items, delegates, shapes
  frameworks/         # Stub headers for WebEngine + single-instance app
  gui/                # UI windows, widgets, old palettes (legacy, partially dead)
  network/            # HTTP and network features
  pdf/                # PDF rendering
  pdf-merger/         # PDF merge library (built-in, uses zlib)
  qml/                # QML files (V2 UI) + UBThemeManager
  web/                # Web controller (stubbed) + OEmbed parser
  podcast/            # Recording (disabled — needs Qt Multimedia rewrite)
  tools/              # Drawing tools (ruler, compass, etc.)
  desktop/            # Desktop annotation mode
  transition/         # Uniboard→Sankoré migration
plugins/              # CFF adaptor plugin
tests/                # QTest unit tests
resources/            # UI forms, icons (Phosphor), translations
.github/workflows/    # CI pipelines
.kiro/steering/       # Steering files for Kiro
```

## QML V2 Interface (v4.1.0+)

### Controllers (src/controllers/)

| Controller | Rôle | QML property name |
|-----------|------|-------------------|
| `UBToolController` | Outil actif, couleurs, tailles, formes | `toolController` |
| `UBPageController` | Navigation pages, ajout, suppression | `pageController` |
| `UBAppController` | Mode app, fond, undo/redo, quit, préférences | `appController` |
| `UBThemeManager` | Thème clair/sombre, couleurs QML | `themeManager` |

### QML Files (src/qml/)

| File | Widget | Position |
|------|--------|----------|
| `StylusPaletteV2.qml` | Barre d'outils bottom | Centre-bas |
| `TopBar.qml` | Barre top (modes, undo, pages, fond, prefs, quit) | Haut |
| `PageNavigator.qml` | Sidebar miniatures pages | Gauche |
| `DrawingPropsBar.qml` | Couleurs/tailles contextuel | Au-dessus de StylusPalette |
| `ShapesPaletteV2.qml` | Palette formes + propriétés | Gauche, au-dessus bottom bar |

### Code legacy encore actif

Voir issue #131 pour l'inventaire complet. Points clés :
- `UBStylusPalette` / `UBDrawingPalette` / `UBNavigatorPalette` / `UBWebToolsPalette` : supprimés (#154)
- `UBDrawingController` : supprimé, fusionné dans `UBToolController` (#148)
- Floating palettes (backgrounds, erase, page) : fonctionnelles, pas encore QML
- `UBDocumentController` : vue Documents avec ancien design (issue #134)

## Build Dependencies (Windows CI)

| Dependency | Source | Status |
|------------|--------|--------|
| Qt 6.8.3 (+ core5compat, multimedia) | aqtinstall | ✅ |
| QuaZip 1.4 (static) | Built from source (CMake) | ✅ |
| zlib 1.3.1 (static) | Built from source (CMake) | ✅ |
| OpenSSL 3.x | vcpkg (openssl:x64-windows) | ✅ |
| QtWebEngine | Stubbed | ⚠️ Not functional |
| pdf-merger | Built-in sources | ✅ |

## Stubs (Compilation Shims)

Files in `src/frameworks/` providing empty implementations:

- `QWebEngineView_stub.h/.cpp` — Stub QWebEngineView with Q_OBJECT
- `QWebEnginePage_stub.h/.cpp` — Stub page with WebAction/NavigationType
- `QWebEngineProfile_stub.h/.cpp` — Stub profile with settings()
- `QWebEngineSettings` — Stub settings with WebAttribute enum
- `QGraphicsWebView.h/.cpp` — Stub (renders "Web view disabled")
- `qtsingleapplication.h/.cpp` — Single-instance via QLockFile (functional)

## What Works vs What's Stubbed

| Feature | Status | Notes |
|---------|--------|-------|
| Core whiteboard | ✅ | Drawing, pages, tools, QML V2 UI |
| Document open/save (.ubz) | ✅ | Via QuaZip |
| PDF export | ✅ | pdf-merger |
| Geometric instruments | ✅ | Ruler, compass, protractor, triangle, aristo |
| Desktop annotation mode | ⚠️ | Réactivé mais crash au clic (#135) |
| Documents view | ✅ | Ancien design, fonctionnel (#134 pour moderniser) |
| Laser pointer | ✅ | Cercle rouge via UBGraphicsScene |
| Shape creation | ✅ | Via ShapesPaletteV2 + UBShapeFactory |
| Pen color change | ✅ | Via DrawingPropsBar + colorPaletteChanged() |
| Web widgets | ⚠️ Stub | "Web view disabled" |
| Embedded browser | ⚠️ Stub | Delegates to system browser |
| Podcast recording | ⚠️ Disabled | Needs Qt Multimedia rewrite (#9) |
| Single-instance | ✅ | QLockFile-based |

## Build Commands

### Windows (CI — GitHub Actions)
```
qmake OpenSankore.pro CONFIG+=no_webengine
nmake release
windeployqt --release build\win32\release\product\Open-Sankore.exe
```

### Linux (Docker local)
```
docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c \
  'qmake6 OpenSankore.pro CONFIG+=no_webengine && make -j$(nproc)'
```

## Distribution Targets

| Platform | Architecture | Format | Status |
|----------|-------------|--------|--------|
| Windows | x64 | .zip (windeployqt) + .exe (Inno Setup) | ✅ |
| Linux | x64 | .deb + .rpm | ✅ |
| Linux | ARM64 | .deb + .rpm | ✅ |

## Important Notes for Development

- **Compilation locale** : Docker Linux uniquement (voir `dev-workflow.md`)
- **Branches** : `fix/<id>-<desc>` ou `feat/<id>-<desc>` (voir `dev-workflow.md`)
- **Push to master** : squash merge uniquement, sur demande explicite
- **Web module is stubbed** — browser/ excluded from build
- **Warnings are non-blocking**: D9025 (/O2 vs /Od) and LNK4217 (UBCFFAdaptor dllimport)

## Icons — Phosphor Icons

The QML V2 UI uses **Phosphor Icons** (regular weight) for all toolbar and palette buttons.

### Location

- SVG files: `resources/icons/phosphor/*.svg`
- QRC file: `resources/icons/phosphor/phosphor.qrc`
- QRC prefix: `/icons/phosphor/`
- QML usage: `source: "qrc:/icons/phosphor/" + iconName + ".svg"`

### Rules for Kiro

1. **Always use icons from the existing set** in `resources/icons/phosphor/` first. List available icons before choosing.
2. **If an icon is missing**, download it from the official Phosphor Icons repository: `https://raw.githubusercontent.com/phosphor-icons/core/main/assets/regular/<name>.svg`
3. **Never hand-craft SVG icons** — always use official Phosphor SVGs.
4. **Add new icons to `phosphor.qrc`** in alphabetical order.
5. **Icon rendering**: via `Image` + `ColorOverlay` in QML for theming support.

### Current icon mapping

| Tool / Action | Phosphor Icon | File |
|--------------|---------------|------|
| Pen | pen | pen.svg |
| Eraser | eraser | eraser.svg |
| Marker | highlighter-circle | highlighter-circle.svg |
| Selector | cursor | cursor.svg |
| Play | play | play.svg |
| Hand | hand | hand.svg |
| Zoom In | magnifying-glass-plus | magnifying-glass-plus.svg |
| Zoom Out | magnifying-glass-minus | magnifying-glass-minus.svg |
| Pointer | crosshair | crosshair.svg |
| Line | line-segment | line-segment.svg |
| Text | text-aa | text-aa.svg |
| Capture | selection | selection.svg |
| OCR | magic-wand | magic-wand.svg |
| Shapes toggle | shapes | shapes.svg |
| Preferences | gear | gear.svg |
| Quit | sign-out | sign-out.svg |
| Undo | arrow-counter-clockwise | arrow-counter-clockwise.svg |
| Redo | arrow-clockwise | arrow-clockwise.svg |
| Bold | text-b | text-b.svg |
| Italic | text-italic | text-italic.svg |
| Underline | text-underline | text-underline.svg |
| Delete | x-circle | x-circle.svg |
| Layer up | arrow-up | arrow-up.svg |
| Layer down | arrow-down | arrow-down.svg |

## Licensing & Legal — Rules for Kiro

### Project license structure

| File | Purpose |
|------|---------|
| `LICENSE.md` | GPL-3.0 full text + verbatim OpenSSL linking exception |
| `NOTICE.md` | Copyright holders, licensing summary, non-affiliation disclaimer |
| `CREDITS.md` | Human-readable acknowledgments (fonts, icons, translators) |
| `THIRD_PARTY.md` | Technical/legal inventory of ALL third-party dependencies |
| `AUTHORS.md` | Copyright holders and maintainer role (separate from each other) |
| `SECURITY.md` | Vulnerability reporting via GitHub Private Security Advisories |
| `CONTRIBUTING.md` | Contribution guide, dev setup, PR checklist |
| `MAINTAINERS.md` | Current maintainer and responsibilities |
| `BRANDING.md` | Logo and visual identity documentation |

### Rules for Kiro when modifying these files

1. **Never mix copyright and maintainer role.** Copyright = intellectual property. Maintainer = operational responsibility. They go in separate sections.
2. **THIRD_PARTY.md is the single source of truth** for all third-party dependencies. When adding or removing a dependency, update this file with: component, version, license (SPDX), bundled status, linking method, commercial redistribution terms, source URL.
3. **CREDITS.md references THIRD_PARTY.md** for details. Keep CREDITS.md human-readable and focused on acknowledgments.
4. **New source files** must include `SPDX-License-Identifier: GPL-3.0-only` in the header. Do NOT include the OpenSSL exception — it applies only to original GIP ENA files.
5. **Never add non-free assets** (fonts, icons, images) without checking the license first. All bundled assets must be free for commercial redistribution (OFL, MIT, Apache-2.0, AGPL with font exception, etc.).

### Fonts — Rules for Kiro

1. **All bundled fonts must be free/open-source** (OFL, Apache-2.0, or equivalent). Non-commercial, CC BY-NC-ND, or proprietary fonts are NOT acceptable.
2. **Current free fonts**: Andika Basic (OFL), Écolier Court (OFL), Écolier Lignes Court (OFL), Marelle (OFL), URW Base35 (AGPL + font exception).
3. **Marelle is the primary cursive school font** — it replaces all previously removed non-free school fonts (Cursive Standard, EcritureA/B, ScriptEcole, Alphonet, etc.).
4. **When adding a new font**, add it to `resources/customizations/fonts/`, include its license file, and update both CREDITS.md and THIRD_PARTY.md.
5. **Font loading is dynamic** — `UBResources::buildFontList()` loads all fonts from `customizations/fonts/` via `QFontDatabase::addApplicationFont()`. No code changes needed when adding/removing font files.

### Icons — Rules for Kiro (extends Phosphor section above)

1. **Phosphor Icons (MIT)** are the only icon set used in the QML V2 UI. Attribution is in CREDITS.md and THIRD_PARTY.md.
2. **Font Awesome (OFL/MIT)** is embedded in 2 legacy widgets only (Combinoscope, Compteur). Do not add Font Awesome to new code.
3. **No Kamiyamane/Fugue icons** remain in the project. The historical credit was removed — do not re-add.

### OpenSSL exception

- Original files (GIP ENA, 2010-2013) carry a GPL-3.0 + OpenSSL linking exception in their headers.
- New files use GPL-3.0-only — do NOT add the OpenSSL exception to new code.
- The verbatim exception text is preserved in `LICENSE.md` for legal continuity.
- Since OpenSSL 3.0 (Apache-2.0), the exception is no longer technically necessary but is preserved for the inherited headers.

### Security

- **No personal emails** in any committed file. Use GitHub Private Security Advisories for vulnerability reporting.
- **No secrets, tokens, or credentials** in committed files.
