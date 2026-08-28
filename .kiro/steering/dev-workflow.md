# Open-Sankoré — Workflow de développement

## Mode PROD — Règles de travail

### Flow complet

```
Issue → Branche → Commits → Push branche → CI passe → Test Windows VM → PR → Squash merge → Branche supprimée
```

**Master est toujours stable.** On ne push jamais directement sur master. Tout passe par une branche + PR.

### 1. Tout travail est lié à une issue GitHub

- On ne commence JAMAIS un travail sans issue GitHub associée.
- Si l'issue n'existe pas, on la crée d'abord avec un titre clair, une description du problème/besoin, et un label (bug/enhancement).
- Référence : https://github.com/davidguyomarch/Sankore/issues

### 2. Nommage des branches

Chaque issue a sa branche dédiée, créée depuis master :

```
fix/<issue-id>-<description-courte>     # pour les bugs
feat/<issue-id>-<description-courte>    # pour les fonctionnalités
```

Exemples :
- `fix/135-desktop-crash`
- `feat/134-documents-qml-view`
- `fix/133-capture-cursor`

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

### 5. Push et validation

Quand le travail est prêt à tester :
1. Compiler en Docker pour valider la syntaxe
2. Commiter avec le bon format `fix(#ID)` / `feat(#ID)`
3. Pousser la branche (jamais master)
4. Le CI tourne automatiquement sur la branche
5. Le développeur vérifie que le CI passe (Windows + Linux)
6. Le développeur teste sur la VM Windows si nécessaire

### 6. Pull Request

Quand le CI passe et le test Windows est OK, Kiro crée la PR :

```bash
gh pr create \
  --base master \
  --title "fix(#135): description du fix" \
  --body "## Changements

- Description des changements

## Tests
- [ ] CI Windows passe
- [ ] CI Linux passe
- [ ] Testé sur VM Windows

Closes #135"
```

### 7. Merge — uniquement sur demande explicite du développeur

Kiro ne merge JAMAIS de sa propre initiative. Quand le développeur valide :

```bash
gh pr merge --squash --delete-branch
```

Cela fait automatiquement :
- Squash merge (1 seul commit propre sur master)
- Suppression de la branche distante
- Fermeture de l'issue (grâce à `Closes #ID` dans le body de la PR)

### 8. Nettoyage local après merge

```bash
git checkout master && git pull
git branch -d <branche-locale>
```

---

## Boucle de développement (Build → Validate → Test → Fix)

Le développeur travaille sur **macOS ARM (M4 Pro)**. Il n'y a **pas de compilation locale Windows**.
La validation se fait en 4 étapes :

```
Code → Docker Linux (2 min) → Push branche → CI Windows (25 min) → VM Windows (test manuel)
                ↑                                                            |
                └────────────────── Kiro corrige ← startup.log ←────────────┘
                                                                    ↓
                                                              PR → Merge master
```

### Étape 1 : Validation rapide — Docker Linux (~2 min)

Avant tout push, compiler localement avec le Docker Linux pour attraper les erreurs C++ :

```bash
docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c \
  'qmake6 OpenSankore.pro CONFIG+=no_webengine && make -j$(nproc)'
```

- **Image** : `sankore-qt6` (Ubuntu 24.04, Qt 6.2, aarch64)
- **Ce que ça valide** : syntaxe C++, résolution de symboles, moc, QML dans les .qrc
- **Ce que ça NE valide PAS** : compilation MSVC, runtime Windows, comportement visuel
- **Durée** : ~2 min (incrémental) / ~5 min (clean)

Pour un build clean (après changements de .pro, .h ou .qrc) :
```bash
docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c \
  'make clean; qmake6 OpenSankore.pro CONFIG+=no_webengine && make -j$(nproc)'
```

### Étape 1b : Smoke test local — Docker Linux (~10 sec)

Après compilation, on peut lancer l'app en headless pour valider le démarrage et lire `startup.log` :

```bash
docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c '
  rm -f /src/build/linux/release/product/startup.log
  xvfb-run -s "-screen 0 1920x1080x24" timeout 10 \
    ./build/linux/release/product/Open-Sankore --quit-after=5 2>&1
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

### Étape 2 : Push branche + CI

Pousser la branche (pas master) déclenche les builds CI :

```bash
git push
```

Le CI (`build-windows.yml`, `build-linux.yml`) tourne sur la branche. Vérifier que les builds passent avant de demander un test Windows.

Pour déclencher manuellement un build sur la branche :
```bash
gh workflow run build-windows.yml --ref fix/135-desktop-crash
```

### Étape 3 : Test sur VM Windows — deploy + run-test.bat

Le développeur déploie l'artefact CI de la branche et le teste :

```bash
# Trouver le run ID du build de la branche
gh run list --workflow=build-windows.yml --branch=fix/135-desktop-crash --status=success --limit=1

# Télécharger l'artefact
./scripts/deploy-latest.sh <run-id>
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

### Étape 4 : PR + Merge

Quand le test Windows est OK :
1. Kiro crée la PR (`gh pr create`)
2. Le développeur valide
3. Kiro merge (`gh pr merge --squash --delete-branch`)
4. Nettoyage local (`git checkout master && git pull && git branch -d <branche>`)

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
3. **Compiler en Docker Linux** après les modifications pour valider la syntaxe
4. **Ne PAS compiler localement pour Windows** — pas de toolchain MSVC sur la machine
5. **Ne JAMAIS push sur master** — toujours pousser la branche feature/fix

### Après modifications

1. Compiler en Docker : `docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c 'qmake6 OpenSankore.pro CONFIG+=no_webengine && make -j$(nproc)'`
2. Si la compilation échoue, corriger avant de proposer un push
3. Si la compilation réussit, commiter avec `fix(#ID)` / `feat(#ID)` et pousser la branche
4. Le développeur vérifie le CI, teste sur VM, puis demande la PR et le merge

### Quand le développeur envoie un startup.log

1. Analyser chaque section du log
2. Identifier les erreurs QML, positions incorrectes, widgets manquants
3. Proposer des corrections précises avec les fichiers et lignes concernés
4. Re-compiler en Docker pour valider

### Ajouter des diagnostics

Si un bug est difficile à comprendre sans plus de contexte runtime :
1. Ajouter des logs `[TAG]` dans le code C++ aux points clés
2. Compiler, pousser la branche, et demander au développeur de renvoyer le startup.log
3. Une fois le bug corrigé, nettoyer les logs temporaires (garder les logs structurels permanents)
