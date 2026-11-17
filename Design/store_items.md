# Store items

Catalog of everything that can be (or is planned to be) bought from the dungeon store, grouped by what the item **does**. Buying, placing, edit mode, HUD wiring and art pipeline live in [Store.md](./Store.md).

**Done column:** `[x]` only when the item is **fully shipped** (store row when applicable, place rules, icon, model when applicable, gameplay matches the row). Power/economy rows 11–13 count as in the project but not done until they are store rows with behavior.

**In store:** whether the item has a HUD row (`StoreItem_*`) in the current eleven-row build.

---

## Power and economy

Mana, room power, cap, and essence rewards.

| # | Done | In store | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | --- | ---: | --- | --- | --- | --- |
| 1 | [x] | [x] | **Mana Generator** | 30 | +1 mana per second each | `mana_generator/Mana_Generator_mana_generator`, theme glow | Works only inside a **powered** room ([Future.md](./Future.md)). No unlock step written | `mana_generator.png` |
| 11 | [ ] | [ ] | **Ley Conduit** | 25 | Not in store yet; no behavior | `ley_conduit/ley_conduit_v2` in project | Powers the room it stands in so generators away from the core can run ([Future.md](./Future.md)) | `ley_conduit.png` |
| 12 | [ ] | [ ] | **Mana Reservoir** | 35 | Not in store yet; no behavior | `mana_reservoir/mana_reservoir_unreal` in project | Raises mana cap by 25 each; spills stored mana when broken | `mana_reservoir.png` |
| 13 | [ ] | [ ] | **Siphon Font** | 40 | Not in store yet; no behavior | `siphon_font/siphon_font_unreal` in project | Extra essence from invaders who fall in this room ([Overall.md](./Overall.md)) | `siphon_font.png` |

---

## Generators and spawners

Placeable objects that produce creatures or ongoing room effects (not single-use traps).

| # | Done | In store | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | --- | --- | ---: | --- | --- | --- |
| 0 | [x] | [x] | **Monster Generator** | 40 | Click to open monster menu and buy creatures | `Monster_Generator_v2`, theme glow | Creature menu at tutorial beat 8 (first defender), after first trap | `monster_generator.png` |
| 9 | [ ] | [x] | **Spore Generator** | 30 | Nothing yet | `spore_generator/Spore_Generator_Spore_Generator`, no glow | **Powered room:** crowd of spores; invaders in the same room **asleep** while active. Neutral shell now; per-theme shells later ([Theme.md](./Theme.md)) | `spore_generator.png` |

