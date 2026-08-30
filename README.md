# Open-Sankoré — Community Edition

![Build Windows](https://github.com/davidguyomarch/Sankore/actions/workflows/build-windows.yml/badge.svg)
![Build Linux x64](https://github.com/davidguyomarch/Sankore/actions/workflows/build-linux.yml/badge.svg)
![Build Linux ARM64](https://github.com/davidguyomarch/Sankore/actions/workflows/build-linux-arm64.yml/badge.svg)
![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/davidguyomarch/0b042ec6f4db15ba496d37215c18a649/raw/coverage.json)

Interactive whiteboard application for education, built with C++ and Qt 6.

## About

Open-Sankoré Community Edition is an actively maintained continuation of the
original [Open-Sankoré](https://en.wikipedia.org/wiki/Open-Sankor%C3%A9) project
developed by GIP ENA (2010–2013). The original project was discontinued in 2013.

This Community Edition modernizes the software for current operating systems,
adds a new QML-based user interface, and preserves full compatibility with
existing `.ubz` documents.

## Features

- **Freehand drawing** — pen, marker, eraser with configurable colors and sizes
- **Geometric instruments** — ruler, compass, protractor, triangle, aristo
- **Shape creation** — rectangles, ellipses, lines, polygons via the shapes palette
- **Document management** — open, save, and organize `.ubz` documents
- **PDF import and export** — import PDF as backgrounds, export boards to PDF
- **Desktop annotation mode** — draw over your desktop
- **Multi-page whiteboard** — page navigator with thumbnail sidebar
- **Handwriting recognition** — OCR via Zinnia (Linux) or Windows Ink (Windows)
- **Laser pointer** — visual pointer for presentations
- **Modern QML interface** — StylusPalette, TopBar, PageNavigator, DrawingPropsBar, ShapesPalette
- **Dark / light theme** — switchable via the theme manager
- **School fonts** — bundled cursive fonts for education (Marelle, Andika, Écolier)

## Downloads

Pre-built packages are available on the
[Releases](https://github.com/davidguyomarch/Sankore/releases) page:

| Platform | Architecture | Format |
|----------|-------------|--------|
| Windows | x64 | `.exe` installer + `.zip` portable |
| Linux | x64 | `.deb` (Debian/Ubuntu) + `.rpm` (Fedora/RHEL) |
| Linux | ARM64 | `.deb` (Debian/Ubuntu) + `.rpm` (Fedora/RHEL) |

## Building from source

### Requirements

| Dependency | Tested version | Notes |
|------------|---------------|-------|
| Qt 6 | 6.8.2 (Windows CI), 6.8.3 (Linux) | Modules: core5compat, multimedia, svg, qml, quick, quickwidgets, printsupport |
| OpenSSL | 3.x | Apache-2.0. Installed via vcpkg (Win) or system (Linux) |
| QuaZip | 1.4 | Built from source in CI |
| zlib | 1.3.1 | Built from source (Win) or system (Linux) |
| Zinnia | system | Linux only, for handwriting recognition (`libzinnia-dev`) |

### Windows (MSVC 2022)

```bash
qmake OpenSankore.pro CONFIG+=no_webengine
nmake release
windeployqt --release build\win32\release\product\Open-Sankore.exe
```

### Linux (Docker — recommended)

The project provides a Docker-based development workflow. Build the dev image
once, then use the build script:

```bash
# Build the dev image (once)
docker build -f Dockerfile.dev -t sankore-dev .

# Full validation: build + unit tests + coverage (~1-3 min)
./scripts/docker-build.sh

# Build only (~30s incremental)
./scripts/docker-build.sh --build-only

# Tests + coverage only
./scripts/docker-build.sh --test-only
```

See [`.kiro/steering/dev-workflow.md`](.kiro/steering/dev-workflow.md) for the
complete development workflow.

## Testing

Unit tests use the QTest framework and run inside Docker:

```bash
./scripts/docker-build.sh --test-only
```

Current status: 17+ test suites, 83%+ line coverage on tested modules.

## Architecture

```
src/
  adaptors/           # Import/export (PDF, SVG, CFF, documents)
  board/              # Whiteboard controller, view, palette manager
  controllers/        # QML V2 controllers (Tool, Page, App)
  core/               # Application lifecycle, settings
  document/           # Document model and controller
  domain/             # Graphics scene, items, shapes
  frameworks/         # Stub headers (WebEngine) + single-instance app
  gui/                # Legacy UI widgets
  qml/                # QML V2 interface files + ThemeManager
  recognition/        # Handwriting recognition (Zinnia / Windows Ink)
  tools/              # Geometric instruments (ruler, compass, etc.)
  web/                # OEmbed parser (web controller is stubbed)
  pdf/                # PDF rendering
  pdf-merger/         # Built-in PDF merge library
```

## License

GNU General Public License v3 — see [LICENSE.md](LICENSE.md).

Original source files (GIP ENA) include an OpenSSL linking exception.
New files are GPL-3.0-only. See [NOTICE.md](NOTICE.md) for details.

## Credits

See [CREDITS.md](CREDITS.md) for acknowledgments and
[THIRD_PARTY.md](THIRD_PARTY.md) for the full third-party dependency inventory.
