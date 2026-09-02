# Open-Sankoré — Workflow de développement

## Mode PROD — Règles de travail

### Flow complet

```
Issue → Branche → Code → Build local → Tests local → Push → PR → CI passe → Test Windows VM → Squash merge → Branche supprimée
```

**Master est toujours stable.** On ne push jamais directement sur master. Tout passe par une branche + PR.

### 1. Tout travail est lié à une issue GitHub

- On ne commence JAMAIS un travail sans issue GitHub associée.
- Si l'issue n'existe pas, on la crée d'abord avec un titre clair, une description du problème/besoin, et un label (bug/enhancement).
- **Langue : tout ce qui est sur GitHub est en anglais** — issues, titres de PR, messages de commit, commentaires, labels. Les conversations avec le développeur restent en français.
- Référence : https://github.com/davidguyomarch/Sankore/issues

### 2. Nommage des branches

Chaque issue a sa branche dédiée, créée depuis master :

```
fix/<issue-id>-<description-courte>     # pour les bugs
feat/<issue-id>-<description-courte>    # pour les fonctionnalités
chore/<description-courte>              # pour docs, steering, CI, nettoyage (pas de code app)
```

Exemples :
- `fix/135-desktop-crash`
- `feat/134-documents-qml-view`
- `fix/133-capture-cursor`
- `chore/update-steering`
- `chore/ci-concurrency`

**Docs/steering/CI** peuvent être inclus dans un commit de feature si c'est lié au même travail. Si ce sont des modifications isolées (pas de code app), utiliser une branche `chore/` dédiée pour éviter de déclencher des builds inutiles sur une branche feature.

Commande de création :
```bash
git checkout master && git pull
git checkout -b fix/135-desktop-crash
git push -u origin fix/135-desktop-crash
```

### 3. Vérification de branche

À chaque reprise de travail, Kiro vérifie qu'il est sur la bonne branche :
```bash
git branch --show-current
```
Si la branche ne correspond pas à l'issue en cours, switcher avant de faire quoi que ce soit.

### 4. Commits

Chaque commit référence l'issue ID :
```
fix(#135): description courte du changement
feat(#134): description courte du changement
```

### 5. Push et PR

Quand le travail est prêt à tester :
1. Valider localement en Docker (build + tests + couverture — voir boucle de développement ci-dessous)
2. Commiter avec le bon format `fix(#ID)` / `feat(#ID)`
3. Pousser la branche (jamais master)
4. Créer la PR immédiatement — **le CI Windows ne tourne que sur les PRs** (pas sur les branches seules)
5. Le développeur vérifie que le CI passe (Windows + Linux)
6. Le développeur teste sur la VM Windows si nécessaire

```bash
gh pr create \
  --base master \
  --title "fix(#135): description du fix" \
  --body "## Changes

- Description des changements

## Local validation
- [x] Docker Linux ARM64 build passes
- [x] Unit tests pass (17 suites, 0 failed)
- [x] Coverage: XX.X% lines

## CI validation
- [ ] CI Windows passes
- [ ] CI Linux x64 passes
- [ ] CI Linux ARM64 passes
- [ ] Tested on VM Windows

Closes #135"
```

### 6. Merge — uniquement sur demande explicite du développeur

Kiro ne merge JAMAIS de sa propre initiative. Quand le développeur valide :

```bash
gh pr merge --squash --delete-branch
```

Cela fait automatiquement :
- Squash merge (1 seul commit propre sur master)
- Suppression de la branche distante
- Fermeture de l'issue (grâce à `Closes #ID` dans le body de la PR)

### 7. Nettoyage local après merge

```bash
git checkout master && git pull
git branch -d <branche-locale>
```

---

## Publier une release (piloté par le tag)

**Le tag git `vX.Y.Z` est la seule source de vérité pour la version.** On ne
bump aucun numéro de version à la main, on ne fait aucun commit de préparation :
poser le tag suffit.

### Poser une release

Sur le commit de master à publier :

```bash
git tag v4.3.0 <commit-de-master>   # ou juste `git tag v4.3.0` sur HEAD
git push origin v4.3.0
```

Le push du tag déclenche `release.yml`, qui fait tout :

