---
inclusion: fileMatch
fileMatchPattern: 'docs/adr/**'
---
# Open-Sankoré — Architecture Decision Records (ADR)

Les décisions structurantes du projet sont consignées dans `docs/adr/` sous forme
d'ADR (format MADR-lite, un fichier Markdown par décision). Ce steering décrit le
format et le workflow. Voir aussi `docs/adr/README.md` (l'index) et
`docs/adr/0000-template.md` (le gabarit à copier).

## Format

- Un fichier par décision : `docs/adr/NNNN-titre-en-kebab-case.md`, `NNNN`
  séquentiel (0001, 0002, …).
- Sections : **Status**, **Date**, **Deciders**, **Related** (issue/PR/ADR), puis
  **Context / Decision / Consequences / Alternatives considered**.
- **Langue : anglais** (comme les issues, PR, commits).
- Statuts : `Proposed` → `Accepted` → éventuellement `Superseded by ADR-XXXX`.
- **Immuable une fois `Accepted`.** Pour revenir sur une décision, écrire un
  **nouvel** ADR et passer l'ancien à `Superseded by ADR-XXXX` — ne jamais
  supprimer ni réécrire un ADR accepté.

## Ce qui mérite un ADR (et ce qui n'en mérite pas)

Oui — décisions qui **contraignent le futur** :
- choix d'architecture ou de techno (ex. migration UI QML V2) ;
- changement de format de fichier ou d'API publique ;
- abandon / remplacement / stub d'un module (ex. web stubbé) ;
- tradeoff de perf, sécurité ou build assumé (ex. stratégie premoc) ;
- décision produit structurante (ex. palette de couleurs partagée stylo/formes).

Non — détails d'implémentation, correctifs de bug ponctuels, refactors locaux,
choix réversibles sans impact durable. Ceux-là vivent dans les issues/PR.

## Workflow — comment Kiro trace les décisions

**Kiro ne crée jamais un ADR de sa propre initiative** (même règle que merge/tag).

1. Quand le développeur prend une décision structurante en conversation (« on
   part sur l'option B », « on stubbe le web », « la version vient du tag »…),
   Kiro le détecte et **propose** : « Je crée un ADR pour cette décision ? ».
2. Sur accord, Kiro crée `docs/adr/NNNN-*.md` à partir de `0000-template.md`,
   statut `Accepted` (ou `Proposed` si la décision est encore ouverte), en
   remplissant Context / Decision / Consequences / Alternatives, et en
   référençant l'issue/PR liée.
3. Kiro **met à jour l'index** `docs/adr/README.md` (table + statut).
4. L'ADR est commité en anglais, sur la branche du travail concerné, ou sur une
   branche `chore/adr-NNNN-*` dédiée si la décision est isolée (pas de code app).
5. Numérotation : prendre le prochain `NNNN` libre (regarder l'index).

Rétroactif : il est légitime de consigner a posteriori une décision structurante
déjà prise (marquer « Recorded retroactively » dans l'ADR), pour ne pas perdre le
« pourquoi ».

## Règle pour Kiro

- Proposer un ADR dès qu'une décision structurante est prise ; ne jamais en créer
  un sans l'accord explicite du développeur.
- Ne jamais éditer le fond d'un ADR `Accepted` : créer un nouvel ADR qui le
  remplace et marquer l'ancien `Superseded by ADR-XXXX`.
- Toujours tenir `docs/adr/README.md` synchronisé avec les fichiers présents.
