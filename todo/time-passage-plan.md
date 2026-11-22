# Time passage — planning note

Brainstorm companion to [ideas.md](./ideas.md). No implementation yet.

## Why this exists

Traps reference **minutes** and **hours** of sleep; captives in the idol vault should survive **days to a week** while the Core milks mana/XP. That only works if we separate **how long things take in-fiction** from **how fast the player experiences the clock**.

---

## How “civilization” games usually handle time

They almost never use one clock for everything. They pick a **dominant layer** and abstract the rest.

### Turn-based 4X (Civilization, Old World, Humankind)

| Idea | What it does |
| --- | --- |
| **Discrete turns** | The world advances only when the turn ends (or phase ends). |
| **Turn = flexible calendar** | Early turns might be years; later turns can still be years but **costs are in turns**, not seconds. |
| **Game speed** | Quick / Standard / Epic / Marathon (Civ VI) **multiply** research, production, growth, and movement **costs** — same rules, different calendar density. |
| **Unit orders** | Combat might be tactical (one battle) or auto-resolve; either way it does not eat “50 years” on the map unless design says so. |

**Takeaway:** Calendar is a **label on ticks**. Balance is in **“how many ticks until X completes.”**

### Real-time 4X / empire builders (Age of Empires, Sins, some 4X hybrids)

| Idea | What it does |
| --- | --- |
| **Continuous clock** | Time always runs unless pause. |
| **Speed buttons** | 1× / 2× / 5× scale simulation rate, not rules. |
| **Tasks in real seconds** | “Train archer 30s” at 1×; same task at 2× finishes in 15 wall-clock seconds. |

**Takeaway:** Good when the player **watches** the map; still one **simulation delta** (`dt`) driving everything.

### Grand strategy ticks (Paradox: CK, EU, Stellaris)

| Idea | What it does |
| --- | --- |
| **Fixed tick** (often daily or hourly in-fiction) | Events, attrition, siege progress advance per tick. |
| **Pause anytime** | Strategic default. |
| **Speed** | 1×–5× changes tick rate. |

**Takeaway:** Captivity, siege, and “they stay alive for a week” map cleanly to **daily ticks**.

### Colony sim (RimWorld)

| Idea | What it does |
| --- | --- |
| **Ticks + in-game hours** | Needs, sleep, healing use **game hours/days**. |
| **Speed controls** | Pause / 1× / 2× / 3× (and “super speed” when no threats). |
| **Threat gating** | Fast-forward often disabled while raids/combat active. |

**Takeaway:** **Different speeds when safe vs under attack** — strong fit for a dungeon that alternates **raid** and **downtime**.

---

## Proposed layers for Dungeon Core

Use **one simulation clock** with named **epochs** so trap text can say “5 minutes” or “7 days” without lying.

| Layer | In-fiction | Typical use | Player feel |
| --- | --- | --- | --- |
| **Combat** | Seconds | Pathing, trap slam, wall-close sprint, dart volley | Real-time (raid active) |
| **Raid** | Minutes–hours | One surface party’s incursion; sleep from darts; rope rescue | Real-time or mild slow-mo; ends when party flees / wiped / objective done |
| **Strategic** | Hours–days–weeks | Captive in idol vault, starvation clock, worker re-arm, between-raid mana drip, world reacts | Pause + 1×/3×/5× when **no raid** |

**Conversion (placeholder — tune in play):**

- `1 strategic_day` = base unit for captivity and drip rates.
- At **Standard** speed: e.g. `1 strategic_day` = 60–120 s wall clock while unpaused and safe (RimWorld-like).
- **Raid active:** strategic clock **pauses** (locked — matches build lock below).

Captive survival **7 strategic_days** = a week in-fiction; at 90 s/day that's ~10.5 min wall clock at 1× — player can speed up when safe.

**Workers are the main consumer of strategic time.** Player **intent** (mark dig, arm reset, claim room) is often instant; **layout change** and **maintenance** advance only as worker tasks complete on the strategic clock — except where we later define instant Core powers.

---

## Workers — tasks, queue, and time

Family **Workers** dig and build ([Monsters.md](../Design/Monsters.md), [Tutorial.md](../Design/Tutorial.md) — mark block → worker shatters). Trap resets and future chores should use the **same job pipeline**, not a hidden instant fix.

### Task model (proposed)