1. Dérive la version `4.3.0` du nom du tag.
2. Dispatche `build-windows.yml`, `build-linux.yml`, `build-linux-arm64.yml`
   **sur le tag** (`gh workflow run --ref v4.3.0`), attend leur succès, et
   récupère les artefacts. On ne réutilise jamais un build de branche/master :
   il porterait la version dev `0.0.0`.
3. Lance `scripts/check-sbom.py --set-version 4.3.0`, qui écrit la version dans
   `OpenSankore.pro` et `sbom.spdx.json` **dans le checkout du runner
   uniquement** (jamais commité).
4. Publie la GitHub Release avec les `.exe`/`.zip`/`.deb`/`.rpm`/`.tar.gz` +
   le SBOM, tous étiquetés `4.3.0`.

### D'où vient la version à chaque étage

| Consommateur | Source de la version |
|--------------|----------------------|
| Binaire (`UBVERSION`, `.deb`/`.rpm`/`.exe`) | `GITHUB_REF_NAME` (le tag) → patché dans `OpenSankore.pro` par chaque workflow de build |
| Installer Windows (`installer.iss`) | Réécrit depuis le tag par `build-windows.yml` |
| SBOM (`sbom.spdx.json`) | `check-sbom.py --set-version` dans `release.yml` |
| Nom des assets de la release | `github.ref_name` dans `release.yml` |

### Fallback dev (builds non taggés)

Entre deux releases, `OpenSankore.pro` et `sbom.spdx.json` portent `0.0.0`
(affiché `0.0.0-dev`). C'est volontaire : un build de PR/master n'est pas une
release officielle. `UBBoardController` affiche d'ailleurs l'avertissement
« This is not a final release » dans ce cas. **Ne jamais hand-bumper ces
valeurs pour une release** — pousser le tag suffit.

### Règles pour Kiro

- **Ne jamais** modifier un numéro de version dans les fichiers du repo pour
  faire une release. La version vient du tag.
- Poser/pousser un tag `v*` est une opération de release : ne le faire que sur
  **demande explicite** du développeur (même règle que le merge).
- Le SBOM commité doit rester cohérent avec `OpenSankore.pro` (le CI
  `sbom-check.yml` le vérifie). Après un changement de version du `.pro`,
  resynchroniser avec `python3 scripts/check-sbom.py --set-version X.Y.Z`.
- Tests de la mécanique de version : `python3 scripts/test_check_sbom.py`.

---

## Boucle de développement (Build → Test → Validate → Push)

Le développeur travaille sur **macOS ARM (M4 Pro)**. Il n'y a **pas de compilation locale Windows**.

### Images Docker

| Image | Base | Qt | Usage |
|-------|------|-----|-------|
| `sankore-dev` | Ubuntu 25.04 | Qt 6.8.3 | Build + tests + couverture (ARM64 natif) |
| `sankore-dev-x64` | Ubuntu 25.04 | Qt 6.8.3 | Build + tests x64 via QEMU |
| `sankore-qt6` | Ubuntu 24.04 | Qt 6.4 | Legacy — build app uniquement (pas de tests) |

Pour construire les images (une seule fois) :
```bash
docker build -f Dockerfile.dev -t sankore-dev .
docker build -f Dockerfile.dev --platform linux/amd64 -t sankore-dev-x64 .
```

### Script principal : `scripts/docker-build.sh`

Point d'entrée unique pour toute validation locale :

```bash
# Workflow complet ARM64 natif : build app + tests + couverture (~3 min)
./scripts/docker-build.sh

# Build app uniquement (~2 min incrémental)
./scripts/docker-build.sh --build-only

# Tests + couverture uniquement (app déjà buildée)
./scripts/docker-build.sh --test-only

# Build x64 via QEMU (~10-15 min — plus lent)
./scripts/docker-build.sh --x64

# Supprimer les objets compilés et relancer
./scripts/docker-build.sh --clean

# Sans couverture (plus rapide)
./scripts/docker-build.sh --no-coverage
```

### Boucle complète

```
Code → docker-build.sh (3 min) → Push + PR → CI Windows (25 min) → VM Windows (test manuel)
         ↑  build ✓                                                          |
         ↑  tests ✓ (17 suites)                                              |
         ↑  coverage ✓ (83%+)                                                |
         └───────────── Kiro corrige ← startup.log ←────────────────────────┘
                                                                    ↓
                                                              Merge master
```

### Étape 1 : Validation locale complète — Docker (~3 min)