**Open (Spore Generator):** stack multiple in one room; sleep vs traps that need awake targets; powered room like mana generators ([Future.md](./Future.md)) — see [Store.md](./Store.md#open-questions).

---

## Traps, barriers, and control

Damage, pathing, slowing, misdirection, and rail-dependent hazards.

| # | Done | In store | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | --- | ---: | --- | --- | --- | --- |
| 2 | [x] | [x] | **Prism Beam Trap** | 35 | Nothing yet | `beam_trap/Prism_Beam_Trap_PrismBeamTrap`, theme glow | Needs mirrors and sightlines (see **Mirror Pillar** below). No unlock step written | `prism_beam_trap.png` |
| 3 | [x] | [x] | **Resonance Plate Trap** | 20 | Nothing yet | `PlateTrap/Plate_Trap_Plate_Trap`, theme glow | First trap: menu at tutorial beat 7 when first dug room is powered | `resonance_plate_trap.png` |
| 4 | [x] | [x] | **Gravity Well Trap** | 45 | Nothing yet | `gravity_well_trap/Gravity_Well_Trap_tmpq61u_eep_obj`, own white/purple glow | No unlock step written | `gravity_well_trap.png` |
| 5 | [ ] | [x] | **Runaway Cart Trap** | 30 | Nothing yet | None, placeholder block | Needs a **built rail line** ([Future.md](./Future.md)) | `runaway_cart_trap.png` |
| — | [ ] | — | **Mirror Pillar** | 15 | Not built | No art yet | Free-standing mirror; turns Prism Beam 90°; facing set in edit mode | — |
| — | [ ] | — | **Drag Lattice** | 20 | Not built | No art yet | Floor lattice: half speed, no damage | — |
| — | [ ] | — | **Displacement Glyph** | 45 | Not built | No art yet | Sold as linked pair; step on one, move to the other | — |
| — | [ ] | — | **Crystal Barricade** | 10 | Not built | No art yet | Blocks one square until broken; defenders pass | — |
| — | [ ] | — | **Mirage Veil** | 30 | Not built | No art yet | Fake solid edge; scouts / veterans ignore after seeing one | — |

**Why Mirror Pillar:** Prism Beam design assumes wall mirrors; nothing in the shipped list supplied them until this row ([Store.md](./Store.md#traps-and-control)).

**Placement notes (proposed):** Mirage Veil and Crystal Barricade may need edge/block rules unlike today's one-square items ([Store.md](./Store.md#open-questions-for-these)).

---

## Lures and bait

Pull invaders or punish approach without being a damage trap.

| # | Done | In store | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | --- | ---: | --- | --- | --- | --- |
| 6 | [x] | [x] | **Loot Bait** | 15 | Nothing yet | `LootBait/Loot_Bait_Loot_Bait`, no glow | Taught in free play after tutorial ("first loot placed as bait") | `loot_bait.png` |

Pairs with **Mirage Veil** (steer away) and **Mimic** (punish opening) in [Store.md](./Store.md).

---

## Defenders

Bought constructs that fight (not generators).

| # | Done | In store | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | --- | ---: | --- | --- | --- | --- |
| 7 | [ ] | [x] | **Unshackled Golem** | 50 | Nothing yet | None, placeholder block | No unlock step written | `unshackled_golem.png` |
| 8 | [ ] | [x] | **Shard Sentinel** | 25 | Nothing yet | None, placeholder block | No unlock step written | `shard_sentinel.png` |

**Open:** bought once each or via generator like family creatures? ([Store.md](./Store.md#open-questions))

---

## Defender support, intel, and warnings

Recovery, fog, scouting, and false objectives.

| # | Done | In store | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | --- | ---: | --- | --- | --- | --- |
| 10 | [x] | [x] | **Map Table** | 25 | **+1 fog-of-war reveal** each, stacking | `map_table/map_table_v5_SM` | **Base reveal 2**, **max 4**, **max two tables** per dungeon; 5×5 ft table, one square | `map_table.png` |
| — | [ ] | — | **Mending Node** | 30 | Not built | No art yet | Slow heal for defenders/constructs in room | — |
| — | [ ] | — | **Alarm Chime** | 15 | Not built | No art yet | Rings on invader entry; nearby defenders rally | — |
| — | [ ] | — | **Scrying Lens** | 20 | Not built | No art yet | Wide fog clear + show invaders in ring | — |
| — | [ ] | — | **Decoy Core** | 60 | Not built | No art yet | Dim core copy; invaders treat as real until reached; once per run | — |

**Fog-of-war (Map Table):** beyond dug floor, fog stays clear for N grid squares. **Base: 2.** Each Map Table **+1**, cap **4** (two tables).

**Stacking (open):** Map Table global reveal depth vs Scrying Lens local ring — see [Store.md](./Store.md#open-questions-for-these).

---

## Mimic

Creature sold as a store item; disguised as Loot Bait ([Store.md](./Store.md#mimic) for full behavior spec).

| Done | Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | --- | ---: | --- | --- |
| [ ] | **Mimic** | 35 | Looks like **Loot Bait** until opened; stick, bite, slow reveal, reset after wave | Punishes chest-opening; every real bait becomes a gamble |

---

## Unlock rules (all items)

From other design docs; not built in store UI yet.

| Rule | Source |
| --- | --- |
| **Progressive reveal:** tutorial unlocks each menu when taught; skip tutorial opens all | [Tutorial.md](./Tutorial.md) |
| **Absorb to unlock:** new monsters/traps from absorption, not only leveling — which store rows this affects TBD | [Overall.md](./Overall.md) |
| **Core level:** boss chambers and powered pockets capped by level — no store item tied yet | [Dungeon Core Levels.md](./Dungeon%20Core%20Levels.md) |

---

## In design docs but not store catalog rows

| Thing | Where |
| --- | --- |
| **Worker** | Essence at tutorial beat 3 ([Tutorial.md](./Tutorial.md)) |
| **Family creatures** | Creature menu ([Monsters.md](./Monsters.md)); store sells generator only |
| **Door and archway** | Room thresholds ([Theme.md](./Theme.md)) |
| **Rails** | Required by Runaway Cart ([Future.md](./Future.md)) |

---

## Category index (store #)

Quick lookup from HUD `ItemIndex` to section:

| # | Item | Section |
| ---: | --- | --- |
| 0 | Monster Generator | Generators and spawners |
| 1 | Mana Generator | Power and economy |
| 2 | Prism Beam Trap | Traps, barriers, and control |
| 3 | Resonance Plate Trap | Traps, barriers, and control |
| 4 | Gravity Well Trap | Traps, barriers, and control |
| 5 | Runaway Cart Trap | Traps, barriers, and control |
| 6 | Loot Bait | Lures and bait |
| 7 | Unshackled Golem | Defenders |
| 8 | Shard Sentinel | Defenders |
| 9 | Spore Generator | Generators and spawners |
| 10 | Map Table | Defender support, intel, and warnings |
| 11–13 | Ley Conduit, Mana Reservoir, Siphon Font | Power and economy (wired TBD) |