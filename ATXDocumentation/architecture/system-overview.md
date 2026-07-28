# System Overview

## Application Identity

- **Name**: Open-Sankoré (formerly Uniboard)
- **Version**: 2.5.1 (release)
- **License**: GNU General Public License v3 with OpenSSL linking exception
- **Copyright**: 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
- **Purpose**: Interactive whiteboard software for digital education

## Technology Stack

| Layer | Technology | Version/Details |
|-------|-----------|----------------|
| Language | C++ | Pre-C++11 (circa 2010-2013) |
| Framework | Qt | Qt 4.x (uses QtWebKit, Phonon, QPlastiqueStyle) |
| Build System | qmake | Qt project file (OpenSankore.pro) |
| GUI | Qt Widgets + QGraphicsView | Custom palette system |
| Web Engine | QtWebKit | For embedded web content |
| Multimedia | Phonon | Audio/video playback |
| XML | Qt XML + XmlPatterns | Document parsing/querying |
| Scripting | Qt Script | JavaScript embedding |
| PDF | Custom (src/pdf, src/pdf-merger) | PDF rendering and merging |
| Compression | QuaZIP (third-party) | ZIP/document archive handling |
| Single Instance | QtSingleApplication (Trolltech) | Prevents multiple instances |
| Crypto | OpenSSL (libcrypto) | Cryptographic operations |
| Parallelism | OpenMP | Image processing acceleration |

## Platform Support

| Platform | SDK/Target | Notes |
|----------|-----------|-------|
| macOS | MacOSX 10.6 SDK, deployment target 10.5 | 32-bit x86 only (QuickTime limitation) |
| Windows | Visual C++ 9 (VS2008) | x86 and x64, QAxContainer for ActiveX |
| Linux | GCC/Clang | X11 integration |

## High-Level Architecture

Open-Sankoré follows a **modified MVC architecture** centered around a QGraphicsScene-based interactive whiteboard:

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                        │
│  UBApplication (QtSingleApplication) - Entry Point           │
│  UBApplicationController - Mode Management (Board/Web/Doc)   │
└─────────────────┬───────────────────────────────────────────┘
                  │
    ┌─────────────┼─────────────┐
    │             │             │
    ▼             ▼             ▼
┌────────┐  ┌─────────┐  ┌──────────┐
│  Board │  │   Web   │  │ Document │
│  Mode  │  │  Mode   │  │   Mode   │
└───┬────┘  └────┬────┘  └────┬─────┘
    │             │             │
    ▼             ▼             ▼
┌────────────────────────────────────────────────┐
│              Domain Layer (Model)               │
│  UBGraphicsScene, UBGraphicsItems, UBDocument  │
└────────────────────────────────────────────────┘
    │
    ▼
┌────────────────────────────────────────────────┐
│           Persistence Layer                     │
│  UBPersistenceManager, Adaptors (SVG/PDF/CFF) │
└────────────────────────────────────────────────┘
```

## Application Modes

The application operates in three primary modes managed by `UBApplicationController`:

1. **Board Mode** - Interactive whiteboard with drawing tools, shapes, media
2. **Internet/Web Mode** - Embedded web browser (QtWebKit) for content
3. **Document Mode** - Document management and organization

## Key Architectural Decisions

1. **QGraphicsView Framework**: All board content is rendered via Qt's QGraphicsView/QGraphicsScene, enabling hardware-accelerated 2D graphics
2. **Singleton Pattern**: Core managers (UBPersistenceManager, UBDrawingController, UBSettings) use singletons
3. **SVG-based Persistence**: Documents are stored as SVG subsets for vector fidelity
4. **Plugin Architecture**: CFF (Common File Format) adaptor as a plugin for IWB interoperability
5. **Adaptor Pattern**: Import/Export operations abstracted through adaptor interfaces
6. **Scene Cache**: UBSceneCache provides lazy loading of document scenes

## Source Code Organization

The project contains 19 source modules under `src/`:

| Module | Purpose |
|--------|---------|
| `core` | Application bootstrap, settings, persistence, download management |
| `board` | Board controller, drawing controller, palette management |
| `domain` | Graphics items, scene, undo commands, shapes |
| `gui` | UI widgets, palettes, toolbar, main window |
| `adaptors` | Import/export adaptors (SVG, PDF, CFF, images) |
| `web` | Web browser, WebKit integration, OEmbed |
| `document` | Document proxy, container management |
| `network` | HTTP operations, cookie management, auto-save |
| `frameworks` | Utility classes (file system, geometry, crypto, platform) |
| `tools` | Ruler, protractor, compass, curtain, cache tools |
| `podcast` | Screen recording (QuickTime/Windows Media/YouTube) |
| `desktop` | Desktop annotation mode |
| `pdf` | PDF rendering |
| `pdf-merger` | PDF document merging |
| `transition` | Uniboard-to-Sankoré data migration |
| `interfaces` | Interface definitions |
| `customWidgets` | Custom Qt widgets |
| `api` | JavaScript API for embedded widgets |
| `globals` | Global definitions |

## Cross-References

- [Components Detail](components.md)
- [Dependencies](dependencies.md)
- [Architectural Patterns](patterns.md)
- [Technical Debt Report](../technical-debt-report.md)
