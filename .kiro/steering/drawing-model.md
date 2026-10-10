---
inclusion: fileMatch
fileMatchPattern: '**/{UBShapeFactory,UBAbstractGraphicsItem,UBAbstractGraphicsPathItem,UBGraphicsRectItem,UBGraphicsEllipseItem,UBGraphicsLineItem,UBEditableGraphicsRegularShapeItem,UBEditableGraphicsPolygonItem,UBGraphicsFreehandItem,UBSmoothStrokeItem,UBGraphicsStrokesGroup,UBBackgroundRenderer,UBInkColorUtils,UBToolController,UBSettings,UBSceneContext,UBColorPickerDialog,UBColorPicker,UBDrawingStrokePropertiesPalette,UBDrawingFillPropertiesPalette,UBTextDelegateDialogHandler,DrawingPropsBar,StylusPaletteV2,ShapesPaletteV2}*'
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

## Remplissage des formes (outil pot / `ChangeFill`, #429/#436)

Le remplissage est un **axe distinct** du trait, longtemps resté inerte. Points clés :

- **Le pot n'a pas de couleur par défaut** : `mCurrentFillFirstColor` du factory vaut
  `Qt::transparent` et `mFillType == Transparent` à la construction. Donc, sans
  câblage, cliquer une forme avec le pot la remplit… en transparent (rien de
  visible). C'était le bug #429.
- **Il faut poser couleur + type AVANT d'utiliser le pot.**
  `UBToolController::activateFillTool()` pose `setFillType(Full)` +
  `setFillingFirstColor(palette[m_shapeFillColorIndex])` puis `setActiveTool(ChangeFill)`.
- **Index de remplissage SÉPARÉ du trait.** `m_shapeFillColorIndex` ≠
  `m_shapeColorIndex`. En mode `ChangeFill`, les propriétés tool-aware
  (`currentColors`/`currentColorIndex`/`setCurrentColorIndex`) routent vers la couleur
  de **remplissage** (pas le trait) ; la `DrawingPropsBar` masque le groupe
  **épaisseurs** (`isFill`, `activeTool === 13`) et montre les couleurs seules.
  `showDrawingProps()` inclut `ChangeFill`.
