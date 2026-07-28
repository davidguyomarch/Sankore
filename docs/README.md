# Open-Sankoré — Documentation technique

## Table des matières

1. [Présentation du projet](#présentation-du-projet)
2. [Stack technique](#stack-technique)
3. [Architecture du projet](#architecture-du-projet)
4. [Structure des répertoires](#structure-des-répertoires)
5. [Modules source (src/)](#modules-source)
6. [Système de build](#système-de-build)
7. [Système de plugins](#système-de-plugins)
8. [Ressources](#ressources)
9. [Internationalisation](#internationalisation)
10. [Support multi-plateforme](#support-multi-plateforme)
11. [Compilation](#compilation)
12. [Licence](#licence)

---

## Présentation du projet

**Open-Sankoré** est un logiciel libre de tableau numérique interactif (TNI) destiné à l'enseignement. Il permet aux enseignants de créer et d'animer des présentations interactives sur tableau blanc interactif ou écran.

| Champ | Valeur |
|-------|--------|
| Version | 2.5.1 |
| Licence | GNU GPL v3 (avec exception de liaison OpenSSL) |
| Copyright | 2010-2013 GIP ENA (Groupement d'Intérêt Public pour l'Éducation Numérique en Afrique) |
| Langage | C++ |
| Framework | Qt 4.8 |

### Fonctionnalités principales

- Annotation et dessin sur tableau blanc interactif
- Outils géométriques (règle, compas, rapporteur, équerre)
- Import/export de documents (PDF, CFF, SVG, images, vidéos)
- Navigateur web intégré
- Bibliothèque de ressources (animations, widgets, images, sons, vidéos)
- Enregistrement de séances (podcast)
- Mode annotation bureau (desktop)
- Clavier virtuel
- Gestion de documents multi-pages
- Undo/Redo complet

---

## Stack technique

| Composant | Technologie |
|-----------|-------------|
| Langage | C++ |
| Framework GUI | Qt 4.8 (QtGui, QtWebKit, QtSvg, QtNetwork, Phonon, QtXml, QtScript, QtXmlPatterns) |
| Build system | qmake (.pro / .pri) |
| Multimédia | Phonon (audio/vidéo) |
| PDF | XPDF (rendu), pdf-merger (manipulation) |
| Compression | QuaZip (gestion ZIP) |
| Cryptographie | OpenSSL |
| Instance unique | QtSingleApplication |
| Installeur Windows | Inno Setup |
| Installeur macOS | DMG (dmgutil) |
| Installeur Linux | .deb (Debian/Ubuntu) |

---

## Architecture du projet

L'application suit un pattern **MVC** (Modèle-Vue-Contrôleur) avec une architecture modulaire :

```
┌─────────────────────────────────────────────────────┐
│                   UBApplication                      │
│              (QtSingleApplication)                   │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  │
│  │ UBBoard      │  │ UBDocument   │  │ UBWeb    │  │
│  │ Controller   │  │ Controller   │  │ Controller│  │
│  └──────┬───────┘  └──────┬───────┘  └────┬─────┘  │
│         │                  │               │        │
│  ┌──────▼───────┐  ┌──────▼───────┐  ┌────▼─────┐  │
│  │ UBGraphics   │  │ UBDocument   │  │ Web      │  │
│  │ Scene/View   │  │ Proxy        │  │ View     │  │
│  └──────────────┘  └──────────────┘  └──────────┘  │
│                                                     │
├─────────────────────────────────────────────────────┤
│  Adaptors (Import/Export) │ Frameworks │ Network    │
├─────────────────────────────────────────────────────┤
│  Plugins (CFF Adaptor)    │ PDF Merger │ Tools      │
└─────────────────────────────────────────────────────┘
```

### Composants clés

- **UBApplication** — Singleton principal, hérite de `QtSingleApplication` (une seule instance autorisée)
- **UBBoardController** — Gère la logique du tableau (dessin, outils, pages)
- **UBDocumentController** — Gère les documents (création, ouverture, navigation)
- **UBWebController** — Gère le navigateur web intégré
- **UBGraphicsScene** — Scène graphique Qt contenant tous les éléments du tableau
- **UBApplicationController** — Gère les transitions entre les modes (board, document, web)
- **UBSettings** — Configuration et préférences de l'application
- **UBPersistenceManager** — Persistance des documents

---

## Structure des répertoires

```
Open-Sankore/
├── src/                    # Code source principal
├── plugins/                # Plugins (CFF Adaptor)
├── resources/              # Ressources de l'application
│   ├── forms/              # Fichiers UI Qt Designer
│   ├── i18n/               # Traductions (40+ langues)
│   ├── images/             # Icônes et images (SVG, PNG)
│   ├── fonts/              # Polices PostScript
│   ├── library/            # Bibliothèque de contenus
│   ├── etc/                # Fichiers de configuration
│   ├── macx/               # Ressources macOS
│   ├── linux/              # Ressources Linux
│   └── sankore.qrc         # Fichier de ressources Qt
├── OpenSankore.pro         # Fichier projet principal
├── buildDebianPackage.sh   # Script de build Linux
├── release.macx.sh         # Script de build macOS
├── release.win7.vc9.bat    # Script de build Windows
├── LICENSE.txt             # Licence GPLv3
└── docs/                   # Documentation (ce répertoire)
```

---

## Modules source

Le répertoire `src/` est organisé en modules indépendants, chacun avec son fichier `.pri` :

| Module | Rôle |
|--------|------|
| `core/` | Initialisation, application, settings, point d'entrée (`main.cpp`) |
| `board/` | Contrôleur du tableau, vue, dessin, gestion des pages |
| `gui/` | Interface utilisateur (fenêtre principale, palettes, dialogues) |
| `domain/` | Éléments graphiques (items, scènes, strokes, delegates) |
| `document/` | Gestion des documents (proxy, container, contrôleur) |
| `adaptors/` | Import/export (PDF, CFF, SVG, web, métadonnées) |
| `frameworks/` | Utilitaires (plateforme, fichiers, crypto, géométrie) |
| `web/` | Navigateur web intégré |
| `network/` | Opérations réseau (téléchargements, HTTP) |
| `pdf/` | Rendu PDF (basé sur XPDF) |
| `pdf-merger/` | Manipulation et fusion de PDF |
| `podcast/` | Enregistrement vidéo et encodage de séances |
| `tools/` | Outils géométriques (règle, compas, rapporteur, équerre) |
| `desktop/` | Mode annotation bureau |
| `api/` | API pour widgets et bibliothèque |
| `transition/` | Migration Uniboard → Sankoré |
| `interfaces/` | Définitions d'interfaces |
| `customWidgets/` | Widgets Qt personnalisés |
| `globals/` | Définitions globales |

### Fichiers source clés

| Fichier | Description |
|---------|-------------|
| `src/core/main.cpp` | Point d'entrée, gestion des messages, logging |
| `src/core/UBApplication.h/cpp` | Classe application principale |
| `src/core/UB.h` | Enums globaux (outils, types MIME, couches) |
| `src/core/UBSettings.h/cpp` | Gestion des paramètres |
| `src/board/UBBoardController.h/cpp` | Logique du tableau |
| `src/board/UBBoardView.h/cpp` | Rendu du tableau |
| `src/domain/UBGraphicsScene.h/cpp` | Scène graphique principale |
| `src/gui/UBMainWindow.h/cpp` | Fenêtre principale |

---

## Système de build

### Fichier projet principal : `OpenSankore.pro`

Le projet utilise **qmake** avec un système modulaire de fichiers `.pri` :

```qmake
TARGET = "Open-Sankore"
TEMPLATE = app
QT += webkit svg network phonon xml script xmlpatterns

# Modules inclus
include(src/core/core.pri)
include(src/board/board.pri)
include(src/gui/gui.pri)
# ... (18 modules au total)

# Plugins
include(plugins/plugins.pri)

# Dépendances tierces
include($THIRD_PARTY_PATH/quazip/quazip.pri)
include($THIRD_PARTY_PATH/trolltech/singleapplication/qtsingleapplication.pri)
```

### Versionnement

```
VERSION_MAJ = 2
VERSION_MIN = 5
VERSION_TYPE = r    # a=alpha, b=beta, r=release
VERSION_PATCH = 1
```

### Répertoires de build

La sortie est organisée par plateforme et configuration :

```
build/
├── macx/
│   ├── debug/
│   └── release/product/    # Application finale
├── win32/
│   ├── debug/
│   └── release/product/
└── linux/
    ├── debug/
    └── release/product/
```

### Dépendance externe

Le projet dépend d'un répertoire tiers externe :

```
THIRD_PARTY_PATH = ../Sankore-ThirdParty
```

Ce répertoire doit contenir :
- QuaZip (gestion ZIP)
- QtSingleApplication
- Bibliothèques spécifiques à la plateforme (`libs.pri`)

---

## Système de plugins

### Architecture

Le système de plugins est intégré au build via `plugins/plugins.pri`. Actuellement, un seul plugin est implémenté :

### CFF Adaptor (`plugins/cffadaptor/`)

Gère l'import/export au format **CFF** (Common File Format / IWB format) :

```
plugins/cffadaptor/
├── UBCFFAdaptor.pro        # Projet du plugin
├── src/
│   ├── UBCFFAdaptor.h/cpp  # Adaptateur principal
│   ├── UBCFFAdaptor_global.h
│   ├── UBCFFConstants.h    # Constantes du format
│   └── UBGlobals.h         # Globales du plugin
└── resources/              # Ressources du plugin
```

---

## Ressources

### Formulaires UI (`resources/forms/`)

Fichiers Qt Designer pour l'interface :

| Fichier | Description |
|---------|-------------|
| `mainWindow.ui` | Fenêtre principale |
| `preferences.ui` | Dialogue de préférences |
| `brushProperties.ui` | Propriétés du pinceau |
| `documents.ui` | Gestionnaire de documents |
| `trapFlash.ui` | Capture Flash |
| `youTubePublishingDialog.ui` | Publication YouTube |

### Bibliothèque de contenus (`resources/library/`)

```
library/
├── animations/       # Animations interactives
├── applications/     # Applications/widgets
├── audios/           # Fichiers audio
├── interactivities/  # Activités interactives
├── pictures/         # Images
├── search/           # Moteur de recherche
├── shape/            # Formes géométriques
└── videos/           # Vidéos
```

### Images (`resources/images/`)

Plus de 100 fichiers (SVG et PNG) pour l'interface : icônes d'outils, curseurs, palettes de couleurs, arrière-plans.

---

## Internationalisation

L'application supporte **40+ langues** via le système Qt Linguist :

| Code | Langue | Code | Langue |
|------|--------|------|--------|
| ar | Arabe | it | Italien |
| bg | Bulgare | ja | Japonais |
| ca | Catalan | ko | Coréen |
| cs | Tchèque | nb | Norvégien |
| da | Danois | nl | Néerlandais |
| de | Allemand | pl | Polonais |
| el | Grec | pt | Portugais |
| en | Anglais | ro | Roumain |
| es | Espagnol | ru | Russe |
| fi | Finnois | sk | Slovaque |
| fr | Français | sv | Suédois |
| fr_CH | Français (Suisse) | tr | Turc |
| hi | Hindi | zh | Chinois |
| hu | Hongrois | zh_CN | Chinois simplifié |
| iw | Hébreu | zh_TW | Chinois traditionnel |

Les fichiers de traduction (`.ts`) sont dans `resources/i18n/` et compilés en `.qm` lors du build.

---

## Support multi-plateforme

### Windows

- **Compilateur** : Visual Studio 2008 (VC9), 32-bit et 64-bit
- **Qt** : 4.8 (répertoire `../Qt-4.8`)
- **Installeur** : Inno Setup (`Sankore 3.1.iss`)
- **Script** : `release.win7.vc9.bat`
- **Spécificités** : QAxContainer (ActiveX), compilation parallèle (`/MP`)

### macOS

- **Architecture** : Intel 32-bit (x86) — limitation QuickTime
- **SDK** : MacOSX 10.6
- **Déploiement** : OS X 10.5+
- **Installeur** : DMG via `dmgutil`
- **Script** : `release.macx.sh`
- **Spécificités** : Framework Foundation, OpenSSL, Breakpad (crash reporting)

### Linux

- **Distributions** : Debian/Ubuntu
- **Architectures** : i386, amd64
- **Package** : .deb
- **Script** : `buildDebianPackage.sh`
- **Dépendances** : Qt 4.8, bibliothèques système
- **Note** : Installer `ttf-mscorefonts-installer` pour les polices web

---

## Compilation

### Prérequis

1. **Qt 4.8** installé (avec les modules webkit, svg, phonon, xml, script, xmlpatterns)
2. **Sankore-ThirdParty** cloné au même niveau que le projet :
   ```
   parent/
   ├── Open-Sankore/          # Ce dépôt
   └── Sankore-ThirdParty/    # Dépendances tierces
   ```
3. Compilateur C++ adapté à la plateforme

### Build rapide

```bash
# Générer le Makefile
qmake OpenSankore.pro

# Compiler
make -j$(nproc)
```

### Build release par plateforme

**macOS :**
```bash
./release.macx.sh
```

**Linux (paquet .deb) :**
```bash
./buildDebianPackage.sh
```

**Windows :**
```batch
release.win7.vc9.bat
```

### Sortie

L'exécutable final se trouve dans :
```
build/<plateforme>/release/product/
```

---

## Licence

Open-Sankoré est distribué sous **GNU General Public License v3** avec une exception de liaison spécifique pour la bibliothèque OpenSSL.

```
Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
l'Éducation Numérique en Afrique (GIP ENA)

Open-Sankoré is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License,
with a specific linking exception for the OpenSSL project's
"OpenSSL" library.
```

Le texte complet de la licence est disponible dans `LICENSE.txt` et `COPYING`.
