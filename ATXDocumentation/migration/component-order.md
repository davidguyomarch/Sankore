# Component Migration Order

## Migration Strategy: Bottom-Up

Migration should proceed from the lowest-dependency layers upward, ensuring each layer compiles before moving to the next.

## Phase 1: Foundation Layer (No Internal Dependencies)

### 1.1 frameworks/ module
- **Reason**: No dependencies on other src/ modules
- **Changes**: Replace Qt 4 includes, update platform utilities
- **Blockers**: OpenSSL API compatibility (1.x → 3.x)

### 1.2 interfaces/ module
- **Reason**: Single header, no complex dependencies
- **Changes**: Minimal - update Qt includes only

### 1.3 globals/ module
- **Reason**: Global definitions only
- **Changes**: Update Qt macros

## Phase 2: Core Infrastructure

### 2.1 core/ module (partial - UBSettings, UBSetting)
- **Reason**: Settings used by all modules
- **Changes**: Remove QPlastiqueStyle dependency, update API

### 2.2 network/ module
- **Reason**: Small module, limited dependencies
- **Changes**: Remove QHttp/QFtp, use QNetworkAccessManager throughout

### 2.3 pdf/ and pdf-merger/ modules
- **Reason**: Self-contained PDF operations
- **Changes**: Update Qt includes, verify poppler/PDF library compatibility

## Phase 3: Domain Layer

### 3.1 domain/ module (UBItem, base items)
- **Reason**: Core data model, required by board and adaptors
- **Changes**: Update QGraphicsItem API usage, modernize C++
- **Note**: Start with UBItem, then simple items, then complex ones

### 3.2 tools/ module
- **Reason**: Depends only on domain items and frameworks
- **Changes**: Update geometry calculations, Qt painting API

## Phase 4: Adaptor Layer

### 4.1 adaptors/ module
- **Reason**: Depends on domain items, needed for persistence
- **Changes**: Update XML parsing, SVG handling, ensure all item types serialize correctly
- **Critical**: This is the format compatibility gate

### 4.2 document/ module
- **Reason**: Document proxy and controller
- **Changes**: Update model/view usage

## Phase 5: Controller Layer

### 5.1 core/ module (remaining - UBPersistenceManager, UBApplicationController)
- **Reason**: Core controllers depend on domain + adaptors
- **Changes**: Replace deprecated networking, update threading

### 5.2 board/ module
- **Reason**: Central controller, depends on core + domain + gui
- **Changes**: Update event handling, modernize tool system

## Phase 6: Web & Media Layer (Major Replacements)

### 6.1 web/ module
- **Reason**: Requires QtWebKit → Qt WebEngine replacement
- **Changes**: Complete rewrite of web integration
- **Scope**: Largest single-module effort

### 6.2 podcast/ module
- **Reason**: Requires Phonon → Qt Multimedia replacement
- **Changes**: New encoder backends, new media pipeline

### 6.3 api/ module
- **Reason**: Requires QtScript → QJSEngine replacement
- **Changes**: Widget API bridge rewrite

## Phase 7: GUI Layer

### 7.1 gui/ module
- **Reason**: Largest module, depends on everything else
- **Changes**: Update widget system, palette styling
- **Note**: Should be last as it exercises all other modules

### 7.2 desktop/ module
- **Reason**: Desktop overlay depends on gui + board
- **Changes**: Update platform-specific desktop integration

## Phase 8: Build System & Packaging

### 8.1 Build system migration
- **Action**: Replace OpenSankore.pro with CMakeLists.txt
- **Include**: All module .pri files → CMake targets

### 8.2 Third-party dependency management
- **Action**: Vendor or configure package manager for QuaZIP, SingleApplication replacement

### 8.3 Platform packaging
- **Action**: Update macOS bundle, Windows installer, Linux package scripts

## Dependency-Ordered Compilation Check

```
frameworks ✓
├── interfaces ✓
├── globals ✓
├── core (settings) ✓
├── network ✓
├── pdf, pdf-merger ✓
├── domain ✓
│   ├── tools ✓
│   ├── adaptors ✓
│   ├── document ✓
│   ├── core (full) ✓
│   ├── board ✓
│   ├── web (rewrite) ✓
│   ├── podcast (rewrite) ✓
│   ├── api (rewrite) ✓
│   ├── gui ✓
│   └── desktop ✓
└── plugins/cffadaptor ✓
```

## Cross-References
- [Remediation Plan](../technical-debt/remediation-plan.md)
- [Dependencies](../architecture/dependencies.md)
- [Test Specifications](test-specifications.md)