| Field | Purpose |
| --- | --- |
| `TaskKind` | `DigCell`, `BreachCell`, `AbsorbRubble` (optional), `ResetTrap`, `BuildAssist`, … |
| `Target` | Cell, trap id, room id |
| `WorkRemainingUU` or `WorkSecondsStrategic` | How much **strategic** time left at 1× |
| `AssignedWorkerId` | Who is on it (0 = queued) |
| `Priority` | Player bump; default FIFO |

**Progress rule:** each strategic tick (or each `AdvanceStrategicTime(dt)`), subtract `dt × gameSpeed` from remaining work for **active** tasks. **Raid active:** strategic clock paused → **all worker task progress paused** (matches raid lock).

### Task kinds (starter list)

| Task | When queued | Duration driver (tune later) | Notes |
| --- | --- | --- | --- |
| **DigCell** | Player marks fog/wall block | Per-cell strategic time; harder rock = more | Tutorial blind dig; multi-cell tunnels = **chain** of jobs or one job with `N × cost` |
| **BreachCell** | Scripted / biome pocket | Same as dig or spike for tutorial beat 10 | May gate morph until complete |
| **ResetTrap** | End of raid (auto) or manual | Per trap type + optional mana fee | Pit / darts: default **between raids**. Idol vault: **only after captive dead** |
| **BuildAssist** | Player places store item (if we require worker install) | Short strategic beat | TBD: instant place vs worker carries part |
| _(future)_ haul, capillary extend, corpse drag | — | — | Same queue |

### Queue behavior

