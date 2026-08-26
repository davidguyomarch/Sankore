# Open-Sankoré — Workflow de développement

## Boucle de développement (Build → Validate → Test → Fix)

Le développeur travaille sur **macOS ARM (M4 Pro)**. Il n'y a **pas de compilation locale Windows**.
La validation se fait en 3 étapes :

```
Code → Docker Linux (2 min) → Push → CI Windows (25 min) → VM Windows (test manuel)
                ↑                                                       |
                └───────────── Kiro corrige ← startup.log ←────────────┘
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

### Étape 2 : CI Windows — GitHub Actions (~25 min)

Pousser le code déclenche le build Windows CI :

```bash
git push
```

Le CI (`build-windows.yml`) fait : compilation MSVC → unit tests → windeployqt → smoke test → artefact.

Pour une branche feature (pas master) :
```bash
gh workflow run build-windows.yml --ref ma-branche
```

### Étape 3 : Test sur VM Windows — deploy + run-test.bat

Le développeur déploie l'artefact CI et le teste sur une VM Windows 11 ARM64 (UTM) :

```bash
# Sur Mac — télécharge le dernier artefact CI
./scripts/deploy-latest.sh

# Ou un run spécifique
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

### Étape 4 : Analyse des logs

Après le test, le développeur copie le contenu de `startup.log` dans la conversation Kiro.
Kiro utilise ces logs pour diagnostiquer les bugs et proposer des corrections.

Le log est aussi lisible depuis le Mac (si le partage VirtIO fonctionne) :
```bash
cat ../sankore-install/startup.log
```
Mais préférer la lecture depuis la VM pour fiabilité : `type C:\Sankore\startup.log`

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
Build: 4.0.2.r
Date: 2026-08-25T14:30:00
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

## Règles pour Kiro

### Avant de modifier du code

1. **Toujours lire le fichier avant de le modifier** — ne jamais proposer des changements sur du code pas lu
2. **Compiler en Docker Linux** après les modifications pour valider la syntaxe
3. **Ne PAS compiler localement pour Windows** — pas de toolchain MSVC sur la machine

### Après modifications

1. Compiler en Docker : `docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c 'qmake6 OpenSankore.pro CONFIG+=no_webengine && make -j$(nproc)'`
2. Si la compilation échoue, corriger avant de proposer un push
3. Si la compilation réussit, proposer les commandes git (add, commit, push)
4. Le développeur lance le CI, déploie, teste, et revient avec le startup.log

### Quand le développeur envoie un startup.log

1. Analyser chaque section du log
2. Identifier les erreurs QML, positions incorrectes, widgets manquants
3. Proposer des corrections précises avec les fichiers et lignes concernés
4. Re-compiler en Docker pour valider

### Ajouter des diagnostics

Si un bug est difficile à comprendre sans plus de contexte runtime :
1. Ajouter des logs `[TAG]` dans le code C++ aux points clés
2. Compiler, pousser, et demander au développeur de renvoyer le startup.log
3. Une fois le bug corrigé, nettoyer les logs temporaires (garder les logs structurels permanents)
