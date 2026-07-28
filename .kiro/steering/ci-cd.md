---
inclusion: manual
---

# Open-Sankoré — CI/CD Pipeline

## Vue d'ensemble

Le projet utilise GitHub Actions avec 3 workflows :

| Workflow | Fichier | Déclencheur | Durée |
|----------|---------|-------------|-------|
| Build Windows | `build-windows.yml` | push master, PR | ~15 min |
| Build Linux x64 | `build-linux.yml` | push master, PR | ~10 min |
| Release | `release.yml` | tag `v*` | ~25 min |

## Workflow : Build Windows (`build-windows.yml`)

### Étapes principales

1. **Checkout** code
2. **Install Qt 6.8.2** via aqtinstall (modules : qt5compat, qtmultimedia)
3. **Setup MSVC** (ilammy/msvc-dev-cmd)
4. **Install OpenSSL** via vcpkg (dynamique + statique)
5. **Build zlib** from source (statique)
6. **Build QuaZip** from source (statique, lié à zlib)
7. **qmake + nmake** release (CONFIG+=no_webengine)
8. **Build unit tests** (qmake + nmake dans tests/)
9. **Run unit tests** (-v2, QT_QPA_PLATFORM=offscreen)
10. **windeployqt** — package l'exe avec les DLLs Qt
11. **Smoke test** — lance l'exe 8 secondes, vérifie qu'il ne crash pas
12. **Build installer** — Inno Setup (.exe)
13. **Upload artifacts** — exe, installer

### Artefacts produits

- `Open-Sankore-Windows-x64` : dossier complet avec DLLs (portable)
- `Open-Sankore-Windows-x64-Setup` : installeur .exe (Inno Setup)

## Workflow : Build Linux x64 (`build-linux.yml`)

### Environnement

- Container `ubuntu:22.04` (sur ubuntu-latest runner)
- Qt 6.2 via packages système (`qt6-base-dev`, etc.)
- GCC 11, C++17

### Étapes principales

1. **Install dépendances** système (Qt 6, cmake, lcov, rpm...)
2. **Checkout** code
3. **Build QuaZip** from source (shared lib)
4. **qmake + make** avec `--coverage` (gcov instrumentation)
5. **Smoke test** — lance l'app avec `--quit-after=5` (exit propre, flush gcda)
6. **Collect app coverage** — lcov sur les .gcda générés par le smoke test
7. **Build unit tests** (avec premoc/ pour contourner bug moc)
8. **Run unit tests** (-v2, offscreen)
9. **Generate coverage report** — lcov unit tests + merge avec app coverage
10. **Update coverage badge** — push le % vers un gist (shields.io)
11. **Package .deb + .rpm** via `scripts/package-linux.sh`
12. **Upload artifacts** — binaire, coverage HTML, .deb, .rpm

### Artefacts produits

- `Open-Sankore-Linux-x64` : binaire compilé
- `coverage-report-linux` : rapport HTML (genhtml)
- `open-sankore-deb-x64` : package .deb
- `open-sankore-rpm-x64` : package .rpm

## Workflow : Release (`release.yml`)

### Déclencheur

Push d'un tag `v*` (ex: `v3.1.0`)

### Comportement

1. Build Windows (même étapes que build-windows.yml)
2. Build Linux x64 (même étapes que build-linux.yml)
3. Crée une GitHub Release avec les assets :
   - `Open-Sankore-Windows-x64.zip`
   - `Open-Sankore-*-setup.exe`
   - `open-sankore_*_amd64.deb`
   - `open-sankore-*.x86_64.rpm`

## Couverture de code (Coverage)

### Mécanisme

L'application est compilée avec `--coverage` (GCC). Deux sources de données :

| Source | Méthode | Ce qu'elle mesure |
|--------|---------|-------------------|
| App coverage | Lancement + quit après 5s | Code d'initialisation (~20% lignes) |
| Unit test coverage | 6 suites de tests QTest | Modules utilitaires testés (~93% des lignes testées) |
| Combined | Merge lcov des deux | Vision globale (~23% lignes) |

### --quit-after (CI only)

L'option `--quit-after=N` dans `main.cpp` permet à l'app de quitter proprement après N secondes.
Un signal handler appelle `__gcov_dump()` pour flush les .gcda même en cas de SIGINT/SIGTERM.

### Badge dynamique

Le % combined est poussé vers un gist public via l'API GitHub.
Shields.io lit le gist et affiche un badge dans le README.

- **Gist ID** : `0b042ec6f4db15ba496d37215c18a649`
- **URL badge** : `https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/davidguyomarch/0b042ec6f4db15ba496d37215c18a649/raw/coverage.json`

## Secrets GitHub Actions

| Secret | Scope | Usage | Qui peut lire |
|--------|-------|-------|---------------|
| `GIST_TOKEN` | `gist` uniquement | Mettre à jour le gist coverage badge | Workflows sur push master uniquement |

### Sécurité du GIST_TOKEN

- **Ce qu'il peut faire** : créer/modifier/supprimer les gists du propriétaire
- **Ce qu'il NE peut PAS faire** : accéder au code, issues, settings, secrets, repos, packages, actions
- **Qui peut le lire** : uniquement les workflows déclenchés par un push sur master (pas les PRs de forks)
- **Rotation** : le token peut être révoqué/recréé à tout moment sans impact sur le build

## Tests unitaires

### Structure

```
tests/
  main.cpp              # Exécute les 6 suites de tests
  tst_UBStringUtils.cpp # Tests chaînes/encodage
  tst_UBFileSystemUtils.cpp  # Tests système de fichiers
  tst_UBGeometryUtils.cpp    # Tests géométrie
  tst_UBCryptoUtils.cpp      # Tests chiffrement (OpenSSL)
  tst_UBDocumentProxy.cpp    # Tests modèle document
  tst_UBSettings.cpp         # Tests settings
  stubs/                # Implémentations minimales des dépendances
  premoc/               # Fichiers moc pré-générés (contourne bug moc Linux)
```

### Bug moc Linux

Sur GitHub Actions Linux, moc ne parvient pas à parser les headers système C++ (bug reproductible avec toutes versions de moc testées). Solution : les fichiers moc sont pré-générés localement avec moc 6.2.4 et committes dans `tests/premoc/`.

## Scripts utilitaires

| Script | Rôle |
|--------|------|
| `scripts/deploy-latest.sh` | Télécharge le dernier artefact CI, le place dans `../sankore-install/` |
| `scripts/package-linux.sh` | Crée .deb et .rpm à partir du binaire compilé |
| `scripts/installer.iss` | Script Inno Setup pour l'installeur Windows |

## Badges README

```markdown
![Build Windows](https://github.com/davidguyomarch/Sankore/actions/workflows/build-windows.yml/badge.svg)
![Build Linux](https://github.com/davidguyomarch/Sankore/actions/workflows/build-linux.yml/badge.svg)
![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/davidguyomarch/0b042ec6f4db15ba496d37215c18a649/raw/coverage.json)
```

## Durées typiques

| Workflow | Durée | Coût (GitHub Free) |
|----------|-------|-------------------|
| Build Windows | 12-15 min | ~15 min billable |
| Build Linux | 8-12 min | ~10 min billable |
| Release (les deux) | 20-25 min | ~25 min billable |

GitHub Free offre 2000 min/mois. Un push sur master consomme ~25 min total (les deux builds).
