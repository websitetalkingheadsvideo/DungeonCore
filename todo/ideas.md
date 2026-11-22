# Game ideas

Brainstorming only — not a task list. Capture mechanics, traps, creatures, UX sparks, and “what if” moments here.

**Trap maintenance (global):** workers **reset** fired traps **between raids** unless a trap says otherwise; some types cost a **small mana** fee to re-arm.

**Time plan:** sleep minutes, raid chases, **multi-day captivity**, and **worker jobs** (dig, trap reset, …) share strategic time — see [time-passage-plan.md](./time-passage-plan.md) (Civ-style ticks + worker task queue).

**Raid lock (genre):** while **active raiders** are inside, the Core **cannot build** and the **store is hard-blocked** (no buy, place, or queue); strategic time **pauses**. After the raid ends, store and digging return even if a **captive** is still in a vault (drip on strategic clock).

**Workers & raids:** Core warns → **safe room** hide or **finish-if-close**; **killable**; interrupt save = **Crisis Discipline** on New Game (**Sanctuary Accord** / **Standard Delve** / **Iron Ley Compact**). [time-passage-plan.md](./time-passage-plan.md#crisis-discipline-new-game)

---

## Traps & defenses

### Smoke emitter

- Fills the **cell** (one gameplay block) with smoke.
- Hard to see through — vision and targeting degrade for invaders inside the cloud.
- **Disorientation**: invaders may lose their sense of position (pathing confusion, wrong room memory, hesitation).
- Thematic fit: area denial without direct damage; pairs with chokepoints and blind corners.

### Indiana Jones–style pit trap

**Vibe:** temple floor that *looks* like every other tile until weight triggers it — sudden drop, stakes at the bottom, party panic.

- **Grid:** one **floor cell** (300 UU ≈ 10 ft); pit is the **cell directly below** — dig empty space under the plate (no auto-void).
- **Outcome:** **heavy fall damage**, not insta-kill; victim **cannot leave** without help unless they have **climb-walls** (thief / rogue types).
- **After trigger:** floor **falls away** — open pit; rest of party **sees** ally at the bottom (no hidden state).
- **Second victim:** another invader does **not** auto-fall through the same hole while allies are visible; **low caution / reckless** units may **jump down** anyway (archetype + party AI).
- **Escape paths (party composition matters):**
  - **Climb walls** — thief / rogue (and similar) can self-extract when pit walls are climbable (default yes for standard pits).
  - **10 ft rope** — Scouts / thieves carry by default ([Character-Sheets.md](../Design/Character-Sheets.md#expedition-gear-invaders)); matches one cube depth; **rope rescue is easy for them** but still costs **time** (cast, anchor, haul — not a zero-frame save).
  - **Flight** — invaders with sustained fly (kit / ultimate tag on sheet) can extract self or assist; slower than “trivial skip.”
  - No rope, no climb, no fly → party must **commit time** at the edge or **abandon** and route on.
- **Balance goal:** never trivial auto-save; never guaranteed kill — **who showed up** decides whether the pit is a tempo win or a shrug.
- **Concealment:** normal floor until triggered; **Scouts / high trap literacy** can spot the plate before anyone steps.
- **Reuse:** workers re-arm **between raids** (global default); optional small mana cost.
- **Bottom dressing:** spikes optional (visual / Indy tone).
- **Trigger** (TBD): first step, weight threshold, remote arm.
- **Combos:** smoke (mis-step onto plate), displacement onto plate.
- **Still open:** exact rescue timing; jump-down weights per archetype; mana/cost; tutorial beat.
- **Sheet hooks:** `HasRope`, `RopeLengthUU` (300), `CanClimbWalls`, `HasSustainedFlight` — [Character-Sheets.md](../Design/Character-Sheets.md#expedition-gear-invaders).

### Indiana Jones–style blow-dart trap

**Vibe:** stone idol vents — trip a floor trigger and a **salvo of poisoned darts** riddles the **target cell** (temple corridor ambush).

- **Grid:** one **trigger cell** (pressure plate or trip cord at edge — TBD); darts affect **occupants of that cell** (same 300 UU cube).
- **Trigger:** invader steps on plate → trap fires **several darts** into the cell (VFX from walls, pillars, or hidden holes).
- **To-hit:** each dart (or each wave — TBD count) rolls **vs everyone currently in the cell**; hits apply poison exposure.
- **Dodge:** sheet skill or rating improves **dodge chance** vs dart to-hit (Scout / Assassin / rogue types higher; tank lower). Unharmed by a salvo = no poison save for that burst.
- **Poison (sleep drug):** on **hit**, resolve a **poison save** (same test system as other hazards — ability scores / saves TBD):
  | Outcome | Effect |
  | --- | --- |
  | **Major fail** | **Asleep** **30 minutes** (**raid-time**; see time plan). |
  | **Minor fail** | **Asleep** **5 minutes** (raid-time). |
  | **Minor success** | No sleep; **−1 to all tests** for **5 minutes** (raid-time). |
  | **Major success** | **No effect**. |
- **While asleep:** same family as [Spore Generator](../Design/Store.md) sleep — vulnerable, needs wake or time; parties with `Knows_SporeSleep`-style literacy may treat dart corridors like spore rooms after one witness ([Invaders.md](../Design/Invaders.md)).
- **Reuse:** workers re-arm **between raids**; optional small mana cost.
- **Balance:** dodge + save tiers = **not instant shutdown**, not **always shrug** — scouts slip through; careless parties lose a body for half an hour.
- **Combos:** pit plate **in** dart cell (step → darts → mis-step damage); smoke (harder to see plate); bait a cleric into the cell after tank soaks darts.
- **Still open:** dart count per salvo; one save per hit vs one save per trigger; does plate stay armed after first fire; mana/cost; `Knows_PoisonDarts` literacy tag.
- **Sheet hooks:** `TrapDodgeRating` (or derive from `Rating_Mobility` + archetype); poison save uses shared **`PoisonSave`** / **`Fortitude`** (name TBD) — parallel spore sleep in [Character-Sheets.md](../Design/Character-Sheets.md).

### Raiders idol vault *(big trap — very costly)*

**Vibe:** *Raiders of the Lost Ark* — glittering **treasure** on a pedestal; the moment an invader **takes it**, a **stone slab** drops at the **room’s exit**, sealing whoever didn’t bolt in time. Run for your life.

- **Store / cost:** flagship trap — **very expensive** mana and placement rules (single set-piece or one active vault per branch TBD).
- **Set-piece room:** on place, defines a **sealed chamber** **10×10×10 gameplay cells** (3000×3000×3000 UU). Not hand-dug tile-by-tile — prefab vault grafted onto the dungeon ([General.md](../Design/General.md) cube snap).
- **Layout:** **treasure** at the **far end** (pedestal / idol); **sliding wall** animates down at the **near / entrance end**, blocking retreat from the room.
- **Trigger:** invader **sees treasure** (loot draw / `TrustLoot`) → path to pedestal → **pickup** fires the mechanism (no pickup = vault stays idle).
- **Escape window:** wall closes over **N seconds** (tune). Race from pedestal back to exit:
  - **Fast / agile** (`MoveSpeed`, `Rating_Mobility`, dash/blink tags) — realistic sprint out.
  - **Slow** (tanks, wounded, encumbered) — caught.
  - **Observant** (high `ScoutRating`, trap literacy, witnessed vault before) — may **refuse the grab**, yell, or leave a scout outside as insurance.
- **Caught inside:** invader is **trapped in the vault** with the treasure until released or dead. Sealed volume — no pathing out through the slab.
- **Party counterplay:** rescue only if the group has **wall-breaking** (siege kit, boss bruiser ability, sustained mining fantasy) — rate-limited HP on the slab, noise attracts defenders, raid time burns. No break tool → captive is on a **slow death clock** over **strategic days** (realistically **days to ~a week** alive — starvation / thirst / hazard inside — TBD, not instant gib).
- **Core boon (the hook):** while the captive **stays alive** inside the vault, the Core gains a **steady mana + XP drip** on the **strategic clock** (distinct from one-shot kill essence in [Overall.md](../Design/Overall.md)). Holding them **alive for days** is the big win; killing early dumps only burst essence.
- **Risk / reward:** mis-set vault (no invaders greedy enough) = wasted premium; one scout snatches idol and everyone else escapes = little payoff; tank trapped for **several strategic days** = **massive** drip if the surface never frees them.
- **Relation to [Siphon Font](../Design/Store.md):** siphon = essence spike when invaders **fall in that room**; idol vault = **living** tap over strategic time. Stack both only if tuning allows.
- **Reuse:** workers reset the vault **only after trapped invader(s) die** — slab up, treasure restored, trap re-armed (mana cost TBD). **No** worker reset while someone is still alive inside.
- **Still open:** drip rate per strategic day vs death clock length; pause strategic time during raids; calendar UI; does wall damage defenders; tutorial / unlock band; literacy tag `Knows_IdolVault`; party AI split (one greedy, rest camp exit).
- **Sheet hooks:** `TrustLoot`, `Rating_Mobility`, dash/blink abilities; wall-break tags on kits; optional `GreedyLootWeight` per archetype.

---

## Time & captivity (summary)

Full notes: [time-passage-plan.md](./time-passage-plan.md).

| Pattern | Examples |
| --- | --- |
| **Civ / 4X** | Turns = calendar; **game speed** scales how long a “year” takes, not the rules. |
| **RimWorld** | Game hours/days for needs; **pause / 1×–3×**; fast-forward when safe. |
| **Dungeon Core (proposed)** | **Combat** (seconds) → **Raid** (minutes–hours, dart sleep) → **Strategic** (days–weeks, idol captive + worker maintenance). **No layout changes during an active raid.** |

---

## Scratch

_Add new sections or bullets as ideas land. Duplicate structure above (### title + bullets) when a idea deserves its own block._