---
inclusion: fileMatch
fileMatchPattern: '**/{UBShapeFactory,UBAbstractGraphicsItem,UBAbstractGraphicsPathItem,UBGraphicsRectItem,UBGraphicsEllipseItem,UBGraphicsLineItem,UBEditableGraphicsRegularShapeItem,UBEditableGraphicsPolygonItem,UBGraphicsFreehandItem,UBSmoothStrokeItem,UBGraphicsStrokesGroup,UBBackgroundRenderer,UBInkColorUtils,UBToolController,DrawingPropsBar,StylusPaletteV2,ShapesPaletteV2}*'
---

# Open-Sankoré — Modèle de dessin (traits, formes, couleurs)

Ce steering capture le fonctionnement réel du dessin dans Sankoré : **traits**
(pen/marker), **formes** géométriques, et **recoloration jour/nuit**. Il existe
parce que plusieurs de ces mécanismes sont contre-intuitifs et ont déjà coûté des
allers-retours de débogage (#307, #308, #317, #319). Vérifier ici avant de
modifier le code de dessin.

## Deux familles d'items, à ne PAS confondre

| Famille | Classe(s) | Type() | Couleur | Recolor jour/nuit |
|---------|-----------|--------|---------|-------------------|
| **Trait pen/marker moderne** | `UBSmoothStrokeItem` | `SmoothStrokeItemType` | **paire** light/dark stockée (`mColorOnLight` / `mColorOnDark`) | `applyBackgroundColor(isDark)` rebascule sur la bonne couleur stockée |
| **Trait groupé (ancien)** | `UBGraphicsStrokesGroup` | `UBGraphicsStrokesGroup::Type` (= `UBGraphicsItemType::StrokeItemType`) | paire via `color(colorType)` (`colorOnLightBackground` / `colorOnDarkBackground`) | `setColor(color(reqCol))` |
| **Forme géométrique** | `UBAbstractGraphicsItem` et dérivés | voir table types | **une seule** couleur (pen + brush) | flip du **noir/blanc par défaut** uniquement (`recoloredDefaultInk`) |

Point clé : **un trait pen n'est PAS une forme**. Les traits pen modernes sont des
`UBSmoothStrokeItem` autonomes (jamais enveloppés dans un `UBGraphicsStrokesGroup`).
C'est la cause de #307 (traits invisibles en mode nuit) : la boucle de recolor ne
traitait que les groupes. Et de #308 (gomme qui n'effaçait pas les traits pen) : le
`switch` sur le type ne gérait pas `SmoothStrokeItemType`.

## Hiérarchie des formes

Toutes les formes dérivent de `UBAbstractGraphicsItem`
(`UBItem` + `UBGraphicsItem` + `QAbstractGraphicsShapeItem`). Donc dans une boucle
sur `scene()->items()`, `dynamic_cast<UBAbstractGraphicsItem*>(item)` **réussit**
pour toutes les formes.

| Forme | Classe | `type()` = `UBGraphicsItemType::` |
|-------|--------|-----------------------------------|
| Rectangle / Carré | `UB3HEditableGraphicsRectItem` / `UB1HEditableGraphicsSquareItem` | `GraphicsShapeItemType` |
| Ellipse / Cercle | `UB3HEditableGraphicsEllipseItem` / `UB1HEditableGraphicsCircleItem` | `GraphicsShapeItemType` |
| Ligne | `UBEditableGraphicsLineItem` | `GraphicsShapeItemType` |
| Polygone régulier | `UBEditableGraphicsRegularShapeItem` | `GraphicsRegularPathItemType` |
| Polygone libre | `UBEditableGraphicsPolygonItem` | `GraphicsPathItemType` |
| Main levée (outil forme « Pen ») | `UBGraphicsFreehandItem` | `GraphicsFreehandItemType` |

Les valeurs de `type()` sont `QGraphicsItem::UserType + n` (enum `UBGraphicsItemType`
dans `src/core/UB.h`). Ne pas coder de valeur en dur ; comparer aux constantes.

## PIÈGE MAJEUR : `hasStrokeProperty()` / `hasFillingProperty()`

Sur `UBAbstractGraphicsItem`, `setStrokeColor` / `setStrokeSize` / `setFillColor`
sont **gardés** par `hasStrokeProperty()` / `hasFillingProperty()`. Si le garde est
faux, le setter ne fait **rien, silencieusement**.

Historiquement (avant #317/#319), ces méthodes étaient :

```cpp
bool hasStrokeProperty() const { return pen()   != QPen();   }
bool hasFillingProperty() const { return brush() != QBrush(); }
```

**Bug** : le pen par défaut d'une forme (`SolidLine`, width 1, `Qt::black`, posé par
`initializeStrokeProperty()`) est **égal** au `QPen()` sentinelle. Donc une forme
fraîchement créée renvoyait `hasStrokeProperty() == false` → **tous** les
`setStrokeColor`/`setStrokeSize` étaient ignorés. Ce seul bug cassait à la fois :
- #319 (choisir couleur/épaisseur ne changeait rien),
- #317 (les formes ne basculaient pas en blanc en mode nuit).

**Correctif (en place)** : des flags explicites `mHasStrokeProperty` /
`mHasFillingProperty`, mis à `true` par `initializeStrokeProperty()` /
`initializeFillingProperty()` (que chaque forme appelle dans son constructeur selon
ses capacités), et propagés dans `copyItemParameters()` pour les clones.

**Règle** : ne JAMAIS revenir à un test « valeur != sentinelle par défaut » pour une
capacité. Utiliser un flag explicite. Une valeur légitime peut coïncider avec la
sentinelle.

## Recoloration jour/nuit

Chemin réel du switch (UI V2) :

```
TopBar.qml (bouton lune/soleil)
  → UBAppController::setBackgroundDark()/Light()
  → UBBoardController::changeBackground()/changeBackgroundType()
  → UBGraphicsScene::setBackgroundType()
  → UBBackgroundRenderer::setBackgroundType(isDark, grid)
      └─ SI mDarkBackground change → recolorAllItems()
```

`recolorAllItems()` (`UBBackgroundRenderer.cpp`) itère `scene()->items()` et
distingue par `type()` :
- `UBGraphicsStrokesGroup::Type` → `setColor(color(reqCol))` (paire light/dark)
- `UBSmoothStrokeItem::Type` → `applyBackgroundColor(!isLight)` (paire light/dark)
- sinon `dynamic_cast<UBAbstractGraphicsItem*>` (forme) → flip du noir/blanc **par
  défaut** via `UBInkColors::recoloredDefaultInk`

Piège : `recolorAllItems()` n'est appelé **que si `mDarkBackground` change**
réellement. Un setter qui change le fond sans passer par ce chemin ne recolore rien.

## `UBInkColors::recoloredDefaultInk` (`src/domain/UBInkColorUtils.h`)

Helper **pur** (header only, testable — `tst_UBInkColorUtils`), partagé par le
renderer et la création de forme.

```cpp
QColor recoloredDefaultInk(const QColor& current, bool nowLight);
```

- `nowLight` = état du fond **APRÈS** le flip (`true` = clair). Passer en nuit ⇒
  `nowLight = false`.
- Ne bascule **que** l'encre par défaut : noir ⇄ blanc **purs**
  (`current.rgb() == oldDefault.rgb()`). `rgb()` ignore l'alpha, donc un noir/blanc
  avec alpha reste reconnu.
- Toute couleur choisie par l'utilisateur (rouge, bleu…) est **préservée**.
- Un fill totalement transparent (`alpha == 0`) est laissé tel quel.

Conséquence pour les formes : elles ne stockent **qu'une** couleur (pas de paire
light/dark). Pour éviter le « noir sur noir », `UBShapeFactory::instanciateCurrentShape`
adapte la couleur par défaut au fond **à la création** via ce même helper.

