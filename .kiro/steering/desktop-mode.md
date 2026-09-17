---
inclusion: fileMatch
fileMatchPattern: '**/{UBDesktopAnnotationController,UBCustomCaptureWindow,UBWindowCapture,UBWindowCaptureDelegate_win,DesktopToolbar,UBApplicationController,UBDisplayManager,UBScreenMirror}*'
---
# Open-Sankoré — Mode Desktop annotation (overlay transparent)

Ce steering capture le fonctionnement réel du **mode Desktop annotation** : la
surface transparente plein écran par-dessus le bureau, sa barre d'outils, les
captures d'écran, et le retour au tableau. Depuis #336 la barre d'outils est en
QML V2 (`DesktopToolbar.qml`) et l'ancienne `UBDesktopPalette` a été supprimée.
Vérifier ici avant de toucher au code desktop.

## Qui possède quoi

| Élément | Classe / fichier | Rôle |
|---------|------------------|------|
| Contrôleur du mode | `UBDesktopAnnotationController` (`src/desktop/`) | Possède l'overlay, la scène, la toolbar QML, les captures, le retour au tableau |
| Instance unique | `UBApplicationController::mUninoteController` | Créée une fois ; accessible via `UBApplication::applicationController->uninotesController()` |
| Overlay plein écran | `mTransparentDrawingView` (`UBBoardView`) | Top-level `Qt::Window`, frameless, always-on-top, translucide |
| Scène de l'overlay | `mTransparentDrawingScene` (`UBGraphicsScene`) | `setDrawingMode(true)`, brush transparent |
| Barre d'outils | `mToolbarQml` (`QQuickWidget`) → `DesktopToolbar.qml` | Fenêtre **top-level** (bas-centre) ; overlay = **transient parent** |
| Outil / couleurs | `UBToolController::toolController()` (singleton) | Partagé avec le mode tableau |

## Entrée / sortie du mode

