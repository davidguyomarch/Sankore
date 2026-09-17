---
inclusion: fileMatch
fileMatchPattern: '**/{tests.pro,docker-build.sh,build-linux,build-linux-arm64,tst_*,main.cpp,*_stub,*_testable}*'
---
# Open-Sankoré — moc & premoc (contournement du bug moc Linux)

Ce steering explique **pourquoi** le dossier `tests/premoc/` existe et **comment**
ajouter une classe `QObject` sous test sans casser le CI. C'est un piège
récurrent : un test peut passer en local (Docker) et **échouer au CI Linux** si
son moc pré-généré n'est pas commité. Lire avant de toucher à `tests/tests.pro`,
`scripts/docker-build.sh`, aux workflows Linux, ou d'ajouter un `tst_*`.

## Le problème racine

`moc` (le Meta-Object Compiler de Qt) génère le code des `QObject` (signals,
slots, `Q_PROPERTY`, `Q_OBJECT`). Sous Linux, **le moc système ne peut pas
parser les headers système GCC** quand qmake lui passe `--include moc_predefs.h`
(qui tire `<type_traits>` etc.) : il sort « No relevant classes found » ou plante.
Le build applicatif (le gros `qmake`/`make`) n'est pas affecté ; **seul le target
de test** (`tests/tests.pro`) l'est, car il compile des `QObject` de test et des
sources `QObject` sous test.

## La solution : pré-générer le moc « à la main », sans `moc_predefs.h`

Pour chaque `QObject` du target de test, on lance `moc` manuellement **sans**
`--include moc_predefs.h`, et on écrit le résultat dans `tests/premoc/moc_*.cpp`.
`tests.pro` compile ces fichiers `premoc/` au lieu des `build/moc/` (vides).

Trois familles de fichiers concernées :
- une **source `QObject` sous test** (ex. `UBLibraryController.cpp`) → `premoc/moc_UBLibraryController.cpp`
- un **stub/shim `QObject`** (ex. `UBBlackoutWidget`) → `premoc/moc_UBBlackoutWidget.cpp`
- une **classe de test** avec des `private slots` (tout `tst_*.h`) → `premoc/moc_tst_*.cpp`

## DEUX chemins, une divergence à connaître (le vrai piège)

| Contexte | Ce qui génère le premoc |
|----------|-------------------------|
| **Local** (`scripts/docker-build.sh`) | **Régénère** tous les premoc listés dans ses deux boucles, à chaque build, avant `make`. |
| **CI Linux** (`build-linux.yml`, `build-linux-arm64.yml`) | Ne régénère **que** `UBFileSystemUtils` et `UBOEmbedParser`. Pour **tout le reste**, il utilise les fichiers **commités** dans `tests/premoc/`. |

Conséquence directe et contre-intuitive :

> **Si tu ajoutes un `QObject` de test mais n'ajoutes pas son `premoc/moc_*.cpp`
> commité, le build passe en local (docker-build.sh le régénère) et ÉCHOUE au CI
> (fichier absent, symboles moc manquants au link).**

C'est pourquoi les ~40+ fichiers `tests/premoc/moc_*.cpp` sont **suivis par git**.
`tests/premoc/` n'est **pas** dans `.gitignore` — c'est volontaire.

## Checklist — ajouter un QObject sous test (ou un `tst_*`)

Pour chaque nouveau `QObject` (source sous test, stub, ou classe de test) :

1. **`tests/tests.pro`** — compiler la vraie source (ou le stub) et déclarer le
   moc. Modèle à copier (bloc « Library » #258) :
   ```pro
   SOURCES += ../src/controllers/UBLibraryController.cpp
   win32-msvc* {
       HEADERS += ../src/controllers/UBLibraryController.h   # MSVC : auto-moc
   } else {
       SOURCES += premoc/moc_UBLibraryController.cpp          # Linux : premoc
   }
   ```
   Pour une classe de test, ajouter `tst_Foo.cpp` aux `SOURCES` et le bloc
   `win32-msvc* / else premoc/moc_tst_Foo.cpp`.

2. **`scripts/docker-build.sh`** — ajouter la génération du moc :
   - une source/stub `QObject` : ajouter un appel `$MOC_BIN $MOC_COMMON_FLAGS <header>.h -o premoc/moc_<name>.cpp` ;
   - une classe de test `tst_Foo` : l'ajouter aux **deux** boucles `for HEADER in … tst_… ; do`
     (la boucle de **génération** ET la boucle de **patch du Makefile**).

3. **`tests/main.cpp`** — `#include "tst_Foo.h"` et ajouter le bloc d'exécution :
   ```cpp
   { TestFoo test; status |= QTest::qExec(&test, argc, argv); }
   ```
   (sinon la suite compile, linke… mais **ne s'exécute jamais**).

4. **Générer et COMMITER le premoc** : lancer `./scripts/docker-build.sh`
   (il crée `tests/premoc/moc_*.cpp`), puis `git add tests/premoc/moc_<...>.cpp`.
   **Ne pas oublier ce commit** — c'est ce qui fait passer le CI Linux.

5. **Régénérer** un `premoc/moc_tst_*.cpp` si on **ajoute/retire des slots** dans
   un `tst_*.h` déjà existant (sinon la vtable moc est périmée).

## MSVC (Windows) — pas de premoc

Sur Windows/MSVC, moc fonctionne normalement : on met le header dans `HEADERS +=`
et qmake auto-moc. D'où le pattern `win32-msvc* { HEADERS += … } else { SOURCES += premoc/… }`
partout dans `tests.pro`.

## Détails d'implémentation (pour référence)

- `docker-build.sh` régénère `moc_predefs.h` pour l'archi courante, puis **retire
  les `-I` système** des commandes moc du `Makefile` (sed sur `libexec/moc`) —
  c'est l'autre moitié du contournement.
- Les flags moc communs sont dans `MOC_COMMON_FLAGS` (défini dans le script).
- Le build applicatif principal n'utilise **pas** premoc : c'est spécifique au
  target de test. Ne pas introduire de premoc côté app.
- Couverture : les `premoc/`, `moc_*`, `stubs/` et `tst_*` sont exclus du calcul
  lcov (voir `docker-build.sh`).

## Règle pour Kiro

- Ajouter un `QObject` au target de test = suivre la checklist **en entier**, et
  **committer le `tests/premoc/moc_*.cpp` généré**. Un premoc manquant ne se voit
  pas en local (docker-build.sh le régénère) mais casse le CI Linux.
- Ne jamais ajouter `tests/premoc/` à `.gitignore`.
- En cas d'échec de link « undefined reference to `…::qt_metacall` / `staticMetaObject` »
  au CI Linux alors que le local passe : c'est presque toujours un `premoc/moc_*.cpp`
  non commité ou périmé.
