# Aura prompt — Random-tier creature spawn (Dungeon Core)

Copy everything below the line into Aura as the task brief.

---

## Project context

**Dungeon Core** — Unreal Engine dungeon defender. Authoritative gameplay in C++ where it exists; monster purchase/spawn today is largely Blueprint-driven (`BP_DungeonGrid`, creature menu). Six dungeon families; each has tiers Worker / Minion / Elite / Boss plus a fifth **Random** special (not a normal tier pick).

**Art pillar:** Crystalline Ley-Line Arcana (`Design/General.md`). Grid cell = 300 UU unless a file says otherwise.

**Do not edit** `Design/*.md` unless the human explicitly permits it in chat.

## Objective

Implement **Random-tier spawn replacement**: on every **dungeon creature spawn** (player-bought defenders from the family spawn flow), roll once whether that family's **Random** special replaces the spawn. If the roll succeeds, spawn the Random creature **instead of** the scheduled family/tier. Same family context as the spawn request.

This is **independent** of:
- **[Random Events](Design/Random%20Events.md)** (raid/meta events; different frequency knob on New Game).
- **Invader raids** (separate systems in `Design/Invaders.md`).

## Player setting (run save)

**Randomness** on the New Game card (`Design/New%20Game.md`):

| Setting | Replace chance |
| --- | ---: |
| Low | 0.1% (0.001) |
| Medium (default) | 1% (0.01) |
| High | 5% (0.05) |

Persist **Randomness** with the run save. **UI on New Game card is not built yet** — wire a saved field (enum or int) and default to **Medium** until the card exposes Low/Medium/High.

**Separate field:** Random Events frequency — do not conflate with Randomness.

## Algorithm (required)

For each defender spawn request `S`:

1. Inputs: `Family` (enum), intended `Tier` (Worker|Minion|Elite|Boss), spawn transform/room, run's `Randomness`.
2. If `Family == Mutants` and Random asset is undefined → **skip roll**; spawn planned tier (Mutants Random TBD in design).
3. Draw `u ~ Uniform(0,1)`.
4. Let `p` = table chance for `Randomness`.
5. If `u >= p` → spawn **planned** tier mesh for `Family` + `Tier` (existing path).
6. If `u < p` → spawn **Random** mesh for `Family` (table below). **Do not** also spawn the planned tier.
7. Log or telemetry hook (optional): `Family`, planned tier, outcome (`normal`|`random`), `p`, `u` — for debugging.

**Order:** Roll **before** resolving mesh/ABP for the planned tier. One roll per spawn invocation, not per chamber lifetime.

**No chain:** A Random spawn does **not** trigger a second Random roll.

## Mesh table (canonical `/Game/Monsters/` paths)

Bind spawn resolver to these. No silent fallback to wrong Paragon skin.

| Family | Random name | Skeletal mesh (content path) |
| --- | --- | --- |
| Orcs | Narbash | `/Game/Monsters/Orcs/ParagonNarbash/Characters/Heroes/Narbash/Meshes/Narbash` |
| Evil Fairies | Fey (Opaline) | `/Game/Monsters/EvilFairies/ParagonFey/Characters/Heroes/Fey/Skins/Opaline/Meshes/Fey_Opaline` |
| Fungal | Fungal Queen | `/Game/Monsters/Fungal/FungalQueen/Fungal_Queen` |
| Spiders | Peacock | `/Game/Monsters/Spiders/Peacock` |
| Undead | Sevarog | `/Game/Monsters/Undead/ParagonSevarog/Characters/Heroes/Sevarog/Meshes/Sevarog` |
| Mutants | — | **No asset** — skip Random branch until design adds one |

**Paragon notes:**
- Narbash uses full Paragon anim set; do not force CC shared skeleton on Random spawns (see `TODO.md` conversion list — Narbash is skipped).
- Fey default skin is **Opaline** only for automatic Random spawn unless a separate **choose-a-fairy** UI is implemented (see phase 2).

## Phase 1 (MVP — ship first)

- C++ or centralized Blueprint function: `ResolveCreatureSpawn(Family, Tier, Randomness) -> (bIsRandom, MeshPath or spawn data)`.
- Hook at the **single choke point** where the creature menu / generator commits a spawn (find existing spawn-after-purchase flow; do not duplicate per-family logic).
- Data: `UDataAsset` or `UDataTable` keyed by family → Random soft object path; load from paths above.
- Mutants: Random roll disabled or always fails until row exists.
- Save game: store `Randomness` enum; load on run start.

## Phase 2 (explicitly later — do not block MVP)

**Fey — choose-a-fairy encounter** (`TODO.md`): when Evil Fairies Random roll hits, **pause** silent swap; show UI listing fairy-tier options; player picks one; then spawn that pick. Still consumes the Random roll for that spawn. Not required for first merge.

**Paragon skin policy** (`TODO.md`): document fixed vs random skin per Paragon; Narbash/Sevarog may need skin picker rules later.

## Out of scope for this task

- Random Events raid logic.
- Invader spawn tables.
- Narbash drum hide/show (cosmetic; separate TODO).
- Mutants Random creature design/import.
- New Game card widget for Randomness (stub default Medium only until UI exists).

## Acceptance criteria

1. With Randomness **High** and Orc Minion spawn, ~5% of spawns over many trials use **Narbash** mesh, rest use planned minion.
2. With **Low**, hit rate ~0.1%; **Medium** ~1%.
3. Fungal/Undead/Spiders/Orcs Random hits use exact paths in table (verify in editor via spawn).
4. Mutants never Random-replace until an asset row exists.
5. Random Events settings unchanged when Randomness changes.
6. No double-spawn (planned + Random).

## References (read before coding)

- `Design/Monsters.md` — Random tier section + In UE grid
- `Design/New Game.md` — Randomness vs Random Events
- `Design/TODO.md` — lines: Random-tier spawn roll, Wire Random meshes, Fey encounter, Mutants Random, Paragon skins
- `Design/Invaders.md` — invader knowledge tags (future; not this spawn path)

## Engineering constraints

- Modern UE C++: `UPROPERTY` / `UENUM` for family, tier, randomness.
- Prefer event/delegate at spawn commit over Tick polling.
- Minimal diff; reuse existing monster spawn actor class if one exists.
- Failed mesh load: **log error**; do not fall back to cube or wrong tier silently.

## Deliverables

1. Short note in chat: which files changed and where spawn hook lives.
2. Optional: `tmp/random-tier-spawn-manifest.json` listing family → soft object paths if not using DataTable in repo.
3. Manual test steps (PIE): force Randomness High, buy 20 same-family creatures, count Random mesh appearances.