Avant tout push, lancer le workflow complet :

```bash
./scripts/docker-build.sh
```

Ce script fait dans l'ordre :
1. **Build app** avec `--coverage` flags (qmake + make)
2. **Build tests** (qmake tests.pro + pré-génération des moc + make)
3. **Run tests** (17 suites QTest en mode offscreen)
4. **Couverture** (lcov capture + filtrage + rapport)

**Critères de validation** :
- ✅ Build app réussit (pas d'erreur de compilation)
- ✅ Build tests réussit (link OK)
- ✅ **0 tests failed** — tout test en échec doit être corrigé avant push
- ✅ Couverture ≥ 80% lignes sur le code sous test
- ⚠️ La couverture peut baisser temporairement si on ajoute du code source sans tests associés

**Ce que ça valide** : syntaxe C++, résolution de symboles, moc, QML dans les .qrc, logique métier via tests unitaires
**Ce que ça NE valide PAS** : compilation MSVC, runtime Windows, comportement visuel, interaction souris

### Étape 1b : Smoke test local — Docker (~10 sec)

Après compilation, on peut lancer l'app en headless pour valider le démarrage et lire `startup.log` :

```bash
docker run --rm -v $(pwd):/src -w /src sankore-dev bash -c '
  rm -f /src/build/linux/release/product/startup.log
  export QT_QPA_PLATFORM=offscreen
  timeout 10 ./build/linux/release/product/Open-Sankore --quit-after=5 2>&1
  echo ""; echo "=== STARTUP LOG ==="; echo ""
  cat /src/build/linux/release/product/startup.log 2>/dev/null || echo "NO LOG"
'
```

- **Ce que ça valide** : démarrage de l'app, chargement des QML (status=1 = OK), positions des widgets, outil actif, visibilité des palettes
- **Ce que ça NE valide PAS** : rendu visuel, interaction souris, comportement réel des outils
- **Durée** : ~10 secondes

Ce qu'il faut vérifier dans le log :
- `ToolController activeTool=0` → Pen sélectionné au démarrage
- `LeftPalette: visible=0` / `RightPalette: visible=0` → Dock palettes cachées
- `status=1` pour tous les QML widgets → Pas d'erreur de parsing QML
- Aucune ligne `ERROR:` → Pas de crash QML

### Étape 2 : Push branche + PR + CI

Pousser la branche et créer la PR immédiatement. Le CI Windows ne tourne que sur les PRs :

```bash
git push
gh pr create --base master --title "fix(#ID): description" --body "..."
```

Le CI (`build-windows.yml`, `build-linux.yml`, `build-linux-arm64.yml`) tourne sur la PR. Vérifier que les builds passent avant de demander un test Windows.

### Étape 3 : Test sur VM Windows — deploy + run-test.bat

Le développeur déploie l'artefact CI de la PR et le teste. Le script `deploy-latest.sh` sélectionne automatiquement le dernier build Windows réussi et signale les builds en cours ou les échecs plus récents :

```bash
# Par PR (résout la branche automatiquement)
./scripts/deploy-latest.sh --pr 212

# Par branche
./scripts/deploy-latest.sh --branch fix/135-desktop-crash

# Par run ID explicite
./scripts/deploy-latest.sh <run-id>

# Dernier build réussi sur master (défaut)
./scripts/deploy-latest.sh
```

Puis dans la VM Windows :
```
Z:\sankore-install\run-test.bat
```

Le script `run-test.bat` :
1. Tue toute instance en cours
2. Copie les fichiers dans `C:\Sankore` (requis — l'émulation x64 ne charge pas les DLLs depuis un share réseau)
3. Supprime l'ancien `startup.log`
4. Lance `Open-Sankore.exe`
5. Affiche le contenu complet de `startup.log` après fermeture/crash

### Étape 4 : Merge

Quand le CI passe et le test Windows est OK :
1. Le développeur valide
2. Kiro merge (`gh pr merge --squash --delete-branch`)
3. Nettoyage local (`git checkout master && git pull && git branch -d <branche>`)

### Étape 5 : Analyse des logs (si bug)

Après le test, le développeur copie le contenu de `startup.log` dans la conversation Kiro.
Kiro utilise ces logs pour diagnostiquer les bugs et proposer des corrections.

Le log est aussi lisible depuis le Mac (si le partage VirtIO fonctionne) :
```bash
cat ../sankore-install/startup.log
```
Mais préférer la lecture depuis la VM pour fiabilité : `type C:\Sankore\startup.log`

---

## Système de diagnostic : startup.log

### Principe

`startup.log` est un fichier texte écrit à côté de `Open-Sankore.exe`.
Il sert de canal de communication entre l'app qui tourne sur la VM et Kiro qui analyse les logs.

### Ce qui est loggé

| Source | Ce qui est écrit | Fichier |
|--------|-----------------|---------|
| `main.cpp` | Version, date, OS, Qt, infos crash (exception handler) | Au démarrage + au crash |
| `UBBoardPaletteManager.cpp` | Positions et tailles de tous les widgets QML, erreurs QML, état des palettes | Après setupDockPaletteWidgets + containerResized |

### Format typique

```
=== Open-Sankore Startup ===
Build: 4.1.0
Date: 2026-08-28T10:00:00
Qt: 6.8.3
OS: Windows 11 (10.0.26100)

=== QML UI V2 Widget Positions ===
Container: 1920x1040
StylusPalette: pos=860,968 size=200x52 visible=1
TopBar: pos=0,0 size=1920x48 visible=1
PageNav: pos=0,48 size=180x940 visible=1
PropsBar: pos=820,912 size=280x48 visible=1
LeftPalette: vis=0 size=0x0
RightPalette: vis=0 size=0x0
ToolController activeTool=0
===================================
```

### Comment ajouter des diagnostics

Pour déboguer un problème spécifique, ajouter des logs dans le code C++ :

```cpp
// Pattern standard — écrire dans startup.log
{
    QFile logFile(QCoreApplication::applicationDirPath() + "/startup.log");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "\n[MON_TAG] info=" << maVariable << "\n";
        logFile.close();
    }
}
```

Conventions :
- **Tag entre crochets** : `[SHAPES TOGGLE]`, `[TOOL CHANGE]`, `[COLOR INDEX]`, etc.
- **Append mode** : toujours `QIODevice::Append` (sauf le premier write dans main.cpp qui est `WriteOnly`)
- **Fermer le fichier** après chaque write (pas de handle global)
- Les logs de diagnostic temporaires peuvent être supprimés une fois le bug corrigé

### Crash handler (Windows)

`main.cpp` installe un `SetUnhandledExceptionFilter` qui écrit dans `startup.log` :
- Le code d'exception (ex: `0xc0000005` = access violation)
- L'adresse fautive
- Le stack trace si disponible

Si le fichier `startup.log` n'existe PAS après un crash, c'est que le crash a eu lieu avant l'initialisation statique (problème de DLL manquante ou incompatible).

---

## Règles pour Kiro

### Avant de modifier du code

1. **Vérifier la branche** — `git branch --show-current` doit correspondre à l'issue
2. **Toujours lire le fichier avant de le modifier** — ne jamais proposer des changements sur du code pas lu
3. **Ne PAS compiler localement pour Windows** — pas de toolchain MSVC sur la machine
4. **Ne JAMAIS push sur master** — toujours pousser la branche feature/fix

### Correction de bug — reproduire d'abord en test unitaire

**Règle : avant de corriger un bug, essayer de le reproduire par un test unitaire, puis corriger.**

C'est l'approche « test-first » (ou red-green) appliquée aux bugs. Le flow :

1. **Écrire un TU qui reproduit le bug** — le test doit échouer (ou crasher) sur le code
   actuel. C'est la preuve tangible que le bug existe et qu'on l'a bien compris.
2. **Lancer le TU sur le code non corrigé** pour confirmer qu'il échoue/crashe.
3. **Corriger le code**.
4. **Relancer le TU** : il doit maintenant passer. Le test reste dans la suite comme test
   de non-régression permanent.
5. **Valider la suite complète** (`./scripts/docker-build.sh`) pour vérifier qu'aucune
   régression n'a été introduite.

Pourquoi :
- Le test documente le bug et empêche sa réapparition (non-régression).
- Il force à comprendre la cause racine avant de coder la correction.
- Il donne une preuve reproductible plutôt qu'une correction « à l'aveugle ».

Cas particuliers :
- **Bugs de mémoire non initialisée / undefined behavior** : un simple test peut passer par
  chance (ex. un pointeur non initialisé qui vaut `nullptr` par hasard). Pour rendre la
  reproduction **déterministe**, construire l'objet sur un buffer « empoisonné »
  (placement-new sur un tampon rempli d'un motif non nul comme `0xEF`) afin que le membre
  non initialisé soit lu comme une adresse invalide non nulle → crash garanti sur le code
  buggé, test qui passe sur le code corrigé. (Exemple : #229 `UBOEmbedParser`.)
- **Bug non reproductible en TU** (rendu visuel, interaction souris, comportement runtime
  Windows) : si aucun TU raisonnable ne peut le capturer, le documenter dans la PR, ajouter
  des diagnostics `[TAG]` dans `startup.log` (voir plus bas), et corriger avec l'aide des
  logs de la VM. Noter explicitement pourquoi le bug n'a pas pu être couvert par un TU.
- **Ajout d'une nouvelle classe QObject sous test** : compiler la vraie source dans
  `tests/tests.pro`, pré-générer son moc dans `docker-build.sh` (motif `premoc/`), et
  régénérer le `premoc/moc_tst_*.cpp` du fichier de test si on ajoute des slots.

### Après modifications — validation locale obligatoire

Avant tout commit et push, Kiro doit exécuter la validation locale complète :

1. **Build app** : `./scripts/docker-build.sh --build-only`
   - Si échec → corriger avant de continuer
2. **Tests unitaires + couverture** : `./scripts/docker-build.sh --test-only`
   - Si un test échoue → corriger avant de continuer
   - Vérifier que la couverture ne régresse pas significativement
3. **Si tout passe** → commiter avec `fix(#ID)` / `feat(#ID)` et pousser la branche
4. **Créer la PR** immédiatement (le CI ne tourne que sur les PRs)
5. Le développeur vérifie le CI, teste sur VM, puis demande le merge

Commande unique pour tout valider :
```bash
./scripts/docker-build.sh
```

### Quand le développeur envoie un startup.log

1. Analyser chaque section du log
2. Identifier les erreurs QML, positions incorrectes, widgets manquants
3. Proposer des corrections précises avec les fichiers et lignes concernés
4. Re-valider en Docker (build + tests) avant de proposer un push

### Ajouter des diagnostics

Si un bug est difficile à comprendre sans plus de contexte runtime :
1. Ajouter des logs `[TAG]` dans le code C++ aux points clés
2. Compiler, pousser la branche, et demander au développeur de renvoyer le startup.log
3. Une fois le bug corrigé, nettoyer les logs temporaires (garder les logs structurels permanents)

---

## Infrastructure Docker locale

### Fichiers

| Fichier | Rôle |
|---------|------|
| `Dockerfile.dev` | Image dev complète (Ubuntu 25.04, Qt 6.8.3, GCC 14, lcov, xvfb, gdb) |
| `Dockerfile.qt6` | Image legacy (Ubuntu 24.04, Qt 6.4) — build app uniquement, pas de tests |
| `scripts/docker-build.sh` | Script orchestrateur : build + tests + couverture |
| `tests/run_tests.sh` | Runner de tests standalone (utilisable dans le container) |

### Performances (ARM64 natif sur M4 Pro)

| Opération | Durée |
|-----------|-------|
| Build app (incrémental) | ~30s |
| Build app (clean) | ~120s |
| Build tests | ~25s |
| Run tests (17 suites) | <1s |
| Couverture (lcov) | ~2s |
| **Workflow complet (incrémental)** | **~60s** |
| **Workflow complet (clean)** | **~3 min** |
| Build x64 via QEMU | ~10-15 min |

### Notes techniques

- **moc bug** : le moc système Qt 6.8.3 ne peut pas parser les system headers GCC 14 via `moc_predefs.h`. Le script `docker-build.sh` contourne ce problème en pré-générant les moc sans `moc_predefs.h` pour les headers affectés (`UBFileSystemUtils.h` + 4 headers de test).
- **x64 via QEMU** : fonctionne via `--platform linux/amd64` dans Docker Desktop. 5-10x plus lent que le natif ARM64. Utile pour valider la compatibilité x64 avant le CI.
- **Couverture** : mesurée uniquement sur le code source sous test (src/frameworks, src/core, src/document, src/adaptors, src/web, src/domain). Les fichiers moc, stubs et tests sont exclus.
