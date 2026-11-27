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
| 11 | `EvilFairies.Minion` | **done** |
| 12 | `EvilFairies.Elite` | **done** |
| 13 | `EvilFairies.Boss` | **done** |
| 14 | `Fungal.Minion` | **done** |
| 15 | `Fungal.Elite` | **done** |
| 16 | `Fungal.Boss` | **done** |
| 17 | `Spiders.Minion` | **done** |
| 18 | `Spiders.Elite` | **done** |
| 19 | `Spiders.Boss` | **done** |
| 20 | `Undead.Minion` | **done** |
| 21 | `Undead.Elite` | **done** |
| 22 | `Undead.Boss` | **done** |
| 23 | `Mutants.Minion` | **done** |
| 24 | `Mutants.Elite` | **done** |
| 25 | `Mutants.Boss` | **done** |
| — | **Defender tier pass** | **complete** (6×5 + shared workers) |
| next | Paragon / Common invaders (non-interviewed) | optional |

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

### `EvilFairies.Minion` (Dark Fairy)

- **Ranged burst** + **Glamour**; glass (**30 HP**, low basic).
- **INT/WIS 8**, Agility **8**, Durability **2**.
- **Guard** at **Long** range; **chase only if target &lt;35% HP** (`ChaseInjuredTargetsOnly`).

### `EvilFairies.Elite` (Dark Fairy Elite) — auto

- **Mage** (not bruiser): **Bramble lash** + **Cursed glamour**; 44 HP glass-elite.
- INT **9** / WIS **8**; same injured-chase ranged guard as minion.

### `EvilFairies.Boss` (Dark Fairy Matron) — auto

- **Mage** room boss: **Bramble patch** + **Withering glam** (AOE control).
- **72 HP** (fae glass boss); INT **10** / WIS **9**.
- **Stationary**, **Long**, **300 UU** leash; chase finishers below **25% HP**.
- **`EvilFairies.Hero`** = **The Fey** (Paragon) — unchanged.

### `Fungal.Minion` (Fungal Minion)

- **Spore jab** + **Bloat rot**; **Standard** medium bruiser (52 HP, 8 basic).
- INT/WIS **1**; chase + **HarassInvaders** (orc minion pattern).

### `Fungal.Elite` — auto

- **Spore cleave** + **Mycelium rush**; 70 HP bruiser; INT/WIS **1**; harass chase.

### `Fungal.Boss` (Myconid Golem) — auto

- **Spore bulwark** + **Golem slam**; **132 HP** tank; INT/WIS **1**; slow chase harass.
- Fungal family complete except tier baselines if you revisit later.

### `Spiders.Minion` (Spider Minion) — auto

- **Venom shot** + **Entangling web**; agile glass (38 HP); INT/WIS **2**.
- **Guard** / **Long**; harass + chase finishers below **35% HP** (Peacock family pattern).

### `Spiders.Elite` — auto

- **Archer** (not bruiser): **Venom burst** + **Pounce strike**; 54 HP; same ranged-guard AI.

### `Spiders.Boss` (Brood Tarantula) — auto

- **Web storm** + **Leg rend**; **96 HP**; room **Stationary** / **Long** / **300 UU** (Peacock-style).
- Chase finishers below **25% HP**. **`Spiders.Hero`** = **Peacock** — unchanged.

### `Undead.Minion` (Skeleton Soldier) — auto

- **Bone slash** + **Rattle fear**; sturdy **50 HP**, slow; INT/WIS **1**; harass **Chase**.

### `Undead.Elite` (Mummy)

- **Shroud strike** + **Grave choke**; damage bruiser (62 HP, high ability dmg).
- **Guard** corridor hold, **500 UU** leash; no roam/harass. INT/WIS **1**.

### `Undead.Boss` (Litch)

- **Mage** necromancy: **Soul drain** + **Death coil**; drain-tank (**108 HP**, light regen).
- **Stationary** room boss, **Long**, **300 UU**; INT/WIS **9**. (AI defaulted — say if you want corridor Guard instead.)

### `Mutants.Minion` (Mutant Minion)

- **Rending claw** + **Frenzy howl**; chunky **54 HP** but low defenses (fragile berserker).
- INT/WIS **1**; chase + harass.

### `Mutants.Elite` (Mutant Elite)

- **Fighter** — **Maw slam** + **Lurching charge**; elite **76 HP** chunky.
- **`PackTactics: true`** (sheet hook for pack engage); harass chase. WIS **2** (pack sniff); INT **1**.

### `Mutants.Boss` (Maw Titan)

- **Tank**: **Devour** + **Ground pound**; **158 HP**, high defenses.
- **Stationary** **300 UU**; **`PackTactics`** pack alpha. INT **2** / WIS **4**.
- **`Mutants.Hero`** = **Rampage** (Paragon) — unchanged.