## Création de forme : le flux réel

```
ShapesPaletteV2.qml (clic sur une forme)
  → UBToolController::createShape("rectangle")
      → factory.createRectangle(true)  // pose mShapeType, mIsCreating, setStylusTool(Drawing)
      → m_activeTool = Drawing (14) + emit activeToolChanged()
  → à la souris : UBShapeFactory::onMousePress
      → instanciateCurrentShape()  // crée l'item, applique couleur/épaisseur
      → scene()->addItem(item)     // item DIRECT de la scène (pas de groupe)
```

- `mShapeType` (enum interne `UBShapeFactory`) ≠ `UBStylusTool::Drawing (14)`
  (l'outil actif exposé à QML). Ne pas confondre les deux.
- `instanciateCurrentShape()` relit `mCurrentStrokeColor` et `mThickness` du factory :
  la **prochaine** forme dessinée prend les réglages courants.
- Les setters `setStrokeColor`/`setThickness` du factory n'agissent que sur
  `scene()->selectedItems()` (+ la dernière forme dessinée, cf. #319). La barre de
  propriétés apparaît dès la **sélection de l'outil**, avant tout tracé : le réglage
  doit donc surtout viser les **prochaines** formes, pas une sélection.

## Câblage barre de propriétés (couleur / épaisseur)

`DrawingPropsBar.qml` lie les propriétés **tool-aware** de `UBToolController` :

| QML | Q_PROPERTY UBToolController | En mode Drawing (formes) |
|-----|-----------------------------|--------------------------|
| `currentColors` | `currentColors` | `penColors()` (palette pen **partagée** avec le stylo) |
| `currentColorIndex` | `currentColorIndex` (WRITE `setCurrentColorIndex`) | route vers `shapeFactory().setStrokeColor(palette[index])`, stocke `m_shapeColorIndex` |
| `currentWidthIndex` | `currentWidthIndex` (WRITE `setCurrentWidthIndex`) | mappe 0/1/2 → largeurs px, `shapeFactory().setThickness()`, stocke `m_shapeWidthIndex` |
| visibilité | `showDrawingProps` (NOTIFY `activeToolChanged`) | inclut `Drawing` |

Décision produit : **palette de couleurs partagée** entre stylo et formes (pas de
palette dédiée aux formes). L'index 0 de la palette pen est noir pur (`#000000`) sur
fond clair, blanc pur (`#FFFFFF`) sur fond sombre (`UBSettings.cpp`), ce qui se marie
avec `recoloredDefaultInk`.

## Palette d'outils et sélection de forme (surlignage)

- Chaque bouton de `StylusPaletteV2.qml` est lié à un `id` = valeur de
  `UBStylusTool::Enum` (Pen=0, Eraser=1, Marker=2, … Drawing=14). Le surlignage
  bleu = `activeTool === id`.
- Le bouton **Formes** est un `isToggle` (id `-1`). Ouvrir le menu Formes
  (`toggleShapes()`) **active immédiatement l'outil forme** avec le rond par défaut
  (`createShape("ellipse")`) : le bouton Formes devient bleu tout de suite
  (`activeTool === 14 Drawing`), sans attendre que l'utilisateur choisisse une forme.
- La forme active est exposée par `UBToolController::currentShape` (QString, ex.
  `"ellipse"`), posée par `createShape` et **vidée** quand on quitte l'outil Drawing
  (dans `setStylusTool`). `ShapesPaletteV2.qml` surligne en bleu le bouton dont
  `action === toolController.currentShape` (fond `primary` + icône `onPrimary`).
- Bien distinguer trois états : « palette ouverte » (`shapesVisible`), « outil forme
  actif » (`activeTool === Drawing`), « quelle forme » (`currentShape`).

## Modification d'une forme vs préparation de la prochaine

Après un tracé, la forme est **désélectionnée** (`setSelected(false)` en fin de
`onMouseRelease`, #319). Conséquence voulue :
- changer couleur/épaisseur sur la barre → prépare **la prochaine forme** seulement
  (stocké dans `mCurrentStrokeColor`/`mThickness`, relu par `instanciateCurrentShape`) ;
- modifier une forme existante = action **explicite** : la re-sélectionner avec
  l'outil Sélection (elle entre alors dans `selectedItems()`, que les setters du
  factory ciblent).

Piège à éviter : NE PAS réintroduire un fallback « appliquer à la dernière forme
dessinée » dans `setStrokeColor`/`setThickness`. La forme restant sélectionnée après
tracé, cela recolorait silencieusement la forme précédente (bug rapporté puis corrigé
en #319).

## Diagnostics runtime

Le comportement de dessin (souris, rendu, sélection) n'est pas reproductible en test
unitaire — les classes de forme tirent tout le graphe UI (delegate + frame). Pour
déboguer, ajouter au besoin un helper temporaire qui écrit des logs `[SHAPES]` dans
`startup.log` (motif : `QFile(QCoreApplication::applicationDirPath()+"/startup.log")`
en `Append`, voir le pattern décrit dans `dev-workflow.md`), pousser une branche, et
lire le log renvoyé de la VM Windows. **Retirer ces diagnostics temporaires une fois
le bug corrigé** (les diags #248 ont été nettoyés dans `chore/remove-shapes-diagnostics`).
Voir `dev-workflow.md` → diagnostics `startup.log`.

## Ce qui EST testable en unitaire

- La logique de couleur pure : `UBInkColors::recoloredDefaultInk`
  (`tests/tst_UBInkColorUtils.cpp`).
- Toute logique extraite en header/fonction pure (préférer ce découpage à une méthode
  enfouie dans un `.cpp` non compilé en test).

Ce qui n'est PAS raisonnablement testable : le rendu, l'interaction souris, le
comportement d'un `UBAbstractGraphicsItem` instancié (constructeur = delegate + frame).
Le documenter dans la PR plutôt que de forcer un TU artificiel.
