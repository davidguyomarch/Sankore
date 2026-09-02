# Open-Sankoré Community Edition — Technical Documentation

## Table of contents

1. [Project overview](#project-overview)
2. [Tech stack](#tech-stack)
3. [Architecture](#architecture)
4. [Source directory structure](#source-directory-structure)
5. [QML V2 interface](#qml-v2-interface)
6. [Build system](#build-system)
7. [Platform support](#platform-support)
8. [Building from source](#building-from-source)
9. [Testing](#testing)
10. [Resources](#resources)
11. [Third-party dependencies](#third-party-dependencies)
12. [License](#license)

---

## Project overview

**Open-Sankoré Community Edition** is a free, open-source interactive whiteboard
application for education. It is an actively maintained continuation of the
original Open-Sankoré project (GIP ENA, 2010–2013), modernized for current
operating systems.

| Field | Value |
|-------|-------|
| Version | See [latest release](https://github.com/davidguyomarch/Sankore/releases/latest) |
| License | GPL-3.0 (with OpenSSL linking exception on original files) |
| Original copyright | © 2010–2013 GIP ENA |
| Community Edition copyright | © 2026 David Guyomarch |
| Language | C++17 |
| Framework | Qt 6.8 |
| Platforms | Windows x64, Linux x64, Linux ARM64 |

This project is not affiliated with, endorsed by, or maintained by GIP ENA or
the original Open-Sankoré project.

### Key features

- Freehand drawing (pen, marker, eraser) with configurable colors and sizes
- Geometric instruments (ruler, compass, protractor, triangle, aristo)
- Shape creation (rectangles, ellipses, lines, polygons)
- Document management (`.ubz` format)
- PDF import and export
- Desktop annotation mode
- Multi-page whiteboard with thumbnail sidebar
- Handwriting recognition (Zinnia on Linux, Windows Ink on Windows)
- Laser pointer for presentations
- Modern QML V2 interface with dark/light theme
- Bundled cursive school fonts (Marelle, Andika, Écolier)

---

## Tech stack

| Component | Technology |
|-----------|-----------|
| Language | C++17 |
| Framework | Qt 6.8.2 (Windows), Qt 6.8.3 (Linux) |
| UI | QML V2 (StylusPalette, TopBar, PageNavigator, DrawingPropsBar, ShapesPalette) + legacy Qt Widgets |
| Build system | qmake (`.pro` / `.pri` files) |
| PDF | pdf-merger (built-in, uses zlib for FlateDecode) |
| Compression | QuaZip 1.4 (`.ubz` document format) |
| Cryptography | OpenSSL 3.x (Apache-2.0) |
| Single instance | QLockFile-based (`qtsingleapplication`) |
| Handwriting (Linux) | Zinnia (BSD-3-Clause) |
| Handwriting (Windows) | Windows Ink API |
| Icons | Phosphor Icons (MIT) |
| Installer (Windows) | Inno Setup |
| Packaging (Linux) | `.deb` + `.rpm` |
| CI/CD | GitHub Actions |
| Dev environment | Docker (Ubuntu 25.04, Qt 6.8.3) |

### What's stubbed / disabled

| Feature | Status | Notes |
|---------|--------|-------|
| Web widgets | ⚠️ Stubbed | QtWebEngine not linked; "Web view disabled" |
| Embedded browser | ⚠️ Stubbed | Delegates to system browser |
| Podcast recording | ⚠️ Disabled | Needs Qt Multimedia rewrite |

---

## Architecture

The application follows an MVC pattern with a modular architecture:

```
┌─────────────────────────────────────────────────────────┐
│                    UBApplication                         │
│               (QLockFile single-instance)                │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │ UBBoard     │  │ UBDocument   │  │ UBApplication │  │
│  │ Controller  │  │ Controller   │  │ Controller    │  │
│  └──────┬──────┘  └──────┬───────┘  └───────┬───────┘  │
│         │                │                   │          │
│  ┌──────▼──────┐  ┌──────▼───────┐  ┌───────▼───────┐  │
│  │ UBGraphics  │  │ UBDocument   │  │ QML V2 UI     │  │
│  │ Scene/View  │  │ Proxy        │  │ Controllers   │  │
│  └─────────────┘  └──────────────┘  └───────────────┘  │
│                                                         │
├─────────────────────────────────────────────────────────┤
│  QML V2 Controllers: UBToolController, UBPageController, │
│  UBAppController, UBThemeManager                         │
├─────────────────────────────────────────────────────────┤
│  Adaptors (PDF, SVG, CFF) │ Frameworks │ Recognition    │
├─────────────────────────────────────────────────────────┤
│  Plugins (CFF Adaptor)    │ PDF Merger │ Tools          │
└─────────────────────────────────────────────────────────┘
```

### Key components

- **UBApplication** — Main singleton, QLockFile-based single instance
- **UBBoardController** — Whiteboard logic (drawing, tools, pages)
- **UBDocumentController** — Document management (open, save, organize)
- **UBToolController** — Active tool, colors, sizes, shapes (QML V2)
- **UBPageController** — Page navigation, add/remove (QML V2)
- **UBAppController** — App mode, background, undo/redo, quit (QML V2)
- **UBThemeManager** — Dark/light theme, QML color palette
- **UBGraphicsScene** — Qt graphics scene containing all board items
- **UBSettings** — Application settings and preferences
- **UBPersistenceManager** — Document persistence

---

## Source directory structure

```
src/
  adaptors/           # Import/export (PDF, SVG, CFF, documents)
  board/              # Whiteboard controller, view, palette manager
  controllers/        # QML V2 controllers (Tool, Page, App)
  core/               # Application lifecycle, settings, main.cpp
  document/           # Document model, document controller
  domain/             # Graphics scene, items, delegates, shapes
  frameworks/         # Stubs (WebEngine), single-instance, utilities
  gui/                # Legacy UI windows, widgets, dialogs
  network/            # HTTP and network features
  pdf/                # PDF rendering (XPDFRenderer stubbed)
  pdf-merger/         # Built-in PDF merge library (uses zlib)
  qml/                # QML V2 interface files + UBThemeManager
  recognition/        # Handwriting recognition (Zinnia / Windows Ink)
  tools/              # Geometric instruments (ruler, compass, etc.)
  desktop/            # Desktop annotation mode
  web/                # OEmbed parser (web controller stubbed)
  transition/         # Uniboard → Sankoré migration
plugins/              # CFF adaptor plugin
tests/                # QTest unit tests (17+ suites)
resources/            # UI forms, icons (Phosphor), fonts, translations
.github/workflows/    # CI pipelines
scripts/              # Build, packaging, and deploy scripts
```

### Key source files

| File | Description |
|------|-------------|
| `src/core/main.cpp` | Entry point, startup logging, crash handler (Windows) |
| `src/core/UBApplication.h/cpp` | Main application class |
| `src/core/UBSettings.h/cpp` | Settings management |
| `src/board/UBBoardController.h/cpp` | Whiteboard logic |
| `src/board/UBBoardView.h/cpp` | Board rendering |
| `src/domain/UBGraphicsScene.h/cpp` | Main graphics scene |
| `src/controllers/UBToolController.h/cpp` | QML V2 tool controller |
| `src/controllers/UBPageController.h/cpp` | QML V2 page controller |
| `src/controllers/UBAppController.h/cpp` | QML V2 app controller |
| `src/qml/UBThemeManager.h/cpp` | Theme manager (dark/light) |

---

## QML V2 interface

The v4.1.0+ UI is built with QML, replacing most of the legacy Qt Widgets palettes.

### Controllers

| Controller | Role | QML property |
|-----------|------|-------------|
| `UBToolController` | Active tool, colors, sizes, shapes | `toolController` |
| `UBPageController` | Page navigation, add/remove | `pageController` |
| `UBAppController` | App mode, background, undo/redo, quit, preferences | `appController` |
| `UBThemeManager` | Dark/light theme, QML color palette | `themeManager` |

### QML files (`src/qml/`)

| File | Widget | Position |
|------|--------|----------|
| `StylusPaletteV2.qml` | Tool palette | Bottom center |
| `TopBar.qml` | Top bar (modes, undo, pages, background, prefs, quit) | Top |
| `PageNavigator.qml` | Page thumbnail sidebar | Left |
| `DrawingPropsBar.qml` | Color/size context bar | Above StylusPalette |
| `ShapesPaletteV2.qml` | Shapes palette + properties | Left, above bottom bar |
| `DocumentsTopBar.qml` | Documents mode top bar | Top (documents mode) |

### Icons

The QML V2 UI uses **Phosphor Icons** (regular weight, MIT License).

- SVG files: `resources/icons/phosphor/*.svg`
- QRC prefix: `/icons/phosphor/`
- Rendering: `Image` + `ColorOverlay` for theme support

---

## Build system

### Project file: `OpenSankore.pro`

The project uses **qmake** with modular `.pri` files:

```qmake
TARGET = "Open-Sankore"
TEMPLATE = app
CONFIG += c++20

QT += svg svgwidgets network multimedia multimediawidgets
QT += xml qml quick quickwidgets widgets printsupport core5compat

# WebEngine is optional (disabled by default)
# qmake OpenSankore.pro CONFIG+=no_webengine
```

### Version

```
VERSION_MAJ = 4
VERSION_MIN = 2
VERSION_TYPE = r    # a=alpha, b=beta, r=release
VERSION_PATCH = 0
```

### Build output

```
build/
├── win32/release/product/    # Windows executable
└── linux/release/product/    # Linux executable
```

---

## Platform support

| Platform | Architecture | CI | Compiler | Qt | Packages |
|----------|-------------|-----|----------|-----|----------|
| Windows | x64 | ✅ GitHub Actions | MSVC 2022 | 6.8.2 (aqtinstall) | `.exe` installer, `.zip` |
| Linux | x64 | ✅ GitHub Actions | GCC | 6.8.2 (aqtinstall) | `.deb`, `.rpm` |
| Linux | ARM64 | ✅ GitHub Actions | GCC | 6.8.3 (system) | `.deb`, `.rpm` |
| macOS | ARM64/x64 | — | — | — | Dev only (Docker builds) |

---

## Building from source

### Windows (MSVC 2022, CI)

```bash
qmake OpenSankore.pro CONFIG+=no_webengine
nmake release
windeployqt --release build\win32\release\product\Open-Sankore.exe
```

### Linux (Docker — recommended)

```bash
# Build the dev image (once)
docker build -f Dockerfile.dev -t sankore-dev .

# Full validation: build + tests + coverage (~1-3 min)
./scripts/docker-build.sh

# Build only (~30s incremental)
./scripts/docker-build.sh --build-only

# Tests + coverage only
./scripts/docker-build.sh --test-only

# x64 via QEMU (~10-15 min)
./scripts/docker-build.sh --x64
```

### Dependencies

| Dependency | Version | Source | Linking |
|------------|---------|--------|---------|
| Qt 6 | 6.8.2 / 6.8.3 | aqtinstall (CI) / system (Linux) | Dynamic |
| OpenSSL | 3.x | vcpkg (Win) / system (Linux) | Dynamic |
| QuaZip | 1.4 | Built from source | Static (Win), Dynamic (Linux) |
| zlib | 1.3.1 | Built from source (Win) / system (Linux) | Static (Win) |
| Zinnia | system | `libzinnia-dev` (Linux only) | Dynamic |

---

## Testing

Unit tests use the **QTest** framework and run inside Docker:

```bash
./scripts/docker-build.sh --test-only
```

- **17+ test suites** covering: document model, file utilities, geometry, SVG transforms,
  string utilities, version parsing, settings, metadata, base32, OEmbed, crypto,
  smooth stroke rendering, graphics scene, handwriting recognition, visual regression
- **83%+ line coverage** on tested modules (measured with lcov)
- **Smoke test**: headless app startup via `--quit-after=5` with `startup.log` analysis

---

## Resources

### Fonts (`resources/customizations/fonts/`)

All bundled fonts are free/open-source:

| Font | License | Purpose |
|------|---------|---------|
| Marelle (8 variants) | OFL 1.1 | Primary cursive school font |
| Andika Basic | OFL 1.1 | Sans-serif reading font |
| Écolier Court | OFL | Cursive school font |
| Écolier Lignes Court | OFL | Cursive school font with lines |

PostScript fonts (URW Base35, AGPL-3.0 + font exception) are in `resources/fonts/`.

Font loading is dynamic — `UBResources::buildFontList()` loads all fonts from the
`customizations/fonts/` directory via `QFontDatabase::addApplicationFont()`.

### Translations (`resources/i18n/`)

40+ languages supported via Qt Linguist (`.ts` files compiled to `.qm` at build time).

### Widgets (`resources/library/applications/`)

30 `.wgt` mini-applications (HTML/JS/CSS) inherited from the original project.

---

## Third-party dependencies

See [THIRD_PARTY.md](../THIRD_PARTY.md) for the complete inventory with licenses,
bundling status, and commercial redistribution terms.

| Component | License | Bundled? |
|-----------|---------|----------|
| Qt 6 | LGPL-3.0 | No (system/CI) |
| OpenSSL | Apache-2.0 | No (vcpkg/system) |
| QuaZip | LGPL-2.1 | No (built from source) |
| zlib | zlib | No (built from source / system) |
| pdf-merger | GPL-3.0 | Yes (`src/pdf-merger/`) |
| Zinnia | BSD-3-Clause | No (system, Linux only) |
| Phosphor Icons | MIT | Yes (`resources/icons/phosphor/`) |
| Font Awesome | OFL / MIT | Yes (2 legacy widgets only) |

---

## License

Open-Sankoré Community Edition is distributed under the **GNU General Public
License version 3** (GPL-3.0).

**Original source files** (GIP ENA, 2010–2013) include an OpenSSL linking exception
as an additional permission under GPL section 7. The verbatim text of this exception
is preserved in [LICENSE.md](../LICENSE.md).

**New source files** (David Guyomarch, 2026) are licensed under GPL-3.0-only,
without the OpenSSL exception.

Since OpenSSL 3.0 (2021), OpenSSL uses the Apache License 2.0, which is GPL-3.0
compatible. The exception is preserved for legal continuity with the inherited
source headers.

See [LICENSE.md](../LICENSE.md), [NOTICE.md](../NOTICE.md),
[CREDITS.md](../CREDITS.md), and [THIRD_PARTY.md](../THIRD_PARTY.md) for full details.
