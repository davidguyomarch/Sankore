---
inclusion: fileMatch
fileMatchPattern: '**/{UBThemeManager,DrawingPropsBar,StylusPaletteV2,ShapesPaletteV2,PageNavigator,TopBar,TooltipLabel,UBFeaturesWidget,UBFeaturesActionBar,UBActionPalette,UBAbstractSubPalette,UBCreateLinkPalette,UBMainWindow,UBKeyboardPalette,check-ui-colors,ui-colors-baseline}*'
---

# Open-Sankoré — Theming des couleurs UI (#297)

Toute couleur de l'UI doit venir du thème (`UBThemeManager`). Objectif de #297 :
zéro littéral de couleur hard-codé dans la couche widget/QML, pour pouvoir
ajouter des thèmes au-delà de clair/sombre. La migration se fait **par lots**
(un widget ou petit groupe par PR), pas en big-bang.

## Source de vérité : `UBThemeManager` (`src/qml/UBThemeManager.h/.cpp`)

Singleton QML + accessible en C++ via `UBThemeManager::instance()`.
Deux thèmes : `applyLightTheme()` / `applyDarkTheme()`. Ajouter un thème = éditer
ce fichier uniquement (c'est le but).

Rôles sémantiques (Q_PROPERTY, `NOTIFY themeChanged`) :

| Rôle | Usage |
|------|-------|
| `surface` / `surfaceVariant` / `surfaceHover` | fonds |
| `onSurface` / `onSurfaceDim` | texte/icônes sur surface |
| `primary` / `onPrimary` | accent (sélection, surlignage bleu) |
| `border` | bordures |
| `disabled` | premier plan désactivé (#297) |
| `error` / `onError` | fond+texte de validation/erreur (#297) |
| `tooltipBase` / `onTooltip` | fond+texte de tooltip (#297) |

**Ne pas** hard-coder de couleur. S'il manque un rôle, l'ajouter à
`UBThemeManager` (les deux thèmes) plutôt qu'écrire un littéral.

## QML

Lier directement : `color: themeManager.surface`, etc. (voir StylusPaletteV2,
PageNavigator, DrawingPropsBar). Les bindings se réévaluent sur `themeChanged`.

## QWidget (widgets legacy) — le cas qui coûte le plus

Les widgets legacy posent des stylesheets QSS avec des littéraux en dur. Pattern
de migration :

1. Construire la feuille de style à partir des couleurs du thème via le helper
   `UBThemeManager::css(QColor)` → rend `#RRGGBB` (opaque) ou `#AARRGGBB`
   (translucide), tous deux acceptés par Qt Style Sheets.
   ```cpp
   auto* tm = UBThemeManager::instance();
   setStyleSheet(QString("background:%1; border:2px solid %2; color:%3;")
                     .arg(UBThemeManager::css(tm->surfaceVariant()),
                          UBThemeManager::css(tm->border()),
                          UBThemeManager::css(tm->onSurface())));
   ```
2. Extraire cette construction dans une méthode privée (ex. `applyTheme()`),
   l'appeler dans le constructeur ET la reconnecter à
   `UBThemeManager::instance()`, `&UBThemeManager::themeChanged` pour re-styler
   quand le thème change.
3. Précédent de référence : le clavier virtuel (`UBKeyboardPalette` /
   `UBKeyboardPaletteColors.h`, #263/#284) dérive ses couleurs du thème et pose
   le contraste WCAG des labels. S'en inspirer pour tout contenu texte.

## Contraste / accessibilité

Pour tout couple texte/fond, viser WCAG AA (ratio ≥ 4.5 pour du texte normal,
≥ 3.0 pour un état/élément large). Les helpers WCAG réutilisables sont dans
`src/gui/UBKeyboardPaletteColors.h` (`contrastRatio`, `relativeLuminance`,
`keyLabelColorFor`) — header pur, déjà utilisé en test. Réutiliser plutôt que
recalculer.

## Garde CI anti-régression (ratchet)

- `scripts/check-ui-colors.py` compte les littéraux de couleur
  (`#RGB`/`#RRGGBB`, `rgb(...)`, `rgba(...)`) sous `src/{gui,qml,board,document}`
  et échoue si le total **dépasse** la baseline commitée
  (`scripts/ui-colors-baseline.json`). Workflow : `ui-colors-check.yml`.
- Le check n'oblige pas à tout migrer d'un coup : il empêche les **nouveaux**
  littéraux et doit **descendre** à chaque lot.
- Après avoir migré un widget : lancer `python3 scripts/check-ui-colors.py --list`
  pour voir ce qui reste, puis **abaisser la baseline** avec
  `python3 scripts/check-ui-colors.py --update-baseline` et committer le
  `ui-colors-baseline.json` mis à jour. La baseline ne doit **jamais** remonter.

## Littéraux qui NE sont PAS des couleurs de thème (à laisser)

Ne pas « thémifier » ni compter comme dette : magic numbers de type de fichier
(`0x5542647A` dans `UBExportDocument*`), gardes de version Qt (`0x040600`…),
contenu de document exporté en SVG (`stop-color:rgb(...)` — c'est du contenu,
pas du chrome). Ces cas sont hors des dossiers scannés ou exclus dans
`check-ui-colors.py`.

## Règle pour Kiro

- Jamais de nouvelle couleur hard-codée dans un widget/QML : ajouter/utiliser un
  rôle `UBThemeManager`.
- Migration = par lots, une PR par widget (ou petit groupe), testable en VM
  (rendu non validable en headless).
- Chaque lot qui supprime des littéraux **abaisse la baseline** et commite le
  fichier baseline mis à jour.
