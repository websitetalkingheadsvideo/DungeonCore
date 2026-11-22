---
name: brainstorm-game-objects
description: >-
  Brainstorms new store-placeable ideas for Dungeon Core (traps, economy, bait,
  defenders, intel). Outputs Design/Store.md table rows; category rules in
  constraints/. Use for store items, trap concepts, or todo/ideas expansions.
  Never edits Design/ unless the user explicitly asks.
---

# Store-placeable brainstormer

## Scope

**Parent:** **store-placeables** — anything that could become a store row, room prop, or parked catalog entry ([Design/Store.md](../../../Design/Store.md), [Design/store_items.md](../../../Design/store_items.md)).

**Children** (pick **one** per run): read the matching file under [constraints/](./constraints/):

| Child category | Constraints file |
| --- | --- |
| Traps, barriers, and control | [constraints/traps-barriers-control.md](./constraints/traps-barriers-control.md) |
| Power and economy | [constraints/power-and-economy.md](./constraints/power-and-economy.md) |
| Generators and spawners | [constraints/generators-spawners.md](./constraints/generators-spawners.md) |
| Lures and bait | [constraints/lures-and-bait.md](./constraints/lures-and-bait.md) |
| Defenders (constructs) | [constraints/defenders.md](./constraints/defenders.md) |
| Defender support, intel, warnings | [constraints/defender-support-intel.md](./constraints/defender-support-intel.md) |
| Mimic | [constraints/mimic.md](./constraints/mimic.md) |

**Traps** are a **subset** of store-placeables — when the child is traps, always load `traps-barriers-control.md`.

## Hard rules

1. **Do not** create, edit, rename, or delete anything under `Design/` unless the user **explicitly** asks in that chat.
2. **Default deliverable:** proposed rows in **chat** using the Store table below; append to [todo/ideas.md](../../../todo/ideas.md) only when the user asks.
3. **DRY:** before proposing mechanics, search existing store rows, [todo/ideas.md](../../../todo/ideas.md), and [Design/store_items.md](../../../Design/store_items.md); **list overlaps** in chat (bullet names + where they live).
4. **Art pillar:** Crystalline Ley-Line Arcana — read [Design/General.md](../../../Design/General.md) for tone; do not edit it.

## Phase 1: Context (before ideation)

Read in order:

| Source | Why |
| --- | --- |
| Child **constraints/** file | Category-specific rules |
| [Design/Store.md](../../../Design/Store.md) | Table format, placement, glow, wiring notes |
| [Design/store_items.md](../../../Design/store_items.md) | Category sections, existing rows |
| [todo/time-passage-plan.md](../../../todo/time-passage-plan.md) | Workers, raids, reset, difficulty, raid lock |
| [Design/Invaders.md](../../../Design/Invaders.md) | Trap literacy, party AI (read only) |

If the user did not name a **child category**, ask **one** question: which row in the child table above?

**One concept per run** unless the user requests a batch (then max **3** table rows, same child category).

## Phase 2: Ideation

- **Grounded (default):** fits grid, powered-room rules, invader behavior, worker maintenance (see time-passage-plan).
- **Wildcard (only if user asks):** one high-cost / set-piece idea — must still satisfy the child constraints file.

## Output format — Store.md table (required)

Every proposal includes **one markdown table** matching [Store.md § Items](../../../Design/Store.md#items):

```markdown
| # | Done | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | ---: | --- | --- | --- | --- |
| — | [ ] | **Item Name** | (suggest) | Not built | None, placeholder block | (design bullets; invader/worker/grid detail) | `slug.png` |
```

- **#** = `—` until assigned a HUD `ItemIndex`.
- **Price:** placeholder; compare sibling rows in the same category in store_items.md.
- **Needs to open:** mechanics, 300 UU grid/cell, powered room if relevant, invader counterplay, worker reset, literacy tags, combos with existing items.
- **Traps:** for deep mechanic specs (multi-bullet), add `### Item Name` under [todo/ideas.md](../../../todo/ideas.md) only when the user asks to append ideas — match existing trap sections there.

## Phase 3: Handoff

Ask the user:

1. Append detailed mechanics to `todo/ideas.md`?
2. Update a file under `constraints/` with new rules?
3. Edit `Design/Store.md` or `Design/store_items.md` — **only** if they explicitly permit `Design/` edits in that message.

Do not assign store row numbers in Design docs without permission.

## Related skills

- Visual prompts / icons: `firefly-store-item-prompt`, `asset-library-lookup`
- Out of scope: pure lore with no placement or store mechanic