**Entrée** — `UBApplicationController::showDesktop()` :
1. cache + **désactive** la board view (`controlView()->setEnabled(false)`) pour
   éviter des events souris périmés (#135) ;
2. `emit desktopMode(true)` → `changeMode(DESKTOP)` cache les palettes QML V2 du
   tableau (elles sont parentées à `mContainer`, la board view) ;
3. cache la main window, appelle `mUninoteController->showWindow()` ;
4. `toolController()->setInDesktopMode(true)` puis force l'outil Selector.

**`showWindow()`** : affiche la toolbar, applique la transparence sur l'overlay
(Windows : `WA_TranslucentBackground` + brush scène transparent pour voir le vrai
bureau à travers, cf #241), `showFullScreen()` (Linux : `show()` + `updateMask`),
`UBPlatformUtils::setDesktopMode(true)`, remet la toolbar au premier plan.

**Sortie** — le bouton « Retour au tableau » de la toolbar appelle
`UBDesktopAnnotationController::goToUniboard()` :
`hideWindow()` → `setDesktopMode(false)` → `setInDesktopMode(false)` →
`emit restoreUniboard()`, connecté à `UBApplicationController::hideDesktop()`, qui
réactive la board view et appelle `showBoard()/showInternet()/showDocument()`
selon `mMainMode`. `hideWindow()` cache la toolbar + l'overlay et **restaure
l'outil tableau** précédent (`mBoardStylusTool`).

## La barre d'outils QML V2 (`DesktopToolbar.qml`, #336)

Calquée sur `StylusPaletteV2.qml` : `Rectangle` `radius:12`, couleurs 100% via
`themeManager`, icônes Phosphor (`Image` + `ColorOverlay`), tooltips via
`TooltipLabel`. Deux familles de boutons dans un même `Repeater` :

- **`kind:"tool"`** → `toolController.activeTool = id`. Les `id` sont ceux de
  `UBStylusTool::Enum` : Stylo=0, Gomme=1, Marqueur=2, Sélection=3, **Pointeur=8**.
  Le surlignage bleu = `activeTool === id` (comme StylusPaletteV2).
- **`kind:"action"`** → appelle un slot du contrôleur exposé à QML :
  `customCapture()` (capture zone), `screenCapture()` (capture écran plein),
  `goToUniboard()` (retour tableau).
- **`kind:"separator"`** → trait fin `themeManager.border`.

Actions dans l'ordre : Stylo, Gomme, Marqueur, Sélection, Pointeur | Capture
zone, Capture écran | Retour au tableau. Icônes : `pen`, `eraser`,
`highlighter-circle`, `cursor`, `crosshair`, `selection`, `desktop`,
`chalkboard-teacher` (toutes déjà dans `phosphor.qrc`).

### Le pattern d'hébergement (à réutiliser)

**Leçon durement acquise sur la VM Windows (#336, 4 itérations).** Contrairement
aux palettes du tableau (enfants du container **opaque** `mContainer`), la toolbar
desktop **ne peut PAS être un `QQuickWidget` enfant de l'overlay** :

| Approche | Rend sur Windows ? | Reçoit les clics ? |
|----------|--------------------|--------------------|
| Enfant de l'overlay, backing translucide | ❌ | — |
| Enfant de l'overlay, backing **opaque** | ❌ | — |
| Fenêtre top-level séparée | ✅ | ❌ (l'overlay plein écran capte les clics) |
| **Top-level + overlay en transient parent** | ✅ | ✅ |

Un `QQuickWidget` **enfant d'une fenêtre top-level translucide** ne composite
jamais son backing RHI sur Windows (rien peint, même avec `status=Ready`,
`rootObject` présent, `visible=1`). Seule une **fenêtre top-level** peint. Mais
deux fenêtres `WindowStaysOnTopHint` (overlay plein écran vs toolbar) se disputent
le z-order, et `event->ignore()` sur l'overlay ne transfère pas le clic à une
autre fenêtre. Solution qui marche :

```cpp
mToolbarQml = new QQuickWidget(nullptr);                    // TOP-LEVEL
mToolbarQml->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
mToolbarQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
mToolbarQml->setClearColor(Qt::transparent);
mToolbarQml->setAttribute(Qt::WA_TranslucentBackground);    // OK pour un top-level
mToolbarQml->setAttribute(Qt::WA_AlwaysStackOnTop);
mToolbarQml->rootContext()->setContextProperty("themeManager", UBThemeManager::instance());
mToolbarQml->rootContext()->setContextProperty("toolController", UBToolController::toolController());
mToolbarQml->rootContext()->setContextProperty("desktopController", this); // slots capture/retour
mToolbarQml->setSource(QUrl("qrc:/qml/DesktopToolbar.qml"));
// taille fixe + masque arrondi (laisse passer les clics hors du rect arrondi)

// Dans showToolbar(), une fois les deux handles natifs créés :
mToolbarQml->winId();                                       // force la fenêtre native
mToolbarQml->windowHandle()->setTransientParent(
    mTransparentDrawingView->windowHandle());               // ⇒ reste au-dessus + reçoit les clics
```

Positionnée en **coordonnées GLOBALES** (top-level), **bas-centre**, via
`positionToolbar()`, rappelée depuis `onTransparentWidgetResized()` (connecté à
`UBBoardView::resized`). Top-level sans parent QObject ⇒ **supprimée
explicitement** dans le destructeur.

**Slots exposés à QML** : `customCapture`, `screenCapture`, `goToUniboard` sont
des **public slots** du contrôleur → automatiquement invocables depuis QML via le
context object `desktopController` (pas besoin de `Q_INVOKABLE`).

## Captures d'écran

Toutes dans `UBDesktopAnnotationController`, même séquence :
`mIsFullyTransparent=true` + `updateBackground()` → **cacher la toolbar**
(`hideToolbarForCapture()` = `hide()` + `processEvents()`) → capturer → **remontrer
la toolbar** (`restoreToolbarAfterCapture()`) → `emit imageCaptured(pixmap,false)`.

| Méthode | Ce qu'elle fait |
|---------|-----------------|
| `customCapture()` | Zone : ouvre `UBCustomCaptureWindow` sur `getScreenPixmap()`, émet la sélection |
| `screenCapture()` | Plein écran : `getScreenPixmap()` puis émet |
| `windowCapture()` | Fenêtre : `UBWindowCapture` (délégué par plateforme) — **pas câblée à un bouton** de la toolbar V2 |

`getScreenPixmap()` capture l'écran de `mTransparentDrawingView->screen()`
(fallback primary) via `screen->grabWindow(0, ...)`. **Ne pas** rajouter de
`processEvents()` dans le chemin de capture : c'est ce qui recréait la
ré-entrance et le crash #135. `imageCaptured` est connecté à
`UBApplicationController::addCapturedPixmap`.

## Le masque de clics (`desktopPalettePath`)

`UBBoardView::tabletEvent`/event filter consulte
`uninotesController()->desktopPalettePath()` : si le clic tombe dedans, l'overlay
**n'accepte pas** l'event (il laisse le widget de la toolbar le recevoir). Depuis
#336 ce chemin est simplement `mToolbarQml->geometry()` (avant : la géométrie de
l'ancienne `UBDesktopPalette`). Toute nouvelle zone d'UI cliquable posée sur
l'overlay doit être ajoutée ici, sinon elle « dessine à travers ».

Sous Linux, `updateMask(bool)` construit en plus un masque de fenêtre X11
(toolbar + clavier virtuel + annotations) pour le WM ; il est rafraîchi par
`refreshMask()`.

## Pièges (appris pendant #336 / #135 / #241)

- **Hébergement de la toolbar** : fenêtre **top-level** + overlay en **transient
  parent** (voir le tableau plus haut). PAS un `QQuickWidget` enfant de l'overlay
  (ne rend pas sur Windows), PAS une top-level sans transient parent (ne reçoit
  pas les clics).
- **Toujours re-`raise()` la toolbar après `showFullScreen()`** de l'overlay,
  sinon elle passe sous la vue.
- **#135 (ré-entrance)** : ne pas appeler `processEvents()` pendant la transition
  de mode ni dans la capture ; désactiver la board view à l'entrée.
- **#241 (transparence Windows)** : la vraie transparence (voir le bureau) exige
  `WA_TranslucentBackground` sur la vue **et** son viewport + brush de scène
  transparent ; l'ancienne approche « screenshot figé en backgroundBrush » ne
  s'alignait jamais.
- **Sélection d'outil** : passer par `toolController.activeTool = id` (QML) ou
  `setStylusTool(id)` (C++). La couleur/épaisseur est gérée par le
  `DrawingPropsBar` partagé — **ne pas** réintroduire les anciens popups
  pen/marker/eraser à hold-timer (supprimés avec `UBDesktopPalette`).

## Ce qui n'est PAS testable en unitaire

Le mode desktop tire tout le graphe UI (overlay, QQuickWidget, souris, rendu,
capture d'écran, plein écran). Aucun de ces comportements n'est reproductible en
headless. Pour déboguer, instrumenter avec des logs `[TAG]` dans `startup.log` et
lire le retour de la VM (voir `dev-workflow.md` → diagnostics `startup.log`).
Documenter dans la PR pourquoi un TU n'est pas raisonnable plutôt que d'en forcer
un artificiel.