- **Pas de recolor rétroactif** : choisir une couleur en mode pot vise le **prochain**
  clic de pot (cohérent avec le reste, cf. #319). Le remplissage effectif passe par
  `UBBoardView` (branche `ChangeFill`) → `shapeFactory().changeFillColor(pos)` →
  `applyCurrentStyle` sur la forme cliquée.
- **Curseur** : tout outil doit avoir son cas dans `UBBoardView::setToolCursor`,
  sinon il tombe sur le **curseur pen** par défaut (bug vécu : le pot montrait le
  stylo). `ChangeFill` → `fillCursor` (construit depuis `:/icons/phosphor/paint-bucket.svg`
  dans `UBResources`, motif identique à `ocrCursor`).

> Rappel du piège sélection (voir plus haut) : `setStrokeColor`/`setFillingFirstColor`/
> `updateFillingPropertyOnSelectedItems` n'agissent que sur `scene()->selectedItems()`,
> **vide par défaut** après tracé (#319). Les boutons « Contour »/« Aligner » de
> `ShapesPaletteV2` (#430/#431) sont donc **inertes sans modèle de sélection** (#366) :
> le fil QML est correct, c'est la sélection qui manque. Ne pas « corriger » en
> réintroduisant un fallback « dernière forme ».

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

## Architecture des couleurs (#475) — à lire avant TOUTE évolution couleur

Le traitement de la couleur de dessin repose sur **deux pipelines distincts** plus
**un canal de couleur libre** et **un sélecteur UI unifié**. Toute nouvelle
fonctionnalité ou correction autour des couleurs DOIT suivre ce pattern — ne pas
réintroduire de `QColorDialog` ad hoc ni de nouveau canal de résolution couleur.

### Les deux pipelines (ne pas les confondre)

| | **Stylo / Marqueur (traits)** | **Formes / Remplissage / Texte** |
|---|---|---|
| Stockage | palette **indexée** + paire jour/nuit (`UBSettings`) | `QColor` simple (pas d'index, pas de paire) |
| Résolution | `index → liste.at(index)` via `UBSettings::penColor(onDark)` / `markerColor(onDark)` | couleur directe poussée dans `UBShapeFactory` |
| Point de lecture au tracé | `UBSceneContext::penColorOn{Light,Dark}Background()` lu par `UBInputRouter::inputDevicePress` → baké dans `UBSmoothStrokeItem` (paire) | `UBShapeFactory::instanciateCurrentShape` relit `mCurrentStrokeColor`/`mCurrentFillFirstColor` |
| Alpha marqueur | **bakée dans le stockage** (0.5 via `boardMarkerAlpha`, posée par `UBColorListSetting`) | — |

Conséquence structurante : **la couleur d'un trait pen/marker ne vient QUE de la
pastille sélectionnée** (il n'existe pas de « couleur directe » dans ce pipeline,
contrairement aux formes). C'est pourquoi une couleur libre a besoin d'un canal
dédié (ci-dessous).

### Le canal de couleur libre transitoire (#475)

Pour appliquer une couleur **arbitraire** au stylo/marqueur **sans écraser** une des
4 pastilles (décision produit = option B) :

- **Stockage** : `UBSettings::mTransientPenColor` / `mTransientMarkerColor`
  (`QColor` invalide = pas d'override). **Non persisté** (session uniquement).
- **Écriture** : `setTransientPenColor()` force **alpha 1.0** (stylo opaque) ;
  `setTransientMarkerColor()` force **alpha = `boardMarkerAlpha`** (surligneur
  translucide). C'est indispensable : `UBSmoothStrokeItem::paint` décide
  « est-ce un marqueur ? » **uniquement** sur `alphaF() < 1` — une couleur libre
  marqueur sans alpha serait peinte comme un stylo opaque.
- **Résolution** : `penColor()` / `markerColor()` renvoient l'override s'il est
  valide, sinon la pastille. Donc `UBSceneContext` et `UBInputRouter` le lisent
  **sans aucun changement** — le canal se branche au seul point de résolution.
- **Paire jour/nuit** : une couleur libre n'a qu'une valeur → elle est renvoyée
  **identique** sur light et dark (préservée au flip jour/nuit, exactement comme
  une couleur utilisateur de forme). Un override jour/nuit « intelligent » n'est
  PAS fait ici (piste pour #494, au bureau, via luminance).
- **Reset** : `UBToolController::setCurrentColorIndex` (clic sur une pastille)
  **efface** l'override → retour à la pastille. `currentColorIndex()` renvoie
  **-1** quand un override est actif (aucune pastille surlignée dans la
  `DrawingPropsBar`).

**Règle** : pour toute nouvelle couleur libre d'un trait, passer par ce canal
(`setTransient*Color`), jamais par un écrasement de slot de palette (ça, c'est
l'édition explicite des pastilles — #492, voir ci-dessous).

### Le sélecteur UI unifié : `UBColorPickerDialog::pick()`

`src/gui/UBColorPickerDialog.h` — **header-only** (namespace + `inline`, pas de
moc, à la `UBIconUtils`), donc appelable depuis les contrôleurs QML **et** les
delegates C++ sans câblage moc/premoc.

```cpp
QColor c = UBColorPickerDialog::pick(initial, parentWidget, /*withAlpha*/ true, tr("…"));
if (c.isValid()) { /* appliquer */ }   // invalide == annulé
```

Il centralise ce qui était dupliqué dans **6 sites** : parenting, canal alpha,
garde de lisibilité thème sombre (`setStyleSheet("background-color: white;")`),
opt-out dialogue natif macOS. Les 6 sites migrés :
`UBGraphicsItemDelegate::pickFillColour`, `UBTextDelegateDialogHandler` (texte +
fond), `UBDrawingStrokePropertiesPalette`, `UBDrawingFillPropertiesPalette`
(1re + 2de couleur).

**Règle** : tout nouveau choix de couleur arbitraire passe par
`UBColorPickerDialog::pick()`. **Ne jamais** réintroduire un `QColorDialog` brut
(sinon on recrée la divergence parenting/alpha/thème que #475 a supprimée).

### Surface publique couleur de `UBToolController`

- Lecture tool-aware pour la `DrawingPropsBar` : `currentColors()`,
  `currentColorIndex()` (**-1** si couleur libre active), `currentToolColor()`.
- `setCurrentColorIndex(index)` : route selon l'outil (Drawing → stroke factory ;
  ChangeFill → fill factory ; Marker/Pen → index de palette + **clear override**).
- `pickCustomColor()` (Q_INVOKABLE, bouton « couleur perso » de la
  `DrawingPropsBar`) : ouvre le picker, applique selon l'outil (Drawing/ChangeFill
  → factory ; Pen/Marker → `setTransient*Color`).
- `setPenColor(onDark, color, index)` / `setMarkerColor(onDark, color, index)` :
  **écrasent un slot de palette** (persisté) et émettent `colorPaletteChanged`.
  C'est l'édition **explicite** d'une pastille — à distinguer du canal transitoire.

### Règles pour Kiro (évolutions/bugs couleur)

1. **Trait pen/marker, couleur arbitraire** → canal transitoire
   (`setTransient*Color`), jamais écraser un slot sauf édition explicite demandée.
2. **Forme / remplissage / texte, couleur arbitraire** → pousser la `QColor`
   directement dans `UBShapeFactory` / le delegate (pas d'index).
3. **Choisir une couleur dans l'UI** → `UBColorPickerDialog::pick()`. Jamais de
   `QColorDialog` nu.
4. **Alpha marqueur** : toute couleur marqueur (slot OU libre) doit porter
   l'alpha marqueur, sinon elle se peint en opaque (détection par `alphaF() < 1`).
5. **Paire jour/nuit** : les traits portent une paire (préservée par
   `applyBackgroundColor`) ; les formes ne flippent que l'encre par défaut
   (`recoloredDefaultInk`) ; une couleur libre est stable au flip (même valeur).
6. **Thème UI** (chrome : boutons, pastilles de la `DrawingPropsBar`) → couleurs
   `UBThemeManager` uniquement (voir `ui-theming.md`), **jamais** de littéral.
   Ne pas confondre avec la couleur d'**encre** (contenu de scène), qui elle vient
   du modèle couleur ci-dessus.

### Ce qui EST testable / évolutions ouvertes

- Testable en TU : la logique de résolution pure (ex. l'override transitoire
  renvoyé par `penColor()`/`markerColor()`, le forçage d'alpha) si on l'exerce via
  `UBSettings` sans graphe UI. Le rendu réel du trait et l'ouverture du picker
  restent **VM-only** (non headless).
- **#492** : édition explicite des 4 pastilles (long-press/clic-droit → picker →
  `setPenColor`/`setMarkerColor`). Suit le pattern : réutilise le picker unifié et
  les setters de slot persistés.
- **#494** : au bureau, choisir la variante jour/nuit du trait depuis la luminance
  réelle du bureau (et non le flag de la page board). N'affecte que le choix de
  **variante de pastille** — la couleur libre, elle, n'a pas de paire.