- **Between raids:** workers pull from queue; player can reorder/cancel if no raid.
- **`RaidApproach`:** no new tasks; in-flight **ResetTrap** may still tick per [Core policy](#warn-worker--hide-vs-finish-core-decision).
- **`RaidActive`:** no new tasks; in-flight tasks **freeze** (no half-dug exploit mid-fight).
- **Multiple workers:** parallel **different** targets; same target either one lead + helpers (faster) or blocked (no dogpile) — **open**.
- **ETA UI:** telemetry shows `~2 strategic hours` / `~1 day` using current speed — ties to calendar UI decision.

### Relationship to raid lock

| Actor | During active raid |
| --- | --- |
| **Player** dig / store / place | **Blocked** |
| **Worker** map-changing tasks | **No progress** (frozen) |
| **Worker** purely cosmetic / idle anim | OK |
| **Defenders** | Combat layer (real-time) |

After raid ends, **frozen tasks resume**; **auto trap-reset** jobs run when their conditions are met (see below).

### Trap reset via workers (recap)

- **Default:** after raid, queue `ResetTrap` for each spent trap (mana debit on **start** or **complete** — open).
- **Idol vault:** queue only when captive **dies**; until then vault stays sealed — no reset job.
- Reset duration is **not** instant: a stuck corridor might stay dangerous for **part of a strategic day** even though invaders are gone.

### Civ / RimWorld parallels

| Game | Worker time |
| --- | --- |
| **Civ** | Builder unit **movement + build turns**; speed setting changes turns per build. |
| **RimWorld** | `WorkToMake` / `WorkToBuild` drained per tick; speed = tick rate. |
| **Dungeon Core** | **Strategic** work remaining on task; **game speed** scales drain; **raid** = RimWorld threat pause. |

### Implementation hooks (future)

- `UWorkerTaskSubsystem` — queue, assign, advance on `TimeAuthority::StrategicDelta`.
- `UCoreBuildPolicy` — player orders create **tasks**, not immediate chunk delete (align with tutorial worker dig).
- Defender `WorkerRole` `Dig` / `Build` / `Both` on sheet ([Character-Sheets.md](../Design/Character-Sheets.md)) — speed multipliers per family later.

---

## Core awareness — who learns a raid is coming?

**Genre default:** the Core **knows what is in the dungeon** (litRPG books) — ley grid, creatures, traps, workers, fog state inside its influence. Workers do **not** scout or “hear rumors”; they get **Core directives** (link / command channel).

| Zone | Who knows what |
| --- | --- |
| **Powered / ley-sensed interior** | Core has **full situational awareness** — worker positions, reset progress, wounded defender. No separate worker perception check. |
| **Unpowered fog / far tunnels** | Core knows only what design allows (e.g. Will **one room ahead** of capillaries — [Will.md](../Design/Will.md)); workers are blind until the Core marks work. |
| **Surface / mouth approach** | Core learns via **raid telegraph** (reputation timer, Will scout ping, scrying lens item later) — not via the worker on the trap tile. |

**UX:** workers don’t spawn quest text; the Core (and Will bark) **warns the player**. Optional telegraph on the worker: icon when **FleeOrdered**.

### Raid phases (for timing drama)

| Phase | Strategic clock | Worker trap reset | Invaders |
| --- | --- | --- | --- |
| **Safe** | Runs | Progresses normally | None inside |
| **`RaidApproach`** | Runs | **Policy-driven** (below) | Announced / marching; **not yet** `RaidActive` |
| **`RaidActive`** | **Paused** | **Frozen** (no progress) | Inside → **raid lock** |

`RaidApproach` is the buffer where the Core asks: *finish the reset or pull the worker?*

### Trap reset — timer over the worker (player fantasy)

- Active `ResetTrap` shows **world-space or HUD countdown** on that worker (work remaining at current speed).
- Same number drives telemetry: *“Pit plate — 0:42”*.

### Warn worker → hide vs finish (core decision)

| Choice | Worker | Reset progress | Risk |
| --- | --- | --- | --- |
| **Abort & hide** | Path to a **safe room** and **hide** (idle until raid ends) | **Interrupted** — trap stays **spent** until reset finishes; **how much job progress you keep** = [Crisis Discipline](#crisis-discipline-new-game) | Low risk **if** they reach hiding before contact |
| **Hold until done** | Stay on task until bar hits **0** | **Completes**; then run for safe room before `RaidActive` if time allows | **High risk** — exposed on job; **killable** if caught |
| **Finish-if-close** (default proposal) | If `WorkRemaining ≤ FinishThreshold` **and** `ApproachTimeRemaining ≥ WorkRemaining` → **hold**; else **abort** | Complete or lose per row above | Readable drama without infinite delay |

**Can the Core delay the raid?** Only within **`RaidApproach`** — invaders are “near,” not inside. Optional:

- **Micro-delay:** if reset ETA is **≤ N strategic minutes** and player policy is **Finish-if-close**, extend approach clock by **at most** that ETA (hard cap so no stalling meta).
- **No delay** once party is at the mouth / line of sight — raid flips to **Active**; worker task **freezes**; trap still **spent** until a reset job hits **100%** (partial **job** progress may be banked — below).

Workers don’t “decide”; **Core policy + player preset** decides (see below). Defender escort to cover a finishing worker is future AI — v1 can be hide only.

### Crisis Discipline (New Game)

**Card row** on [New Game](../Design/New%20Game.md) — saved with the run. Sets what happens when a worker **leaves an unfinished job** (usually **flee to safe room** on raid warning). Independent of **Random Events** and **Randomness**.

| Setting | Player-facing name | On interrupt (flee / killed mid-task) |
| --- | --- | --- |
| `Sanctuary` | **Sanctuary Accord** | **100%** of completed work **banked** — same worker or another resumes from that point after the raid |
| `Standard` | **Standard Delve** (default) | Bank **up to 50%** of the task’s total work: `Banked = min(CompletedWork, TotalWork × 0.5)` |
| `IronLey` | **Iron Ley Compact** | **0%** banked — job restarts from scratch; trap stays fully **spent** |

- **CompletedWork** = `TotalWork − WorkRemaining` at flee moment.
- Trap is **armed** only when the reset job finishes (**100%**); partial bank = time saved, not a “half-working” trap (v1).
- **Death** mid-task uses the **same** bank rules as flee (still killable = stakes on Hard).
- **Finish-if-close** that completes before flee = no interrupt; trap arms normally.

### Safe rooms (decided)

When the Core issues **FleeOrdered** (default on abort, or after finish-if-close / finish-then-flee):

- Worker paths to the nearest **safe room** and enters **hide** state (no tasks, no map edits).
- **Safe room** = a **powered room** the player has flagged as safe (v1: default **core chamber** counts; later: designate in room UI or place a bolt-hole store item).
- Hiding is **not invulnerability** — invaders can **enter the safe room**, kill workers on the **path**, or catch a worker **still resetting**. Risk stays real.
- If the safe room is **unpowered** or **breached**, treat as normal combat space.

### Workers are killable (decided)

- Workers use normal **combat vitals** ([Character-Sheets.md](../Design/Character-Sheets.md) Worker tier — low durability, no real attack).
- **Invaders may target workers** during `RaidApproach` and `RaidActive` if they have line of sight / path (raid AI tag: `Prey_Worker` or priority below defenders).
- Death: task dropped, trap stays **spent**, respawn costs **essence** (same economy as spawn — tune later).
- **No** “immune while hiding” — hide only means they stop working and stay in a room the player chose; murder hobos still happen.

### Player policy presets (telemetry or codex)

| Preset | On `RaidApproach` |
| --- | --- |
| **Workers first** | Abort all maintenance immediately; hide |
| **Balanced** | Finish-if-close |
| **Traps first** | Longer finish threshold; higher risk tolerance (worker may die) |

Per-task override: pin one `ResetTrap` as **must complete** (idol vault excluded — different rules).

### Worker casualties

Caught on path, on task, or in a breached safe room → **death** (v1; injury / recover later optional). Apply **Crisis Discipline** bank rules to the dropped task; trap **spent** until a reset job reaches 100%.

### Open (raid × workers)

10. ~~**Hide goal**~~ → **safe room** (core default + player-designated powered rooms; bolt-hole item later).
11. ~~**Partial reset progress**~~ → **Crisis Discipline** on New Game (decided); v1 banks **job** progress only, not half-armed traps.
12. **Will line** on approach: single bark vs countdown pings at 60% / 30% approach time.
13. ~~**Invaders target workers?**~~ → **Yes** (decided).

---

## Raid lock — no construction while invaders are inside

**Genre default** (many dungeon-core LitRPGs): the Core cannot **reshape the dungeon** while the current raid is in progress — focus and mana go to **defense**, not earth-moving.

| While `RaidState == Active` | Allowed? |
| --- | --- |
| **Dig / delete blocks** | **No** |
| **Store place / rotate / edit items** | **No** |
| **Store open / buy** | **No** — entire store **hard-blocked** during raid (no purchase, no place, no queue) |
| **Worker orders** (reset trap, haul) that change the map | **No** |
| **Already-placed traps, monsters, beams** | **Yes** — they run as authored |
| **Camera, telemetry, codex** | **Yes** |

**Definition — “active raiders inside”:**

- At least one **living invader** on the current raid roster is **inside the dungeon** (within ley / dig boundary — exact volume TBD), and the raid has **not** ended (flee, wipe, retreat signal).
- **Does not** include a **lone captive** left after the party fled (raid ended, unit still in idol vault): **building allowed again**; **strategic time** and **mana/XP drip** from captivity **continue**.

That split keeps raids tense (no digging a panic tunnel) while still letting the player expand during a **week-long battery** between waves.

**UX:**

- Store panel **closed or disabled**; dig HUD **greyed**. One-line reason: *“Raid in progress — store and reshape when clear.”*
- Optional companion bark (per [Overall.md](../Design/Overall.md)) when player clicks disabled control.

**Implementation hook (future):** `UCoreBuildPolicy::CanMutateLayout()` → false when `RaidSubsystem->IsActive()` && `RaidSubsystem->HasInvaderInside()`.

---

## Effects on trap design (from ideas doc)

| Effect | Clock layer |
| --- | --- |
| Dart sleep 5 min / 30 min | **Raid** (or combat if raids stay short) |
| Pit rope rescue “takes time” | **Combat** within raid |
| Idol vault mana/XP drip | **Strategic** while captive alive |
| Captive death from hunger etc. | **Strategic** over days |
| Worker trap reset | **Strategic** — `ResetTrap` **task** after conditions met |

---

## Open decisions (need your call)

1. ~~**Strategic clock during raid**~~ → **pause** while raid active; captives between raids still tick (decided with raid lock).
2. **Calendar UI:** show `Day 42` on telemetry vs only “raid in 2 days.”
3. **Game speed on New Game card:** mirror Civ (Standard/Epic) or RimWorld (1×–3× anytime)?
4. **Sleep durations:** keep literal minutes in **raid time**, or rescale to % of raid length?
5. **One `TimeAuthority` in C++:** single `AdvanceStrategicTime`, `AdvanceRaidTime`, combat uses `World->GetDeltaSeconds()` only when `RaidState == Active`.
6. **Dig / place instant vs worker-only:** is *all* excavation worker-mediated post-tutorial, or does Core keep a slow instant dig?
7. **Trap reset automation:** auto-queue all spent traps at raid end vs player toggle per trap.
8. **Multiple workers on one tunnel:** speedup formula or single-worker only.
9. **Work costs essence/mana per strategic hour** while digging (upkeep) or flat per cell?

---

## Cross-links

- Ideas / traps: [ideas.md](./ideas.md)
- Essence loop: [Overall.md](../Design/Overall.md)
- Siphon / captive contrast: [Store.md](../Design/Store.md)