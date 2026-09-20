---
inclusion: fileMatch
fileMatchPattern: 'src/qml/**'
---
# Open-Sankoré — Interface QML V2 : composants partagés et pièges

Ce steering capture le fonctionnement de l'UI QML V2 (barres d'outils, palettes)
et surtout les **pièges QML** qui ont coûté plusieurs cycles VM. À lire avant de
toucher à un `.qml` ou d'ajouter un composant partagé. Le rendu QML ne se valide
**pas** au build ni aux tests headless : une erreur ne se voit qu'au runtime
(souvent seulement sur la VM Windows). Toujours lancer `qmllint` (voir
`dev-workflow.md` → Étape 1c).

## Barres d'outils : `UBToolbar` + `UBToolButton` + `ToolbarSeparator`

- **`UBToolbar.qml`** : coquille partagée (Rectangle thémé + `Row` + `Repeater` +
  delegate). **Aucune** dépendance controller. Paramètres :
  - `property var model` : descripteurs de boutons ;
  - `property bool isVertical` ;
  - `property var isActive(row)` / `property var isPrimary(row)` : prédicats de
    surlignage fournis par l'appelant ;
  - `signal buttonClicked(row)` : l'appelant décide de l'action.
- **`StylusPaletteV2.qml`** (tableau) et **`DesktopToolbar.qml`** (desktop) sont
  de **fines instances** de `UBToolbar` : elles ne fournissent que `model`,
  `isActive`/`isPrimary` et `onButtonClicked`. Toute logique de layout/rendu vit
  dans `UBToolbar` — ne pas la dupliquer.
- **Schéma de modèle unifié** : `{ kind: "tool"|"toggle"|"action"|"separator",
  id, icon, tooltip, action }`. `tool` → `toolController.activeTool = id` ;
  `toggle` → bascule (Formes) ; `action` → slot exposé (`desktopController`…) ;
  `separator` → trait.
- **`UBToolButton.qml`** : le bouton (fond thémé, icône Phosphor `Image
  visible:false` + `ColorOverlay`, hover/active, indicateur, tooltip). Le parent
  décide `active`/`primaryHighlight` et réagit à `clicked()`.

## Piège n°1 — `required property` + composant `.qml` chargé par un delegate

Un composant défini dans un **fichier `.qml` séparé** (ex. `UBToolButton`)
instancié par un `Repeater`/`Loader` **ne voit PAS** les propriétés du delegate
par résolution de scope, et une `required property` n'est **pas** satisfaite par
une propriété homonyme du delegate/Loader. Symptômes vécus (#351) :
- `required property var toolData` sur le composant chargé → le composant **ne
  s'instancie jamais** → barre vide, « décalage » des outils, bouton Formes mort,
  et même le footer d'une autre palette cassé (chargement QML compromis).
- passer la donnée par un `toolData` **nu** (non qualifié) → `undefined`
  (« Unable to assign [undefined] to bool »).

**Règle** : passer la donnée de ligne par une **référence qualifiée**
(`del.modelData`, l'`id` du delegate), jamais par scope nu ni par `required
property` sur le composant chargé. Lire tout flag de modèle avec `=== true` pour
qu'un flag absent devienne un vrai `false` (pas `undefined`).

## Piège n°2 — effets graphiques et backend software (VM sans GPU)

La VM de test fait tourner le build x64 **sous émulation, sans GPU** → Qt Quick
bascule sur son **backend software**, qui **n'exécute pas** les effets
shader/layer. Conséquence : `ColorOverlay` (Qt5Compat), `MultiEffect`
(QtQuick.Effects) et `layer.effect` peuvent ne **rien peindre** alors que les
`Rectangle` simples s'affichent. Les SVG se décodent bien (plugin `qsvg` déployé,
`QImage` les lit) — le problème est la composition GPU, pas la ressource.

**Règle** : pour un rendu qui doit marcher partout, ne pas dépendre d'un effet
GPU sophistiqué. Le rendu d'icône actuel (`Image visible:false` + `ColorOverlay`)
fonctionne ; avant de le remplacer par un autre effet, se rappeler que le vrai
blocage historique était le Piège n°1 (instanciation), pas l'effet lui-même.
Instrumenter/valider sur la VM avant de conclure.

## Piège n°3 — noms réservés et collisions de type

- **Ne pas nommer une propriété comme un nom réservé/attaché QML** (`primary`,
  `parent`, `data`, `state`, `visible`…). Vécu (#351) : une propriété `primary`
  → « Cannot assign to non-existent property "primary" », le fichier ne charge
  pas. Préfixer (`primaryHighlight`).
- **Ne pas nommer un composant du projet comme un type built-in** de
  QtQuick/Controls (`ToolButton`, `Button`, `Label`, `Slider`, `ToolBar`,
  `Rectangle`, `Text`…). L'import explicite `QtQuick.Controls` prime sur l'import
  implicite du répertoire → le nom résout vers le built-in, `qmllint` signale
  « Could not find property … » pour toutes les propriétés custom. Préfixer :
  `UBToolButton`, `UBToolbar`. (`ToolbarSeparator` est OK — pas de type built-in
  de ce nom.)

## Warnings `qmllint` à ignorer / à corriger

- **À corriger (bloquant runtime)** : `non-existent property`, `Could not find
  property`, `Cannot assign`, `reserved`, `Type … not found`, syntaxe.
- **À ignorer** : `Unqualified access` sur les *context properties* injectées en
  C++ (`themeManager`, `toolController`, `pageController`, `appController`,
  `desktopController`) — invisibles de `qmllint` ; et `Unused import`.

## Context properties (injectées en C++)

| Barre / palette | Hébergée par | Context properties |
|-----------------|--------------|--------------------|
| StylusPaletteV2, TopBar, PageNavigator, DrawingPropsBar, ShapesPaletteV2 | `UBBoardPaletteManager` | `themeManager`, `toolController`, `pageController`, `appController` |
| DesktopToolbar, DrawingPropsBar (desktop) | `UBDesktopAnnotationController` | `themeManager`, `toolController`, `desktopController` |

Les slots publics d'un context object (ex. `desktopController.customCapture()`)
sont invocables depuis QML **sans** `Q_INVOKABLE`.

## Ce qui n'est PAS validable en headless

Le rendu réel, l'interaction souris, la composition (translucide/effets) ne sont
pas reproductibles en headless. Le smoke test offscreen valide le **chargement**
(`status=1`, absence d'erreur de binding) mais pas l'affichage. Valider le visuel
sur la VM ; instrumenter avec des logs `[TAG]`/`[ICON]` dans `startup.log` en cas
de doute (voir `dev-workflow.md`).
