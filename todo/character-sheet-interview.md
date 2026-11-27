# Character sheet interview queue

One character at a time: agent asks → you answer → agent patches `Data/character-sheets.json` + generator → you say **approved** on the write-up → next row.

## Queue

| # | SheetId | Status |
| --- | --- | --- |
| 1 | `Invader.Wanderer.Common` | **done** |
| 2 | `Invader.Thief.Common` | **done** |
| 3 | `Fungal.Hero` | **done** |
| 4 | `Spiders.Hero` | **done** |
| 5 | `Invader.Champion.WarriorQueen` | **done** |
| 6 | `Invader.Cleric.FireflySlot` | **done** |
| 7 | `Orcs.Worker` | **done** |
| 8 | `Orcs.Minion` | **done** |
| 8b | `*.Worker` (all families) | **done** (shared template) |
| 9 | `Orcs.Elite` | **done** |
| 10 | `Orcs.Boss` | **done** |
| 11 | `EvilFairies.Minion` | **in progress** |
| 9+ | Defender tiers | pending |

## Completed notes

### `Invader.Wanderer.Common`

- Display **Wanderer**; kit unchanged (Opportunist strike / Short sprint).
- **Near-bottom** fodder vitals (22 HP, 3.5 basic).
- **INT 7 / WIS 7** — generalist survivor; `Caution` 0.45, `ScoutRating` 0.4.
- Tags: `Knows_LeyTraps`, `Knows_MimicRisk`.
- Mesh: **Hi3D** → `/Game/Invaders/Wanderer/` (credits blocked).

### `Invader.Thief.Common`

- Display **Thief**; actives **Lockpick** + **Climb walls** (`ClimbWall` tag; `CanClimbWalls` on gear).
- **Below Wanderer** — 18 HP, 3 basic, 13/10 def.
- **Agility 9** / Mobility 8; **INT 4 / WIS 4**.
- Tags: `Knows_MimicRisk`, `Knows_LeyTraps`, `Knows_MirageVeil`.
- Mesh: **Hi3D** → `/Game/Invaders/Thief/`.

### `Fungal.Hero` (Fungal Queen)

- **Mage** — **Spores** (ranged AOE) + **Roots** (root/slow).
- Glass **58 HP**, low basic; **INT 9 / WIS 8**, Durability 3.
- **Stationary** defender, **Long** range, **300 UU** leash (spawn room).

### `Spiders.Hero` (Peacock)

- **Archer** — **Ranged web** (root/slow at range) + **Leap attack** (melee dash).
- **Tough** — 88 HP, Durability 7; solid basic damage.
- Same room guard as Queen: **Stationary**, **Long**, leash **300 UU**.

### `Invader.Champion.WarriorQueen`

- Champion kit unchanged; **Standard** tier frontliner (62 HP).
- **Party lead** — CohesionBias 0.45, high follower/helper weights.
- Ratings ~**7** across board (not maxed); no trap tags.
- Mesh: **existing** in UE → `/Game/Invaders/WarriorQueen/Warrior_Queen` (fix path if yours differs).

### `Invader.Cleric.FireflySlot`

- Default cleric kit; **heavy support** — 76 HP, **MaxStamina 120**, WIS 9, other ratings low.
- `PartyTemplates`: **FireflyFour**; mesh Hi3D pending (Zinx fallback until rigged).
- Tags: `Knows_LeyTraps`, `Knows_AllyHeal` (ally heal discipline — add to Invaders.md trap table when you approve).

### All `*.Worker` (shared template)

- **Dig** + **Reset traps**; weak **2** basic; ratings all **2–3**.
- **Flee** on raid intel or intruder in area; `WorkerRole` still per family (Dig/Build/Both).
- Display names: Orc Worker, Undead Worker, Evil Fairy Worker, Fungal Worker, Spider Worker, Mutant Worker.

### `Orcs.Minion` (Orc Minion)

- **Combo strike** + **War cry**; **Standard** tier medium (48 HP, 7 basic).
- INT/WIS **2**; chase + **HarassInvaders**.

### `Orcs.Elite` / `Orcs.Boss` (defaults — no interview)

- **Orc Elite:** bruiser kit tuned (68 HP); harass chase. **Orc Boss:** tank kit (175 HP); frontline chase. Orc line INT/WIS **2**.
- **`Orcs.Hero`** stays **Narbash** (Paragon) — already on sheet.