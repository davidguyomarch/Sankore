# Dependencies

## External Framework Dependencies

### Qt Modules (Qt 4.x)

| Module | Usage | Status |
|--------|-------|--------|
| QtGui | Core UI framework | Stable (Qt4) |
| QtWebKit | Embedded web browser, widget rendering | **Removed in Qt 5.6+** |
| QtSvg | SVG rendering/parsing | Available |
| QtNetwork | HTTP, FTP, cookies | Available |
| Phonon | Audio/video playback | **Removed in Qt 5** |
| QtXml | XML DOM parsing | Available |
| QtScript | JavaScript engine for widgets | **Deprecated in Qt 5** |
| QtXmlPatterns | XQuery/XPath support | **Removed in Qt 6** |
| QAxContainer (Windows) | ActiveX embedding | Windows-only |

### Third-Party Libraries

| Library | Location | Purpose |
|---------|----------|---------|
| QuaZIP | `../Sankore-ThirdParty/quazip/` | ZIP compression for .ubz documents |
| QtSingleApplication | `../Sankore-ThirdParty/trolltech/singleapplication/` | Single instance enforcement |
| OpenSSL (libcrypto) | System library | Cryptographic operations (UBCryptoUtils) |
| libX11 | System (Linux only) | X11 window management |

### Platform SDKs

| Platform | SDK | Notes |
|----------|-----|-------|
| macOS | MacOSX 10.6 SDK | Deployment target: 10.5 |
| macOS | Foundation framework | Obj-C bridge |
| Windows | Visual Studio 2008 (VC9) | MSVC compiler |
| Linux | Standard GCC/Clang toolchain | With OpenMP support |

## Internal Module Dependencies

### Dependency Graph (top-down)

```
UBApplication (core)
├── UBApplicationController (core)
│   ├── UBBoardController (board)
│   │   ├── UBGraphicsScene (domain)
│   │   ├── UBBoardView (board)
│   │   ├── UBBoardPaletteManager (board)
│   │   ├── UBShapeFactory (domain)
│   │   ├── UBDocumentContainer (document)
│   │   └── UBFeaturesController (board)
│   ├── UBWebController (web)
│   │   └── QtWebKit classes
│   ├── UBDocumentController (gui)
│   └── UBDesktopAnnotationController (desktop)
├── UBPersistenceManager (core)
│   ├── UBSvgSubsetAdaptor (adaptors)
│   ├── UBSceneCache (core)
│   └── UBDocumentTreeModel (core)
├── UBDocumentManager (core)
│   ├── UBImportAdaptor* (adaptors)
│   └── UBExportAdaptor* (adaptors)
├── UBSettings (core)
└── UBMainWindow (gui)
    └── All palette widgets (gui)
```

### Module Coupling Analysis

| From Module | To Module | Coupling Type |
|-------------|-----------|---------------|
| board → domain | High | Direct scene/item manipulation |
| board → core | High | Settings, persistence, application access |
| board → gui | Medium | Palette manager, action palette |
| core → domain | Medium | Scene creation/loading |
| adaptors → domain | High | Item serialization/deserialization |
| gui → core | Medium | Settings, controller access |
| gui → domain | Low | Item display only |
| web → core | Low | URL handling, settings |
| podcast → board | Medium | Screen capture from board view |
| network → core | Low | Download management |
| frameworks → (none) | None | Utility layer, no upward dependencies |

## Build System Dependencies

### qmake Configuration
- **Project file**: `OpenSankore.pro` (TEMPLATE = app)
- **Third-party path**: `../Sankore-ThirdParty` (relative, external)
- **Include mechanism**: `.pri` files per module
- **Build output**: `build/{platform}/{debug|release}/`

### Compiler Flags
- **macOS**: `-fopenmp`, `-gdwarf-2`, `-mdynamic-no-pic`, x86 only
- **Windows**: `/MP` (multi-process), `/Od /Zi` (debug in release)
- **Linux**: `-fopenmp`

## Plugin Dependencies

### CFF Adaptor Plugin (`plugins/cffadaptor/`)
- **Purpose**: Common File Format (IWB interoperability standard)
- **Integration**: Compiled into the main application via `plugins.pri`
- **Resources**: Has own `.qrc` resource file

## Resource Dependencies

| Resource | Path | Purpose |
|----------|------|---------|
| Forms (.ui) | `resources/forms/` | 10 UI forms (mainWindow, preferences, documents, etc.) |
| Translations | `resources/i18n/` | 36+ language translations |
| Icons/Assets | `resources/` | Application resources via sankore.qrc |
| Library shapes | `resources/library/shape/` | Default shapes |
| Fonts | `resources/fonts/` | Bundled fonts |
| etc config | `resources/etc/` | Configuration files |

## Cross-References
- [System Overview](system-overview.md)
- [Components](components.md)
- [Dependency Analysis](../analysis/dependency-analysis.md)
- [Outdated Components](../technical-debt/outdated-components.md)
