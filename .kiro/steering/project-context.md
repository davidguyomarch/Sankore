# Open-Sankoré — Project Context

## Overview

Open-Sankoré is a C++/Qt interactive whiteboard application originally built with Qt 4.8.
It has been migrated to Qt 6 and features a new QML V2 interface (v4.1.0+).

## Current Status (v4.1.0)

- **Windows x64 build**: ✅ Open-Sankore.exe + installer (.exe) via GitHub Actions CI
- **Linux x64 build**: ✅ binary + .deb + .rpm via GitHub Actions CI
- **Linux ARM64 build**: ✅ binary + .deb + .rpm via GitHub Actions CI
- **QML V2 UI**: ✅ StylusPaletteV2, TopBar, PageNavigator, DrawingPropsBar, ShapesPaletteV2
- **Unit tests**: ✅ 6 suites QTest
- **Documents mode**: ✅ Réactivé (ancienne vue UBDocumentController)
- **Desktop mode**: ⚠️ Réactivé mais crash au clic souris (#135)

## Architecture

- **Build system**: qmake (`.pro` / `.pri` files)
- **Language**: C++17
- **Framework**: Qt 6.8.2 (Windows), Qt 6.2 (Linux)
- **Target platforms**: Windows x64 (MSVC 2022), Linux x64 (GCC 11), Linux ARM64
- **CI**: GitHub Actions (`build-windows.yml`, `build-linux.yml`, `build-linux-arm64.yml`, `release.yml`)
- **Docker dev**: `sankore-qt6` image (Ubuntu 24.04, Qt 6, aarch64)
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
- `UBStylusPalette` / `UBDrawingPalette` : cachés mais `buttonGroup()` encore appelé
- `UBDrawingController` : coexiste avec `UBToolController` (issue #128)
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
