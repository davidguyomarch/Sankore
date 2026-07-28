# Program Structure

## Top-Level Organization

```
Sankore-3.1/
├── OpenSankore.pro          # Main project file (qmake)
├── src/                     # Source code (19 modules)
│   ├── adaptors/            # Import/Export format adaptors
│   ├── api/                 # JavaScript widget API
│   ├── board/               # Board controller and view
│   ├── core/                # Application core (settings, persistence)
│   ├── customWidgets/       # Custom Qt widgets
│   ├── desktop/             # Desktop annotation mode
│   ├── document/            # Document model and controller
│   ├── domain/              # Graphics scene items and undo
│   ├── frameworks/          # Platform utilities and helpers
│   ├── globals/             # Global definitions
│   ├── gui/                 # UI palettes and main window
│   ├── interfaces/          # Abstract interfaces
│   ├── network/             # HTTP, cookies, download
│   ├── pdf/                 # PDF rendering
│   ├── pdf-merger/          # PDF merging
│   ├── podcast/             # Screen recording
│   ├── tools/               # Geometry tools (ruler, compass, etc.)
│   ├── transition/          # Uniboard→Sankoré migration
│   └── web/                 # Web browser integration
├── plugins/                 # Plugin modules
│   └── cffadaptor/          # CFF/IWB format plugin
├── resources/               # Assets, forms, translations, icons
└── tools/                   # Build/packaging scripts
```

## Class Hierarchy - Domain Items

```
QGraphicsItem (Qt)
├── QGraphicsObject
│   ├── UBGraphicsPixmapItem : UBItem, UBGraphicsItem
│   ├── UBGraphicsSvgItem : UBItem, UBGraphicsItem
│   ├── UBGraphicsMediaItem : UBItem, UBGraphicsItem
│   │   ├── UBGraphicsVideoItem
│   │   └── UBGraphicsAudioItem
│   └── UBGraphicsGroupContainerItem : UBItem, UBGraphicsItem
├── QGraphicsRectItem
│   ├── UBGraphicsRectItem
│   └── UBGraphicsPDFItem : UBItem, UBGraphicsItem
├── QGraphicsEllipseItem
│   └── UBGraphicsEllipseItem
├── QGraphicsLineItem
│   └── UBGraphicsLineItem
├── QAbstractGraphicsShapeItem
│   ├── UBGraphicsPolygonItem : UBItem, UBGraphicsItem
│   └── UBGraphicsFreehandItem
├── QGraphicsTextItem
│   └── UBGraphicsTextItem : UBItem, UBGraphicsItem
├── QGraphicsProxyWidget
│   ├── UBGraphicsProxyWidget
│   └── UBAbstractGraphicsProxyWidget
└── QGraphicsWidget
    └── UBGraphicsWidgetItem : UBItem, UBGraphicsItem
```

## Class Hierarchy - Controllers

```
QObject
├── UBApplication (extends QtSingleApplication)
├── UBApplicationController
├── UBDocumentContainer
│   └── UBBoardController
├── UBDrawingController (singleton)
├── UBPersistenceManager (singleton)
├── UBDocumentManager
├── UBDisplayManager
├── UBWebController
├── UBPodcastController
├── UBDesktopAnnotationController
├── UBFeaturesController
└── UBBoardPaletteManager
```

## Class Hierarchy - Undo Commands

```
QUndoCommand
└── UBAbstractUndoCommand
    ├── UBGraphicsItemUndoCommand
    ├── UBGraphicsItemTransformUndoCommand
    ├── UBGraphicsItemGroupUndoCommand
    ├── UBGraphicsTextItemUndoCommand
    ├── UBDocumentUndoCommand
    └── UBPageSizeUndoCommand
```

## Class Hierarchy - GUI Palettes

```
QWidget
├── UBFloatingPalette
│   ├── UBStylusPalette
│   ├── UBZoomPalette
│   ├── UBNavigatorPalette
│   ├── UBClockPalette
│   ├── UBStartupHintsPalette
│   └── UBActionPalette
│       ├── UBDrawingPalette
│       ├── UBShapesPalette
│       ├── UBRegularShapesPalette
│       └── UBFavoriteToolPalette
├── UBDockPalette
│   ├── UBLeftPalette
│   └── UBRightPalette
├── UBDockPaletteWidget
│   ├── UBDockDownloadWidget
│   ├── UBDockResourcesWidget
│   └── UBDockTeacherGuideWidget
└── UBMainWindow (extends QMainWindow)
```

## Class Hierarchy - Tools

```
QGraphicsItem / QGraphicsRectItem
├── UBAbstractDrawRuler
│   ├── UBGraphicsRuler
│   ├── UBGraphicsTriangle
│   ├── UBGraphicsProtractor
│   ├── UBGraphicsAristo
│   └── UBGraphicsCompass
├── UBGraphicsCurtainItem (screen mask)
└── UBGraphicsCache (screen spotlight)
```

## Namespace and Global Enumerations (`src/core/UB.h`)

Key enumerations:
- `UBStylusTool::Enum` - Pen, Marker, Eraser, Selector, Play, Hand, ZoomIn, ZoomOut, Pointer, Line, Text, Capture
- `itemLayerType::Enum` - Object layering categories
- `UBFeatureBackgroundDisposition` - Center, Adjust, Mosaic, Fill, Extend

## Cross-References
- [Components](../architecture/components.md)
- [Interfaces](interfaces.md)
- [Data Models](data-models.md)
- [Modules](modules.md)
