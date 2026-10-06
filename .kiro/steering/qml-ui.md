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

## Piège n°4 — curseur d'outil manquant → curseur pen par défaut

`UBBoardView::setToolCursor(int tool)` est un `switch` sur `UBStylusTool::Enum`
avec un `default` qui pose le **curseur pen**. **Tout nouvel outil doit avoir son
cas** dans ce switch (et un `QCursor` dans `UBResources`), sinon il affiche le
curseur stylo — symptôme vécu : l'outil Remplissage (`ChangeFill`) montrait le
stylo (#436). Un curseur se construit depuis un SVG comme `ocrCursor`/`fillCursor`
(`QCursor(QPixmap(":/icons/phosphor/<name>.svg").scaled(32,32,...), hotX, hotY)`).

## Piège n°5 — icône Phosphor absente du build → tuile vide

Une icône référencée en QML (`qrc:/icons/phosphor/<name>.svg`) mais **absente de
`phosphor.qrc`** ne charge pas (tuile vide, `Image.status === Error`), sans erreur
de build ni de `qmllint`. Avant d'utiliser une icône : vérifier qu'elle est dans
`resources/icons/phosphor/`. Si manquante, la **télécharger depuis le dépôt
officiel** (`https://raw.githubusercontent.com/phosphor-icons/core/main/assets/regular/<name>.svg`)
et l'ajouter au `phosphor.qrc` **en ordre alphabétique** — jamais la dessiner à la
main (vécu cette session : `arrow-left`, `file`, pour #258). Le même SVG sert aussi
de curseur (piège n°4).

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

## Menus contextuels des objets du tableau (modèle de capacités, ADR-0010)

Les objets du tableau (formes, traits, texte, image, SVG, PDF, widget, média,
groupe) n'ont **pas** de menu au clic droit natif. Un objet sélectionné affiche
un **cadre** (`UBGraphicsDelegateFrame`) avec des boutons flottants
(Supprimer / Dupliquer / « … » / Z-ordre) ; le bouton **« … »** ouvre le QMenu
construit par `UBGraphicsItemDelegate::decorateMenu()`. Détail complet et décision
dans **l'ADR-0010** — ce qui suit est le réflexe à avoir avant d'y toucher.

- **Le menu est piloté par une matrice de capacités**, pas par des flags épars.
  La composition et l'**ordre** des entrées de base viennent de la fonction pure
  `UBItemMenu::baseMenuEntries(caps)` (`src/domain/UBItemCapabilities.h`), qui est
  **testée en TU** (`tst_UBItemCapabilities`). Pour changer quelles entrées
  existent ou leur ordre, c'est **là** qu'on édite — et on met à jour le TU.
- **Un type déclare ses capacités en un seul endroit** via un profil pur
  (`UBItemMenu::forShape()` / `forImage()` / `forPdf()` / …), que son constructeur
  applique par `Delegate()->applyMenuCapabilities(UBItemMenu::forXxx())`
  **après `init()`**. Ne pas revenir aux anciens setters menu épars
  (`setHorizontalMirror`/`setVerticalMirror`/`setCanTrigAnAction`/
  `setCanReturnInCreationMode`) dans les constructeurs : passer par le profil.
- **Ajouter un type d'objet** : écrire un `forXxx()` (+ son test), l'appliquer dans
  le constructeur après `init()`. **Ajouter une entrée de menu** : ajouter une
  valeur à l'enum `Entry`, la gater dans `baseMenuEntries()`, la mapper vers un
  `QAction` dans `decorateMenu()`, l'exposer via un champ de `Capabilities`.
- **Flags à NE PAS migrer vers le profil** (double usage ou runtime) :
  `setFlippable`/`setRotatable` (pilotent aussi le cadre), `setCanDuplicate`
  (bouton Dupliquer, et **PDF le pose à `false` AVANT `init()`** car `init()`
  construit les boutons). Mutations runtime à préserver : le **groupe** recalcule
  flippable/rotatable depuis ses enfants (`addToGroup`/`removeFromGroup`),
  `setAction()` force l'entrée « Link an action », le widget `setOwnFolder` pose
  rotatable. Ces comportements ne sont pas exprimables en profil statique.
- **Flip = `horizontalMirror || flippable`** : les formes utilisent les flags
  mirror, image/SVG/traits utilisent `flippable` — les deux montrent Flip. Ne pas
  fusionner ces deux champs.
- **Icônes de menu** : via le helper `UBGraphicsItemDelegate::themedMenuIcon()`
  (SVG Phosphor reteinté au thème `onSurface`, lisible sur menu clair/sombre).
  Les sous-delegates (texte « Editable », widget « Frozen »/« Transform as Tool »)
  **appellent la base** puis ajoutent leurs entrées — ne pas réimplémenter la base
  (piège historique du groupe, corrigé #455).
- **Non validable en headless** : le rendu réel du menu et l'instanciation d'un
  vrai item (constructeur = delegate + frame) restent VM-only. Seule la logique de
  capacités (`baseMenuEntries` + profils `forXxx()`) est testable en TU.
